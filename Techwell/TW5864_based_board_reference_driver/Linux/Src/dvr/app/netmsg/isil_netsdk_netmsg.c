#include <stdio.h>
#include <sys/stat.h>
#include <strings.h>
#include <string.h>


#include "isil_netsdk_netmsg.h"
#include "isil_netsdk_cfgfile.h"
#include "isil_media_config.h"
#include "isil_parsemessage.h"
//#include "isil_fm_play.h"
//#include "isil_fm_msg.h"
#include "isil_msg_reg_cb.h"
#include "isil_channel_map_cfg.h"
#include "isil_endian_trans.h"
#include "isil_netsdk_activecfg.h"
#include "isil_channel_map.h"
#include "isil_channel_map_cfg.h"
#include "default_cfg_value.h"
#include "isil_chan_chip_map.h"

//static net_msg_reg_func glb_local_net_msg_reg_t;

//net_msg_reg_func *get_local_gui_rec_reg_t( void )
//{
//    return &glb_local_net_msg_reg_t;
//}
static U32 u32AlarmType[CHANNEL_NUMBER];
#define TEST_BUFFER_SIZE (512+32)


static void debug_mem(int len ,char *buff)
{
	int i;

	printf(" \n");
	for( i = 0 ; i < len ; i++ ){

		printf(" %x " ,buff[i]);

	}

	printf(" \n");
}

static int ISILNetsdk_GetDevInfo(void* buff)
{
    LPNET_DEVICEINFO devinfo;
    int ret = 0;
    if(NULL == buff)
        return -1;
    /*从配置文件中读取*/
    devinfo = (LPNET_DEVICEINFO)buff;
    ret = ISIL_GetDeviceInfo(devinfo);
    /*通道个数暂先这样定*/
    #ifndef ISIL_USE_SYS_CFG_FILE
    devinfo->byChanNum = get_media_max_chan_number();
    #else
    devinfo->byChanNum = ISIL_GetSysMaxChnCnt();
    #endif
    return ret;
}

static int ISILNetsdk_GetSysAttr(void* buff)
{
    if(NULL == buff)
        return -1;
    LPISIL_DEV_SYSTEM_ATTR_CFG cfg;
    int ret;

    cfg = (LPISIL_DEV_SYSTEM_ATTR_CFG)buff;

    ret = ISIL_GetSysAttrCfg(cfg);
    return ret;
}

static int ISILNetsdk_SetSysAttr(void* buff)
{
    if(NULL == buff)
        return -1;

    LPISIL_DEV_SYSTEM_ATTR_CFG cfg;
    int ret = -1;

    cfg = (LPISIL_DEV_SYSTEM_ATTR_CFG)buff;

    ret = ISIL_SetSysAttrCfg(cfg);
    return ret;
}

static int ISILNetsdk_GetNetCfg(void* buff)
{
    if(NULL == buff)
        return -1;
    ISIL_DEV_NET_CFG* cfg;
    int ret;
    cfg = (ISIL_DEV_NET_CFG*)buff;

    ret = ISIL_GetNetCfg(cfg);
    return ret;
}

static int ISILNetsdk_SetNetCfg(void* buff)
{
    if(NULL == buff)
        return -1;
    ISIL_DEV_NET_CFG* cfg;
    int ret;
    cfg = (ISIL_DEV_NET_CFG*)buff;

    ret = ISIL_SetNetCfg(cfg);
    return ret;
}



static int ISILNetsdk_GetChannelCfg(int channel, void* buff)
{
    if(NULL == buff)
        return -1;
    LPISIL_DEV_CHANNEL_CFG cfg;
    S32 ret;
    cfg = (LPISIL_DEV_CHANNEL_CFG)buff;

    ret = ISIL_GetChannelCfg(channel, cfg);
    return ret;
}

static int ISILNetsdk_SetChannelCfg(int channel, void* buff)
{
    if(NULL == buff)
        return -1;
    LPISIL_DEV_CHANNEL_CFG cfg;
    S32 ret;
    cfg = (LPISIL_DEV_CHANNEL_CFG)buff;
    printf("%s, ch name %s, IF inter %d\n", __FUNCTION__,  cfg->szChannelName,
        cfg->stMainVideoEncOpt[0].bIFrameInterval);
    //ISIL_LOCAL_TransChanelCfg(cfg);
    ret = ISIL_SetChannelCfg(channel, cfg);
    return ret;
}


static int ISILNetsdk_GetRecCfg(int channel, void* buff)
{
    if(NULL == buff)
        return -1;
    LPISIL_RECORD_CFG cfg;
    S32 ret;
    cfg = (LPISIL_RECORD_CFG)buff;
    ret = ISIL_GetRecordCfg(channel, cfg);
    return ret;
}

static int ISILNetsdk_SetRecCfg(int channel, void* buff)
{
    if(NULL == buff)
        return -1;
    LPISIL_RECORD_CFG cfg;
    S32 ret;
    cfg = (LPISIL_RECORD_CFG)buff;
    ret = ISIL_SetRecordCfg(channel, buff);
    return ret;
}


static int ISILNetsdk_GetComCfg(void* buff)
{
    if(NULL == buff)
        return -1;
    ISIL_DEV_COMM_CFG* cfg;
    S32 ret;
    cfg = (ISIL_DEV_COMM_CFG*)buff;
    ret = ISIL_GetNetSDKComCfg(cfg);
    return ret;
}

static int ISILNetsdk_SetComCfg(void* buff)
{
    if(NULL == buff)
        return -1;
    ISIL_DEV_COMM_CFG* cfg;
    S32 ret;
    cfg = (ISIL_DEV_COMM_CFG*)buff;
    ret = ISIL_SetNetSDKComCfg(cfg);
    return ret;
}


static int ISILNetsdk_GetAlarmSchedule(void* buff)
{
    if(NULL == buff)
        return -1;
    ISIL_DEV_ALARM_SCHEDULE* cfg;
    cfg = (ISIL_DEV_ALARM_SCHEDULE*)buff;
    /*TODO获取alarm schedule配置*/
    return 0;
}
static int ISILNetsdk_GetNetTime(void* buff)
{
    if(NULL == buff)
        return -1;
    LPNET_TIME cfg;
    cfg = (LPNET_TIME)buff;
    /*TODO获取net time*/
    return 0;
}

static int ISILNetsdk_GetPreviewCfg(void* buff)
{
    if(NULL == buff)
        return -1;
    ISIL_DEV_PREVIEW_CFG* cfg;
    cfg = (ISIL_DEV_PREVIEW_CFG*)buff;
    /*TODO获取preview cfg配置*/
    return 0;
}

static int ISILNetsdk_GetAutoCfg(void* buff)
{
    if(NULL == buff)
        return -1;
    ISIL_DEV_AUTOMT_CFG* cfg;
    cfg = (ISIL_DEV_AUTOMT_CFG*)buff;
    /*TODO获取preview cfg配置*/
    return 0;
}

/*获取抓图配置*/
static int ISILNetsdk_GetSnapPicCfg(int channel, void* buff)
{
    if(NULL == buff)
        return -1;
    int ret;
    //LPSNAP_PARAMS snapcfg;

    //snapcfg = (LPSNAP_PARAMS)buff;
    ret = ISIL_GetSnapPictureCfg(channel, buff);
    return ret;
}

static int ISILNetsdk_SetSnapPicCfg(int channel, void* buff)
{
    if(NULL == buff)
        return -1;
    int ret;
    //LPSNAP_PARAMS snapcfg;

    //snapcfg = (LPSNAP_PARAMS)buff;
    ret = ISIL_SetSnapPictureCfg(channel, buff);
    return ret;
}


static int ISILNetsdk_GetVideoCoverCfg(int channel, void* buff)
{
    if(NULL == buff)
        return -1;
    ISIL_DEV_VIDEOCOVER_CFG* cfg;
    cfg = (ISIL_DEV_VIDEOCOVER_CFG*)buff;
    /*TODO获取通道遮挡配置*/
    return 0;
}

static int ISILNetsdk_GetVENC_H264_INFO(int channel, void* buff)
{
    if(NULL == buff)
        return -1;

    VENC_H264_INFO* cfg;
    cfg = (VENC_H264_INFO*)buff;
    return 0;
}

