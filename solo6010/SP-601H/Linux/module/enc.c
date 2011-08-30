#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/highmem.h>
#include <linux/pagemap.h>
#include <linux/poll.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
#include <linux/mempool.h>
#endif
#include <asm/uaccess.h>

#include "solo6010.h"
#include "sl_queue.h"

#include <solo6010/enc.h>

#define EXT_BUFFER_MARGIN	(1<<20)

#define CBR_MODE		0
#define CBR_QP_MIN		2
#define CBR_QP_MAX		15
#define CBR_BITRATE		(2*1000*1000)

struct CBR_CTRL
{
	int target_bps;
	int check[5];
	int fifo;
	int avr_size;
};

struct MPEG_INDEX
{
	struct list_head lh;

	unsigned int motion;
	unsigned int vop_type;
	unsigned int channel;
	unsigned int mpeg_offset;
	unsigned int mpeg_size;
	unsigned int jpeg_offset;
	unsigned int jpeg_size;
};

struct JPEG_INDEX
{
	struct list_head lh;

	unsigned int channel;
	unsigned int offset;
	unsigned int size;
	unsigned int hsize;
	unsigned int vsize;
};

#define INDEX_MIN_NR	480

struct SOLO6010_ENC
{
	struct SOLO6010 *solo6010;
	solo6010_device_t *device;

	u32 scale[32];
	u32 gop[32];
	u32 reset_gop[32];
	u32 qp[32];
	u32 interval[32];

	struct CBR_CTRL cbr[32];

	u32 wait_i_frame[32];

	u32 cap_osd_enable;

	u32 index_current;

	struct SL_QUEUE mpeg_queue;
	struct SL_QUEUE jpeg_queue;

	u32 align;

	u32 read_offset;
	u32 overwrited;
};

#define CAPTURE_MAX_PAGE		(18 * 16)
#define CAPTURE_MAX_BANDWIDTH	32			// D1 4channel (D1 == 4)

#define MPEG_INDEX_SIZE			64

#define VI_PROG_HSIZE			(1280-16)
#define VI_PROG_VSIZE			(1024-16)

void solo6010_capture_config(struct SOLO6010 *solo6010)
{
	unsigned long height;
	unsigned long width;

	solo6010_reg_write(solo6010, REG_CAP_BASE, CAP_MAX_PAGE(CAPTURE_MAX_PAGE) | CAP_BASE_ADDR((solo6010->cap_ext_addr>>16)&0xffff));
	solo6010_reg_write(solo6010, REG_CAP_BTW, (1<<17) | CAP_PROG_BANDWIDTH(2) | CAP_MAX_BANDWIDTH(CAPTURE_MAX_BANDWIDTH));

	// set scale 1, 9 dimension
	width = solo6010->video_hsize;
	height = solo6010->video_vsize;
	solo6010_reg_write(solo6010, REG_DIM_SCALE1,
		DIM_H_MB_NUM(width/16) | DIM_V_MB_NUM_FRAME(height/8) | DIM_V_MB_NUM_FIELD(height/16));

	// set scale 2, 10 dimension
	width = solo6010->video_hsize/2;
	height = solo6010->video_vsize/1;
	solo6010_reg_write(solo6010, REG_DIM_SCALE2,
		DIM_H_MB_NUM(width/16) | DIM_V_MB_NUM_FRAME(height/8) | DIM_V_MB_NUM_FIELD(height/16));

	// set scale 3, 11 dimension
	width = solo6010->video_hsize/2;
	height = solo6010->video_vsize/2;
	solo6010_reg_write(solo6010, REG_DIM_SCALE3,
		DIM_H_MB_NUM(width/16) | DIM_V_MB_NUM_FRAME(height/8) | DIM_V_MB_NUM_FIELD(height/16));

	// set scale 4, 12 dimension
	width = solo6010->video_hsize/3;
	height = solo6010->video_vsize/3;
	solo6010_reg_write(solo6010, REG_DIM_SCALE4,
		DIM_H_MB_NUM(width/16) | DIM_V_MB_NUM_FRAME(height/8) | DIM_V_MB_NUM_FIELD(height/16));

	// set scale 5, 13 dimension
	width = solo6010->video_hsize/4;
	height = solo6010->video_vsize/2;
	solo6010_reg_write(solo6010, REG_DIM_SCALE5,
		DIM_H_MB_NUM(width/16) | DIM_V_MB_NUM_FRAME(height/8) | DIM_V_MB_NUM_FIELD(height/16));

	// progressive
	width 	= VI_PROG_HSIZE;
	height 	= VI_PROG_VSIZE;
	solo6010_reg_write(solo6010, REG_DIM_PROG,
		DIM_H_MB_NUM(width/16) | DIM_V_MB_NUM_FRAME(height/16*1) | DIM_V_MB_NUM_FIELD(height/16));

	solo6010_reg_write(solo6010, REG_VE_OSD_CH, 0);
	solo6010_reg_write(solo6010, REG_VE_OSD_BASE, solo6010->eosd_ext_addr>>16);
	solo6010_reg_write(solo6010, REG_VE_OSD_CLR, 0xF0<<16 | 0x80<<8 |0x80);
	solo6010_reg_write(solo6010, REG_VE_OSD_OPT, 0);
}

