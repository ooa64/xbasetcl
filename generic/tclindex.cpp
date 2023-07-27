#include "tclindex.hpp"

#if defined(DEBUG) && !defined(TCLINDEX_DEBUG)
#   undef DEBUGLOG
#   define DEBUGLOG(_x_)
#endif

#define interp tclInterp

int TclIndex::Command (int objc, struct Tcl_Obj * CONST objv[])
{
  static CONST char *commands[] = {
    "create", "open", "close", "reindex",
    "name",   "dbf",   "type",
    "first",  "last", "prev",  "next", "find",
    0L
  };
  enum commands {
    cmCreate, cmOpen, cmClose, cmReindex,
    cmName,   cmDbf,  cmType,
    cmFirst,  cmLast, cmPrev,  cmNext, cmFind
  };
  int command, rc;
  void *p; 

  if (objc < 2) {
    Tcl_WrongNumArgs(interp, 1, objv, "command");
    return TCL_ERROR;
  }

  if (Tcl_GetIndexFromObj(interp, objv[1],
                          (CONST char **) commands, "command", 0, &command) != TCL_OK) {
    return TCL_ERROR;
  }

  switch ((enum commands)(command)) {

  case cmCreate:

    if (objc < 4) {
      Tcl_WrongNumArgs(interp, 2, objv, 
               "?-unique? ?-overlay? ?-descending? filename expression");
      return TCL_ERROR;
    } else {
      int rc, descending = 0, unique = 0, overlay = 0;
      const char * indexname;
      const char * expression;
      const char * filter = "";

      Tcl_DString s;
      
      for (int i = 2; i < (objc - 2); i++) {
        if (strcmp(Tcl_GetString(objv[i]), "-unique") == 0) {
          unique = 1;
        } else if (strcmp(Tcl_GetString(objv[i]), "-overlay") == 0) {
          overlay = 1;
        } else if (strcmp(Tcl_GetString(objv[i]), "-descending") == 0) {
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
      
      rc = index->CreateTag(indexname, expression, filter, descending, unique, overlay, &p);
      
      Tcl_DStringFree(&s);
      
      if (CheckRC(rc) != TCL_OK) {
        return TCL_ERROR;
      }
    }
    Tcl_AppendResult(interp, (const char *)index->GetFileName(), NULL);
    
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
        
        rc = index->Open(indexname);

        Tcl_DStringFree(&s);

        if (CheckRC(rc) != TCL_OK) {
          return TCL_ERROR;
        }
    }
    Tcl_AppendResult(interp, (const char *)index->GetFileName(), NULL);

    break;

  case cmName:

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else {
      Tcl_AppendResult(interp, (const char *)index->GetFileName(), NULL);
    }
    
    break;

  case cmClose:

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else if (CheckRC(index->Close()) != TCL_OK) {
      return TCL_ERROR;
    }

    break;

  case cmReindex:

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else if (CheckRC(index->Reindex(&p)) != TCL_OK) {
      return TCL_ERROR;
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
    case cmFirst: rc = index->GetFirstKey(&p, 1); break;
    case cmLast: rc = index->GetLastKey(&p, 1); break;
    case cmPrev: rc = index->GetPrevKey(&p, 1); break;
    case cmNext: rc = index->GetNextKey(&p, 1); break;
    default: rc = XB_NO_ERROR;
    }
    // XB_INVALID_RECORD is for FirstKey on empty index
    if (rc == XB_INVALID_RECORD || rc == XB_BOF || rc == XB_EOF) {
      Tcl_SetObjResult(interp, Tcl_NewIntObj(0));
    } else if (CheckRC(rc) != TCL_OK) {
      return TCL_ERROR;
    } else {
      Tcl_SetObjResult(interp, Tcl_NewIntObj((((TclDbf *)pParent)->Dbf())->GetCurRecNo()));
    }

    break;

  case cmFind:

    if (objc != 3) {
      Tcl_WrongNumArgs(interp, 2, objv, "value");
      return TCL_ERROR;
    } else {
/*
  char cBuf[128];
  std::cin.getline( cBuf, 128 );
  xbInt16 iRc = 0;

  if( cKeyType == 'C' ){
    iRc = pIx->FindKey( vpCurTag, cBuf, (xbInt32) strlen( cBuf ), 1 );

  } else if( cKeyType == 'F' || cKeyType == 'N' ){
    xbDouble d = atof( cBuf );
    iRc = pIx->FindKey( vpCurTag, d, 1 );

  } else if( cKeyType == 'D' ){
    xbDate dt( cBuf );
    iRc = pIx->FindKey( vpCurTag, dt, 1 );
  }

  if( iRc == XB_NO_ERROR )
      dActiveTable->DumpRecord( dActiveTable->GetCurRecNo(), 2);
  else
      x->DisplayError( iRc );
 */      
      xbString s = ((TclDbf *)pParent)->DecodeTclString(Tcl_GetString(objv[2]));
      rc = index->FindKey(&p, s, 1);

      // XB_NOT_FOUND is documented, XB_EOF is real NOTFOUND
      if (rc == XB_NOT_FOUND) {
        Tcl_SetObjResult(interp, Tcl_NewIntObj(0));
      } else if (CheckRC(rc) != TCL_OK) {
        return TCL_ERROR;
      } else {
        Tcl_SetObjResult(interp, Tcl_NewIntObj((((TclDbf *)pParent)->Dbf())->GetCurRecNo()));
      }
    }
    
    break;
    
  default:
      return TCL_ERROR;
  }
  
  return TCL_OK;
}