static int ISILNetsdk_RecCfg(LOC_GUI_RECORD_CTL* rec, S32 iConnectfd)
{
    int i;
    int ret;
    LOC_GUI_RECORD_CTL* tmp;
    MSG_FM_STOP_REC stStopRec;
    MSG_FM_START_REC stStartRec;

    tmp = rec;
    net_msg_reg_func *net_msg_reg_ptr = get_glb_net_msg_reg_t();

    if( !net_msg_reg_ptr->start_record_file_cb  || !net_msg_reg_ptr->stop_record_file_cb ){
    	printf("%s:start_record_file_cb  stop_record_file_cb null\n", __FUNCTION__);
        return ISIL_FAIL;
    }

    printf("%s:start find record channel\n", __FUNCTION__);
    #ifndef ISIL_USE_SYS_CFG_FILE
    for(i = 0; i < CHANNEL_NUMBER; i++)
    #else
    for(i = 0; i < ISIL_GetSysMaxChnCnt(); i++)
    #endif
	{
		if(tmp->ucAudio|| tmp->ucMainStream
				|| tmp->ucMotionJPEG || tmp->ucSubStream)
		{
			printf("chanel %d start rec\n", i);
			memset(&stStartRec, 0x00, sizeof(MSG_FM_START_REC));
			stStartRec.lChannel = i;
			stStartRec.ucRecType = 0x4;//REC_LOG_TYPE_MANUAL_REC;
			if(tmp->ucAudio == 1)
				stStartRec.u8Stream = REC_ONLY_AUDIO;
			if (tmp->ucMainStream == 1)
				stStartRec.u8Stream = stStartRec.u8Stream | REC_MAIN_VIDEO;
			if (tmp->ucMotionJPEG == 1)
				stStartRec.u8Stream = stStartRec.u8Stream | REC_ONLY_MJPG;
			if (tmp->ucSubStream == 1)
				stStartRec.u8Stream = stStartRec.u8Stream | REC_ONLY_MJPG;

			ret = net_msg_reg_ptr->start_record_file_cb( (void *)&stStartRec);
			if(ret < 0){
				fprintf(stderr,"start rec file err .\n");
				return ISIL_FAIL;
			}
		}
		else/*stop*/
		{
			printf("chanel %d stop rec\n", i);
			stStopRec.lChannel = i;
			ret = net_msg_reg_ptr->stop_record_file_cb( stStopRec.lChannel );
			if(ret < 0){
				fprintf(stderr,"stop record file  err .\n");
				return ISIL_FAIL;
			}
		}
		tmp++;

	}
    return 0;
}

static int ISILNetsdk_SetChnMap(LOC_GUI_CHN_MAP* map)
{
    int i;
    int ret;
    LOC_GUI_CHN_MAP* tmp;

    char tmpbuff[sizeof(CODEC_CHIP_CHLOGIC2PHY_MAP) + CHANNEL_NUMBER*sizeof(CHAN_MAP_T)];
    CODEC_CHIP_CHLOGIC2PHY_MAP *MapTbl_p;
    CHAN_MAP_T *pchanMapTbl,*tmpchanMapTbl;

    memset(tmpbuff,0x00,sizeof(tmpbuff));

    if(NULL == map)
        return -1;
    tmp = map;

    MapTbl_p = (CODEC_CHIP_CHLOGIC2PHY_MAP *)tmpbuff;
    /*设置CHN MAP*/
    MapTbl_p->e_bind_type = LOGIC_MAP_TABLE_BIND_H264E;
    #ifndef ISIL_USE_SYS_CFG_FILE
    MapTbl_p->i_chan_number = CHANNEL_NUMBER;
    #else
    MapTbl_p->i_chan_number = ISIL_GetSysMaxChnCnt();
    #endif
    pchanMapTbl = tmpchanMapTbl = MapTbl_p->map_table;
    #ifndef ISIL_USE_SYS_CFG_FILE
    for(i = 0; i < CHANNEL_NUMBER; i++)
    #else
    for(i = 0; i < ISIL_GetSysMaxChnCnt(); i++)
    #endif
    {
       pchanMapTbl->logic_slot_id = tmp->logicChID;
       pchanMapTbl->phy_slot_id = tmp->phyChipID;
       pchanMapTbl++;
       tmp++;
    }
    ret = ISIL_CODEC_CTL_SetChLogic2PhyMap(tmp->phyChipID,
                MapTbl_p);

    /*save config*/
    ISIL_SetLocGuiChnMapCfg(-1, map);
    return ret;
}

static S32 ISILNetsdk_GetChnMap(void* map)
{
    int ret;
    //CODEC_CHIP_CHLOGIC2PHY_MAP MapTbl;
    //int pMapTbl[CHANNEL_NUMBER];
    if(NULL == map)
        return -1;

    ret = ISIL_GetLocGuiChnMapCfg(-1, map);
    /*save config*/
    return ret;
}


static int ISILNetsdk_SetVENC_H264_INFO(int channel, void* buff)
{
    if(NULL == buff)
        return -1;
    VENC_H264_INFO* cfg;
    cfg = (VENC_H264_INFO*)buff;
    return 0;
}


static int ISIL_ClientLogin(S08 *data,S32 iConnectfd)
{
    ISIL_NET_IE_ASK_S *ieheader;
    S08 *ieheader_payload;

    ISIL_ACK_DATA_S ackdata;
    S32 ret = -1;
    S32 datasize = 0;
    NET_DEVICEINFO devinfo;

    if(NULL == data)
    {
        return -1;
    }

    ieheader = (ISIL_NET_IE_ASK_S*)data;
    memset(&ackdata, 0x00, sizeof(ISIL_ACK_DATA_S));

    ieheader = (ISIL_NET_IE_ASK_S*)(ieheader);
    ieheader_payload = (S08*)(ieheader + 1);

    datasize = sizeof(NET_DEVICEINFO);
    memset(&devinfo, 0, datasize);
    ret = ISILNetsdk_GetDevInfo(&devinfo);

    if(ret >= 0)
        ISIL_BuildAckData(&ackdata, &devinfo, datasize,
                IE_TYPE_ACK_LOGIN, 0);
    else
        ISIL_BuildAckData(&ackdata, NULL, 0,
                IE_TYPE_ACK_LOGIN, -1);
    ISIL_SendAckData(iConnectfd, &ackdata, 1, NULL, 0, ISIL_LOCAL_MSG);
    return 0;
}

