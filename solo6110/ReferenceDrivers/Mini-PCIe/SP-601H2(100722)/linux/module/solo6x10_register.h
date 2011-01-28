#ifndef __SOLO6x10_REGISTER_H
#define __SOLO6x10_REGISTER_H

/*
 * SYSTEM
 */
#define	REG_SYS_CONFIG					0x0000
#define		SYS_CONFIG_RESET				(1<<31)		// SDRAM SW RESET
#define		SYS_CONFIG_SDRAM64BIT			(1<<30)		// SDRAM data width	select : 0=32 bit,1=64 bit mode
#define		SYS_CONFIG_PCLK_DELAY(n)		((n)<<26)
#define		SYS_CONFIG_NCLK_DELAY(n)		((n)<<24)
#define		SYS_CONFIG_CLOCK_DIVIDE			(1<<19)
#define		SYS_CONFIG_INPUTDIV(n)			((n)<<14)
#define		SYS_CONFIG_FEEDBACKDIV(n)		((n)<<5)
#define		SYS_CONFIG_OUTDIV(n)			((n)<<3)
#define		SYS_CONFIG_PLL_PWDN				(1<<2)
#define		SYS_CONFIG_PLL_BYPASS			(1<<1)
#define		SYS_CONFIG_FOUT_EN				(1<<0)

#define	REG_DMA_CTRL					0x0004
#define		DMA_CTRL_REFRESH_CYCLE(n)		((n)<<8)	// refresh cycle
#define		DMA_CTRL_SDRAM_SIZE(n)			((n)<<6)	// sdram size(32bit/64bit) : 0=16/32MB, 1=32/64MB, 2=64/128MB, 3=128/256MB
#define		DMA_CTRL_SDRAM_CLK_INVERT		(1<<5)		// sdram clock invert
#define		DMA_CTRL_STROBE_SELECT			(1<<4)		//
#define		DMA_CTRL_READ_DATA_SELECT		(1<<3)		//
#define		DMA_CTRL_READ_CLK_SELECT		(1<<2)		//
#define		DMA_CTRL_LATENCY(n)				((n)<<0)	// latency

#define REG_SYS_VCLK					0x000C
#define		SYS_CONFIG_VOUT_CLK_INVERT		(1<<22)		// video output clock invert
#define		SYS_CONFIG_VOUT_CLK_SELECT(n)	((n)<<20)	// video output clock select : 0=sys_clk/4, 1=sys_clk/2, 2=clk_in/2 of system input

#define		SYS_CONFIG_VIN1415_DELAY(n)		((n)<<14)	// video input clock delay ch14,15
#define		SYS_CONFIG_VIN1213_DELAY(n)		((n)<<12)	// video input clock delay ch12,13
#define		SYS_CONFIG_VIN1011_DELAY(n)		((n)<<10)	// video input clock delay ch10,11
#define		SYS_CONFIG_VIN0809_DELAY(n)		((n)<<8)	// video input clock delay ch8,9
#define		SYS_CONFIG_VIN0607_DELAY(n)		((n)<<6)	// video input clock delay ch6,7
#define		SYS_CONFIG_VIN0405_DELAY(n)		((n)<<4)	// video input clock delay ch4,5
#define		SYS_CONFIG_VIN0203_DELAY(n)		((n)<<2)	// video input clock delay ch2,3
#define		SYS_CONFIG_VIN0001_DELAY(n)		((n)<<0)	// video input clock delay ch0,1

/*
 *	INTERRUPT
 */
#define REG_INTR_STAT					0x0010
#define REG_INTR_ENABLE					0x0014
#define		INTR_P2M(n)						(1<<((n)+17))
#define		INTR_GPIO						(1<<16)
#define		INTR_VIDEO_LOSS					(1<<15)
#define		INTR_VIDEO_SYNC					(1<<14)
#define		INTR_MOTION						(1<<13)
#define		INTR_ATA_CMD					(1<<12)
#define		INTR_ATA_DIR					(1<<11)
#define		INTR_PCI_ERR					(1<<10)
#define		INTR_PS2_1						(1<<9)
#define		INTR_PS2_0						(1<<8)
#define		INTR_SPI						(1<<7)
#define		INTR_IIC						(1<<6)
#define		INTR_UART(n)					(1<<((n) + 4))
#define		INTR_G723						(1<<3)
#define		INTR_DECODER					(1<<1)
#define		INTR_ENCODER					(1<<0)

#define REG_CHIP_OPTION					0x001C

