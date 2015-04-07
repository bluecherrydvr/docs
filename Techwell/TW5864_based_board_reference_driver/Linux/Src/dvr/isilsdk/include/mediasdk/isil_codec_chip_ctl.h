#ifndef _ISIL_CODEC_CHIP_CTL_H_

#define _ISIL_CODEC_CHIP_CTL_H_

#ifdef __cplusplus
extern "C" {
#endif 

#include "isil_codec_common.h"
#include "config.h"

#define ISIL_PHY_VD_CHAN_NUMBER (16)

//codec chip control 
//msg control
typedef struct
{
    unsigned int u32ChipID;//chip handle
    unsigned int u32Cmd;//control cmd
    unsigned int u32Blk;//block or nonblock operator
    unsigned int u32MsgLen;//msg length
    char         pMsg[0];//msg content
}CODEC_CHIP_MSG;

//codec chip version info
typedef struct ISIL_chip_id_info
{
    unsigned int i_vendor_id;
    unsigned int i_version_id;
}CODEC_CHIP_VERSION;

// codec chip tv standard
enum ISIL_VIDEO_STANDARD{
	ISIL_VIDEO_STANDARD_PAL,
	ISIL_VIDEO_STANDARD_NTSC,
	ISIL_VIDEO_STANDARD_USER_DEFINE,
};

#define ISIL_VIDEO_BUS_PARAM_ENABLE_CHANGE_VIDEO_STANDARD_MASK        (0x00000001)

typedef struct ISIL_chip_video_param{
     unsigned int change_mask_flag;
     enum ISIL_VIDEO_STANDARD e_video_standard;
}CODEC_CHIP_VIDEO_STD;


//vd parament
struct ISIL_vd_chan_param{
    unsigned int  	change_mask_flag;
    unsigned char 	i_phy_chan_id;
    unsigned char  	i_phy_chan_enable;
    unsigned char  	i_phy_chan_fps;
    unsigned char  	interleave;
    unsigned int  	hor_reverse;    /*Horizontal reverse*/
    unsigned int 	ver_reverse;    /*Vertical   reverse*/
    unsigned short 	i_phy_video_width_mb_size;
    unsigned short 	i_phy_video_height_mb_size;
};

typedef struct ISIL_chip_vd_param{
	unsigned int i_vd_chan_number;
	struct ISIL_vd_chan_param vd_param[ISIL_PHY_VD_CHAN_NUMBER];
}ENC_CTL_VD_CFG;


typedef enum{
     LOGIC_MAP_TABLE_BIND_H264E = 0, /*bind vi to h264e*/
     LOGIC_MAP_TABLE_BIND_H264D,    /*bind vo to h264d*/
     LOGIC_MAP_TABLE_BIND_AENC,    /*bind ai to aenc*/
     LOGIC_MAP_TABLE_BIND_ADEC,    /*bind ao to adec*/
     LOGIC_MAP_TABLE_BIND_JPEGE,    /*bind vi to jpege*/
     LOGIC_MAP_TABLE_BIND_RESERVED,
}LOGIC_MAP_TABLE_TYPE;

typedef struct {
#if defined(__LITTLE_ENDIAN_BITFIELD)
    unsigned int phy_slot_id:16;
	unsigned int logic_slot_id:16;
#else
	unsigned int logic_slot_id:16;
    unsigned int phy_slot_id:16;
#endif
}CHAN_MAP_T;

typedef struct{
    unsigned int i_chan_number;   /*map channel number*/
    LOGIC_MAP_TABLE_TYPE e_bind_type;
    CHAN_MAP_T map_table[0];
}CODEC_CHIP_CHLOGIC2PHY_MAP;

typedef struct{
	unsigned int i_vin_chan_nubmer;//encoding channel 
	unsigned int i_vin_ad_chan_number;
	unsigned int i_cascade_vin_ad_chan_number;
}ENC_VI_MAXCH;

typedef struct {
	unsigned int i_ain_chan_nubmer;
	unsigned int i_ain_ad_chan_number;
	unsigned int i_cascade_ain_ad_chan_number;
}ENC_AI_MAXCH;

enum ISIL_FRAME_MODE{
    ISIL_FRAME_MODE_INTERLACE = 0,
    ISIL_FRAME_MODE_PROGRESSIVE,
    ISIL_FRAME_MODE_RESERVED,
};


enum ISIL_VIDEO_SIZE{
	ISIL_VIDEO_SIZE_QCIF = 0,
	ISIL_VIDEO_SIZE_QHALF_D1,
	ISIL_VIDEO_SIZE_CIF,
	ISIL_VIDEO_SIZE_HALF_D1,
	ISIL_VIDEO_SIZE_D1,
	ISIL_VIDEO_SIZE_HCIF,
	ISIL_VIDEO_SIZE_2CIF,
	ISIL_VIDEO_SIZE_4CIF,
	ISIL_VIDEO_SIZE_VGA,
	ISIL_VIDEO_SIZE_SVGA,
	ISIL_VIDEO_SIZE_XGA,
	ISIL_VIDEO_SIZE_H960,
	ISIL_VIDEO_SIZE_720P,
	ISIL_VIDEO_SIZE_1080P,
	ISIL_VIDEO_SIZE_USER,
};

struct chan_map_info{
    unsigned int phy_slot_id;
    unsigned int map_logic_slot_id;
    unsigned int enable;
    unsigned int fps;
    unsigned int roundTableStride;
    int video_size;
    unsigned int logic_slot_discard_table;
    enum ISIL_FRAME_MODE interleave;
};

struct vd_map_param{
    struct chan_map_info map_info[ISIL_PHY_VD_CHAN_NUMBER];
};     

typedef struct vd_map_param VD_CONFIG_PARAM;
typedef VD_CONFIG_PARAM VP_CONFIG_PARAM;

//see bar_ctl.h
typedef struct{
	enum ISIL_VIDEO_STANDARD video_std;
	int drop_frame;//0:not drop frame,1:drop 
	int chan[ISIL_PHY_VD_CHAN_NUMBER];
	int video_size[ISIL_PHY_VD_CHAN_NUMBER];
	int frame_rate[ISIL_PHY_VD_CHAN_NUMBER];//PAL [0, 25], NTSC [0, 30];
	enum ISIL_FRAME_MODE interleave[ISIL_PHY_VD_CHAN_NUMBER];
}VD_CONFIG_REALTIME;


typedef VD_CONFIG_REALTIME VP_CONFIG;

//decoder av sync
enum sync_type {
	SYNC_METHOD_NORMAL = 0,
	SYNC_METHOD_AV,
	SYNC_METHOD_MULTI,
	SYNC_METHOD_USER_SPECIFY,
	SYNC_METHOD_INVALID = 0x10,   //max 16 different sync method
};
	
struct syn_item {
   unsigned short         type;     //video: 0;  audio: 1
   unsigned short         chan_id; //channel id (logic)
};

typedef struct  syn_arg {
	 enum sync_type         type;
	 unsigned short         nr;
	 unsigned short         idx_ref;
	 struct syn_item        items[0];
}DEC_AV_SYNC;

//set timestamp resync
typedef struct
{
	unsigned int u32Ts;//timestamp
}VDEC_CH_TSRESYNC;


typedef struct ISIL_chip_h264d_bind_info{
     unsigned int i_h264d_logic_id;
     unsigned int i_display_phy_id;
}BIND_DEC2VO;

typedef struct ISIL_chip_vi2vo_bind_info{
     unsigned int i_vi_phy_id;
     unsigned int i_display_phy_id;
}BIND_VI2VO;

/********************************API********************************/
/*codec chip control api*/
/***********************************************************
 function:
     ISIL_CODEC_CTL_OpenChip
 discription:
     open codec chip control device and get fd
 input:
         char *pChipNode: chip device node path
         unsigned int u32ChipID: chip id
 output: none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_EXIST: chip had been open
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/
extern int ISIL_CODEC_CTL_OpenChip(char *pChipNode, unsigned int u32ChipID);

/***********************************************************
 function:
     ISIL_CODEC_CTL_CloseChip
 discription:
     close chip and chip fd
 input:
     unsigned int u32ChipID: chip id
 output: none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST: chip had been close
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/

extern int ISIL_CODEC_CTL_CloseChip(unsigned int u32ChipID);


/***********************************************************
 function:
     ISIL_CODEC_CTL_GetChipHandle
 discription:
     get chip fd
 input:
     unsigned int u32ChipID: chip id
 output:
    CODEC_HANDLE* pChipHandle: get chip fd
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST: chip had been close or not open
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/
extern int ISIL_CODEC_CTL_GetChipHandle(unsigned int u32ChipID, CODEC_HANDLE* pChipHandle);

/***********************************************************
 function:
     ISIL_CODEC_CTL_SendMsg
 discription:
     get chip fd
 input:
     unsigned int u32ChipID: chip id
     CODEC_CHIP_MSG *pOp: control message
 output: none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST: chip had been close or not open
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/
extern int ISIL_CODEC_CTL_SendMsg(unsigned int u32ChipID, CODEC_CHIP_MSG *pMsg);

/***********************************************************
 function:
     ISIL_CODEC_CTL_GetChipVersion
 discription:
     get codec chip version and verdor info
 input:
     unsigned int u32ChipID: chip id
 output:
	CODEC_CHIP_VERSION *pVersion: pointor of version data
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST: chip had been close or not open
     CODEC_ERR_INVALID: input parament is invalid
**********************************************************/

extern int ISIL_CODEC_CTL_GetChipVersion(unsigned int u32ChipID, CODEC_CHIP_VERSION *pVersion);

/***********************************************************
 function:
     ISIL_CODEC_CTL_SetVideoSTD
 discription:
     set codec chip video standard
 input:
 	 unsigned int u32ChipID: chip id
	 CODEC_CHIP_VIDEO_STD* pVideoStd: video standard
 output:
	 none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST: chip had been close or not open
     CODEC_ERR_INVALID: input parament is invalid
**********************************************************/
extern int ISIL_CODEC_CTL_SetVideoSTD(unsigned int u32ChipID, CODEC_CHIP_VIDEO_STD* pVideoStd);

/***********************************************************
 function:
     ISIL_CODEC_CTL_GetVideoSTD
 discription:
     get codec chip video standard
 input:
 	 unsigned int u32ChipID: chip id
 output:
	 CODEC_CHIP_VIDEO_STD* pVideoStd: video standard
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST: chip had been close or not open
     CODEC_ERR_INVALID: input parament is invalid
**********************************************************/

extern int ISIL_CODEC_CTL_GetVideoSTD(unsigned int u32ChipID, CODEC_CHIP_VIDEO_STD* pVideoStd);

/***********************************************************
 function:
     ISIL_CODEC_CTL_SetChLogic2PhyMap
 discription:
     set logic channel to physical channel map table
 input:
 	 unsigned int u32ChipID: chip id
	 CODEC_CHIP_CHLOGIC2PHY_MAP* pMapTbl: map table
 output:
	 none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST: chip had been close or not open
     CODEC_ERR_INVALID: input parament is invalid
**********************************************************/
extern int ISIL_CODEC_CTL_SetChLogic2PhyMap(unsigned int u32ChipID, CODEC_CHIP_CHLOGIC2PHY_MAP* pMapTbl);

/***********************************************************
 function:
     ISIL_CODEC_CTL_SetChLogic2PhyMap
 discription:
     get logic channel to physical channel map table
 input:
 	 unsigned int u32ChipID: chip id
 output:
	 CODEC_CHIP_CHLOGIC2PHY_MAP* pMapTbl: map table
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST: chip had been close or not open
     CODEC_ERR_INVALID: input parament is invalid
**********************************************************/
extern int ISIL_CODEC_CTL_GetChLogic2PhyMap(unsigned int u32ChipID, CODEC_CHIP_CHLOGIC2PHY_MAP* pMapTbl);

/***********************************************************
 function:
     ISIL_CODEC_CTL_SetVdConfigParam
 discription:
     set vd config parament
 input:
 	 unsigned int u32ChipID: chip id
 	 VD_CONFIG_PARAM* pVdParam: vd parament data
 output:
     none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST: chip had been close or not open
     CODEC_ERR_INVALID: input parament is invalid
**********************************************************/
extern int ISIL_CODEC_CTL_SetVdConfigParam(unsigned int u32ChipID, VD_CONFIG_PARAM* pVdParam);

/***********************************************************
 function:
     ISIL_CODEC_CTL_GetVdConfigParam
 discription:
     Get vd config parament
 input:
 	 unsigned int u32ChipID: chip id
 output:
	 VD_CONFIG_PARAM* pVdParam: vd parament data
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST: chip had been close or not open
     CODEC_ERR_INVALID: input parament is invalid
**********************************************************/
extern int ISIL_CODEC_CTL_GetVdConfigParam(unsigned int u32ChipID, VD_CONFIG_PARAM* pVdParam);

/***********************************************************
 function:
     ISIL_CODEC_CTL_SetVpConfigParam
 discription:
     set vp config parament
 input:
 	 unsigned int u32ChipID: chip id
 	 VP_CONFIG_PARAM* pVdParam: vp parament data
 output:
     none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST: chip had been close or not open
     CODEC_ERR_INVALID: input parament is invalid
**********************************************************/
extern int ISIL_CODEC_CTL_SetVpConfigParam(unsigned int u32ChipID, VP_CONFIG_PARAM* pVpParam);

/***********************************************************
 function:
     ISIL_CODEC_CTL_GetVpConfigParam
 discription:
     Get vp config parament
 input:
 	 unsigned int u32ChipID: chip id
 output:
	 VP_CONFIG_PARAM* pVpParam: vp parament data
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST: chip had been close or not open
     CODEC_ERR_INVALID: input parament is invalid
**********************************************************/
extern int ISIL_CODEC_CTL_GetVpConfigParam(unsigned int u32ChipID, VP_CONFIG_PARAM* pVpParam);

//as usr config, calc and set new vd parament,open or close nonrealtime
extern int ISIL_CODEC_CTL_CalcSetVd(unsigned int u32ChipID, VD_CONFIG_REALTIME* pRealtime);

//as usr config, calc and set new vp parament
extern int ISIL_CODEC_CTL_CalcSetVp(unsigned int u32ChipID, VP_CONFIG* pVpcfg);



//only for encoder
/***********************************************************
 function:
     ISIL_ENC_CTL_SetVDCfg
 discription:
     only for encoder chip, set video bus font end config 
 input:
 	 unsigned int u32ChipID: chip id
	 ENC_CTL_VD_CFG* pVdCfg: encoder vd cfg
 output:
	 none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST: chip had been close or not open
     CODEC_ERR_INVALID: input parament is invalid
**********************************************************/
extern int ISIL_ENC_CTL_SetVDCfg(unsigned int u32ChipID, ENC_CTL_VD_CFG* pVdCfg);

/***********************************************************
 function:
     ISIL_ENC_CTL_GetVDCfg
 discription:
     only for encoder chip, get video bus font end config 
 input:
 	 unsigned int u32ChipID: chip id
 output:
	 ENC_CTL_VD_CFG* pVdCfg: encoder vd cfg
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST: chip had been close or not open
     CODEC_ERR_INVALID: input parament is invalid
**********************************************************/
extern int ISIL_ENC_CTL_GetVDCfg(unsigned int u32ChipID, ENC_CTL_VD_CFG* pVdCfg);

extern int ISIL_ENC_CTL_TrySetVDCfg(unsigned int u32ChipID, struct ISIL_vd_chan_param* pChanvdcfg);
extern int ISIL_ENC_CTL_TryGetVDCfg(unsigned int u32ChipID, struct ISIL_vd_chan_param* pChanvdcfg);


/***********************************************************
 function:
     ISIL_ENC_CTL_GetViChipMaxCh
 discription:
     get codec chip max vi channel
 input:
		unsigned int u32ChipID: chip id
 output:
         ENC_VI_MAXCH *pChipCh: pointer to chip max channel
 return:
     CODEC_ERR_OK: success
 	 CODEC_ERR_FAIL:fail
	 CODEC_ERR_NOT_EXIST: chip not open
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/
extern int ISIL_ENC_CTL_GetViChipMaxCh(unsigned int u32ChipID, ENC_VI_MAXCH *pChipCh);

/***********************************************************
 function:
     ISIL_ENC_CTL_GetAiChipMaxCh
 discription:
     get codec chip max ai channel
 input:
		unsigned int u32ChipID: chip id
 output:
         ENC_AI_MAXCH *pChipCh: pointer to chip max channel
 return:
     CODEC_ERR_OK: success
 	 CODEC_ERR_FAIL:fail
	 CODEC_ERR_NOT_EXIST: chip not open
     CODEC_ERR_INVALID: input parament is invalid
 **********************************************************/

extern int ISIL_ENC_CTL_GetAiChipMaxCh(unsigned int u32ChipID, ENC_AI_MAXCH *pAiCh);

//decoder av sync
//start
/***********************************************************
 function:
     ISIL_DEC_CTL_StartAVSync
 discription:
     when decoding, set av sync start
 input:
 	 unsigned int u32ChipID: chip id
	 DEC_AV_SYNC* pAVSync: av sync data
 output:
	 none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST: chip had been close or not open
     CODEC_ERR_INVALID: input parament is invalid
**********************************************************/

extern int ISIL_DEC_CTL_StartAVSync(unsigned int u32ChipID, DEC_AV_SYNC* pAVSync);


//stop
/***********************************************************
 function:
     ISIL_DEC_CTL_StopAVSync
 discription:
     when decoding, set av sync stop
 input:
 	 unsigned int u32ChipID: chip id
	 DEC_AV_SYNC* pAVSync: av sync data
 output:
	 none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST: chip had been close or not open
     CODEC_ERR_INVALID: input parament is invalid
**********************************************************/
extern int ISIL_DEC_CTL_StopAVSync(unsigned int u32ChipID, DEC_AV_SYNC* pAVSync);


/***********************************************************
 function:
     ISIL_DEC_CTL_TSResync
 discription:
     when decoding, resync timestamp again
 input:
 	 unsigned int u32ChipID: chip id
	 VDEC_CH_TSRESYNC* pTS: timestamp
 output:
	 none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST: chip had been close or not open
     CODEC_ERR_INVALID: input parament is invalid
**********************************************************/

extern int ISIL_DEC_CTL_TSResync(unsigned int u32ChipID, VDEC_CH_TSRESYNC* pTS);

/***********************************************************
 function:
     ISIL_DEC_CTL_BindDec2VO
 discription:
     bind decoder channel to display channel
 input:
 	 unsigned int u32ChipID: chip id
	 BIND_DEC2VO *pD2VO: channel data
 output:
	 none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST: chip had been close or not open
     CODEC_ERR_INVALID: input parament is invalid
**********************************************************/
extern int ISIL_DEC_CTL_BindDec2VO(unsigned int u32ChipID, BIND_DEC2VO *pD2VO);

/***********************************************************
 function:
     ISIL_DEC_CTL_UnbindDec2VO
 discription:
     unbind decoder channel to display channel
 input:
 	 unsigned int u32ChipID: chip id
	 BIND_DEC2VO *pD2VO: channel data
 output:
	 none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST: chip had been close or not open
     CODEC_ERR_INVALID: input parament is invalid
**********************************************************/
extern int ISIL_DEC_CTL_UnbindDec2VO(unsigned int u32ChipID, BIND_DEC2VO *pD2VO);


/***********************************************************
 function:
     ISIL_DEC_CTL_BindVI2VO
 discription:
     bind video input channel to display channel, that's preview, not througt encoder
 input:
 	 unsigned int u32ChipID: chip id
	 BIND_VI2VO *pVi2VO: channel data
 output:
	 none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST: chip had been close or not open
     CODEC_ERR_INVALID: input parament is invalid
**********************************************************/
extern int ISIL_DEC_CTL_BindVI2VO(unsigned int u32ChipID, BIND_VI2VO *pVi2VO);

/***********************************************************
 function:
     ISIL_DEC_CTL_UnbindVI2VO
 discription:
     unbind video input channel to display channel
 input:
 	 unsigned int u32ChipID: chip id
	 BIND_VI2VO *pVi2VO: channel data
 output:
	 none
 return:
     CODEC_ERR_OK: success
     CODEC_ERR_FAIL:fail
     CODEC_ERR_NOT_EXIST: chip had been close or not open
     CODEC_ERR_INVALID: input parament is invalid
**********************************************************/
extern int ISIL_DEC_CTL_UnbindVI2VO(unsigned int u32ChipID, BIND_VI2VO *pVi2VO);

#ifdef __cplusplus
}
#endif
#endif

