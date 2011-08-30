#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/highmem.h>
#include <linux/pagemap.h>

#include "solo6010.h"

#define P2M_NR_DESC			256
#define P2M_DESC_SIZE		(P2M_NR_DESC*16)

#define NR_P2M_DEV			4

struct P2M_DEV
{
	unsigned int id;
	struct SOLO6010 *solo6010;

	struct semaphore mutex;
	struct completion completion;
	int error;
	u32 *desc;

	struct page **pages;
	int nr_page;
};

static int init_p2m_dev(struct P2M_DEV *p2m_dev)
{
	init_MUTEX(&p2m_dev->mutex);
	init_completion(&p2m_dev->completion);
	p2m_dev->desc = kmalloc(P2M_DESC_SIZE, GFP_KERNEL);
	if(p2m_dev->desc == NULL)
		return -ENOMEM;

	memset(p2m_dev->desc, 0, P2M_DESC_SIZE);

	p2m_dev->pages = kmalloc(16<<10, GFP_KERNEL);
	if(p2m_dev->pages == NULL)
	{
		kfree(p2m_dev->desc);
		return -ENOMEM;
	}

	return 0;
}

static void clear_p2m_dev(struct P2M_DEV *p2m_dev)
{
	kfree(p2m_dev->pages);
	kfree(p2m_dev->desc);
}

void solo6010_p2m_interrupt(struct SOLO6010 *solo6010, int id)
{
	struct P2M_DEV *p2m_dev;

	p2m_dev = (struct P2M_DEV *)solo6010->p2m_dev;
	p2m_dev += id;

	solo6010_reg_write(solo6010, REG_INTR_STAT, INTR_P2M(id));
	complete(&p2m_dev->completion);
}

void solo6010_p2m_error_interrupt(struct SOLO6010 *solo6010, u32 status)
{
	struct P2M_DEV *p2m_dev;

	if(status & PCI_ERROR_P2M)
	{
		int i;

		for(i=0, p2m_dev=(struct P2M_DEV *)solo6010->p2m_dev; i<4; i++, p2m_dev++)
		{
			p2m_dev->error = 1;
			solo6010_reg_write(solo6010, REG_P2M_CONTROL(i), 0);
			complete(&p2m_dev->completion);
		}
	}
}

int solo6010_p2m_dma(struct SOLO6010 *solo6010, int id, int wr, void *sys_addr, u32 ext_addr, u32 size)
{
	struct P2M_DEV *p2m_dev;
	unsigned int timeout = 0;
	dma_addr_t dma_addr;

	p2m_dev = (struct P2M_DEV *)solo6010->p2m_dev;
	p2m_dev += id;

	down(&p2m_dev->mutex);

	dma_addr = pci_map_single(solo6010->pdev, sys_addr, size, wr ? PCI_DMA_TODEVICE : PCI_DMA_FROMDEVICE);

start_dma:
	INIT_COMPLETION(p2m_dev->completion);
	p2m_dev->error = 0;
	solo6010_reg_write(solo6010, REG_P2M_TAR_ADR(id), dma_addr);
	solo6010_reg_write(solo6010, REG_P2M_EXT_ADR(id), ext_addr);
	solo6010_reg_write(solo6010, REG_P2M_EXT_CFG(id), P2M_COPY_SIZE(size>>2));
	solo6010_reg_write(solo6010, REG_P2M_CONTROL(id), P2M_BURST_SIZE(P2M_BURST_256) | (wr ? P2M_WRITE : 0) | P2M_TRANS_ON);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 11))
	wait_for_completion(&p2m_dev->completion);
#else
	timeout = wait_for_completion_timeout(&p2m_dev->completion, HZ);
	if(timeout == 0)
	{
		unsigned int p2m_stat;
		p2m_stat = solo6010_reg_read(solo6010, REG_P2M_STATUS(id));
		dbg_msg("p2m timeout(p2m_stat : %08x)\n", p2m_stat);
	}
#endif

	solo6010_reg_write(solo6010, REG_P2M_CONTROL(id), 0);
	if(p2m_dev->error)
	{
		p2m_dev->error = 0;
		goto start_dma;
	}

	pci_unmap_single(solo6010->pdev, dma_addr, size, wr ? PCI_DMA_TODEVICE : PCI_DMA_FROMDEVICE);

	up(&p2m_dev->mutex);

	return (timeout == 0) ? -1 : 0;
}