/*
 * ERROR
 */
#define REG_PCI_ERROR					0x0070
#define		PCI_ERROR_FSM2(stat)			(0x1f&((stat)>>24))
#define		PCI_ERROR_FSM1(stat)			(0x0f&((stat)>>20))
#define		PCI_ERROR_FSM0(stat)			(0x0f&((stat)>>16))
#define		PCI_ERROR_P2M_DESC				(1<<6)
#define		PCI_ERROR_ATA					(1<<5)
#define		PCI_ERROR_P2M					(1<<4)
#define		PCI_ERROR_TIMEOUT				(1<<3)
#define		PCI_ERROR_TARGET				(1<<2)
#define		PCI_ERROR_PARITY				(1<<1)
#define		PCI_ERROR_FATAL					(1<<0)

/*
 *	P2M
 */
#define REG_P2M_BASE					0x0080

#define REG_P2M_CONFIG(n)				(0x0080 + ((n)*0x20))
#define		P2M_DMA_INTERVAL(n)				((n)<<6)			// N*32 clocks
#define		P2M_CSC_BYTE_REORDER			(1<<5)				// BGR -> RGB
#define		P2M_CSC_16BIT_565				(1<<4)				// 0:r=[14:10] g=[9:5] b=[4:0], 1:r=[15:11] g=[10:5] b=[4:0]
#define		P2M_UV_SWAP						(1<<3)
#define		P2M_PCI_MASTER_MODE				(1<<2)
#define		P2M_DESC_INTR_OPT				(1<<1)				// 1:Empty Desc, 0:Each Command End
#define		P2M_DESC_MODE					(1<<0)

#define REG_P2M_DES_ADR(n)				(0x0084 + ((n)*0x20))

#define REG_P2M_DESC_ID(n)				(0x0088 + ((n)*0x20))
#define		P2M_SWAP(n)						((n)<<8)			//(6110only) 0:normal, 1:4byte, 2:2byte
#define		P2M_UPDATE_ID(n)				((n)<<0)

#define REG_P2M_STATUS(n)				(0x008C + ((n)*0x20))
#define		P2M_COMMAND_DONE				(1<<8)
#define		P2M_CURRENT_ID(stat)			(0xff & (stat))

#define REG_P2M_CONTROL(n)				(0x0090 + ((n)*0x20))
#define		P2M_PCI_INC(n)					((n)<<20)
#define		P2M_REPEAT(n)					((n)<<10)
#define		P2M_BURST_SIZE(n)				((n)<<7)			// 0:512, 1:256, 2:128, 3:64, 4:32, 5:128(2page)
#define			P2M_BURST_256				1
#define			P2M_BURST_128				2
#define			P2M_BURST_64				3
#define			P2M_BURST_32				4
#define		P2M_CSC_16BIT					(1<<6)				// 0:24bit, 1:16bit
#define		P2M_ALPHA_MODE(n)				((n)<<4)			// 0:Y[0]<-0(OFF), 1:Y[0]<-1(ON), 2:Y[0]<-G[0], 3:Y[0]<-Bit[15]
#define		P2M_CSC_ON						(1<<3)
#define		P2M_INTERRUPT_REQ				(1<<2)
#define		P2M_WRITE						(1<<1)
#define		P2M_TRANS_ON					(1<<0)

#define REG_P2M_EXT_CFG(n)				(0x0094 + ((n)*0x20))
#define		P2M_EXT_INC(n)					((n)<<20)
#define		P2M_COPY_SIZE(n)				((n)<<0)

#define REG_P2M_TAR_ADR(n)				(0x0098 + ((n)*0x20))

#define REG_P2M_EXT_ADR(n)				(0x009C + ((n)*0x20))

#define REG_P2M_BUFFER(i)				(0x2000 + ((i)*4))

/*
 * VIN
 */
#define REG_VI_CH_SWITCH_0				0x0100
#define REG_VI_CH_SWITCH_1				0x0104
#define REG_VI_CH_SWITCH_2				0x0108

#define	REG_VI_CH_ENA					0x010C
#define	REG_VI_CH_FORMAT				0x0110
#define		VI_FD_SEL_MASK(n)				((n)<<16)
#define		VI_PROG_MASK(n)					((n)<<0)

#define REG_VI_FMT_CFG					0x0114
#define		VI_FMT_CHECK_VCOUNT				(1<<31)
#define		VI_FMT_CHECK_HCOUNT				(1<<30)

