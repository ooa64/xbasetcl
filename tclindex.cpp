#include "tclindex.hpp"
#include "tclindexfilter.hpp"

#if defined(DEBUG) && !defined(TCLINDEX_DEBUG)
#   undef DEBUGLOG
#   define DEBUGLOG(_x_)
#endif

#define interp tclInterp

int TclIndex::Command (int objc, struct Tcl_Obj * CONST objv[])
{
  static char *commands[] = {
    "create", "open", "close", "reindex", "flush",
    "status", "name", "dbf",   "type",
    "first",  "last", "prev",  "next", "find",
    "filter",
    0L
  };
  enum commands {
    cmCreate, cmOpen, cmClose, cmReindex, cmFlush,
    cmStatus, cmName, cmDbf,   cmType,
    cmFirst,  cmLast, cmPrev,  cmNext, cmFind,
    cmFilter
  };
  int command, rc;

  if (objc < 2) {
    Tcl_WrongNumArgs(interp, 1, objv, "command");
    return TCL_ERROR;
  }

  if (Tcl_GetIndexFromObj(interp, objv[1],
                          commands, "command", 0, &command) != TCL_OK) {
    return TCL_ERROR;
  }

  switch ((enum commands)(command)) {

  case cmCreate:

    if (objc < 4 || objc > 7) {
      Tcl_WrongNumArgs(interp, 2, objv, 
		       "?-unique? ?-overlay? filename expression");
      return TCL_ERROR;
    } else {
	  int rc, unique = XB_NOT_UNIQUE, overlay = 0;
	  const char * indexname;
	  const char * expression;
	  Tcl_DString s;
	  
	  for (int i = 2; i < (objc - 2); i++) {
		if (strcmp(Tcl_GetString(objv[i]), "-unique") == 0) {
		  unique = XB_UNIQUE;
		} else if (strcmp(Tcl_GetString(objv[i]), "-overlay") == 0) {
		  overlay = 1;
		} else {
		  Tcl_AppendResult(interp, "bad option ", Tcl_GetString(objv[i]), NULL);
		  return TCL_ERROR;
		}
	  }
	  
	  indexname = Tcl_TranslateFileName(interp, Tcl_GetString(objv[objc - 2]), &s);
	  if (indexname == NULL) {
		return TCL_ERROR;
	  }
	  
	  expression = 
		((TclDbf *)pParent)->DecodeTclString(Tcl_GetString(objv[objc - 1]));
	  
	  rc = index->CreateIndex(indexname, expression, unique, overlay);
	  
	  Tcl_DStringFree(&s);
	  
	  if (CheckRC(rc) != TCL_OK) {
		return TCL_ERROR;
	  }
    }
    Tcl_AppendResult(interp, (const char *)index->IndexName, NULL);
	
    break;
	
  case cmOpen:

    if (objc != 3) {
      Tcl_WrongNumArgs(interp, 2, objv, "filename");
      return TCL_ERROR;
    } else {
	int rc;
	char * indexname;
	Tcl_DString s;
	
	indexname = Tcl_GetString(objv[2]);
	indexname = Tcl_TranslateFileName(interp, indexname, &s);
	if (indexname == NULL) {
	  return TCL_ERROR;
	}
	
	rc = index->OpenIndex(indexname);

	Tcl_DStringFree(&s);

	if (CheckRC(rc) != TCL_OK) {
	  return TCL_ERROR;
	}
    }
    Tcl_AppendResult(interp, (const char *)index->IndexName, NULL);

    break;

  case cmName:

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else {
      Tcl_AppendResult(interp, (const char *)index->IndexName, NULL);
    }
    
    break;

  case cmClose:

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else if (CheckRC(index->CloseIndex()) != TCL_OK) {
      return TCL_ERROR;
    }

    break;

  case cmReindex:

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else if (CheckRC(index->ReIndex()) != TCL_OK) {
      return TCL_ERROR;
    }

    break;

  case cmFlush:

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else {
	  index->Flush();
    }

    break;

  case cmStatus:

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else {
      switch (index->IndexStatus) {
      case XB_CLOSED: Tcl_AppendResult(interp, "closed", NULL); break;
      case XB_OPEN:   Tcl_AppendResult(interp, "open", NULL); break;
      default:
	Tcl_SetObjResult(interp, Tcl_NewIntObj(index->IndexStatus)); break;
      }
    }

    break;

  case cmDbf:

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else {
      Tcl_AppendResult(interp, ((TclDbf *)pParent)->Name(), NULL);
    }

    break;

  case cmType:

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else {
      return Type();
    }

    break;

  case cmFirst:
  case cmLast:
  case cmPrev:
  case cmNext:

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    }
    switch ((enum commands)command) {
    case cmFirst: rc = index->GetFirstKey(); break;
    case cmLast: rc = index->GetLastKey(); break;
    case cmPrev: rc = index->GetPrevKey(); break;
    case cmNext: rc = index->GetNextKey(); break;
    default: rc = XB_NO_ERROR;
    }
    // XB_INVALID_RECORD is for FirstKey on empty index
    if (rc == XB_INVALID_RECORD || rc == XB_BOF || rc == XB_EOF) {
      Tcl_SetObjResult(interp, Tcl_NewIntObj(0));
    } else if (CheckRC(rc) != TCL_OK) {
      return TCL_ERROR;
    } else {
      Tcl_SetObjResult(interp, Tcl_NewLongObj(index->GetCurDbfRec()));
    }

    break;

  case cmFind:

    if (objc != 3) {
      Tcl_WrongNumArgs(interp, 2, objv, "value");
      return TCL_ERROR;
    } else {
      rc = index->FindKey
	(((TclDbf *)pParent)->DecodeTclString(Tcl_GetString(objv[2])));

      // XB_NOT_FOUND is documented, XB_EOF is real NOTFOUND
      if (rc == XB_NOT_FOUND || rc == XB_EOF) {
	Tcl_SetObjResult(interp, Tcl_NewIntObj(0));
      } else if (rc == XB_FOUND) {
	Tcl_SetObjResult(interp, Tcl_NewLongObj(index->GetCurDbfRec()));
      } else {
	(void) CheckRC(rc);
	return TCL_ERROR;
      }
    }
    
    break;
    
  case cmFilter:

    if (objc != 4) {
      Tcl_WrongNumArgs(interp, 2, objv, "name expression");
      return TCL_ERROR;
    }
    (void) new TclIndexFilter(interp, 
			    Tcl_GetString(objv[2]), \
			    Tcl_GetString(objv[3]), this);
    Tcl_SetObjResult(interp, objv[2]);

    break;

  default:
      return TCL_ERROR;
  }
  
  return TCL_OK;
}
