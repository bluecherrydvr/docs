#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/delay.h>

#include "solo6010.h"
#include "saa7128.h"
#include "tw2815.h"

#define PCI_VENDOR_ID_SOFTLOGIC		0x9413
#define PCI_DEVICE_ID_SOLO6010		0x6010
#define SOLO6010_NAME			"solo6010"

#define SYS_CLK_MHz			104

static unsigned int max_channel = 16;
static unsigned int video_type = 0;
static unsigned int video_hsize = 704;
static unsigned int vout_hstart = 6;
static unsigned int vout_vstart = 8;
static unsigned int vin_hstart = 8;
static unsigned int vin_vstart = 2;

static unsigned int device_count = 0;

int solo6010_clock_config(struct SOLO6010 *solo6010);

static struct SOLO6010 *alloc_solo6010_dev(struct pci_dev *pdev)
{
	struct SOLO6010 *solo6010;

	solo6010 = kmalloc(sizeof(struct SOLO6010), GFP_KERNEL);
	if(solo6010 == NULL)
	{
		dbg_msg("kmalloc error!\n");
		return NULL;
	}

	memset(solo6010, 0, sizeof(struct SOLO6010));

	solo6010->pdev = pdev;
	solo6010->reg_base = ioremap_nocache(pdev->resource[0].start, pdev->resource[0].end - pdev->resource[0].start);
	if(solo6010->reg_base == NULL)
	{
		dbg_msg("ioremap_nocache error!\n");
		goto error_return;
	}

	spin_lock_init(&solo6010->reg_io_lock);

	solo6010->irq = pdev->irq;

	solo6010->system_clock = 108000000;

	solo6010->max_channel = max_channel;
	solo6010->video_type = video_type;
	solo6010->video_hsize = video_hsize;
	solo6010->video_vsize = (video_type == 0) ? 240 : 288;
	solo6010->vout_hstart = vout_hstart;
	solo6010->vout_vstart = vout_vstart;
	solo6010->vin_hstart = vin_hstart;
	solo6010->vin_vstart = vin_vstart;

	solo6010->encoder_ability = SOLO6010_CODEC_ABILITY_MAX;
	solo6010->decoder_ability = SOLO6010_CODEC_ABILITY_TOTAL - solo6010->encoder_ability;

								//start			//size
	solo6010->disp_ext_addr		= 0x00000000;	//0x00480000
	solo6010->dec2live_ext_addr	= 0x00480000;	//0x00240000
	solo6010->osg_ext_addr		= 0x006C0000;	//0x00120000
	solo6010->motion_ext_addr	= 0x007E0000;	//0x00080000
	solo6010->g723_ext_addr		= 0x00860000;	//0x00010000
	solo6010->cap_ext_addr		= 0x00870000;	//0x01320000 = 0x00120000 * (16+1)
								//0x01B90000;
	solo6010->eosd_ext_addr		= 0x02A00000;	//0x00200000
	solo6010->dref_ext_addr		= 0x02C00000;	//0x01400000 = 0x00140000 * 16
	solo6010->eref_ext_addr		= 0x04000000;	//0x02800000 = 0x00140000 * 32
	solo6010->jpeg_ext_addr		= 0x06800000;
	solo6010->jpeg_ext_size	= 0x00800000;
	solo6010->mp4e_ext_addr		= 0x07000000;
	solo6010->mp4e_ext_size	= 0x00800000;
	solo6010->mp4d_ext_addr		= 0x07800000;
	solo6010->mp4d_ext_size	= 0x00800000;

	solo6010->stream_align = 64;	// 64, 32, 16, 8

	if(solo6010_chip_init(&solo6010->chip, device_count) != 0)
	{
		dbg_msg("solo6010_chip_init error!\n");
		goto error_return;
	}

	dbg_msg("solo6010 chip id : %d\n", solo6010_reg_read(solo6010, 0x001C) & 0x07);

	device_count++;

	solo6010->proc = proc_mkdir(solo6010->chip.name, NULL);

	return solo6010;

error_return:
	if(solo6010->reg_base)
		iounmap((void *)solo6010->reg_base);
	if(solo6010)
		kfree(solo6010);

	return NULL;
}

