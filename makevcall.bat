mkdir Tcl83
del /q Tcl83\*.*
call makevc clean
call makevc "TCLROOT=d:\tcl" all > Tcl83\build.lst
call makevc "TCLROOT=d:\tcl" test > Tcl83\test.lst
move xbasetcl10.* Tcl83\
mkdir Tcl80
del /q Tcl80\*.*
call makevc "TCLROOT=d:\tcl80" "TCLVERSION=80" clean all > Tcl80\build.lst
call makevc "TCLROOT=d:\tcl80" "TCLVERSION=80" test > Tcl80\test.lst
move xbasetcl10.* Tcl80\
:call makevc clean
