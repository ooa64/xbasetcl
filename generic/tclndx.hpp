// $Id$

#ifndef TCLNDX_H
#define TCLNDX_H

#include "tclindex.hpp"

class TclNdx : public TclIndex {

public:

  TclNdx(Tcl_Interp * interp, char * name, TclDbf * tcldbf)
    : TclIndex(interp, name, tcldbf) {
    index = new xbIxNdx(tcldbf->Dbf());
  };

  virtual ~TclNdx() {
    delete index;
  };

private:

  virtual int Type() {
    Tcl_AppendResult(tclInterp, "ndx", NULL);
    return TCL_OK;
  };

};

#endif