static void free_solo6010_dev(struct SOLO6010 *solo6010)
{
	if(solo6010->proc)
		remove_proc_entry(solo6010->proc->name, NULL);

	device_count--;

	solo6010_chip_exit(&solo6010->chip);
	iounmap((void *)solo6010->reg_base);
	kfree(solo6010);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 19))
static irqreturn_t solo6010_interrupt(int irq, void *dev_instance)
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 4, 23))
static irqreturn_t solo6010_interrupt(int irq, void *dev_instance, struct pt_regs *regs)
#else
static void solo6010_interrupt(int irq, void *dev_instance, struct pt_regs *regs)
#endif
{
	struct SOLO6010 *solo6010;
	u32 status;

	solo6010 = (struct SOLO6010 *)dev_instance;
	status = solo6010_reg_read(solo6010, REG_INTR_STAT);

	if(!status)
	{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 4, 23))
		return IRQ_NONE;
#else
		return;
#endif
	}

	if(status & (~solo6010->interrupt_enable_mask))
	{
		dbg_msg("interrupt status : %08x\n", status);
		solo6010_reg_write(solo6010, REG_INTR_STAT, status & (~solo6010->interrupt_enable_mask));
		status &= solo6010->interrupt_enable_mask;
	}

	if(status & INTR_PCI_ERR)
	{
		u32 error_status;

		error_status = solo6010_reg_read(solo6010, REG_PCI_ERROR);

		solo6010_p2m_error_interrupt(solo6010, error_status);
		solo6010_reg_write(solo6010, REG_INTR_STAT, INTR_PCI_ERR);

		dbg_msg("solo6010 pci error! (0x%08x)\n", error_status);
	}

	if(status & INTR_P2M(0))
		solo6010_p2m_interrupt(solo6010, 0);

	if(status & INTR_P2M(1))
		solo6010_p2m_interrupt(solo6010, 1);

	if(status & INTR_P2M(2))
		solo6010_p2m_interrupt(solo6010, 2);

	if(status & INTR_P2M(3))
		solo6010_p2m_interrupt(solo6010, 3);

	if(status & INTR_IIC)
		solo6010_i2c_interrupt(solo6010);

	if(status & INTR_ENCODER)
		solo6010_encoder_interrupt(solo6010);

	if(status & INTR_DECODER)
		solo6010_decoder_interrupt(solo6010);

	if(status & INTR_G723)
		solo6010_g723_interrupt(solo6010);

	if(status & INTR_UART(0))
		solo6010_uart_interrupt(solo6010, 0);

	if(status & INTR_UART(1))
		solo6010_uart_interrupt(solo6010, 1);

	if(status & INTR_VIDEO_IN)
	{
		solo6010_reg_write(solo6010, REG_INTR_STAT, INTR_VIDEO_IN);
	}

	if(status & INTR_MOTION)
		solo6010_motion_interrupt(solo6010);

	if(status & INTR_GPIO)
	{
		u32 gpio_int_status;

		gpio_int_status = solo6010_reg_read(solo6010, REG_GPIO_INT_ACK_STA);
		solo6010_reg_write(solo6010, REG_GPIO_INT_ACK_STA, gpio_int_status);
		solo6010_reg_write(solo6010, REG_INTR_STAT, INTR_GPIO);
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 4, 23))
	return IRQ_HANDLED;
#endif
}

void gpio_mode(struct SOLO6010 *solo6010, unsigned int port_mask, unsigned int mode)
{
	int port;
	unsigned int ret;

	ret = solo6010_reg_read(solo6010, REG_GPIO_CONFIG_0);

	for(port=0; port<16; port++)
	{
		if(!((1<<port)&port_mask))
			continue;

		ret &= (~(3<<(port<<1)));
		ret |= ((mode&3)<<(port<<1));
	}

	solo6010_reg_write(solo6010, REG_GPIO_CONFIG_0, ret);

	ret = solo6010_reg_read(solo6010, REG_GPIO_CONFIG_1);

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

	solo6010_reg_write(solo6010, REG_GPIO_CONFIG_1, ret);
}

void gpio_set(struct SOLO6010 *solo6010, unsigned int value)
{
	solo6010_reg_write(solo6010, REG_GPIO_DATA_OUT, solo6010_reg_read(solo6010, REG_GPIO_DATA_OUT) | value);
}

