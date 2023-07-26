// $Id$

#ifndef TCLDBF_H
#define TCLDBF_H

#include <xbase/xbase.h>
#include <tcl.h>

#include "tclcmd.hpp"
#include "tclxbase.hpp"
#include "tcldbf.hpp"

const unsigned MaxFieldNameLength = 10;

const unsigned SchemaIncludeName = 1;
const unsigned SchemaIncludeType = 2;
const unsigned SchemaIncludeLen = 4;
const unsigned SchemaIncludeDec = 8;
const unsigned SchemaIncludeAll = 15;

class TclDbf : public TclCmd {

public:

  TclDbf (Tcl_Interp * interp, char * name, TclXbase * tclxbase)
    : TclCmd(interp, name, tclxbase) {
    dbf = new xbDbf(XBase());
#ifdef TCL_UTF_MAX
    encoding = NULL;
    Tcl_DStringInit(&dstring);
#endif
  };

  virtual ~TclDbf() {
#ifdef TCL_UTF_MAX
    Tcl_DStringFree(&dstring);
    Tcl_FreeEncoding(encoding);
#endif
    delete dbf;
  };

#ifdef TCL_UTF_MAX
  inline Tcl_Encoding Encoding() {
    return (encoding ? encoding : ((TclXbase *)pParent)->Encoding());
  };

  inline CONST char * EncodeTclString(CONST char * s) {
    // make Tcl string from C string
    Tcl_DStringFree(&dstring);
    Tcl_ExternalToUtfDString(Encoding(), s, -1, &dstring);
    return Tcl_DStringValue(&dstring);
  };

  inline CONST char * DecodeTclString(CONST char * s) {
    // make C string from Tcl string
    Tcl_DStringFree(&dstring);
    Tcl_UtfToExternalDString(Encoding(), s, -1, &dstring);
    return Tcl_DStringValue(&dstring);
  };

  void ResetTclString() {
    Tcl_DStringFree(&dstring);
  };
#else
  inline char * EncodeTclString(const char * s) {return (char *)s;};
  inline char * DecodeTclString(const char * s) {return (char *)s;};
  inline void ResetTclString() {};
#endif

  inline int CheckRC (int rc) {return ((TclXbase *)pParent)->CheckRC(rc);};
  
  inline xbXBase * XBase() {return ((TclXbase *)pParent)->XBase();};
  inline xbDbf * Dbf() {return dbf;};

protected:

  xbDbf * dbf;

private:

#ifdef TCL_UTF_MAX
  Tcl_Encoding encoding;
  Tcl_DString dstring;
#endif

  virtual void Cleanup();
  virtual int Command (int objc, struct Tcl_Obj * CONST objv[]);

  int Create (Tcl_Obj * dbname, Tcl_Obj * schema, int overlay);
  int Schema (Tcl_Obj * result, unsigned include);
  int Fields (Tcl_Obj * result, Tcl_Obj * namev, Tcl_Obj * valuev);
};

#endif

