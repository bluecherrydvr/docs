#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/time.h>

#include "solo6x10.h"
#include "saa7128.h"
#include "tw2815.h"
#include "tw2865.h"

#define PCI_VENDOR_ID_SOFTLOGIC		0x9413
#define PCI_DEVICE_ID_SOLO6010		0x6010
#define PCI_DEVICE_ID_SOLO6110		0x6110
#define SOLO6x10_NAME				"solo6010"

static unsigned int max_channel = 16;
static unsigned int video_type = 0;
static unsigned int video_hsize = 704;
static unsigned int vout_hstart = 6;
static unsigned int vout_vstart = 20;
static unsigned int vin_hstart = 8;
static unsigned int vin_vstart = 2;
static unsigned int sdram_size = 128;
static unsigned int fdma_control = 0;	// for debuging

static unsigned int device_count = 0;

int solo6x10_clock_config(struct SOLO6x10 *solo6x10);
int p2m_test(struct SOLO6x10 *solo6x10, u32 base, int size);

static struct SOLO6x10 *alloc_solo6x10_dev(struct pci_dev *pdev)
{
	struct SOLO6x10 *solo6x10;

	solo6x10 = kmalloc(sizeof(struct SOLO6x10), GFP_KERNEL);
	if(solo6x10 == NULL)
	{
		dbg_msg("kmalloc error!\n");
		return NULL;
	}

	memset(solo6x10, 0, sizeof(struct SOLO6x10));

	solo6x10->pdev = pdev;
	solo6x10->reg_base = ioremap_nocache(pdev->resource[0].start, pdev->resource[0].end - pdev->resource[0].start);
	if(solo6x10->reg_base == NULL)
	{
		dbg_msg("ioremap_nocache error!\n");
		goto error_return;
	}

	spin_lock_init(&solo6x10->reg_io_lock);

	solo6x10->irq = pdev->irq;

	solo6x10->max_channel = max_channel;
	solo6x10->video_type = video_type;
	solo6x10->video_hsize = video_hsize;
	solo6x10->video_vsize = (video_type == 0) ? 240 : 288;
	solo6x10->vout_hstart = vout_hstart;
	solo6x10->vout_vstart = vout_vstart;
	solo6x10->vin_hstart = vin_hstart;
	solo6x10->vin_vstart = vin_vstart;

	if(solo6x10->pdev->device == 0x6110)
	{
		solo6x10->encoder_ability = 5;
		solo6x10->decoder_ability = 4;
	}
	else
	{
		solo6x10->encoder_ability = SOLO6010_CODEC_ABILITY_MAX;
		solo6x10->decoder_ability = SOLO6010_CODEC_ABILITY_TOTAL - solo6x10->encoder_ability;
	}

								//start			//size
	solo6x10->disp_ext_addr		= 0x00000000;	//0x00480000
	solo6x10->dec2live_ext_addr	= 0x00480000;	//0x00240000
	solo6x10->osg_ext_addr		= 0x006C0000;	//0x00120000
	solo6x10->motion_ext_addr	= 0x007E0000;	//0x00080000
	solo6x10->g723_ext_addr		= 0x00860000;	//0x00010000
	solo6x10->cap_ext_addr		= 0x00870000;	//0x01320000 = 0x00120000 * (16+1)
	solo6x10->cap_ext_size = 0x01320000;

	solo6x10->eosd_ext_addr		= 0x02800000;	//0x00400000 = 0x00020000 * 32
	solo6x10->dref_ext_addr		= 0x02C00000;	//0x01400000 = 0x00140000 * 16
	solo6x10->eref_ext_addr		= 0x04000000;	//0x02800000 = 0x00140000 * 32
	solo6x10->jpeg_ext_addr		= 0x06800000;
	solo6x10->jpeg_ext_size	= 0x00800000;
	solo6x10->mp4e_ext_addr		= 0x07000000;
	solo6x10->mp4e_ext_size	= 0x00800000;
	solo6x10->mp4d_ext_addr		= 0x07800000;
	solo6x10->mp4d_ext_size	= 0x00800000;

	solo6x10->stream_align = 64;	// 64, 32, 16, 8

	if(solo6x10_chip_init(&solo6x10->chip, device_count) != 0)
	{
		dbg_msg("solo6x10_chip_init error!\n");
		goto error_return;
	}

	dbg_msg("solo6x10 chip id : %d\n", solo6x10_reg_read(solo6x10, 0x001C) & 0x07);

	device_count++;

	solo6x10->proc = proc_mkdir(solo6x10->chip.name, NULL);

	return solo6x10;

error_return:
	if(solo6x10->reg_base)
		iounmap((void *)solo6x10->reg_base);
	if(solo6x10)
		kfree(solo6x10);

