#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>


#include "isil_handle_win_msg.h"
#include "isil_type.h"
#include "isil_debug.h"
#include "isil_messagestruct.h"
#include "isil_parsemessage.h"
#include "ISIL_SDKError_Def.h"
#include "isil_ap_mediasdk.h"
#include "isil_enc_data_reg.h"
#include "isil_net_send_session.h"
#include "isil_netsdk_netmsg.h"
#include "isil_net_config.h"
#include "isil_channel_map.h"


int ISIL_ParseNetMessages(int readlen, void* buf, struct NET_TASK* task)
{

	S08 *data = NULL;
    S08 *pTemp = NULL;

	S32 iLen;
	ISIL_MSG_HERADE_S *sMsgHead;


    if( !readlen || !buf ) {
        return -1;
    }

	iLen = sizeof(sMsgHead);
	ISIL_TRACE(BUG_NORMAL, "[%s] fd %d", __FUNCTION__, task->fd);

    pTemp = buf;
    sMsgHead = (ISIL_MSG_HERADE_S*)pTemp;

    ISIL_TRACE(BUG_NORMAL, "%s: msg type: 0x%x", __FUNCTION__, sMsgHead->nMsgType);

    data = pTemp + sizeof(ISIL_MSG_HERADE_S);

    iLen = _swab32(sMsgHead->lMsgLen);
    switch(_swab16(sMsgHead->nMsgType))
    {
        case MSG_TYPE_ASK_REALPLAY:
        {
            return -1;
            //iRet = ISIL_ParseRealPlay(data, iConnectFd, arg);
        }
        break;
        case MSG_TYPE_ASK_AUTOSEARCH:
        {
            //ISIL_ParseAutoSearchDev(data, iConnectFd, arg);
        }
        break;
        case MSG_TYPE_ASK_LOGIN:
        {
            ISIL_ParseLogin(data, task->fd);
        }
        break;
        case MSG_TYPE_PCTODEV_REQ:
        case MSG_TYPE_FILE:
        {
            ISIL_ClientReqCfg(data, task->fd, NULL);
        }
        break;


        default:
            break;
    }

	data = NULL;

	return ISIL_SUCCESS;

}

#if 1
/*解析本地网络连接MSG*/
int ISIL_ParseLocalNetMessages(int readlen, void* buf, struct NET_TASK* task)
{

	S08 *data = NULL;
    S08 *pTemp = NULL;

	S32 iLen;
	ISIL_MSG_HERADE_S *sMsgHead;


    if( !readlen || !buf ) {
        return -1;
    }

	iLen = sizeof(sMsgHead);
	ISIL_TRACE(BUG_NORMAL, "[%s] fd %d", __FUNCTION__, task->fd);

    pTemp = buf;
    sMsgHead = (ISIL_MSG_HERADE_S*)pTemp;

    ISIL_TRACE(BUG_NORMAL, "%s: msg type: 0x%x", __FUNCTION__, sMsgHead->nMsgType);

    data = pTemp + sizeof(ISIL_MSG_HERADE_S);

#ifdef LOCAL_GUI_ON_BOARD
	iLen = (sMsgHead->lMsgLen);
	ISIL_ParseLocalNetMsg((sMsgHead->nMsgType), data, iLen, task->fd);
#else
	iLen = _swab32(sMsgHead->lMsgLen);
	ISIL_ParseLocalNetMsg(_swab16(sMsgHead->nMsgType), data, iLen, task->fd);
#endif

	data = NULL;

	return ISIL_SUCCESS;

}

#endif

