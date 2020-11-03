// $Id$

#include <iostream.h>
#include "tclcmd.hpp"

// We need the Tcl_Obj interface that was started in 8.0
#if TCL_MAJOR_VERSION < 8
#   error "we need Tcl 8.0 or greater to build this"
#elif defined(USE_TCL_STUBS) && TCL_MAJOR_VERSION == 8 && \
	(TCL_MINOR_VERSION == 0 || \
        (TCL_MINOR_VERSION == 1 && TCL_RELEASE_LEVEL != TCL_FINAL_RELEASE))
#   error "Stubs interface doesn't work in 8.0 and alpha/beta 8.1; only 8.1.0+"
#endif

#ifdef _MSC_VER
#   ifdef USE_TCL_STUBS
	// Mark this .obj as needing tcl's Stubs library.
#	pragma comment(lib, "tclstub" \
		STRINGIFY(JOIN(TCL_MAJOR_VERSION,TCL_MINOR_VERSION)) ".lib")
#	if !defined(_MT) || !defined(_DLL) || defined(_DEBUG)
	    // This fixes a bug with how the Stubs library was compiled.
	    // The requirement for msvcrt.lib from tclstubXX.lib should
	    // be removed.
#	    pragma comment(linker, "-nodefaultlib:msvcrt.lib")
#	endif
#   else
	// Mark this .obj needing the import library
#	pragma comment(lib, "tcl" \
		STRINGIFY(JOIN(TCL_MAJOR_VERSION,TCL_MINOR_VERSION)) ".lib")
#   endif
#endif

class TclHelloCmd : public TclCmd {
public:
  TclHelloCmd(Tcl_Interp * interp, char * name): TclCmd(interp, name) {};
private:    
  virtual int Command(int objc, Tcl_Obj * const objv[]);
  virtual void Cleanup() {
    cerr << "Cleanup!\n";
  };
};

int TclHelloCmd::Command(int objc, Tcl_Obj * const objv[]) {
  Tcl_Obj * result = Tcl_GetObjResult(tclInterp);

  Tcl_AppendToObj(result, "Hello,", -1);
  if (objc > 1) {
    for (int i = 1; i < objc; i++) {
      Tcl_AppendToObj(result, " ", -1);
      Tcl_AppendObjToObj(result, objv[i]);
    }
  } else {
     Tcl_AppendToObj(result, " World", -1);
  }
  Tcl_AppendToObj(result, "!", -1);
  return TCL_OK;
};

#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLEXPORT

EXTERN int Hellotcl_Init (Tcl_Interp *interp) {
#ifdef USE_TCL_STUBS
  if (Tcl_InitStubs(interp, "8.1", 0) == 0L) {
    return TCL_ERROR;
  }
#endif

  new TclHelloCmd(interp, "hello");

  Tcl_PkgProvide(interp, "hellotcl", "1.0");

  return TCL_OK;
}
