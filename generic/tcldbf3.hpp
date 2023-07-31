// $Id$

#ifndef TCLDBF3_H
#define TCLDBF3_H

#include <assert.h>

#include "tcldbf.hpp"

class TclDbf3 : public TclDbf {

public:

  TclDbf3(Tcl_Interp * interp, char * name, TclXbase * tclxbase)
    : TclDbf(interp, name, tclxbase) {
    dbf = new xbDbf3(XBase());
  };

  TclDbf3(Tcl_Interp * interp, char * name, TclXbase * tclxbase, xbDbf3 * adbf)
    : TclDbf(interp, name, tclxbase) {
    assert(adbf);
    dbf = adbf;
  };

  virtual ~TclDbf3() {
    delete dbf;
  };

};

#endif