#define	REG_VI_PAGE_SW					0x0118
#define		FI_INV_DISP_LIVE(n)				((n)<<8)
#define		FI_INV_DISP_OUT(n)				((n)<<7)
#define		DISP_SYNC_FI(n)					((n)<<6)
#define		PIP_PAGE_ADD(n)					((n)<<3)
#define		NORMAL_PAGE_ADD(n)				((n)<<0)

#define	REG_VI_ACT_I_P					0x011C
#define	REG_VI_ACT_I_S					0x0120
#define	REG_VI_ACT_P					0x0124
#define		VI_FI_INVERT					(1<<31)
#define		VI_H_START(n)					((n)<<21)
#define		VI_V_START(n)					((n)<<11)
#define		VI_V_STOP(n)					((n)<<0)

#define REG_VI_STATUS0					0x0128
#define REG_VI_STATUS1					0x012C

// Video PB to Live
#define VI_PB_CONFIG					0x0130
#define		VI_PB_USER_MODE					(1<<1)
#define		VI_PB_PAL						(1<<0)
#define VI_PB_RANGE_HV					0x0134
#define		VI_PB_HSIZE(h)					((h)<<12)
#define		VI_PB_VSIZE(v)					((v)<<0)
#define VI_PB_ACT_H						0x0138
#define		VI_PB_HSTART(n)					((n)<<12)
#define		VI_PB_HSTOP(n)					((n)<<0)
#define VI_PB_ACT_V						0x013C
#define		VI_PB_VSTART(n)					((n)<<12)
#define		VI_PB_VSTOP(n)					((n)<<0)

// Mosaic
#define	REG_VI_MOSAIC(ch)				(0x0140 + ((ch)*4))
#define		VI_MOSAIC_SX(x)					((x)<<24)
#define		VI_MOSAIC_EX(x)					((x)<<16)
#define		VI_MOSAIC_SY(x)					((x)<<8)
#define		VI_MOSAIC_EY(x)					((x)<<0)

#define		VI_WIN_CTRL0(ch)				(0x0180 + ((ch)*4))
#define		VI_WIN_CTRL1(ch)				(0x01C0 + ((ch)*4))

	#define		VI_WIN_CHANNEL(n)			((n)<<28)

	#define		VI_WIN_PIP(n)				((n)<<27)		//	27
	#define		VI_WIN_SCALE(n)				((n)<<24)		//	23..26

	#define		VI_WIN_SX(x)				((x)<<12)		//	12..23
	#define		VI_WIN_EX(x)				((x)<<0)		// 	 0..11

	#define		VI_WIN_SY(x)				((x)<<12)
	#define		VI_WIN_EY(x)				((x)<<0)

#define		VI_LIVE_ON(ch)				(0x0200 + ((ch)*4))

#define REG_VI_WIN_SW					0x0240
#define REG_VI_WIN_LIVE_AUTO_MUTE		0x0244

// Motion
#define	REG_VI_MOT_ADR					0x0260
#define		VI_MOTION_EN(mask)				((mask)<<16)
#define	REG_VI_MOT_CTRL					0x0264
#define		VI_MOTION_FRAME_COUNT(n)		((n)<<24)
#define		VI_MOTION_SAMPLE_LENGTH(n)		((n)<<16)
#define		VI_MOTION_INTR_START_STOP		(1<<15)
#define		VI_MOTION_FREEZE_DATA			(1<<14)
#define		VI_MOTION_SAMPLE_COUNT(n)		((n)<<0)
#define REG_VI_MOT_CLEAR				0x0268
#define REG_VI_MOT_STATUS				0x026C
#define		VI_MOTION_CNT(n)				((n)<<0)
#define REG_VI_MOTION_BORDER			0x0270
#define REG_VI_MOTION_BAR				0x0274
#define		VI_MOTION_Y_SET					(1<<29)
#define		VI_MOTION_Y_ADD					(1<<28)
#define		VI_MOTION_CB_SET				(1<<27)
#define		VI_MOTION_CB_ADD				(1<<26)
#define		VI_MOTION_CR_SET				(1<<25)
#define		VI_MOTION_CR_ADD				(1<<24)
#define		VI_MOTION_Y_VALUE(v)			((v)<<16)
#define		VI_MOTION_CB_VALUE(v)			((v)<<8)
#define		VI_MOTION_CR_VALUE(v)			((v)<<0)

/*
 *	VIDEO OUTPUT
 */
