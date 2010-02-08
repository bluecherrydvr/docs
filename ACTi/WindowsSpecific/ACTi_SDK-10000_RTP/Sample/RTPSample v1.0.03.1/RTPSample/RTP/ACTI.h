#pragma once

// vim: ts=4 sw=4
/* #######################################################################
 * ACTI.h
 *     Acti's definied header 
 * ####################################################################### */

/* #######################################################################
 * B2 Header
 * ####################################################################### */
/* definitions of Type in B2_HEADER */
#define ACTI_HEAD_MSG_B2_VIDEO          	0x00000001
#define ACTI_HEAD_MSG_B2_AUDIO          	0x00000002
/* definitions of Type in ACTi header */
/* LIVE CHECK used in the control session */
#define ACTI_HEAD_MSG_LIVE_REQ				0x00000030
#define ACTI_HEAD_MSG_LIVE_RSP          	0x00010030	/* not used */
#define ACTI_HEAD_MSG_EXIT_REQ          	0x00000031
#define ACTI_HEAD_MSG_EXIT_RSP          	0x00010031	/* not used */
/* DIOs used in the control session */
#define ACTI_HEAD_MSG_DIO_OUT_REQ       	0x00000032
#define ACTI_HEAD_MSG_DIO_OUT_RSP       	0x00010032	/* not used*/
#define ACTI_HEAD_MSG_DIO_STATUS_REQ    	0x00000033
#define ACTI_HEAD_MSG_DIO_STATUS_RSP    	0x00010033
#define ACTI_HEAD_MSG_DIO_INPUT_REQ     	0x00000034	/* not used */
#define ACTI_HEAD_MSG_DIO_INPUT_RSP     	0x00010034
/* RS485 used in the control session */
#define ACTI_HEAD_MSG_SERIAL_RECV_REQ   	0x00000035	/* not used */
#define ACTI_HEAD_MSG_SERIAL_RECV_RSP   	0x00010035
#define ACTI_HEAD_MSG_SERIAL_SEND_REQ   	0x00000036
#define ACTI_HEAD_MSG_SERIAL_SEND_RSP   	0x00010036	/* not used */
/* AUDIO_IN used in the control session */
#define ACTI_HEAD_MSG_AUDIO_PLAY_REQ    	0x00000037
#define ACTI_HEAD_MSG_AUDIO_PLAY_RSP    	0x00010037	/* not used */
/* VIDEO LOSS used in the control session */
#define ACTI_HEAD_MSG_VIDEO_LOSS_REQ    	0x00000038	/* not used */
#define ACTI_HEAD_MSG_VIDEO_LOSS_RSP    	0x00010038
/* MOTION used in the control session */
#define ACTI_HEAD_MSG_MOTION_DETECT_REQ 	0x00000039	/* not used */
#define ACTI_HEAD_MSG_MOTION_DETECT_RSP 	0x00010039
/* CAMERA NAME in the control session */
#define ACTI_HEAD_MSG_CAMERA_NAME_REQ        0x00000040
#define ACTI_HEAD_MSG_CAMERA_NAME_RSP        0x00010040

typedef struct {
	unsigned char Key[4];  /* 00 00 01 B2 */
	unsigned char  Type;
	unsigned char Stream_id;
	unsigned char Ext_b2_len;	// 1: length of the ext. b2 private data appended to B2 Frame
	unsigned char Rsvd;
	unsigned int  Len;
} B2_HEADER, ACTI_HEADER;

#define B2_HEADER_LEN                 sizeof(B2_HEADER)
#define ACTI_HEADER_LEN               sizeof(ACTI_HEADER)

typedef struct {
	B2_HEADER      Head;
	struct timeval TS;
	unsigned char  Rsvd[8];
} AUDIO_HEADER;
#define AUDIO_HEADER_LEN              sizeof(AUDIO_HEADER)
#define AUDIO_B2_LEN                  AUDIO_HEADER_LEN-B2_HEADER_LEN

/* #######################################################################
 * Private data followed by B2 header
 * ####################################################################### */
/* definitions of VLoss in PRIVATE_DATA */
#define B2_VIDEO_LOSS_NOT_FOUND     1       /* video ok */
#define B2_VIDEO_LOSS_FOUND         0       /* video loss */
typedef struct {
	time_t         Time;
	unsigned char  TimeZone;   /* mapping the TIME_ZONE in conf file. 0:-12, ..., 24:+13 */
	unsigned char  VLoss;      /* 0: video loss, 1 : video ok */
	unsigned char  Motion;
	unsigned char  DIO;			/* for DIs, 0: DI triggered. 1: no triggered */
	unsigned int   Cnt;
	unsigned char  Resolution;  /* mapping the VIDEO_RESOLUTION in cond file. 0:N720x480, ... */
	unsigned char  BitRate;     /* mapping the VIDEO_BITRATE in cond file. 0:28K, ... */
	unsigned char  FpsMode;     /* mapping the VIDEO_FPS in cond file. 0:MODE1(constant), 1:0:MODE2 */
	unsigned char  FpsNum;      /* in constant FPS mode, it indicates the video server's constant
								   FPS number, i.e. atoi(VIDEO_FPS_NUM).
								   in variable FPS mode, in indicates the variable FPS number which
								   was requested by the TCP host. If it is not in TCP, it indicates
								   the variable FPS number, i.e. atoi(VIDEO_VARIABLE_FPS) */
	struct timeval TS;
	unsigned char  Rsvd[8];
} PRIVATE_DATA;
#define PRIVATE_DATA_LEN         sizeof(PRIVATE_DATA_LEN)

typedef struct {
	B2_HEADER      Head;
	PRIVATE_DATA   Data;
} B2_FRAME;

#define B2_FRAME_LEN                    sizeof(B2_FRAME)

#define MAX_RESERVED_AUDIO_HEADER       AUDIO_HEADER_LEN

typedef struct {
	B2_HEADER      Head;
	struct timeval TS;
	unsigned char  Rsvd[8];
} AUDIO_B2_FRAME;

#define AUDIO_B2_FRAME_LEN		sizeof(AUDIO_B2_FRAME)

/* #######################################################################
 * definitions of status in TCP_AUTHEN_REPLY
 * ####################################################################### */
#define TCP_AUTHEN_SUCCESS              0x00
#define TCP_AUTHEN_ERROR                0x01
typedef struct {
	char Status;
	char Rsvd[3];
	int  Sock;
	char CameraName[120];
} TCP_AUTHEN_REPLY;

// resolutions
#define XNTSC720x480	    0		// 0x00  0000 0000
#define XNTSC352x240	    1		// 0x01  0000 0001
#define XNTSC160x112	    2		// 0x02  0000 0010
#define XPAL720x576			3		// 0x03  0000 0011
#define XPAL352x288			4		// 0x04  0000 0100
#define XPAL176x144 	    5		// 0x05  0000 0101
#define XNTSC176x120	    6		// 0x06  0000 0110

// new resolution
#define XNTSC640x480	    64		// 0x40  0100 0000
#define XPAL640x480		    192		// 0xC0  1100 0000

#define XNTSC1280x720		65		// 0x41  0100 0001
#define XNTSC1280x960		66		// 0x42  0100 0010
#define XNTSC1280x1024		67		// 0x43  0100 0011
#define XNTSC1600x1200      68      // 0x44  0100 0100
#define XNTSC1920x1080		69		// 0x45  0100 0101

#define XNTSC320x240        70      // 0x46  0100 0110
#define XNTSC160x120        71      // 0x47  0100 0111
