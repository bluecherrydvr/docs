#ifndef _ISIL_CHAN_CHIP_MAP_H
#define _ISIL_CHAN_CHIP_MAP_H


#ifdef __cplusplus
extern "C" {
#endif

#include "isil_media_config.h"
#include "isil_channel_map.h"
#include "isil_ap_mediasdk.h"

#ifndef TEST_CHIP_CNT
#define TEST_CHIP_CNT 1
#endif



#ifndef MAX_ENC_NUM_PER_CHIP
#define MAX_ENC_NUM_PER_CHIP 16
#endif



typedef enum{
	V_CHAN_SIZE_IS_SD = 0,
	V_CHAN_SIZE_IS_HD ,
	V_CHAN_SIZE_BUTTON
}VIDEO_CHAN_SIZE_TYPE_E;


typedef struct _CHIP_CHANS_DEF_T{
	unsigned int sd_chan_cnts;
	unsigned int hd_chan_cnts;
}CHIP_CHANS_DEF_T;

typedef struct{
	unsigned int chip_id;  
	unsigned int valid_chan_nums;
	unsigned char logic_chanid_arr[MAX_ENC_NUM_PER_CHIP];
	unsigned char log_chan_v_size_arr[MAX_ENC_NUM_PER_CHIP];  //value is VIDEO_CHAN_SIZE_TYPE_E

}CHIP_CHAN_PRI_MAP_T;


typedef struct{
	unsigned int chip_nums;
	CHIP_CHAN_PRI_MAP_T chip_chan_pri_map_arr[TEST_CHIP_CNT];

}SYS_CHIPS_CHAN_PRI_MAP_T;


extern SYS_CHIPS_CHAN_PRI_MAP_T* get_glb_sys_chips_chan_pri_map( void );

extern int get_chip_chn_phy_id_by_logic_map(CHIP_CHN_PHY_ID* phy_id,unsigned int logic_id);

extern int get_video_size_by_logic_id(unsigned int log_id);

extern void init_glb_sys_chans_map( void );

extern int get_valid_chan_num_by_chip( unsigned int chip_id);

extern void debug_glb_sys_chans_map( void );


#ifdef __cplusplus
}
#endif //__cplusplus

#endif

