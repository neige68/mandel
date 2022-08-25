@rem <exec.bat> -*- coding: cp932-dos -*- - exec script
@echo off
rem
rem Project GoldenMandel
rem Copyright (C) 2022 neige68
rem https://github.com/neige68/mandel
rem This program is released under license GPLv3
rem

setlocal
pushd %~dp0
set @config=debug
set @name=GoldenMandel
set @verbose=
set @build=..\build
:loop
if "%1"=="--" goto optbreak
if "%1"=="r" goto rel
if "%1"=="-r" goto rel
if "%1"=="rel" goto rel
if "%1"=="d" goto deb
if "%1"=="-d" goto deb
if "%1"=="deb" goto deb
if "%1"=="6" goto x64
if "%1"=="64" goto x64
if "%1"=="x64" goto x64
if "%1"=="v" goto verbose
if "%1"=="-v" goto verbose
if "%1"=="h" goto help
if "%1"=="-h" goto help
if "%1"=="--help" goto help
goto optend
:rel
set @config=release
shift
goto loop
:deb
set @config=debug
shift
goto loop
:x64
set @build=..\build64
shift
goto loop
:verbose
set @verbose=t
shift
goto loop
:optbreak
shift
:optend
if not "%1"=="" set @name=%1
if "%@name%"=="" goto help
set @exe=%@build%\%@config%\%@name%.exe
if not "%@verbose%"=="" echo INFO: %@exe%
shift
shift
%@exe% %0 %1 %2 %3 %4 %5 %6 %7 %8 %9
goto end
:help
echo usage: exec [v] [deb/rel] [x64] [--] name
:end
popd
rem end of exec.bat
