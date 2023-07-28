// $Id$

#ifndef TCLFILTER_H
#define TCLFILTER_H

#include "tclcmd.hpp"
#include "tcldbf.hpp"

class TclFilter : public TclCmd {

public:

  TclFilter(Tcl_Interp * interp, char * name, char * expression, TclCmd * parent) : 
            TclCmd (interp, name, parent) {
    filter = new xbFilter(((TclDbf *)parent)->Dbf());
    filter->Set(expression);
  };

  virtual ~TclFilter() {
    delete filter;
  };

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

  TclDbf * ParentDbf() {
    return (TclDbf *)pParent;
  };

protected:

  xbFilter * filter;

private:

  virtual int Command (int objc, struct Tcl_Obj * CONST objv[]);
};

#endif