static int ISIL_GetDevCfg(S08 *data,S32 iConnectfd)
{
    ISIL_NET_IE_ASK_S *ieheader;
    S08 *ieheader_payload;
    S08 *pTmp = NULL;
    U08* buff = NULL;
    ISIL_ACK_DATA_S ackdata;
    S32 ret = -1;
    S32 channel;
    S32 datasize = 0;


    ieheader = (ISIL_NET_IE_ASK_S*)data;
    memset(&ackdata, 0x00,sizeof(ISIL_ACK_DATA_S));

    ieheader = (ISIL_NET_IE_ASK_S*)(ieheader);
    ieheader_payload = (S08*)(ieheader + 1);

#ifdef LOCAL_GUI_ON_BOARD
    channel = (ieheader->lReserve);

    printf("%s %d, ie type 0x%x, ch %x\n", __FUNCTION__, __LINE__, (ieheader->nAskType),
        (ieheader->lReserve));
    switch((ieheader->nAskType))
#else
    channel = _swab32(ieheader->lReserve);

    printf("%s %d, ie type 0x%x, ch %x, len %d\n", __FUNCTION__, __LINE__,
    	_swab16(ieheader->nAskType),
        _swab32(ieheader->lReserve),
        _swab16(ieheader->nAskLength));
    switch(_swab16(ieheader->nAskType))
#endif
    {
        case ISIL_DEV_DEVICECFG:
        {
            ISIL_DEV_SYSTEM_ATTR_CFG attrcfg;
            datasize = sizeof(ISIL_DEV_SYSTEM_ATTR_CFG);
            memset(&attrcfg, 0, datasize);

            ret = ISILNetsdk_GetSysAttr(&attrcfg);
            ISIL_LOCAL_TransSysAttrCfg(&attrcfg);
            pTmp = (S08*)&attrcfg;

        }
        break;
        case ISIL_DEV_NETCFG:
        {
            ISIL_DEV_NET_CFG netcfg;
            datasize = sizeof(ISIL_DEV_NET_CFG);
            memset(&netcfg, 0, datasize);
            ret = ISILNetsdk_GetNetCfg(&netcfg);
            pTmp = (S08*)&netcfg;
        }
        break;

        case ISIL_DEV_CHANNELCFG:
        {
            #ifndef ISIL_USE_SYS_CFG_FILE
            if(get_media_max_chan_number() == 0) 
            #else
            if(ISIL_GetSysMaxChnCnt() == 0)
            #endif
            {
                ret = -1;
                break;
            }
            ISIL_DEV_CHANNEL_CFG channelcfg[CHANNEL_NUMBER];
            datasize = sizeof(channelcfg);
            memset(channelcfg, 0, datasize);
            ret = ISILNetsdk_GetChannelCfg(channel, channelcfg);
            ISIL_LOCAL_TransChanelCfg((ISIL_DEV_CHANNEL_CFG *)&channelcfg);
            pTmp = (S08*)channelcfg;
        }
        break;

        case ISIL_DEV_RECORDCFG:
        {
            ISIL_DEV_RECORD_CFG reccfg;
            datasize = sizeof(ISIL_DEV_RECORD_CFG);
            memset(&reccfg, 0, datasize);
            ret = ISILNetsdk_GetRecCfg(channel, &reccfg);
            pTmp = (S08*)&reccfg;
        }
        break;
        case ISIL_DEV_COMMCFG:
        {
            ISIL_DEV_COMM_CFG comcfg;
            datasize = sizeof(ISIL_DEV_COMM_CFG);
            memset(&comcfg, 0, datasize);
            ret = ISILNetsdk_GetComCfg(&comcfg);
            pTmp = (S08*)&comcfg;
        }
        break;

        case ISIL_DEV_ALARMCFG:
        {
            ISIL_DEV_ALARM_SCHEDULE alarsch;
            datasize = sizeof(ISIL_DEV_ALARM_SCHEDULE);
            memset(&alarsch, 0, datasize);
            ret = ISILNetsdk_GetAlarmSchedule(&alarsch);
            pTmp = (S08*)&alarsch;
        }
        break;

        case ISIL_DEV_TIMECFG:
        {
            NET_TIME nettime;
            datasize = sizeof(NET_TIME);
            memset(&nettime, 0, datasize);
            ret = ISILNetsdk_GetAlarmSchedule(&nettime);
            pTmp = (S08*)&nettime;
        }
        break;

        case ISIL_DEV_PREVIEWCFG:
        {
            ISIL_DEV_PREVIEW_CFG preview;
            datasize = sizeof(ISIL_DEV_PREVIEW_CFG);
            memset(&preview, 0, datasize);
            ret = ISILNetsdk_GetPreviewCfg(&preview);
            pTmp = (S08*)&preview;
        }
        break;

        case ISIL_DEV_AUTOMTCFG:
        {
            ISIL_DEV_AUTOMT_CFG autocfg;
            datasize = sizeof(ISIL_DEV_AUTOMT_CFG);
            memset(&autocfg, 0, datasize);
            ret = ISILNetsdk_GetPreviewCfg(&autocfg);
            pTmp = (S08*)&autocfg;
        }
        break;

        case ISIL_DEV_VEDIO_MARTIX:
            break;
        case ISIL_DEV_MULTI_DDNS:
            break;
        case ISIL_DEV_SNAP_CFG_INFO:
            /*设置抓图相关信息*/
            break;
        case ISIL_DEV_WEB_URL_CFG:
            break;
        case ISIL_DEV_FTP_PROTO_CFG_INFO:
            break;
        case ISIL_DEV_INTERVIDEO_CFG:
            break;
        case ISIL_DEV_VIDEO_COVER:
        {
            ISIL_DEV_VIDEOCOVER_CFG videocover;
            datasize = sizeof(ISIL_DEV_VIDEOCOVER_CFG);
            memset(&videocover, 0, datasize);
            ret = ISILNetsdk_GetVideoCoverCfg(channel, &videocover);
            pTmp = (S08*)&videocover;
        }
        break;

        case ISIL_DEV_TRANS_STRATEGY:
            break;
        case ISIL_DEV_DOWNLOAD_STRATEGY:
            break;
        case ISIL_DEV_WATERMAKE_CFG_INFO:
            break;

        case ISIL_DEV_WLAN_CFG:
            break;
        case ISIL_DEV_WLAN_DEVICE_CFG:
            break;
        case ISIL_DEV_REGISTER_CFG:
            break;
        case ISIL_DEV_CAMERA_CFG_INFO:
            break;
        case ISIL_DEV_INFRARED_CFG:
            break;
        case ISIL_DEV_MAIL_CFG_INFO:
            break;
        case ISIL_DEV_DNS_CFG_INFO:
            break;
        case ISIL_DEV_NTP_CFG_INFO:
            break;
        case ISIL_DEV_SNIFFER_CFG_INFO:
            break;
        case ISIL_DEV_AUDIO_DETECT_CFG:
            break;
        case ISIL_DEV_STORAGE_STATION_CFG:
            break;
        case ISIL_DEV_DST_CFG_INFO:
            break;
        case ISIL_DEV_VIDEO_OSD_CFG:
            break;
        case ISIL_DEV_ALARM_CENTER_CFG:
            break;
        case ISIL_DEV_CDMAGPRS_CFG_INFO:
            break;
        case ISIL_DEV_IPFILTER_CFG:
            break;
        case ISIL_DEV_TALK_ENCODE_CFG_INFO:
            break;
        case ISIL_DEV_RECORD_PACKET_CFG_INFO:
            break;
        case ISIL_DEV_MMS_CFG_INFO:
            break;
        case ISIL_DEV_SMSACTIVATION_CFG_INFO:
            break;
        case ISIL_DEV_SNIFFER_CFG_EX:
            break;
        case ISIL_DEV_DOWNLOAD_RATE_CFG:
            break;
        case ISIL_DEV_PANORAMA_SWITCH_CFG:
            break;
        case ISIL_DEV_LOST_FOCUS_CFG:
            break;
        case ISIL_DEV_ALARM_DECODE_CFG:
            break;
        case ISIL_DEV_VIDEOOUT_CFG:
            break;
        case ISIL_DEV_POINT_CFG_INFO:
            break;
        case ISIL_DEV_IP_COLLISION_CFG:
            break;
        case ISIL_DEV_OSD_ENABLE_CFG_INFO:
            break;
        case ISIL_DEV_LOCALALARM_CFG:
            break;
        case ISIL_DEV_NETALARM_CFG:
            break;
        case ISIL_DEV_MOTIONALARM_CFG:
            break;
        case ISIL_DEV_VIDEOLOSTALARM_CFG:
            break;
        case ISIL_DEV_BLINDALARM_CFG:
            break;
        case ISIL_DEV_DISKALARM_CFG:
            break;
        case ISIL_DEV_NETBROKENALARM_CFG:
            break;
        case ISIL_DEV_ENCODER_CFG:
            break;

        /*for mediasdk*/
        case ISIL_ENC_H264_CFG:
        {
            VENC_H264_INFO info[CHANNEL_NUMBER+1];
            #ifndef ISIL_USE_SYS_CFG_FILE
            datasize = sizeof(info);
            #else
            datasize = sizeof(VENC_H264_INFO)*(ISIL_GetSysMaxChnCnt()+1);
            #endif
            memset(info, 0, datasize);

            ret = ISIL_GetMediaSdkEncode(channel, info);
            pTmp = (S08*)info;
        }
        break;
        case ISIL_ENC_MJPG_CFG:
        {
            VENC_MJPG_INFO info[CHANNEL_NUMBER+1];
            #ifndef ISIL_USE_SYS_CFG_FILE
            datasize = sizeof(info);
            #else
            datasize = sizeof(VENC_MJPG_INFO)*(ISIL_GetSysMaxChnCnt()+1);
            #endif
            memset(info, 0, datasize);

            ret = ISIL_GetMediaSdkMjpeg(channel, info);
            pTmp = (S08*)info;
            fprintf(stderr, "---------datasize: %d, ret: %d------------", datasize, ret);
        }
        break;
        case ISIL_ENC_AUDIO_CFG:
        {
            VENC_AUDIO_INFO info[CHANNEL_NUMBER+1];

			#ifndef ISIL_USE_SYS_CFG_FILE
			datasize = sizeof(info);
			#else
			datasize = sizeof(VENC_AUDIO_INFO)*(ISIL_GetSysMaxChnCnt()+1);
			#endif
			memset(info, 0, datasize);

            ret = ISIL_GetMediaSdkAudio(channel, &info);
            pTmp = (S08*)&info;
            fprintf(stderr, "--------get aud cfg-datasize: %d, ret: %d------------", datasize, ret);
        }
        break;
        case ISIL_ENC_NOREALTIME_CFG:
        {
            ENC_CTL_VD_INFO info;
            datasize = sizeof(info);
            memset(&info, 0, datasize);
            ret = ISIL_GetMediaSdkNoRT(&info);
            pTmp = (S08*)&info;
        }
        break;
        case ISIL_VI_ANALOG_CFG:
        {
            VI_ANALOG_INFO info[CHANNEL_NUMBER+1];
            printf("get media  cfg\n");
            #ifndef ISIL_USE_SYS_CFG_FILE
            datasize = sizeof(info);
            #else
            datasize = sizeof(VI_ANALOG_INFO)*(ISIL_GetSysMaxChnCnt()+1);
            #endif
            memset(info, 0, datasize);

            ret = ISIL_GetMediaSdkAD(channel, info);
            pTmp = (S08*)info;
        }
        break;
        case ISIL_MANUAL_REC:
        {
            //LOC_GUI_RECORD_CTL* rec;

        }
        break;
        case ISIL_CHN_MAP:
        {
            //ISIL_CODEC_CTL_GetChLogic2PhyMap(unsigned int u32ChipID,
                //CODEC_CHIP_CHLOGIC2PHY_MAP* pMapTbl);
            LOC_GUI_CHN_MAP ch_map[CHANNEL_NUMBER];
            ret = ISILNetsdk_GetChnMap(ch_map);
            pTmp = (S08*)ch_map;
            datasize = sizeof(ch_map);
        }
        break;
        case ISIL_DIAPLAY_PORT_MODE_CFG:
        {
            //ISIL_DIAPLAY_PORT_MODE
        }
        break;
        case ISIL_TEST_RW:{
        	S32 * tmp;
			S32 sdsize;
			tmp = (int*)ieheader_payload;
			S08 readbuff[TEST_BUFFER_SIZE];

			printf("isil test read, type %d...\n", *tmp);

			memset(readbuff, 0x00, TEST_BUFFER_SIZE);
			ret = ISIL_NetSDK_TestR(channel, (U32* )&sdsize, (void*)ieheader_payload, (void*)readbuff);
			pTmp = readbuff;
			datasize = sdsize;
		}
       break;
        case ISIL_ALARM_TYPE_CFG:
		{
			U32 tmp[CHANNEL_NUMBER];
			memset(u32AlarmType, 0x00, sizeof(u32AlarmType));
			ret = ISIL_GetLocGuiAlarmTypeCfg((void*)u32AlarmType);
			if(channel < 0){
				int i;
				pTmp = (S08*)tmp;
				datasize = sizeof(u32AlarmType);//CHANNEL_NUMBER*sizeof(int);
				printf("get alarm size %d, alarm type:\n", datasize);
                #ifndef ISIL_USE_SYS_CFG_FILE
				for(i = 0; i < CHANNEL_NUMBER; i++)
                #else
                for(i = 0; i < ISIL_GetSysMaxChnCnt(); i++)
                #endif
				{
					printf("%x ", u32AlarmType[i]);
					tmp[i] = u32AlarmType[i];
					ISIL_TransAlarmType(&tmp[i]);
				}
				printf("\n");
			}
			else{
				printf("get a channel alarm type\n");
				tmp[channel] = u32AlarmType[channel];
				ISIL_TransAlarmType(&tmp[channel]);
				pTmp = (S08 *)&tmp[channel];
				datasize = sizeof(int);
			}
		}
        break;
        case ISIL_OSDINFO_CFG:
		{
            ISIL_OSD_CFG osdcfg[CHANNEL_NUMBER+1];
			#ifndef ISIL_USE_SYS_CFG_FILE
			datasize = sizeof(osdcfg);
			#else
			datasize = sizeof(ISIL_OSD_CFG)*(ISIL_GetSysMaxChnCnt()+1);
			#endif
	        memset(osdcfg, 0x00, datasize);

            ret = ISIL_GetMediaSdkOsd(channel, (void*)osdcfg);           
            TransMediaSdkOsdByCh(channel, (void*)osdcfg);

            pTmp = (S08*)osdcfg;
            fprintf(stderr, "--------get aud cfg-datasize: %d, ret: %d------------", datasize, ret);
#if 0
            if(channel < 0) {
                datasize = sizeof(osdcfg);
            }
            else{
                pTmp = (S08*)&osdcfg[channel];
                datasize = sizeof(ISIL_OSD_CFG);
            }
#endif
            
		}
		break;
        case ISIL_CHIP_CHN_MAP:{
            //add chip count and channel config
            SYS_CHIPS_CHAN_PRI_MAP_T mapcfg;
            SYS_CHIPS_CHAN_PRI_MAP_T *ptr_map_cfg;
            memset(&mapcfg, 0x00, sizeof(SYS_CHIPS_CHAN_PRI_MAP_T));
            /*Get chip channel map cfg from system config */
            ptr_map_cfg = get_glb_sys_chips_chan_pri_map();
            memcpy( &mapcfg ,ptr_map_cfg ,sizeof(SYS_CHIPS_CHAN_PRI_MAP_T) );
            
            pTmp = (S08*)&mapcfg;
            datasize = sizeof(SYS_CHIPS_CHAN_PRI_MAP_T);

            ISIL_TransChipsChanPriMap(&mapcfg);
        }
        break;
		
		default:
		break;
    }
    if(ret >= 0)
        ISIL_BuildAckData(&ackdata, pTmp, datasize,
#ifdef LOCAL_GUI_ON_BOARD
                (ieheader->nAskType),
#else
                _swab16(ieheader->nAskType),
#endif
                0);
    else
        ISIL_BuildAckData(&ackdata, NULL, 0,
#ifdef LOCAL_GUI_ON_BOARD
            (ieheader->nAskType),
#else
            _swab16(ieheader->nAskType),
#endif
                -1);

    ISIL_SendAckData(iConnectfd, &ackdata, 1, NULL, 0, ISIL_LOCAL_MSG);

    return 0;

}

