
/*
*******************************************************************
				Copyright (C), 2005-2009, Dvmicro Tech. Co., Ltd.

	File Name: Dvm_handlemessage.h
	Author: wangjunbin
	Version:0.1
	Date: 2009-03-19
	Description:define some data struct for Dvm.


	Other:
	Function list:
	History:

*******************************************************************
*/
#ifndef _ISIL_HANDLE_MESSAGE_H
#define _ISIL_HANDLE_MESSAGE_H
#ifdef __cplusplus
extern "C" {
#endif
//#include "service.h"
//#include "task.h"

#include "isil_messagestruct.h"
#include "net_task.h"

#define IE_NO_PLAYLOAD 0


/*temp define frame header*/
typedef struct _ISIL_FRAME_HEADER_STRU
{
	void * header;
}ISIL_FRAME_HEADER_S;


//extern S32 ISIL_ParseNetMessages(int readlen, void* buf, struct NET_TASK* task);
extern S32 ISIL_RecvMsgFromOther(void* service ,void** task);
extern S32 ISIL_SendAlarmToPC(S32 iChannel, S32 iAlarmType, void* service);
extern S32 ISIL_SendMsgToOther(S32 ServiceID, S32 InfoType, S32 iMsgLen, S32 iMsgFd, S32 MsgSrc, void* MsgBody);

extern S32 ISIL_SetAlarmActive(S32 iChannel, S32 iVideoAlarmType, void *pAlarmCfg);

extern void ISIL_SetChannelParameterByStreamCfg(S32 iConnectfd, G_STREAM_CFG* pCfg);

extern S32 ISIL_SetStreamCfg(S32 iConnectfd, G_STREAM_CFG* pCfg);

extern S32 ISIL_HandleRecLinkage(void* pLinkageInfo);

extern void ISIL_HandleLinkage(U32 lLinkageType, void* pLinkageInfo);

extern S32 ISIL_SetLinkageActive(U32 Channel, U32 AlarmType);

extern S32 ISIL_HandleShootLinkage(void* pLinkageInfo);
extern S32 ISIL_SendAckData(S32 sockfd, ISIL_ACK_DATA_S* ackData, S32 msg_num, void* net_addr, U32 uiFd, S32 where);
extern void ISIL_BuildAckData(ISIL_ACK_DATA_S *ackdata, void * data, S32 len,
									S32 type, S32 ackfalg);
extern S32 ISIL_HandleSearchEvnt(S32 iFd, ISIL_SEARCH_EVNET* pEvnt, S32 iMsgNum, S32 MsgWhere);

extern S32 ISIL_GetEncodeParmByType(S32 ch, S32 type, S32 streamType, void* buff, int len);

extern S32 ISIL_InitCfgFile();

extern int ISIL_ParseLogin(S08 *data, S32 iConnectfd);

extern S32  ISIL_ClientReqCfg(S08 *data, S32 iConnectfd, void * arg);

extern void ISIL_TransRealPlay(ISIL_NET_REALPLAY_S* cfg);
extern S32 ISIL_SetPlaybackAction(S32 iConnectFd, S32 iActionType, S32 MsgSrc,
                                    NET_FILE_COMMAND* pActionParm, U32 filefd);
extern void ISIL_GetPlaybackFileInfo(S32 iConnectFd, S32 MsgSrc, ISIL_NETFILE_OPEN * pFileOpen, void * arg);
extern S32 ISIL_SetDefaultVD16Cif();
extern S32 ISIL_SetDefaultVD4D1();

extern void ISIL_ImageSizeEnumToDigit(U32 type, U32* w1, U32* h1, U32 w2, U32 h2);

extern S32 ISIL_SetVDByOpenChn(S32 chn);

#ifdef __cplusplus
}
#endif
#endif


