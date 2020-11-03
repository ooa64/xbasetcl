// $Id$

#include "tclxbase.hpp"
#include "tcldbf.hpp"

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
  static char *commands[] = {
    "version", "dateformat", "encoding", "dbf", 0L
  };
  enum commands {
    cmVersion, cmDateformat, cmEncoding, cmDbf
  };
  int index;

  if (objc < 2) {
    Tcl_WrongNumArgs(interp, 1, objv, "command");
    return TCL_ERROR;
  }
  
  if (Tcl_GetIndexFromObj(interp, objv[1], 
			  commands, "command", 0, &index) != TCL_OK) {
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
    
  case cmDbf:
    
    if (objc != 3) {
      Tcl_WrongNumArgs(interp, 2, objv, "name");
      return TCL_ERROR;
    }
    (void) new TclDbf(interp, Tcl_GetString(objv[2]), this);
    Tcl_SetObjResult(interp, objv[2]);

    break;

  } // switch index

  return TCL_OK;
}
