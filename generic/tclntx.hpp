// $Id$

#ifndef TCLNTX_H
#define TCLNTX_H

#include <xbase/xbase.h>
#include <xbase/index.h>
#include <xbase/ntx.h>
#include <tcl.h>

#include "tclcmd.hpp"
#include "tclxbase.hpp"
#include "tcldbf.hpp"
#include "tclindex.hpp"

class TclNtx : public TclIndex {

public:

  TclNtx (Tcl_Interp * interp, char * name, TclDbf * tcldbf)
    : TclIndex(interp, name, tcldbf) {
    index = new xbNtx(tcldbf->Dbf());
  };

  virtual ~TclNtx() {
    delete index;
  };

private:

  virtual int Type() {
    Tcl_AppendResult(tclInterp, "ntx", NULL);
    return TCL_OK;
  };

};

#endif
