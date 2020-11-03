@echo off
call "D:\Program Files\Microsoft Visual Studio\VC98\Bin\VCVARS32.BAT"
nmake -nologo -f makefile-hello.vc DEBUG=1 %1


