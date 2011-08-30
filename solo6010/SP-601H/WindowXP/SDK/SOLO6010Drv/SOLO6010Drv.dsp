# Microsoft Developer Studio Project File - Name="SOLO6010Drv" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=SOLO6010Drv - Win32 Checked
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SOLO6010Drv.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SOLO6010Drv.mak" CFG="SOLO6010Drv - Win32 Checked"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SOLO6010Drv - Win32 Free" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "SOLO6010Drv - Win32 Checked" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SOLO6010Drv - Win32 Free"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "SOLO6010Drv___Win32_Free"
# PROP BASE Intermediate_Dir "SOLO6010Drv___Win32_Free"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Free"
# PROP Intermediate_Dir "Free"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Gz /Zp4 /W3 /Z7 /O2 /I "C:\NTDDK\inc" /I "C:\NTDDK\inc\ddk" /D "STD_CALL" /D _X86_=1 /D i386=1 /D CONDITION_HANDLING=1 /D WIN32_LEAN_AND_MEAN=1 /D NT_INST=0 /D WIN32=100 /D _NT1X_=100 /D NT_UP=1 /D try=__try /D leave=__leave /D except=__except /D finally=__finally /D _CRTAPI1=__cdecl /D _CRTAPI2=__cdecl /D itoa=_itoa /D strcmpi=_strcmpi /D stricmp=_stricmp /D wcsicmp=_wcsicmp /D wcsnicmp=_wcsnicmp /D DEVL=1 /D "_IDWBUILD" /D "__KERNEL__" /D "WINNT" /D "PCI_CODE" /D "LITTLE_ENDIAN" /D "WDM_DRIVER" /FR -Oxs /Zel /c
# ADD CPP /nologo /Gz /Zp4 /W3 /Z7 /O2 /I "C:\NTDDK\inc" /I "C:\NTDDK\inc\ddk" /I "..\Common" /D "STD_CALL" /D _X86_=1 /D i386=1 /D CONDITION_HANDLING=1 /D WIN32_LEAN_AND_MEAN=1 /D NT_INST=0 /D WIN32=100 /D _NT1X_=100 /D NT_UP=1 /D try=__try /D leave=__leave /D except=__except /D finally=__finally /D _CRTAPI1=__cdecl /D _CRTAPI2=__cdecl /D itoa=_itoa /D strcmpi=_strcmpi /D stricmp=_stricmp /D wcsicmp=_wcsicmp /D wcsnicmp=_wcsnicmp /D DEVL=1 /D "_IDWBUILD" /D "__KERNEL__" /D "WINNT" /D "PCI_CODE" /D "LITTLE_ENDIAN" /D "WDM_DRIVER" /FR -Oxs /Zel /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL/win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL/win32
# ADD BASE RSC /l 0x409 /i "C:\NTDDK\inc" /d "NDEBUG"
# ADD RSC /l 0x409 /i "C:\NTDDK\inc" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"Free/solo6000.bsc"
# ADD BSC32 /nologo /o"Free/solo6000.bsc"
LINK32=link.exe
# ADD BASE LINK32 C:\NTDDK\libfre\i386\ntoskrnl.lib C:\NTDDK\libfre\i386\hal.lib C:\NTDDK\libfre\i386\int64.lib C:\NTDDK\libfre\i386\libcntpr.lib C:\NTDDK\libfre\i386\dxapi.lib /base:"0x10000" /version:1.1 /entry:"DriverEntry@8" /pdb:none /machine:IX86 /out:"Free_FastLive\solo6000.sys" -debug:notmapped,MINIMAL -MERGE:_PAGE=PAGE -MERGE:_TEXT=.text -SECTION:INIT,d -OPT:REF -RELEASE -FULLBUILD -FORCE:MULTIPLE -IGNORE:4001 -IGNORE:4037 -IGNORE:4039 -IGNORE:4065 -IGNORE:4070 -IGNORE:4078 -IGNORE:4087 -IGNORE:4089 -osversion:3.51 -MERGE:.rdata=.text -align:0x20 -driver -subsystem:native,3.51
# ADD LINK32 C:\NTDDK\libfre\i386\ntoskrnl.lib C:\NTDDK\libfre\i386\hal.lib C:\NTDDK\libfre\i386\int64.lib C:\NTDDK\libfre\i386\libcntpr.lib C:\NTDDK\libfre\i386\dxapi.lib /base:"0x10000" /version:1.1 /entry:"DriverEntry@8" /pdb:none /machine:IX86 /out:"../bin/solo6010.sys" -debug:notmapped,MINIMAL -MERGE:_PAGE=PAGE -MERGE:_TEXT=.text -SECTION:INIT,d -OPT:REF -RELEASE -FULLBUILD -FORCE:MULTIPLE -IGNORE:4001 -IGNORE:4037 -IGNORE:4039 -IGNORE:4065 -IGNORE:4070 -IGNORE:4078 -IGNORE:4087 -IGNORE:4089 -osversion:3.51 -MERGE:.rdata=.text -align:0x20 -driver -subsystem:native,3.51

