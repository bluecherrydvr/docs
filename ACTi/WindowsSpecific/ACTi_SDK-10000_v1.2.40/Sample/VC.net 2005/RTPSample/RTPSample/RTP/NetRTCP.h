#pragma once

// vim: ts=4 sw=4
/* #######################################################################
 * NetRtcp.h
 * Created by MingYoung   Date:2007/0410
 * Description:
 *    Header file for NetRTCP Library.
 * ####################################################################### */

/* #######################################################################
 * RTCP STATE VARIABLES
 * ####################################################################### */
#define RTCP_OK           0
#define RTCP_ERR          1

/* #######################################################################
 * RTCP TIMER VARIABLES
 * ####################################################################### */
#define MIN_SR_INTERVAL   6    /* minimum of interval in sec for sending SR */
#define MIN_RR_INTERVAL   6    /* minimum of interval in sec for sending RR */

/* #######################################################################
 * RTCP Configuration Database 
 * ####################################################################### */
typedef struct {
	/* timer of sending/receiving  the Sender Report packet */
	time_t SRTimer;
	int    SRInterval;
	/* timer of sending/receiving Receiver Report packet */
	time_t RRTimer;
	int    RRInterval;
	/* statistics for Sender Report in RTCP SR packet */
	unsigned int SRPkts;
	unsigned int SRBytes;
	/* statistics for Receiver Report in RTCP RR packet */
	unsigned int RRPkts;
	unsigned int RRBytes;
	unsigned int ErrRRPkts;
} RTCP_DATA;

typedef struct {
	int          Sock;
	int          SrvPort;
	int          HostPort;
	/* SSRC */
	unsigned int SrvSSRC;
	unsigned int HostSSRC;
	int          Seq;      /* last sequence of the RTP packet received */
	/* Timer and Statictics */
	RTCP_DATA    Data;
} RTCP_MEDIA;
/* ##### definitions of State in RTCP_HOST_CONF ##### */
#define RTCP_HOST_STATE_INIT	1
#define RTCP_HOST_STATE_RUN		2
#define RTCP_HOST_STATE_CLOSE	3

typedef struct {
	int        State;
	int        RTPProt;     /* RTP streaming protocol, refer to NetRtsp.h  */
	char       SrvIP[16];	/* RTP Server's IP adress, depends on the RTP
							   protocol, it could be unicast IP or Mcast IP */
	char       HostIP[16];	/* RTCP Host IP address */
	RTCP_MEDIA Media[2];
} RTCP_HOST_CONF;

/* #######################################################################
 * RTCP PACKETS Format
 * RTCP HEADER
 * ####################################################################### */
/* ##### RTCP's payload type coding ##### */
#define RTCP_PT_SR             200
#define RTCP_PT_RR             201
#define RTCP_PT_SDES           202
#define RTCP_PT_BYE            203
#define RTCP_PT_APP            204

typedef struct {
	unsigned short Cnt:5; /* varies by payload type */
	unsigned short Pad:1; /* padding flag           */
	unsigned short Ver:2; /* packet type            */
	unsigned short PT:8;  /* payload type           */
	unsigned short Len;   /* packet length          */
} RTCP_HEADER_COMMON;

typedef struct {
	unsigned int SSRC;   /* ID */
	unsigned int NTPSec; /* timestamp in NTP format */
	unsigned int NTPFrac;
	unsigned int NTPTs;
	unsigned int Pkts;   /* packet counter */
	unsigned int Bytes;  /* byte counter */
} RTCP_SR;

typedef struct {
	unsigned int SSRC;           /* The ssrc to which this RR pertains */
	unsigned int TotalLost:24;
	unsigned int FractLost:8;
	unsigned int LastSeq;
	unsigned int Jitter;
	unsigned int LSR;
	unsigned int DLSR;
} RTCP_RR;

typedef struct {
	unsigned short SubType:5; /* application dependent  */
	unsigned short Pad:1;     /* padding flag           */
	unsigned short Ver:2;     /* RTP version            */
	unsigned short PT:8;      /* packet type            */
	unsigned short Len;       /* packet length          */
	unsigned int   SSRC;
	char           Name[4];   /* four ASCII characters  */
	char           Data[1];   /* variable length field  */
} RTCP_APP;

/* ##### definitions of SDES packet types ##### */
#define RTCP_SDES_END     0
#define RTCP_SDES_CNAME   1
#define RTCP_SDES_NAME    2
#define RTCP_SDES_EMAIL   3
#define RTCP_SDES_PHONE   4
#define RTCP_SDES_LOC     5
#define RTCP_SDES_TOOL    6
#define RTCP_SDES_NOTE    7
#define RTCP_SDES_PRIV    8

typedef struct {
	unsigned char Type;    /* type of SDES item              */
	unsigned char Len;     /* length of SDES item (in bytes) */
	char          Data[1]; /* text, not zero-terminated      */
} RTCP_SDES_ITEM;

typedef struct {
	RTCP_HEADER_COMMON Comm;
	union {
		struct {
			RTCP_SR    SRPkt;
			RTCP_RR    RRPkt; /* variable-length list */
		} SR;
		struct {
			unsigned int SSRC;  /* source this RTCP packet is coming from */
			RTCP_RR      RRPkt; /* variable-length list */
		} RR;
		struct {
			unsigned int    SSRC;
			RTCP_SDES_ITEM  Item;        /* list of SDES */
		} SDES;
		struct {
			unsigned int SSRC;        /* list of sources */
			/* can't express the trailing text... */
		} BYE;
		struct {
			unsigned int  SSRC;
			unsigned char Name[4];
			unsigned char Data[1];
		} APP;
	} Head;
} RTCP_HEADER;

typedef struct {
	/* RR Packet */
	RTCP_HEADER_COMMON Head;
	unsigned int       SSRC;
	RTCP_RR            RR;
	/* BYE packet */
	RTCP_HEADER_COMMON ByeHead;
	unsigned int       SSRCBye;
} RTCP_BYE_PKT;

typedef struct {
	/* RR packet */
	RTCP_HEADER_COMMON Head;
	unsigned int       SSRC;
	RTCP_RR            RR;
	/* SDES packet */
	RTCP_HEADER_COMMON SDESHead;
	unsigned int       SSRCSdes;
	unsigned char      SdesType;     /* type of SDES item              */
	unsigned char      SdesLen;      /* length of SDES item (in bytes) */
	char               SdesData[2];  /* text, not zero-terminated      */
} RTCP_RR_PKT;