	return NULL;
}

static void free_solo6x10_dev(struct SOLO6x10 *solo6x10)
{
	if(solo6x10->proc)
		remove_proc_entry(solo6x10->proc->name, NULL);

	device_count--;

	solo6x10_chip_exit(&solo6x10->chip);
	iounmap((void *)solo6x10->reg_base);
	kfree(solo6x10);
}

static inline void solo6x10_set_current_time(struct SOLO6x10 *solo6x10)
{
	struct timeval tv;
	do_gettimeofday(&tv);
	solo6x10_reg_write(solo6x10, REG_TIMER_SEC, tv.tv_sec);
	solo6x10_reg_write(solo6x10, REG_TIMER_USEC, tv.tv_usec);
}

void solo6x10_timer_sync(struct SOLO6x10 *solo6x10)
{
	static int count = 0;
	count++;
	if(count >= 60)
	{
		count = 0;
		#if 1
		{
			u32 chip_sec;
			u32 chip_usec;
			struct timeval tv;
			int diff;

			chip_sec = solo6x10_reg_read(solo6x10, REG_TIMER_SEC);
			chip_usec = solo6x10_reg_read(solo6x10, REG_TIMER_USEC);
			do_gettimeofday(&tv);
			diff = (int)tv.tv_sec - (int)chip_sec;
			diff = (diff * 1000000) + (int)tv.tv_usec - (int)chip_usec;

			if((diff > 1000) || (diff < -1000))
			{
				solo6x10_set_current_time(solo6x10);
				//dbg_msg("time diff : %08x\n", diff);
				diff = 0;
			}
			else if(diff)
			{
				int usec_lsb = solo6x10->usec_lsb;
				usec_lsb -= diff / 4;
				if(usec_lsb < 0)
					usec_lsb = 0;
				else if(usec_lsb > 255)
					usec_lsb = 255;
				solo6x10->usec_lsb = (unsigned int)usec_lsb;
				solo6x10_reg_write(solo6x10, 0x0D20, solo6x10->usec_lsb);
				//dbg_msg("diff:%d usec_lsb:%02x\n", diff, usec_lsb);
			}
		}
		#endif
	}
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 19))
static irqreturn_t solo6x10_interrupt(int irq, void *dev_instance)
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 4, 23))
static irqreturn_t solo6x10_interrupt(int irq, void *dev_instance, struct pt_regs *regs)
#else
static void solo6x10_interrupt(int irq, void *dev_instance, struct pt_regs *regs)
#endif
{
	struct SOLO6x10 *solo6x10;
	u32 status;

	solo6x10 = (struct SOLO6x10 *)dev_instance;
	status = solo6x10_reg_read(solo6x10, REG_INTR_STAT);

	if(!status)
	{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 4, 23))
		return IRQ_NONE;
#else
		return;
#endif
	}

	if(status & (~solo6x10->interrupt_enable_mask))
	{
		dbg_msg("interrupt status : %08x\n", status);
		solo6x10_reg_write(solo6x10, REG_INTR_STAT, status & (~solo6x10->interrupt_enable_mask));
		status &= solo6x10->interrupt_enable_mask;
	}

	if(status & INTR_PCI_ERR)
		solo6x10_p2m_error_interrupt(solo6x10);

	if(status & INTR_P2M(0))
		solo6x10_p2m_interrupt(solo6x10, 0);

	if(status & INTR_P2M(1))
		solo6x10_p2m_interrupt(solo6x10, 1);

	if(status & INTR_P2M(2))
		solo6x10_p2m_interrupt(solo6x10, 2);

	if(status & INTR_P2M(3))
		solo6x10_p2m_interrupt(solo6x10, 3);

	if(status & INTR_IIC)
		solo6x10_i2c_interrupt(solo6x10);

	if(status & INTR_ENCODER)
		solo6x10_encoder_interrupt(solo6x10);

	if(status & INTR_DECODER)
		solo6x10_decoder_interrupt(solo6x10);

	if(status & INTR_G723)
		solo6x10_g723_interrupt(solo6x10);

	if(status & INTR_UART(0))
		solo6x10_uart_interrupt(solo6x10, 0);

	if(status & INTR_UART(1))
		solo6x10_uart_interrupt(solo6x10, 1);

	if(status & INTR_VIDEO_SYNC)
	{
		u32 vi_status0 = solo6x10_reg_read(solo6x10, REG_VI_STATUS0);

		solo6x10_reg_write(solo6x10, REG_INTR_STAT, INTR_VIDEO_SYNC);
		if(solo6x10->pdev->device == 0x6110)
		{
			solo6x10_timer_sync(solo6x10);
			solo6x10_decoder_update(solo6x10, (vi_status0>>3) & 1);
		}
	}

	if(status & INTR_MOTION)
		solo6x10_motion_interrupt(solo6x10);

	if(status & INTR_GPIO)
	{
		u32 gpio_int_status;

		gpio_int_status = solo6x10_reg_read(solo6x10, REG_GPIO_INT_ACK_STA);
		solo6x10_reg_write(solo6x10, REG_GPIO_INT_ACK_STA, gpio_int_status);
		solo6x10_reg_write(solo6x10, REG_INTR_STAT, INTR_GPIO);
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 4, 23))
	return IRQ_HANDLED;