!ELSEIF  "$(CFG)" == "SOLO6010Drv - Win32 Checked"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "SOLO6010Drv___Win32_Checked"
# PROP BASE Intermediate_Dir "SOLO6010Drv___Win32_Checked"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Checked"
# PROP Intermediate_Dir "Checked"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Gz /Zp4 /W3 /Z7 /Ot /Oi /Gf /Gy /I "C:\NTDDK\inc" /I "C:\NTDDK\inc\ddk" /I "C:\NTDDK\inc\ddk\wdm" /I "C:\NTDDK\private\inc" /D "STD_CALL" /D _X86_=1 /D i386=1 /D CONDITION_HANDLING=1 /D WIN32_LEAN_AND_MEAN=1 /D NT_INST=0 /D WIN32=100 /D _NT1X_=100 /D NT_UP=1 /D try=__try /D leave=__leave /D except=__except /D finally=__finally /D _CRTAPI1=__cdecl /D _CRTAPI2=__cdecl /D itoa=_itoa /D strcmpi=_strcmpi /D stricmp=_stricmp /D wcsicmp=_wcsicmp /D wcsnicmp=_wcsnicmp /D DEVL=1 /D "_IDWBUILD" /D "__KERNEL__" /D "WINNT" /D "RDRDBG" /D "SRVDBG" /D DBG=1 /D FPO=0 /D "PCI_CODE" /D "LITTLE_ENDIAN" /D "PLX_DRIVER" /D "WDM_DRIVER" /FR /Zel /c
# ADD CPP /nologo /Gz /Zp4 /W3 /Z7 /Ot /Oi /Gf /Gy /I "C:\NTDDK\inc" /I "C:\NTDDK\inc\ddk" /I "C:\NTDDK\inc\ddk\wdm" /I "C:\NTDDK\private\inc" /I "..\Common" /D "STD_CALL" /D _X86_=1 /D i386=1 /D CONDITION_HANDLING=1 /D WIN32_LEAN_AND_MEAN=1 /D NT_INST=0 /D WIN32=100 /D _NT1X_=100 /D NT_UP=1 /D try=__try /D leave=__leave /D except=__except /D finally=__finally /D _CRTAPI1=__cdecl /D _CRTAPI2=__cdecl /D itoa=_itoa /D strcmpi=_strcmpi /D stricmp=_stricmp /D wcsicmp=_wcsicmp /D wcsnicmp=_wcsnicmp /D DEVL=1 /D "_IDWBUILD" /D "__KERNEL__" /D "WINNT" /D "RDRDBG" /D "SRVDBG" /D DBG=1 /D FPO=0 /D "PCI_CODE" /D "LITTLE_ENDIAN" /D "WDM_DRIVER" /FAs /FR /Zel /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL/win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL/win32
# ADD BASE RSC /l 0x409 /i "c:\NTDDK\inc" /d "_DEBUG"
# ADD RSC /l 0x409 /i "c:\NTDDK\inc" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 C:\NTDDK\libchk\i386\ntoskrnl.lib C:\NTDDK\libchk\i386\hal.lib C:\NTDDK\libchk\i386\int64.lib C:\NTDDK\libchk\i386\libcntpr.lib C:\NTDDK\libchk\i386\dxapi.lib /base:"0x10000" /version:1.1 /entry:"DriverEntry@8" /pdb:none /machine:IX86 /out:"Checked_FastLive\solo6000.sys" -debug:notmapped,FULL -MERGE:_PAGE=PAGE -MERGE:_TEXT=.text -SECTION:INIT,d -OPT:REF -RELEASE -FULLBUILD -FORCE:MULTIPLE -IGNORE:4001 -IGNORE:4037 -IGNORE:4039 -IGNORE:4065 -IGNORE:4070 -IGNORE:4078 -IGNORE:4087 -IGNORE:4089 -osversion:3.51 -MERGE:.rdata=.text -align:0x20 -driver -subsystem:native,3.51
# ADD LINK32 C:\NTDDK\libchk\i386\ntoskrnl.lib C:\NTDDK\libchk\i386\hal.lib C:\NTDDK\libchk\i386\int64.lib C:\NTDDK\libchk\i386\libcntpr.lib C:\NTDDK\libchk\i386\dxapi.lib /base:"0x10000" /version:1.1 /entry:"DriverEntry@8" /pdb:none /machine:IX86 /out:"../bin_debug/solo6010.sys" -debug:notmapped,FULL -MERGE:_PAGE=PAGE -MERGE:_TEXT=.text -SECTION:INIT,d -OPT:REF -RELEASE -FULLBUILD -FORCE:MULTIPLE -IGNORE:4001 -IGNORE:4037 -IGNORE:4039 -IGNORE:4065 -IGNORE:4070 -IGNORE:4078 -IGNORE:4087 -IGNORE:4089 -osversion:3.51 -MERGE:.rdata=.text -align:0x20 -driver -subsystem:native,3.51

