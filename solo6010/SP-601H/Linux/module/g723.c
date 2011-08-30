#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
#include <linux/mempool.h>
#endif
#include <linux/poll.h>
#include <asm/uaccess.h>

#include "solo6010.h"
#include "sl_thread.h"

#include <solo6010/enc.h>

#define G723_INTR_ORDER		0
#define G723_FDMA_PAGES		32

#define G723_MAX_ITEM		120

#define OUTMODE_MASK		0x300
#define G723_DEC_CHANNEL	17

#define SAMPLERATE			8000
#define BITRATE				25

struct G723_ITEM
{
	struct list_head lh;

	u8 data[960];
};

struct SOLO6010_G723
{
	struct SOLO6010 *solo6010;
	solo6010_device_t *device;

	int ext_current;

	sl_thread_t thread;
	wait_queue_head_t thread_wq;

	char cache_name[64];
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20))
	struct kmem_cache *cache;
#else
	kmem_cache_t *cache;
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
	mempool_t *pool;
#endif

	int rd_pos;
	int nr_rd_item;
	spinlock_t rd_lock;
	struct list_head rd_lh;
	wait_queue_head_t rd_wq;

	int wr_pos;
	int nr_wr_item;
	spinlock_t wr_lock;
	struct list_head wr_lh;
	wait_queue_head_t wr_wq;

	int pb_clear;
};

static void solo6010_g723_config(struct SOLO6010 *solo6010)
{
	int clk_div;

	clk_div = solo6010->system_clock / (SAMPLERATE*(BITRATE*2)*2);

	solo6010_reg_write(solo6010, REG_AUDIO_SAMPLE,
		AUDIO_BITRATE(BITRATE) |
		AUDIO_CLK_DIV(clk_div));

	solo6010_reg_write(solo6010, REG_AUDIO_FDMA_INTR,
		AUDIO_FDMA_INTERVAL(1) |
		AUDIO_INTR_ORDER(G723_INTR_ORDER) |
		AUDIO_FDMA_BASE((solo6010->g723_ext_addr>>16) & 0xffff));

	solo6010_reg_write(solo6010, REG_AUDIO_CONTROL,
		AUDIO_ENABLE |
		AUDIO_I2S_MODE |
		AUDIO_I2S_MULTI(3) |
		AUDIO_MODE(OUTMODE_MASK));
}

static int solo6010_g723_thread(void *data)
{
	sl_thread_t *sl_thread;
	struct SOLO6010_G723 *g723;
	struct SOLO6010 *solo6010;
	u8 *clear_buffer;
	int ret;

	sl_thread = (sl_thread_t *)data;
	g723 = (struct SOLO6010_G723 *)sl_thread_data(sl_thread);
	solo6010 = g723->solo6010;

	sl_thread_init(sl_thread, "g723_%d", solo6010->chip.id);

	clear_buffer = (u8 *)__get_free_pages(GFP_KERNEL, get_order(32<<10));
	memset(clear_buffer, 0, 32<<10);

	while(sl_thread_alived(sl_thread))
	{
		struct G723_ITEM *g723_item;
		unsigned long flags;

		ret = wait_event_interruptible(g723->thread_wq,
			(g723->rd_pos != g723->ext_current) ||
			g723->pb_clear ||
			((!list_empty(&g723->wr_lh)) && (g723->wr_pos != ((g723->ext_current+G723_FDMA_PAGES-1)%G723_FDMA_PAGES))));
		if(ret != 0)
			continue;

		if(g723->rd_pos != g723->ext_current)
		{
			while(g723->nr_rd_item >= G723_MAX_ITEM)
			{
				spin_lock_irqsave(&g723->rd_lock, flags);
				g723_item = list_entry(g723->rd_lh.prev, struct G723_ITEM, lh);
				list_del(&g723_item->lh);
				g723->nr_rd_item--;
				spin_unlock_irqrestore(&g723->rd_lock, flags);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
				mempool_free(g723_item, g723->pool);
#else
				kmem_cache_free(g723->cache, g723_item);
#endif
			}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
			g723_item = mempool_alloc(g723->pool, GFP_KERNEL);
#else
			g723_item = kmem_cache_alloc(g723->cache, SLAB_KERNEL);
#endif
			if(!g723_item)
				continue;

			solo6010_p2m_dma(solo6010, 0, 0, g723_item->data, solo6010->g723_ext_addr + (g723->rd_pos<<10), 960);

			spin_lock_irqsave(&g723->rd_lock, flags);
			list_add(&g723_item->lh, &g723->rd_lh);
			g723->nr_rd_item++;
			spin_unlock_irqrestore(&g723->rd_lock, flags);

			g723->rd_pos = (g723->rd_pos + 1) % G723_FDMA_PAGES;
			wake_up(&g723->rd_wq);
		}

		if(g723->pb_clear)
		{
			solo6010_p2m_dma(solo6010, 0, 1, clear_buffer, solo6010->g723_ext_addr + (32<<10), 32<<10);
			g723->pb_clear = 0;
		}

		if((!list_empty(&g723->wr_lh)) &&
			(g723->wr_pos != ((g723->ext_current+G723_FDMA_PAGES-1)%G723_FDMA_PAGES)))
		{
			spin_lock_irqsave(&g723->wr_lock, flags);
			if(list_empty(&g723->wr_lh))
			{
				spin_unlock_irqrestore(&g723->wr_lock, flags);
				continue;
			}

			g723_item = list_entry(g723->wr_lh.prev, struct G723_ITEM, lh);
			list_del(&g723_item->lh);
			g723->nr_wr_item--;
			spin_unlock_irqrestore(&g723->wr_lock, flags);

			solo6010_p2m_dma(solo6010, 0, 1, g723_item->data, solo6010->g723_ext_addr + (32<<10) + (g723->wr_pos<<10), 960);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
			mempool_free(g723_item, g723->pool);
#else
			kmem_cache_free(g723->cache, g723_item);
#endif

			g723->wr_pos = (g723->wr_pos + 1) % G723_FDMA_PAGES;
			wake_up(&g723->wr_wq);
		}
	}

	free_pages((unsigned long)clear_buffer, get_order(32<<10));

	sl_thread_exit(sl_thread, 0);
	return 0;
}