static void solo6010_mp4e_config(struct SOLO6010 *solo6010)
{
	int i;

	u32 byte_align;

	switch(solo6010->stream_align)
	{
		case 64:	byte_align = 3;	break;
		case 32:	byte_align = 2;	break;
		case 16:	byte_align = 1;	break;
		default:	byte_align = 0;
	}

	solo6010_reg_write(solo6010, REG_VE_CFG0,
		VE_INTR_CTRL(0) |
		VE_BLOCK_SIZE(solo6010->mp4e_ext_size>>16) |
		VE_BLOCK_BASE(solo6010->mp4e_ext_addr>>16));

	solo6010_reg_write(solo6010, REG_VE_CFG1,
		VE_BYTE_ALIGN(byte_align) |
		VE_INSERT_INDEX |
		VE_MOTION_MODE(0));

	solo6010_reg_write(solo6010, REG_VE_WMRK_POLY, 0);
	solo6010_reg_write(solo6010, REG_VE_VMRK_INIT_KEY, 0);
	solo6010_reg_write(solo6010, REG_VE_WMRK_STRL, 0);
	solo6010_reg_write(solo6010, REG_VE_ENCRYP_POLY, 0);
	solo6010_reg_write(solo6010, REG_VE_ENCRYP_INIT, 0);

	solo6010_reg_write(solo6010, REG_VE_ATTR,
		VE_LITTLE_ENDIAN |
		COMP_ATTR_FCODE(1) |
		COMP_TIME_INC(0) |
		COMP_TIME_WIDTH(15) |
		DCT_INTERVAL(36/solo6010->encoder_ability));

	for(i=0; i<16; i++)
	{
		solo6010_reg_write(solo6010, REG_VE_CH_REF_BASE(i),
			(solo6010->eref_ext_addr + (i * 0x00140000))>>16);
	}

	for(i=0; i<16; i++)
	{
		solo6010_reg_write(solo6010, REG_VE_CH_REF_BASE_E(i),
			(solo6010->eref_ext_addr + ((16 + i) * 0x00140000))>>16);
	}

	for(i=0; i<solo6010->max_channel; i++)
	{
		solo6010_reg_write(solo6010, REG_VE_CH_MOT(i), 0x100);
	}
}

unsigned int change_encoder_ability(struct SOLO6010 *solo6010, unsigned int ability)
{
	if(ability > SOLO6010_CODEC_ABILITY_MAX)
		ability = SOLO6010_CODEC_ABILITY_MAX;

	if(ability < SOLO6010_CODEC_ABILITY_MIN)
		ability = SOLO6010_CODEC_ABILITY_MIN;

	solo6010->encoder_ability = ability;

	solo6010_reg_write(solo6010, REG_VE_ATTR,
		VE_LITTLE_ENDIAN |
		COMP_ATTR_FCODE(1) |
		COMP_TIME_INC(0) |
		COMP_TIME_WIDTH(15) |
		DCT_INTERVAL(36/solo6010->encoder_ability));

	dbg_msg("encoder ability : %d\n", solo6010->encoder_ability);

	return solo6010->encoder_ability;
}

static void solo6010_jpeg_config(struct SOLO6010 *solo6010)
{
	solo6010_reg_write(solo6010, REG_VE_JPEG_QP_TBL, (2<<24) | (2<<16) | (2<<8) | (2<<0));
	solo6010_reg_write(solo6010, REG_VE_JPEG_QP_CH_L, 0);
	solo6010_reg_write(solo6010, REG_VE_JPEG_QP_CH_H, 0);
	solo6010_reg_write(solo6010, REG_VE_JPEG_CFG, (((solo6010->jpeg_ext_size>>16)<<16) | (solo6010->jpeg_ext_addr>>16)));
	solo6010_reg_write(solo6010, REG_VE_JPEG_CTRL, 0xffffffff);
}