static U32  VIDEO_SIZE_FROM_WIDTH_HEIGHT(U32 width, U32 height, enum ISIL_VIDEO_STANDARD standard)
{
    //if(standard == ISIL_VIDEO_STANDARD_PAL) {
    fprintf(stderr, "%s w %d h %d\n", __FUNCTION__, width, height);
    if((width == WIDTH_FRAME_QCIF_PAL) && (height == HEIGHT_FRAME_QCIF_PAL)) {
        return ISIL_VIDEO_SIZE_QCIF;
    }
    if((width == WIDTH_FRAME_QCIF_PAL) && (height == HEIGHT_FRAME_QCIF_PAL)) {
        return ISIL_VIDEO_SIZE_QHALF_D1;
    }
    if((width == WIDTH_FRAME_CIF_PAL) && (height == HEIGHT_FRAME_CIF_PAL)) {
        return ISIL_VIDEO_SIZE_CIF;
    }
    if((width == (WIDTH_FRAME_D1_PAL >> 1)) && (height == HEIGHT_FRAME_D1_PAL >> 1)) {
        return ISIL_VIDEO_SIZE_CIF;
    }
    if((width == WIDTH_FRAME_HALF_D1_PAL) && (height == HEIGHT_FRAME_HALF_D1_PAL)) {
        return ISIL_VIDEO_SIZE_HALF_D1;
    }
    if((width == WIDTH_FRAME_D1_PAL) && (height == HEIGHT_FRAME_D1_PAL)) {
        return ISIL_VIDEO_SIZE_D1;
    }
    if((width == WIDTH_FRAME_HCIF_PAL) && (height == HEIGHT_FRAME_HCIF_PAL)) {
        return ISIL_VIDEO_SIZE_HCIF;
    }
    if((width == (WIDTH_FRAME_D1_PAL >> 1)) && (height == HEIGHT_FRAME_D1_PAL >> 2)) {
        return ISIL_VIDEO_SIZE_HCIF;
    }
    if((width == WIDTH_FRAME_2CIF_PAL) && (height == HEIGHT_FRAME_2CIF_PAL)) {
        return ISIL_VIDEO_SIZE_2CIF;
    }
    if((width == WIDTH_FRAME_4CIF_PAL) && (height == HEIGHT_FRAME_4CIF_PAL)) {
        return ISIL_VIDEO_SIZE_4CIF;
    }
    if((width == WIDTH_FRAME_VGA) && (height == HEIGHT_FRAME_VGA)) {
        return ISIL_VIDEO_SIZE_VGA;
    }
    if((width == WIDTH_FRAME_SVGA) && (height == HEIGHT_FRAME_VGA)) {
        return ISIL_VIDEO_SIZE_SVGA;
    }
    if((width == WIDTH_FRAME_XGA) && (height == HEIGHT_FRAME_XGA)) {
        return ISIL_VIDEO_SIZE_XGA;
    }
    if((width == WIDTH_FRAME_720P) && (height == HEIGHT_FRAME_720P)) {
        return ISIL_VIDEO_SIZE_720P;
    }
    if((width == WIDTH_FRAME_1080P) && ((height == HEIGHT_FRAME_1080P) || (height == ((HEIGHT_FRAME_1080P >> 4) << 4)))) {
        return ISIL_VIDEO_SIZE_1080P;
    }
    //}else if(standard == ISIL_VIDEO_STANDARD_NTSC){
    if((width == WIDTH_FRAME_QCIF_NTSC) && (height == HEIGHT_FRAME_QCIF_NTSC)) {
        return ISIL_VIDEO_SIZE_QCIF;
    }
    if((width == WIDTH_FRAME_QCIF_NTSC) && (height == HEIGHT_FRAME_QCIF_NTSC)) {
        return ISIL_VIDEO_SIZE_QHALF_D1;
    }
    if((width == WIDTH_FRAME_CIF_NTSC) && (height == HEIGHT_FRAME_CIF_NTSC)) {
        return ISIL_VIDEO_SIZE_CIF;
    }
    if((width == (WIDTH_FRAME_D1_NTSC >> 1)) && (height == HEIGHT_FRAME_D1_NTSC >> 1)) {
        return ISIL_VIDEO_SIZE_CIF;
    }
    if((width == WIDTH_FRAME_HALF_D1_NTSC) && (height == HEIGHT_FRAME_HALF_D1_NTSC)) {
        return ISIL_VIDEO_SIZE_HALF_D1;
    }
    if((width == WIDTH_FRAME_D1_NTSC) && (height == HEIGHT_FRAME_D1_NTSC)) {
        return ISIL_VIDEO_SIZE_D1;
    }
    if((width == WIDTH_FRAME_HCIF_NTSC) && (height == HEIGHT_FRAME_HCIF_NTSC)) {
        return ISIL_VIDEO_SIZE_HCIF;
    }
    if((width == (WIDTH_FRAME_D1_NTSC >> 1)) && (height == HEIGHT_FRAME_D1_NTSC >> 2)) {
        return ISIL_VIDEO_SIZE_HCIF;
    }
    if((width == WIDTH_FRAME_2CIF_NTSC) && (height == HEIGHT_FRAME_2CIF_NTSC)) {
        return ISIL_VIDEO_SIZE_2CIF;
    }
    if((width == WIDTH_FRAME_4CIF_NTSC) && (height == HEIGHT_FRAME_4CIF_NTSC)) {
        return ISIL_VIDEO_SIZE_4CIF;
    }
    if((width == WIDTH_FRAME_VGA) && (height == HEIGHT_FRAME_VGA)) {
        return ISIL_VIDEO_SIZE_VGA;
    }
    if((width == WIDTH_FRAME_SVGA) && (height == HEIGHT_FRAME_VGA)) {
        return ISIL_VIDEO_SIZE_SVGA;
    }
    if((width == WIDTH_FRAME_XGA) && (height == HEIGHT_FRAME_XGA)) {
        return ISIL_VIDEO_SIZE_XGA;
    }
    if((width == WIDTH_FRAME_720P) && (height == HEIGHT_FRAME_720P)) {
        return ISIL_VIDEO_SIZE_720P;
    }
    if((width == WIDTH_FRAME_1080P) && ((height == HEIGHT_FRAME_1080P) || (height == ((HEIGHT_FRAME_1080P >> 4) << 4)))) {
        return ISIL_VIDEO_SIZE_1080P;
    }
    //}else{
    return ISIL_VIDEO_SIZE_USER;
    //}

    return ISIL_VIDEO_SIZE_USER;
}