#define	REG_VO_FMT_ENC					0x0300
#define		VO_SCAN_MODE_PROGRESSIVE		(1<<31)
#define		VO_FMT_TYPE_PAL					(1<<30)
#define		VO_USER_SET						(1<<29)

#define		VO_FI_CHANGE					(1<<20)
#define		VO_USER_COLOR_SET_VSYNC			(1<<19)
#define		VO_USER_COLOR_SET_HSYNC			(1<<18)
#define		VO_USER_COLOR_SET_NAV			(1<<17)
#define		VO_USER_COLOR_SET_NAH			(1<<16)
#define		VO_NA_COLOR_Y(Y)				((Y)<<8)
#define		VO_NA_COLOR_CB(CB)				(((CB)/16)<<4)
#define		VO_NA_COLOR_CR(CR)				(((CR)/16)<<0)

#define	REG_VO_ACT_H					0x0304
#define		VO_H_BLANK(n)					((n)<<22)
#define		VO_H_START(n)					((n)<<11)
#define		VO_H_STOP(n)					((n)<<0)

#define	REG_VO_ACT_V					0x0308
#define		VO_V_BLANK(n)					((n)<<22)
#define		VO_V_START(n)					((n)<<11)
#define		VO_V_STOP(n)					((n)<<0)

#define	REG_VO_RANHE_HV					0x030C
#define		VO_SYNC_INVERT					(1<<24)
#define		VO_HSYNC_INVERT					(1<<23)
#define		VO_VSYNC_INVERT					(1<<22)
#define		VO_H_LEN(n)						((n)<<11)
#define		VO_V_LEN(n)						((n)<<0)

#define	REG_VO_DISP_CTRL				0x0310
#define		VO_DISP_ON						(1<<31)
#define		VO_DISP_ERASE_COUNT(n)			((n)<<24)	// auto erase page count
#define		VO_DISP_DOUBLE_SCAN				(1<<22)		// max page set to 2
#define		VO_DISP_SINGLE_PAGE				(1<<21)
#define		VO_DISP_BASE(n)					(((n)>>16) & 0xffff)	// n * 64KB (display memory page offset)

#define REG_VO_DISP_ERASE				0x0314
#define		VO_DISP_ERASE_ON				(1<<0)

#define	REG_VO_ZOOM_CTRL				0x0318
#define		VO_ZOOM_VER_ON					(1<<24)
#define		VO_ZOOM_HOR_ON					(1<<23)
#define		VO_ZOOM_V_COMP					(1<<22)		// vertical compensation when vertical zoon
#define		VO_ZOOM_SX(h)					(((h)/2)<<11)	// zoom offset of horizontal *2
#define		VO_ZOOM_SY(v)					(((v)/2)<<0)	// zoom vertical offset

#define REG_VO_FREEZE_CTRL				0x031C
#define		VO_FREEZE_ON					(1<<1)
#define		VO_FREEZE_INTERPOLATION			(1<<0)

#define	REG_VO_BKG_COLOR				0x0320
#define		BG_Y(y)							((y)<<16)
#define		BG_U(u)							((u)<<8)
#define		BG_V(v)							((v)<<0)

#define	REG_VO_DEINTERLACE				0x0324
#define		VO_DEINTERLACE_THRESHOLD(n)		((n)<<8)
#define		VO_DEINTERLACE_EDGE_VALUE(n)	((n)<<0)

#define REG_VO_BORDER_LINE_COLOR		0x0330
#define REG_VO_BORDER_FILL_COLOR		0x0334
#define REG_VO_BORDER_LINE_MASK			0x0338
#define REG_VO_BORDER_FILL_MASK			0x033c

#define REG_VO_BORDER_X(n)				(0x0340+((n)*4))
#define REG_VO_BORDER_Y(n)				(0x0354+((n)*4))

#define REG_VO_CELL_EXT_SET				0x0368
#define REG_VO_CELL_EXT_START			0x036c
#define REG_VO_CELL_EXT_STOP			0x0370

#define REG_VO_CELL_EXT_SET2			0x0374
#define REG_VO_CELL_EXT_START2			0x0378
#define REG_VO_CELL_EXT_STOP2			0x037c

#define REG_VO_RECTANGLE_CTRL(n)		(0x0368+((n)*12))
#define REG_VO_RECTANGLE_START(n)		(0x036c+((n)*12))
#define REG_VO_RECTANGLE_STOP(n)		(0x0370+((n)*12))

