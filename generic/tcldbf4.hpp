// $Id$

#ifndef TCLDBF4_H
#define TCLDBF4_H

#include "tcldbf.hpp"

class TclDbf4 : public TclDbf {

public:

  TclDbf4(Tcl_Interp * interp, char * name, TclXbase * tclxbase)
    : TclDbf(interp, name, tclxbase) {
    dbf = new xbDbf4(XBase());
  };

  virtual ~TclDbf4() {
    delete dbf;
  };

};

#endif

