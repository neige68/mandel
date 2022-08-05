@rem <build.bat> -*- coding: cp932-dos -*- - build script
@echo off
rem
rem Project try-mandel
rem Copyright (C) 2022 neige68
rem https://github.com/neige68/mandel
rem

setlocal
pushd %~dp0
set @exec_cmake=

rem VC の vcvarsall.bat のあるディレクトリを指定
set VC=%VC142%
rem set VC=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build

:optloop
if "%1"=="" goto optend
if "%1"=="cm" set @exec_cmake=t
if "%1"=="cm" goto optnext
echo WARN: build.bat: オプション %1 が無効です.
:optnext
shift
goto optloop
:optend

if "%VCToolsVersion%"=="" call "%VC%\vcvarsall.bat" x86

rem ------------------------------------------------------------
rem main
rem ------------------------------------------------------------
rem build
set @build=..\build
set @cmake_opt=-G "Visual Studio 16 2019" -A Win32 
call :common
if errorlevel 1 goto pop1_end

rem ------------------------------------------------------------
rem build64
set @build=..\build64
set @cmake_opt=-G "Visual Studio 16 2019" -A x64
call :common
if errorlevel 1 goto pop1_end

rem ------------------------------------------------------------
rem common
rem ------------------------------------------------------------
:common
if not exist %@build% mkdir %@build%
pushd %@build%
if not exist ALL_BUILD.vcxproj set @exec_cmake=t
if not "%@exec_cmake%"=="" cmake %@cmake_opt% ../src
msbuild ALL_BUILD.vcxproj /p:Configuration=Debug /m
echo INFO: build.bat: msbuild Debug Done.
if errorlevel 1 goto common_end
msbuild ALL_BUILD.vcxproj /p:Configuration=Release /m
echo INFO: build.bat: msbuild Release Done.
:common_end
popd
goto :EOF

rem ------------------------------------------------------------
rem finish
rem ------------------------------------------------------------
:pop1_end
popd
if errorlevel 1 echo エラーがあります.
if errorlevel 1 exit /b 1
rem end of build.bat