#define REG_VO_CURSOR_POS				(0x0380)
#define REG_VO_CURSOR_CLR				(0x0384)
#define REG_VO_CURSOR_CLR2				(0x0388)
#define REG_VO_CURSOR_MASK(id)			(0x0390+((id)*4))
#define		CURSOR_MASK(a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14,a15)	\
	((a0<<30)|(a1<<28)|(a2<<26)|(a3<<24)|(a4<<22)|(a5<<20)|(a6<<18)|(a7<<16)|	\
	(a8<<14)|(a9<<12)|(a10<<10)|(a11<<8)|(a12<<6)|(a13<<4)|(a14<<2)|(a15<<0))

#define REG_VO_EXPANSION(id)			(0x0250+((id)*4))

/*
 * OSG
 */
#define	REG_OSG_CONFIG					0x03E0
#define		VO_OSG_ON						(1<<31)
#define		VO_OSG_COLOR_MUTE				(1<<28)
#define		VO_OSG_ALPHA_RATE(n)			((n)<<22)
#define		VO_OSG_ALPHA_BG_RATE(n)			((n)<<16)
#define		VO_OSG_BASE(offset)				(((offset)>>16)&0xffff)

#define REG_OSG_ERASE					0x03E4
#define		OSG_ERASE_ON					(1<<0)

#define REG_VO_OSG_BLINK				0x03E8
#define		VO_OSG_BLINK_ON					(1<<1)
#define		VO_OSG_BLINK_INTREVAL18			(1<<0)			// OSG blink interval 0=15frame, 1=18frame

/*
 *	VIDEO CAPTURE REGISTER
 */
#define REG_CAP_BASE					0x0400
#define		CAP_MAX_PAGE(n)					((n)<<16)
#define		CAP_BASE_ADDR(n)				((n)<<0)
#define REG_CAP_BTW						0x0404
#define		CAP_PROG_BANDWIDTH(n)			((n)<<8)
#define		CAP_MAX_BANDWIDTH(n)			((n)<<0)

#define REG_DIM_SCALE1					0x0408
#define REG_DIM_SCALE2					0x040C
#define REG_DIM_SCALE3					0x0410
#define REG_DIM_SCALE4					0x0414
#define REG_DIM_SCALE5					0x0418
#define		DIM_V_MB_NUM_FRAME(n)			((n)<<16)
#define		DIM_V_MB_NUM_FIELD(n)			((n)<<8)
#define		DIM_H_MB_NUM(n)					((n)<<0)

#define REG_DIM_PROG					0x041C
#define REG_CAP_STATUS					0x0420

#define REG_CAP_CH_SCALE(ch)			(0x0440+((ch)*4))
#define REG_CAP_CH_COMP_ENA_E(ch)		(0x0480+((ch)*4))
#define REG_CAP_CH_INTV(ch)				(0x04C0+((ch)*4))
#define REG_CAP_CH_INTV_E(ch)			(0x0500+((ch)*4))

/*
 *	MPEG4 ENCODER REGISTER
 */

#define REG_VE_CFG0						0x0610
#define		VE_TWO_PAGE_MODE				(1<<31)
#define		VE_INTR_CTRL(n)					((n)<<24)	// interrpt moe : every 0=1,1=2 ,2=4 ,3=8 ,4=16 th frames
#define		VE_BLOCK_SIZE(n)				((n)<<16)	// 64KB block
#define		VE_BLOCK_BASE(n)				((n)<<0)	// 64KB block

#define REG_VE_CFG1						0x0614
#define		VE_MPEG_SIZE_H(n)				((n)<<28)	//(6110only)
#define		VE_BYTE_ALIGN(n)				((n)<<24)	// byte align:0=8,1=16.2=32,3=64
#define		VE_JPEG_SIZE_H(n)				((n)<<20)	//(6110only)
#define		VE_INSERT_INDEX					(1<<18)
#define		VE_MOTION_MODE(n)				((n)<<16)	// insert motion flag data :  0=off.1=after ndex 256 bytes,2=user assigned
#define		VE_MOTION_BASE(n)				((n)<<0)

#define REG_VE_WMRK_POLY				0x061C
#define REG_VE_WMRK_INIT_KEY			0x0620
#define REG_VE_WMRK_STRL				0x0624
#define REG_VE_WMRK_ENABLE				0x068C	//(6010only)

#define REG_VE_ENCRYP_POLY				0x0628
#define REG_VE_ENCRYP_INIT				0x062C
#define REG_VE_ATTR						0x0630
#define REG_VE_ENCRYP_ENABLE			0x0684	//(6010only)

