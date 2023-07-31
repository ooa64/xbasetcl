// $Id$

#ifndef TCLDBF4_H
#define TCLDBF4_H

#include <assert.h>

#include "tcldbf.hpp"

class TclDbf4 : public TclDbf {

public:

  TclDbf4(Tcl_Interp * interp, char * name, TclXbase * tclxbase)
    : TclDbf(interp, name, tclxbase) {
    dbf = new xbDbf4(XBase());
  };

  TclDbf4(Tcl_Interp * interp, char * name, TclXbase * tclxbase, xbDbf4 * adbf)
    : TclDbf(interp, name, tclxbase) {
    assert(adbf);
    dbf = adbf;
  };

  virtual ~TclDbf4() {
    delete dbf;
  };

};

#endif