!ENDIF 

# Begin Target

# Name "SOLO6010Drv - Win32 Free"
# Name "SOLO6010Drv - Win32 Checked"
# Begin Group "Source File"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Dispatch.c
DEP_CPP_DISPA=\
	"..\Common\Solo6010_Global.h"\
	"..\Common\Solo6010_Ioctl.h"\
	"..\Common\solo6010_register.h"\
	".\Dispatch.h"\
	".\Driver.h"\
	".\DriverDefs.h"\
	".\Interrupts.h"\
	".\Solo6010.h"\
	".\Solo6010_Display.h"\
	".\Solo6010_DrvGlobal.h"\
	".\Solo6010_G723.h"\
	".\Solo6010_GPIO.h"\
	".\Solo6010_I2C.h"\
	".\Solo6010_MP4Dec.h"\
	".\Solo6010_MP4Enc.h"\
	".\Solo6010_TW2815.h"\
	".\Solo6010_UART.h"\
	".\Solo6010_VideoIn.h"\
	".\SupportFunc.h"\
	".\TypesDef.h"\
	"C:\NTDDK\inc\alpharef.h"\
	"C:\NTDDK\inc\basetsd.h"\
	"C:\NTDDK\inc\bugcodes.h"\
	"C:\NTDDK\inc\guiddef.h"\
	"C:\NTDDK\inc\ia64reg.h"\
	"C:\NTDDK\inc\ntdef.h"\
	"C:\NTDDK\inc\ntiologc.h"\
	"C:\NTDDK\inc\ntstatus.h"\
	"C:\NTDDK\inc\wdm.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Driver.c
DEP_CPP_DRIVE=\
	"..\Common\Solo6010_Global.h"\
	"..\Common\solo6010_register.h"\
	".\Dispatch.h"\
	".\Driver.h"\
	".\DriverDefs.h"\
	".\Interrupts.h"\
	".\PlugPlay.h"\
	".\Power.h"\
	".\Solo6010.h"\
	".\Solo6010_DrvGlobal.h"\
	".\Solo6010_I2C.h"\
	".\Solo6010_P2M.h"\
	".\Solo6010_UART.h"\
	".\SupportFunc.h"\
	".\TypesDef.h"\
	"C:\NTDDK\inc\alpharef.h"\
	"C:\NTDDK\inc\basetsd.h"\
	"C:\NTDDK\inc\bugcodes.h"\
	"C:\NTDDK\inc\guiddef.h"\
	"C:\NTDDK\inc\ia64reg.h"\
	"C:\NTDDK\inc\ntdef.h"\
	"C:\NTDDK\inc\ntiologc.h"\
	"C:\NTDDK\inc\ntstatus.h"\
	"C:\NTDDK\inc\wdm.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Interrupts.c
