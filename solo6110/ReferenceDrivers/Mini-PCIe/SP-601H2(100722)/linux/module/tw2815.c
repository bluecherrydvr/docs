#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/module.h>

#include "solo6x10.h"

#define ENABLE_AGC				1

#define DEFAULT_HDELAY          (32 - 8)
#define DEFAULT_HACTIVE         (720 + 16)

#define DEFAULT_VDELAY_NTSC     (7 - 2)
#define DEFAULT_VACTIVE_NTSC    (240 + 4)

#define DEFAULT_VDELAY_PAL      (5 - 2)
#define DEFAULT_VACTIVE_PAL     (288 + 2)

int tw2815_setup(struct SOLO6x10 *solo6x10, int i2c_channel, u8 dev_addr)
{
	u8 tbl_ntsc_tw2815_common[] = {	// CH1			CH2			CH3			CH4
		0x00,0xc8,0x20,0xd0,		// 0x00~0x03	0x10~0x13	0x20~0x23	0x30~0x33
		0x06,0xf0,0x08,0x80,		// 0x04~0x07	0x14~0x17	0x24~0x27	0x34~0x37
		0x80,0x80,0x80,0x02,		// 0x08~0x0b	0x18~0x1b	0x28~0x2b	0x38~0x3b
		0x06,0x00,0x11				// 0x0c~0x0e	0x1c~0x1e	0x2c~0x2e	0x3c~0x3e
	};

	u8 tbl_pal_tw2815_common[] = {	// CH1			CH2			CH3			CH4
		0x00,0x88,0x20,0xd0,		// 0x00~0x03	0x10~0x13	0x20~0x23	0x30~0x33
		0x05,0x20,0x28,0x80,		// 0x04~0x07	0x14~0x17	0x24~0x27	0x34~0x37
		0x80,0x80,0x80,0x82,		// 0x08~0x0b	0x18~0x1b	0x28~0x2b	0x38~0x3b
		0x06,0x00,0x11				// 0x0c~0x0e	0x1c~0x1e	0x2c~0x2e	0x3c~0x3e
	};

	u8 tbl_tw2815_sfr[] = {
		0x00,0x00,0x00,0xc0,		//...0x40~0x43
		0x45,0xa0,0xd0,0x2f,		//...0x44~0x47
		0x64,0x80,0x80,0x82,		//...0x48~0x4b
		0x82,0x80,0x00,0x00,		//...0x4c~0x4f
		0x00,0x0f,0x05,0x00,		//...0x50~0x53
		0x00,0x80,0x06,0x00,		//...0x54~0x57
		0x00,0x00,0x00,0xff,		//...0x58~0x5b
		0x8f,0x00,0x00,0x00,		//...0x5c~0x5f
		0x88,0x88,0xc0,0x00,		//...0x60~0x63
		0x10,0x32,0x54,0x76,		//...0x64~0x67
		0x98,0xba,0xdc,0xfe,		//...0x68~0x6b
		0x00,0x00,0x88,0x88,		//...0x6c~0x6f
		0x88,0x11,0x40,0x88,		//...0x70~0x73
		0x88,0x00					//...0x74~0x75
	};

	u8 *tbl_tw2815_common;

	int i;
	int ch;

	u8 dev_id;

	dev_id = (solo6x10_i2c_readbyte(solo6x10, i2c_channel, dev_addr, 0x59) & 0xF8)>>3;
	dev_id |= (solo6x10_i2c_readbyte(solo6x10, i2c_channel, dev_addr, 0x58) & 0xC0)>>1;

	if(dev_id != 0x64)
	{
		dbg_msg("dev_id : %d\n", dev_id);
		return -1;
	}

	dbg_msg("%02x - TW2815\n", dev_addr);

	tbl_ntsc_tw2815_common[0x06] = 0;

	// Horizontal Delay Control
	tbl_ntsc_tw2815_common[0x02] = (unsigned char)(0xff & DEFAULT_HDELAY);
	tbl_ntsc_tw2815_common[0x06] |= (unsigned char)((0x03 & (DEFAULT_HDELAY>>8))<<0);

	// Horizontal Active Control
	tbl_ntsc_tw2815_common[0x03] = (unsigned char)(0xff & DEFAULT_HACTIVE);
	tbl_ntsc_tw2815_common[0x06] |= (unsigned char)((0x03 & (DEFAULT_HACTIVE>>8))<<2);

	// Vertical Delay Control
	tbl_ntsc_tw2815_common[0x04] = (unsigned char)(0xff & DEFAULT_VDELAY_NTSC);
	tbl_ntsc_tw2815_common[0x06] |= (unsigned char)((0x01 & (DEFAULT_VDELAY_NTSC>>8))<<4);

	// Vertical Active Control
	tbl_ntsc_tw2815_common[0x05] = (unsigned char)(0xff & DEFAULT_VACTIVE_NTSC);
	tbl_ntsc_tw2815_common[0x06] |= (unsigned char)((0x01 & (DEFAULT_VACTIVE_NTSC>>8))<<5);

	tbl_pal_tw2815_common[0x06] = 0;

	// Horizontal Delay Control
	tbl_pal_tw2815_common[0x02] = (unsigned char)(0xff & DEFAULT_HDELAY);
	tbl_pal_tw2815_common[0x06] |= (unsigned char)((0x03 & (DEFAULT_HDELAY>>8))<<0);

	// Horizontal Active Control
	tbl_pal_tw2815_common[0x03] = (unsigned char)(0xff & DEFAULT_HACTIVE);
	tbl_pal_tw2815_common[0x06] |= (unsigned char)((0x03 & (DEFAULT_HACTIVE>>8))<<2);

	// Vertical Delay Control
	tbl_pal_tw2815_common[0x04] = (unsigned char)(0xff & DEFAULT_VDELAY_PAL);
	tbl_pal_tw2815_common[0x06] |= (unsigned char)((0x01 & (DEFAULT_VDELAY_PAL>>8))<<4);

	// Vertical Active Control
	tbl_pal_tw2815_common[0x05] = (unsigned char)(0xff & DEFAULT_VACTIVE_PAL);
	tbl_pal_tw2815_common[0x06] |= (unsigned char)((0x01 & (DEFAULT_VACTIVE_PAL>>8))<<5);

	tbl_tw2815_common = (solo6x10->video_type == 0) ? tbl_ntsc_tw2815_common : tbl_pal_tw2815_common;

	tbl_tw2815_common[0x0d] |= 1<<2;	// Dual ITU-R BT.656 format
	tbl_tw2815_sfr[0x4d-0x40] = 0x01;		// Clock delay
#if (!ENABLE_AGC)
	tbl_tw2815_common[0x01] &= ~(1<<3);
#endif

// AUDIO CONFIGURATION
	// Audio multi-chip operation
	tbl_tw2815_sfr[0x62-0x40] &= ~(3<<6);
	switch(dev_addr)
	{
		case (0x28+0):
			tbl_tw2815_sfr[0x62-0x40] |= 1<<6;	// first stage
			break;
		case (0x28+3):
			tbl_tw2815_sfr[0x62-0x40] |= 2<<6;	// last stage
			break;
		default:
			tbl_tw2815_sfr[0x62-0x40] |= 0<<6;	// middle stage
	}

	tbl_tw2815_sfr[0x63-0x40] |= 3;				// ADATR : 16 audios

	// solo6x10 compatible
	tbl_tw2815_sfr[0x64-0x40] = 0x20;
	tbl_tw2815_sfr[0x65-0x40] = 0x64;
	tbl_tw2815_sfr[0x66-0x40] = 0xa8;
	tbl_tw2815_sfr[0x67-0x40] = 0xec;
	tbl_tw2815_sfr[0x68-0x40] = 0x31;
	tbl_tw2815_sfr[0x69-0x40] = 0x75;
	tbl_tw2815_sfr[0x6a-0x40] = 0xb9;
	tbl_tw2815_sfr[0x6b-0x40] = 0xfd;

	tbl_tw2815_sfr[0x62-0x40] |= 1<<2;	// 16KHz
	tbl_tw2815_sfr[0x6c-0x40] |= 1<<2;	// 16KHz

	tbl_tw2815_sfr[0x6c-0x40] |= 1<<5;		// playback right channel
// AUDIO CONFIGURATION END

	tbl_tw2815_sfr[0x5c-0x40] |= 1<<5;	// reserved value
	tbl_tw2815_sfr[0x5b-0x40] = 0x0f;

	for(ch=0; ch<4; ch++)
	{
		tbl_tw2815_common[0x0d] &= ~3;
		if(ch == 0)
			tbl_tw2815_common[0x0d] |= 1;
		if(ch == 1)
			tbl_tw2815_common[0x0d] |= 0;
		if(ch == 2)
			tbl_tw2815_common[0x0d] |= 3;
		if(ch == 3)
			tbl_tw2815_common[0x0d] |= 2;

		for(i=0; i<0x0f; i++)
		{
			if(i == 0x00)	continue;	// read-only

			solo6x10_i2c_writebyte(solo6x10, i2c_channel, dev_addr, (ch*0x10) + i, tbl_tw2815_common[i]);
		}
	}

	for(i=0x40; i<0x76; i++)
	{
		if(i == 0x40)	continue;	// read-only
		if(i == 0x59)	continue;	// read-only
		if(i == 0x5A)	continue;	// read-only
		if(i == 0x5d)	continue;	// none
		if(i == 0x5e)	continue;	// none
		if(i == 0x5f)	continue;	// none

		solo6x10_i2c_writebyte(solo6x10, i2c_channel, dev_addr, i, tbl_tw2815_sfr[i-0x40]);
	}

	return 0;
}
