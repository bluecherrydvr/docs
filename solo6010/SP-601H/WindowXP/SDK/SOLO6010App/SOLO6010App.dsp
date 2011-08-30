# Microsoft Developer Studio Project File - Name="SOLO6010App" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=SOLO6010App - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SOLO6010App.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SOLO6010App.mak" CFG="SOLO6010App - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SOLO6010App - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "SOLO6010App - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SOLO6010App - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x412 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x412 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 ddraw.lib dxguid.lib Winmm.lib ijl15.lib slmp4core.dll.a /nologo /subsystem:windows /machine:I386 /out:"../bin/SOLO6010App.exe" /libpath:"..\common\MPEG4" /libpath:"..\common\JPEG"

!ELSEIF  "$(CFG)" == "SOLO6010App - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x412 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x412 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ddraw.lib dxguid.lib Winmm.lib ijl15.lib slmp4core.dll.a /nologo /subsystem:windows /debug /machine:I386 /out:"../bin_debug/SOLO6010App.exe" /pdbtype:sept /libpath:"..\common\MPEG4" /libpath:"..\common\JPEG"

!ENDIF 

# Begin Target

# Name "SOLO6010App - Win32 Release"
# Name "SOLO6010App - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\DlgCtrlGPIO.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgCtrlI2C.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgCtrlLive.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgCtrlPlayer.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgCtrlRegister.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgCtrlUART.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgSettingGeneral.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgSettingMosaicVMotion.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgSettingRecord.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgTest.cpp
# End Source File
# Begin Source File

SOURCE=..\Common\G723\G723Coder.cpp
# End Source File
# Begin Source File

SOURCE=.\HWDecManager.cpp
# End Source File
# Begin Source File

SOURCE=.\IndexButton.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayerManager.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayerManager_G723.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayerManager_JPEG.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayerManager_MP4.cpp
# End Source File
# Begin Source File

SOURCE=.\RecManager.cpp
# End Source File
# Begin Source File

SOURCE=.\SOLO6010App.cpp
# End Source File
# Begin Source File

SOURCE=.\SOLO6010App.rc
# End Source File
# Begin Source File

SOURCE=.\SOLO6010AppDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# End Source File
# Begin Source File

SOURCE=.\WndLiveDisp.cpp
# End Source File
# Begin Source File

SOURCE=.\WndPlayer.cpp
# End Source File
# Begin Source File

SOURCE=.\Xdefine.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\DlgCtrlGPIO.h
# End Source File
# Begin Source File

SOURCE=.\DlgCtrlI2C.h
# End Source File
# Begin Source File

SOURCE=.\DlgCtrlLive.h
# End Source File
# Begin Source File

SOURCE=.\DlgCtrlPlayer.h
# End Source File
# Begin Source File

SOURCE=.\DlgCtrlRegister.h
# End Source File
# Begin Source File

SOURCE=.\DlgCtrlUART.h
# End Source File
# Begin Source File

SOURCE=.\DlgSettingGeneral.h
# End Source File
# Begin Source File

SOURCE=.\DlgSettingMosaicVMotion.h
# End Source File
# Begin Source File

SOURCE=.\DlgSettingRecord.h
# End Source File
# Begin Source File

SOURCE=.\DlgTest.h
# End Source File
# Begin Source File

SOURCE=..\Common\G723\G723Coder.h
# End Source File
# Begin Source File

SOURCE=.\HWDecManager.h
# End Source File
# Begin Source File

SOURCE=.\IndexButton.h
# End Source File
# Begin Source File

SOURCE=.\PlayerManager.h
# End Source File
# Begin Source File

SOURCE=.\RecManager.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\SOLO6010App.h
# End Source File
# Begin Source File

SOURCE=.\SOLO6010AppDlg.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\WndLiveDisp.h
# End Source File
# Begin Source File

SOURCE=.\WndPlayer.h
# End Source File
# Begin Source File

SOURCE=.\Xdefine.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\SOLO6010App.ico
# End Source File
# Begin Source File

SOURCE=.\res\SOLO6010App.rc2
# End Source File
# End Group
# Begin Group "SOLO6010"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Common\SOLO6010.cpp
# End Source File
# Begin Source File

SOURCE=..\Common\SOLO6010.h
# End Source File
# Begin Source File

SOURCE=..\Common\Solo6010_ExRecFileInfo.h
# End Source File
# Begin Source File

SOURCE=..\Common\Solo6010_Global.h
# End Source File
# Begin Source File

SOURCE=..\Common\Solo6010_InternalDef.h
# End Source File
# Begin Source File

SOURCE=..\Common\Solo6010_Ioctl.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
