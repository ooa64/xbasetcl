// $Id$

#ifndef TCLMDX_H
#define TCLMDX_H

#include "tclindex.hpp"

class TclMdx : public TclIndex {

public:

  TclMdx(Tcl_Interp * interp, char * name, TclDbf * tcldbf)
    : TclIndex(interp, name, tcldbf) {
    index = new xbIxMdx(tcldbf->Dbf());
  };

  TclMdx(Tcl_Interp * interp, char * name, TclDbf * tcldbf, xbIxMdx * aindex)
    : TclIndex(interp, name, tcldbf) {
    assert(aindex);
    index = aindex;
  };

  virtual ~TclMdx() {
    delete index;
  };

private:

  virtual int Type() {
    Tcl_AppendResult(tclInterp, "mdx", NULL);
    return TCL_OK;
  };

};

#endif
