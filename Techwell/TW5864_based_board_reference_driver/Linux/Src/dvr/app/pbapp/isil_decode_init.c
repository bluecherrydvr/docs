#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "isil_decode_init.h"
#include "isil_fm_play.h"
#include "isil_media_config.h"
#include "isil_pb_mgt.h"
#include "isil_vo.h"
#include "isil_pb_process.h"


int isil_decode_init( void )
{
    int total_chips_num ,i ,ret ;
    CHIP_NODE_CFG *chip_node_cfg_ptr ;

    total_chips_num = isil_get_chips_count();
    if(total_chips_num <= 0 ) {
        return -1;
    }

    init_glb_pb_list();

    ISIL_FM_PLAY_InitAllPlayer();

    create_dec_netlink_ev();

#if 0
    chip_node_cfg_ptr = get_glb_chip_node_cfg( );

    if( chip_node_cfg_ptr ) {
        for( i = 0 ; i < total_chips_num ; i++) {

            if( !strlen(chip_node_cfg_ptr->vo_dir) ) {
                continue ;
            }

            ret = ISIL_VO_CTL_OpenChip(chip_node_cfg_ptr->vo_dir, i);
            if( ret < 0 ) {
                fprintf( stderr,"ISIL_VO_CTL_OpenChip err .\n");
                return -1;
            }

            chip_node_cfg_ptr++;
        }
    }

#endif
    return 0;
    
}