static S32 ISIL_LocalGui_SetVD(S32 channel,void *buff)
{
    S32 iRet = 0, i = 0, chip = 0;//i:gui chan, j: chip, x:chan per chip
    VENC_H264_INFO* cfg;
    VD_CONFIG_REALTIME Realtime;
    S32 sdcnt, hdcnt, allcnt;
    U32 chip_nums;
    S32 gui_max_ch;
    char *tmp_buff = NULL;
    int tmp_buff_len = 0;

    if(buff == NULL)
    {
        return ISIL_FAIL;
    }

	chip_nums = isil_get_chips_count();
	if(chip_nums == 0) {
		fprintf(stderr, "isil_get_chips_count fail\n");
		return -1;
	}

	gui_max_ch = ISIL_GetSysMaxChnCnt();

	fprintf(stderr, "----gui totle chan: %d------\n", gui_max_ch);

    //convert data to Realtime struct
	tmp_buff_len = gui_max_ch*sizeof(VENC_H264_INFO);
	tmp_buff = malloc(tmp_buff_len);
	memcpy(tmp_buff, buff, tmp_buff_len);
    cfg = (VENC_H264_INFO*)tmp_buff;


    if(channel < 0){//all channel together

		fprintf(stderr, "----gui set all channel together------\n");

		for(chip = 0; chip < chip_nums; chip++){

			if(!gui_max_ch){
				break;
			}

		    memset(&Realtime, 0x00, sizeof(VD_CONFIG_REALTIME));
		    for(i = 0; i < sizeof(Realtime.chan)/sizeof(int); i++){
		    	Realtime.chan[i] = i;
		    }

		    ISIL_MediaSDK_GetVideoStandard(chip, &Realtime.video_std);
		    fprintf(stderr, "----Realtime.video_std: %d------\n", Realtime.video_std);

		    Realtime.drop_frame = 0;

			#ifdef ISIL_USE_SYS_CFG_FILE
			allcnt = ISIL_GetChnCntPerChip(&sdcnt, &hdcnt, chip);
			if(allcnt < 0)
			{
				continue;
			}
			#endif

			gui_max_ch -= hdcnt;

			for(i = 0; i < sdcnt; i++)
			{
				//little endian or big endian
				ISIL_LOCAL_Trans_VENC_H264_INFO(cfg);

				//convert data to realtime  struct
				Realtime.chan[i] = i;
				Realtime.video_size[i] = VIDEO_SIZE_FROM_WIDTH_HEIGHT(cfg->venc_h264cfg[0].i_logic_video_width_mb_size << 4, \
						cfg->venc_h264cfg[0].i_logic_video_height_mb_size << 4, Realtime.video_std);
				Realtime.frame_rate[i] = (cfg->venc_h264cfg[0].i_fps > cfg->venc_h264cfg[1].i_fps)? \
									   cfg->venc_h264cfg[0].i_fps : cfg->venc_h264cfg[1].i_fps;
				Realtime.interleave[i] = ISIL_FRAME_MODE_INTERLACE;

				cfg++;
				gui_max_ch--;

				fprintf(stderr, "---gui_max_ch: %d, chip: %d, i: %d, sdcnt: %d--------\n", gui_max_ch, chip, i, sdcnt);
			}

			cfg += hdcnt;

			iRet = ISIL_MediaSDK_ClacSetVd(chip, &Realtime);
			if( iRet < 0 ) {
				fprintf( stderr,"ISIL_MediaSDK_ClacSetVd err .\n");
			    free(tmp_buff);
				return -1;
			}
		}
	}else{//single channel
		fprintf(stderr, "----gui set single channel,not support!!!------\n");
		//ISIL_LOCAL_Trans_VENC_H264_INFO(cfg);
	    free(tmp_buff);
		return ISIL_FAIL;
	}

    free(tmp_buff);
    return ISIL_SUCCESS;
}
static S32 ISIL_NetSDK_SetVD()
{
    S32 ret;
    S32 i=0 ,j;
    S32 sdcnt, hdcnt, allcnt;
    VENC_H264_INFO info[CHANNEL_NUMBER+1];
              
    VD_CONFIG_REALTIME Realtime;
    U32 chip_nums;

    chip_nums = isil_get_chips_count();
    if(chip_nums == 0) {
        fprintf(stderr, "isil_get_chips_count fail\n"); 
        return -1;
    }

    memset(&Realtime, 0x00, sizeof(VD_CONFIG_REALTIME));
    memset(info, 0, sizeof(info));

   ret = ISIL_GetMediaSdkEncode(-1, info);

    Realtime.video_std = 0;
    Realtime.drop_frame = 0;

    for( j = 0 ; j < chip_nums ; j++) {
    
        
        #ifdef ISIL_USE_SYS_CFG_FILE
        
        allcnt = ISIL_GetChnCntPerChip((unsigned int *)&sdcnt, (unsigned int *)&hdcnt, j);
        if(allcnt < 0)
        {
            continue;
        }
        #endif
        
        for(i = 0; i < PER_CHIP_MAX_CHAN_NUM; i++)
        {
            Realtime.chan[i] = i;
            #ifdef ISIL_USE_SYS_CFG_FILE
            if(i < sdcnt) 
            #endif
            {
                Realtime.frame_rate[i] = 
                    (info[j*PER_CHIP_MAX_CHAN_NUM+i].venc_h264cfg[0].i_fps\
                     > info[j*PER_CHIP_MAX_CHAN_NUM+i].venc_h264cfg[1].i_fps)?\
                    info[j*PER_CHIP_MAX_CHAN_NUM+i].venc_h264cfg[0].i_fps : \
                    info[j*PER_CHIP_MAX_CHAN_NUM+i].venc_h264cfg[1].i_fps;
                    
                    Realtime.video_size[i] = 
                        VIDEO_SIZE_FROM_WIDTH_HEIGHT(
                            info[j*PER_CHIP_MAX_CHAN_NUM+i].venc_h264cfg[0].i_logic_video_width_mb_size << 4,
                            info[j*PER_CHIP_MAX_CHAN_NUM+i].venc_h264cfg[0].i_logic_video_height_mb_size << 4, 1);
                    fprintf(stderr, "%s chn %d frame rate %d, size %d....\n", __FUNCTION__, 
                            i, Realtime.frame_rate[i], Realtime.video_size[i]);
            }
            
        }
        
        ret = ISIL_MediaSDK_ClacSetVd(j, &Realtime);
        if( ret < 0 ) {
            fprintf( stderr,"ISIL_MediaSDK_ClacSetVd err .\n");
            return -1;
        }
    }
    
    return 0;
}

