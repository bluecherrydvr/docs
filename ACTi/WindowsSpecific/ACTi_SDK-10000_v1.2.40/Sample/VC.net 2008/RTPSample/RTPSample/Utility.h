#pragma once

typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned char  u_int8;
typedef unsigned short u_int16;
typedef unsigned int   u_int32;
typedef short int16;

typedef unsigned char uint8_t;
typedef long int int32_t;
typedef unsigned long int uint32_t;

/*
* RTP data header from RFC1889
*/
typedef struct {
	unsigned int version:2;		/* protocol version */
	unsigned int p:1;					/* padding flag */
	unsigned int x:1;					/* header extension flag */
	unsigned int cc:4;				/* CSRC count */
	unsigned int m:1;					/* marker bit */
	unsigned int pt:7;				/* payload type */
	u_int16 seq;							/* sequence number */
	u_int32 ts;								/* timestamp */
	u_int32 ssrc;							/* synchronization source */
	u_int32 csrc[1];					/* optional CSRC list */
} rtp_hdr_t;

#define RTP_HDR_SZ 12
/* The following definition is from RFC1890 */
#define RTP_PT_JPEG 26

struct jpeghdr
{
#if 0
	unsigned int tspec:8;			/* type-specific field */
	unsigned int off:24;			/* fragment byte offset */
	u_int8 type;							/* id of jpeg decoder params */
	u_int8 q;									/* quantization factor (or table id) */
	u_int8 width;							/* frame width in 8 pixel blocks */
	u_int8 height;						/* frame height in 8 pixel blocks */
#else
	unsigned int tspec:8;
	unsigned int off:24;
	unsigned char type;
	unsigned char q;
	unsigned char width;
	unsigned char height;
#endif
};

struct jpeghdr_rst
{
#if 0
	u_int16 dri;
	unsigned int f:1;
	unsigned int l:1;
	unsigned int count:14;
#else
	unsigned short dri;
	unsigned short f:1;
	unsigned short l:1;
	unsigned short count:14;
#endif
};

#define JPEG_QUANT_SIZE			172

struct jpeghdr_qtable
{
#if 1
	u_int8 mbz;
	u_int8 precision;
	u_int16 length;
#else
	unsigned char mbz;
	unsigned char precision;
	unsigned short length;
	unsigned char table[JPEG_QUANT_SIZE];
#endif
};

struct rtpjpeg
{
	jpeghdr head;
	jpeghdr_rst restart;
	jpeghdr_qtable quant;
};

int fnTrans( const unsigned char *buf, const int buflen, unsigned char *target, const int maxsize );
