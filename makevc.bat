@echo off
call "D:\Program Files\Microsoft Visual Studio\VC98\Bin\VCVARS32.BAT"

nmake -nologo -f makefile.vc %1 %2 %3 %4 %5 %6 %7 %8 %9