S32 ISIL_NetSdkSetVDByOpenChn(S32 chn)
{
    S32 ret;
    S32 i=0, j;
    U32 chip_nums;
    S32 sdcnt, hdcnt, allcnt;
    VENC_H264_INFO info[CHANNEL_NUMBER+1];
              
    VD_CONFIG_REALTIME Realtime;
    CHIP_ENC_STATUS_INF chip_enc_status;

    chip_nums = isil_get_chips_count();
    if(chip_nums == 0) {
        fprintf(stderr, "isil_get_chips_count fail\n"); 
        return -1;
    }
    ret = ISIL_GetMediaSdkEncode(-1, info);

    memset(&chip_enc_status, 0x00, sizeof(CHIP_ENC_STATUS_INF));
    memset(&Realtime, 0x00, sizeof(VD_CONFIG_REALTIME));
    memset(info, 0, sizeof(info));
    Realtime.video_std = 0;
    Realtime.drop_frame = 0;

    
    for(j = 0; j < chip_nums; j++) {
        ret = get_enc_chan_status_by_chip(j, &chip_enc_status);
        if(ret < 0){
            fprintf(stderr, "get channel status fail\n");
            return ret;
        }
        #ifdef ISIL_USE_SYS_CFG_FILE
        allcnt = ISIL_GetChnCntPerChip((unsigned *)&sdcnt, (unsigned *)&hdcnt, (unsigned)j);
        if(allcnt < 0)
        {
            continue;
        }
        #endif
        for(i = 0; i < PER_CHIP_MAX_CHAN_NUM; i++)
        {
           
            Realtime.chan[i] = i;
            #ifdef ISIL_USE_SYS_CFG_FILE
            if(i < sdcnt) 
            #endif
            {
                if(chip_enc_status.h264_main_stream_status_arr[i] == 1 || 
                   chip_enc_status.h264_sub_stream_status_arr[i] == 1 ||
                   i == chn%PER_CHIP_MAX_CHAN_NUM)
                {    
                    Realtime.frame_rate[i] = 
                        (info[j*PER_CHIP_MAX_CHAN_NUM+i].venc_h264cfg[0].i_fps\
                         > info[j*PER_CHIP_MAX_CHAN_NUM+i].venc_h264cfg[1].i_fps)?\
                        info[j*PER_CHIP_MAX_CHAN_NUM+i].venc_h264cfg[0].i_fps : \
                        info[j*PER_CHIP_MAX_CHAN_NUM+i].venc_h264cfg[1].i_fps;
                    Realtime.interleave[i] = 0;
                }
            }
        }
        ret = ISIL_MediaSDK_ClacSetVd(j, &Realtime);
        if( ret < 0 ) {
            fprintf( stderr,"ISIL_MediaSDK_ClacSetVd err .\n");
            return -1;
        }
    }
    
    return 0;
}



