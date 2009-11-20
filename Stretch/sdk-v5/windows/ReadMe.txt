Modification of Date:  2009/ 05/ 27

<Important Note>

1. You must first install vcredist_x86.exe package before 
    running *.EXE in DemoApp folder.

2. These sample programs were developed with VC2005, 
   so you must install DirectShow SDK first.

3. You must execute these *.EXEs individually,
   they can not be executed an the same time.

4. The SDK is for windows OS.

5. For I/O card, please be reminded, the GND of your device should be connected to GND
   (pin 9) of I/O card.

6. Please be reminded the recording file will be saved under the path "C:\EncodeDvrFCD Record" for simple SDK.

7. Windows XP Settings
   The following are recommended for operating 33¡V64 channels:
   1. Make sure that at least 2 GB of memory is installed in the system.
      More may be needed depending on your application requirements.
   2. Using the registry editor, change the value of the registry setting
      HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\Memory
      Management\PagedPoolSize to 0x20000000 (512 MB). The system must be rebooted for this change to
      take affect.
      After these changes have taken affect, all boards should be reported as working in Windows Device Manager.


<Simple Step>
1. first, execute main *.EXE file.

2. select *.Rom which is under the path ..\RomFiles.
   => The *.Rom file must depend on how many ports for your PC DVR card.

3. You must select *.bdf which is under the path, ..\Font_Files, in order to use different OSD language



<Description of Folders>

1. DemoApp :: Execute files for demo
   (1) ..\Advanced_Bin => The sample programs include the full functions for the HW device.
   (2) ..\Simple_Bin   => The sample program is designed under MFC framework.

2. Doc_Files :: Document files
   (1) .\Advanced => Thes document is for the SDVR_SRC sample.
   (2) .\Simple   => Thes document is for the SIMPLE_SRC sample.

3. Drv_Files :: Driver files
   (1) *.inf & *.sys

4. Font_Files :: Font files
   (1)  *.bdf
   (2) You can make the font files depended on different languages by youself.
	
5. RomFiles :: Rom files
   (1)  *.rom
   (2) It is important to load the rom file which must be matched with your card.

6  SRC_Files :: Source Code
   (1) .\SDVR_SRC 
       => The sample programs support full functions(including to-TV), but it must be developed under QT.
                 You must set the QT development by yourself.
       =>  The best advantage of QT is that it can be developed under windows and linux OS.

       (a) .\common_include  => Include Files
       (b) .\DemoBin         => Execute Files
       (c) .\host            => Main source code


   (2) .\SIMPLE_SRC(MFC)
       => We consider someone don't understand QT, 
          so we support the simple demo SDK.
       => The sample programs support basic functions.
          If you want to use advanced function, you still can refer to SDVR_SRC.
       
       (a) .\bin
       (b) .\codecs
       (c) .\include	     => Include Files
       (d) .\lib
       (e) .\Obj
       (f) .\SDK_Bin 	     => execute files.
       (g) .\source	     => Main source code
       
PS:
   1. MPEG4 is ALPHA release.
      not yet decided whether to formally support it until now.
      It is not suggested to apply it.
   2. The HW decoder function only decodes video format of H264.



<Tested Platforms>

We like to provide the list which includes the tested platforms. The list will keep growing
once more tests have been done

1.       Gigabyte GA-G1975X (Intel 975 chipset) for PCI-e 1x & 4x

2.       Gigabyte GA-D33-DS3R (Intel G33 chipset) for PCI-e 1x & 4x

3.       Gigabyte GA-8I945GMF (Intel 945 chipset) for PCI-e 1x

4.       Gigabyte GA-EP45-DS3R (Intel P45 chipset) for PCI-e 1x & 4x

5.       ASUS P5KPL-C/1600 (Intel G31 chipset) for PCI-e 1x

6.       ASUS P5LD2-VM (Intel 945 chipset) for PCI-e 1x

7.       ASUS P5LD2 (Intel 945 chipset) for PCI-e 1x

8.       Asrock 4core1333-GLAN/M2 for PCI-e 1x

9.       Gigabyte GA-G31MX-S2 for PCI-e 4x

10.      ASUS     P5E64 WS Evolution for PCI-e 4x

11.      ASUS     P5B-VM SE for PCI-e 1x

12.      ASUS     P5GD1-VM for PCI-e 1x