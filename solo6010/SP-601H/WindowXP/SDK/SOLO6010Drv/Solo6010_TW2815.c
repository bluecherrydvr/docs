#include "Solo6010_TW2815.h"

#include "Solo6010.h"
#include "Solo6010_I2C.h"


#define CLOCK_DELAY		1	// n * 2ns (0 ~ 15)

#define DEFAULT_HDELAY          (32 - 8)
#define DEFAULT_HACTIVE         (720 + 16)

#define DEFAULT_VDELAY_NTSC     (7 - 2)
#define DEFAULT_VACTIVE_NTSC    (240 + 4)

#define DEFAULT_VDELAY_PAL      (4 - 2)
#define DEFAULT_VACTIVE_PAL     (288 + 6)

#define NUM_I2C_WRITE_RETRY		8

int S6010_SetupTW2815 (DEVICE_EXTENSION *pdx, int nIDW, int nChannel)
{
	unsigned char tbl_ntsc_tw2815_common[] = {	// CH1			CH2			CH3			CH4
//		0x00,0xcc,0x20,0xc0,					// 0x00~0x03	0x10~0x13	0x20~0x23	0x30~0x33	// AGC ON, 7.5 IRE, 704
		0x00,0xc9,0x20,0xd0,					// 0x00~0x03	0x10~0x13	0x20~0x23	0x30~0x33	// AGC ON, 0 IRE, 720 (OLD)
		0x06,0xf0,0x08,0x80,					// 0x04~0x07	0x14~0x17	0x24~0x27	0x34~0x37
		0x80,0x80,0x80,0x02,					// 0x08~0x0b	0x18~0x1b	0x28~0x2b	0x38~0x3b	// YPEAK_GN 25% (OLD)
//		0x80,0x80,0x80,0x0c,					// 0x08~0x0b	0x18~0x1b	0x28~0x2b	0x38~0x3b	// YPEAK_GN 150%
		0x06,0x00,0x11							// 0x0c~0x0e	0x1c~0x1e	0x2c~0x2e	0x3c~0x3e
	};

	unsigned char tbl_pal_tw2815_common[] = {	// CH1			CH2			CH3			CH4
		0x00,0x88,0x20,0xd0,					// 0x00~0x03	0x10~0x13	0x20~0x23	0x30~0x33
		0x05,0x20,0x28,0x80,					// 0x04~0x07	0x14~0x17	0x24~0x27	0x34~0x37
		0x80,0x80,0x80,0x82,					// 0x08~0x0b	0x18~0x1b	0x28~0x2b	0x38~0x3b
		0x06,0x00,0x11							// 0x0c~0x0e	0x1c~0x1e	0x2c~0x2e	0x3c~0x3e
	};

	unsigned char tbl_tw2815_sfr[] = {
		0x00,0x00,0x00,0xc0,		//...0x40~0x43		// WPEAK_MD 00 (OLD)
//		0x00,0xff,0x00,0xc0,		//...0x40~0x43		// WPEAK_MD 11
		0x45,0xa0,0xd0,0x2f,		//...0x44~0x47
		0x64,0x80,0x80,0x82,		//...0x48~0x4b
		0x82,0x80,0x00,0x00,		//...0x4c~0x4f
		0x00,0x0f,0x05,0x00,		//...0x50~0x53
		0x00,0x80,0x06,0x00,		//...0x54~0x57
		0xc0,0x00,0x00,0x00,		//...0x58~0x5b
		0x8f,0x00,0x00,0x00,		//...0x5c~0x5f
		0x88,0x88,0xc0,0x00,		//...0x60~0x63
		0x10,0x32,0x54,0x76,		//...0x64~0x67
		0x98,0xba,0xdc,0xfe,		//...0x68~0x6b
		0x00,0x00,0x88,0x88,		//...0x6c~0x6f
		0x88,0x11,0x40,0x88,		//...0x70~0x73	// Audio output: Ch0 to Ch15 mix
//		0x88,0x10,0x40,0x88,		//...0x70~0x73	// Audio output: Ch16
		0x88,0x00					//...0x74~0x75
	};

	unsigned char *tbl_tw2815_common;

	int i, j;
	int ch;

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

	tbl_tw2815_common = (pdx->iS6010.typeVideo == VIDEO_TYPE_NTSC) ?
		tbl_ntsc_tw2815_common : tbl_pal_tw2815_common;

	tbl_tw2815_common[0x0d] |= 1<<2;	// Dual ITU-R BT.656 format
	tbl_tw2815_sfr[0x4d-0x40] = CLOCK_DELAY;

	// Audio multi-chip operation
#if __NUM_TW2815_CHIPS == 1
	tbl_tw2815_sfr[0x62-0x40] |= (3 <<6);

	tbl_tw2815_sfr[0x63-0x40] |= 3;				// ADATR : 16 audios
#elif __NUM_TW2815_CHIPS == 2
	tbl_tw2815_sfr[0x62-0x40] &= ~(3 <<6);

	if (nIDW == TW2815_A_IIC_ID)
	{
		tbl_tw2815_sfr[0x62-0x40] |= 1 <<6;	// first stage
	}
	else
	{
		tbl_tw2815_sfr[0x62-0x40] |= 2 <<6;	// last stage
	}

	tbl_tw2815_sfr[0x63-0x40] |= 3;				// ADATR : 16 audios
#elif __NUM_TW2815_CHIPS == 3
	tbl_tw2815_sfr[0x62-0x40] &= ~(3 <<6);
	switch(nIDW)
	{
		case TW2815_A_IIC_ID:
			tbl_tw2815_sfr[0x62-0x40] |= 1 <<6;	// first stage
			break;
		case TW2815_B_IIC_ID:
			tbl_tw2815_sfr[0x62-0x40] |= 0 <<6;	// middle stage
			break;
		case TW2815_C_IIC_ID:
			tbl_tw2815_sfr[0x62-0x40] |= 2 <<6;	// last stage
			break;
	}

	tbl_tw2815_sfr[0x63-0x40] |= 3;				// ADATR : 16 audios
#elif __NUM_TW2815_CHIPS == 4
	tbl_tw2815_sfr[0x62-0x40] &= ~(3 <<6);
	switch(nIDW)
	{
		case TW2815_A_IIC_ID:
			tbl_tw2815_sfr[0x62-0x40] |= 1 <<6;	// first stage
			break;
		case TW2815_D_IIC_ID:
			tbl_tw2815_sfr[0x62-0x40] |= 2 <<6;	// last stage
			break;
		default:
			tbl_tw2815_sfr[0x62-0x40] |= 0 <<6;	// middle stage
	}

	tbl_tw2815_sfr[0x63-0x40] |= 3;				// ADATR : 16 audios
#endif

	// solo6010 compatible
	tbl_tw2815_sfr[0x64-0x40] = 0x20;
	tbl_tw2815_sfr[0x65-0x40] = 0x64;
	tbl_tw2815_sfr[0x66-0x40] = 0xa8;
	tbl_tw2815_sfr[0x67-0x40] = 0xec;
	tbl_tw2815_sfr[0x68-0x40] = 0x31;
	tbl_tw2815_sfr[0x69-0x40] = 0x75;
	tbl_tw2815_sfr[0x6a-0x40] = 0xb9;
	tbl_tw2815_sfr[0x6b-0x40] = 0xfd;

//	tbl_tw2815_sfr[0x62-0x40] |= 1<<2;	// Recording 16KHz
//	tbl_tw2815_sfr[0x6c-0x40] |= 1<<2;	// Playback 16KHz

	tbl_tw2815_sfr[0x6c-0x40] |= 1<<5;		// playback right channel

	// test
	//tbl_tw2815_common[0x0d] |= 1<<6;	// background color on

	tbl_tw2815_sfr[0x5c-0x40] |= 1<<5;	// reserved value

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
			int ret;

			if(i == 0x00)	continue;	// read-only

			for (j=0; j<NUM_I2C_WRITE_RETRY; j++)
			{
				S6010_I2C_Write (pdx, nChannel, nIDW, (ch *0x10) +i, tbl_tw2815_common[i]);
				ret = S6010_I2C_Read (pdx, nChannel, nIDW, nIDW +1, (ch *0x10) +i);

				if (ret != tbl_tw2815_common[i])
				{
					KdPrint((DBG_NAME " reg %02x : %02x Try:%d\n", (ch *0x10) +i, ret, j));
				}
				else
				{
					break;
				}
			}
			if (j == NUM_I2C_WRITE_RETRY)
			{
				KdPrint((DBG_NAME " reg %02x : %02x [FAIL]\n", (ch *0x10) +i, ret));
			}
		}
	}

	for(i=0x40; i<0x76; i++)
	{
		int ret;

		if(i == 0x40)	continue;	// read-only
		if(i == 0x59)	continue;	// read-only
		if(i == 0x5A)	continue;	// read-only
		if(i == 0x5d)	continue;	// none
		if(i == 0x5e)	continue;	// none
		if(i == 0x5f)	continue;	// none

		for (j=0; j<NUM_I2C_WRITE_RETRY; j++)
		{
			S6010_I2C_Write (pdx, nChannel, nIDW, i, tbl_tw2815_sfr[i -0x40]);
			ret = S6010_I2C_Read (pdx, nChannel, nIDW, nIDW +1, i);
			if (ret != tbl_tw2815_sfr[i -0x40])
			{
					KdPrint((DBG_NAME " reg %02x : %02x Try:%d\n", i, ret, j));
			}
			else
			{
				break;
			}
		}
		if (j == NUM_I2C_WRITE_RETRY)
		{
			KdPrint((DBG_NAME " reg %02x : %02x [FAIL]\n", i, ret));
		}
	}

	return 0;
}