#endif
}

void gpio_mode(struct SOLO6x10 *solo6x10, unsigned int port_mask, unsigned int mode)
{
	int port;
	unsigned int ret;

	ret = solo6x10_reg_read(solo6x10, REG_GPIO_CONFIG_0);

	for(port=0; port<16; port++)
	{
		if(!((1<<port)&port_mask))
			continue;

		ret &= (~(3<<(port<<1)));
		ret |= ((mode&3)<<(port<<1));
	}

	solo6x10_reg_write(solo6x10, REG_GPIO_CONFIG_0, ret);

	ret = solo6x10_reg_read(solo6x10, REG_GPIO_CONFIG_1);

	for(port=0; port<16; port++)
	{
		if(!((1<<(port+16))&port_mask))
			continue;

		ret |= 1<<(port+16);

		if(!mode)
			ret &= ~(1<<port);
		else
			ret |= 1<<port;
	}

	solo6x10_reg_write(solo6x10, REG_GPIO_CONFIG_1, ret);
}

void gpio_set(struct SOLO6x10 *solo6x10, unsigned int value)
{
	solo6x10_reg_write(solo6x10, REG_GPIO_DATA_OUT, solo6x10_reg_read(solo6x10, REG_GPIO_DATA_OUT) | value);
}

void gpio_clear(struct SOLO6x10 *solo6x10, unsigned int value)
{
	solo6x10_reg_write(solo6x10, REG_GPIO_DATA_OUT, solo6x10_reg_read(solo6x10, REG_GPIO_DATA_OUT) & ~value);
}

void solo6x10_remove_subdev(struct SOLO6x10 *solo6x10)
{
	if(solo6x10->uart)
		solo6x10_uart_exit(solo6x10);

	if(solo6x10->g723)
		solo6x10_g723_exit(solo6x10);

	if(solo6x10->dec)
		solo6x10_dec_exit(solo6x10);

	if(solo6x10->enc)
		solo6x10_enc_exit(solo6x10);

	if(solo6x10->fb)
		solo6x10fb_exit(solo6x10);

	if(solo6x10->disp)
		solo6x10_disp_exit(solo6x10);

	if(solo6x10->i2c)
		solo6x10_i2c_exit(solo6x10);

	if(solo6x10->vin)
		solo6x10_vin_exit(solo6x10);

	if(solo6x10->p2m_dev)
		solo6x10_p2m_exit(solo6x10);
}

static int tw2815_clk_delay_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	struct SOLO6x10 *solo6x10;
	char *str;
	int i;
	u8 delay = 0;

	solo6x10 = data;
	str = kmalloc(count, GFP_KERNEL);
	if(!str)
		return -ENOMEM;

	if(copy_from_user(str, buffer, count))
	{
		kfree(str);
		return -EFAULT;
	}

	if((count > 2) && (str[0] == '0') && (str[1] == 'x'))
	{
		for(i=2; i<count; i++)
		{
			if((str[i] >= '0') && (str[i] <= '9'))
			{
				delay *= 16;
				delay += str[i] - '0';
			}
			else if((str[i] >= 'a') && (str[i] <= 'f'))
			{
				delay *= 16;
				delay += str[i] - 'a' + 10;
			}
			else if((str[i] >= 'A') && (str[i] <= 'F'))
			{
				delay *= 16;
				delay += str[i] - 'A' + 10;
			}
			else
				break;
		}
	}
	else
	{
		for(i=0; i<count; i++)
		{
			if((str[i] >= '0') && (str[i] <= '9'))
			{
				delay *= 10;
				delay += str[i] - '0';
			}
			else
				break;
		}
	}

	kfree(str);

	solo6x10_i2c_writebyte(solo6x10, 0, 0x28+0, 0x4d, delay);
	solo6x10_i2c_writebyte(solo6x10, 0, 0x28+1, 0x4d, delay);
	solo6x10_i2c_writebyte(solo6x10, 0, 0x28+2, 0x4d, delay);
	solo6x10_i2c_writebyte(solo6x10, 0, 0x28+3, 0x4d, delay);

	return count;
}

