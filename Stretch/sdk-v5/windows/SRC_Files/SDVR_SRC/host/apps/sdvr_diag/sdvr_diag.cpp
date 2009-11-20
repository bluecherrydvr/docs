/************************************************************************
 *
 * Copyright C 2007 Stretch, Inc. All rights reserved.  Stretch products are
 * protected under numerous U.S. and foreign patents, maskwork rights,
 * copyrights and other intellectual property laws.  
 *
 * This source code and the related tools, software code and documentation, and
 * your use thereof, are subject to and governed by the terms and conditions of
 * the applicable Stretch IDE or SDK and RDK License Agreement
 * (either as agreed by you or found at www.stretchinc.com).  By using these
 * items, you indicate your acceptance of such terms and conditions between you
 * and Stretch, Inc.  In the event that you do not agree with such terms and
 * conditions, you may not use any of these items and must immediately destroy
 * any copies you have made.
 *
 ************************************************************************/
/*****************************************************************************
  sdvr_diag.cpp : 
  Defines the entry point for the Stretch DVR PCIe board console application.
 
  This application loads the given diagnostics firmware to the given 
  DVR PCIe board to run various diagnostic tests on the DVR board.  It then
  prints the test result for each PE on the console.

  See function usage() for the usage of this program.

**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "sdvr_sdk.h"

void usage();
void print_diag_result(int pe_no, sdvr_diag_code_e diag_code);
void print_pci_info(sx_uint32 board_index);

#define MAX_NUM_PROCESSORS 32
#define MAX_DVR_BOARDS     8
/************************************************************************
  The application version
************************************************************************/
#define DIAG_TOOL_VERSION_MAJOR     4
#define DIAG_TOOL_VERSION_MINOR     1
#define DIAG_TOOL_VERSION_REVISION  1
#define DIAG_TOOL_VERSION_BUILD     0


/************************************************************************
                        M A I N    F U N C T I O N
************************************************************************/
int main(int argc, char * argv[])
{
    sdvr_err_e err;
    sx_uint32 board_index;
    char *diag_file_name;
    sdvr_diag_code_e diag_code[MAX_NUM_PROCESSORS];
    int diag_code_size;
    sx_uint32 board_count;
    sx_uint32 start_board_index;
    sx_uint32 end_board_index;
    sx_uint32 cur_board_index;

    /* Print the tools banner */
    printf("*******************************************************\n");
    printf("   Stretch Diagnostics Tool - Version %d.%d.%d.%d\n",
        DIAG_TOOL_VERSION_MAJOR,DIAG_TOOL_VERSION_MINOR,DIAG_TOOL_VERSION_REVISION,DIAG_TOOL_VERSION_BUILD);
    printf("*******************************************************\n\n");


    /* Validate the command line arguments */
    if (argc != 3)
    {
        usage();
        return 1;
    }

    board_index = atoi(argv[1]);
    if (board_index > MAX_DVR_BOARDS)
    {
        usage();
        return 1;
    }


    diag_file_name = argv[2];

    err = sdvr_sdk_init();
    if (err != SDVR_ERR_NONE)
    {
        printf("Application Error: SDK initialization failed. [Error = %s]\n",sdvr_get_error_text(err));
        return 1;
    }

    board_count = sdvr_get_board_count();
    printf("Total number of PCIe DVR board(s) installed:\t%d\n\n",board_count);

    if (board_index > board_count)
    {
        printf("Application Error: Invalid board index [%d].\n",board_index);
        return 1;
    }

    if (board_index == 0)
    {
        start_board_index = 0;
        end_board_index = board_count;
    }
    else
    {
        start_board_index =  board_index - 1;
        end_board_index = start_board_index + 1;
    }

    printf("Diagnostics ROM file: %s\n",diag_file_name);
    for (cur_board_index = start_board_index; cur_board_index < end_board_index; cur_board_index++)
    {

        /* Print PCIe board specific information */
        print_pci_info(cur_board_index);

        printf("Running the diagnostics on DVR board id %d...\n\n",cur_board_index+1);

        if ((err = sdvr_run_diagnostics(cur_board_index, diag_file_name, diag_code, &diag_code_size))
            == SDVR_ERR_NONE)
        {
            for (int proc_no = 0; proc_no < diag_code_size; proc_no++)
                print_diag_result(proc_no, diag_code[proc_no]);
        }
        else
        {
            switch (err)
            {
            case SDVR_ERR_INVALID_BOARD: 
                printf("Application Error: Invalid board index [%d].\n",cur_board_index);
                break;
            case  SDVR_ERR_LOAD_FIRMWARE:
                printf("Application Error: Failed to load the firmware [%s]\n", diag_file_name);
                break;
            default:
                printf("Application Error: sdvr_run_diagnostics failed. Error [%s]\n", 
                    sdvr_get_error_text(err));
                break;
            }

            return 1;
        }

    }
	return 0;
}