int S6010_GetColor (DEVICE_EXTENSION *pdx, INFO_COLOR *pInfoColor)
{
	int nIDW;

	switch (pInfoColor->idxCh /4)
	{
	case 0:
		nIDW = TW2815_A_IIC_ID;
		break;
	case 1:
		nIDW = TW2815_B_IIC_ID;
		break;
	case 2:
		nIDW = TW2815_C_IIC_ID;
		break;
	case 3:
		nIDW = TW2815_D_IIC_ID;
		break;
	}

	if (pInfoColor->bAccHue)
	{
		pInfoColor->nHue = S6010_I2C_Read (pdx, 0, nIDW, nIDW +1,
			0x07 +(pInfoColor->idxCh %4) *0x10);
	}
	if (pInfoColor->bAccSaturation)
	{
		pInfoColor->nSaturation = S6010_I2C_Read (pdx, 0, nIDW, nIDW +1,
			0x08 +(pInfoColor->idxCh %4) *0x10);
	}
	if (pInfoColor->bAccContrast)
	{
		pInfoColor->nContrast = S6010_I2C_Read (pdx, 0, nIDW, nIDW +1,
			0x09 +(pInfoColor->idxCh %4) *0x10);
	}
	if (pInfoColor->bAccBrightness)
	{
		pInfoColor->nBrightness = S6010_I2C_Read (pdx, 0, nIDW, nIDW +1,
			0x0a +(pInfoColor->idxCh %4) *0x10);
	}
		
	return 0;
}

