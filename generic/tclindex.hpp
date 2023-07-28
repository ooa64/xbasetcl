// $Id$

#ifndef TCLINDEX_H
#define TCLINDEX_H

#include "tclcmd.hpp"
#include "tcldbf.hpp"

class TclIndex : public TclCmd {

public:

  TclIndex(Tcl_Interp * interp, char * name, TclDbf * tcldbf)
    : TclCmd(interp, name, tcldbf), index(NULL) {};

  virtual ~TclIndex() {};

  inline int CheckRC (int rc) {return ((TclDbf *)pParent)->CheckRC(rc);};

  xbXBase * XBase() {return ((TclDbf *)pParent)->XBase();};
  xbDbf * Dbf() {return ((TclDbf *)pParent)->Dbf();};
  xbIx * Index() {return index;};

protected:

  xbIx * index;

private:

  virtual int Command (int objc, struct Tcl_Obj * CONST objv[]);
  virtual int Type() = 0;
};

#endif
