// $Id$

#ifndef TCLMDX_H
#define TCLMDX_H

#include <xbase.h>
#include <xbindex.h>
#include <tcl.h>

#include "tclcmd.hpp"
#include "tclxbase.hpp"
#include "tcldbf.hpp"
#include "tclindex.hpp"

class TclMdx : public TclIndex {

public:

  TclMdx (Tcl_Interp * interp, char * name, TclDbf * tcldbf)
    : TclIndex(interp, name, tcldbf) {
    index = new xbIxMdx(tcldbf->Dbf());
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
