#pragma once

// vim: ts=4 sw=4
/* #######################################################################
 * NetRtp.h
 * Created by MingYoung   Date:2007/0410
 * Description:
 *    Header file for NetRTP Library.
 * ####################################################################### */

/* #######################################################################
 * RTCP STATE VARIABLES
 * ####################################################################### */
#define RTP_OK           0
#define RTP_ERR          1

/* #######################################################################
 * RTP Configuration Database
 * ####################################################################### */
typedef struct {
	/* statistics for Sender Report in RTCP SR packet */
	unsigned int SRPkts;
	unsigned int SRBytes;
	/* statistics for Receiver Report in RTCP RR packet */
	unsigned int RRPkts;
	unsigned int RRBytes;

	unsigned int ErrRRPkts;
	unsigned int RFrms;
	unsigned long RTSs;

	unsigned int SCRFrms;
	unsigned long SCRRTSs;
} RTP_DATA;

/* ##### definitions of State in RTP_MEDIA ##### */
#define RTP_HOST_STATE_INIT    1
#define RTP_HOST_STATE_RUN     2
#define RTP_HOST_STATE_CLOSE   3
typedef struct {
	int          State;
	int          Sock;
	int          Port;
	/* SSRC */
	unsigned int SrvSSRC;
	int          Seq;
	/* Timer and Statictics */
	RTP_DATA     Data;

	int Type;
	int DataType;
} RTP_MEDIA;

typedef struct {
	int        RTPProt;     /* RTP streaming protocol, refer to NetRtsp.h  */
	char       SrvIP[16];   /* RTP Server's IP adress, depends on the RTP
							   protocol, it could be unicast IP or Mcast IP */
	char       HostIP[16];  /* RTCP Host IP address */
	RTP_MEDIA  Media[2];
} RTP_HOST_CONF;

/* ######################################################################
 * RTP HEADER
 * ###################################################################### */
typedef struct {
	unsigned short CsrcCnt:4; /* CSRC count                 */
	unsigned short Ext:1;     /* header extension flag      */
	unsigned short Pad:1;     /* padding flag               */
	unsigned short Ver:2;     /* RTP versio = 2             */
	unsigned short PT:7;      /* payload type               */
	unsigned short Marker:1;  /* marker bit                 */
	unsigned short Seq;       /* sequence number            */
	unsigned int   TS;        /* timestamp                  */
	unsigned int   SSRC;      /* synchronization source     */
	/* The csrc list, header extension and data follow, but can't
	 * be represented in the struct. */
} RTP_HEADER;

#define RTP_HEADER_LEN         sizeof(RTP_HEADER)

/* #######################################################################
 * RTP FRAME
 * ####################################################################### */
/* ##### definition of Type in RTP_FRAME #### */
#define RTP_FRAME_UNKNOWN   0
#define RTP_FRAME_AUDIO     1
#define RTP_FRAME_VIDEO_I	2
#define RTP_FRAME_VIDEO_P   3
#define RTP_FRAME_B2		4

typedef struct {
	int Size;   /* Length of the MAX size of Buf or 
				   Length of RTP Payload + B2Frame(44B) */
	int Len;
	int  Type;  /* Frame Type */
	unsigned char *Buf;	/* location of the ACTi Frame Payload. 
						   In Video, It will be B2+MPEG4 Raw Data.
						   In Audio, it will be Audio Raw data */
} RTP_FRAME;