void gpio_clear(struct SOLO6010 *solo6010, unsigned int value)
{
	solo6010_reg_write(solo6010, REG_GPIO_DATA_OUT, solo6010_reg_read(solo6010, REG_GPIO_DATA_OUT) & ~value);
}

void solo6010_remove_subdev(struct SOLO6010 *solo6010)
{
	if(solo6010->uart)
		solo6010_uart_exit(solo6010);

	if(solo6010->g723)
		solo6010_g723_exit(solo6010);

	if(solo6010->dec)
		solo6010_dec_exit(solo6010);

	if(solo6010->enc)
		solo6010_enc_exit(solo6010);

	if(solo6010->fb)
		solo6010fb_exit(solo6010);

	if(solo6010->disp)
		solo6010_disp_exit(solo6010);

	if(solo6010->i2c)
		solo6010_i2c_exit(solo6010);

	if(solo6010->vin)
		solo6010_vin_exit(solo6010);

	if(solo6010->p2m_dev)
		solo6010_p2m_exit(solo6010);
}

static int tw2815_clk_delay_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	struct SOLO6010 *solo6010;
	char *str;
	int i;
	u8 delay = 0;

	solo6010 = data;
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

	solo6010_i2c_writebyte(solo6010, 0, 0x28+0, 0x4d, delay);
	solo6010_i2c_writebyte(solo6010, 0, 0x28+1, 0x4d, delay);
	solo6010_i2c_writebyte(solo6010, 0, 0x28+2, 0x4d, delay);
	solo6010_i2c_writebyte(solo6010, 0, 0x28+3, 0x4d, delay);

	return count;
}

static int vin_clk_delay_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	struct SOLO6010 *solo6010;
	char *str;
	int i;
	u8 delay = 0;

	solo6010 = (struct SOLO6010 *)data;
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
		solo6010_reg_write(solo6010, REG_SYS_VCLK,
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
	struct SOLO6010 *solo6010;
	char *str;
	int i;
	unsigned int value = 0;

	solo6010 = (struct SOLO6010 *)data;
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

	value = change_encoder_ability(solo6010, value);
	change_decoder_ability(solo6010, SOLO6010_CODEC_ABILITY_TOTAL - value);

	return count;
}

static int decoder_ability_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	struct SOLO6010 *solo6010;
	char *str;
	int i;
	unsigned int value = 0;

	solo6010 = (struct SOLO6010 *)data;
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

	value = change_decoder_ability(solo6010, value);
	change_encoder_ability(solo6010, SOLO6010_CODEC_ABILITY_TOTAL - value);

	return count;
}

static int solo6010_chip_option(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	struct SOLO6010 *solo6010;
	u32 chip_option;
	int len;

	solo6010 = (struct SOLO6010 *)data;
	chip_option = solo6010_reg_read(solo6010, REG_CHIP_OPTION);
	chip_option &= 0x07;
	switch(chip_option)
	{
		case 7:
			len = sprintf(page, "SOLO6010-16");
			break;
		case 6:
			len = sprintf(page, "SOLO6010-9");
			break;
		case 5:
			len = sprintf(page, "SOLO6010-4");
			break;
		default:
			len = sprintf(page, "Unknown chip_option : %d", chip_option);
	}

	*eof = 1;

	return len;
}

int solo6010_create_proc_entry(struct SOLO6010 *solo6010)
{
	struct proc_dir_entry *entry;

	entry = create_proc_entry("tw2815_clk_delay", S_IFREG | S_IRUSR, solo6010->proc);
	if(entry)
	{
		entry->data = solo6010;
		entry->write_proc = tw2815_clk_delay_write;
	}

	entry = create_proc_entry("vin_clk_delay", S_IFREG | S_IRUSR, solo6010->proc);
	if(entry)
	{
		entry->data = solo6010;
		entry->write_proc = vin_clk_delay_write;
	}

	entry = create_proc_entry("encoder_ability", S_IFREG | S_IRUSR, solo6010->proc);
	if(entry)
	{
		entry->data = solo6010;
		entry->write_proc = encoder_ability_write;
	}

	entry = create_proc_entry("decoder_ability", S_IFREG | S_IRUSR, solo6010->proc);
	if(entry)
	{
		entry->data = solo6010;
		entry->write_proc = decoder_ability_write;
	}

	entry = create_proc_entry("chip_option", S_IFREG | S_IRUSR, solo6010->proc);
	if(entry)
	{
		entry->data = solo6010;
		entry->read_proc = solo6010_chip_option;
	}

	return 0;
}