#define		VE_LITTLE_ENDIAN				(1<<31)
#define		COMP_ATTR_RN					(1<<30)
#define		COMP_ATTR_FCODE(n)				((n)<<27)
#define		COMP_TIME_INC(n)				((n)<<25)
#define		COMP_TIME_WIDTH(n)				((n)<<21)
#define		DCT_INTERVAL(n)					((n)<<16)

#define REG_VE_STATE(n)					(0x0640+((n)*4))

#define REG_VE_JPEG_QP_TBL				0x0670
#define REG_VE_JPEG_QP_CH_L				0x0674
#define REG_VE_JPEG_QP_CH_H				0x0678
#define REG_VE_JPEG_CFG					0x067C
#define REG_VE_JPEG_CTRL				0x0680

#define REG_VE_OSD_CH					0x0690
#define REG_VE_OSD_BASE					0x0694
#define REG_VE_OSD_CLR					0x0698
#define REG_VE_OSD_OPT					0x069C
#define		VE_OSD_V_DOUBLE					(1<<16)	//(6010only)

#define REG_VE_CH_INTL(ch)				(0x0700+((ch)*4))
#define REG_VE_CH_MOT(ch)				(0x0740+((ch)*4))
#define REG_VE_CH_QP(ch)				(0x0780+((ch)*4))
#define REG_VE_CH_QP_E(ch)				(0x07C0+((ch)*4))
#define REG_VE_CH_GOP(ch)				(0x0800+((ch)*4))
#define REG_VE_CH_GOP_E(ch)				(0x0840+((ch)*4))
#define REG_VE_CH_REF_BASE(ch)			(0x0880+((ch)*4))
#define REG_VE_CH_REF_BASE_E(ch)		(0x08C0+((ch)*4))

#define REG_VE_MPEG4_QUE(n)				(0x0A00+((n)*8))
#define REG_VE_JPEG_QUE(n)				(0x0A04+((n)*8))

/*
 *	MPEG4 DECODER REGISTER
 */
#define REG_VD_CFG0						0x0900
#define		VD_CFG_NO_DISP_ERROR		(1<<24)	//(6110only)
#define		VD_CFG_NO_WRITE_NO_WINDOW	(1<<24)
#define		VD_CFG_BUSY_WIAT_CODE		(1<<23)
#define		VD_CFG_BUSY_WIAT_REF		(1<<22)
#define		VD_CFG_BUSY_WIAT_RES		(1<<21)
#define		VD_CFG_BUSY_WIAT_MS			(1<<20)
#define		VD_CFG_SINGLE_MODE			(1<<18)
#define		VD_CFG_SCAL_MANUAL			(1<<17)
#define		VD_CFG_USER_PAGE_CTRL		(1<<16)
#define		VD_CFG_LITTLE_ENDIAN		(1<<15)
#define		VD_CFG_START_FI				(1<<14)
#define		VD_CFG_ERR_LOCK				(1<<13)
#define		VD_CFG_ERR_INT_ENA			(1<<12)
#define		VD_CFG_TIME_WIDTH(n)		((n)<<8)
#define		VD_CFG_DCT_INTERVAL(n)		((n)<<0)

#define REG_VD_CFG1						0x0904
#define		VD_CFG_MAX_MBH(n)			((n)<<26)	//(6110only)
#define		VD_CFG_MAX_MBV(n)			((n)<<20)	//(6110only)
#define		VD_CFG_TIMEOUT(n)			((n)<<16)	//(6110only)
#define		VD_CFG_DEC2LIVE_ADDR(n)		((n)>>16)

#define	REG_VD_DEINTERLACE				0x0908
#define		VD_DEINTERLACE_THRESHOLD(n)		((n)<<8)
#define		VD_DEINTERLACE_EDGE_VALUE(n)	((n)<<0)

#define REG_VD_CODE_ADR					0x090C

#define REG_VD_CTRL						0x0910
#define		VD_OPER_ON						(1<<31)
#define		VD_MAX_ITEM(n)					((n)<<0)

#define REG_VD_STATUS0					0x0920
#define		VD_STATUS0_INTR_MBSIZE_ERR		(1<<24)	//(6110only)
#define		VD_STATUS0_INTR_TIMEOUT			(1<<23)	//(6110only)
#define		VD_STATUS0_INTR_ACK				(1<<22)
#define		VD_STATUS0_INTR_EMPTY			(1<<21)
#define		VD_STATUS0_INTR_ERR				(1<<20)

