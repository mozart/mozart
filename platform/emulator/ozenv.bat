@echo off

set OZHOME=y:\oz-devel

set PATH=%PATH%;%OZHOME%\platform\win32-i486;C:\WINDOWS;C:\WINDOWS\COMMAND;C:\BIN;C:\DOS;C:\MWW\DLL;e:\WATCOM\BINNT;e:\WATCOM\BINW

set OZPATH=.;%OZHOME%\lib;%OZHOME%\demo
rem set OZRC=ozrc

set TCL_LIBRARY=%OZHOME%\lib\wish\tcl
set TK_LIBRARY=%OZHOME%\lib\wish\tk