static S32 ISIL_ParseRealPlay(S08 *data, S32 iConnectfd, struct NET_TASK* task)
{
    S08 cTemp[ISIL_NAME_LEN];
    ISIL_NET_IE_ASK_S *ieheader;
    S08 *ieheader_payload;
    ISIL_ACK_DATA_S ackdata;
    S32 ret = 0;
    S32 channel;
    ISIL_NET_REALPLAY_S* spRealPlay = NULL;
    struct sockaddr_in *sa = NULL;

    if( !task )
    {
        ISIL_TRACE(BUG_ERROR, "[%s] arg null", __FUNCTION__);
        return ISIL_FAIL;
    }

#if 1
    if(task->trans_type == NET_TCP_E) {
        sa = NULL;
    }
    else{
        sa = &task->sa;
    }
#endif

    ieheader = (ISIL_NET_IE_ASK_S*)data;

	memset(&ackdata, 0x00,sizeof(ISIL_ACK_DATA_S));

	ieheader = (ISIL_NET_IE_ASK_S*)((char*)ieheader);
	ieheader_payload = (S08*)(ieheader + 1);
	channel = _swab32(ieheader->lReserve);


    switch(_swab16(ieheader->nAskType))
    {
        case IE_TYPE_ASK_REALPLAY:
        {
            if(_swab16(ieheader->nAskLength) != sizeof(ISIL_NET_REALPLAY_S)){
                return ISIL_FAIL;
            }

            spRealPlay = (ISIL_NET_REALPLAY_S*)(ieheader_payload);

            ISIL_TransRealPlay(spRealPlay);
			/*改为注册session 到 net_list*/
            printf("ask realplay. stream type 0x%x\n", spRealPlay->nLinkType);

            CHIP_CHN_PHY_ID phy_id_t;
            #ifndef ISIL_USE_SYS_CFG_FILE
            ret = get_chip_chn_id_by_logic_id(channel,&phy_id_t);
            #else
            ret = ISIL_GetChipChnIDByLogicID(channel,&phy_id_t);
            #endif
            if( ret < 0 ) {
                ISIL_BuildAckData(&ackdata, &ret, IE_NO_PLAYLOAD,
        			IE_TYPE_ACK_REALPLAY, ISIL_ERR_DEV_FAILURE);
                break;
            }

            ISIL_BuildAckData(&ackdata, cTemp, sizeof(cTemp),
                    			IE_TYPE_ACK_REALPLAY, 0);

            ISIL_SendAckData(iConnectfd, &ackdata, 1,
                                (void *)sa, 0,ISIL_PC_MSG);


#ifdef USE_NEW_DATA_DISPATCH_POLICY
            reg_net_task_write_callback(task,new_send_enc_data_to_client_cb);
#else
            reg_net_task_write_callback(task,send_enc_data_to_client_cb);
#endif


            if(spRealPlay->nLinkType & ISIL_NET_LINK_AUDIO ) {

            	DEBUG_FUNCTION();
                ret = net_task_reg_enc( task ,
                                                        phy_id_t.chipid,
                                                        phy_id_t.chanid,
                                                        IS_AUDIO_STREAM);
                if( ret < 0) {
                    ISIL_BuildAckData(&ackdata, &ret, IE_NO_PLAYLOAD,
        			IE_TYPE_ACK_REALPLAY, ISIL_ERR_DEV_FAILURE);
                    break;
                }
            }

            if(spRealPlay->nLinkType & (ISIL_NET_LINK_MAIN_I | ISIL_NET_LINK_MAIN) ) {

                /*TODO: set ioctl*/

                DEBUG_FUNCTION();
                
                if(spRealPlay->nLinkType & ISIL_NET_LINK_MAIN_I) {
                    DEBUG_FUNCTION();
                    task->frame_type = NET_ONLY_NEED_I_FRAME_E;
                }
                else{
                    task->frame_type = NET_NEED_ALL_FRAME_E;
                }

                DEBUG_FUNCTION();
                alloc_enc_param_t_to_net_task(task);

                ret = net_task_reg_enc( task ,
                                        phy_id_t.chipid,
                                        phy_id_t.chanid,
                                        IS_H264_MAIN_STREAM);
                if( ret < 0) {
                    ISIL_BuildAckData(&ackdata, &ret, IE_NO_PLAYLOAD,
        			IE_TYPE_ACK_REALPLAY, ISIL_ERR_DEV_FAILURE);
                    break;
                }
                DEBUG_FUNCTION();
            }


            if(spRealPlay->nLinkType & (ISIL_NET_LINK_SUB_I | ISIL_NET_LINK_SUB ) ) {

                /*TODO: set ioctl*/

            	DEBUG_FUNCTION();
                if(spRealPlay->nLinkType & ISIL_NET_LINK_SUB_I) {
                    task->frame_type = NET_ONLY_NEED_I_FRAME_E;
                }
                else{
                    task->frame_type = NET_NEED_ALL_FRAME_E;
                }

                alloc_enc_param_t_to_net_task(task);
                ret = net_task_reg_enc( task ,
                                                        phy_id_t.chipid,
                                                        phy_id_t.chanid,
                                                        IS_H264_SUB_STREAM);
                if( ret < 0) {
                    ISIL_BuildAckData(&ackdata, &ret, IE_NO_PLAYLOAD,
        			IE_TYPE_ACK_REALPLAY, ISIL_ERR_DEV_FAILURE);
                    break;
                }
                DEBUG_FUNCTION();
            }


            if(spRealPlay->nLinkType & ISIL_NET_LINK_PIC ) {

            	DEBUG_FUNCTION();

                /*TODO: set ioctl*/
                ret = net_task_reg_enc( task ,
                                                        phy_id_t.chipid,
                                                        phy_id_t.chanid,
                                                        IS_MJPEG_STREAM);
                if( ret < 0) {
                    ISIL_BuildAckData(&ackdata, &ret, IE_NO_PLAYLOAD,
        			IE_TYPE_ACK_REALPLAY, ISIL_ERR_DEV_FAILURE);
                    break;
                }
                DEBUG_FUNCTION();
            }

            
            return ISIL_SUCCESS;

#if 0
            if(ret < 0)
                ISIL_BuildAckData(&ackdata, &ret, IE_NO_PLAYLOAD,
        			IE_TYPE_ACK_REALPLAY, ISIL_ERR_DEV_FAILURE);
            else
                ISIL_BuildAckData(&ackdata, cTemp, sizeof(cTemp),
        			IE_TYPE_ACK_REALPLAY, 0);
#endif






        }
        break;
	case IE_TYPE_NET_ALIVE:

#ifdef DEL_KEEPLIVE
			//del_session_ev((SESSION*)arg);
#endif
        return 0;
        default:
        return 0;
    }


    ISIL_SendAckData(iConnectfd, &ackdata, 1,
        (void *)sa, 0,ISIL_PC_MSG);
    return ISIL_SUCCESS;
}


