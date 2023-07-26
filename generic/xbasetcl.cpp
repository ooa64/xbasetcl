// $Id$

#include "tclxbase.hpp"

extern "C" {
  DLLEXPORT int Xbasetcl_Init(Tcl_Interp *interp);
  DLLEXPORT int Xbasetcl_SafeInit(Tcl_Interp *interp);
}

int Xbasetcl_Init (Tcl_Interp *interp)
{
#ifdef USE_TCL_STUBS
  if (Tcl_InitStubs(interp, "8.6", 0) == 0L) {
    return TCL_ERROR;
  }
#endif

  (void) new TclXbase(interp, "xbase");

  Tcl_PkgProvide(interp, "xbasetcl", "1.0");

  return TCL_OK;
}

int Xbasetcl_SafeInit (Tcl_Interp *interp)
{
  return Xbasetcl_Init(interp);
}
