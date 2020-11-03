// $Id$

#include "tclxbase.hpp"

#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLEXPORT

EXTERN int
Xbasetcl_Init (Tcl_Interp *interp)
{
#ifdef USE_TCL_STUBS
  if (Tcl_InitStubs(interp, "8.1", 0) == 0L) {
    return TCL_ERROR;
  }
#endif

  (void) new TclXbase(interp, "xbase");

  Tcl_PkgProvide(interp, "xbasetcl", "1.0");

  return TCL_OK;
}

EXTERN int
Xbasetcl_SafeInit (Tcl_Interp *interp)
{
  return Xbasetcl_Init(interp);
}