DEP_CPP_INTER=\
	"..\Common\Solo6010_Global.h"\
	"..\Common\solo6010_register.h"\
	".\DriverDefs.h"\
	".\Interrupts.h"\
	".\Solo6010.h"\
	".\Solo6010_DrvGlobal.h"\
	".\SupportFunc.h"\
	".\TypesDef.h"\
	"C:\NTDDK\inc\alpharef.h"\
	"C:\NTDDK\inc\basetsd.h"\
	"C:\NTDDK\inc\bugcodes.h"\
	"C:\NTDDK\inc\guiddef.h"\
	"C:\NTDDK\inc\ia64reg.h"\
	"C:\NTDDK\inc\ntdef.h"\
	"C:\NTDDK\inc\ntiologc.h"\
	"C:\NTDDK\inc\ntstatus.h"\
	"C:\NTDDK\inc\wdm.h"\
	
# End Source File
# Begin Source File

SOURCE=.\PlugPlay.c
DEP_CPP_PLUGP=\
	"..\Common\Solo6010_Global.h"\
	"..\Common\solo6010_register.h"\
	".\Dispatch.h"\
	".\Driver.h"\
	".\DriverDefs.h"\
	".\PlugPlay.h"\
	".\Solo6010_DrvGlobal.h"\
	".\SupportFunc.h"\
	".\TypesDef.h"\
	"C:\NTDDK\inc\alpharef.h"\
	"C:\NTDDK\inc\basetsd.h"\
	"C:\NTDDK\inc\bugcodes.h"\
	"C:\NTDDK\inc\guiddef.h"\
	"C:\NTDDK\inc\ia64reg.h"\
	"C:\NTDDK\inc\ntdef.h"\
	"C:\NTDDK\inc\ntiologc.h"\
	"C:\NTDDK\inc\ntstatus.h"\
	"C:\NTDDK\inc\wdm.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Power.c
DEP_CPP_POWER=\
	"..\Common\Solo6010_Global.h"\
	"..\Common\solo6010_register.h"\
	".\Driver.h"\
	".\DriverDefs.h"\
	".\Power.h"\
	".\Solo6010_DrvGlobal.h"\
	".\SupportFunc.h"\
	".\TypesDef.h"\
	"C:\NTDDK\inc\alpharef.h"\
	"C:\NTDDK\inc\basetsd.h"\
	"C:\NTDDK\inc\bugcodes.h"\
	"C:\NTDDK\inc\guiddef.h"\
	"C:\NTDDK\inc\ia64reg.h"\
	"C:\NTDDK\inc\ntdef.h"\
	"C:\NTDDK\inc\ntiologc.h"\
	"C:\NTDDK\inc\ntstatus.h"\
	"C:\NTDDK\inc\wdm.h"\
	
# End Source File
# Begin Source File

SOURCE=.\SupportFunc.c
DEP_CPP_SUPPO=\
	"..\Common\Solo6010_Global.h"\
	"..\Common\solo6010_register.h"\
	".\DriverDefs.h"\
	".\Solo6010_DrvGlobal.h"\
	".\SupportFunc.h"\
	".\TypesDef.h"\
	"C:\NTDDK\inc\alpharef.h"\
	"C:\NTDDK\inc\basetsd.h"\
	"C:\NTDDK\inc\bugcodes.h"\
	"C:\NTDDK\inc\guiddef.h"\
	"C:\NTDDK\inc\ia64reg.h"\
	"C:\NTDDK\inc\ntdef.h"\
	"C:\NTDDK\inc\ntiologc.h"\
	"C:\NTDDK\inc\ntstatus.h"\
	"C:\NTDDK\inc\wdm.h"\
	"C:\NTDDK\inc\wdmguid.h"\
	
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Dispatch.h
# End Source File
# Begin Source File

SOURCE=.\Driver.h
# End Source File
# Begin Source File

SOURCE=.\DriverDefs.h
# End Source File
# Begin Source File

SOURCE=.\Interrupts.h
# End Source File
# Begin Source File

SOURCE=.\PlugPlay.h
# End Source File
# Begin Source File

SOURCE=.\Power.h
# End Source File
# Begin Source File

SOURCE=.\SupportFunc.h
# End Source File
# Begin Source File

