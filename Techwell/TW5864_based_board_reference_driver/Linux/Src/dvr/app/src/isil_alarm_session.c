#include <stdio.h>


#include "isil_alarm_session.h"
#include "isil_enc_sdk.h"

#include "isil_net_config.h"
#include "io_utils.h"
#include "net_debug.h"
#include "isil_channel_map.h"



static void debug_mem(int len ,char *buff)
{
	int i;

	printf(" \n");
	for( i = 0 ; i < len ; i++ ){

		printf(" %x " ,buff[i]);

	}

	printf(" \n");
}



static int recv_and_send_alarm_msg(void *alarm_packet , void *pContext)
{
	int fd ,ret ,len = 0 ;

	struct NET_TASK *net_task;
	ISIL_ALARM_PACKET *alarm_pck;

	NET_AYSN_MSG_T *net_aysn_msg_ptr;

	char buff[NET_ALARM_BUFF_SIZE];

	if( !alarm_packet || !pContext) {
		return -1;
	}

	net_task = (struct NET_TASK *)pContext;

	fd = GET_NET_TASK_FD(net_task);
	if( fd <= 0 ) {
		fprintf(stderr,"GET_NET_TASK_FD failed .\n");
		return -1;
	}

	net_aysn_msg_ptr = (NET_AYSN_MSG_T *)buff;

	len += sizeof(NET_AYSN_MSG_T);
	
	alarm_pck = (ISIL_ALARM_PACKET *)alarm_packet;

	net_aysn_msg_ptr->msg_type = bswap32(alarm_pck->alarm_type_e);

	switch(alarm_pck->alarm_type_e) {
	case VIDEO_LOST_DETECTION_E:{

		ISIL_VIDEO_LOST_DETECTION_T *video_lost_detect = (ISIL_VIDEO_LOST_DETECTION_T *)(&(alarm_pck->alarm_type_u.v_lost_detect_inf));
	    net_aysn_msg_ptr->msg_len = bswap32(sizeof(ISIL_VIDEO_LOST_DETECTION_T));
		ISIL_VIDEO_LOST_DETECTION_T *net_v_lost_ptr = (ISIL_VIDEO_LOST_DETECTION_T *)( buff + len);
		net_v_lost_ptr->chan_id = bswap16(video_lost_detect->chan_id);
		net_v_lost_ptr->chip_id = bswap16(video_lost_detect->chip_id);
		//fprintf(stderr, "[%s %d] chipid %d chn id %d======\n", __FUNCTION__, __LINE__, net_v_lost_ptr->chip_id,
		//		net_v_lost_ptr->chan_id);
		net_v_lost_ptr->video_contect_valid = bswap32(video_lost_detect->video_contect_valid);
		net_v_lost_ptr->video_lost_valid = bswap32(video_lost_detect->video_lost_valid);
		len += sizeof(ISIL_VIDEO_LOST_DETECTION_T);
		
		ret = r_write(fd ,buff , len);
		if( ret <= 0 ) {
			fprintf( stderr,"write to net alarm err.\n");
			return -1;
		}

		


	}
		break;
	case VIDEO_NIGHT_DETECTION_E :{

		ISIL_VIDEO_NIGHT_DETECTION_T *video_night_detect = (ISIL_VIDEO_NIGHT_DETECTION_T *)(&(alarm_pck->alarm_type_u.v_night_detect_inf));
		net_aysn_msg_ptr->msg_len = bswap32(sizeof(ISIL_VIDEO_NIGHT_DETECTION_T));
		ISIL_VIDEO_NIGHT_DETECTION_T *net_v_ngiht_ptr = (ISIL_VIDEO_NIGHT_DETECTION_T *)( buff + len);
		net_v_ngiht_ptr->chan_id = bswap16(video_night_detect->chan_id);
		net_v_ngiht_ptr->chip_id = bswap16(video_night_detect->chip_id);
		//fprintf(stderr, "[%s %d] chipid %d chn id %d======\n", __FUNCTION__, __LINE__, 
		//		net_v_ngiht_ptr->chip_id,
		//		net_v_ngiht_ptr->chan_id);
		net_v_ngiht_ptr->video_night_valid_from_day_to_night = bswap32(video_night_detect->video_night_valid_from_day_to_night);
		net_v_ngiht_ptr->video_night_valid_from_night_to_day = bswap32(video_night_detect->video_night_valid_from_night_to_day);
		len += sizeof(ISIL_VIDEO_NIGHT_DETECTION_T);

		ret = r_write(fd ,buff , len);
		if( ret <= 0 ) {
			fprintf( stderr,"write to net alarm err.\n");
			return -1;
		}

	}
		break;
	case VIDEO_BLIND_DETECTION_E:{


		ISIL_VIDEO_BLIND_DETECTION_T *video_blind_detect = (ISIL_VIDEO_BLIND_DETECTION_T *)(&(alarm_pck->alarm_type_u.v_blind_detect_inf));
		net_aysn_msg_ptr->msg_len = bswap32(sizeof(ISIL_VIDEO_BLIND_DETECTION_T));
		ISIL_VIDEO_BLIND_DETECTION_T *net_v_blind_ptr = (ISIL_VIDEO_BLIND_DETECTION_T *)( buff + len);
		net_v_blind_ptr->chan_id = bswap16(video_blind_detect->chan_id);
		net_v_blind_ptr->chip_id = bswap16(video_blind_detect->chip_id);
		//fprintf(stderr, "[%s %d] chipid %d chn id %d======\n", __FUNCTION__, __LINE__, 
		//		net_v_blind_ptr->chip_id,
		//		net_v_blind_ptr->chan_id);
		net_v_blind_ptr->video_blind_add_valid = bswap32(video_blind_detect->video_blind_add_valid);
		net_v_blind_ptr->video_blind_remove_valid = bswap32(video_blind_detect->video_blind_remove_valid);
		len += sizeof(ISIL_VIDEO_BLIND_DETECTION_T);

		ret = r_write(fd ,buff , len);
		if( ret <= 0 ) {
			fprintf( stderr,"write to net alarm err.\n");
			return -1;
		}

		

	}
		break;
	case VIDEO_STANDARD_MODIFY_DETECTION_E:{
		
		ISIL_VIDEO_STANDARD_DETECTION_T *video_standard_detect = (ISIL_VIDEO_STANDARD_DETECTION_T *)(&(alarm_pck->alarm_type_u.v_standard_detect_inf));
		net_aysn_msg_ptr->msg_len = bswap32(sizeof(ISIL_VIDEO_STANDARD_DETECTION_T));
		ISIL_VIDEO_STANDARD_DETECTION_T *net_v_standard_ptr = (ISIL_VIDEO_STANDARD_DETECTION_T *)( buff + len);
		net_v_standard_ptr->chan_id = bswap16(video_standard_detect->chan_id);
		net_v_standard_ptr->chip_id = bswap16(video_standard_detect->chip_id);
		//fprintf(stderr, "[%s %d] chipid %d chn id %d======\n", __FUNCTION__, __LINE__, 
		//		net_v_standard_ptr->chip_id,
		//		net_v_standard_ptr->chan_id);
		net_v_standard_ptr->latest_video_standard = bswap32(video_standard_detect->latest_video_standard);
		net_v_standard_ptr->curr_video_standard = bswap32(video_standard_detect->curr_video_standard);
		len += sizeof(ISIL_VIDEO_STANDARD_DETECTION_T);

		ret = r_write(fd ,buff , len);
		if( ret <= 0 ) {
			fprintf( stderr,"write to net alarm err.\n");
			return -1;
		}

	}
		break;
	default:
		return -1;
	}


	return 0;


}


