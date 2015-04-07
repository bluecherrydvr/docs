#ifndef  _ISIL_ALARM_SESSION_H
#define  _ISIL_ALARM_SESSION_H



#ifdef __cplusplus
extern "C" {
#endif

#include "isil_data_stream.h"
#include "net_task.h"

#define NET_ALARM_BUFF_SIZE (32)

#define NET_ALARM_STATUS_FAILED (-1)
#define NET_ALARM_STATUS_SUCCEED (0)


typedef enum{

	ALARM_START_MSG_ANSWER_TYPE = 0x0000f001,
	ALARM_STOP_MSG_ANSWER_TYPE  = 0x0000f002,
	ALARM_START_MSG_ASK_TYPE = 0x0000f003,
	ALARM_STOP_MSG_ASK_TYPE  = 0x0000f004,
	
}ALARM_RECORD_STATUS_TYPE_E;



typedef struct _NET_AYSN_MSG_T{
	int msg_type;
	int msg_len;
}NET_AYSN_MSG_T;


typedef struct _NET_ALARM_STATUS_T{
	
	unsigned int chip_id;
	int result;
	int reserv;
	
}NET_ALARM_STATUS_T;


extern int start_alarm_session( unsigned int chip_id , struct NET_TASK *net_task);

extern int stop_alarm_session(unsigned int chip_id ,struct NET_TASK *net_task);

extern int async_net_read_cb(int readlen, void* buff, struct NET_TASK* task);

#ifdef __cplusplus
}
#endif

#endif
