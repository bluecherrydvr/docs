@echo off
@call "C:\Program Files\Microsoft Visual Studio 8\VC\vcvarsall.bat" x86

@if "%1" == "msvcexpress" goto START_BUILD
@if "%1" == "msvc" goto START_BUILD

@ cls
@echo Usage:
@echo    dvr_host compiler_ver [dvrcp]
@
@echo       compiler_ver: msvcexpress for MS Visual Studio Express edition
@echo                     msvc for MS Visual Studio standard or professional edition
@echo       dvrcp: [optional] To build dvrcp in addition to SDK and driver
@echo              Qt 4.2.2 is need to be installed in order to build dvrcp
rem ============================================================
goto END_BUILD
@rem ============================================================

:START_BUILD
rem Create some local variables
if "%1" == "msvcexpress" (
  	SET MSVC_BUILD_CMD=VCExpress.exe
) ELSE (
  	SET MSVC_BUILD_CMD=devenv.exe
)

rem ============================================================
rem create bin directories it is needed for the drive 

if not exist bin mkdir bin
if not exist bin\windows mkdir bin\windows
if not exist bin\windows\Debug mkdir bin\windows\Debug
if not exist bin\windows\Release mkdir bin\windows\Release

rem ============================================================

%MSVC_BUILD_CMD% dvr_host.sln /clean Release
%MSVC_BUILD_CMD% dvr_host.sln /clean Debug

@echo Building Release version of host libraries...
%MSVC_BUILD_CMD% dvr_host.sln /Rebuild Release

IF "%2" == "dvrcp" (
	@echo QT_ROOT is %QT_ROOT%
	call %QT_ROOT%\bin\qtvars.bat vsvars

	if not exist bin\redistributables mkdir bin\redistributables
        copy %QT_ROOT%\bin\QtCore4.dll bin\redistributables
        copy %QT_ROOT%\bin\QtGui4.dll bin\redistributables
        copy %QT_ROOT%\bin\QtOpenGL4.dll bin\redistributables
        copy %QT_ROOT%\bin\QtXml4.dll bin\redistributables
	
	cd apps\dvrcp
	@echo Running QMake...
	qmake dvrcp.pro
	cd ..\..

	@echo Building release version of dvrcp...
	%MSVC_BUILD_CMD% dvrcp.sln /build Release

	rem MSVC Express does not support setup skip it.
	if "%1" == "msvcexpress" goto BLD_HOST_DEBUG
	
	rem Build the installer if there was a successful build
	if not exist bin\windows\Release\dvrcp.exe goto BLD_HOST_DEBUG
	@echo Building the dvrcp installer ...
	%MSVC_BUILD_CMD% sdvrdemo_setup.sln /Rebuild Release
	
) 

:BLD_HOST_DEBUG
@echo Building debug version of host libraries...
%MSVC_BUILD_CMD% dvr_host.sln /Rebuild Debug

copy src\svc\stRtpIO\pthread\win32\pthreadVC2.dll bin\windows\Debug\pthreadVC2.dll
copy src\svc\stRtpIO\pthread\win32\pthreadVC2.dll bin\windows\Release\pthreadVC2.dll


rem ============================================================
rem echo "Save all the log files"
if exist apps\dvrcp\debug\BuildLog.htm  copy apps\dvrcp\debug\BuildLog.htm bin\DvrcpDebugLog.htm
if exist apps\dvrcp\release\BuildLog.htm  copy apps\dvrcp\release\BuildLog.htm bin\DvrcpReleaseLog.htm

if exist src\driver\windows\Driver\Release\BuildLog.htm copy src\driver\windows\Driver\Release\BuildLog.htm bin\DriverReleaseLog.htm
if exist src\driver\windows\Driver\Debug\BuildLog.htm	copy src\driver\windows\Driver\Debug\BuildLog.htm bin\DriverDebugLog.htm
if exist src\driver\windows\SCT\Release\BuildLog.htm	copy src\driver\windows\SCT\Release\BuildLog.htm bin\SctReleaseLog.htm
if exist src\driver\windows\SCT\Debug\BuildLog.htm	copy src\driver\windows\SCT\Debug\BuildLog.htm bin\SctDebugLog.htm
if exist src\sdk\Release\BuildLog.htm			copy src\sdk\Release\BuildLog.htm bin\SdkReleaseLog.htm
if exist src\sdk\Debug\BuildLog.htm			copy src\sdk\Debug\BuildLog.htm bin\SdkDebugLog.htm