int solo6010_dma_user(struct SOLO6010 *solo6010, int id, int wr, const char __user *buf, u32 ext_addr, u32 size)
{
	struct P2M_DEV *p2m_dev;
	unsigned int timeout = 0;
	dma_addr_t dma_addr;
	u32 cp_size;

	int ret;

	unsigned long user_base;
	unsigned long offset;
	int current_page = 0;

	p2m_dev = (struct P2M_DEV *)solo6010->p2m_dev;
	p2m_dev += id;

	down(&p2m_dev->mutex);

	user_base = (unsigned long)buf & PAGE_MASK;
	offset = (unsigned long)buf - user_base;
	p2m_dev->nr_page = PAGE_ALIGN(offset + size) >> PAGE_SHIFT;

	down_read(&current->mm->mmap_sem);
	ret = get_user_pages(current, current->mm, user_base, p2m_dev->nr_page, !wr, 0, p2m_dev->pages, NULL);
	up_read(&current->mm->mmap_sem);
	if(ret < p2m_dev->nr_page)
	{
		dbg_msg("get_user_pages error (%d, %d)\n", p2m_dev->nr_page, ret);

		while(ret > 0)
		{
			ret--;
			page_cache_release(p2m_dev->pages[ret]);
		}

		return ret ? ret : -ENOMEM;
	}

	while(size)
	{
		cp_size = (size > (PAGE_SIZE-offset)) ? (PAGE_SIZE-offset) : size;
		dma_addr = pci_map_page(solo6010->pdev, p2m_dev->pages[current_page], offset, cp_size, wr ? PCI_DMA_TODEVICE : PCI_DMA_FROMDEVICE);

start_dma:
		INIT_COMPLETION(p2m_dev->completion);
		solo6010_reg_write(solo6010, REG_P2M_TAR_ADR(id), dma_addr);
		solo6010_reg_write(solo6010, REG_P2M_EXT_ADR(id), ext_addr);
		solo6010_reg_write(solo6010, REG_P2M_EXT_CFG(id), P2M_COPY_SIZE(cp_size>>2));
		solo6010_reg_write(solo6010, REG_P2M_CONTROL(id), P2M_BURST_SIZE(P2M_BURST_256) | (wr ? P2M_WRITE : 0) | P2M_TRANS_ON);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 11))
		wait_for_completion(&p2m_dev->completion);
#else
		timeout = wait_for_completion_timeout(&p2m_dev->completion, HZ);
		if(timeout == 0)
		{
			unsigned int p2m_stat;
			p2m_stat = solo6010_reg_read(solo6010, REG_P2M_STATUS(id));
			dbg_msg("p2m timeout(p2m_stat : %08x)\n", p2m_stat);
		}
#endif

		solo6010_reg_write(solo6010, REG_P2M_CONTROL(id), 0);
		if(p2m_dev->error)
		{
			p2m_dev->error = 0;
			goto start_dma;
		}

		pci_unmap_page(solo6010->pdev, dma_addr, cp_size, wr ? PCI_DMA_TODEVICE : PCI_DMA_FROMDEVICE);

		offset = 0;
		current_page++;
		size -= cp_size;
		ext_addr += cp_size;
	}

	while(p2m_dev->nr_page)
	{
		p2m_dev->nr_page--;
		if((!wr) && (!PageReserved(p2m_dev->pages[p2m_dev->nr_page])))
			SetPageDirty(p2m_dev->pages[p2m_dev->nr_page]);
		page_cache_release(p2m_dev->pages[p2m_dev->nr_page]);
	}

	up(&p2m_dev->mutex);

	return (timeout == 0) ? -1 : 0;
}

int solo6010_osg_dma_page(struct SOLO6010 *solo6010, int id, int alpha, int byte_per_pixel, struct page *page, int offset, u32 ext_addr, u32 width)
{
	struct P2M_DEV *p2m_dev;
	dma_addr_t dma_addr;

	if((offset + (width*byte_per_pixel)) >= PAGE_SIZE)
		return -1;

	p2m_dev = (struct P2M_DEV *)solo6010->p2m_dev;
	p2m_dev += id;

	down(&p2m_dev->mutex);

	dma_addr = pci_map_page(solo6010->pdev, page, offset, width*byte_per_pixel, PCI_DMA_TODEVICE);

start_dma:
	INIT_COMPLETION(p2m_dev->completion);
	solo6010_reg_write(solo6010, REG_P2M_TAR_ADR(id), dma_addr);
	solo6010_reg_write(solo6010, REG_P2M_EXT_ADR(id), ext_addr);
	solo6010_reg_write(solo6010, REG_P2M_EXT_CFG(id), P2M_COPY_SIZE((width<<1)>>2));
	solo6010_reg_write(solo6010, REG_P2M_CONTROL(id),
		P2M_BURST_SIZE(P2M_BURST_256) | P2M_WRITE | P2M_TRANS_ON |
		((byte_per_pixel==2) ? P2M_CSC_16BIT : 0) | P2M_ALPHA_MODE(alpha) | P2M_CSC_ON);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 11))
	wait_for_completion(&p2m_dev->completion);
