#ifndef __SOLO6010_H
#define __SOLO6010_H

#include <linux/pci.h>
#include <linux/proc_fs.h>
#include <asm/delay.h>
#include <linux/version.h>

#include "solo6010_register.h"
#include "device.h"

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 4, 27))
#define __user
#endif

#define dbg_msg(str, args...)		printk(str, ##args)

struct SOLO6010
{
	struct pci_dev *pdev;
	spinlock_t reg_io_lock;
	volatile u8 *reg_base;
	int irq;
	u32 interrupt_enable_mask;

	unsigned int system_clock;

	unsigned int max_channel;
	unsigned int video_type;
	unsigned int video_hsize;
	unsigned int video_vsize;
	unsigned int vout_hstart;
	unsigned int vout_vstart;
	unsigned int vin_hstart;
	unsigned int vin_vstart;
	unsigned int encoder_ability;
	unsigned int decoder_ability;

	u32 disp_ext_addr;
	u32 dec2live_ext_addr;
	u32 osg_ext_addr;
	u32 motion_ext_addr;
	u32 g723_ext_addr;
	u32 cap_ext_addr;
	u32 eosd_ext_addr;
	u32 dref_ext_addr;
	u32 eref_ext_addr;
	u32 jpeg_ext_addr;
	u32 jpeg_ext_size;
	u32 mp4e_ext_addr;
	u32 mp4e_ext_size;
	u32 mp4d_ext_addr;
	u32 mp4d_ext_size;

	u32 stream_align;

	dev_t dev_base;
	solo6010_chip_t chip;

	void *disp;
	void *fb;
	void *enc;
	void *dec;
	void *g723;
	void *vin;
	void *uart;

	void *p2m_dev;
	void *i2c;
	struct semaphore i2c_mutex;
	void *i2c_current;

	struct proc_dir_entry *proc;
};


static inline u32 solo6010_reg_read(struct SOLO6010 *solo6010, int reg)
{
	unsigned long flags;
	u32 ret;
	u16 val;

	spin_lock_irqsave(&solo6010->reg_io_lock, flags);

	ret = readl(solo6010->reg_base + (reg));
	rmb();
	pci_read_config_word(solo6010->pdev, PCI_STATUS, &val);
	rmb();

	spin_unlock_irqrestore(&solo6010->reg_io_lock, flags);

	return ret;
}

static inline void solo6010_reg_write(struct SOLO6010 *solo6010, int reg, u32 data)
{
	unsigned long flags;
	u16 val;

	spin_lock_irqsave(&solo6010->reg_io_lock, flags);

	writel(data, solo6010->reg_base + (reg));
	wmb();
	pci_read_config_word(solo6010->pdev, PCI_STATUS, &val);
	rmb();

	spin_unlock_irqrestore(&solo6010->reg_io_lock, flags);
}

static inline void solo6010_enable_interrupt(struct SOLO6010 *solo6010, u32 mask)
{
	solo6010->interrupt_enable_mask |= mask;
	solo6010_reg_write(solo6010, REG_INTR_ENABLE, solo6010->interrupt_enable_mask);
}

static inline void solo6010_disable_interrupt(struct SOLO6010 *solo6010, u32 mask)
{
	solo6010->interrupt_enable_mask &= ~mask;
	solo6010_reg_write(solo6010, REG_INTR_ENABLE, solo6010->interrupt_enable_mask);
}

#include "p2m.h"
#include "vin.h"
#include "disp.h"
#include "fb.h"
#include "enc.h"
#include "dec.h"
#include "g723.h"
#include "i2c.h"
#include "uart.h"

#define SOLO6010_CODEC_ABILITY_TOTAL	6
#define SOLO6010_CODEC_ABILITY_MAX		4
#define SOLO6010_CODEC_ABILITY_MIN		2

#endif

