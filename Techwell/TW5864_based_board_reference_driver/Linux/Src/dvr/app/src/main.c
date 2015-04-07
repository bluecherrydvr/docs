#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "isil_net_sdk.h"
#include "isil_sdk_main.h"
#include "net_parse_reg.h"
#include "isil_enc_sdk.h"
#include "isil_enc_data_mgt_cent.h"
#include "isil_parsemessage.h"
#include "isil_handle_win_msg.h"
#include "isil_udp.h"
#include "isil_file_record_session.h"
#include "isil_reg_func_cent.h"
#include "isil_decode_init.h"
#include "isil_sdk_center_mgt.h"
#include "net_thread_pool.h"
#include "isil_channel_map.h"
#include "isil_chan_chip_map.h"

int main(int argc , char *argv[])
{

    int ret ;
    fprintf(stderr, "start init confige file\n");
    ISIL_InitCfgFile();
    fprintf(stderr, "start init net sdk\n");
    isil_net_sdk_init();

    fprintf(stderr, "start init enc data reg arry\n");
    init_enc_data_reg_array();

    fprintf(stderr, "start init media sdk\n");
    ret = ISIL_MediaSDK_Init();
    if(ret < 0 ) {
        goto DVR_CLEANUP;
    }

    /* FILE record init*/
    //ret = file_record_init();
    

    /*reg net data and ctrl func*/

    fprintf(stderr, "start init glb reg func\n");
    glb_reg_func_init();
    fprintf(stderr, "start init dec\n");
    isil_decode_init();

    init_glb_sys_chans_map();

    ISIL_TestChnMap();
    fprintf(stderr, "start run media sdk\n");
    ret = ISIL_MediaSDK_Run();

    if(ret < 0 ) {
        goto DVR_CLEANUP;
    }

    //ISIL_SetDefaultVD4D1();
    //ISIL_SetDefaultVD16Cif();

    fprintf(stderr, "start net thread run\n");
     ret = isil_net_thread_run();
     if( ret < 0) {
         fprintf(stderr,"isil_net_sdk_run err .\n");
         goto DVR_CLEANUP;
     }






DVR_CLEANUP:

    isil_net_sdk_cleanup();

    sdk_thr_exit();

    ISIL_MediaSDK_Cleanup();

    net_release_glb_thread_pool();
    return 0;

}

