// $Id$

#ifndef TCLFILTER_H
#define TCLFILTER_H

#include <xbase/xbase.h>
#include <xbase/xbfilter.h>
#include <tcl.h>

#include "tclcmd.hpp"
#include "tclxbase.hpp"
#include "tcldbf.hpp"

class TclFilter : public TclCmd {

public:

  TclFilter(Tcl_Interp * interp, char * name, TclCmd * parent) : 
    TclCmd (interp, name, parent) {};

  virtual ~TclFilter() {};

  inline int CheckRC (int rc) {
    return ParentDbf()->CheckRC(rc);
  };

  xbXBase * XBase() {
    return ParentDbf()->XBase();
  };

  xbDbf * Dbf() {
    return ParentDbf()->Dbf();
  };

  xbFilter * Filter() {
    return filter;
  };

  virtual TclDbf * ParentDbf() = 0;

protected:

  xbFilter * filter;

private:

  virtual int Command (int objc, struct Tcl_Obj * CONST objv[]);
};

#endif