const unsigned int cap_osd_font_map[512] = {
0x00000000,0x0000c0c8,0xccfefe0c,0x08000000,0x00000000,0x10103838,0x7c7cfefe,0x00000000,
0x00000000,0xfefe7c7c,0x38381010,0x10000000,0x00000000,0x7c82fefe,0xfefefe7c,0x00000000,
0x00000000,0x00001038,0x10000000,0x00000000,0x00000000,0x0010387c,0xfe7c3810,0x00000000,
0x00000000,0x00384444,0x44380000,0x00000000,0x00000000,0x38448282,0x82443800,0x00000000,
0x00000000,0x007c7c7c,0x7c7c0000,0x00000000,0x00000000,0x6c6c6c6c,0x6c6c6c6c,0x00000000,
0x00000000,0x061e7efe,0xfe7e1e06,0x00000000,0x00000000,0xc0f0fcfe,0xfefcf0c0,0x00000000,
0x00000000,0xc6cedefe,0xfedecec6,0x00000000,0x00000000,0xc6e6f6fe,0xfef6e6c6,0x00000000,
0x00000000,0x12367efe,0xfe7e3612,0x00000000,0x00000000,0x90d8fcfe,0xfefcd890,0x00000000,
0x00000038,0x7cc692ba,0x92c67c38,0x00000000,0x00000038,0x7cc6aa92,0xaac67c38,0x00000000,
0x00000038,0x7830107c,0xbaa8680c,0x00000000,0x00000038,0x3c18127c,0xb8382c60,0x00000000,
0x00000044,0xaa6c8254,0x38eec67c,0x00000000,0x00000082,0x44288244,0x38c6827c,0x00000000,
0x00000038,0x444444fe,0xfeeec6fe,0x00000000,0x00000018,0x78187818,0x3c7e7e3c,0x00000000,
0x00000000,0x3854929a,0x82443800,0x00000000,0x00000000,0x00c0c8cc,0xfefe0c08,0x00000000,
0x0000e0a0,0xe040e00e,0x8a0ea40e,0x00000000,0x0000e0a0,0xe040e00e,0x0a8e440e,0x00000000,
0x0000007c,0x82829292,0x929282fe,0x00000000,0x000000f8,0xfc046494,0x946404fc,0x00000000,
0x0000003f,0x7f404c52,0x524c407f,0x00000000,0x0000007c,0x82ba82ba,0x82ba82fe,0x00000000,
0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x183c3c3c,0x18180018,0x18000000,
0x00000066,0x66240000,0x00000000,0x00000000,0x00000000,0x6c6cfe6c,0x6c6cfe6c,0x6c000000,
0x00001010,0x7cd6d616,0x7cd0d6d6,0x7c101000,0x00000000,0x0086c660,0x30180cc6,0xc2000000,
0x00000000,0x386c6c38,0xdc766666,0xdc000000,0x0000000c,0x0c0c0600,0x00000000,0x00000000,
0x00000000,0x30180c0c,0x0c0c0c18,0x30000000,0x00000000,0x0c183030,0x30303018,0x0c000000,
0x00000000,0x0000663c,0xff3c6600,0x00000000,0x00000000,0x00001818,0x7e181800,0x00000000,
0x00000000,0x00000000,0x00000e0e,0x0c060000,0x00000000,0x00000000,0x7e000000,0x00000000,
0x00000000,0x00000000,0x00000006,0x06000000,0x00000000,0x80c06030,0x180c0602,0x00000000,
0x0000007c,0xc6e6f6de,0xcec6c67c,0x00000000,0x00000030,0x383c3030,0x303030fc,0x00000000,
0x0000007c,0xc6c06030,0x180cc6fe,0x00000000,0x0000007c,0xc6c0c07c,0xc0c0c67c,0x00000000,
0x00000060,0x70786c66,0xfe6060f0,0x00000000,0x000000fe,0x0606067e,0xc0c0c67c,0x00000000,
0x00000038,0x0c06067e,0xc6c6c67c,0x00000000,0x000000fe,0xc6c06030,0x18181818,0x00000000,
0x0000007c,0xc6c6c67c,0xc6c6c67c,0x00000000,0x0000007c,0xc6c6c6fc,0xc0c06038,0x00000000,
0x00000000,0x18180000,0x00181800,0x00000000,0x00000000,0x18180000,0x0018180c,0x00000000,
0x00000060,0x30180c06,0x0c183060,0x00000000,0x00000000,0x007e0000,0x007e0000,0x00000000,
0x00000006,0x0c183060,0x30180c06,0x00000000,0x0000007c,0xc6c66030,0x30003030,0x00000000,
0x0000007c,0xc6f6d6d6,0x7606067c,0x00000000,0x00000010,0x386cc6c6,0xfec6c6c6,0x00000000,
0x0000007e,0xc6c6c67e,0xc6c6c67e,0x00000000,0x00000078,0xcc060606,0x0606cc78,0x00000000,
0x0000003e,0x66c6c6c6,0xc6c6663e,0x00000000,0x000000fe,0x0606063e,0x060606fe,0x00000000,
0x000000fe,0x0606063e,0x06060606,0x00000000,0x00000078,0xcc060606,0xf6c6ccb8,0x00000000,
0x000000c6,0xc6c6c6fe,0xc6c6c6c6,0x00000000,0x0000003c,0x18181818,0x1818183c,0x00000000,
0x00000060,0x60606060,0x6066663c,0x00000000,0x000000c6,0xc666361e,0x3666c6c6,0x00000000,
0x00000006,0x06060606,0x060606fe,0x00000000,0x000000c6,0xeefed6c6,0xc6c6c6c6,0x00000000,
0x000000c6,0xcedefef6,0xe6c6c6c6,0x00000000,0x00000038,0x6cc6c6c6,0xc6c66c38,0x00000000,
0x0000007e,0xc6c6c67e,0x06060606,0x00000000,0x00000038,0x6cc6c6c6,0xc6d67c38,0x60000000,
0x0000007e,0xc6c6c67e,0x66c6c6c6,0x00000000,0x0000007c,0xc6c60c38,0x60c6c67c,0x00000000,
0x0000007e,0x18181818,0x18181818,0x00000000,0x000000c6,0xc6c6c6c6,0xc6c6c67c,0x00000000,
0x000000c6,0xc6c6c6c6,0xc66c3810,0x00000000,0x000000c6,0xc6c6c6c6,0xd6d6fe6c,0x00000000,
0x000000c6,0xc6c66c38,0x6cc6c6c6,0x00000000,0x00000066,0x66666666,0x3c181818,0x00000000,
0x000000fe,0xc0603018,0x0c0606fe,0x00000000,0x0000003c,0x0c0c0c0c,0x0c0c0c3c,0x00000000,
0x00000002,0x060c1830,0x60c08000,0x00000000,0x0000003c,0x30303030,0x3030303c,0x00000000,
0x00001038,0x6cc60000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00fe0000,
0x00001818,0x30000000,0x00000000,0x00000000,0x00000000,0x00003c60,0x7c66667c,0x00000000,
0x0000000c,0x0c0c7ccc,0xcccccc7c,0x00000000,0x00000000,0x00007cc6,0x0606c67c,0x00000000,
0x00000060,0x60607c66,0x6666667c,0x00000000,0x00000000,0x00007cc6,0xfe06c67c,0x00000000,
0x00000078,0x0c0c0c3e,0x0c0c0c0c,0x00000000,0x00000000,0x00007c66,0x6666667c,0x60603e00,
0x0000000c,0x0c0c7ccc,0xcccccccc,0x00000000,0x00000030,0x30003830,0x30303078,0x00000000,
0x00000030,0x30003c30,0x30303030,0x30301f00,0x0000000c,0x0c0ccc6c,0x3c6ccccc,0x00000000,
0x00000030,0x30303030,0x30303030,0x00000000,0x00000000,0x000066fe,0xd6d6d6d6,0x00000000,
0x00000000,0x000078cc,0xcccccccc,0x00000000,0x00000000,0x00007cc6,0xc6c6c67c,0x00000000,
0x00000000,0x00007ccc,0xcccccc7c,0x0c0c0c00,0x00000000,0x00007c66,0x6666667c,0x60606000,
0x00000000,0x000076dc,0x0c0c0c0c,0x00000000,0x00000000,0x00007cc6,0x1c70c67c,0x00000000,
0x00000000,0x1818fe18,0x18181870,0x00000000,0x00000000,0x00006666,0x6666663c,0x00000000,
0x00000000,0x0000c6c6,0xc66c3810,0x00000000,0x00000000,0x0000c6d6,0xd6d6fe6c,0x00000000,
0x00000000,0x0000c66c,0x38386cc6,0x00000000,0x00000000,0x00006666,0x6666667c,0x60603e00,
0x00000000,0x0000fe60,0x30180cfe,0x00000000,0x00000070,0x1818180e,0x18181870,0x00000000,
0x00000018,0x18181800,0x18181818,0x00000000,0x0000000e,0x18181870,0x1818180e,0x00000000,
0x000000dc,0x76000000,0x00000000,0x00000000,0x00000000,0x0010386c,0xc6c6fe00,0x00000000};

