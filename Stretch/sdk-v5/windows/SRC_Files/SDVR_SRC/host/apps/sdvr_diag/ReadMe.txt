========================================================================
    CONSOLE APPLICATION : sdvr_diag Project Overview
========================================================================

This file contains a summary of what you will find in each of the files that
make up your sdvr_diag application.


sdvr_diag.vcproj
    This is the main project file for VC++ projects generated using an Application Wizard. 
    It contains information about the version of Visual C++ that generated the file, and 
    information about the platforms, configurations, and project features selected with the
    Application Wizard.

sdvr_diag.cpp
	Defines the entry point for the Stretch DVR PCIe board console application.


/////////////////////////////////////////////////////////////////////////////
Other notes:

note 1:
  This applications loads the given diagnostics firmware to the given 
  DVR PCIe board to run various diagnostic tests on the DVR board.  It then
  prints the test result for each PE on the console.

   Usage:

       sdvr_diag <board_index> <diag_fw>

   where:
       board_index: The nth PCIe DVR board installed on your PC. Currently
                    this value is always 1. The valid range is 1 - 4;

       diag_fw:     The path to the romable diagnostics firmware.

note 2:
	This project is dependent on sct.dll/sct.lib as well as sdk_sdvr.lib
/////////////////////////////////////////////////////////////////////////////
