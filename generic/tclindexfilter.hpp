// $Id$

#ifndef TCLINDEXFILTER_H
#define TCLINDEXFILTER_H

#include <xbase/xbase.h>
#include <xbase/xbfilter.h>
#include <tcl.h>

#include "tclcmd.hpp"
#include "tclxbase.hpp"
#include "tcldbf.hpp"
#include "tclindex.hpp"
#include "tclfilter.hpp"

class TclIndexFilter : public TclFilter {

public:

  TclIndexFilter(Tcl_Interp * interp, char * name, char * expression,
                 TclIndex * parent) : TclFilter(interp, name, parent) {
    filter = new xbFilter(((TclIndex *)parent)->Dbf(), 
                           ((TclIndex *)parent)->Index(), 
                           expression);
  }

  virtual ~TclIndexFilter() {
    delete filter;
  };

  virtual TclDbf * ParentDbf() {
    return (TclDbf *)((TclIndex *)pParent)->Parent();
  };

};

#endif