static int cap_osd_print(struct SOLO6010_ENC *enc, int channel, int x, int y, const unsigned char *str)
{
	struct SOLO6010 *solo6010;
	unsigned char *buf;
	int len;
	int i;
	int j;

	solo6010 = enc->solo6010;

	buf = (unsigned char *)__get_free_pages(GFP_KERNEL, get_order(0x10000));
	if(!buf)
		return -ENOMEM;

	memset(buf, 0, 0x10000);

	len = strlen(str);
	for(i=0; i<len; i++)
	{
		for(j=0;j<16;j++)
			buf[(j*2) + ((x+i)*32) + (y*2048)] = (cap_osd_font_map[(str[i]*4)+(j/4)]>>((3-(j%4))*8)) & 0xff;
	}

	solo6010_p2m_dma(solo6010, 0, 1, buf, solo6010->eosd_ext_addr + (channel * 0x10000), 0x10000);
	enc->cap_osd_enable |= 1<<channel;
	solo6010_reg_write(solo6010, REG_VE_OSD_CH, enc->cap_osd_enable);

	free_pages((unsigned long)buf, get_order(0x10000));

	return 0;
}

static void reset_gop(struct SOLO6010_ENC *enc, unsigned int channel)
{
	solo6010_reg_write(enc->solo6010, REG_VE_CH_GOP(channel), 1);
	enc->reset_gop[channel] = 1;
}

static int set_cbr(struct SOLO6010_ENC *enc, int channel, int bps)
{
	struct CBR_CTRL *cbr;
	int interval;
	int i;

	cbr = &enc->cbr[channel];

	if(enc->scale[channel%16] & (1<<3))
	{
		interval = (enc->interval[channel] + 1) * 2;
	}
	else
	{
		interval = (enc->interval[channel] == 0) ? 1 : (enc->interval[channel] * 2);
	}

	cbr->avr_size = (bps * interval) / 60;
	cbr->avr_size /= 8;
	for(i=0; i<5; i++)
	{
		cbr->check[i] = cbr->avr_size * i * 8;
	}
	cbr->fifo = 0;

	cbr->target_bps = bps;

	return 0;
}

static void cbr_control(struct SOLO6010_ENC *enc, struct MPEG_INDEX *ext_index)
{
	struct SOLO6010 *solo6010;
	int i;
	int qp_change = 0;
	int mpeg_size = ext_index->mpeg_size - MPEG_INDEX_SIZE;

	solo6010 = enc->solo6010;

	enc->cbr[ext_index->channel].fifo += mpeg_size;
	enc->cbr[ext_index->channel].fifo -= enc->cbr[ext_index->channel].avr_size;

	for(i=4; i>0; i--)
	{
		if(enc->cbr[ext_index->channel].fifo > enc->cbr[ext_index->channel].check[i-1])
		{
			qp_change = i+1;
			break;
		}
		else if(enc->cbr[ext_index->channel].fifo < (-enc->cbr[ext_index->channel].check[i-1]))
		{
			qp_change = -(i+1);
			break;
		}
	}

	if(!qp_change)
	{
		if(mpeg_size > (enc->cbr[ext_index->channel].avr_size<<2))
		{
			qp_change = 1;
		}
		else if(mpeg_size > (enc->cbr[ext_index->channel].avr_size>>2))
		{
			qp_change = -1;
		}
	}

	if(enc->cbr[ext_index->channel].fifo > enc->cbr[ext_index->channel].check[4])
		enc->cbr[ext_index->channel].fifo = enc->cbr[ext_index->channel].check[4];

	if(enc->cbr[ext_index->channel].fifo < (-enc->cbr[ext_index->channel].check[4]))
		enc->cbr[ext_index->channel].fifo = (-enc->cbr[ext_index->channel].check[4]);

	if(qp_change)
	{
		qp_change += (int)enc->qp[ext_index->channel];
		if(qp_change > CBR_QP_MAX)
			enc->qp[ext_index->channel] = CBR_QP_MAX;
		else if(qp_change < CBR_QP_MIN)
			enc->qp[ext_index->channel] = CBR_QP_MIN;
		else
			enc->qp[ext_index->channel] = qp_change;

		if(ext_index->channel < 16)
			solo6010_reg_write(solo6010, REG_VE_CH_QP(ext_index->channel), enc->qp[ext_index->channel]);
		else
			solo6010_reg_write(solo6010, REG_VE_CH_QP_E(ext_index->channel-16), enc->qp[ext_index->channel]);
	}
}