void solo6010_remove_proc_entry(struct SOLO6010 *solo6010)
{
	remove_proc_entry("chip_option", solo6010->proc);
	remove_proc_entry("decoder_ability", solo6010->proc);
	remove_proc_entry("encoder_ability", solo6010->proc);
	remove_proc_entry("vin_clk_delay", solo6010->proc);
	remove_proc_entry("tw2815_clk_delay", solo6010->proc);
}

static int __devinit solo6010_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	struct SOLO6010 *solo6010;
	int retval;

	pci_write_config_byte(pdev, 0x0C, 0x20);	// Cache line size
	pci_write_config_byte(pdev, 0x0D, 0x80);	// Latency Timer

	pci_write_config_byte(pdev, 0x40, 0x00);	// Retry Timeout disable
	pci_write_config_byte(pdev, 0x41, 0x00);	// TRDY Timeout disable

	retval = pci_enable_device(pdev);
	if(retval)
		return retval;

	pci_set_master(pdev);

	retval = pci_request_regions(pdev, SOLO6010_NAME);
	if(retval)
	{
		dbg_msg("pci_request_regions error!\n");
		pci_disable_device(pdev);
		return retval;
	}

	solo6010 = alloc_solo6010_dev(pdev);
	if(solo6010 == NULL)
	{
		pci_release_regions(pdev);
		pci_disable_device(pdev);
		return -ENOMEM;
	}

	solo6010_disable_interrupt(solo6010, 0xFFFFFFFF);

	solo6010_reg_write(solo6010, REG_SYS_CONFIG,
		SYS_CONFIG_SDRAM64BIT |
		SYS_CONFIG_INPUTDIV(25) |
		SYS_CONFIG_FEEDBACKDIV((SYS_CLK_MHz*2)-2) |
		SYS_CONFIG_OUTDIV(3));

	solo6010_reg_write(solo6010, REG_TIMER_CLK_NUM, SYS_CLK_MHz-1);
	solo6010->system_clock = SYS_CLK_MHz * 1000000;

	mdelay(1);	// PLL Locking time (1ms)

	solo6010_reg_write(solo6010, REG_DMA_CTRL,
		DMA_CTRL_REFRESH_CYCLE(1) |
		DMA_CTRL_SDRAM_SIZE(2) |
		DMA_CTRL_SDRAM_CLK_INVERT |
		DMA_CTRL_READ_CLK_SELECT |
		DMA_CTRL_LATENCY(1));

	solo6010_reg_write(solo6010, 0x0008, 4<<8);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 18))
	retval = request_irq(solo6010->irq, solo6010_interrupt, IRQF_SHARED, SOLO6010_NAME, solo6010);
#else
	retval = request_irq(solo6010->irq, solo6010_interrupt, SA_SHIRQ, SOLO6010_NAME, solo6010);
#endif
	if(retval)
	{
		dbg_msg("request_irq error!(%d)\n", solo6010->irq);
		free_solo6010_dev(solo6010);
		pci_release_regions(pdev);
		pci_disable_device(pdev);
		return retval;
	}

	solo6010_enable_interrupt(solo6010, INTR_PCI_ERR);

	if(solo6010_p2m_init(solo6010) != 0)
		goto error_return;

	if(solo6010_vin_init(solo6010) != 0)
		goto error_return;

	if(solo6010_disp_init(solo6010) != 0)
		goto error_return;

	if(solo6010fb_init(solo6010) != 0)
		goto error_return;

	// video reset
	gpio_mode(solo6010, (1<<5) | (1<<4), 1);

	gpio_clear(solo6010, (1<<5) | (1<<4));
	udelay(100);
	gpio_set(solo6010, (1<<5) | (1<<4));
	udelay(100);

	gpio_mode(solo6010, (1<<3) | (1<<2) | (1<<1) | (1<<0), 2);

	if(solo6010_i2c_init(solo6010) != 0)
		goto error_return;

	saa7128_setup(solo6010, 1, 0x46);

	tw2815_setup(solo6010, 0, 0x28+0);
	tw2815_setup(solo6010, 0, 0x28+1);
	tw2815_setup(solo6010, 0, 0x28+2);
	tw2815_setup(solo6010, 0, 0x28+3);

	if(solo6010_enc_init(solo6010))
		goto error_return;

	if(solo6010_dec_init(solo6010))
		goto error_return;

	if(solo6010_g723_init(solo6010))
		goto error_return;

	if(solo6010_uart_init(solo6010))
		goto error_return;

	pci_set_drvdata(pdev, (void *)solo6010);

	solo6010_create_proc_entry(solo6010);

	return 0;

