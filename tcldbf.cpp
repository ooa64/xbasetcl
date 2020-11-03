#include "tcldbf.hpp"
#include "tcldbffilter.hpp"
#include "tclndx.hpp"
#include "tclntx.hpp"

#define F_SETLK 6
#define F_SETLKW 7

#if defined(DEBUG) && !defined(TCLDBF_DEBUG)
#   undef DEBUGLOG
#   define DEBUGLOG(_x_)
#endif

#define interp tclInterp

void TclDbf::Cleanup () {
  TclCmd * o;
  while ((o = pChildren)) {
    DEBUGLOG("TclDbf::Cleanup  *" << this << " for " << o);
    delete o;
  }
};

int TclDbf::Command (int objc, struct Tcl_Obj * CONST objv[])
{
  static char *commands[] = {
    "create", "open",    "close",  "pack",   "zap",
    "name",   "version", "status", "schema", "encoding",
    "blank",  "fields",  "record", "append", "update",   "deleted", 
    "first",  "last",    "prev",   "next",   "goto",     "position", "size",
    "index",  "filter",
    0L
  };
  enum commands {
    cmCreate, cmOpen,    cmClose,  cmPack,   cmZap, 
    cmName,   cmVersion, cmStatus, cmSchema, cmEncoding,
    cmBlank,  cmFields,  cmRecord, cmAppend, cmUpdate,   cmDeleted,
    cmFirst,  cmLast,    cmPrev,   cmNext,   cmGoto,     cmPosition, cmSize,
    cmIndex,  cmFilter
  };
  int index, rc;

  if (objc < 2) {
    Tcl_WrongNumArgs(interp, 1, objv, "command");
    return TCL_ERROR;
  }

  if (Tcl_GetIndexFromObj(interp, objv[1],
                          commands, "command", 0, &index) != TCL_OK) {
    return TCL_ERROR;
  }

  switch ((enum commands)(index)) {

  case cmEncoding:

#ifdef TCL_UTF_MAX
    if (objc > 3) {
      Tcl_WrongNumArgs(interp, 2, objv, "?encoding?");
      return TCL_ERROR;
    }
    else if (objc == 3) {
      Tcl_Encoding e = Tcl_GetEncoding(interp, Tcl_GetString(objv[2]));
      if (e) {
	Tcl_FreeEncoding(encoding);
	encoding = e;
      } else {
	return TCL_ERROR;
      }
    }
    Tcl_AppendResult(interp, Tcl_GetEncodingName(Encoding()), NULL);
#else
    Tcl_AppendResult(interp, "identity", NULL);
#endif
    
    break;


  case cmCreate:

    if (objc < 4 || objc > 5) {
      Tcl_WrongNumArgs(interp, 2, objv, "filename schema ?-overlay?");
      return TCL_ERROR;
    } else {
      int overlay = XB_DONTOVERLAY;
      if (objc == 5) {
	if (strcmp(Tcl_GetString(objv[4]), "-overlay") == 0) {
	  overlay = XB_OVERLAY;
	} else {
	  Tcl_AppendResult(interp, "bad option ", Tcl_GetString(objv[4]), NULL);
	  return TCL_ERROR;
	}
      }
      if (Create(objv[2], objv[3], overlay) != TCL_OK) {
	return TCL_ERROR;
      }
    }
    Tcl_AppendResult(interp, (const char *)dbf->GetDbfName(), NULL);

    break;

  case cmOpen:

    if (objc != 3) {
      Tcl_WrongNumArgs(interp, 2, objv, "filename");
      return TCL_ERROR;
    } else {
	int rc;
	char * xdbname;
	Tcl_DString s;
	
	xdbname = Tcl_GetString(objv[2]);
	xdbname = Tcl_TranslateFileName(interp, xdbname, &s);
	if (xdbname == NULL) {
	  return TCL_ERROR;
	}
	
	rc = dbf->OpenDatabase(xdbname);

	Tcl_DStringFree(&s);

	if (CheckRC(rc) != TCL_OK) {
	  return TCL_ERROR;
	}
    }
    Tcl_AppendResult(interp, (const char *)dbf->GetDbfName(), NULL);

    break;

  case cmClose:

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else if (CheckRC(dbf->CloseDatabase(0)) != TCL_OK) {
      return TCL_ERROR;
    }

    break;

  case cmPack:

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else if (CheckRC(dbf->PackDatabase(F_SETLKW, NULL, NULL)) != TCL_OK) {
      return TCL_ERROR;
    }

    break;

  case cmZap:

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else if (CheckRC(dbf->Zap(F_SETLKW)) != TCL_OK) {
      return TCL_ERROR;
    }

    break;

  case cmStatus:

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else {
      switch (dbf->GetDbfStatus()) {
      case XB_CLOSED:  Tcl_AppendResult(interp, "closed", NULL); break;
      case XB_OPEN:    Tcl_AppendResult(interp, "open", NULL); break;
      case XB_UPDATED: Tcl_AppendResult(interp, "updated", NULL); break;
      default:
	Tcl_SetObjResult(interp, Tcl_NewIntObj(dbf->GetDbfStatus())); break;
      }
    }
    
    break;
    
  case cmName:

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else {
      Tcl_AppendResult(interp, (const char *)dbf->GetDbfName(), NULL);
    }

    break;

  case cmVersion:
    
    if (objc > 3) {
      Tcl_WrongNumArgs(interp, 2, objv, "?integer?");
      return TCL_ERROR;
    } else {
      int ver;
      if (objc == 3) {
	if (Tcl_GetIntFromObj(interp, objv[2], &ver) != TCL_OK) {
	  return TCL_ERROR;
	}
      } else {
	ver = 0;
      }
      ver = dbf->SetVersion(ver);
      if (ver < 0) {
	(void) CheckRC(ver);
	return TCL_ERROR;
      }
      Tcl_SetObjResult(interp, Tcl_NewIntObj(ver));
    } 
    
    break;

  case cmSchema:

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else if (Schema(Tcl_GetObjResult(interp), SchemaIncludeAll) != TCL_OK) {
      return TCL_ERROR;
    }

    break;

  case cmBlank:
    
    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else if (CheckRC(dbf->BlankRecord()) != TCL_OK) {
      return TCL_ERROR;
    } else if (Fields(Tcl_GetObjResult(interp), NULL, NULL) != TCL_OK) {
      return TCL_ERROR;
    }
    
    break;

  case cmFields:

    if (objc > 4) {
      Tcl_WrongNumArgs(interp, 2, objv, "?names? ?values?");
      return TCL_ERROR;
    } else if (objc == 2) {
      Tcl_Obj * result = Tcl_GetObjResult(interp);
      for (int i = 0; i < dbf->FieldCount(); ++i) {
	Tcl_ListObjAppendElement
	  (NULL, result, 
	   Tcl_NewStringObj(EncodeTclString(dbf->GetFieldName(i)), -1));
      }
    } else if (Fields(Tcl_GetObjResult(interp), \
		      objv[2], (objc == 4) ? objv[3] : NULL) != TCL_OK) {
      return TCL_ERROR;
    }

    break;
    
  case cmRecord:
  case cmAppend:
  case cmUpdate:
    
    if (objc > 3) {
      Tcl_WrongNumArgs(interp, 2, objv, "?values?");
      return TCL_ERROR;
    } else if (Fields(Tcl_GetObjResult(interp), \
		      NULL, (objc == 3) ? objv[2] : NULL) != TCL_OK) {
      return TCL_ERROR;
    }
    switch ((enum commands)index) {
    case cmAppend: rc = dbf->AppendRecord(); break;
    case cmUpdate: rc = dbf->PutRecord(); break;
    default: rc = XB_NO_ERROR;
    }
    if (CheckRC(rc) != TCL_OK) {
      return TCL_ERROR;
    }
    
    break;
    
  case cmDeleted:
    
    if (objc > 3) {
      Tcl_WrongNumArgs(interp, 2, objv, "?boolean?");
      return TCL_ERROR;
    } else if (objc == 3) {
      int del;
      if (Tcl_GetBooleanFromObj(interp, objv[2], &del)) {
	return TCL_ERROR;
      }
      if (del) {
	if (CheckRC(dbf->DeleteRecord()) != TCL_OK) {
	  return TCL_ERROR;
	}
      } else {
	if (CheckRC(dbf->UndeleteRecord()) != TCL_OK) {
	  return TCL_ERROR;
	}
      }
    }
    Tcl_SetObjResult(interp, Tcl_NewIntObj(dbf->RecordDeleted()));
    
    break;

  case cmPosition:
  case cmFirst:
  case cmLast:
  case cmPrev:
  case cmNext:

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    }
    switch ((enum commands)index) {
    case cmFirst: rc = dbf->GetFirstRecord(); break;
    case cmLast: rc = dbf->GetLastRecord(); break;
    case cmPrev: rc = dbf->GetPrevRecord(); break;
    case cmNext: rc = dbf->GetNextRecord(); break;
    default: rc = XB_NO_ERROR;
    }
    if (rc == XB_EOF || rc == XB_BOF) {
      Tcl_SetObjResult(interp, Tcl_NewIntObj(0));
    } else if (CheckRC(rc) != TCL_OK) {
      return TCL_ERROR;
    } else {
      Tcl_SetObjResult(interp, Tcl_NewLongObj(dbf->GetCurRecNo()));
    }

    break;

  case cmGoto:

    if (objc != 3) {
      Tcl_WrongNumArgs(interp, 2, objv, "position");
      return TCL_ERROR;
    } else  {
      long position;
      if (Tcl_GetLongFromObj(interp, objv[2], &position)) {
	return TCL_ERROR;
      }
      if (CheckRC(dbf->GetRecord(position)) != TCL_OK) {
	return TCL_ERROR;
      }
    }
    Tcl_SetObjResult(interp, Tcl_NewLongObj(dbf->GetCurRecNo()));

    break;

  case cmSize:

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    }
    Tcl_SetObjResult(interp, Tcl_NewLongObj(dbf->NoOfRecords()));

    break;

  case cmIndex:
    
    if (objc < 3 || objc > 4) {
      Tcl_WrongNumArgs(interp, 2, objv, "name ?-ndx|-ntx?");
      return TCL_ERROR;
    } else if (objc == 4) {
      if (strcmp(Tcl_GetString(objv[3]), "-ndx") == 0) {
	(void) new TclNdx(interp, Tcl_GetString(objv[2]), this);
      } else if (strcmp(Tcl_GetString(objv[3]), "-ntx") == 0) {
	(void) new TclNtx(interp, Tcl_GetString(objv[2]), this);
      } else {
	Tcl_AppendResult(interp, "bad option ", Tcl_GetString(objv[3]), NULL);
	return TCL_ERROR;
      }
    } else {
      (void) new TclNdx(interp, Tcl_GetString(objv[2]), this);
    }
    Tcl_SetObjResult(interp, objv[2]);
    
    break;
    
  case cmFilter:
    
    if (objc != 4) {
      Tcl_WrongNumArgs(interp, 2, objv, "name expression");
      return TCL_ERROR;
    }
    (void) new TclDbfFilter(interp, 
			    Tcl_GetString(objv[2]), \
			    Tcl_GetString(objv[3]), this);
    Tcl_SetObjResult(interp, objv[2]);

    break;

  default:
      return TCL_ERROR;

  } // switch index
  
  return TCL_OK;
}

