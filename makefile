PACKAGE = xbasetcl
VERSION = 1.0

#XBASE = /opt/xbase-2.1
#XBASE_CFLAGS = -I$(XBASE)/include
#XBASE_LIBS = -L$(XBASE)/lib $(XBASE)/lib/libxbase.so

XBASE_SRC = ../xbase
XBASE_CFLAGS = -I$(XBASE_SRC)
XBASE_LIBS = -L$(XBASE_SRC)/xbase $(XBASE_SRC)/xbase/.libs/libxbase.so
#XBASE_LIBS = -L$(XBASE_SRC)/xbase/.libs -lxbase

#TCL = /usr
#TCL_VERSION = 8.6
#TCL_CFLAGS = -I$(TCL)/include -I$(TCL)/include/tcl -DUSE_TCL_STUBS=1
#TCL_LIBS = -L$(TCL)/lib -ltcl$(TCL_VERSION) -ltclstub$(TCL_VERSION)
#TCLSH = $(TCL)/bin/tclsh$(TCL_VERSION)

TCL_SRC = ../tcl
TCL_VERSION = 8.6
TCL_CFLAGS = -I$(TCL_SRC)/generic -DUSE_TCL_STUBS=1
TCL_LIBS = -L$(TCL_SRC)/unix -ltcl$(TCL_VERSION) $(TCL_SRC)/unix/libtclstub$(TCL_VERSION).a
TCLSH = $(TCL_SRC)/unix/tclsh

DEFINES += -DTCL_MEM_DEBUG -D_DEBUG -DDEBUG
DEFINES += -DTCLCMD_DEBUG
DEFINES += -DTCLXBASE_DEBUG
DEFINES += -DTCLDBF_DEBUG
DEFINES += -DTCLINDEX_DEBUG
DEFINES += -DTCLFILTER_DEBUG

CPPFLAGS = -g -Wall -fPIC $(XBASE_CFLAGS) $(TCL_CFLAGS) $(DEFINES)
LDFLAGS = -g -shared -fPIC -Wl,--export-dynamic 

LIB = ./$(PACKAGE)$(VERSION).so

OBJS = xbasetcl.o tclcmd.o tclxbase.o tcldbf.o tclindex.o tclfilter.o

# rules

all: $(LIB)

clean:
	-rm $(OBJS) $(LIB)

test: $(LIB)
	env LD_LIBRARY_PATH=$(XBASE)/lib:$(XBASE_SRC)/xbase/.libs \
		XBASETCLDLL=$(LIB) $(TCLSH) \
		xbasetcl.test -verbose bpse

debug: $(LIB)
	 env LD_LIBRARY_PATH=$(XBASE)/lib:$(XBASE_SRC)/xbase/.libs \
		XBASETCLDLL=$(LIB) gdb $(TCLSH)

$(LIB): $(OBJS)
	g++ $(LDFLAGS) -o $(LIB) $(OBJS) -lstdc++ $(XBASE_LIBS) $(TCL_LIBS)