SOURCE=.\TypesDef.h
# End Source File
# End Group
# Begin Group "SOLO6010"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Solo6010.c
DEP_CPP_SOLO6=\
	"..\Common\Solo6010_Global.h"\
	"..\Common\solo6010_register.h"\
	".\DriverDefs.h"\
	".\Solo6010.h"\
	".\Solo6010_Display.h"\
	".\Solo6010_DrvGlobal.h"\
	".\Solo6010_G723.h"\
	".\Solo6010_GPIO.h"\
	".\Solo6010_I2C.h"\
	".\Solo6010_MP4Dec.h"\
	".\Solo6010_MP4Enc.h"\
	".\Solo6010_P2M.h"\
	".\Solo6010_SAA7128.h"\
	".\Solo6010_TW2815.h"\
	".\Solo6010_UART.h"\
	".\Solo6010_VideoIn.h"\
	".\SupportFunc.h"\
	".\TypesDef.h"\
	"C:\NTDDK\inc\alpharef.h"\
	"C:\NTDDK\inc\basetsd.h"\
	"C:\NTDDK\inc\bugcodes.h"\
	"C:\NTDDK\inc\guiddef.h"\
	"C:\NTDDK\inc\ia64reg.h"\
	"C:\NTDDK\inc\ntdef.h"\
	"C:\NTDDK\inc\ntiologc.h"\
	"C:\NTDDK\inc\ntstatus.h"\
	"C:\NTDDK\inc\wdm.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Solo6010.h
# End Source File
# Begin Source File

SOURCE=.\Solo6010_Display.c
DEP_CPP_SOLO60=\
	"..\Common\Solo6010_Global.h"\
	"..\Common\solo6010_register.h"\
	".\DriverDefs.h"\
	".\Solo6010.h"\
	".\Solo6010_Display.h"\
	".\Solo6010_DrvGlobal.h"\
	".\SupportFunc.h"\
	".\TypesDef.h"\
	"C:\NTDDK\inc\alpharef.h"\
	"C:\NTDDK\inc\basetsd.h"\
	"C:\NTDDK\inc\bugcodes.h"\
	"C:\NTDDK\inc\guiddef.h"\
	"C:\NTDDK\inc\ia64reg.h"\
	"C:\NTDDK\inc\ntdef.h"\
	"C:\NTDDK\inc\ntiologc.h"\
	"C:\NTDDK\inc\ntstatus.h"\
	"C:\NTDDK\inc\wdm.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Solo6010_Display.h
# End Source File
# Begin Source File

SOURCE=.\Solo6010_DrvGlobal.h
# End Source File
# Begin Source File

SOURCE=.\Solo6010_G723.c
DEP_CPP_SOLO601=\
	"..\Common\Solo6010_Global.h"\
	"..\Common\solo6010_register.h"\
	".\DriverDefs.h"\
	".\Solo6010.h"\
	".\Solo6010_DrvGlobal.h"\
	".\Solo6010_G723.h"\
	".\Solo6010_I2C.h"\
	".\Solo6010_P2M.h"\
	".\SupportFunc.h"\
	".\TypesDef.h"\
	"C:\NTDDK\inc\alpharef.h"\
	"C:\NTDDK\inc\basetsd.h"\
	"C:\NTDDK\inc\bugcodes.h"\
	"C:\NTDDK\inc\guiddef.h"\
	"C:\NTDDK\inc\ia64reg.h"\
	"C:\NTDDK\inc\ntdef.h"\
	"C:\NTDDK\inc\ntiologc.h"\
	"C:\NTDDK\inc\ntstatus.h"\
	"C:\NTDDK\inc\wdm.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Solo6010_G723.h
# End Source File
# Begin Source File

SOURCE=..\Common\Solo6010_Global.h
# End Source File
# Begin Source File

SOURCE=.\Solo6010_GPIO.c
DEP_CPP_SOLO6010=\
	"..\Common\Solo6010_Global.h"\
	"..\Common\solo6010_register.h"\
	".\DriverDefs.h"\
	".\Solo6010.h"\
	".\Solo6010_DrvGlobal.h"\
	".\Solo6010_GPIO.h"\
	".\SupportFunc.h"\
	".\TypesDef.h"\
	"C:\NTDDK\inc\alpharef.h"\
	"C:\NTDDK\inc\basetsd.h"\
	"C:\NTDDK\inc\bugcodes.h"\
	"C:\NTDDK\inc\guiddef.h"\
	"C:\NTDDK\inc\ia64reg.h"\
	"C:\NTDDK\inc\ntdef.h"\
	"C:\NTDDK\inc\ntiologc.h"\
	"C:\NTDDK\inc\ntstatus.h"\
	"C:\NTDDK\inc\wdm.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Solo6010_GPIO.h
