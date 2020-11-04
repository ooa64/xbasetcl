#include <tcl.h>

#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLIMPORT

EXTERN int Xbasetcl_Init _ANSI_ARGS_ ((Tcl_Interp *));
EXTERN int Xbasetcl_SafeInit _ANSI_ARGS_ ((Tcl_Interp *));

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

int Xbasetcl_AppInit(Tcl_Interp *interp)
{
#ifdef USE_TCL_STUBS
    if (Tcl_InitStubs(interp, "8.1", 0) == 0L) {
        return TCL_ERROR;
    }
#endif

    if (Tcl_Init(interp) == TCL_ERROR) {
        return TCL_ERROR;
    }

    if (Xbasetcl_Init(interp) == TCL_ERROR) {
        return TCL_ERROR;
    }

    Tcl_StaticPackage(interp, "xbasetcl", Xbasetcl_Init, Xbasetcl_SafeInit);
    Tcl_SetVar(interp, "tcl_rcFileName", "~/xbaseshrc.tcl", TCL_GLOBAL_ONLY);
    return TCL_OK;
}
