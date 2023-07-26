// $Id$

#ifndef TCLINDEX_H
#define TCLINDEX_H

#include <xbase/xbase.h>
#include <xbase/index.h>
#include <tcl.h>

#include "tclcmd.hpp"
#include "tclxbase.hpp"
#include "tcldbf.hpp"

class TclIndex : public TclCmd {

public:

  TclIndex (Tcl_Interp * interp, char * name, TclDbf * tcldbf)
    : TclCmd(interp, name, tcldbf) {};

  virtual ~TclIndex() {};

  inline int CheckRC (int rc) {return ((TclDbf *)pParent)->CheckRC(rc);};

  xbXBase * XBase() {return ((TclDbf *)pParent)->XBase();};
  xbDbf * Dbf() {return ((TclDbf *)pParent)->Dbf();};
  xbIndex * Index() {return index;};

protected:

  xbIndex * index;

private:

  virtual int Command (int objc, struct Tcl_Obj * CONST objv[]);
  virtual int Type() = 0;
};

#endif