void print_diag_result(int pe_no, sdvr_diag_code_e diag_code)
{
    printf ("Result for PE %d. Diagnostics Code: 0x%x\n\t",pe_no,diag_code);

    switch (diag_code)
    {
    case SDVR_DIAG_OK:                        
        printf("All the Diagnostics Tests Passed.\n");
        break;

    // *  Boot-loader DDR test */

    case SDVR_DIAG_DDR_WRITEREAD_FAIL:
        printf("Diagnostics Test DDR_WRITEREAD_FAIL - DDR write/read Failure.\n");
        break;

    case SDVR_DIAG_DDR_ADDRLINES_FAIL:
        printf("Diagnostics Test DDR_ADDRLINES_FAIL - DDR address lines Failure.\n");
        break;

    case SDVR_DIAG_DDR_BITFLIP_FAIL:
        printf("Diagnostics Test DDR_BITFLIP_FAIL - DDR bit-flip Failure.\n");
        break;

    case SDVR_DIAG_DDR_DMA_FAIL:
        printf("Diagnostics Test DDR_DMA_FAIL - DDR DMA Failure.\n");
        break;

    case SDVR_DIAG_DDR_READ_DMA_FAIL:  
        printf("Diagnostics Test DDR_READ_DMA_FAIL - DDR read/DMA Failure.\n");
        break;

    /* PLL test errors */

    case SDVR_DIAG_PLL_TEST_MHZ:
        printf("Diagnostics Test SDVR_DIAG_PLL_TEST_MHZ - Processor Speed Failure. \n");
        break;

    case SDVR_DIAG_PLL_TEST_SYS:
        printf("Diagnostics Test PLL_TEST_SYS Failed.\n");
        break;

    case SDVR_DIAG_PLL_TEST_IO:
        printf("Diagnostics Test PLL_TEST_IO Failed.\n");
        break;

    case SDVR_DIAG_PLL_TEST_AIM:
        printf("Diagnostics Test PLL_TEST_AIM Failed.\n");
        break;
        
    case SDVR_DIAG_PLL_TEST_DP0:
        printf("Diagnostics Test PLL_TEST_DP0 Failed.\n");
        break;

    case SDVR_DIAG_PLL_TEST_DP2:
        printf("Diagnostics Test PLL_TEST_DP2 Failed.\n");
        break;

    case SDVR_DIAG_PLL_TEST_DDR:
        printf("Diagnostics Test PLL_TEST_DDR Failed.\n");
        break;

    /* SPI flash test errors */

    case SDVR_DIAG_SPI_TEST_READ:
        printf("Diagnostics Test SPI_TEST_READ - Flash Read Failure.\n");
        break;

    case SDVR_DIAG_SPI_TEST_ERASE:
        printf("Diagnostics Test SPI_TEST_ERASE - Flash Erase Failure.\n");
        break;

    case SDVR_DIAG_SPI_TEST_PROG:
        printf("Diagnostics Test SPI_TEST_PROG - Flash Program Failure.\n");
        break;

    case SDVR_DIAG_SPI_TEST_UNLOCK:
        printf("Diagnostics Test SPI_TEST_UNLOCK - Flash Unlock Failure.\n");
        break;

    case SDVR_DIAG_SPI_TEST_COMPARE:
        printf("Diagnostics Test SPI_TEST_COMPARE - Flash Data miscomare Failure.\n");
        break;

    case SDVR_DIAG_SPI_TEST_MAINT:
        printf("Diagnostics Test SPI_TEST_MAINT - Flash Maintenance Command Failure.\n");
        break;

    case SDVR_DIAG_SPI_TEST_MISC:
        printf("Diagnostics Test SPI_TEST_MISC - Miscellaneous Flash Failure.\n");
        break;

    /* TWI EEPROM test errors */

    case SDVR_DIAG_TWI_EEPROM_TEST_READ:
        printf("Diagnostics Test TWI_EEPROM_TEST_READ - Read Failure.\n");
        break;

    case SDVR_DIAG_TWI_EEPROM_TEST_WRITE:
        printf("Diagnostics Test TWI_EEPROM_TEST_WRITE - Write Failure.\n");
        break;

    case SDVR_DIAG_TWI_EEPROM_TEST_INIT:
        printf("Diagnostics Test TWI_EEPROM_TEST_INIT - Initialization Failure.\n");
        break;

    case SDVR_DIAG_TWI_EEPROM_TEST_COMPARE:
        printf("Diagnostics Test TWI_EEPROM_TEST_COMPARE - miscompare Failure.\n");
        break;

    case SDVR_DIAG_TWI_EEPROM_TEST_WP_COMPARE:
        printf("Diagnostics Test TWI_EEPROM_TEST_WP_COMPARE - write-protect Failure.\n");
        break;

    /* Epson test errors */

    case SDVR_DIAG_EPSON_REG_TEST_INIT:
        printf("Diagnostics Test EPSON_REG_TEST_INIT - Initialization Failure.\n");
        break;

    case SDVR_DIAG_EPSON_REG_TEST_WALKING:
        printf("Diagnostics Test EPSON_REG_TEST_WALKING - register bit-walk Failure.\n");
        break;

    /* Decoder test errors */

    case SDVR_DIAG_DECODER_AUDIO_TEST_INIT:
        printf("Diagnostics Test DECODER_AUDIO_TEST_INIT - Audio Test Init Failure.\n");
        break;

    case SDVR_DIAG_DECODER_AUDIO_TEST_NO_AUDIO:
        printf("Diagnostics Test DECODER_AUDIO_TEST_NO_AUDIO - Audio Not Received Failure.\n");
        break;

    case SDVR_DIAG_TW2815_REG_TEST:
        printf("Diagnostics Test TW2815_REG_TEST - Register Test Failure.\n");
        break;

    case SDVR_DIAG_TW2864_REG_TEST:
        printf("Diagnostics Test TW2864_REG_TEST - Register Test Failure.\n");
        break;

    case SDVR_DIAG_DECODER_VIDEO_TEST_INIT:
        printf("Diagnostics Test DECODER_VIDEO_TEST_INIT - Video Test Init Failure.\n");
        break;

    case SDVR_DIAG_DECODER_VIDEO_TEST_NO_VIDEO:
        printf("Diagnostics Test DECODER_VIDEO_TEST_NO_VIDEO - Video Not Received Failure.\n");
        break;

    case SDVR_DIAG_DECODER_VIDEO_TEST_TIMEOUT:
        printf("Diagnostics Test SDVR_DIAG_DECODER_VIDEO_TEST_TIMEOUT - Video Test Timeout Failure.\n");
        break;

    case SDVR_DIAG_DECODER_VIDDET_TEST_INIT_ERR :  
        printf("Diagnostics Test DECODER_VIDDET_TEST_INIT_ERR - Video Detect Init Failure.\n");
        break;

    case SDVR_DIAG_DECODER_VIDDET_TEST_UNKNOWN_CHIP:
        printf("Diagnostics Test SDVR_DIAG_DECODER_VIDDET_TEST_UNKNOWN_CHIP - Video Detect Unknown Chip Failure.\n");
        break;

    case SDVR_DIAG_DECODER_VIDDET_TEST_NO_INPUT_ERR:
        printf("Diagnostics Test DECODER_VIDDET_TEST_NO_INPUT - Video Detect No Input Failure.\n");
        break;

    case SDVR_DIAG_DECODER_VIDDET_TEST_CONFLICT_ERR:
        printf("Diagnostics Test DECODER_VIDDET_TEST_CONFLICT - Video Detect Conflict Failure.\n");
        break;

    case SDVR_DIAG_DECODER_VIDDET_TEST_NO_SYNC_ERR:
        printf("Diagnostics Test DECODER_VIDDET_TEST_NO_SYNC - Video Detect No Synch Failure.\n");
        break;

    case SDVR_DIAG_DECODER_AUDDET_TEST_NO_SYNC_ERR:
        printf("Diagnostics Test DECODER_AUDDET_TEST_NO_SYNC - Audio Detect No Synch Failure.\n");
        break;

    case SDVR_DIAG_DECODER_UNIQUE_VIDEO_TEST:
        printf("Diagnostics Test DECODER_UNIQUE_VIDEO_TEST - None unique video inputs Failure.\n");
        break;

    case SDVR_DIAG_NVP1114_REG_TEST:
        printf("Diagnostics Test NVP1114_REG_TEST - Nextchip register test Failure.\n");
        break;

    case SDVR_DIAG_DECODER_AUDIO_TEST_TIMEOUT:
        printf("Diagnostics Test DECODER_AUDIO_TEST_TIMEOUT - Audio Detect Timeout Failure.\n");
        break;

    case SDVR_DIAG_PCIE_EYEMASK_TEST_NO_CBB:
        printf("Diagnostics Test SDVR_DIAG_PCIE_EYEMASK_TEST_NO_CBB - PCIe did not detect the CBB test board.\n");
        break;

    case SDVR_DIAG_PCIE_EYEMASK_TEST_ERR:
        printf("Diagnostics Test DVR_DIAG_PCIE_EYEMASK_TEST_ERR - PCIe Eyemask Test Failure.\n");
        break;

    case SDVR_DIAG_PCIE_EYEMASK_TEST_TIMEOUT:
        printf("Diagnostics Test SDVR_DIAG_PCIE_EYEMASK_TEST_TIMEOUT  - PCIe Eyemask Test Timeout.\n");
        break;

    default:
        printf("Unknown Diagnostics Test [%d] Failure.\n",diag_code);

        break;
    }
    
} // void print_result(int pe_no, sdvr_diag_code_e diag_code)