static int ISIL_SetDevCfg(S08 *data,S32 iConnectfd)
{
    ISIL_NET_IE_ASK_S *ieheader;
    S08 *ieheader_payload;
    S08 *pTmp = NULL;
    ISIL_ACK_DATA_S ackdata;
    S32 ret = -1;
    U32 channel;
    S32 datasize = 0;

    ieheader = (ISIL_NET_IE_ASK_S*)data;
    memset(&ackdata, 0x00,sizeof(ISIL_ACK_DATA_S));

    ieheader = (ISIL_NET_IE_ASK_S*)(ieheader);
    ieheader_payload = (S08*)(ieheader + 1);

#ifdef LOCAL_GUI_ON_BOARD
        channel = (ieheader->lReserve);

        printf("%s %d, ie type 0x%x, ch %x, len %d.\n", __FUNCTION__, __LINE__, (ieheader->nAskType),
            (ieheader->lReserve), (ieheader->nAskLength));
        switch((ieheader->nAskType))
#else
        channel = _swab32(ieheader->lReserve);

        printf("%s %d, ie type 0x%x, ch %x, len %d\n", __FUNCTION__, __LINE__,
            _swab16(ieheader->nAskType),
            _swab32(ieheader->lReserve),
            _swab16(ieheader->nAskLength));
        switch(_swab16(ieheader->nAskType))
#endif

    {
        case ISIL_DEV_DEVICECFG:
        {
            LPISIL_DEV_SYSTEM_ATTR_CFG cfg;
            cfg = (LPISIL_DEV_SYSTEM_ATTR_CFG)ieheader_payload;
            printf("%s ISIL_DEV_DEVICECFG, in %d, out %d\n", __FUNCTION__,
                cfg->byAlarmInNum, cfg->byAlarmOutNum);
            ret = ISILNetsdk_SetSysAttr((S08*)ieheader_payload);
        }
        break;
        case ISIL_DEV_NETCFG:
        {
            ISIL_DEV_NET_CFG* netcfg;
            netcfg = (ISIL_DEV_NET_CFG*)ieheader_payload;
            ret = ISILNetsdk_SetNetCfg(netcfg);
        }
        break;

        case ISIL_DEV_CHANNELCFG:
        {
            LPISIL_DEV_CHANNEL_CFG channelcfg;
            channelcfg = (LPISIL_DEV_CHANNEL_CFG)ieheader_payload;
            ret = ISILNetsdk_SetChannelCfg(channel, channelcfg);
        }
        break;

        case ISIL_DEV_RECORDCFG:
        {
            ISIL_DEV_RECORD_CFG* reccfg;
            reccfg = (ISIL_DEV_RECORD_CFG*)ieheader_payload;
            ret = ISILNetsdk_SetRecCfg(channel, reccfg);
        }
        break;
        case ISIL_DEV_COMMCFG:
        {
            ISIL_DEV_COMM_CFG* comcfg;
            comcfg = (ISIL_DEV_COMM_CFG*)ieheader_payload;
            ret = ISILNetsdk_SetComCfg(comcfg);
        }
        break;

        case ISIL_DEV_ALARMCFG:
        {
            ISIL_DEV_ALARM_SCHEDULE* alarsch;
            alarsch = (ISIL_DEV_ALARM_SCHEDULE*)ieheader_payload;
            //ret = ISILNetsdk_SetAlarmSchedule(&alarsch);
        }
        break;

        case ISIL_DEV_TIMECFG:
        {
            NET_TIME* nettime;
            nettime = (NET_TIME*)ieheader_payload;
            //ret = ISILNetsdk_setAlarmSchedule(&nettime);
        }
        break;

        case ISIL_DEV_PREVIEWCFG:
        {
            ISIL_DEV_PREVIEW_CFG* preview;

            //ret = ISILNetsdk_GetPreviewCfg(&preview);
        }
        break;

        case ISIL_DEV_AUTOMTCFG:
        {
            ISIL_DEV_AUTOMT_CFG* autocfg;
            //ret = ISILNetsdk_GetPreviewCfg(&autocfg);
        }
        break;

        case ISIL_DEV_VEDIO_MARTIX:
            break;
        case ISIL_DEV_MULTI_DDNS:
            break;
        case ISIL_DEV_SNAP_CFG_INFO:
            break;
        case ISIL_DEV_WEB_URL_CFG:
            break;
        case ISIL_DEV_FTP_PROTO_CFG_INFO:
            break;
        case ISIL_DEV_INTERVIDEO_CFG:
            break;
        case ISIL_DEV_VIDEO_COVER:
        {
            ISIL_DEV_VIDEOCOVER_CFG* videocover;
            //ret = ISILNetsdk_GetVideoCoverCfg(channel, &videocover);
        }
        break;

        case ISIL_DEV_TRANS_STRATEGY:
            break;
        case ISIL_DEV_DOWNLOAD_STRATEGY:
            break;
        case ISIL_DEV_WATERMAKE_CFG_INFO:
            break;

        case ISIL_DEV_WLAN_CFG:
            break;
        case ISIL_DEV_WLAN_DEVICE_CFG:
            break;
        case ISIL_DEV_REGISTER_CFG:
            break;
        case ISIL_DEV_CAMERA_CFG_INFO:
            break;
        case ISIL_DEV_INFRARED_CFG:
            break;
        case ISIL_DEV_MAIL_CFG_INFO:
            break;
        case ISIL_DEV_DNS_CFG_INFO:
            break;
        case ISIL_DEV_NTP_CFG_INFO:
            break;
        case ISIL_DEV_SNIFFER_CFG_INFO:
            break;
        case ISIL_DEV_AUDIO_DETECT_CFG:
            break;
        case ISIL_DEV_STORAGE_STATION_CFG:
            break;
        case ISIL_DEV_DST_CFG_INFO:
            break;
        case ISIL_DEV_VIDEO_OSD_CFG:
            break;
        case ISIL_DEV_ALARM_CENTER_CFG:
            break;
        case ISIL_DEV_CDMAGPRS_CFG_INFO:
            break;
        case ISIL_DEV_IPFILTER_CFG:
            break;
        case ISIL_DEV_TALK_ENCODE_CFG_INFO:
            break;
        case ISIL_DEV_RECORD_PACKET_CFG_INFO:
            break;
        case ISIL_DEV_MMS_CFG_INFO:
            break;
        case ISIL_DEV_SMSACTIVATION_CFG_INFO:
            break;
        case ISIL_DEV_SNIFFER_CFG_EX:
            break;
        case ISIL_DEV_DOWNLOAD_RATE_CFG:
            break;
        case ISIL_DEV_PANORAMA_SWITCH_CFG:
            break;
        case ISIL_DEV_LOST_FOCUS_CFG:
            break;
        case ISIL_DEV_ALARM_DECODE_CFG:
            break;
        case ISIL_DEV_VIDEOOUT_CFG:
            break;
        case ISIL_DEV_POINT_CFG_INFO:
            break;
        case ISIL_DEV_IP_COLLISION_CFG:
            break;
        case ISIL_DEV_OSD_ENABLE_CFG_INFO:
            break;
        case ISIL_DEV_LOCALALARM_CFG:
            break;
        case ISIL_DEV_NETALARM_CFG:
            break;
        case ISIL_DEV_MOTIONALARM_CFG:
            break;
        case ISIL_DEV_VIDEOLOSTALARM_CFG:
            break;
        case ISIL_DEV_BLINDALARM_CFG:
            break;
        case ISIL_DEV_DISKALARM_CFG:
            break;
        case ISIL_DEV_NETBROKENALARM_CFG:
            break;
        case ISIL_DEV_ENCODER_CFG:
            break;
        case ISIL_ENC_H264_CFG:
        {
            VENC_H264_INFO *info;
            info = (VENC_H264_INFO*)ieheader_payload;
            printf("set media sdk enc cfg: stream type %d, w %d h%d\n", info->streamType,
            		info->venc_h264cfg[0].i_logic_video_width_mb_size,
            		info->venc_h264cfg[0].i_logic_video_height_mb_size);
            ret = ISIL_LocalGui_SetVD(channel, info);
            if(ret < 0){
            	printf("------ISIL_LocalGui_SetVD fail----\n");
            }else{
				ret = ISIL_SetMediaSdkEncode(channel, info);
				if(ret < 0){
					printf("------ISIL_SetMediaSdkEncode fail----\n");
				}
            }
        }
        break;
        case ISIL_ENC_MJPG_CFG:
        {
            VENC_MJPG_INFO* info;
            info = (VENC_MJPG_INFO*)ieheader_payload;
            //printf("set mjpg: 0x%x\n", info->venc_mjpg_cfg[0].i_capture_type);
            printf("set media sdk mjpeg cfg: stream type %d, cap type: 0x%x\n",
                                                info->streamType,
                                                info->venc_mjpg_cfg[0].i_capture_type);
            ret = ISIL_SetMediaSdkMjpeg(channel, info);
        }
        break;
        case ISIL_ENC_AUDIO_CFG:
        {
            VENC_AUDIO_INFO* info;
            info = (VENC_AUDIO_INFO*)ieheader_payload;
            printf("set media sdk audio cfg: stream type %d\n", info->streamType);
            ret = ISIL_SetMediaSdkAudio(channel, info);
        }
        break;

        case ISIL_ENC_NOREALTIME_CFG:
        {
            ENC_CTL_VD_INFO* info;
            info = (ENC_CTL_VD_INFO*)ieheader_payload;
            printf("set media sdk not RT cfg\n");
            ret = ISIL_SetMediaSdkNoRT(channel, info);
        }
        break;
        //ISIL_CODEC_ENC_SetOSDCfg();
        case ISIL_VI_ANALOG_CFG:
        {
            VI_ANALOG_INFO* info;
            info = (VI_ANALOG_INFO*)ieheader_payload;
            //printf("set media  cfg；\n");
            ret = ISIL_SetMediaSdkAD(channel, info);
        }
        break;

         case ISIL_MANUAL_REC:
        {
            LOC_GUI_RECORD_CTL* rec;
            printf("local gui record\n");
            rec = (LOC_GUI_RECORD_CTL*)ieheader_payload;
            ret = ISILNetsdk_RecCfg(rec, iConnectfd);
        }
        break;
        case ISIL_CHN_MAP:
        {
            LOC_GUI_CHN_MAP* info;
            info = (LOC_GUI_CHN_MAP*)ieheader_payload;
            ret = ISILNetsdk_SetChnMap(info);
        }
        break;
        case ISIL_SET_SUB_WIN:
        {
            ISIL_SUB_WINDOWS *win;
            win = (ISIL_SUB_WINDOWS*)ieheader_payload;
            ISIL_TransSubWindows(win);
            ret = 0;//ISIL_NetSDK_SetSubWindows(win);
        }
        break;
        case ISIL_DIAPLAY_PORT_MODE_CFG:
        {
            ISIL_DIAPLAY_PORT_MODE* portmod;
            portmod = (ISIL_DIAPLAY_PORT_MODE*)ieheader_payload;
            ISIL_TransDisplayPortMode(portmod);
            printf("%s set display port mode p%d %d %d\n",__FUNCTION__,
                portmod->i_port, portmod->e_mode, portmod->e_interleave);
            ret = ISIL_NetSDK_SetDisplayPortMode(portmod);

        }
        break;
        case ISIL_TEST_RW:{
        	//int * tmp;
        	printf("isil test write...\n");
        	//tmp = (int*)ieheader_payload;
        	ret = ISIL_NetSDK_TestW(channel, (void*)ieheader_payload);
        }
        break;

        case ISIL_ALARM_TYPE_CFG:
        {
        	int i;
        	printf("set alarm type....\n");
        	if(channel < 0){
                #ifndef ISIL_USE_SYS_CFG_FILE
        		memcpy((U08 *)u32AlarmType, (U08*)ieheader_payload, CHANNEL_NUMBER*sizeof(ISIL_ALARM_TYPE));
        		for(i = 0; i < CHANNEL_NUMBER; i++)
                #else
                memcpy((U08 *)u32AlarmType, (U08*)ieheader_payload, ISIL_GetSysMaxChnCnt()*sizeof(ISIL_ALARM_TYPE));
        		for(i = 0; i < ISIL_GetSysMaxChnCnt(); i++)
                #endif
                {
        			ISIL_TransAlarmType(&u32AlarmType[i]);
        		}
        	}
        	else{
        		printf("input alarm type %x\n", *((U32*)ieheader_payload));
        		u32AlarmType[channel] = *((U32*)ieheader_payload);
        		ISIL_TransAlarmType(&u32AlarmType[channel]);
        	}

        	ret = ISIL_SetLocGuiAlarmTypeCfg(channel, (void*)u32AlarmType);
        }
        break;
        case ISIL_OSDINFO_CFG:
		{
            ISIL_OSD_CFG* osdcfg;
            osdcfg = (ISIL_OSD_CFG*)ieheader_payload;
            TransMediaSdkOsdByCh(channel, osdcfg);
            ret = ISIL_SetMediaSdkOsd(channel, osdcfg);
		}
		break;
        default:
        break;
    }
    if(ret >= 0)
        ISIL_BuildAckData(&ackdata, pTmp, datasize,

#ifdef LOCAL_GUI_ON_BOARD
                (ieheader->nAskType),
#else
                _swab16(ieheader->nAskType),
#endif

                0);
    else
        ISIL_BuildAckData(&ackdata, NULL, 0,

#ifdef LOCAL_GUI_ON_BOARD
                (ieheader->nAskType),
#else
                _swab16(ieheader->nAskType),
#endif
                -1);
    ISIL_SendAckData(iConnectfd, &ackdata, 1, NULL, 0, ISIL_LOCAL_MSG);

    return 0;

}
static S32 ISIL_SetSnapPicture(S08 *data,S32 iConnectfd)
{
    ISIL_NET_IE_ASK_S *ieheader;
    S08 *ieheader_payload;
    ISIL_ACK_DATA_S ackdata;
    LPSNAP_PARAMS snap;
    U32 channel;
    S32 ret;


    ieheader = (ISIL_NET_IE_ASK_S*)data;
    memset(&ackdata, 0x00,sizeof(ISIL_ACK_DATA_S));

    ieheader = (ISIL_NET_IE_ASK_S*)(ieheader);
    ieheader_payload = (S08*)(ieheader + 1);
    channel = ieheader->lReserve;
    snap = (LPSNAP_PARAMS)ieheader_payload;
    printf("%s, %d %d %d %d %d\n", __FUNCTION__, snap->Channel,
        snap->ImageSize, snap->InterSnap,
        snap->mode, snap->Quality);
    /*启动snap picture*/
    ret = ISILNetsdk_SetSnapPicCfg(channel, snap);
    return ret;
}