int S6010_SetColor (DEVICE_EXTENSION *pdx, INFO_COLOR *pInfoColor)
{
	int nIDW;

	switch (pInfoColor->idxCh /4)
	{
	case 0:
		nIDW = TW2815_A_IIC_ID;
		break;
	case 1:
		nIDW = TW2815_B_IIC_ID;
		break;
	case 2:
		nIDW = TW2815_C_IIC_ID;
		break;
	case 3:
		nIDW = TW2815_D_IIC_ID;
		break;
	}

	if (pInfoColor->bAccHue)
	{
		S6010_I2C_Write (pdx, 0, nIDW,
			0x07 +(pInfoColor->idxCh %4) *0x10, pInfoColor->nHue);
	}
	if (pInfoColor->bAccSaturation)
	{
		S6010_I2C_Write (pdx, 0, nIDW,
			0x08 +(pInfoColor->idxCh %4) *0x10, pInfoColor->nSaturation);
	}
	if (pInfoColor->bAccContrast)
	{
		S6010_I2C_Write (pdx, 0, nIDW,
			0x09 +(pInfoColor->idxCh %4) *0x10, pInfoColor->nContrast);
	}
	if (pInfoColor->bAccBrightness)
	{
		S6010_I2C_Write (pdx, 0, nIDW,
			0x0a +(pInfoColor->idxCh %4) *0x10, pInfoColor->nBrightness);
	}
		
	return 0;
}

int S6010_GetVLoss (DEVICE_EXTENSION *pdx, INFO_VLOSS *pInfoVLoss)
{
#if __NUM_TW2815_CHIPS == 1
	pInfoVLoss->maskVLoss =
		 ((S6010_I2C_Read (pdx, 0, TW2815_A_IIC_ID, TW2815_A_IIC_ID +1, 0x5a) &0xf) <<0);
#elif __NUM_TW2815_CHIPS == 2
	pInfoVLoss->maskVLoss =
		 ((S6010_I2C_Read (pdx, 0, TW2815_A_IIC_ID, TW2815_A_IIC_ID +1, 0x5a) &0xf) <<0)
		|((S6010_I2C_Read (pdx, 0, TW2815_B_IIC_ID, TW2815_B_IIC_ID +1, 0x5a) &0xf) <<4);
#elif __NUM_TW2815_CHIPS == 3
	pInfoVLoss->maskVLoss =
		 ((S6010_I2C_Read (pdx, 0, TW2815_A_IIC_ID, TW2815_A_IIC_ID +1, 0x5a) &0xf) <<0)
		|((S6010_I2C_Read (pdx, 0, TW2815_B_IIC_ID, TW2815_B_IIC_ID +1, 0x5a) &0xf) <<4)
		|((S6010_I2C_Read (pdx, 0, TW2815_C_IIC_ID, TW2815_C_IIC_ID +1, 0x5a) &0xf) <<8);
#elif __NUM_TW2815_CHIPS == 4
	pInfoVLoss->maskVLoss =
		 ((S6010_I2C_Read (pdx, 0, TW2815_A_IIC_ID, TW2815_A_IIC_ID +1, 0x5a) &0xf) <<0)
		|((S6010_I2C_Read (pdx, 0, TW2815_B_IIC_ID, TW2815_B_IIC_ID +1, 0x5a) &0xf) <<4)
		|((S6010_I2C_Read (pdx, 0, TW2815_C_IIC_ID, TW2815_C_IIC_ID +1, 0x5a) &0xf) <<8)
		|((S6010_I2C_Read (pdx, 0, TW2815_D_IIC_ID, TW2815_D_IIC_ID +1, 0x5a) &0xf) <<12);
#endif

	return 0;
}