static int vin_clk_delay_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	struct SOLO6x10 *solo6x10;
	char *str;
	int i;
	u8 delay = 0;

	solo6x10 = (struct SOLO6x10 *)data;
	str = kmalloc(count, GFP_KERNEL);
	if(!str)
		return -ENOMEM;

	if(copy_from_user(str, buffer, count))
	{
		kfree(str);
		return -EFAULT;
	}

	if((count > 2) && (str[0] == '0') && (str[1] == 'x'))
	{
		for(i=2; i<count; i++)
		{
			if((str[i] >= '0') && (str[i] <= '9'))
			{
				delay *= 16;
				delay += str[i] - '0';
			}
			else if((str[i] >= 'a') && (str[i] <= 'f'))
			{
				delay *= 16;
				delay += str[i] - 'a' + 10;
			}
			else if((str[i] >= 'A') && (str[i] <= 'F'))
			{
				delay *= 16;
				delay += str[i] - 'A' + 10;
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		for(i=0; i<count; i++)
		{
			if((str[i] >= '0') && (str[i] <= '9'))
			{
				delay *= 10;
				delay += str[i] - '0';
			}
			else
			{
				break;
			}
		}
	}

	kfree(str);

	if(delay > 3)
	{
		dbg_msg("wrong delay value : %d\n", delay);
	}
	else
	{
		solo6x10_reg_write(solo6x10, REG_SYS_VCLK,
			SYS_CONFIG_VOUT_CLK_SELECT(2) |
			SYS_CONFIG_VIN1415_DELAY(delay) |
			SYS_CONFIG_VIN1213_DELAY(delay) |
			SYS_CONFIG_VIN1011_DELAY(delay) |
			SYS_CONFIG_VIN0809_DELAY(delay) |
			SYS_CONFIG_VIN0607_DELAY(delay) |
			SYS_CONFIG_VIN0405_DELAY(delay) |
			SYS_CONFIG_VIN0203_DELAY(delay) |
			SYS_CONFIG_VIN0001_DELAY(delay));
	}

	return count;
}

static int encoder_ability_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	struct SOLO6x10 *solo6x10;
	char *str;
	int i;
	unsigned int value = 0;

	solo6x10 = (struct SOLO6x10 *)data;
	str = kmalloc(count, GFP_KERNEL);
	if(!str)
		return -ENOMEM;

	if(copy_from_user(str, buffer, count))
	{
		kfree(str);
		return -EFAULT;
	}

	if((count > 2) && (str[0] == '0') && (str[1] == 'x'))
	{
		for(i=2; i<count; i++)
		{
			if((str[i] >= '0') && (str[i] <= '9'))
			{
				value *= 16;
				value += str[i] - '0';
			}
			else if((str[i] >= 'a') && (str[i] <= 'f'))
			{
				value *= 16;
				value += str[i] - 'a' + 10;
			}
			else if((str[i] >= 'A') && (str[i] <= 'F'))
			{
				value *= 16;
				value += str[i] - 'A' + 10;
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		for(i=0; i<count; i++)
		{
			if((str[i] >= '0') && (str[i] <= '9'))
			{
				value *= 10;
				value += str[i] - '0';
			}
			else
			{
				break;
			}
		}
	}

	kfree(str);

	value = change_encoder_ability(solo6x10, value);
	change_decoder_ability(solo6x10, SOLO6010_CODEC_ABILITY_TOTAL - value);

	return count;
}

static int decoder_ability_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	struct SOLO6x10 *solo6x10;
	char *str;
	int i;
	unsigned int value = 0;

	solo6x10 = (struct SOLO6x10 *)data;
	str = kmalloc(count, GFP_KERNEL);
	if(!str)
		return -ENOMEM;

	if(copy_from_user(str, buffer, count))
	{
		kfree(str);
		return -EFAULT;
	}

	if((count > 2) && (str[0] == '0') && (str[1] == 'x'))
	{
		for(i=2; i<count; i++)
		{
			if((str[i] >= '0') && (str[i] <= '9'))
			{
				value *= 16;
				value += str[i] - '0';
			}
			else if((str[i] >= 'a') && (str[i] <= 'f'))
			{
				value *= 16;
				value += str[i] - 'a' + 10;
			}
			else if((str[i] >= 'A') && (str[i] <= 'F'))
			{
				value *= 16;
				value += str[i] - 'A' + 10;
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		for(i=0; i<count; i++)
		{
			if((str[i] >= '0') && (str[i] <= '9'))
			{
				value *= 10;
				value += str[i] - '0';
			}
			else
			{
				break;
			}
		}
	}

	kfree(str);

	value = change_decoder_ability(solo6x10, value);
	change_encoder_ability(solo6x10, SOLO6010_CODEC_ABILITY_TOTAL - value);

	return count;
}

static int solo6x10_chip_option(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	struct SOLO6x10 *solo6x10;
	u32 chip_option;
	int len;

	solo6x10 = (struct SOLO6x10 *)data;
	chip_option = solo6x10_reg_read(solo6x10, REG_CHIP_OPTION);
	chip_option &= 0x07;
	switch(chip_option)
	{
		case 7:
			len = sprintf(page, "SOLO6x10-16");
			break;
		case 6:
			len = sprintf(page, "SOLO6x10-9");
			break;
		case 5:
			len = sprintf(page, "SOLO6x10-4");
			break;
		default:
			len = sprintf(page, "Unknown chip_option : %d", chip_option);
	}

	*eof = 1;

	return len;
}

int solo6x10_create_proc_entry(struct SOLO6x10 *solo6x10)
{
	struct proc_dir_entry *entry;

	entry = create_proc_entry("tw2815_clk_delay", S_IFREG | S_IRUSR, solo6x10->proc);
	if(entry)
	{
		entry->data = solo6x10;
		entry->write_proc = tw2815_clk_delay_write;
	}

	entry = create_proc_entry("vin_clk_delay", S_IFREG | S_IRUSR, solo6x10->proc);
	if(entry)
	{
		entry->data = solo6x10;
		entry->write_proc = vin_clk_delay_write;
	}

	entry = create_proc_entry("encoder_ability", S_IFREG | S_IRUSR, solo6x10->proc);
	if(entry)
	{
		entry->data = solo6x10;
		entry->write_proc = encoder_ability_write;
	}

	entry = create_proc_entry("decoder_ability", S_IFREG | S_IRUSR, solo6x10->proc);
	if(entry)
	{
		entry->data = solo6x10;
		entry->write_proc = decoder_ability_write;
	}

	entry = create_proc_entry("chip_option", S_IFREG | S_IRUSR, solo6x10->proc);
	if(entry)
	{
		entry->data = solo6x10;
		entry->read_proc = solo6x10_chip_option;
	}

	return 0;
}

void solo6x10_remove_proc_entry(struct SOLO6x10 *solo6x10)
{
	remove_proc_entry("chip_option", solo6x10->proc);
	remove_proc_entry("decoder_ability", solo6x10->proc);
	remove_proc_entry("encoder_ability", solo6x10->proc);
	remove_proc_entry("vin_clk_delay", solo6x10->proc);
	remove_proc_entry("tw2815_clk_delay", solo6x10->proc);
}

int sys_clock_config(struct SOLO6x10 *solo6x10)
{
	u32 cfg_sdram_size;

	switch(sdram_size)
	{
		case 256:
			cfg_sdram_size = 3;
			break;
		case 128:
			cfg_sdram_size = 2;
			break;
		case 64:
			cfg_sdram_size = 1;
			break;
		case 32:
			cfg_sdram_size = 0;
			break;
		default:
			cfg_sdram_size = 2;
	}

	dbg_msg("cfg_sdram_size:%d\n", cfg_sdram_size);

	if(fdma_control)
	{
		u32 sys_config;
		dbg_msg("REG_DMA_CTRL : %08x\n", fdma_control);
		sys_config = SYS_CONFIG_SDRAM64BIT;

		solo6x10_reg_write(solo6x10, REG_DMA_CTRL, fdma_control);
		udelay(1);
		solo6x10_reg_write(solo6x10, REG_SYS_CONFIG, sys_config | SYS_CONFIG_RESET);
		solo6x10_reg_write(solo6x10, REG_SYS_CONFIG, sys_config);
		udelay(5000);
	}
	else
	{
		u32 sys_config;
		u32 fdma_ctrl;

		u32 clk_invert;
		u32 strobe_sel;
		u32 rd_data_sel;
		u32 rd_clk_sel;
		u32 rd_latency;

		int error_count;
		u32 fdma_ok = 0;

		sys_config = SYS_CONFIG_SDRAM64BIT;
		sys_config |= (1<<22);	// live to PB clock 0:27MHz, 1:54MHz, 2:PLL/2, 3: PLL

		for(clk_invert=0; clk_invert<2; clk_invert++)
		for(strobe_sel=0; strobe_sel<2; strobe_sel++)
		for(rd_data_sel=0; rd_data_sel<2; rd_data_sel++)
		for(rd_clk_sel=0; rd_clk_sel<2; rd_clk_sel++)
		for(rd_latency=0; rd_latency<4; rd_latency++)
		{
			schedule();

			fdma_ctrl = DMA_CTRL_REFRESH_CYCLE(1) | DMA_CTRL_SDRAM_SIZE(cfg_sdram_size);
			//fdma_ctrl = (1<<16)/* refresh cycle / 16 */ | DMA_CTRL_REFRESH_CYCLE(4) | DMA_CTRL_SDRAM_SIZE(cfg_sdram_size);
			if(clk_invert)			fdma_ctrl |= DMA_CTRL_SDRAM_CLK_INVERT;
			if(strobe_sel)			fdma_ctrl |= DMA_CTRL_STROBE_SELECT;
			if(rd_data_sel)			fdma_ctrl |= DMA_CTRL_READ_DATA_SELECT;
			if(rd_clk_sel)			fdma_ctrl |= DMA_CTRL_READ_CLK_SELECT;
			fdma_ctrl |= DMA_CTRL_LATENCY(rd_latency);

			solo6x10_reg_write(solo6x10, REG_DMA_CTRL, fdma_ctrl);
			udelay(1);
			solo6x10_reg_write(solo6x10, REG_SYS_CONFIG, sys_config | SYS_CONFIG_RESET);
			solo6x10_reg_write(solo6x10, REG_SYS_CONFIG, sys_config);
			udelay(5000);

			error_count = p2m_test(solo6x10, solo6x10->osg_ext_addr, 0x10000);
			if(!error_count)
			{
				dbg_msg("fdma_ctrl : 0x%08x\n", fdma_ctrl);
				if(!fdma_ok)
					fdma_ok = fdma_ctrl;
			}
		}

		if(fdma_ok)
		{
			dbg_msg("REG_DMA_CTRL : 0x%08x\n", fdma_ok);

			solo6x10_reg_write(solo6x10, REG_DMA_CTRL, fdma_ok);
			udelay(1);
			solo6x10_reg_write(solo6x10, REG_SYS_CONFIG, sys_config | SYS_CONFIG_RESET);
			solo6x10_reg_write(solo6x10, REG_SYS_CONFIG, sys_config);
			udelay(5000);
		}
	}

	if(p2m_test(solo6x10, (((sdram_size<<20)/4)*1)-0x10000, 0x10000) != 0)
		return -1;
	if(p2m_test(solo6x10, (((sdram_size<<20)/4)*2)-0x10000, 0x10000) != 0)
		return -1;
	if(p2m_test(solo6x10, (((sdram_size<<20)/4)*3)-0x10000, 0x10000) != 0)
		return -1;
	if(p2m_test(solo6x10, (((sdram_size<<20)/4)*4)-0x10000, 0x10000) != 0)
		return -1;

	return 0;
}
static int __devinit solo6x10_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	struct SOLO6x10 *solo6x10;
	int retval;
	u8 tmp;

	pci_read_config_byte(pdev, 0x0C, &tmp);
	if(tmp < 2)
		pci_write_config_byte(pdev, 0x0C, 0x00);	// Cache line size
	pci_write_config_byte(pdev, 0x0D, 0x80);	// Latency Timer

	pci_write_config_byte(pdev, 0x40, 0x00);	// Retry Timeout disable
	pci_write_config_byte(pdev, 0x41, 0x00);	// TRDY Timeout disable

	retval = pci_enable_device(pdev);
	if(retval)
		return retval;

	pci_set_master(pdev);

	retval = pci_request_regions(pdev, SOLO6x10_NAME);
	if(retval)
	{
		dbg_msg("pci_request_regions error!\n");
		pci_disable_device(pdev);
		return retval;
	}

	solo6x10 = alloc_solo6x10_dev(pdev);
	if(solo6x10 == NULL)
	{
		pci_release_regions(pdev);
		pci_disable_device(pdev);
		return -ENOMEM;
	}

	solo6x10_disable_interrupt(solo6x10, 0xFFFFFFFF);

	if(solo6x10->pdev->device == 0x6110)
	{
		u32 sys_clock_MHz = 135;
		u32 pll_DIVQ;
		u32 pll_DIVF;

		solo6x10->system_clock = sys_clock_MHz * 1000000;

		if(sys_clock_MHz < 125)
		{
			pll_DIVQ = 3;
			pll_DIVF = (sys_clock_MHz*4)/3 - 1;
		}
		else
		{
			pll_DIVQ = 2;
			pll_DIVF = (sys_clock_MHz*2)/3 - 1;
		}

		solo6x10_reg_write(solo6x10, 0x0020,
			(1<<20) |	// PLL_RANGE
			(8<<15) |	// PLL_DIVR
			(pll_DIVQ<<12) |	// PLL_DIVQ
			(pll_DIVF<<4) |		// PLL_DIVF
			(0<<3) |
			(0<<2) |
			(1<<1) |
			(0<<0));

		solo6x10_reg_write(solo6x10, REG_SYS_CONFIG,
			SYS_CONFIG_SDRAM64BIT);

		solo6x10_reg_write(solo6x10, REG_TIMER_CLK_NUM, sys_clock_MHz-1);

		mdelay(1);	// PLL Locking time (1ms)

		solo6x10_reg_write(solo6x10, REG_DMA_CTRL,
			DMA_CTRL_REFRESH_CYCLE(1) |
			DMA_CTRL_SDRAM_SIZE(2) |
			DMA_CTRL_SDRAM_CLK_INVERT |
			DMA_CTRL_READ_CLK_SELECT |
			DMA_CTRL_LATENCY(1));

		solo6x10_reg_write(solo6x10, 0x0008, 3<<8);

		dbg_msg("system clock %dMhz (DIVQ:%d, DIVF:%d)\n", sys_clock_MHz, pll_DIVQ, pll_DIVF);

		solo6x10->usec_lsb = 0x3f;
		solo6x10_set_current_time(solo6x10);
	}
	else
	{
		u32 sys_clock_MHz = 108;

		solo6x10->system_clock = sys_clock_MHz * 1000000;

		solo6x10_reg_write(solo6x10, REG_SYS_CONFIG,
			SYS_CONFIG_SDRAM64BIT |
			SYS_CONFIG_INPUTDIV(25) |
			SYS_CONFIG_FEEDBACKDIV((sys_clock_MHz*2)-2) |
			SYS_CONFIG_OUTDIV(3));

		solo6x10_reg_write(solo6x10, REG_TIMER_CLK_NUM, sys_clock_MHz-1);

		mdelay(1);	// PLL Locking time (1ms)

		solo6x10_reg_write(solo6x10, REG_DMA_CTRL,
			DMA_CTRL_REFRESH_CYCLE(1) |
			DMA_CTRL_SDRAM_SIZE(2) |
			DMA_CTRL_SDRAM_CLK_INVERT |
			DMA_CTRL_READ_CLK_SELECT |
			DMA_CTRL_LATENCY(1));

		solo6x10_reg_write(solo6x10, 0x0008, 1<<8);
	}

	solo6x10_reg_write(solo6x10, REG_TIMER_WATCHDOG, 0xff);		// disable watchdog

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 18))
	retval = request_irq(solo6x10->irq, solo6x10_interrupt, IRQF_SHARED, SOLO6x10_NAME, solo6x10);
#else
	retval = request_irq(solo6x10->irq, solo6x10_interrupt, SA_SHIRQ, SOLO6x10_NAME, solo6x10);
#endif
	if(retval)
	{
		dbg_msg("request_irq error!(%d)\n", solo6x10->irq);
		free_solo6x10_dev(solo6x10);
		pci_release_regions(pdev);
		pci_disable_device(pdev);
		return retval;
	}

	solo6x10_enable_interrupt(solo6x10, INTR_PCI_ERR);

	if(solo6x10_p2m_init(solo6x10) != 0)
		goto error_return;
/*
	if(solo6x10->pdev->device == 0x6110)
	{
		if(sys_clock_config(solo6x10) != 0)
			goto error_return;

		solo6x10->usec_lsb = 0x3f;
		solo6x10_set_current_time(solo6x10);
	}
*/
	if(solo6x10_vin_init(solo6x10) != 0)
		goto error_return;

	if(solo6x10_disp_init(solo6x10) != 0)
		goto error_return;

	if(solo6x10fb_init(solo6x10) != 0)
		goto error_return;

	// video reset
	gpio_mode(solo6x10, (1<<5) | (1<<4), 1);

	gpio_clear(solo6x10, (1<<5) | (1<<4));
	udelay(100);
	gpio_set(solo6x10, (1<<5) | (1<<4));
	udelay(100);

	gpio_mode(solo6x10, (1<<3) | (1<<2) | (1<<1) | (1<<0), 2);

	if(solo6x10_i2c_init(solo6x10) != 0)
		goto error_return;

	if(tw2815_setup(solo6x10, 0, 0x28+0) != 0)
		tw2865_setup(solo6x10, 0, 0x28+0);
	else
		saa7128_setup(solo6x10, 1, 0x46);

	if(tw2815_setup(solo6x10, 0, 0x28+1) != 0)
		tw2865_setup(solo6x10, 0, 0x28+1);
	if(tw2815_setup(solo6x10, 0, 0x28+2) != 0)
		tw2865_setup(solo6x10, 0, 0x28+2);
	if(tw2815_setup(solo6x10, 0, 0x28+3) != 0)
		tw2865_setup(solo6x10, 0, 0x28+3);

	if(solo6x10_enc_init(solo6x10))
		goto error_return;

	if(solo6x10_dec_init(solo6x10))
		goto error_return;

	if(solo6x10_g723_init(solo6x10))
		goto error_return;

	if(solo6x10_uart_init(solo6x10))
		goto error_return;

	solo6x10_enable_interrupt(solo6x10, INTR_VIDEO_SYNC);

	pci_set_drvdata(pdev, (void *)solo6x10);

	solo6x10_create_proc_entry(solo6x10);

	return 0;

error_return:

	solo6x10_disable_interrupt(solo6x10, INTR_VIDEO_SYNC);

	solo6x10_remove_subdev(solo6x10);

	gpio_clear(solo6x10, (1<<5) | (1<<4));

	solo6x10_disable_interrupt(solo6x10, INTR_PCI_ERR);

	if(solo6x10->interrupt_enable_mask)
	{
		dbg_msg("interrupt_enable_mask : %08x\n", solo6x10->interrupt_enable_mask);
		solo6x10_disable_interrupt(solo6x10, 0xFFFFFFFF);
	}
	free_irq(solo6x10->irq, solo6x10);
	free_solo6x10_dev(solo6x10);
	pci_release_regions(pdev);
	pci_disable_device(pdev);

	return -1;
}