int solo6010_encoder_interrupt(struct SOLO6010 *solo6010)
{
	struct SOLO6010_ENC *enc;
	u32 current_queue;
	u32 reg_mpeg_size;

	enc = (struct SOLO6010_ENC *)solo6010->enc;

	solo6010_reg_write(solo6010, REG_INTR_STAT, INTR_ENCODER);

	current_queue = solo6010_reg_read(solo6010, REG_VE_STATE(11));
	reg_mpeg_size = solo6010_reg_read(solo6010, REG_VE_STATE(0)) & 0x000fffff;
	reg_mpeg_size = (reg_mpeg_size + MPEG_INDEX_SIZE + solo6010->stream_align) & (~(solo6010->stream_align-1));

	if(current_queue >= 16)
		dbg_msg("current_queue : %08x\n", current_queue);

	current_queue = (current_queue  + 1) % 16;

	while(enc->index_current != current_queue)
	{
		struct MPEG_INDEX *ext_index;
		unsigned int channel;
		unsigned int vop_type;
		unsigned int motion;
		unsigned int mpeg_size;
		u32 mpeg_current;
		u32 mpeg_next;
		u32 jpeg_current;
		u32 jpeg_next;

		unsigned long flags;

		mpeg_current = solo6010_reg_read(solo6010, REG_VE_MPEG4_QUE(enc->index_current));
		jpeg_current = solo6010_reg_read(solo6010, REG_VE_JPEG_QUE(enc->index_current));

		enc->index_current = (enc->index_current + 1) % 16;

		mpeg_next = solo6010_reg_read(solo6010, REG_VE_MPEG4_QUE(enc->index_current)) & 0x00ffffff;
		jpeg_next = solo6010_reg_read(solo6010, REG_VE_JPEG_QUE(enc->index_current));

		if((enc->read_offset != 0xffffffff) &&
			(((solo6010->mp4e_ext_size + enc->read_offset - mpeg_next) % solo6010->mp4e_ext_size) <= EXT_BUFFER_MARGIN))
			enc->overwrited = 1;

		while(!sl_queue_empty(&enc->mpeg_queue))
		{
			spin_lock_irqsave(&enc->mpeg_queue.lock, flags);
			if(sl_queue_empty(&enc->mpeg_queue))
			{
				spin_unlock_irqrestore(&enc->mpeg_queue.lock, flags);
				break;
			}

			ext_index = sl_queue_head(&enc->mpeg_queue, struct MPEG_INDEX);
			if(((solo6010->mp4e_ext_size + ext_index->mpeg_offset - mpeg_next) % solo6010->mp4e_ext_size) > EXT_BUFFER_MARGIN)
			{
				spin_unlock_irqrestore(&enc->mpeg_queue.lock, flags);
				break;
			}

			list_del(&ext_index->lh);
			spin_unlock_irqrestore(&enc->mpeg_queue.lock, flags);
			enc->wait_i_frame[ext_index->channel] = 1;
			sl_queue_free(&enc->mpeg_queue, ext_index);
		}

		channel = (mpeg_current>>24) & 0x1f;
		vop_type = (mpeg_current>>29) & 3;
		motion = (mpeg_current>>31) & 1;
		mpeg_current &= 0x00ffffff;
		mpeg_size = (solo6010->mp4e_ext_size + mpeg_next - mpeg_current) % solo6010->mp4e_ext_size;

		if(mpeg_current > mpeg_next)
		{
			if(mpeg_size != reg_mpeg_size)
			{
				reset_gop(enc, channel);
				continue;
			}
		}

		if(enc->reset_gop[channel])
		{
			if(vop_type)
				continue;
			enc->reset_gop[channel] = 0;
			solo6010_reg_write(enc->solo6010, REG_VE_CH_GOP(channel), enc->gop[channel]);
		}

		ext_index = sl_queue_alloc(&enc->mpeg_queue);
		if(!ext_index)
		{
			reset_gop(enc, channel);
			dbg_msg("enc alloc index error!\n");
			continue;
		}

		ext_index->motion = motion;
		ext_index->vop_type = vop_type;
		ext_index->channel = channel;
		ext_index->mpeg_offset = mpeg_current;
		ext_index->mpeg_size = mpeg_size;
		ext_index->jpeg_offset = jpeg_current;
		ext_index->jpeg_size = (solo6010->jpeg_ext_size + jpeg_next - jpeg_current) % solo6010->jpeg_ext_size;

		if(enc->cbr[ext_index->channel].target_bps)
			cbr_control(enc, ext_index);

		spin_lock_irqsave(&enc->mpeg_queue.lock, flags);
		list_add(&ext_index->lh, &enc->mpeg_queue.lh);
		spin_unlock_irqrestore(&enc->mpeg_queue.lock, flags);
		wake_up(&enc->mpeg_queue.wq);
	}
#if 1	// jpeg
{
	struct JPEG_INDEX *jpeg_index;
	u32 channel;
	u32 offset;
	u32 size;
	u32 picture_size;
	unsigned long flags;

	channel = solo6010_reg_read(solo6010, REG_VE_STATE(9));
	offset = solo6010_reg_read(solo6010, REG_VE_STATE(3));
	size = solo6010_reg_read(solo6010, REG_VE_STATE(4)) & 0x000fffff;
	picture_size = solo6010_reg_read(solo6010, REG_VE_STATE(1)) & 0x0000ffff;

	while(!sl_queue_empty(&enc->jpeg_queue))
	{
		spin_lock_irqsave(&enc->jpeg_queue.lock, flags);
		if(sl_queue_empty(&enc->jpeg_queue))
		{
			spin_unlock_irqrestore(&enc->jpeg_queue.lock, flags);
			break;
		}

		jpeg_index = sl_queue_head(&enc->jpeg_queue, struct JPEG_INDEX);
		if(((solo6010->jpeg_ext_size + jpeg_index->offset - (offset + size)) % solo6010->jpeg_ext_size) > EXT_BUFFER_MARGIN)
		{
			spin_unlock_irqrestore(&enc->jpeg_queue.lock, flags);
			break;
		}

		list_del(&jpeg_index->lh);
		spin_unlock_irqrestore(&enc->jpeg_queue.lock, flags);
		sl_queue_free(&enc->jpeg_queue, jpeg_index);
	}

	jpeg_index = sl_queue_alloc(&enc->jpeg_queue);
	if(!jpeg_index)
		return 0;

	jpeg_index->channel = channel;
	jpeg_index->offset = offset;
	jpeg_index->size = size;
	jpeg_index->hsize = ((picture_size>>8) & 0xff) << 4;
	jpeg_index->vsize = ((picture_size>>0) & 0xff) << 4;

	spin_lock_irqsave(&enc->jpeg_queue.lock, flags);
	list_add(&jpeg_index->lh, &enc->jpeg_queue.lh);
	spin_unlock_irqrestore(&enc->jpeg_queue.lock, flags);
	wake_up(&enc->jpeg_queue.wq);
}
#endif
	return 0;
}

static ssize_t jpeg_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	struct SOLO6010_ENC *enc;
	struct SOLO6010 *solo6010;
	struct JPEG_INDEX *index;

	unsigned int dma_size;
	int ret;

	unsigned long flags;

	if((unsigned long)buf & 0xf)
	{
		dbg_msg(" solo6010 enc read buffer should be aligned by 16.\n");
		return -1;
	}

	enc = filp->private_data;
	solo6010 = enc->solo6010;