int ISIL_ParseDataMsg(int readlen, void* buf, struct NET_TASK* task)
{

    S08 *data = NULL;
    S08 *pTemp = NULL;

	S32  iRet,iLen;
	ISIL_MSG_HERADE_S *sMsgHead;


    if( !readlen || !buf ) {
        return -1;
    }

	iLen = sizeof(sMsgHead);
	ISIL_TRACE(BUG_NORMAL, "[%s] fd %d", __FUNCTION__, task->fd);

    pTemp = buf;
    sMsgHead = (ISIL_MSG_HERADE_S*)pTemp;

    ISIL_TRACE(BUG_NORMAL, "%s: msg type: 0x%x", __FUNCTION__, sMsgHead->nMsgType);

    data = pTemp + sizeof(ISIL_MSG_HERADE_S);

    iLen = _swab32(sMsgHead->lMsgLen);

    switch(_swab16(sMsgHead->nMsgType))
    {
        case MSG_TYPE_ASK_REALPLAY:
        {

            //iRet = ISIL_ParseRealPlay(data, iConnectFd, arg);
            
        	iRet = ISIL_ParseRealPlay(data, task->fd, task);
                if( iRet < 0) {
                    fprintf(stderr,"ISIL_ParseRealPlay err .\n");
                    return -1;
                }
            
        }
        break;

        default:
            return -1;
    }

	data = NULL;

	return 0;

}