static void __devexit solo6x10_remove(struct pci_dev *pdev)
{
	struct SOLO6x10 *solo6x10;

	solo6x10 = pci_get_drvdata(pdev);

	solo6x10_remove_proc_entry(solo6x10);

	solo6x10_remove_subdev(solo6x10);

	gpio_clear(solo6x10, (1<<5) | (1<<4));

	solo6x10_disable_interrupt(solo6x10, INTR_PCI_ERR);

	if(solo6x10->interrupt_enable_mask)
	{
		dbg_msg("interrupt_enable_mask : %08x\n", solo6x10->interrupt_enable_mask);
		solo6x10_disable_interrupt(solo6x10, 0xFFFFFFFF);
	}
	free_irq(solo6x10->irq, solo6x10);
	free_solo6x10_dev(solo6x10);
	pci_release_regions(pdev);
	pci_disable_device(pdev);
}

static struct pci_device_id solo6x10_id_table[] = {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 4, 23))
	{PCI_DEVICE(PCI_VENDOR_ID_SOFTLOGIC, PCI_DEVICE_ID_SOLO6010)},
	{PCI_DEVICE(PCI_VENDOR_ID_SOFTLOGIC, PCI_DEVICE_ID_SOLO6110)},
#else
	{PCI_VENDOR_ID_SOFTLOGIC, PCI_DEVICE_ID_SOLO6010, PCI_ANY_ID, PCI_ANY_ID, },
	{PCI_VENDOR_ID_SOFTLOGIC, PCI_DEVICE_ID_SOLO6110, PCI_ANY_ID, PCI_ANY_ID, },