if exist apps\StreamPlayer\StreamPlayer\Release\BuildLog.htm  copy apps\StreamPlayer\StreamPlayer\Release\BuildLog.htm bin\StreamPlayerReleaseLog.htm
if exist apps\StreamPlayer\StreamPlayer\Debug\BuildLog.htm  copy apps\StreamPlayer\StreamPlayer\Debug\BuildLog.htm bin\StreamPlayerDebugLog.htm
if exist apps\svcRouter\Release\BuildLog.htm  copy apps\svcRouter\Release\BuildLog.htm bin\svcRouterReleaseLog.htm
if exist apps\svcRouter\Debug\BuildLog.htm  copy apps\svcRouter\Debug\BuildLog.htm bin\svcRouterDebugLog.htm
if exist src\svc\stRtpIO\Release\BuildLog.htm  copy src\svc\stRtpIO\Release\BuildLog.htm bin\stRtpIOReleaseLog.htm
if exist src\svc\stRtpIO\Debug\BuildLog.htm  copy src\svc\stRtpIO\Release\BuildLog.htm bin\stRtpIODebugLog.htm
if exist src\svc\stRtpIO\live\Release\BuildLog.htm  copy src\svc\stRtpIO\Release\BuildLog.htm bin\liveReleaseLog.htm
if exist src\svc\stRtpIO\live\Debug\BuildLog.htm  copy src\svc\stRtpIO\live\Debug\BuildLog.htm bin\liveDebugLog.htm

rem echo "Clean up all the folders"
if exist apps\dvrcp\debug  rmdir /S /Q apps\dvrcp\debug
if exist apps\dvrcp\release  rmdir /S /Q apps\dvrcp\release
if exist apps\sdvr_diag\Debug rmdir /S /Q apps\sdvr_diag\Debug
if exist apps\sdvr_diag\release rmdir /S /Q apps\sdvr_diag\release

if exist apps\StreamPlayer\StreamPlayer\Release  rmdir /S /Q apps\StreamPlayer\StreamPlayer\Release
if exist apps\StreamPlayer\StreamPlayer\Debug  rmdir /S /Q apps\StreamPlayer\StreamPlayer\Debug
if exist apps\StreamPlayer\Release  rmdir /S /Q apps\StreamPlayer\Release
if exist apps\StreamPlayer\Debug  rmdir /S /Q apps\StreamPlayer\Debug
if exist apps\svcRouter\Release  rmdir /S /Q apps\svcRouter\Release
if exist apps\svcRouter\Debug  rmdir /S /Q apps\svcRouter\Debug
if exist src\svc\stRtpIO\Release  rmdir /S /Q src\svc\stRtpIO\Release
if exist src\svc\stRtpIO\Debug  rmdir /S /Q src\svc\stRtpIO\Debug
if exist src\svc\stRtpIO\live\Release  rmdir /S /Q src\svc\stRtpIO\live\Release
if exist src\svc\stRtpIO\live\Debug  rmdir /S /Q src\svc\stRtpIO\live\Debug

if exist src\driver\windows\Driver\Release rmdir /S /Q src\driver\windows\Driver\Release
if exist src\driver\windows\Driver\Debug rmdir /S /Q src\driver\windows\Driver\Debug
if exist src\driver\windows\Driver\obj rmdir /S /Q src\driver\windows\Driver\obj
if exist src\driver\windows\Driver\objfre_wlh_x86 rmdir /S /Q src\driver\windows\Driver\objfre_wlh_x86
if exist src\driver\windows\Driver\objchk_wlh_x86 rmdir /S /Q src\driver\windows\Driver\objchk_wlh_x86
if exist src\driver\windows\Driver\bin rmdir /S /Q src\driver\windows\Driver\bin

if exist src\driver\windows\Driver\buildfre_wlh_x86.log del src\driver\windows\Driver\buildfre_wlh_x86.log
if exist src\driver\windows\Driver\buildchk_wlh_x86.log del src\driver\windows\Driver\buildchk_wlh_x86.log

if exist src\sdk\Release rmdir /S /Q src\sdk\Release
if exist src\sdk\Debug rmdir /S /Q src\sdk\Debug

if exist src\driver\windows\SCT\Release rmdir /S /Q src\driver\windows\SCT\Release
if exist src\driver\windows\SCT\Debug rmdir /S /Q src\driver\windows\SCT\Debug

if exist src\driver\windows\sctapp\Release rmdir /S /Q src\driver\windows\sctapp\Release
if exist src\driver\windows\sctapp\Debug rmdir /S /Q src\driver\windows\sctapp\Debug

if exist bin\redistributables rmdir /S /Q  bin\redistributables
if exist bin\dvrcp_installer  rmdir /S /Q  bin\dvrcp_installer

rem ============================================================
rem delete all the temp variables
SET MSVC_BUILD_CMD=

:END_BUILD

