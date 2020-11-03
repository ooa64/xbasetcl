// $Id$

#ifndef TCLDBFFILTER_H
#define TCLDBFFILTER_H

#include <xbase/xbase.h>
#include <xbase/xbfilter.h>
#include <tcl.h>

#include "tclcmd.hpp"
#include "tclxbase.hpp"
#include "tcldbf.hpp"
#include "tclfilter.hpp"

class TclDbfFilter : public TclFilter {

public:

  TclDbfFilter(Tcl_Interp * interp, char * name, char * expression,
		 TclDbf * parent) : TclFilter(interp, name, parent) {
    filter = new xbFilter(((TclDbf *)parent)->Dbf(), 
			  NULL, 
			  expression);
  }

  virtual ~TclDbfFilter() {
    delete filter;
  };

  virtual TclDbf * ParentDbf() {return (TclDbf *)pParent;};
};

#endif
