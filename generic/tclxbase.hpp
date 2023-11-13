#ifndef TCLXBASE_H
#define TCLXBASE_H

#include <xbase.h>
#include <xbexp.h>
#include <tcl.h>

#include "tclcmd.hpp"

#ifndef XBASE_VERSION
#define XBASE_VERSION STRINGIFY(xbase_VERSION_MAJOR) "." STRINGIFY(xbase_VERSION_MINOR) "." STRINGIFY(xbase_VERSION_PATCH)
#endif

using namespace xb;

class TclXbase : public TclCmd {

public:

  TclXbase(Tcl_Interp * interp, CONST char * name) : TclCmd(interp, name) {
    Tcl_DStringInit(&dstring);
    encoding = NULL;
    xbase = new xbXBase();
    exp = new xbExp(xbase);
    expCache = NULL;
  };
  
  virtual ~TclXbase() {
    Tcl_DStringFree(&dstring);
    Tcl_FreeEncoding(encoding);
    if (expCache) Tcl_Free(expCache);
    delete exp;
    delete xbase;
  };

  inline Tcl_Encoding Encoding() {return encoding;};

  inline const char * EncodeTclString(const char * s) {
    // make Tcl string from C string
    Tcl_DStringFree(&dstring);
    Tcl_ExternalToUtfDString(encoding, s, -1, &dstring);
    return Tcl_DStringValue(&dstring);
  };

  inline const char * DecodeTclString(const char * s) {
    // make C string from Tcl string
    Tcl_DStringFree(&dstring);
    Tcl_UtfToExternalDString(encoding, s, -1, &dstring);
    return Tcl_DStringValue(&dstring);
  };  

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
  xbExp * exp;
  char * expCache;

private:

  Tcl_Encoding encoding;
  Tcl_DString dstring;  

  virtual void Cleanup();
  virtual int Command (int objc, struct Tcl_Obj * CONST objv[]);

  int Log(int objc, struct Tcl_Obj * CONST objv[]);

};

#endif
