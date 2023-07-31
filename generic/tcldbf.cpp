#include <assert.h>

#include "tcldbf.hpp"
#include "tclfilter.hpp"
#include "tclndx.hpp"
#include "tclmdx.hpp"

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
  static CONST char *commands[] = {
    "create", "open",   "close",   "pack",   "zap",      
    "name",   "alias",  "version", "status", "schema", "autocommit", "encoding",
    "blank",  "fields", "record",  "append", "update", "deleted",    "commit",   "abort",
    "first",  "last",   "prev",    "next",   "goto",   "position",   "size",
    "index",  "filter",
    0L
  };
  enum commands {
    cmCreate, cmOpen,   cmClose,   cmPack,   cmZap,
    cmName,   cmAlias,  cmVersion, cmStatus, cmSchema, cmAutocommit, cmEncoding,
    cmBlank,  cmFields, cmRecord,  cmAppend, cmUpdate, cmDeleted,    cmCommit,   cmAbort,
    cmFirst,  cmLast,   cmPrev,    cmNext,   cmGoto,   cmPosition,   cmSize,
    cmIndex,  cmFilter
  };
  int index, rc;

  if (objc < 2) {
    Tcl_WrongNumArgs(interp, 1, objv, "command");
    return TCL_ERROR;
  }

  if (Tcl_GetIndexFromObj(interp, objv[1],
                          (CONST char **)commands, "command", 0, &index) != TCL_OK) {
    return TCL_ERROR;
  }

  assert(dbf);

  switch ((enum commands)(index)) {

  case cmCreate:

    if (objc < 4 || objc > 5) {
      Tcl_WrongNumArgs(interp, 2, objv, "?-overlay? filename schema");
      return TCL_ERROR;
    } else {
      if (objc == 5) {
        if (strcmp(Tcl_GetString(objv[2]), "-overlay") == 0) {
          if (Create(objv[3], objv[4], XB_OVERLAY) != TCL_OK) {
            return TCL_ERROR;
          }
        } else {
          Tcl_AppendResult(interp, "bad option ", Tcl_GetString(objv[4]), NULL);
          return TCL_ERROR;
        }
      } else {  
        if (Create(objv[2], objv[3], XB_DONTOVERLAY) != TCL_OK) {
          return TCL_ERROR;
        }
      }
    }
    Tcl_AppendResult(interp, (const char *)dbf->GetFqFileName(), NULL);

    break;

  case cmOpen:

    if (objc < 3 || objc > 4) {
      Tcl_WrongNumArgs(interp, 2, objv, "filename ?alias?");
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
    
      rc = dbf->Close();
      if (rc == XB_NO_ERROR) {
        rc = dbf->Open(xdbname, (objc == 4) ? Tcl_GetString(objv[3]) : Tcl_GetString(objv[2]));
      }

      Tcl_DStringFree(&s);

      if (CheckRC(rc) != TCL_OK) {
        return TCL_ERROR;
      }
    }
    Tcl_AppendResult(interp, (const char *)dbf->GetFqFileName(), NULL);

    break;

  case cmClose:

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else if (CheckRC(dbf->Close()) != TCL_OK) {
      return TCL_ERROR;
    }

    break;

  case cmPack:

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else if (CheckRC(dbf->Pack()) != TCL_OK) {
      return TCL_ERROR;
    }

    break;

  case cmZap:

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else if (CheckRC(dbf->Zap()) != TCL_OK) {
      return TCL_ERROR;
    }

    break;

  case cmName:

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else {
      Tcl_AppendResult(interp, (const char *)dbf->GetFqFileName(), NULL);
    }

    break;

  case cmAlias:

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else {
      Tcl_AppendResult(interp, (const char *)dbf->GetTblAlias(), NULL);
    }

    break;

  case cmVersion:
    
    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else {
      Tcl_SetObjResult(interp, Tcl_NewIntObj(dbf->GetVersion()));
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
        default:         Tcl_SetObjResult(interp, Tcl_NewIntObj(dbf->GetDbfStatus())); break;
      }
    }
    
    break;
    
  case cmSchema:

    if (objc == 2) {
      if (Schema(Tcl_GetObjResult(interp), SchemaIncludeAll) != TCL_OK) {
        return TCL_ERROR;
      }
    } else if (objc == 4) {
      if (strcmp(Tcl_GetString(objv[2]), "-fieldno") == 0) {
        int fno = dbf->GetFieldNo(DecodeTclString(Tcl_GetString(objv[3])));
        if (fno >= 0) {
          Tcl_SetObjResult(interp, Tcl_NewIntObj(fno));
        } else {
          Tcl_AppendResult(interp, "field does not exist", NULL);
          return TCL_ERROR;
        }
      } else {
        Tcl_AppendResult(interp, "invalid option ",
                         Tcl_GetString(objv[2]), NULL);
        return TCL_ERROR;
      }
    } else {
      Tcl_WrongNumArgs(interp, 2, objv, "?-fieldno fieldname?");
      return TCL_ERROR;
    }

    break;

  case cmAutocommit:
    
    if (objc > 3) {
      Tcl_WrongNumArgs(interp, 2, objv, "?boolean?");
      return TCL_ERROR;
    } else if (objc == 3) {
      int autocommit;
      if (Tcl_GetBooleanFromObj(interp, objv[2], &autocommit)) {
        return TCL_ERROR;
      }
      if (CheckRC(dbf->SetAutoCommit(autocommit)) != TCL_OK) {
        return TCL_ERROR;
      }
    }
    Tcl_SetObjResult(interp, Tcl_NewIntObj(dbf->GetAutoCommit()));
    
    break;

  case cmEncoding:

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
    
    break;

  case cmBlank:
    
    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else if (CheckRC(dbf->BlankRecord()) != TCL_OK) {
      return TCL_ERROR;
    }
    
    break;

  case cmFields:

    if (objc > 4) {
      Tcl_WrongNumArgs(interp, 2, objv, "?names? ?values?");
      return TCL_ERROR;
    } else if (objc == 2) {
      Tcl_Obj * result = Tcl_GetObjResult(interp);
      xbString name;
      for (int i = 0; i < dbf->GetFieldCnt(); ++i) {
        if (CheckRC(dbf->GetFieldName(i,name)) != TCL_OK) {
          return TCL_ERROR;
        } else {
          Tcl_ListObjAppendElement
            (NULL, result, 
             Tcl_NewStringObj(EncodeTclString(name.Str()), -1));
          }
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
    // NOTE: xbase64-4.1.4 crashes updating unopened dbf
    if (dbf->GetDbfStatus() == XB_CLOSED && index == cmUpdate) {
      Tcl_AppendResult(interp, "(crash)", NULL);
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
    
  case cmCommit:

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else {
      dbf->Commit();
    }

    break;

  case cmAbort:

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else {
      dbf->Abort();
    }

    break;

  case cmDeleted:
    
    if (objc < 2 || objc > 3) {
      Tcl_WrongNumArgs(interp, 2, objv, "?boolean?");
      return TCL_ERROR;
    } else if (objc == 3) {
      int deleted;
      if (Tcl_GetBooleanFromObj(interp, objv[2], &deleted)) {
        return TCL_ERROR;
      }
      if (deleted) {
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
    } else {
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
    } else {
      unsigned int count;
      if (CheckRC(dbf->GetRecordCnt(count)) != TCL_OK) {
        return TCL_ERROR;
      }
      Tcl_SetObjResult(interp, Tcl_NewLongObj(count));
    }

    break;

  case cmIndex:
    
    if (objc < 3 || objc > 4) {
      Tcl_WrongNumArgs(interp, 2, objv, "?-ndx|-mdx? name");
      return TCL_ERROR;
    } else if (objc == 4) {
      if (strcmp(Tcl_GetString(objv[2]), "-ndx") == 0) {
        (void) new TclNdx(interp, Tcl_GetString(objv[3]), this);
      } else if (strcmp(Tcl_GetString(objv[2]), "-mdx") == 0) {
        (void) new TclMdx(interp, Tcl_GetString(objv[3]), this);
      } else {
        Tcl_AppendResult(interp, "bad option ", Tcl_GetString(objv[2]), NULL);
        return TCL_ERROR;
      }
    } else {
      (void) new TclNdx(interp, Tcl_GetString(objv[2]), this);
    }

    Tcl_SetObjResult(interp, objv[objc - 1]);
    
    break;
    
  case cmFilter:
    
    if (objc != 4) {
      Tcl_WrongNumArgs(interp, 2, objv, "name expression");
      return TCL_ERROR;
    }
    (void) new TclFilter(interp, 
                         Tcl_GetString(objv[2]),
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
      strncpy(xschema[i].cFieldName, DecodeTclString(fname),MaxFieldNameLength);
      xschema[i].cType = ftype[0];
      xschema[i].iFieldLen = (unsigned char)flen;
      xschema[i].iNoOfDecs = (unsigned char)fdec;
      DEBUGLOG("CreateDatabase "     << \
               Tcl_GetString(dbname) << " #" << i << " = " << \
               (xschema[i].cFieldName)        << ","  << \
               (xschema[i].cType)             << ","  << \
               (int)(xschema[i].iFieldLen)    << ","  << \
               (int)(xschema[i].iNoOfDecs));
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
    
      rc = dbf->Close();
      if (rc == XB_NO_ERROR) {
        // FIXME: sAlias: fullfilename or justfilename ?
        // FIXME: iShareMode: XB_SINGLE_USER or XB_MULTI_USER ?
        rc = dbf->CreateTable(xdbname, xdbname, xschema, overlay, XB_SINGLE_USER);
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
  xbString fname;
  char ftypech;
  short flen;
  short fdec;
  for (int i = 0; i < dbf->GetFieldCnt(); ++i) {
    if (CheckRC(dbf->GetFieldName(i,fname)) != TCL_OK) {
      return TCL_ERROR;
    }
    if (CheckRC(dbf->GetFieldType(i,ftypech)) != TCL_OK) {
      return TCL_ERROR;
    }
    if (CheckRC(dbf->GetFieldLen(i,flen)) != TCL_OK) {
      return TCL_ERROR;
    }
    if (CheckRC(dbf->GetFieldDecimal(i,fdec)) != TCL_OK) {
      return TCL_ERROR;
    }
    
    Tcl_Obj * field = Tcl_NewObj();

    if (include & SchemaIncludeName)
      Tcl_ListObjAppendElement(NULL, field,
                               Tcl_NewStringObj(EncodeTclString(fname.Str()), -1));
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
    namec = dbf->GetFieldCnt();
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
        Tcl_ResetResult(interp);
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
        char ftypech;
        if (CheckRC(dbf->GetFieldType(fno,ftypech)) != TCL_OK) {
          return TCL_ERROR;
        }
        switch (ftypech) {
        case 'M':
          {
            const char * s = DecodeTclString(Tcl_GetString(value));
            if (strlen(s)) {
              DEBUGLOG("UpdateMemo " << dbf->GetFileName().Str() << " #"  << fno << " = " << \
                       strlen(s) << " chars: " << s);
              rc = dbf->UpdateMemoField(fno, s);
            } else {
              DEBUGLOG("DeleteMemo " << dbf->GetFileName().Str() << " #" << fno);
              rc = dbf->UpdateMemoField(fno, "");
            }
          }
          break;
        case 'L':
          {
            int b;
            if (Tcl_GetBooleanFromObj(NULL, value, &b) == TCL_OK) {
              rc = dbf->PutLogicalField(fno, b ? "T" : "F");
            } else {
              rc = dbf->PutLogicalField(fno, Tcl_GetString(value));
            }
          }
          break;
        case 'C':
          rc = dbf->PutField(fno, DecodeTclString(Tcl_GetString(value)));
          break;
        default:
          rc = dbf->PutField(fno, Tcl_GetString(value));
        }
        if (CheckRC(rc)) {
          Tcl_Obj * result = Tcl_GetObjResult(interp);
          Tcl_AppendObjToObj(result, Tcl_NewStringObj(" at value #", -1));
          Tcl_AppendObjToObj(result, Tcl_NewIntObj(i));
          return TCL_ERROR;
        }
      }
    }

    Tcl_Obj * obj;

    char ftypech;
    if (CheckRC(dbf->GetFieldType(fno, ftypech)) != TCL_OK) {
      return TCL_ERROR;
    }
    switch (ftypech) {
    case 'M':
      if (dbf->MemoFieldExists(fno)) {
        xbString buf;
        if (CheckRC(dbf->GetMemoField(fno, buf)) != TCL_OK) {
          return TCL_ERROR;
        }
        int flen = buf.GetSize() > 0 ? buf.GetSize() - 1 : 0;
        obj = Tcl_NewStringObj(EncodeTclString(buf.Str()), flen);
#if DEBUG        
        // buf[flen] = 0; ???
        DEBUGLOG("GetMemo " << dbf->GetFileName().Str() << " #" << fno << " = " << \
                 flen << " chars: " << buf.Str());
#endif
      } else {
        obj = Tcl_NewStringObj(NULL, 0);
      }
      break;
      //case 'L':
      //obj = Tcl_NewIntObj(dbf->GetLogicalField(fno));
      //obj = Tcl_NewStringObj(dbf->GetField(fno), -1);
      //break;
    case 'C':
      //{
      //    xbString s = dbf->GetStringField(fno); 
      //    while (s[0] && s[0] == ' ') s.remove(0, 1);
      //    s.trim();
      //    obj = Tcl_NewStringObj(EncodeTclString(s), -1);
      //}
      //obj = Tcl_NewStringObj(EncodeTclString(dbf->GetStringField(fno)), -1);
      //obj = Tcl_NewStringObj(EncodeTclString(dbf->GetField(fno)), -1);
        {
          xbString s;
          if (CheckRC(dbf->GetField(fno, s)) != TCL_OK) {
            return TCL_ERROR;
          }
          // if (s[0] == ' ' && s[1] == 0) {
          //   // workaround to the GetField() feature
          //   obj = Tcl_NewStringObj(NULL, 0);
          // } else {
          //   obj = Tcl_NewStringObj(EncodeTclString(s), -1);
          // }
          obj = Tcl_NewStringObj(EncodeTclString(s.Trim()), -1);
        }
        break;
      //case 'N':
      //case 'F':
      //if (dbf->GetFieldDecimal(fno) == 0 && dbf->GetFieldDecimal(fno) < 10) {
      //    obj = Tcl_NewLongObj(dbf->GetLongField(fno));
      //} else {
      //    obj = Tcl_NewDoubleObj(dbf->GetDoubleField(fno));
      //}
      //{
      //  const char * s = dbf->GetField(fno); 
      //  while (s && *s == ' ') s++; 
      //  obj = Tcl_NewStringObj(s, -1);
      // }
      // break;
    default:
      // obj = Tcl_NewStringObj(dbf->GetField(fno), -1);
      {
        xbString s;
        if (CheckRC(dbf->GetField(fno, s)) != TCL_OK) {
          return TCL_ERROR;
        }
        obj = Tcl_NewStringObj(s.Ltrim(), -1);
      }
    }
    if (Tcl_ListObjAppendElement(interp, result, obj) != TCL_OK) {
      Tcl_Obj * obj = Tcl_GetObjResult(interp);
      Tcl_AppendObjToObj(obj, Tcl_NewStringObj(" at value #", -1));
      Tcl_AppendObjToObj(obj, Tcl_NewIntObj(i));
      return TCL_ERROR;
    }
  }
  
  return TCL_OK;
}