error_return:

	solo6010_remove_subdev(solo6010);

	gpio_clear(solo6010, (1<<5) | (1<<4));

	solo6010_disable_interrupt(solo6010, INTR_PCI_ERR);

	if(solo6010->interrupt_enable_mask)
	{
		dbg_msg("interrupt_enable_mask : %08x\n", solo6010->interrupt_enable_mask);
		solo6010_disable_interrupt(solo6010, 0xFFFFFFFF);
	}
	free_irq(solo6010->irq, solo6010);
	free_solo6010_dev(solo6010);
	pci_release_regions(pdev);
	pci_disable_device(pdev);

	return -1;
}

static void __devexit solo6010_remove(struct pci_dev *pdev)
{
	struct SOLO6010 *solo6010;

	solo6010 = pci_get_drvdata(pdev);

	solo6010_remove_proc_entry(solo6010);

	solo6010_remove_subdev(solo6010);

	gpio_clear(solo6010, (1<<5) | (1<<4));

	solo6010_disable_interrupt(solo6010, INTR_PCI_ERR);

	if(solo6010->interrupt_enable_mask)
	{
		dbg_msg("interrupt_enable_mask : %08x\n", solo6010->interrupt_enable_mask);
		solo6010_disable_interrupt(solo6010, 0xFFFFFFFF);
	}
	free_irq(solo6010->irq, solo6010);
	free_solo6010_dev(solo6010);
	pci_release_regions(pdev);
	pci_disable_device(pdev);
}

static struct pci_device_id solo6010_id_table[] = {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 4, 23))
	{PCI_DEVICE(PCI_VENDOR_ID_SOFTLOGIC, PCI_DEVICE_ID_SOLO6010)},
#else
	{PCI_VENDOR_ID_SOFTLOGIC, PCI_DEVICE_ID_SOLO6010, PCI_ANY_ID, PCI_ANY_ID, },
#endif
	{0,}
};

MODULE_DEVICE_TABLE(pci, solo6010_id_table);

static struct pci_driver solo6010_driver = {
	.name = SOLO6010_NAME,
	.id_table = solo6010_id_table,
	.probe = solo6010_probe,
	.remove = solo6010_remove,
};

static int __init solo6010_driver_init(void)
{
	int retval;

	solo6010_uart_register_driver();

	retval = solo6010_device_init();
	if(retval < 0)
		return retval;

	retval = pci_register_driver(&solo6010_driver);
	if(retval < 0)
		solo6010_device_exit();
	else
		retval = 0;

	return retval;
}

static void __exit solo6010_driver_exit(void)
{
	pci_unregister_driver(&solo6010_driver);
	solo6010_device_exit();
	solo6010_uart_unregister_driver();
}

module_init(solo6010_driver_init);
module_exit(solo6010_driver_exit);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
module_param(max_channel, uint, 0);
module_param(video_type, uint, 0);
module_param(video_hsize, uint, 0);
module_param(vout_hstart, uint, 0);
module_param(vout_vstart, uint, 0);
module_param(vin_hstart, uint, 0);
module_param(vin_vstart, uint, 0);
#else
MODULE_PARM(max_channel, "i");
MODULE_PARM(video_type, "i");
MODULE_PARM(video_hsize, "i");
MODULE_PARM(vout_hstart, "i");
MODULE_PARM(vout_vstart, "i");
MODULE_PARM(vin_hstart, "i");
MODULE_PARM(vin_vstart, "i");
#endif

MODULE_AUTHOR("Junho Jeong");
MODULE_LICENSE("Dual BSD/GPL");