void usage()
{
    printf("Usage:\n\tsdvr_diag <board_index> <diag_fw>\n\n");
    printf("where: <board_index> is the nth PCIe DVR board installed.\n");
    printf("       0: To run diagnostics on all the DVR boards.\n");
    printf("       1 - %d: To run diagnostics on a specific DVR board. \n",MAX_DVR_BOARDS);
    printf("\n");
    printf("       <diag_fw> The path to the romable diagnostics firmware.\n");
}

void print_pci_info(sx_uint32 board_index)
{
    sdvr_err_e err;
    sdvr_pci_attrib_t pci_attrib;

    err = sdvr_get_pci_attrib(board_index, &pci_attrib);
    if (err != SDVR_ERR_NONE)
    {
        printf("Application Error: Failed to get PCIe information. [Error = %s]\n",
            sdvr_get_error_text(err));
        return;
    }

    printf("\n=====================================================\n");
    printf("PCIe information for DVR board #%d.\n\n",board_index + 1);
    printf("\tPCI slot #:\t\t%d\n",pci_attrib.pci_slot_num);
    printf("\tBoard type:\t\t0x%x\n",pci_attrib.board_type);
    printf("\tVendor ID:\t\t0x%x\n",pci_attrib.vendor_id);
    printf("\tDevice ID:\t\t0x%x\n",pci_attrib.device_id);
    printf("\tSub-system vendor:\t0x%x\n",pci_attrib.subsystem_vendor);
    printf("\tSub-system ID:\t\t0x%x\n",pci_attrib.subsystem_id);
    printf("\tSerial #:\t\t%s\n",pci_attrib.serial_number);
    printf("\n=====================================================\n");
}