int solo6010_g723_interrupt(struct SOLO6010 *solo6010)
{
	struct SOLO6010_G723 *g723;

	g723 = (struct SOLO6010_G723 *)solo6010->g723;

	solo6010_reg_write(solo6010, REG_INTR_STAT, INTR_G723);

	g723->ext_current = solo6010_reg_read(solo6010, REG_AUDIO_STA) & 0x1f;
	if(g723->wr_pos == ((g723->ext_current + 1) % G723_FDMA_PAGES))
	{
		g723->wr_pos = (g723->wr_pos + 1) % G723_FDMA_PAGES;
		g723->pb_clear = 1;
	}

	wake_up(&g723->thread_wq);

	return 0;
}

static loff_t fops_llseek(struct file *filp, loff_t off, int whence)
{
	return -EINVAL;
}

static unsigned int fops_poll(struct file *filp, poll_table *wait)
{
	struct SOLO6010_G723 *g723;
	unsigned int mask = 0;

	g723 = filp->private_data;

	poll_wait(filp, &g723->rd_wq, wait);
	poll_wait(filp, &g723->wr_wq, wait);

	if(!list_empty(&g723->rd_lh))
		mask |= POLLIN | POLLRDNORM | POLLRDBAND;

	if(g723->nr_wr_item < G723_MAX_ITEM)
		mask |= POLLOUT | POLLWRNORM | POLLWRBAND;

	return mask;
}

static ssize_t fops_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	struct SOLO6010_G723 *g723;
	struct SOLO6010 *solo6010;
	struct G723_ITEM *item;

	int ret;
	unsigned long flags;

	g723 = filp->private_data;
	solo6010 = g723->solo6010;

check_item:
	spin_lock_irqsave(&g723->rd_lock, flags);
	if(list_empty(&g723->rd_lh))
	{
		spin_unlock_irqrestore(&g723->rd_lock, flags);

		if(filp->f_flags & O_NONBLOCK)
			return -EAGAIN;

		ret = wait_event_interruptible(g723->rd_wq, !list_empty(&g723->rd_lh));
		if(ret)
			return ret;

		goto check_item;
	}

	item = list_entry(g723->rd_lh.prev, struct G723_ITEM, lh);
	list_del(&item->lh);
	g723->nr_rd_item--;
	spin_unlock_irqrestore(&g723->rd_lock, flags);

	if(copy_to_user(buf, item->data, 48*16))
		ret = -EFAULT;
	else
		ret = 48*16;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
	mempool_free(item, g723->pool);
#else
	kmem_cache_free(g723->cache, item);
#endif

	return ret;
}

static ssize_t fops_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	struct SOLO6010_G723 *g723;
	struct SOLO6010 *solo6010;
	struct G723_ITEM *item;

	int ret;
	unsigned long flags;

	g723 = filp->private_data;
	solo6010 = g723->solo6010;

	if(count != 48)
		return -EINVAL;

