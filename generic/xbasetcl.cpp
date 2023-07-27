// $Id$

#include "tclxbase.hpp"

extern "C" {
  DLLEXPORT int Xbasetcl_Init(Tcl_Interp *interp);
  DLLEXPORT int Xbasetcl_SafeInit(Tcl_Interp *interp);
}

int Xbasetcl_Init (Tcl_Interp *interp)
{
#ifdef USE_TCL_STUBS
  if (Tcl_InitStubs(interp, "8.1", 0) == 0L) {
    return TCL_ERROR;
  }
#endif

  (void) new TclXbase(interp, "xbase");

  if (Tcl_PkgProvideEx(interp, PACKAGE_NAME, PACKAGE_VERSION, NULL) != TCL_OK) {
      return TCL_ERROR;
  }

  return TCL_OK;
}

int Xbasetcl_SafeInit (Tcl_Interp *interp)
{
  return Xbasetcl_Init(interp);
}
