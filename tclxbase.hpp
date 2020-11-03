#ifndef TCLXBASE_H
#define TCLXBASE_H

#include <xbase/xbase.h>
#include <tcl.h>

#include "tclcmd.hpp"

class TclXbase : public TclCmd {

public:

  TclXbase(Tcl_Interp * interp, char * name) : TclCmd(interp, name) {
    xbase = new xbXBase();
#ifdef TCL_UTF_MAX
    encoding = NULL;
#endif
  };
  
  virtual ~TclXbase() {
#ifdef TCL_UTF_MAX
    Tcl_FreeEncoding(encoding);
#endif
    delete xbase;
  };

#ifdef TCL_UTF_MAX
  Tcl_Encoding Encoding() {return encoding;};
#endif

  xbXBase * XBase() {return xbase;};

  int CheckRC (int rc) {
    if (rc < 0) {
      Tcl_AppendResult(tclInterp, xbase->GetErrorMessage(rc), NULL);
      return TCL_ERROR;                                  	
    }
    return TCL_OK;
  }

protected:

  xbXBase * xbase;

private:

#ifdef TCL_UTF_MAX
  Tcl_Encoding encoding;
#endif

  virtual void Cleanup();
  virtual int Command (int objc, struct Tcl_Obj * CONST objv[]);

};

#endif
