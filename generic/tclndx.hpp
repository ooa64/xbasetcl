// $Id$

#ifndef TCLNDX_H
#define TCLNDX_H

#include <assert.h>

#include "tclindex.hpp"

class TclNdx : public TclIndex {

public:

  TclNdx(Tcl_Interp * interp, char * name, TclDbf * tcldbf)
    : TclIndex(interp, name, tcldbf) {
    index = new xbIxNdx(tcldbf->Dbf());
  };

  TclNdx(Tcl_Interp * interp, char * name, TclDbf * tcldbf, xbIxNdx * aindex)
    : TclIndex(interp, name, tcldbf) {
    assert(aindex);
    index = aindex;
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