check_full:
	if(g723->nr_wr_item >= G723_MAX_ITEM)
	{
		if(filp->f_flags & O_NONBLOCK)
			return -EAGAIN;

		ret = wait_event_interruptible(g723->wr_wq, g723->nr_wr_item < G723_MAX_ITEM);
		if(ret)
			return ret;

		goto check_full;
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
	item = mempool_alloc(g723->pool, GFP_KERNEL);
#else
	item = kmem_cache_alloc(g723->cache, SLAB_KERNEL);
#endif
	if(!item)
		return -ENOMEM;


	if(copy_from_user(item->data + (G723_DEC_CHANNEL * 48), buf, count))
		ret = -EFAULT;
	else
		ret = count;

	spin_lock_irqsave(&g723->wr_lock, flags);
	list_add(&item->lh, &g723->wr_lh);
	g723->nr_wr_item++;
	spin_unlock_irqrestore(&g723->wr_lock, flags);

	return ret;
}

static int fops_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	return -ENOTTY;
}

static int fops_open(struct inode *inode, struct file *filp)
{
	struct SOLO6010_G723 *g723;

	g723 = (struct SOLO6010_G723 *)solo6010_device_data_get(inode_to_solo6010_device(inode));
	if(!g723)
		return -ENODEV;

	filp->private_data = g723;

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

int solo6010_g723_init(struct SOLO6010 *solo6010)
{
	struct SOLO6010_G723 *g723;

	g723 = (struct SOLO6010_G723 *)kmalloc(sizeof(struct SOLO6010_G723), GFP_KERNEL);
	if(g723 == NULL)
		return -ENOMEM;

	memset(g723, 0, sizeof(struct SOLO6010_G723));

	g723->solo6010 = solo6010;

	solo6010_g723_config(solo6010);

	sprintf(g723->cache_name, "solo6010_g723_%d", solo6010->chip.id);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 23))
	g723->cache = kmem_cache_create(g723->cache_name, sizeof(struct G723_ITEM), 0, SLAB_HWCACHE_ALIGN, NULL);
#else
	g723->cache = kmem_cache_create(g723->cache_name, sizeof(struct G723_ITEM), 0, SLAB_HWCACHE_ALIGN, NULL, NULL);
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
	g723->pool = mempool_create(G723_MAX_ITEM/4, mempool_alloc_slab, mempool_free_slab, g723->cache);
#endif
	spin_lock_init(&g723->rd_lock);
	INIT_LIST_HEAD(&g723->rd_lh);
	init_waitqueue_head(&g723->rd_wq);

	spin_lock_init(&g723->wr_lock);
	INIT_LIST_HEAD(&g723->wr_lh);
	init_waitqueue_head(&g723->wr_wq);

	init_waitqueue_head(&g723->thread_wq);
	sl_thread_create(&g723->thread, solo6010_g723_thread, g723);

	g723->device = solo6010_device_add(&solo6010->chip, &fops, "g723_", SOLO6010_G723_DEV_OFFSET, (void *)g723);
	if(IS_ERR(g723->device))
	{
		int ret_val = PTR_ERR(g723->device);
		kfree(g723);
		return ret_val;
	}

	solo6010->g723 = (void *)g723;

	solo6010_enable_interrupt(solo6010, INTR_G723);

	return 0;
}

void solo6010_g723_exit(struct SOLO6010 *solo6010)
{
	struct SOLO6010_G723 *g723;

	g723 = (struct SOLO6010_G723 *)solo6010->g723;

	solo6010_device_remove(g723->device);

	solo6010_disable_interrupt(solo6010, INTR_G723);

	sl_thread_kill(&g723->thread);

	while(!list_empty(&g723->rd_lh))
	{
		struct G723_ITEM *g723_item;
		g723_item = list_entry(g723->rd_lh.prev, struct G723_ITEM, lh);
		list_del(&g723_item->lh);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
		mempool_free(g723_item, g723->pool);
#else
		kmem_cache_free(g723->cache, g723_item);
#endif
	}

	while(!list_empty(&g723->wr_lh))
	{
		struct G723_ITEM *g723_item;
		g723_item = list_entry(g723->wr_lh.prev, struct G723_ITEM, lh);
		list_del(&g723_item->lh);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
		mempool_free(g723_item, g723->pool);
#else
		kmem_cache_free(g723->cache, g723_item);
#endif
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
	mempool_destroy(g723->pool);
#endif
	kmem_cache_destroy(g723->cache);

	solo6010_reg_write(solo6010, REG_AUDIO_CONTROL, 0);

	kfree(g723);
}

