# Microsoft Developer Studio Project File - Name="GoClient" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=GoClient - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GoClient.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GoClient.mak" CFG="GoClient - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GoClient - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "GoClient - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Perforce Project"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GoClient - Win32 Release"

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
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../../../../WISInclude/WISInterface" /I "../../../../WISInclude/DLLInterface" /I "../../../../WISInclude" /I "../../../../Tools/DXSDK81/Include" /I "..\..\..\Tools\VSTUDIO\VC98\Include" /I "..\..\..\Tools\VSTUDIO\VC98\MFC\Include" /I "..\..\..\Tools\VSTUDIO\VC98\ATL\Include" /I "../live/UsageEnvironment/include" /I "../live/BasicUsageEnvironment/include" /I "../live/liveMedia/include" /I "../live/groupsock/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 SDK7007.lib MultiMedia.lib Utilities.lib Winmm.lib libgroupsock.lib libUsageEnvironment.lib libBasicUsageEnvironment.lib libliveMedia.lib /nologo /subsystem:windows /machine:I386 /libpath:"../../../../WISInclude/WISInterface/WISLIB" /libpath:"../../../../WISInclude/DLLInterface/WISDLL" /libpath:"../live/groupsock" /libpath:"../live/UsageEnvironment" /libpath:"../live/BasicUsageEnvironment" /libpath:"../live/liveMedia"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "GoClient - Win32 Debug"

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
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../../../WISInclude/WISInterface" /I "../../../../WISInclude/DLLInterface" /I "../../../../WISInclude" /I "../../../../Tools/DXSDK81/Include" /I "..\..\..\..\Tools\VSTUDIO\VC98\Include" /I "..\..\..\..\Tools\VSTUDIO\VC98\MFC\Include" /I "..\..\..\..\Tools\VSTUDIO\VC98\ATL\Include" /I "../live/UsageEnvironment/include" /I "../live/BasicUsageEnvironment/include" /I "../live/liveMedia/include" /I "../live/groupsock/include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /Yu"stdafx.h" /FD /GZ /c
# SUBTRACT CPP /X
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 SDK7007d.lib MultiMediad.lib Utilitiesd.lib Winmm.lib libgroupsock.lib libUsageEnvironment.lib libBasicUsageEnvironment.lib libliveMedia.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"../../../../WISInclude/WISInterface/WISLIB" /libpath:"../../../../WISInclude/DLLInterface/WISDLL" /libpath:"../live/groupsock" /libpath:"../live/UsageEnvironment" /libpath:"../live/BasicUsageEnvironment" /libpath:"../live/liveMedia"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "GoClient - Win32 Release"
# Name "GoClient - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Channel.cpp
# End Source File
# Begin Source File

SOURCE=.\GoClient.cpp
# End Source File
# Begin Source File

SOURCE=.\GoClient.rc
# End Source File
# Begin Source File

SOURCE=.\GoClientDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MulticastAddrDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MulticastChannel.cpp
# End Source File
# Begin Source File

SOURCE=.\PtsClock.cpp
# End Source File
# Begin Source File

SOURCE=.\SoundDevice.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\UnicastChannel.cpp
# End Source File
# Begin Source File

SOURCE=.\VideoRender.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Channel.h
# End Source File
# Begin Source File

SOURCE=.\GoClient.h
# End Source File
# Begin Source File

SOURCE=.\GoClientDlg.h
# End Source File
# Begin Source File

SOURCE=.\MulticastAddrDlg.h
# End Source File
# Begin Source File

SOURCE=.\MulticastChannel.h
# End Source File
# Begin Source File

SOURCE=.\PtsClock.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\SoundDevice.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\UnicastChannel.h
# End Source File
# Begin Source File

SOURCE=.\VideoRender.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\GoClient.ico
# End Source File
# Begin Source File

SOURCE=.\res\GoClient.rc2
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