check_ext_index:
	spin_lock_irqsave(&enc->jpeg_queue.lock, flags);
	if(sl_queue_empty(&enc->jpeg_queue))
	{
		spin_unlock_irqrestore(&enc->jpeg_queue.lock, flags);

		if(filp->f_flags & O_NONBLOCK)
			return -EAGAIN;

		ret = wait_event_interruptible(enc->jpeg_queue.wq, !sl_queue_empty(&enc->jpeg_queue));
		if(ret)
			return ret;

		goto check_ext_index;
	}

	index = sl_queue_head(&enc->jpeg_queue, struct JPEG_INDEX);
	list_del(&index->lh);
	spin_unlock_irqrestore(&enc->jpeg_queue.lock, flags);

	dma_size = (index->size + 0x0f) & ~0x0f;

	if((dma_size + (sizeof(unsigned int) * 4)) > count)
	{
		ret = -EFAULT;
		goto end_read;
	}

	if(copy_to_user(buf, &index->channel, sizeof(unsigned int)))
	{
		ret = -EFAULT;
		goto end_read;
	}
	buf += sizeof(unsigned int);

	if(copy_to_user(buf, &index->hsize, sizeof(unsigned int)))
	{
		ret = -EFAULT;
		goto end_read;
	}
	buf += sizeof(unsigned int);

	if(copy_to_user(buf, &index->vsize, sizeof(unsigned int)))
	{
		ret = -EFAULT;
		goto end_read;
	}
	buf += sizeof(unsigned int);
	buf += sizeof(unsigned int);

	if((index->offset + dma_size) > (solo6010->jpeg_ext_addr + solo6010->jpeg_ext_size))
	{
		solo6010_dma_user(solo6010, 0, 0, buf,
			index->offset, solo6010->jpeg_ext_addr + solo6010->jpeg_ext_size - index->offset);
		solo6010_dma_user(solo6010, 0, 0, buf + solo6010->jpeg_ext_addr + solo6010->jpeg_ext_size - index->offset,
			solo6010->jpeg_ext_addr, dma_size + index->offset - (solo6010->jpeg_ext_addr + solo6010->jpeg_ext_size));
	}
	else
	{
		solo6010_dma_user(solo6010, 0, 0, buf, index->offset, dma_size);
	}

	ret = index->size + (sizeof(unsigned int) * 4);

end_read:

	sl_queue_free(&enc->jpeg_queue, index);

	return ret;
}

static loff_t fops_llseek(struct file *filp, loff_t off, int whence)
{
	return -EINVAL;
}

static unsigned int fops_poll(struct file *filp, poll_table *wait)
{
	struct SOLO6010_ENC *enc;
	unsigned int mask = 0;

	enc = filp->private_data;

	poll_wait(filp, &enc->mpeg_queue.wq, wait);
	if(!sl_queue_empty(&enc->mpeg_queue))
		mask |= POLLIN | POLLRDNORM | POLLRDBAND;

	return mask;
}

static ssize_t fops_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	struct SOLO6010_ENC *enc;
	struct SOLO6010 *solo6010;
	struct MPEG_INDEX *ext_index;

	int ret = 0;

	unsigned long flags;

	enc = filp->private_data;
	solo6010 = enc->solo6010;

	if((unsigned long)buf & 0xf)
	{
		dbg_msg(" solo6010 enc read buffer should be aligned by 16.\n");
		return -1;
	}

check_ext_index:
	spin_lock_irqsave(&enc->mpeg_queue.lock, flags);
	if(sl_queue_empty(&enc->mpeg_queue))
	{
		spin_unlock_irqrestore(&enc->mpeg_queue.lock, flags);

		if(filp->f_flags & O_NONBLOCK)
			return -EAGAIN;

		ret = wait_event_interruptible(enc->mpeg_queue.wq, !sl_queue_empty(&enc->mpeg_queue));
		if(ret)
			return ret;

		goto check_ext_index;
	}

	ext_index = sl_queue_head(&enc->mpeg_queue, struct MPEG_INDEX);
	if(enc->wait_i_frame[ext_index->channel])
	{
		if(ext_index->vop_type)
		{
			list_del(&ext_index->lh);
			spin_unlock_irqrestore(&enc->mpeg_queue.lock, flags);
			sl_queue_free(&enc->mpeg_queue, ext_index);
			goto check_ext_index;
		}
		enc->wait_i_frame[ext_index->channel] = 0;
	}
	if(count == 0)
	{
		ret = ext_index->mpeg_size;
		spin_unlock_irqrestore(&enc->mpeg_queue.lock, flags);
		return ret;
	}
	list_del(&ext_index->lh);
	spin_unlock_irqrestore(&enc->mpeg_queue.lock, flags);

	if(ext_index->mpeg_size > count)
	{
		enc->wait_i_frame[ext_index->channel] = 1;
		dbg_msg("mpeg_size : %d, count : %d\n", ext_index->mpeg_size, count);
		sl_queue_free(&enc->mpeg_queue, ext_index);
		goto check_ext_index;
	}

	enc->read_offset = ext_index->mpeg_offset;
	if((ext_index->mpeg_offset + ext_index->mpeg_size) > solo6010->mp4e_ext_size)
	{
		solo6010_dma_user(solo6010, 0, 0, buf,
			solo6010->mp4e_ext_addr + ext_index->mpeg_offset, solo6010->mp4e_ext_size - ext_index->mpeg_offset);
		solo6010_dma_user(solo6010, 0, 0, buf + solo6010->mp4e_ext_size - ext_index->mpeg_offset,
			solo6010->mp4e_ext_addr, ext_index->mpeg_size + ext_index->mpeg_offset - solo6010->mp4e_ext_size);
	}
	else
	{
		solo6010_dma_user(solo6010, 0, 0, buf,
			solo6010->mp4e_ext_addr + ext_index->mpeg_offset, ext_index->mpeg_size);
	}
	ret = ext_index->mpeg_size;

	enc->read_offset = 0xffffffff;
	if(enc->overwrited)
	{
		enc->overwrited = 0;
		enc->wait_i_frame[ext_index->channel] = 1;
		sl_queue_free(&enc->mpeg_queue, ext_index);
		goto check_ext_index;
	}

	sl_queue_free(&enc->mpeg_queue, ext_index);

    if((buf[36] > 15) || (buf[36]!=(buf[3]&0xf)))
    {
        reset_gop(enc, 0);
        reset_gop(enc, 1);
        reset_gop(enc, 2);
        reset_gop(enc, 3);
        return 0;
    }

	return ret;
}