static int record_alarm_status( unsigned int chip_id,int fd ,ALARM_RECORD_STATUS_TYPE_E status,int result )
{
	int msg_type ,ret ,len = 0;
	NET_AYSN_MSG_T *net_aysn_msg_ptr;
	NET_ALARM_STATUS_T *net_alarm_status;
	char buff[NET_ALARM_BUFF_SIZE];

	net_aysn_msg_ptr = (NET_AYSN_MSG_T *)buff;

	msg_type = status;

	net_aysn_msg_ptr->msg_type = bswap32((unsigned int)msg_type);

	len += sizeof(NET_AYSN_MSG_T);
	net_aysn_msg_ptr->msg_len = bswap32(len);

	net_alarm_status = (NET_ALARM_STATUS_T *)(buff + len);
	net_alarm_status->chip_id = bswap32(chip_id);	
	net_alarm_status->result  = bswap32(result);

	len += sizeof(NET_ALARM_STATUS_T);
	
	ret = r_write(fd ,buff , len );
	if( ret <= 0 ) {
		fprintf( stderr,"write to net alarm err.\n");
		return -1;
	}

	return 0;


}


int start_alarm_session( unsigned int chip_id , struct NET_TASK *net_task)
{
	int ret;

	if( !net_task ) {
		return -1;
	}
	
	fprintf(stderr, "%s chipid %d=========\n", __FUNCTION__, chip_id);
	ret = ISIL_MediaSDK_RegAlarmCallback(chip_id ,
										 recv_and_send_alarm_msg ,
										 (void *)net_task);
	if( ret < 0 ) {
		fprintf( stderr,"start_alarm_session failed .\n");
		record_alarm_status(chip_id ,net_task->fd ,ALARM_START_MSG_ANSWER_TYPE ,NET_ALARM_STATUS_FAILED);

		return -1;
	}

	ISIL_MediaSDK_StartAlarm(chip_id);

	return 0;
	

}