static void ISIL_GetLocalPlayBackFlag(S32 asktype, S32* flag)
{
    switch(asktype)
    {
        case ISIL_GET_FILE_FRAME:
            *flag = IE_TYPE_FILE_GET_FRAMES;
        break;

        case ISIL_CLOSE_FILE:
            *flag = IE_TYPE_FILE_CLOSE;
        break;
        case ISIL_STOP_FILE_PLAY:
            *flag = ISIL_STOP_FILE_PLAY;
        break;
        default:
        break;
    }
    return;
}

static int ISIL_LocalFileOp(S08 *data,S32 iConnectfd, void* arg)
{
    ISIL_NET_IE_ASK_S *ieheader;
    S08 *ieheader_payload;
    S08 *pTmp = NULL;
    ISIL_ACK_DATA_S ackdata;
    S32 ret = -1;
    U32 channel;
    S32 datasize = 0;
    S32 asktype;

    ieheader = (ISIL_NET_IE_ASK_S*)data;
    memset(&ackdata, 0x00,sizeof(ISIL_ACK_DATA_S));

    ieheader = (ISIL_NET_IE_ASK_S*)(ieheader);
    ieheader_payload = (S08*)(ieheader + 1);

#ifdef LOCAL_GUI_ON_BOARD
        channel = (ieheader->lReserve);

        printf("%s %d, ie type 0x%x, ch %x\n", __FUNCTION__, __LINE__,
            (ieheader->nAskType),
            (ieheader->lReserve));
        //switch((ieheader->nAskType))
        asktype = ieheader->nAskType;
#else
        channel = _swab32(ieheader->lReserve);

        printf("%s %d, ie type 0x%x, ch %x\n", __FUNCTION__, __LINE__,
            _swab16(ieheader->nAskType),
            _swab32(ieheader->lReserve));

        //switch(_swab16(ieheader->nAskType))
        asktype = _swab16(ieheader->nAskType);
#endif
    switch(asktype)
    {
        case IE_TYPE_FILE_SEARCH:
        {
            ISIL_SEARCH_EVNET* pEvnt;
            pEvnt = (ISIL_SEARCH_EVNET *)ieheader_payload;
            printf("%s cmd %d\n", __FUNCTION__, pEvnt->cCommand);
            ISIL_NetSDK_ActiveFileSearch(iConnectfd, pEvnt);
            return 0;
        }
        break;
        case ISIL_OPEN_FILE:
        {
            ISIL_TransFileOpen((ISIL_NETFILE_OPEN*)ieheader_payload);
            ISIL_GetPlaybackFileInfo(iConnectfd, ISIL_LOCAL_MSG,
                (ISIL_NETFILE_OPEN*)ieheader_payload, arg);
            return 0;
        }
        break;
        case ISIL_CLOSE_FILE:
        case ISIL_GET_FILE_FRAME:
        case ISIL_STOP_FILE_PLAY:
       {
            S32 flag;
            ISIL_TransFileCmmd((NET_FILE_COMMAND *)ieheader_payload);
            ISIL_GetLocalPlayBackFlag(asktype, &flag);
            ret = ISIL_SetPlaybackAction(iConnectfd, flag, ISIL_LOCAL_MSG,
                    (NET_FILE_COMMAND*)ieheader_payload, channel);
            //ret = 0;
       }
       break;
       case ISIL_BIND_FILE_WINDOW:
       {
            ISIL_SUB_WINDOWS_BIND* cfg;
            cfg = (ISIL_SUB_WINDOWS_BIND *)ieheader_payload;

            ISIL_TransFileBindWin(cfg);
            /*
            printf("%s set bind chip %d vitype %d dispid %d vich %d  port %d phase %d subwinncnt %d subwx %d subwy %d\n", 
                   __FUNCTION__,
            		cfg->lChipId, cfg->emViType, cfg->lDisplayId, cfg->lVi_chId,
            		cfg->stSubInfo.iPort, cfg->stSubInfo.iPhase, cfg->stSubInfo.cSubWindowsNum,
                   cfg->stSubInfo.stSubWindows[0].iSubWin_x , cfg->stSubInfo.stSubWindows[0].iSubWin_y);
            */
            ret = ISIL_NetSDK_BindFileWindows(iConnectfd, cfg);

       }
       break;
        case ISIL_SET_FILE_PLAYMODE:/*设置快进快退*/
        {
            G_ISIL_PLAY_MOD cfg;
            VDEC_CH_MODE* plyMod;

            plyMod = (VDEC_CH_MODE *)ieheader_payload;
            fprintf(stderr, "%s set playmod, fd %d, direction %d continue %d key %d loop %d spd %d\n",
                __FUNCTION__, channel, plyMod->direction,plyMod->is_continue,
                plyMod->key_frame,plyMod->loop,(unsigned int)plyMod->speed.x_speed_value);
            cfg.fd = channel;
            memcpy(&cfg.mod, plyMod, sizeof(VDEC_CH_MODE));
            ret = ISIL_NetSDK_SetPlayMod(&cfg);
        }
        break;
        case ISIL_SET_FILE_AVSYNC:
        {
            G_FILE_AV_SYNC avsync;
            avsync.fd = channel;
            ret = ISIL_NetSDK_SetAVSync(&avsync);
        }
        break;

        default:
        break;
    }
    if(ret >= 0)
        ISIL_BuildAckData(&ackdata, pTmp, datasize,

#ifdef LOCAL_GUI_ON_BOARD
                (ieheader->nAskType),
#else
                _swab16(ieheader->nAskType),
#endif

                0);
    else
        ISIL_BuildAckData(&ackdata, NULL, 0,

#ifdef LOCAL_GUI_ON_BOARD
                (ieheader->nAskType),
#else
                _swab16(ieheader->nAskType),
#endif
                -1);
    ISIL_SendAckData(iConnectfd, &ackdata, 1, NULL, 0, ISIL_LOCAL_MSG);

    return 0;

}


S32  ISIL_ParseLocalNetMsg(U32 uiMsgType, S08 *data, S32 iLen, S32 iConnectfd)
{
    S32 ret = 0;
    printf("%s, msg type 0x%x\n", __FUNCTION__, uiMsgType);

    switch(uiMsgType)
    {
        case MSG_TYPE_ASK_LOGIN:
            ret = ISIL_ClientLogin(data, iConnectfd);
        break;

        case MSG_TYPE_GETCFG:
            ret = ISIL_GetDevCfg(data, iConnectfd);
        break;


        case MSG_TYPE_SETCFG:
            ret = ISIL_SetDevCfg(data, iConnectfd);
        break;

        case MSG_SNAP_PICTURE:
            ret = ISIL_SetSnapPicture(data, iConnectfd);
        break;

        case MSG_TYPE_FILE_ABOUT:
            ret = ISIL_LocalFileOp(data, iConnectfd, NULL);
            break;

        default:
        break;
    }
    return ret;

}

U32 ISIL_GetPhyChIDByLogicCh(U32 LocCH)
{
    return LocCH;
}