static ssize_t fops_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	return -1;
}

void solo6010_enc_jpeg_mode(struct file *filp);

static int fops_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct SOLO6010_ENC *enc;
	struct SOLO6010 *solo6010;

	if(_IOC_TYPE(cmd) != IOCTL_ENC_MAGIC)
		return -EINVAL;

	if(_IOC_NR(cmd) >= IOCTL_ENC_MAXNR)
		return -EINVAL;

	enc = filp->private_data;
	solo6010 = enc->solo6010;

	switch(cmd)
	{
		case IOCTL_CAP_SET_SCALE:
		{
			struct CAP_SCALE param;

			if(copy_from_user(&param, (void *)arg, sizeof(param)))
				return -EFAULT;

			if(param.channel >= 32)
				return -EINVAL;

			enc->scale[param.channel] = param.value;

			if(param.channel < 16)
			{
				solo6010_reg_write(solo6010, REG_CAP_CH_SCALE(param.channel), 0);
				solo6010_reg_write(solo6010, REG_VE_CH_INTL(param.channel), (enc->scale[param.channel]>>3)&1);
				solo6010_reg_write(solo6010, REG_CAP_CH_SCALE(param.channel), enc->scale[param.channel]);
			}
			else
			{
				solo6010_reg_write(solo6010, REG_CAP_CH_COMP_ENA_E(param.channel%16),
					enc->scale[param.channel] ? 1 : 0);
			}

			reset_gop(enc, param.channel);

			if(enc->cbr[param.channel].target_bps)
				set_cbr(enc, param.channel, enc->cbr[param.channel].target_bps);

			return 0;
		}

		case IOCTL_CAP_SET_INTERVAL:
		{
			struct CAP_INTERVAL param;

			if(copy_from_user(&param, (void *)arg, sizeof(param)))
				return -EFAULT;

			if(param.channel >= 32)
				return -EINVAL;

			enc->interval[param.channel] = param.value;

			if(param.channel < 16)
			{
				solo6010_reg_write(solo6010, REG_CAP_CH_INTV(param.channel),
					enc->interval[param.channel]);
			}
			else
			{
				solo6010_reg_write(solo6010, REG_CAP_CH_INTV_E(param.channel%16),
					enc->interval[param.channel]);
			}

			if(enc->cbr[param.channel].target_bps)
				set_cbr(enc, param.channel, enc->cbr[param.channel].target_bps);

			return 0;
		}

		case IOCTL_MP4E_SET_GOPSIZE:
		{
			struct MP4E_GOPSIZE param;

			if(copy_from_user(&param, (void *)arg, sizeof(param)))
				return -EFAULT;

			if(param.channel >= 32)
				return -EINVAL;

			enc->gop[param.channel] = param.value;

			if(param.channel < 16)
			{
				solo6010_reg_write(solo6010, REG_VE_CH_GOP(param.channel),
					enc->gop[param.channel]);
			}
			else
			{
				solo6010_reg_write(solo6010, REG_VE_CH_GOP_E(param.channel%16),
					enc->gop[param.channel]);
			}

			return 0;
		}

		case IOCTL_MP4E_SET_QP:
		{
			struct MP4E_QP param;

			if(copy_from_user(&param, (void *)arg, sizeof(param)))
				return -EFAULT;

			if(param.channel >= 32)
				return -EINVAL;

			enc->qp[param.channel] = param.value;

			if(param.channel < 16)
			{
				solo6010_reg_write(solo6010, REG_VE_CH_QP(param.channel),
					enc->qp[param.channel]);
			}
			else
			{
				solo6010_reg_write(solo6010, REG_VE_CH_QP_E(param.channel%16),
					enc->qp[param.channel]);
			}

			return 0;
		}

		case IOCTL_CAP_GET_SCALE:
		{
			struct CAP_SCALE param;

			if(copy_from_user(&param, (void *)arg, sizeof(param)))
				return -EFAULT;

			if(param.channel >= 32)
				return -EINVAL;

			param.value = enc->scale[param.channel];

			if(copy_to_user((void *)arg, &param, sizeof(param)))
				return -EFAULT;

			return 0;
		}

		case IOCTL_CAP_GET_INTERVAL:
		{
			struct CAP_INTERVAL param;

			if(copy_from_user(&param, (void *)arg, sizeof(param)))
				return -EFAULT;

			if(param.channel >= 32)
				return -EINVAL;

			param.value = enc->interval[param.channel];

			if(copy_to_user((void *)arg, &param, sizeof(param)))
				return -EFAULT;

			return 0;
		}

		case IOCTL_MP4E_GET_GOPSIZE:
		{
			struct MP4E_GOPSIZE param;

			if(copy_from_user(&param, (void *)arg, sizeof(param)))
				return -EFAULT;

			if(param.channel >= 32)
				return -EINVAL;

			param.value = enc->gop[param.channel];

			if(copy_to_user((void *)arg, &param, sizeof(param)))
				return -EFAULT;

			return 0;
		}

		case IOCTL_MP4E_GET_QP:
		{
			struct MP4E_QP param;

			if(copy_from_user(&param, (void *)arg, sizeof(param)))
				return -EFAULT;

			if(param.channel >= 32)
				return -EINVAL;

			param.value = enc->qp[param.channel];

			if(copy_to_user((void *)arg, &param, sizeof(param)))
				return -EFAULT;

			return 0;
		}

		case IOCTL_SET_JPEG_MODE:
			solo6010_enc_jpeg_mode(filp);
			return 0;

		case IOCTL_GOP_RESET:
		{
			unsigned int channel;

			if(copy_from_user(&channel, (void *)arg, sizeof(unsigned int)))
				return -EFAULT;

			if(channel > 31)
				return -EINVAL;

			reset_gop(enc, channel);

			return 0;
		}

        case IOCTL_ENC_SETWM:
        {
            struct WM param;
            if(copy_from_user(&param, (void *)arg, sizeof(param)))
                return -EFAULT;

            solo6010_reg_write(solo6010, REG_VE_WMRK_POLY, param.polynomial);
            solo6010_reg_write(solo6010, REG_VE_VMRK_INIT_KEY, param.initialkey);
            solo6010_reg_write(solo6010, REG_VE_WMRK_STRL, param.strength);
            
            return 0;
        }
	}

	return -ENOTTY;
}

