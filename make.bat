@echo off
call "D:\Program Files\Microsoft Visual Studio\VC98\Bin\VCVARS32.BAT"

:goto static
goto staticdebug
:goto dynamic
:goto dynamicdebug
:static
nmake -f Makefile-mnc.vc MNCDIR=..\_build STATIC_BUILD=1 DEBUG=0 %1 %2 %3 %4 %5 %6 %7 %8 %9
exit
if errorlevel 1 goto end
:staticdebug
nmake -f Makefile-mnc.vc MNCDIR=..\_build STATIC_BUILD=1 DEBUG=1 %1 %2 %3 %4 %5 %6 %7 %8 %9
exit
:dynamic
if errorlevel 1 goto end
nmake -f Makefile-mnc.vc MNCDIR=..\_build STATIC_BUILD=0 DEBUG=0 %1 %2 %3 %4 %5 %6 %7 %8 %9
exit
if errorlevel 1 goto end
:dynamicdebug
nmake -f Makefile-mnc.vc MNCDIR=..\_build STATIC_BUILD=0 DEBUG=1 %1 %2 %3 %4 %5 %6 %7 %8 %9
exit
if errorlevel 1 goto end
rem echo OK
:end