int stop_alarm_session(unsigned int chip_id ,struct NET_TASK *net_task)
{
	int ret;
	if( !net_task ) {
		return -1;
	}

	ret = ISIL_MediaSDK_StopAlarm(chip_id);
	if( ret < 0 ) {
		record_alarm_status(chip_id ,net_task->fd ,ALARM_STOP_MSG_ANSWER_TYPE ,NET_ALARM_STATUS_FAILED);
		return -1;
	}

	return 0;

}


int async_net_read_cb(int readlen, void* buff, struct NET_TASK* task)
{
	
	int ret,total_chip_num;
	NET_ALARM_STATUS_T *net_alarm_status;
	NET_AYSN_MSG_T *net_aysn_msg_ptr;
	unsigned int msg_type ,msg_len,chip_id;


	DEBUG_FUNCTION();

	if( !readlen || !buff ) {
        return -1;
    }

	if( !task ) {
		return -1;
	}

	if( readlen != ( sizeof(NET_AYSN_MSG_T) + sizeof(NET_ALARM_STATUS_T))) {
		fprintf( stderr,"Read len err .\n");
		return -1;
	}

	total_chip_num = ISIL_GetChipCnt();
	if( total_chip_num <= 0 ) {
		fprintf( stderr,"total_chip_num alarm err .\n");
		return -1;
	}


	//debug_mem(readlen, (char *)buff);

	net_aysn_msg_ptr = (NET_AYSN_MSG_T *)buff;

	msg_len  = bswap32(net_aysn_msg_ptr->msg_len);
	if(msg_len != sizeof(NET_ALARM_STATUS_T)) {
		fprintf( stderr,"msg_len err .\n");
		return -1;
	}

	msg_type = bswap32(net_aysn_msg_ptr->msg_type);

	printf("msg_type[%x],len[%d].\n",msg_type,msg_len);

	net_alarm_status = (NET_ALARM_STATUS_T *)((char *)buff + sizeof(NET_AYSN_MSG_T));

	switch(msg_type) {
	case ALARM_START_MSG_ASK_TYPE:{
		//chip_id = bswap32(net_alarm_status->chip_id);
		
		
		for( chip_id = 0 ; chip_id < total_chip_num ; chip_id++ ) {

			ret = start_alarm_session(chip_id,task);
		
			if(ret < 0 ) {
				fprintf( stderr,"start_alarm_session err .\n");
				return -1;
			}
		}
		

	}
		break;
	case ALARM_STOP_MSG_ASK_TYPE:{
		//chip_id = bswap32(net_alarm_status->chip_id);
		
		for( chip_id = 0 ; chip_id < total_chip_num ; chip_id++ ) {
			ret = stop_alarm_session(chip_id,task);
			if(ret < 0 ) {
				fprintf( stderr,"start_alarm_session err .\n");
				return -1;
			}
		}

	}
		break;
	default:
		fprintf( stderr,"alarm msg type err .\n");
		return -1;
	}

	return 0;

}