# End Source File
# Begin Source File

SOURCE=.\Solo6010_I2C.c
DEP_CPP_SOLO6010_=\
	"..\Common\Solo6010_Global.h"\
	"..\Common\solo6010_register.h"\
	".\DriverDefs.h"\
	".\Solo6010.h"\
	".\Solo6010_DrvGlobal.h"\
	".\Solo6010_I2C.h"\
	".\SupportFunc.h"\
	".\TypesDef.h"\
	"C:\NTDDK\inc\alpharef.h"\
	"C:\NTDDK\inc\basetsd.h"\
	"C:\NTDDK\inc\bugcodes.h"\
	"C:\NTDDK\inc\guiddef.h"\
	"C:\NTDDK\inc\ia64reg.h"\
	"C:\NTDDK\inc\ntdef.h"\
	"C:\NTDDK\inc\ntiologc.h"\
	"C:\NTDDK\inc\ntstatus.h"\
	"C:\NTDDK\inc\wdm.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Solo6010_I2C.h
# End Source File
# Begin Source File

SOURCE=..\Common\Solo6010_Ioctl.h
# End Source File
# Begin Source File

SOURCE=.\Solo6010_MP4Dec.c
DEP_CPP_SOLO6010_M=\
	"..\Common\Solo6010_Global.h"\
	"..\Common\solo6010_register.h"\
	".\DriverDefs.h"\
	".\Solo6010.h"\
	".\Solo6010_DrvGlobal.h"\
	".\Solo6010_MP4Dec.h"\
	".\SupportFunc.h"\
	".\TypesDef.h"\
	"C:\NTDDK\inc\alpharef.h"\
	"C:\NTDDK\inc\basetsd.h"\
	"C:\NTDDK\inc\bugcodes.h"\
	"C:\NTDDK\inc\guiddef.h"\
	"C:\NTDDK\inc\ia64reg.h"\
	"C:\NTDDK\inc\ntdef.h"\
	"C:\NTDDK\inc\ntiologc.h"\
	"C:\NTDDK\inc\ntstatus.h"\
	"C:\NTDDK\inc\wdm.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Solo6010_MP4Dec.h
# End Source File
# Begin Source File

SOURCE=.\Solo6010_MP4Enc.c
DEP_CPP_SOLO6010_MP=\
	"..\Common\Solo6010_Global.h"\
	"..\Common\solo6010_register.h"\
	".\DriverDefs.h"\
	".\Solo6010.h"\
	".\Solo6010_DrvGlobal.h"\
	".\Solo6010_MP4Enc.h"\
	".\Solo6010_P2M.h"\
	".\SupportFunc.h"\
	".\TypesDef.h"\
	"C:\NTDDK\inc\alpharef.h"\
	"C:\NTDDK\inc\basetsd.h"\
	"C:\NTDDK\inc\bugcodes.h"\
	"C:\NTDDK\inc\guiddef.h"\
	"C:\NTDDK\inc\ia64reg.h"\
	"C:\NTDDK\inc\ntdef.h"\
	"C:\NTDDK\inc\ntiologc.h"\
	"C:\NTDDK\inc\ntstatus.h"\
	"C:\NTDDK\inc\wdm.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Solo6010_MP4Enc.h
# End Source File
# Begin Source File

SOURCE=.\Solo6010_P2M.c
DEP_CPP_SOLO6010_P=\
	"..\Common\Solo6010_Global.h"\
	"..\Common\solo6010_register.h"\
	".\DriverDefs.h"\
	".\Solo6010.h"\
	".\Solo6010_DrvGlobal.h"\
	".\Solo6010_P2M.h"\
	".\SupportFunc.h"\
	".\TypesDef.h"\
	"C:\NTDDK\inc\alpharef.h"\
	"C:\NTDDK\inc\basetsd.h"\
	"C:\NTDDK\inc\bugcodes.h"\
	"C:\NTDDK\inc\guiddef.h"\
	"C:\NTDDK\inc\ia64reg.h"\
	"C:\NTDDK\inc\ntdef.h"\
	"C:\NTDDK\inc\ntiologc.h"\
	"C:\NTDDK\inc\ntstatus.h"\
	"C:\NTDDK\inc\wdm.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Solo6010_P2M.h
# End Source File
# Begin Source File