int TclDbf::Create (Tcl_Obj * dbname, Tcl_Obj * schema, int overlay)
{
  int fieldc;

  if (Tcl_ListObjLength(interp, schema, &fieldc) != TCL_OK) {
    return TCL_ERROR;
  } else if (fieldc <= 0) {
    Tcl_AppendResult(interp, "empty schema", NULL);
    return TCL_ERROR;
  } else {
    xbSchema * xschema = (xbSchema *)Tcl_Alloc(sizeof(xbSchema)*(fieldc+1));
    for (int i = 0; i < fieldc; ++i) {
      Tcl_Obj * field;
      Tcl_Obj * param;
      int paramc;
      if (Tcl_ListObjIndex(interp, schema, i, &field) != TCL_OK) {
	Tcl_Free((char *)xschema);
	return TCL_ERROR;
      }
      if (Tcl_ListObjLength(interp, field, &paramc) != TCL_OK) {
	Tcl_Free((char *)xschema);
	return TCL_ERROR;
      } else if (paramc != 4) {
	Tcl_Free((char *)xschema);
	Tcl_AppendResult(interp, "invalid schema, field #", NULL);
	Tcl_AppendObjToObj(Tcl_GetObjResult(interp), Tcl_NewIntObj(i));
	return TCL_ERROR;
      }
      char * fname;
      char * ftype;
      int flen;
      int fdec;
      if ((Tcl_ListObjIndex(NULL, field, 0, &param) != TCL_OK)
	  || (param == NULL)
	  || (strlen(fname = Tcl_GetString(param)) == 0)) {
	Tcl_Free((char *)xschema);
	Tcl_AppendResult(interp, "invalid fname, field #", NULL);
	Tcl_AppendObjToObj(Tcl_GetObjResult(interp), Tcl_NewIntObj(i));
	return TCL_ERROR;
      }
      if ((Tcl_ListObjIndex(NULL, field, 1, &param) != TCL_OK)
	  || (param == NULL)
	  || (strlen(ftype = Tcl_GetString(param)) != 1)) {
	Tcl_Free((char *)xschema);
	Tcl_AppendResult(interp, "invalid ftype, field #", NULL);
	Tcl_AppendObjToObj(Tcl_GetObjResult(interp), Tcl_NewIntObj(i));
	return TCL_ERROR;
      }
      if ((Tcl_ListObjIndex(NULL, field, 2, &param) != TCL_OK)
	  || (param == NULL)
	  || (Tcl_GetIntFromObj(NULL, param, &flen) != TCL_OK)
	  || (flen < 0)
	  || (flen > 255)) {
	Tcl_Free((char *)xschema);
	Tcl_AppendResult(interp, "invalid flen, field #", NULL);
	Tcl_AppendObjToObj(Tcl_GetObjResult(interp), Tcl_NewIntObj(i));
	return TCL_ERROR;
      }
      if ((Tcl_ListObjIndex(NULL, field, 3, &param) != TCL_OK)
	  || (param == NULL)
	  || (Tcl_GetIntFromObj(NULL, param, &fdec) != TCL_OK)
	  || (fdec < 0)
	  || (fdec > 255)) {
	Tcl_Free((char *)xschema);
	Tcl_AppendResult(interp, "invalid fdec, field #", NULL);
	Tcl_AppendObjToObj(Tcl_GetObjResult(interp), Tcl_NewIntObj(i));
	return TCL_ERROR;
      }
      strncpy(xschema[i].FieldName, DecodeTclString(fname),MaxFieldNameLength);
      xschema[i].Type = ftype[0];
      xschema[i].FieldLen = (unsigned char)flen;
      xschema[i].NoOfDecs = (unsigned char)fdec;
      DEBUGLOG("CreateDatabase "     << \
               Tcl_GetString(dbname) << " #" << i << " = " << \
	       (xschema[i].FieldName)        << ","  << \
	       (xschema[i].Type)             << ","  << \
	       (int)(xschema[i].FieldLen)    << ","  << \
	       (int)(xschema[i].NoOfDecs));
      }
      memset(&(xschema[fieldc]), 0, sizeof(xbSchema));

      {
	int rc;
	char * xdbname;
	Tcl_DString s;
	
	xdbname = Tcl_GetString(dbname);
	xdbname = Tcl_TranslateFileName(interp, xdbname, &s);
	if (xdbname == NULL) {
	  Tcl_Free((char *)xschema);
	  return TCL_ERROR;
	}
	
	rc = dbf->CloseDatabase(1);
	if (rc == XB_NO_ERROR) {
	  rc = dbf->CreateDatabase(xdbname, xschema, overlay);
	}

	Tcl_DStringFree(&s);

	if (CheckRC(rc) != TCL_OK) {
	  Tcl_Free((char *)xschema);
	  return TCL_ERROR;
	}
      }

      Tcl_Free((char *)xschema);
    }
    return TCL_OK;
}

