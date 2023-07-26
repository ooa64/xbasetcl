#include "tclfilter.hpp"

#if defined(DEBUG) && !defined(TCLFILTER_DEBUG)
#   undef DEBUGLOG
#   define DEBUGLOG(_x_)
#endif

#define interp tclInterp

int TclFilter::Command (int objc, struct Tcl_Obj * CONST objv[])
{
  static CONST char *commands[] = {
    "dbf", "first", "last", "prev", "next",
    0L
  };
  enum commands {
    cmDbf, cmFirst, cmLast, cmPrev, cmNext
  };
  int command, rc;

  if (objc < 2) {
    Tcl_WrongNumArgs(interp, 1, objv, "command");
    return TCL_ERROR;
  }

  if (Tcl_GetIndexFromObj(interp, objv[1],
                          (CONST char **)commands, "command", 0, &command) != TCL_OK) {
    return TCL_ERROR;
  }

  switch ((enum commands)(command)) {

  case cmDbf:

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    } else {
      Tcl_AppendResult(interp, ParentDbf()->Name(), NULL);
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
    case cmFirst: rc = filter->GetFirstFilterRec(); break;
    case cmLast: rc = filter->GetLastFilterRec(); break;
    case cmPrev: rc = filter->GetPrevFilterRec(); break;
    case cmNext: rc = filter->GetNextFilterRec(); break;
    default: rc = XB_NO_ERROR;
    }
    DEBUGLOG("filter " << Name() << " cmd " << command << " = " << rc);
    if (rc == XB_INVALID_RECORD || rc == XB_BOF || rc == XB_EOF) {
      Tcl_SetObjResult(interp, Tcl_NewIntObj(0));
    } else if (CheckRC(rc) != TCL_OK) {
      return TCL_ERROR;
    } else {
      Tcl_SetObjResult(interp, Tcl_NewLongObj(Dbf()->GetCurRecNo()));
    }

    break;

  default:
      return TCL_ERROR;
  }
  
  return TCL_OK;
}
