#pragma once

// vim: ts=4 sw=4
/* #######################################################################
 * NetRtsp.h
 * Created by MingYoung   Date:2007/0410
 * Description:
 *    Header file for NetRTSP Library. 
 * ####################################################################### */

/* #######################################################################
 * RTSP STATE VARIABLES
 * ####################################################################### */
#define RTSP_OK					0
#define RTSP_ERR				1

/* #######################################################################
 * RTSP Return Codes
 * ####################################################################### */
#define RTSP_RET_OK				200
#define RTSP_RET_BAD_REQUEST    400
#define RTSP_RET_UNAUTHROIZED	401
#define RTSP_RET_NOT_FOUND      404
#define RTSP_RET_NOT_SUPPORTED	461

/* #######################################################################
 * RTSP Timers
 * ####################################################################### */
#define RTSP_TIMER_TX			100	/* 100 msec */

#define THE_RTP_PLAYER			"The Media Player 1.0"

/* #######################################################################
 * RTSP Host Configuration Database
 * ####################################################################### */
#define RTSP_DIGEST_LEN			128
typedef struct {
	char Nonce[RTSP_DIGEST_LEN];	/* Digest Authentication Key 1 string */
	char Realm[RTSP_DIGEST_LEN];	/* Digest Authentication Key 2 string */
	char Uri[RTSP_DIGEST_LEN];		/* Digest Authentication Key 3 string */
	char Resp[RTSP_DIGEST_LEN];		/* Digest Challenge string */
} RTSP_DIGEST;

/* ##### definitions in Type in RTSP_MEDIA ##### */
#define RTSP_MEDIA_JPEG_ES90000 26
#define RTSP_MEDIA_DYNAMIC	96
#define RTSP_MEDIA_PCM16_8KHZ   111
typedef struct {
	unsigned char Config[128]; /* config string in the RTSP Describe Reply packet.
								  It ndicates the header of the I frame in RTP 
								  video stream */
	int  ConfigLen;            /* length of the config string */
	int  Type;
	int  DataType;
	int  Seq;                  /* Sequence number given by RTSP server */
	int  RTPSrvPort;
	int  RTPHostPort;
	int  RTCPSrvPort;
	int  RTCPHostPort;
	char CtrlPath[32];
} RTSP_MEDIA;
/* ##### definitions in OptionCap in RTSP_HOST_INFO ##### */
#define RTSP_CMD_CAP_OPTION		0x01
#define RTSP_CMD_CAP_DESCRIBE	0x02
#define RTSP_CMD_CAP_SETUP		0x04
#define RTSP_CMD_CAP_SETPARAM	0x08
#define RTSP_CMD_CAP_TEARDOWN	0x10
#define RTSP_CMD_CAP_PLAY		0x20
#define RTSP_CMD_CAP_PAUSE		0x40
typedef struct {
	int  SrvRTSPCmdCap;	  /* RTSP Server capability list (bitmap) */
	int  CSeq;			  /* sequence number of RTSP packets, issued by host */
	char  Session[256];         /* Session number increased by Host per RTSP packet */
	RTSP_DIGEST	Digest;	  /* RTSP Digest authentication database */
	RTSP_MEDIA  Media[2]; /* Media[0]: Video, Media[1]: Audio */
} RTSP_HOST_INFO;
/* ##### definitions in Encoder in RTSP_HOST_CONF ##### */
#define RTSP_ENCODER_NONE		0
#define RTSP_ENCODER_DIGEST		1
#define RTSP_ENCODER_BASE64		2
/* ##### definitions in ConnState in RTSP_HOST_CONF ##### */
#define RTSP_CONN_REQ			0	/* request for a RTSP connection */
#define RTSP_CONN_PROC			1	/* report the connection is in process */
#define RTSP_CONN_OK			2	/* report the RTSP connection was built */
#define RTSP_CONN_PAUSE			3	/* report RTP is in PAUSE state */
#define RTSP_CONN_CLOSE			4	/* request for tear down RTSP connection */
#define RTSP_CONN_ERR			20	/* report errors in RTSP connection */
#define RTSP_CONN_ERR_SOCK		(RTSP_CONN_ERR+1)
#define RTSP_CONN_ERR_CONNECT	(RTSP_CONN_ERR+2)
#define RTSP_CONN_ERR_SIGNAL    (RTSP_CONN_ERR+3)
/* ##### definitions in RTPProt in RTSP_HOST_CONF ##### */
#define RTSP_RTP_UDP		    1
#define RTSP_RTP_MULTICAST		2

typedef struct {
	/* RTSP Server configuration */
	int  Sock;		     /* RTSP TCP connection socket ID */
	char HostIP[16];     /* Host's local IP address */
	char SrvIP[16];      /* RTSP Server's IP address */
	char SrvMcastIP[16]; /* RTP Server's Multicast IP address */
	char FilePath[256];
	int  SrvPort;
	int  ConnTime;	/* Max time in sec duration of connecting RTSP Server 
					   0: connect to RTSP server in blocking method */
	int  RespTime;  /* Max time in msec to wait for RTSP server's reply 
					   0: blocking wait for RTSP server's reply */
	int  RTPProt;
	/* RTSP connection configuration */
	int  Encoder;
	char Name[32];
	char Pwd[32];
	/* RTSP connection state control */
	int  ConnState;
	/* RTSP connection information */
	RTSP_HOST_INFO Info;
} RTSP_HOST_CONF;