static int fops_open(struct inode *inode, struct file *filp)
{
	struct SOLO6010_ENC *enc;

	enc = (struct SOLO6010_ENC *)solo6010_device_data_get(inode_to_solo6010_device(inode));
	if(!enc)
		return -ENODEV;

	filp->private_data = enc;

	return 0;
}

static int fops_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static struct file_operations fops = {
	.owner =    THIS_MODULE,
	.llseek =   fops_llseek,
	.poll =		fops_poll,
	.read =     fops_read,
	.write =    fops_write,
	.ioctl =    fops_ioctl,
	.open =     fops_open,
	.release =  fops_release,
};

static struct file_operations jpeg_fops = {
	.owner =    THIS_MODULE,
	.llseek =   fops_llseek,
	.poll =		fops_poll,
	.read =     jpeg_read,
	.write =    fops_write,
	.ioctl =    fops_ioctl,
	.open =     fops_open,
	.release =  fops_release,
};

void solo6010_enc_jpeg_mode(struct file *filp)
{
	filp->f_op = &jpeg_fops;
}

int solo6010_enc_init(struct SOLO6010 *solo6010)
{
	struct SOLO6010_ENC *enc;
	int i;

	enc = (struct SOLO6010_ENC *)kmalloc(sizeof(struct SOLO6010_ENC), GFP_KERNEL);
	if(enc == NULL)
		return -ENOMEM;

	memset(enc, 0, sizeof(struct SOLO6010_ENC));

	enc->solo6010 = solo6010;
	solo6010_capture_config(solo6010);
	solo6010_mp4e_config(solo6010);
	solo6010_jpeg_config(solo6010);

	for(i=0; i<32; i++)
	{
		enc->scale[i] = 0;
		enc->gop[i] = 1;
		enc->qp[i] = 4;
		enc->interval[i] = 1;

		if(i<16)
		{
			solo6010_reg_write(solo6010, REG_CAP_CH_SCALE(i), enc->scale[i]);
			solo6010_reg_write(solo6010, REG_VE_CH_GOP(i), enc->qp[i]);
			solo6010_reg_write(solo6010, REG_VE_CH_QP(i), enc->qp[i]);
			solo6010_reg_write(solo6010, REG_CAP_CH_INTV(i), enc->qp[i]);
		}
		else
		{
			solo6010_reg_write(solo6010, REG_CAP_CH_COMP_ENA_E(i-16), enc->scale[i]);
			solo6010_reg_write(solo6010, REG_VE_CH_GOP_E(i-16), enc->qp[i]);
			solo6010_reg_write(solo6010, REG_VE_CH_QP_E(i-16), enc->qp[i]);
			solo6010_reg_write(solo6010, REG_CAP_CH_INTV_E(i-16), enc->qp[i]);
		}

		reset_gop(enc, i);
		enc->wait_i_frame[i] = 1;
	}

	for(i=0; i<16; i++)
	{
		unsigned char str[16];

		sprintf(str, "CH %02d", i);
		cap_osd_print(enc, i, 2, 2, str);
	}

#if CBR_MODE
	for(i=0; i<16; i++)
		set_cbr(enc, i, CBR_BITRATE);
#endif

	enc->index_current = solo6010_reg_read(solo6010, REG_VE_STATE(11));

	if(sl_queue_init(&enc->mpeg_queue, sizeof(struct MPEG_INDEX), INDEX_MIN_NR, "solo6010_enc%d", solo6010->chip.id))
	{
		kfree(enc);
		return -1;
	}

	if(sl_queue_init(&enc->jpeg_queue, sizeof(struct JPEG_INDEX), INDEX_MIN_NR, "solo6010_jpeg%d", solo6010->chip.id))
	{
		sl_queue_exit(&enc->mpeg_queue);
		kfree(enc);
		return -1;
	}

	solo6010->enc = (void *)enc;

	solo6010_enable_interrupt(solo6010, INTR_ENCODER);

	enc->device = solo6010_device_add(&solo6010->chip, &fops, "enc", SOLO6010_ENC_DEV_OFFSET, (void *)enc);
	if(IS_ERR(enc->device))
	{
		int ret_val = PTR_ERR(enc->device);
		solo6010->enc = NULL;
		sl_queue_exit(&enc->jpeg_queue);
		sl_queue_exit(&enc->mpeg_queue);
		kfree(enc);
		return ret_val;
	}

	return 0;
}

void solo6010_enc_exit(struct SOLO6010 *solo6010)
{
	struct SOLO6010_ENC *enc;
	struct MPEG_INDEX *ext_index;
	struct JPEG_INDEX *jpeg_ext_index;
	int i;

	enc = (struct SOLO6010_ENC *)solo6010->enc;

	solo6010_device_remove(enc->device);

	solo6010_disable_interrupt(solo6010, INTR_ENCODER);
	for(i=0; i<16; i++)
	{
		solo6010_reg_write(solo6010, REG_CAP_CH_SCALE(i), 0);
		solo6010_reg_write(solo6010, REG_CAP_CH_COMP_ENA_E(i), 0);
	}

	sl_queue_clear(&enc->mpeg_queue, ext_index, struct MPEG_INDEX);
	sl_queue_exit(&enc->mpeg_queue);

	sl_queue_clear(&enc->jpeg_queue, jpeg_ext_index, struct JPEG_INDEX);
	sl_queue_exit(&enc->jpeg_queue);

	kfree(enc);
}

