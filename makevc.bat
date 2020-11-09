@echo off

rem call C:\Program Files\Microsoft Visual Studio 14.0\VC\bin\vcvars32.bat

echo SDK:
echo %WindowsSdkDir%
if not exist "%WindowsSdkDir%\lib\kernel32.lib" goto sdkerror

rem fix tailing slash
if "%WindowsSdkDir:~-1%" == "\" set WindowsSdkDir=%WindowsSdkDir:~0,-1%

echo Compiler:
cl > nul
if errorlevel 1 goto clerror

if "%TCLROOT%" == ""      set TCLROOT=C:\ActiveTcl
if "%TCLVERSION%" == ""   set TCLVERSION=86
if "%XBASESRC%" == ""     set XBASESRC=..\xbase
if "%XBASEVERSION%" == "" set XBASEVERSION=21
if "%XBASESTATIC%" == ""  set XBASESTATIC=1

rem defaults: DEBUG=0 STATIC=1 MSVCRT=1
if not "%DEBUG%" == ""    set OPTS=%OPTS% DEBUG=%DEBUG%
if not "%MSVCRT%" == ""   set OPTS=%OPTS% MSVCRT=%MSVCRT%
if not "%STATIC%" == ""   set OPTS=%OPTS% STATIC_BUILD=%STATIC%

nmake -nologo -f makefile.vc USE_TCL_STUBS=1 %OPTS% CFLAGS="%CFLAGS%" SDKINSTALLDIR="%WindowsSdkDir%" XBASESRC="%XBASESRC%" TCLROOT="%TCLROOT%" TCLVERSION="%TCLVERSION%" %*

if not errorlevel 1 goto ok
echo ERROR: MAKE RRROR
goto end

:sdkerror
echo ERROR: SDK NOT FOUND
goto end

:clerror
echo ERROR: COMPILER NOT FOUND
goto end

:ok
echo OK

:end