#else
{
	unsigned int timeout = 0;
	timeout = wait_for_completion_timeout(&p2m_dev->completion, HZ);
	if(timeout == 0)
	{
		unsigned int p2m_stat;
		p2m_stat = solo6010_reg_read(solo6010, REG_P2M_STATUS(id));
		dbg_msg("p2m timeout(p2m_stat : %08x)\n", p2m_stat);
	}
}
#endif

	solo6010_reg_write(solo6010, REG_P2M_CONTROL(id), 0);
	if(p2m_dev->error)
	{
		p2m_dev->error = 0;
		goto start_dma;
	}

	pci_unmap_page(solo6010->pdev, dma_addr, width*byte_per_pixel, PCI_DMA_TODEVICE);

	up(&p2m_dev->mutex);

	return width*byte_per_pixel;
}

int p2m_test(struct SOLO6010 *solo6010, u32 base, int size)
{
	u32 *wr_buf;
	u32 *rd_buf;
	int i;
	int err_cnt=0;
	int order = get_order(size);

	if((wr_buf = (u32 *)__get_free_pages(GFP_KERNEL, order)) == NULL)
		return -1;

	if((rd_buf = (u32 *)__get_free_pages(GFP_KERNEL, order)) == NULL)
	{
		free_pages((unsigned long)wr_buf, order);
		return -1;
	}

	for(i=0; i<((size/2)>>2); i++)
	{
		*(wr_buf + i) = (i<<16) | (i+1);
	}

	for(i=((size/2)>>2); i<(size>>2); i++)
	{
		*(wr_buf + i) = ~((i<<16) | (i+1));
	}

	memset(rd_buf, 0x55, size);

	solo6010_p2m_dma(solo6010, 0, 1, wr_buf, base, size);
	solo6010_p2m_dma(solo6010, 0, 0, rd_buf, base, size);

	for(i=0; i<(size>>2); i++)
	{
		if(*(wr_buf + i) != *(rd_buf + i))
		{
			err_cnt++;

			if(err_cnt < 5)
				dbg_msg(" %08x:%08x\n", *(wr_buf + i), *(rd_buf + i));
		}
	}

	if(err_cnt)
		dbg_msg(" p2m test error!\n");

	free_pages((unsigned long)wr_buf, order);
	free_pages((unsigned long)rd_buf, order);

	return err_cnt;
}

int solo6010_p2m_init(struct SOLO6010 *solo6010)
{
	struct P2M_DEV *p2m_dev;
	int i;

	p2m_dev = kmalloc(sizeof(struct P2M_DEV) * NR_P2M_DEV, GFP_KERNEL);
	if(p2m_dev == NULL)
		return -ENOMEM;

	memset(p2m_dev, 0, sizeof(struct P2M_DEV) * NR_P2M_DEV);

	for(i=0; i<NR_P2M_DEV; i++)
	{
		p2m_dev[i].id = i;
		p2m_dev[i].solo6010 = solo6010;
		if(init_p2m_dev(&p2m_dev[i]) != 0)
		{
			do
			{
				clear_p2m_dev(&p2m_dev[i]);
			}while(i--);

			kfree(p2m_dev);

			return -ENOMEM;
		}

		solo6010_reg_write(solo6010, REG_P2M_DES_ADR(i), virt_to_bus(p2m_dev[i].desc));
		solo6010_reg_write(solo6010, REG_P2M_CONTROL(i), 0);
		solo6010_reg_write(solo6010, REG_P2M_CONFIG(i), P2M_CSC_16BIT_565 | P2M_DMA_INTERVAL(0) | P2M_PCI_MASTER_MODE);
		solo6010_enable_interrupt(solo6010, INTR_P2M(i));
	}

	solo6010->p2m_dev = (void *)p2m_dev;


	return 0;
}

void solo6010_p2m_exit(struct SOLO6010 *solo6010)
{
	struct P2M_DEV *p2m_dev;
	int i;

	if(solo6010->p2m_dev == NULL)
		return;

	p2m_dev = (struct P2M_DEV *)solo6010->p2m_dev;

	for(i=0; i<NR_P2M_DEV; i++)
	{
		solo6010_disable_interrupt(solo6010, INTR_P2M(i));
		clear_p2m_dev(&p2m_dev[i]);
	}

	kfree(p2m_dev);
}