int TclDbf::Schema (Tcl_Obj * result, unsigned include)
{
  for (int i = 0; i < dbf->FieldCount(); ++i) {
    char * fname = dbf->GetFieldName(i);
    char ftypech = dbf->GetFieldType(i);
    int flen = dbf->GetFieldLen(i);
    int fdec = dbf->GetFieldDecimal(i);
    
    Tcl_Obj * field = Tcl_NewObj();

    if (include & SchemaIncludeName)
      Tcl_ListObjAppendElement(NULL, field,
			       Tcl_NewStringObj(EncodeTclString(fname), -1));
    if (include & SchemaIncludeType) 
      Tcl_ListObjAppendElement(NULL, field, Tcl_NewStringObj(&ftypech, 1));
    if (include & SchemaIncludeLen) 
      Tcl_ListObjAppendElement(NULL, field, Tcl_NewIntObj(flen));
    if (include & SchemaIncludeDec) 
      Tcl_ListObjAppendElement(NULL, field, Tcl_NewIntObj(fdec));
    
    Tcl_ListObjAppendElement(NULL, result, field);
  }
  return TCL_OK;
}

int TclDbf::Fields (Tcl_Obj * result, Tcl_Obj * namev, Tcl_Obj * valuev)
{
  int namec;

  if (namev == NULL) {
    namec = dbf->FieldCount();
  } else if (Tcl_ListObjLength(interp, namev, &namec) != TCL_OK) {
    return TCL_ERROR;
  };


  for (int i = 0; i < namec; i++) {
    Tcl_Obj * name;
    Tcl_Obj * value;
    int fno;

    if (namev == NULL) {
      fno = i;
    } else {
      if (Tcl_ListObjIndex(interp, namev, i, &name) != TCL_OK) {
	return TCL_ERROR;
      } 
      fno = dbf->GetFieldNo(DecodeTclString(Tcl_GetString(name)));
      if (fno < 0) {
	Tcl_AppendResult(interp, "invalid field name ", Tcl_GetString(name), NULL);
	return TCL_ERROR;
      }
    }

    if (valuev != NULL) {
      if (Tcl_ListObjIndex(interp, valuev, i, &value) != TCL_OK) {
	return TCL_ERROR;
      }
      if (value != NULL) {
	int rc;
	DEBUGLOG("PutField " << dbf->GetDbfName() << " #" << fno << " = " << \
		 Tcl_GetString(value));
	rc = dbf->PutField(fno, DecodeTclString(Tcl_GetString(value)));
	if (CheckRC(rc)) {
	  Tcl_Obj * result = Tcl_GetObjResult(interp);
	  Tcl_AppendObjToObj(result, Tcl_NewStringObj(" at value #", -1));
	  Tcl_AppendObjToObj(result, Tcl_NewIntObj(i));
	  return TCL_ERROR;
	}
      }
    }

    xbString string = dbf->GetStringField(fno); 
    while (string[0] && string[0] == ' ') string.remove(0, 1);
    string.trim();
    Tcl_ListObjAppendElement
      (NULL, result, Tcl_NewStringObj(EncodeTclString(string), -1));
  }
  return TCL_OK;
}