#endif
	{0,}
};

MODULE_DEVICE_TABLE(pci, solo6x10_id_table);

static struct pci_driver solo6x10_driver = {
	.name = SOLO6x10_NAME,
	.id_table = solo6x10_id_table,
	.probe = solo6x10_probe,
	.remove = solo6x10_remove,
};

static int __init solo6x10_driver_init(void)
{
	int retval;

	solo6x10_uart_register_driver();

	retval = solo6x10_device_init();
	if(retval < 0)
		return retval;

	retval = pci_register_driver(&solo6x10_driver);
	if(retval < 0)
		solo6x10_device_exit();
	else
		retval = 0;

	return retval;
}

static void __exit solo6x10_driver_exit(void)
{
	pci_unregister_driver(&solo6x10_driver);
	solo6x10_device_exit();
	solo6x10_uart_unregister_driver();
}

module_init(solo6x10_driver_init);
module_exit(solo6x10_driver_exit);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
module_param(max_channel, uint, 0);
module_param(video_type, uint, 0);
module_param(video_hsize, uint, 0);
module_param(vout_hstart, uint, 0);
module_param(vout_vstart, uint, 0);
module_param(vin_hstart, uint, 0);
module_param(vin_vstart, uint, 0);
module_param(fdma_control, uint, 0);
#else
MODULE_PARM(max_channel, "i");
MODULE_PARM(video_type, "i");
MODULE_PARM(video_hsize, "i");
MODULE_PARM(vout_hstart, "i");
MODULE_PARM(vout_vstart, "i");
MODULE_PARM(vin_hstart, "i");
MODULE_PARM(vin_vstart, "i");
MODULE_PARM(fdma_control, "i");
#endif

MODULE_AUTHOR("Junho Jeong");
MODULE_LICENSE("Dual BSD/GPL");