#define REG_VD_STATUS1					0x0924

#define REG_VD_IDX0						0x0930
#define		VD_IDX_INTERLACE				(1<<30)
#define		VD_IDX_CHANNEL(n)				((n)<<24)
#define		VD_IDX_SIZE(n)					((n)<<0)

#define REG_VD_IDX1						0x0934
#define		VD_IDX_SRC_SCALE(n)				((n)<<28)
#define		VD_IDX_WINDOW(n)				((n)<<24)
#define		VD_IDX_DEINTERLACE				(1<<16)
#define		VD_IDX_H_BLOCK(n)				((n)<<8)
#define		VD_IDX_V_BLOCK(n)				((n)<<0)

#define REG_VD_IDX2						0x0938
#define		VD_IDX_REF_BASE_SIDE			(1<<31)
#define		VD_IDX_REF_BASE(n)				(((n)>>16)&0xffff)

#define REG_VD_IDX3						0x093C
#define		VD_IDX_DISP_SCALE(n)			((n)<<28)
#define		VD_IDX_INTERLACE_WR				(1<<27)
#define		VD_IDX_INTERPOL					(1<<26)
#define		VD_IDX_HOR2X					(1<<25)
#define		VD_IDX_OFFSET_X(n)				((n)<<12)
#define		VD_IDX_OFFSET_Y(n)				((n)<<0)

#define REG_VD_IDX4						0x0940		// code base
#define		VD_IDX_DEC_WR_PAGE(n)			((n)<<8)
#define		VD_IDX_DISP_RD_PAGE(n)			((n)<<0)

#define REG_VD_WR_PAGE(n)				(0x03F0 + ((n) * 4))

/*
 *	GPIO REGISTER
 */
#define REG_GPIO_CONFIG_0				0x0B00
#define REG_GPIO_CONFIG_1				0x0B04
#define REG_GPIO_DATA_OUT				0x0B08
#define REG_GPIO_DATA_IN				0x0B0C
#define REG_GPIO_INT_ACK_STA			0x0B10
#define REG_GPIO_INT_ENA				0x0B14
#define REG_GPIO_INT_CFG_0				0x0B18
#define REG_GPIO_INT_CFG_1				0x0B1C

/*
 *	IIC REGISTER
 */
#define REG_IIC_CFG						0x0B20
#define		IIC_ENABLE						(1<<8)
#define		IIC_PRESCALE(n)					((n)<<0)	// (SysClk / 5) / ({4'b0,N[7:0],4'b0}+1) = iic clock
														// 317.647 KHz (1), 163.636 KHz (2), 110.204 KHz (3), 83.076 KHz (4),  66.666 KHz (5), 1.3 KHz   (FF)

#define REG_IIC_CTRL					0x0B24
#define		IIC_AUTO_CLEAR					(1<<20)
#define		IIC_STAT_RX_ACK					(1<<19)
#define		IIC_STAT_BUSY					(1<<18)
#define		IIC_STAT_SIG_ERR				(1<<17)
#define		IIC_STAT_TRNS					(1<<16)
#define		IIC_CH(n)						((n)<<5)
#define		IIC_NACK						(1<<4)
#define		IIC_START						(1<<3)
#define		IIC_STOP						(1<<2)
#define		IIC_READ						(1<<1)
#define		IIC_WRITE						(1<<0)

#define REG_IIC_TXD						0x0B28
#define REG_IIC_RXD						0x0B2C

/*
 *	UART REGISTER
 */
#define REG_UART_CONTROL(n)				(0x0BA0 + ((n)*0x20))
#define		UART_CLK_DIV(n)					((n)<<24)	// N=(SysClk/3.375)-1
#define		MODEM_CTRL_EN					(1<<20)
#define		PARITY_ERROR_DROP				(1<<18)
#define		INTR_ERR_EN						(1<<17)
#define		INTR_RX_EN						(1<<16)
#define		INTR_TX_EN						(1<<15)
#define		RX_EN							(1<<14)
#define		TX_EN							(1<<13)
#define		UART_HALF_DUPLEX				(1<<12)
#define		UART_LOOPBACK					(1<<11)

