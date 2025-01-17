#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/delay.h>

#include "solo6x10.h"
#include "sl_thread.h"

#define DEFAULT_HDELAY          (32 - 8)
#define DEFAULT_HACTIVE         (720 + 16)

#define DEFAULT_VDELAY_NTSC     (4 - 2)
#define DEFAULT_VACTIVE_NTSC    (240 + 4)

#define DEFAULT_VDELAY_PAL      (5 - 2)
#define DEFAULT_VACTIVE_PAL     (288 + 2)

int tw2865_setup(struct SOLO6x10 *solo6x10, int i2c_channel, u8 dev_addr)
{
	u8 tbl_ntsc_tw2865_common[] = {
		0x00,  0x00,  0x64,  0x13,
		0x80,  0x80,  0x00,  0x02,
		0x12,  0xF0,  0x0C,  0xD0,
		0x00,  0x00,  0x2F,  0x7F
	};

	u8 tbl_ntsc_tw2865_common2[] = {
		0x00,  0x40,  0x90,  0x08,  0x00,  0x38,  0x80,  0x80,  0x80,  0x80,  0x77,  0x00,  0x00,  0x00,  0x00,  0x00,   //04
		0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,   //05
		0x05,  0x03,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x21,  0x43,   //06
		0x08,  0x00,  0x00,  0x01,  0x01,  0x03,  0xEF,  0x03,  0xE9,  0x03,  0xD9,  0x15,  0x15,  0xE4,  0xA3,  0x80,   //07
		0x00,  0x02,  0x00,  0xCC,  0x00,  0x80,  0x44,  0x50,  0x22,  0x01,  0xD8,  0xBC,  0xB8,  0x44,  0x38,  0x00,   //08
		0x00,  0x78,  0x44,  0x3D,  0x14,  0xA5,  0xE0,  0x05,  0x00,  0x28,  0x44,  0x44,  0xA0,  0x90,  0x52,  0x00,   //09
		0x08,  0x08,  0x08,  0x08,  0x1A,  0x1A,  0x1B,  0x1A,  0x00,  0x00,  0x00,  0xF0,  0xF0,  0xF0,  0xF0,  0x44,   //0A
		0x44,  0x4A,  0x00,  0xFF,  0xEF,  0xEF,  0xEF,  0xEF,  0xFF,  0xE7,  0xE9,  0xE9,  0xEB,  0xFF,  0xD6,  0xD8,   //0B
		0xD8,  0xD7,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x39,  0xE4,  0x00,  0x80,   //0C
		0xA8,  0x88,  0x01,  0x10,  0x32,  0x10,  0x32,  0x10,  0x32,  0x10,  0x32,  0xC1,  0x00,  0x00,  0x00,  0x80,   //0D
		0x14,  0xC0,  0xAA,  0xAA,  0x00,  0x11,  0x00,  0x00,  0x11,  0x00,  0x00,  0x11,  0x00,  0x00,  0x11,  0x00,   //0E
		0x83,  0xB5,  0x09,  0x78,  0x85,  0x00,  0x01,  0x20,  0xC4,  0x51,  0x40,  0x2F,  0xFF,  0xF0,  0x00,  0xC0,   //0F
	};

	u8 tbl_pal_tw2865_common[] = {
		0x00,  0x00,  0x64,  0x13,
		0x80,  0x80,  0x00,  0x12,
		0x12,  0x20,  0x0C,  0xD0,
		0x00,  0x00,  0x27,  0x7F
	};

	u8 tbl_pal_tw2865_common2[] = {
		0x00,  0xD4,  0x90,  0x08,  0x00,  0x38,  0x7F,  0x80,  0x80,  0x80,  0x77,  0x00,  0x00,  0x00,  0x00,  0x00,   //04
		0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,   //05
		0x05,  0x03,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x21,  0x43,   //06
		0x08,  0x00,  0x00,  0x01,  0x01,  0x03,  0xEF,  0x03,  0xEA,  0x03,  0xD9,  0x15,  0x15,  0xE4,  0xA3,  0x80,   //07
		0x00,  0x02,  0x00,  0xCC,  0x00,  0x80,  0x44,  0x50,  0x22,  0x01,  0xD8,  0xBC,  0xB8,  0x44,  0x38,  0x00,   //08
		0x00,  0x78,  0x44,  0x3D,  0x14,  0xA5,  0xE0,  0x05,  0x00,  0x28,  0x44,  0x44,  0xA0,  0x90,  0x52,  0x00,   //09
		0x08,  0x08,  0x08,  0x08,  0x1A,  0x1A,  0x1A,  0x1A,  0x00,  0x00,  0x00,  0xF0,  0xF0,  0xF0,  0xF0,  0x44,   //0A
		0x44,  0x4A,  0x00,  0xFF,  0xEF,  0xEF,  0xEF,  0xEF,  0xFF,  0xE7,  0xE9,  0xE9,  0xE9,  0xFF,  0xD7,  0xD8,   //0B
		0xD9,  0xD8,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x39,  0xE4,  0x00,  0x80,   //0C
		0x88,  0x88,  0x01,  0x10,  0x32,  0x10,  0x32,  0x10,  0x32,  0x10,  0x32,  0xC1,  0x00,  0x00,  0x00,  0x80,   //0D
		0x14,  0xC0,  0xAA,  0xAA,  0x00,  0x11,  0x00,  0x00,  0x11,  0x00,  0x00,  0x11,  0x00,  0x00,  0x11,  0x00,   //0E
		0x83,  0xB5,  0x09,  0x00,  0xA0,  0x00,  0x01,  0x20,  0xC4,  0x51,  0x40,  0x2F,  0xFF,  0xF0,  0x00,  0xC0,   //0F
	};

	u8 *tbl_tw2865_common;
	u8 *tbl_tw2865_common2;

	int i;
	int ch;

	u8 dev_id;

	dev_id = (solo6x10_i2c_readbyte(solo6x10, i2c_channel, dev_addr, 0xFF) & 0xF8)>>3;
	dev_id |= (solo6x10_i2c_readbyte(solo6x10, i2c_channel, dev_addr, 0xFE) & 0xC0)>>1;

	if(dev_id != 0x18)
	{
		dbg_msg("dev_id : %d\n", dev_id);
		return -1;
	}

	dbg_msg("%02x - TW2865\n", dev_addr);

	tbl_tw2865_common = (solo6x10->video_type == 0) ? tbl_ntsc_tw2865_common : tbl_pal_tw2865_common;
	tbl_tw2865_common2 = (solo6x10->video_type == 0) ? tbl_ntsc_tw2865_common2 : tbl_pal_tw2865_common2;

	tbl_tw2865_common2[0xCA-0x40] = 0x55;	// dual 656
	tbl_tw2865_common2[0xCD-0x40] = 0xB1;	// 1nd channel selection
	tbl_tw2865_common2[0xCC-0x40] = 0xE4;	// 2nd channel selection

	tbl_tw2865_common2[0xFB-0x40] = 0x00;	// disable all interrupt

	tbl_tw2865_common2[0x43-0x40] = 0x68;

	tbl_tw2865_common2[0x96-0x40] |= 1<<1;	// loss color : blue

	{
		unsigned char clock_delay = 0x00;
		tbl_tw2865_common2[0x9F-0x40] = clock_delay;
		tbl_tw2865_common2[0x67-0x40] = clock_delay;
		tbl_tw2865_common2[0x68-0x40] = clock_delay;
		tbl_tw2865_common2[0x69-0x40] = clock_delay;
	}

// AUDIO CONFIGURATION
	tbl_tw2865_common2[0x70-0x40] = 0x09;	// 16KHz

	tbl_tw2865_common2[0xD2-0x40] = 0x03;	// 16channel on ADATR

	// audio sequence on ADATR
	tbl_tw2865_common2[0xD3-0x40] = 0x20;
	tbl_tw2865_common2[0xD4-0x40] = 0x64;
	tbl_tw2865_common2[0xD5-0x40] = 0xa8;
	tbl_tw2865_common2[0xD6-0x40] = 0xec;
	tbl_tw2865_common2[0xD7-0x40] = 0x31;
	tbl_tw2865_common2[0xD8-0x40] = 0x75;
	tbl_tw2865_common2[0xD9-0x40] = 0xb9;
	tbl_tw2865_common2[0xDA-0x40] = 0xfd;

	if(dev_addr == 0x28)
		tbl_tw2865_common2[0xDB-0x40] = 0xd1;	// audio DAC & ADC enable, playback right channel, ASYNR & ACLKR output
	else
		tbl_tw2865_common2[0xDB-0x40] = 0xd2;	// audio DAC & ADC enable, playback right channel, ASYNR & ACLKR output

	tbl_tw2865_common2[0xE0-0x40] = 0x13;	// 0x00 ~ 0x0f : live, 0x10 ~ 0x13 : playback, 0x14 : mixed audio
//

	for(ch=0; ch<4; ch++)
	{
		for(i=0; i<16; i++)
		{
			if(i == 0x00)	continue;	// VIDSTAT (read-only)
			if(i == 0x0C)	continue;	// MVSN (read-only)
			if(i == 0x0D)	continue;	// STATUS (read-only)

			solo6x10_i2c_writebyte(solo6x10, i2c_channel, dev_addr, (ch*0x10) + i, tbl_tw2865_common[i]);
		}
	}

	for(i=0x40; i<256; i++)
	{
		solo6x10_i2c_writebyte(solo6x10, i2c_channel, dev_addr, i, tbl_tw2865_common2[i-0x40]);
	}

	return 0;
}

