# Microsoft Developer Studio Project File - Name="S6010StreamTester" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=S6010StreamTester - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "S6010StreamTester.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "S6010StreamTester.mak" CFG="S6010StreamTester - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "S6010StreamTester - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "S6010StreamTester - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "S6010StreamTester - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "colorspace" /I "..\SOLO6010App" /I "G723Sound" /I "..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x412 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x412 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 ddraw.lib dxguid.lib slmp4core.dll.a Winmm.lib LibDeIntFilter.lib /nologo /subsystem:windows /machine:I386 /out:"../bin/S6010StreamTester.exe" /libpath:"..\common\MPEG4"

!ELSEIF  "$(CFG)" == "S6010StreamTester - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "colorspace" /I "..\SOLO6010App" /I "G723Sound" /I "..\common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x412 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ddraw.lib dxguid.lib slmp4core.dll.a Winmm.lib LibDeIntFilterD.lib /nologo /subsystem:windows /debug /machine:I386 /out:"../bin_debug/S6010StreamTester.exe" /pdbtype:sept /libpath:"..\common\MPEG4"

!ENDIF 

# Begin Target

# Name "S6010StreamTester - Win32 Release"
# Name "S6010StreamTester - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\G723Sound\Buffer.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgAVISave.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgG723PlayCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgMainControl.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgPlayerBase.cpp
# End Source File
# Begin Source File

SOURCE=..\Common\LibMPEG4ToAVI\DooMPEG4ToAVI.cpp
# End Source File
# Begin Source File

SOURCE=.\G723Sound\errorprint.cpp
# End Source File
# Begin Source File

SOURCE=.\G723Sound\G723Coder.cpp
# End Source File
# Begin Source File

SOURCE=.\IndexButton.cpp
# End Source File
# Begin Source File

SOURCE=.\InfoS6010StreamIdx.cpp
# End Source File
# Begin Source File

SOURCE=.\S6010StreamTester.cpp
# End Source File
# Begin Source File

SOURCE=.\S6010StreamTester.rc
# End Source File
# Begin Source File

SOURCE=.\SnapshotManager.cpp
# End Source File
# Begin Source File

SOURCE=.\G723Sound\SoundBase.cpp
# End Source File
# Begin Source File

SOURCE=.\G723Sound\SoundIn.cpp
# End Source File
# Begin Source File

SOURCE=.\G723Sound\SoundOut.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# End Source File
# Begin Source File

SOURCE=.\WndS6010Disp.cpp
# End Source File
# Begin Source File

SOURCE=.\Xdefine.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\G723Sound\Buffer.h
# End Source File
# Begin Source File

SOURCE=.\DlgAVISave.h
# End Source File
# Begin Source File

SOURCE=.\DlgG723PlayCtrl.h
# End Source File
# Begin Source File

SOURCE=.\DlgMainControl.h
# End Source File
# Begin Source File

SOURCE=.\DlgPlayerBase.h
# End Source File
# Begin Source File

SOURCE=..\Common\LibMPEG4ToAVI\DooMPEG4ToAVI.h
# End Source File
# Begin Source File

SOURCE=.\G723Sound\errorprint.h
# End Source File
# Begin Source File

SOURCE=.\G723Sound\G723Coder.h
# End Source File
# Begin Source File

SOURCE=.\IndexButton.h
# End Source File
# Begin Source File

SOURCE=.\InfoS6010StreamIdx.h
# End Source File
# Begin Source File

SOURCE=.\LibDeIntFilter.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\S6010StreamTester.h
# End Source File
# Begin Source File

SOURCE=.\SnapshotManager.h
# End Source File
# Begin Source File

SOURCE=.\G723Sound\SoundBase.h
# End Source File
# Begin Source File

SOURCE=.\G723Sound\SoundIn.h
# End Source File
# Begin Source File

SOURCE=.\G723Sound\SoundOut.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\WndS6010Disp.h
# End Source File
# Begin Source File

SOURCE=.\Xdefine.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\S6010StreamTester.ico
# End Source File
# Begin Source File

SOURCE=.\res\S6010StreamTester.rc2
# End Source File
# End Group
# Begin Group "Colorspace"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Colorspace\colorspace.c

!IF  "$(CFG)" == "S6010StreamTester - Win32 Release"

!ELSEIF  "$(CFG)" == "S6010StreamTester - Win32 Debug"

# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Colorspace\colorspace.h
# End Source File
# Begin Source File

SOURCE=.\Colorspace\colorspace_mmx.inc

!IF  "$(CFG)" == "S6010StreamTester - Win32 Release"

# Begin Custom Build
InputDir=.\Colorspace
IntDir=.\Release
InputPath=.\Colorspace\colorspace_mmx.inc
InputName=colorspace_mmx

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasm -f win32 -DPREFIX -I"$(InputDir)"\ -o $(IntDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "S6010StreamTester - Win32 Debug"

# Begin Custom Build
InputDir=.\Colorspace
IntDir=.\Debug
InputPath=.\Colorspace\colorspace_mmx.inc
InputName=colorspace_mmx

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasm -f win32 -DPREFIX -I"$(InputDir)"\ -o $(IntDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Colorspace\colorspace_rgb_mmx.asm

!IF  "$(CFG)" == "S6010StreamTester - Win32 Release"

# Begin Custom Build
InputDir=.\Colorspace
IntDir=.\Release
InputPath=.\Colorspace\colorspace_rgb_mmx.asm
InputName=colorspace_rgb_mmx

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasm -f win32 -DPREFIX -I"$(InputDir)"\ -o $(IntDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "S6010StreamTester - Win32 Debug"

# Begin Custom Build
InputDir=.\Colorspace
IntDir=.\Debug
InputPath=.\Colorspace\colorspace_rgb_mmx.asm
InputName=colorspace_rgb_mmx

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasm -f win32 -DPREFIX -I"$(InputDir)"\ -o $(IntDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Colorspace\colorspace_yuv_mmx.asm

!IF  "$(CFG)" == "S6010StreamTester - Win32 Release"

# Begin Custom Build
InputDir=.\Colorspace
IntDir=.\Release
InputPath=.\Colorspace\colorspace_yuv_mmx.asm
InputName=colorspace_yuv_mmx

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasm -f win32 -DPREFIX -I"$(InputDir)"\ -o $(IntDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "S6010StreamTester - Win32 Debug"

# Begin Custom Build
InputDir=.\Colorspace
IntDir=.\Debug
InputPath=.\Colorspace\colorspace_yuv_mmx.asm
InputName=colorspace_yuv_mmx

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasm -f win32 -DPREFIX -I"$(InputDir)"\ -o $(IntDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Colorspace\colorspace_yuyv_mmx.asm

!IF  "$(CFG)" == "S6010StreamTester - Win32 Release"

# Begin Custom Build
InputDir=.\Colorspace
IntDir=.\Release
InputPath=.\Colorspace\colorspace_yuyv_mmx.asm
InputName=colorspace_yuyv_mmx

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasm -f win32 -DPREFIX -I"$(InputDir)"\ -o $(IntDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "S6010StreamTester - Win32 Debug"

# Begin Custom Build
InputDir=.\Colorspace
IntDir=.\Debug
InputPath=.\Colorspace\colorspace_yuyv_mmx.asm
InputName=colorspace_yuyv_mmx

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasm -f win32 -DPREFIX -I"$(InputDir)"\ -o $(IntDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
