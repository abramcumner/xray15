@echo off
REM
REM Copyright (c) 2005-2017 Intel Corporation
REM
REM Licensed under the Apache License, Version 2.0 (the "License");
REM you may not use this file except in compliance with the License.
REM You may obtain a copy of the License at
REM
REM     http://www.apache.org/licenses/LICENSE-2.0
REM
REM Unless required by applicable law or agreed to in writing, software
REM distributed under the License is distributed on an "AS IS" BASIS,
REM WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
REM See the License for the specific language governing permissions and
REM limitations under the License.
REM
REM
REM
REM
REM

set SCRIPT_NAME=%~nx0
set TBB_BIN_DIR=%~d0%~p0
set TBBROOT=%TBB_BIN_DIR%..

:: Set the default arguments
set TBB_TARGET_ARCH=
set TBB_TARGET_VS=

:ParseArgs
:: Parse the incoming arguments
if /i "%1"==""        goto SetEnv
if /i "%1"=="ia32"         (set TBB_TARGET_ARCH=ia32)     & shift & goto ParseArgs
if /i "%1"=="intel64"      (set TBB_TARGET_ARCH=intel64)  & shift & goto ParseArgs
if /i "%1"=="vs2013"       (set TBB_TARGET_VS=vc12)       & shift & goto ParseArgs
if /i "%1"=="vs2015"       (set TBB_TARGET_VS=vc14)       & shift & goto ParseArgs
if /i "%1"=="vs2017"       (set TBB_TARGET_VS=vc14)       & shift & goto ParseArgs
if /i "%1"=="all"          (set TBB_TARGET_VS=vc_mt)      & shift & goto ParseArgs
:: for any other incoming arguments values
goto Syntax

:SetEnv
:: target architecture is a required argument
if "%TBB_TARGET_ARCH%"=="" goto Syntax
:: TBB_TARGET_VS default value is 'vc_mt'
if "%TBB_TARGET_VS%"=="" set TBB_TARGET_VS=vc_mt

if exist "%TBBROOT%\bin\%TBB_TARGET_ARCH%\%TBB_TARGET_VS%\tbb.dll" (
    set "PATH=%TBBROOT%\bin\%TBB_TARGET_ARCH%\%TBB_TARGET_VS%;%PATH%"
)
if exist "%TBBROOT%\..\redist\%TBB_TARGET_ARCH%\tbb\%TBB_TARGET_VS%\tbb.dll" (
    set "PATH=%TBBROOT%\..\redist\%TBB_TARGET_ARCH%\tbb\%TBB_TARGET_VS%;%PATH%"
)

set "LIB=%TBBROOT%\lib\%TBB_TARGET_ARCH%\%TBB_TARGET_VS%;%LIB%"
set "INCLUDE=%TBBROOT%\include;%INCLUDE%"
set "CPATH=%TBBROOT%\include;%CPATH%"

if exist "%TBBROOT%\..\..\linux\tbb\lib\intel64\gcc4.7\libtbb.so.2" (
    set "LIBRARY_PATH=%TBBROOT%\..\..\linux\tbb\lib\intel64\gcc4.7;%LIBRARY_PATH%"
    set "LD_LIBRARY_PATH=%TBBROOT%\..\..\linux\tbb\lib\intel64\gcc4.7;%LD_LIBRARY_PATH%"
)

if not "%ICPP_COMPILER16%"=="" set TBB_CXX=icl.exe
if not "%ICPP_COMPILER17%"=="" set TBB_CXX=icl.exe
if not "%ICPP_COMPILER18%"=="" set TBB_CXX=icl.exe
goto End

:Syntax
echo Syntax:
echo  %SCRIPT_NAME% ^<arch^> ^<vs^>
echo    ^<arch^> must be one of the following
echo        ia32         : Set up for IA-32  architecture
echo        intel64      : Set up for Intel(R) 64  architecture
echo    ^<vs^> should be one of the following
echo        vs2013      : Set to use with Microsoft Visual Studio 2013 runtime DLLs
echo        vs2015      : Set to use with Microsoft Visual Studio 2015 runtime DLLs
echo        vs2017      : Set to use with Microsoft Visual Studio 2017 runtime DLLs
echo        all         : Set to use TBB statically linked with Microsoft Visual C++ runtime
echo    if ^<vs^> is not set TBB statically linked with Microsoft Visual C++ runtime will be used.
exit /B 1

:End
exit /B 0
