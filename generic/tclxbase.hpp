#ifndef TCLXBASE_H
#define TCLXBASE_H

#include <xbase.h>
#include <tcl.h>

#include "tclcmd.hpp"

#ifndef XBASE_VERSION
#define XBASE_VERSION STRINGIFY(xbase_VERSION_MAJOR) "." STRINGIFY(xbase_VERSION_MINOR) "." STRINGIFY(xbase_VERSION_PATCH)
#endif

using namespace xb;

class TclXbase : public TclCmd {

public:

  TclXbase(Tcl_Interp * interp, CONST char * name) :
    TclCmd(interp, name), encoding(NULL), xbase(new xbXBase()) {};
  
  virtual ~TclXbase() {
    Tcl_FreeEncoding(encoding);
    delete xbase;
  };

  Tcl_Encoding Encoding() {return encoding;};

  xbXBase * XBase() {return xbase;};

  int CheckRC (int rc) {
    if (rc < 0) {
      Tcl_ResetResult(tclInterp);
      Tcl_AppendResult(tclInterp, xbase->GetErrorMessage(rc), NULL);
      return TCL_ERROR;
    }
    return TCL_OK;
  }

protected:

  xbXBase * xbase;

private:

  Tcl_Encoding encoding;

  virtual void Cleanup();
  virtual int Command (int objc, struct Tcl_Obj * CONST objv[]);

  int Log(int objc, struct Tcl_Obj * CONST objv[]);

};

#endif
