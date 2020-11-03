PACKAGE = xbasetcl
VERSION = 1.0

#XBASE = /opt/xbase-2.0.0
XBASE = /opt/xbase-2.1
XBASE_CFLAGS = -I$(XBASE)/include
XBASE_LDFLAGS = -L$(XBASE)/lib -lxbase
#XBASE_LDFLAGS = -$(XBASE)/lib/libxbase.a

#XBASE_SRC = ../xbase-2.1.0-cvs20020509
#XBASE_CFLAGS = -I$(XBASE_SRC)
#XBASE_LDFLAGS = -L$(XBASE_SRC)/xbase/.libs -lxbase

TCL = /opt/tcltk
TCL_VERSION = 8.3
TCL_CFLAGS = -I$(TCL)/include -DUSE_TCL_STUBS=1
TCL_LDFLAGS = -L$(TCL)/lib -ltclstub$(TCL_VERSION)
TCLSH = $(TCL)/bin/tclsh$(TCL_VERSION)

#TCL = /opt/tclpro
#TCL_VERSION = 8.3
#TCL_CFLAGS = -I$(TCL)/include -DUSE_TCL_STUBS=1
#TCL_LDFLAGS = -L$(TCL)/linux-ix86/lib -ltclstub$(TCL_VERSION)
#TCLSH = $(TCL)/linux-ix86/bin/tclsh$(TCL_VERSION)

#TCL = /usr
#TCL_VERSION = 8.0
#TCL_CFLAGS = -I$(TCL)/include
#TCL_LDFLAGS = -L$(TCL)/lib -ltcl$(TCL_VERSION)

DEFINES = -DTCL_MEM_DEBUG -D_DEBUG -DDEBUG
DEFINES = $(DEFINES) -DTCLCMD_DEBUG
DEFINES = $(DEFINES) -DTCLXBASE_DEBUG
DEFINES = $(DEFINES) -DTCLDBF_DEBUG

CPPFLAGS = -g -Wall -fPIC \
    $(XBASE_CFLAGS) $(TCL_CFLAGS) $(CPPTCL_CFLAGS) $(DEFINES)
LDFLAGS = -g -lstdc++ \
    $(XBASE_LDFLAGS) $(TCL_LDFLAGS) $(CPPTCL_LDFLAGS)

LIB = ./$(PACKAGE)$(VERSION).so

OBJS = xbasetcl.o tclcmd.o tclxbase.o tcldbf.o tclindex.o tclfilter.o

# rules

all: $(LIB)

clean:
	-rm $(OBJS) $(LIB)

test: $(LIB)
	env LD_LIBRARY_PATH=$(XBASE)/lib \
		XBASEDLL=$(LIB) $(TCLSH) \
		xbasetcl.test -verbose bps

debug: $(LIB)
	 env  LD_LIBRARY_PATH=$(XBASE)/lib \
		XBASEDLL=$(LIB) gdb $(TCLSH)

$(LIB): $(OBJS)
	cc -shared $(LDFLAGS) -o $(LIB) $(OBJS)
