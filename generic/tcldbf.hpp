// $Id$

#ifndef TCLDBF_H
#define TCLDBF_H

#include <assert.h>

#include "tclxbase.hpp"

const unsigned MaxFieldNameLength = 10;

const unsigned SchemaIncludeName = 1;
const unsigned SchemaIncludeType = 2;
const unsigned SchemaIncludeLen = 4;
const unsigned SchemaIncludeDec = 8;
const unsigned SchemaIncludeAll = 15;

class TclDbf : public TclCmd {

public:

  TclDbf(Tcl_Interp * interp, char * name, TclXbase * tclxbase)
    : TclCmd(interp, name, tclxbase), dbf(NULL), encoding(NULL) {
    assert(tclxbase);
    Tcl_DStringInit(&dstring);
  };

  virtual ~TclDbf() {
    Tcl_DStringFree(&dstring);
    Tcl_FreeEncoding(encoding);
  };

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

  inline int CheckRC (int rc) {return ((TclXbase *)pParent)->CheckRC(rc);};
  
  inline xbXBase * XBase() {return ((TclXbase *)pParent)->XBase();};
  inline xbDbf * Dbf() {return dbf;};

protected:

  xbDbf * dbf;

private:

  Tcl_Encoding encoding;
  Tcl_DString dstring;

  virtual void Cleanup();
  virtual int Command (int objc, struct Tcl_Obj * CONST objv[]);

  int Index (int objc, struct Tcl_Obj * CONST objv[]);
  int Create (Tcl_Obj * filename, Tcl_Obj * alias, Tcl_Obj * schema, int overlay, int share);
  int Schema (Tcl_Obj * result, unsigned include);
  int Fields (Tcl_Obj * result, Tcl_Obj * namev, Tcl_Obj * valuev);
};

#endif