#define		BAUDRATE_230400					((0<<9)|(0<<6))
#define		BAUDRATE_115200					((0<<9)|(1<<6))
#define		BAUDRATE_57600					((0<<9)|(2<<6))
#define		BAUDRATE_38400					((0<<9)|(3<<6))
#define		BAUDRATE_19200					((0<<9)|(4<<6))
#define		BAUDRATE_9600					((0<<9)|(5<<6))
#define		BAUDRATE_4800					((0<<9)|(6<<6))
#define		BAUDRATE_2400					((1<<9)|(6<<6))
#define		BAUDRATE_1200					((2<<9)|(6<<6))
#define		BAUDRATE_300					((3<<9)|(6<<6))

#define		UART_DATA_BIT_8					(3<<4)
#define		UART_DATA_BIT_7					(2<<4)
#define		UART_DATA_BIT_6					(1<<4)
#define		UART_DATA_BIT_5					(0<<4)

#define		UART_STOP_BIT_1					(0<<2)
#define		UART_STOP_BIT_2					(1<<2)

#define		UART_PARITY_NONE				(0<<0)
#define		UART_PARITY_EVEN				(2<<0)
#define		UART_PARITY_ODD					(3<<0)

#define REG_UART_STATUS(n)				(0x0BA4 + ((n)*0x20))
#define		UART_CTS						(1<<15)
#define		UART_RX_BUSY					(1<<14)
#define		UART_OVERRUN					(1<<13)
#define		UART_FRAME_ERR					(1<<12)
#define		UART_PARITY_ERR					(1<<11)
#define		UART_TX_BUSY					(1<<5)

#define		UART_RX_BUFF_CNT(stat)			(((stat)>>6) & 0x1f)
#define		UART_RX_BUFF_SIZE				8
#define		UART_TX_BUFF_CNT(stat)			(((stat)>>0) & 0x1f)
#define		UART_TX_BUFF_SIZE				8

#define REG_UART_TX_DATA(n)				(0x0BA8 + ((n)*0x20))
#define		UART_TX_DATA_PUSH				(1<<8)
#define REG_UART_RX_DATA(n)				(0x0BAC + ((n)*0x20))
#define		UART_RX_DATA_POP				(1<<8)

#define REG_TIMER_CLK_NUM				0x0BE0
#define REG_TIMER_WATCHDOG				0x0BE4
#define REG_TIMER_USEC					0x0BE8
#define REG_TIMER_SEC					0x0BEC

/*
 *	G723 CODEC REGISTER
 */
#define REG_AUDIO_CONTROL				0x0D00
#define		AUDIO_ENABLE					(1<<31)
#define		AUDIO_MASTER_MODE				(1<<30)
#define		AUDIO_I2S_MODE					(1<<29)		// 0 : u-Law, 1 : I2S
#define		AUDIO_I2S_LR_SWAP				(1<<27)
#define		AUDIO_I2S_8BIT					(1<<26)
#define		AUDIO_I2S_MULTI(n)				((n)<<24)	// 0: 2ch, 1: 4ch, 2: 8ch, 3: 16ch (I2S slave only)
#define		AUDIO_MIX_9TO0					(1<<23)
#define		AUDIO_DEC_9TO0_VOL(n)			((n)<<20)
#define		AUDIO_MIX_19TO10				(1<<19)
#define		AUDIO_DEC_19TO10_VOL(n)			((n)<<16)
#define		AUDIO_MODE(n)					((n)<<0)	// Ch9~0(PCM) / Ch19,18~1,0(I2S), 1:Dec, 0:Enc
#define REG_AUDIO_SAMPLE				0x0D04
#define		AUDIO_EE_MODE_ON				(1<<30)		// for testing
#define		AUDIO_EE_ENC_CH(ch)				((ch)<<25)	// for testing
#define		AUDIO_BITRATE(n)				((n)<<16)	// Sampling rate = Clock Freq / (n*a) Hz (master only) (i2s:a=2, pcm:a=1)
#define		AUDIO_CLK_DIV(n)				((n)<<0)	// Clock Freq = SYS_CLK / (n*2) Hz (master only)
#define REG_AUDIO_FDMA_INTR				0x0D08
#define		AUDIO_FDMA_INTERVAL(n)			((n)<<19)
#define		AUDIO_INTR_ORDER(n)				((n)<<16)	// 0~4 available, 2^n pages
#define		AUDIO_FDMA_BASE(n)				((n)<<0)
#define REG_AUDIO_EVOL_0				0x0D0C
#define REG_AUDIO_EVOL_1				0x0D10
#define		AUDIO_EVOL(ch, value)			((value)<<((ch)%10))
#define REG_AUDIO_STA					0x0D14
#endif