SOURCE=..\Common\solo6010_register.h
# End Source File
# Begin Source File

SOURCE=.\Solo6010_SAA7128.c
DEP_CPP_SOLO6010_S=\
	"..\Common\Solo6010_Global.h"\
	"..\Common\solo6010_register.h"\
	".\DriverDefs.h"\
	".\Solo6010.h"\
	".\Solo6010_DrvGlobal.h"\
	".\Solo6010_I2C.h"\
	".\Solo6010_SAA7128.h"\
	".\SupportFunc.h"\
	".\TypesDef.h"\
	"C:\NTDDK\inc\alpharef.h"\
	"C:\NTDDK\inc\basetsd.h"\
	"C:\NTDDK\inc\bugcodes.h"\
	"C:\NTDDK\inc\guiddef.h"\
	"C:\NTDDK\inc\ia64reg.h"\
	"C:\NTDDK\inc\ntdef.h"\
	"C:\NTDDK\inc\ntiologc.h"\
	"C:\NTDDK\inc\ntstatus.h"\
	"C:\NTDDK\inc\wdm.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Solo6010_SAA7128.h
# End Source File
# Begin Source File

SOURCE=.\Solo6010_TW2815.c
DEP_CPP_SOLO6010_T=\
	"..\Common\Solo6010_Global.h"\
	"..\Common\solo6010_register.h"\
	".\DriverDefs.h"\
	".\Solo6010.h"\
	".\Solo6010_DrvGlobal.h"\
	".\Solo6010_I2C.h"\
	".\Solo6010_TW2815.h"\
	".\SupportFunc.h"\
	".\TypesDef.h"\
	"C:\NTDDK\inc\alpharef.h"\
	"C:\NTDDK\inc\basetsd.h"\
	"C:\NTDDK\inc\bugcodes.h"\
	"C:\NTDDK\inc\guiddef.h"\
	"C:\NTDDK\inc\ia64reg.h"\
	"C:\NTDDK\inc\ntdef.h"\
	"C:\NTDDK\inc\ntiologc.h"\
	"C:\NTDDK\inc\ntstatus.h"\
	"C:\NTDDK\inc\wdm.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Solo6010_TW2815.h
# End Source File
# Begin Source File

SOURCE=.\Solo6010_UART.c
DEP_CPP_SOLO6010_U=\
	"..\Common\Solo6010_Global.h"\
	"..\Common\solo6010_register.h"\
	".\DriverDefs.h"\
	".\Solo6010.h"\
	".\Solo6010_DrvGlobal.h"\
	".\Solo6010_UART.h"\
	".\SupportFunc.h"\
	".\TypesDef.h"\
	"C:\NTDDK\inc\alpharef.h"\
	"C:\NTDDK\inc\basetsd.h"\
	"C:\NTDDK\inc\bugcodes.h"\
	"C:\NTDDK\inc\guiddef.h"\
	"C:\NTDDK\inc\ia64reg.h"\
	"C:\NTDDK\inc\ntdef.h"\
	"C:\NTDDK\inc\ntiologc.h"\
	"C:\NTDDK\inc\ntstatus.h"\
	"C:\NTDDK\inc\wdm.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Solo6010_UART.h
# End Source File
# Begin Source File

SOURCE=.\Solo6010_VideoIn.c
DEP_CPP_SOLO6010_V=\
	"..\Common\Solo6010_Global.h"\
	"..\Common\solo6010_register.h"\
	".\DriverDefs.h"\
	".\Solo6010.h"\
	".\Solo6010_DrvGlobal.h"\
	".\Solo6010_P2M.h"\
	".\Solo6010_VideoIn.h"\
	".\SupportFunc.h"\
	".\TypesDef.h"\
	"C:\NTDDK\inc\alpharef.h"\
	"C:\NTDDK\inc\basetsd.h"\
	"C:\NTDDK\inc\bugcodes.h"\
	"C:\NTDDK\inc\guiddef.h"\
	"C:\NTDDK\inc\ia64reg.h"\
	"C:\NTDDK\inc\ntdef.h"\
	"C:\NTDDK\inc\ntiologc.h"\
	"C:\NTDDK\inc\ntstatus.h"\
	"C:\NTDDK\inc\wdm.h"\
	
# End Source File
# Begin Source File

SOURCE=.\Solo6010_VideoIn.h
# End Source File
# End Group
# End Target
# End Project
