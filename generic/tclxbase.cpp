// $Id$

#include "tclxbase.hpp"
#include "tcldbf3.hpp"
#include "tcldbf4.hpp"

#if defined(DEBUG) && !defined(TCLXBASE_DEBUG)
#   undef DEBUGLOG
#   define DEBUGLOG(_x_)
#endif

#define interp tclInterp

void TclXbase::Cleanup () {
  TclCmd * o;
  while ((o = pChildren)) {
    DEBUGLOG("TclXbase::Cleanup  *" << this << " for " << o);
    delete o;
  }
};

int TclXbase::Command (int objc, struct Tcl_Obj * CONST objv[])
{
  static CONST char *commands[] = {
    "version", "dateformat", "encoding", "list", "dbf", 0L
  };
  enum commands {
    cmVersion, cmDateformat, cmEncoding, cmList, cmDbf
  };
  int index;

  if (objc < 2) {
    Tcl_WrongNumArgs(interp, 1, objv, "command");
    return TCL_ERROR;
  }
  
  if (Tcl_GetIndexFromObj(interp, objv[1], 
                          (CONST char **)commands, "command", 0, &index) != TCL_OK) {
    return TCL_ERROR;
  }

  switch ((enum commands)(index)) {

  case cmVersion:

    if (objc > 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    }
    Tcl_AppendResult(interp, XBASE_VERSION, NULL);

    break;

  case cmDateformat:

    if (objc > 3) {
      Tcl_WrongNumArgs(interp, 2, objv, "?format?");
      return TCL_ERROR;
    } else if (objc == 3) {
      xbase->SetDefaultDateFormat(Tcl_GetString(objv[2]));
    }
    Tcl_AppendResult(interp, (const char *)xbase->GetDefaultDateFormat(), NULL);

    break;

  case cmEncoding:

#ifdef TCL_UTF_MAX
    if (objc > 3) {
      Tcl_WrongNumArgs(interp, 2, objv, "?encoding?");
      return TCL_ERROR;
    } else if (objc == 3) {
      Tcl_Encoding e = Tcl_GetEncoding(interp, Tcl_GetString(objv[2]));
      if (e) {
        Tcl_FreeEncoding(encoding);
        encoding = e;
      } else {
        return TCL_ERROR;
      }
    }
    Tcl_AppendResult(interp, Tcl_GetEncodingName(encoding), NULL);
#else
    Tcl_AppendResult(interp, "identity", NULL);
#endif

    break;
    
  case cmList:

    if (objc > 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else {
      Tcl_Obj * result = Tcl_GetObjResult(interp);
      for (int i = 1; i <= xbase->GetOpenTableCount(); ++i) {
        xbDbf * dbf = xbase->GetDbfPtr(i);
        if (dbf) {
          Tcl_ListObjAppendElement(NULL, result, 
              Tcl_NewStringObj((const char *)dbf->GetFqFileName(), -1));
        }
      }
    }

    break;

  case cmDbf:

    if (objc < 3 || objc > 4) {
      Tcl_WrongNumArgs(interp, 2, objv, "?-dbf3|-dbf4? name");
      return TCL_ERROR;
    } else if (objc > 3) {

      if (strcmp(Tcl_GetString(objv[2]), "-dbf3") == 0) {
        (void) new TclDbf3(interp, Tcl_GetString(objv[3]), this);
      } else if (strcmp(Tcl_GetString(objv[2]), "-dbf4") == 0) {
        (void) new TclDbf4(interp, Tcl_GetString(objv[3]), this);
      } else {
        Tcl_AppendResult(interp, "bad option ", Tcl_GetString(objv[2]), NULL);
        return TCL_ERROR;
      }
    } else {
      (void) new TclDbf4(interp, Tcl_GetString(objv[2]), this);
    }

    Tcl_SetObjResult(interp, objv[objc - 1]);   

  } // switch index

  return TCL_OK;
}
