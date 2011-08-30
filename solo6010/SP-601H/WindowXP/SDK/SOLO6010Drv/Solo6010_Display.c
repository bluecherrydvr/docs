#include "Solo6010_Display.h"


#define	MASK2(a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15)	((a0<<30)|(a1<<28)|(a2<<26)|(a3<<24)|(a4<<22)|(a5<<20)|(a6<<18)|(a7<<16)|(a8<<14)|(a9<<12)|(a10<<10)|(a11<<8)|(a12<<6)|(a13<<4)|(a14<<2)|(a15<<0))


static int InitVideoOutConfig (DEVICE_EXTENSION *pdx);


int S6010_InitDisplay (DEVICE_EXTENSION *pdx)
{
	InitVideoOutConfig (pdx);

	return 0;
}

static int InitVideoOutConfig (DEVICE_EXTENSION *pdx)
{
	S6010_RegWrite (pdx, REG_VO_FMT_ENC,
		((!pdx->iS6010.typeVideo) ? 0 : VO_FMT_TYPE_PAL) |
		VO_USER_COLOR_SET_NAV |
		VO_USER_COLOR_SET_NAH |
		VO_NA_COLOR_Y(10) |
		VO_NA_COLOR_CB(16) |
		VO_NA_COLOR_CR(16));

	S6010_RegWrite (pdx, REG_VO_ACT_H,
		VO_H_START(pdx->iS6010.posVOutH)
		|VO_H_STOP(pdx->iS6010.posVOutH +pdx->iS6010.szVideoH));

	S6010_RegWrite (pdx, REG_VO_ACT_V,
		VO_V_START(pdx->iS6010.posVOutV)
		|VO_V_STOP(pdx->iS6010.posVOutV +pdx->iS6010.szVideoV));

	S6010_RegWrite (pdx, REG_VO_RANHE_HV,
		VO_H_LEN(pdx->iS6010.szVideoH) |VO_V_LEN(pdx->iS6010.szVideoV));	// vbnk.vstart,vstop

	//	color for cell border, fill
	//solo_reg_write(solo6010, REG_VO_BORDER_LINE_COLOR, (0x80<<24) | (0x80<<16) | (0x80<<8) | 0x80);	//	line color for cell
	//solo_reg_write(solo6010, REG_VO_BORDER_FILL_COLOR, (0x40<<24) | (0x80<<16) | (0x40<<8) | 0x80);	//	fill color for cell
	S6010_RegWrite (pdx, REG_VO_BKG_COLOR, (16 <<24) |(128 <<16) |(16 <<8) |128);

	S6010_RegWrite (pdx, REG_VO_DISP_ERASE, VO_DISP_ERASE_ON);

	S6010_RegWrite (pdx, REG_VO_ZOOM_CTRL, 0);
	S6010_RegWrite (pdx, REG_VO_FREEZE_CTRL, 0);

#ifdef __USE_SINGLE_DISP_PAGE__
	S6010_RegWrite (pdx, REG_VO_DISP_CTRL, VO_DISP_ON |VO_DISP_ERASE_COUNT(8) |VO_DISP_BASE((VMEM_DISP_WR_BASE >>16) &0xffff) |VO_DISP_SINGLE_PAGE);
#else
	S6010_RegWrite (pdx, REG_VO_DISP_CTRL, VO_DISP_ON |VO_DISP_ERASE_COUNT(8) |VO_DISP_BASE((VMEM_DISP_WR_BASE >>16) &0xffff));
#endif

	// set default border
	S6010_RegWrite (pdx, REG_VO_BORDER_LINE_COLOR, (0xa0 <<24) |(0x88 <<16) |(0xa0 <<8) |0x88);
	S6010_RegWrite (pdx, REG_VO_BORDER_FILL_COLOR, (0x10 <<24) |(0x8f <<16) |(0x10 <<8) |0x8f);

	// set default cursor
	S6010_RegWrite (pdx, REG_VO_CURSOR_POS, (0 <<11) |(0 <<0));	//	pos
	S6010_RegWrite (pdx, REG_VO_CURSOR_CLR, (0x80 <<24) |(0x80 <<16) |(0x10 <<8) |0x80);
	S6010_RegWrite (pdx, REG_VO_CURSOR_CLR2,                          (0xe0 <<8) |0x80);
/*
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK( 0), MASK2(1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK( 1), MASK2(1,3,1,0,0,0,0,0,0,0,0,0,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK( 2), MASK2(1,3,3,1,0,0,0,0,0,0,0,0,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK( 3), MASK2(1,3,3,3,1,0,0,0,0,0,0,0,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK( 4), MASK2(1,3,3,3,3,1,0,0,0,0,0,0,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK( 5), MASK2(1,3,3,3,3,3,1,0,0,0,0,0,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK( 6), MASK2(1,3,3,3,3,3,3,1,0,0,0,0,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK( 7), MASK2(1,3,3,3,3,3,3,3,1,0,0,0,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK( 8), MASK2(1,3,3,3,3,3,3,3,3,1,0,0,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK( 9), MASK2(1,3,3,3,3,3,3,3,3,3,1,0,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK(10), MASK2(1,3,3,3,3,3,3,1,1,1,1,1,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK(11), MASK2(1,3,3,3,1,3,3,1,0,0,0,0,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK(12), MASK2(1,3,3,1,1,3,3,1,0,0,0,0,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK(13), MASK2(1,3,1,0,0,1,3,3,1,0,0,0,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK(14), MASK2(1,1,0,0,0,1,3,3,1,0,0,0,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK(15), MASK2(1,0,0,0,0,0,1,3,3,1,0,0,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK(16), MASK2(0,0,0,0,0,0,1,3,3,1,0,0,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK(17), MASK2(0,0,0,0,0,0,0,1,3,3,1,0,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK(18), MASK2(0,0,0,0,0,0,0,1,3,3,1,0,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK(19), MASK2(0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0));
*/
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK( 0), MASK2(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK( 1), MASK2(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK( 2), MASK2(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK( 3), MASK2(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK( 4), MASK2(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK( 5), MASK2(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK( 6), MASK2(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK( 7), MASK2(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK( 8), MASK2(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK( 9), MASK2(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK(10), MASK2(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK(11), MASK2(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK(12), MASK2(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK(13), MASK2(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK(14), MASK2(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK(15), MASK2(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK(16), MASK2(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK(17), MASK2(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK(18), MASK2(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0));
	S6010_RegWrite (pdx, REG_VO_CURSOR_MASK(19), MASK2(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0));

	S6010_RegWrite (pdx, REG_VI_WIN_SW, 0);

//	S6010_RegWrite (pdx, REG_OSG_CONFIG, VO_OSG_ON);	// Video OSG On

	return 0;
}

void S6010_SetInfoLive (DEVICE_EXTENSION *pdx, INFO_LIVE *pInfoLive)
{
	int i, numShowDisp, idxScale, numDiv, numMulPos, numMulSz, idxPos,
		szH, szV, szCellH, szCellV, nSX, nEX, nSY, nEY;

	szH = pdx->iS6010.szVideoH;
	szV = pdx->iS6010.szVideoV *2;

	numShowDisp = GL_NUM_DISP_FROM_DISP_MODE[pInfoLive->idxSplitMode];
	numDiv = GL_NUM_DIV_FROM_DISP_MODE[pInfoLive->idxSplitMode];
	
	szCellH = (szH /numDiv) /8 *8;
	szCellV = (szV /numDiv) /8 *8;

//	S6010_RegWrite (pdx, REG_VO_DISP_ERASE, VO_DISP_ERASE_ON);

	for (i=0; i<numShowDisp; i++)
	{
		numMulPos = GL_NUM_MUL_POS_FROM_DISP_MODE_IDX_WND[pInfoLive->idxSplitMode][i];
		numMulSz = GL_NUM_MUL_SZ_FROM_DISP_MODE_IDX_WND[pInfoLive->idxSplitMode][i];
		idxPos = GL_IDX_POS_FROM_DISP_MODE_IDX_WND[pInfoLive->idxSplitMode][i];
		idxScale = GL_IDX_SCALE_FROM_DISP_MODE_IDX_WND[pInfoLive->idxSplitMode][i];

		nSX = szCellH *numMulPos *(idxPos %numDiv);
		nEX = nSX +szCellH *numMulSz;
		nSY = szCellV *numMulPos *(idxPos /numDiv);
		nEY = nSY +szCellV *numMulSz;

		S6010_RegWrite (pdx, VI_WIN_CTRL0(i),
			VI_WIN_CHANNEL(pInfoLive->bufIdxCh[i]) |
			VI_WIN_PIP(0) |
			VI_WIN_SCALE(idxScale) |
			VI_WIN_SX(nSX) |
			VI_WIN_EX(nEX));
		S6010_RegWrite (pdx, VI_WIN_CTRL1(i),
			VI_WIN_SY(nSY) |
			VI_WIN_EY(nEY));

		if (((pInfoLive->flagLiveOrDec >>i) &0x1) == 0x1)
		{
			S6010_RegWrite (pdx, VI_LIVE_ON(i), 2);
		}
		else
		{
			S6010_RegWrite (pdx, VI_LIVE_ON(i), 1);
		}
	}

	for (; i<NUM_VID_REAL_ENC_CH; i++)
	{
		S6010_RegWrite (pdx, VI_WIN_CTRL0(i),
			VI_WIN_CHANNEL(pInfoLive->bufIdxCh[i]) |
			VI_WIN_PIP(0) |
			VI_WIN_SCALE(0) |
			VI_WIN_SX(0) |
			VI_WIN_EX(0));
		S6010_RegWrite (pdx, VI_LIVE_ON(i), 0 <<0);
	}

	S6010_RegWrite (pdx, REG_VO_DISP_ERASE, 0);

	ExAcquireFastMutex (&pdx->bufFMutex[FMUX_INFO_LIVE]);
	memcpy (&pdx->iLive, pInfoLive, sizeof(INFO_LIVE));
	ExReleaseFastMutex (&pdx->bufFMutex[FMUX_INFO_LIVE]);
}