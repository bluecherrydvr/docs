#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/highmem.h>
#include <linux/pagemap.h>

#include "solo6x10.h"

#define P2M_NR_DESC			256
#define P2M_DESC_SIZE		(P2M_NR_DESC*16)

#define NR_P2M_DEV			4

struct P2M_DMA_DESC
{
	u32 tar_adr;
	u32 ext_adr;
	u32 ext_cfg;
	u32 control;
};

struct P2M_DEV
{
	unsigned int id;
	struct SOLO6x10 *solo6x10;

	struct semaphore mutex;
	struct completion completion;
	int error;

	struct P2M_DMA_DESC *dma_desc;
	u32 nr_dma_desc;
	u32 current_dma_desc;
};

void solo6x10_p2m_interrupt(struct SOLO6x10 *solo6x10, int id)
{
	struct P2M_DEV *p2m_dev;

	p2m_dev = (struct P2M_DEV *)solo6x10->p2m_dev;
	p2m_dev += id;

	solo6x10_reg_write(solo6x10, REG_INTR_STAT, INTR_P2M(id));

	if(!p2m_dev->nr_dma_desc)
	{
		complete(&p2m_dev->completion);
		return;
	}

	p2m_dev->current_dma_desc++;
	if(p2m_dev->current_dma_desc == p2m_dev->nr_dma_desc)
	{
		complete(&p2m_dev->completion);
		return;
	}

	solo6x10_reg_write(solo6x10, REG_P2M_CONTROL(id), 0);
	solo6x10_reg_write(solo6x10, REG_P2M_TAR_ADR(id), p2m_dev->dma_desc[p2m_dev->current_dma_desc].tar_adr);
	solo6x10_reg_write(solo6x10, REG_P2M_EXT_ADR(id), p2m_dev->dma_desc[p2m_dev->current_dma_desc].ext_adr);
	solo6x10_reg_write(solo6x10, REG_P2M_EXT_CFG(id), p2m_dev->dma_desc[p2m_dev->current_dma_desc].ext_cfg);
	solo6x10_reg_write(solo6x10, REG_P2M_CONTROL(id), p2m_dev->dma_desc[p2m_dev->current_dma_desc].control);
}

void solo6x10_p2m_error_interrupt(struct SOLO6x10 *solo6x10)
{
	struct P2M_DEV *p2m_dev;
	u32 error_status;
	int i;

	error_status = solo6x10_reg_read(solo6x10, REG_PCI_ERROR);
	if(!(error_status & PCI_ERROR_P2M))
	{
		solo6x10_reg_write(solo6x10, REG_INTR_STAT, INTR_PCI_ERR);
		dbg_msg("solo6x10 pci error! (0x%08x)\n", error_status);
		return;
	}

	for(i=0, p2m_dev=(struct P2M_DEV *)solo6x10->p2m_dev; i<4; i++, p2m_dev++)
	{
		solo6x10_reg_write(solo6x10, REG_P2M_CONTROL(i), 0);
		if(!p2m_dev->nr_dma_desc)
		{
			p2m_dev->error = 1;
			complete(&p2m_dev->completion);
		}
	}

	solo6x10_reg_write(solo6x10, REG_INTR_STAT, INTR_PCI_ERR);

	for(i=0, p2m_dev=(struct P2M_DEV *)solo6x10->p2m_dev; i<4; i++, p2m_dev++)
	{
		if(p2m_dev->nr_dma_desc)
		{
			solo6x10_reg_write(solo6x10, REG_P2M_TAR_ADR(i), p2m_dev->dma_desc[p2m_dev->current_dma_desc].tar_adr);
			solo6x10_reg_write(solo6x10, REG_P2M_EXT_ADR(i), p2m_dev->dma_desc[p2m_dev->current_dma_desc].ext_adr);
			solo6x10_reg_write(solo6x10, REG_P2M_EXT_CFG(i), p2m_dev->dma_desc[p2m_dev->current_dma_desc].ext_cfg);
			solo6x10_reg_write(solo6x10, REG_P2M_CONTROL(i), p2m_dev->dma_desc[p2m_dev->current_dma_desc].control);
		}
	}

	dbg_msg("solo6x10 pci error! (0x%08x)\n", error_status);
}

int solo6x10_p2m_dma(struct SOLO6x10 *solo6x10, int id, int wr, void *sys_addr, u32 ext_addr, u32 size)
{
	struct P2M_DEV *p2m_dev;
	unsigned int timeout = 0;
	dma_addr_t dma_addr;

	id = 0;	// use id 0 only

	p2m_dev = (struct P2M_DEV *)solo6x10->p2m_dev;
	p2m_dev += id;

	dma_addr = pci_map_single(solo6x10->pdev, sys_addr, size, wr ? PCI_DMA_TODEVICE : PCI_DMA_FROMDEVICE);

	down(&p2m_dev->mutex);

start_dma:
	INIT_COMPLETION(p2m_dev->completion);
	p2m_dev->error = 0;
	solo6x10_reg_write(solo6x10, REG_P2M_TAR_ADR(id), dma_addr);
	solo6x10_reg_write(solo6x10, REG_P2M_EXT_ADR(id), ext_addr);
	solo6x10_reg_write(solo6x10, REG_P2M_EXT_CFG(id), P2M_COPY_SIZE(size>>2));
	solo6x10_reg_write(solo6x10, REG_P2M_CONTROL(id), P2M_BURST_SIZE(P2M_BURST_256) | (wr ? P2M_WRITE : 0) | P2M_TRANS_ON);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 11))
	wait_for_completion(&p2m_dev->completion);
#else
	timeout = wait_for_completion_timeout(&p2m_dev->completion, HZ);
	if(timeout == 0)
	{
		unsigned int p2m_stat;
		p2m_stat = solo6x10_reg_read(solo6x10, REG_P2M_STATUS(id));
		dbg_msg("p2m timeout(p2m_stat : %08x)\n", p2m_stat);
	}
#endif

	solo6x10_reg_write(solo6x10, REG_P2M_CONTROL(id), 0);
	if(p2m_dev->error)
	{
		p2m_dev->error = 0;
		goto start_dma;
	}

	up(&p2m_dev->mutex);

	pci_unmap_single(solo6x10->pdev, dma_addr, size, wr ? PCI_DMA_TODEVICE : PCI_DMA_FROMDEVICE);

	return (timeout == 0) ? -1 : 0;
}

int solo6x10_dma_user(struct SOLO6x10 *solo6x10, int id, int wr, const char __user *buf, u32 ext_addr, u32 size)
{
	struct P2M_DEV *p2m_dev;
	unsigned int timeout = 0;
	dma_addr_t dma_addr;
	u32 cp_size;

	struct page **pages;
	int nr_page;
	struct P2M_DMA_DESC *p2m_dma_desc;

	int ret;

	unsigned long user_base;
	unsigned long offset;
	int current_page = 0;

	id = 0;	// use id 0 only

	p2m_dev = (struct P2M_DEV *)solo6x10->p2m_dev;
	p2m_dev += id;

	user_base = (unsigned long)buf & PAGE_MASK;
	offset = (unsigned long)buf - user_base;
	nr_page = PAGE_ALIGN(offset + size) >> PAGE_SHIFT;
	pages = kmalloc(nr_page * (sizeof(struct page *) + sizeof(struct P2M_DMA_DESC)), GFP_KERNEL);
	p2m_dma_desc = (struct P2M_DMA_DESC *)&pages[nr_page];
	if(!pages)
		return -ENOMEM;

	down_read(&current->mm->mmap_sem);
	ret = get_user_pages(current, current->mm, user_base, nr_page, !wr, 0, pages, NULL);
	up_read(&current->mm->mmap_sem);
	if(ret < nr_page)
	{
		dbg_msg("get_user_pages error (%d, %d)\n", nr_page, ret);

		while(ret > 0)
		{
			ret--;
			page_cache_release(pages[ret]);
		}
		kfree(pages);
		return ret ? ret : -ENOMEM;
	}

	while(size)
	{
		cp_size = (size > (PAGE_SIZE-offset)) ? (PAGE_SIZE-offset) : size;
		dma_addr = pci_map_page(solo6x10->pdev, pages[current_page], offset, cp_size, wr ? PCI_DMA_TODEVICE : PCI_DMA_FROMDEVICE);

		p2m_dma_desc[current_page].tar_adr = dma_addr;
		p2m_dma_desc[current_page].ext_adr = ext_addr;
		p2m_dma_desc[current_page].ext_cfg = P2M_COPY_SIZE(cp_size>>2);
		p2m_dma_desc[current_page].control = P2M_BURST_SIZE(P2M_BURST_256) | (wr ? P2M_WRITE : 0) | P2M_TRANS_ON;

		offset = 0;
		current_page++;
		size -= cp_size;
		ext_addr += cp_size;
	}

	down(&p2m_dev->mutex);

	p2m_dev->dma_desc = p2m_dma_desc;
	p2m_dev->nr_dma_desc = nr_page;
	p2m_dev->current_dma_desc = 0;

	INIT_COMPLETION(p2m_dev->completion);
	solo6x10_reg_write(solo6x10, REG_P2M_TAR_ADR(id), p2m_dma_desc[0].tar_adr);
	solo6x10_reg_write(solo6x10, REG_P2M_EXT_ADR(id), p2m_dma_desc[0].ext_adr);
	solo6x10_reg_write(solo6x10, REG_P2M_EXT_CFG(id), p2m_dma_desc[0].ext_cfg);
	solo6x10_reg_write(solo6x10, REG_P2M_CONTROL(id), p2m_dma_desc[0].control);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 11))
	wait_for_completion(&p2m_dev->completion);
#else
	timeout = wait_for_completion_timeout(&p2m_dev->completion, HZ);
	if(timeout == 0)
	{
		unsigned int p2m_stat;
		p2m_stat = solo6x10_reg_read(solo6x10, REG_P2M_STATUS(id));
		dbg_msg("p2m timeout(p2m_stat : %08x)\n", p2m_stat);
		dbg_msg("cur:%d\n", p2m_dev->current_dma_desc);
	}
#endif

	solo6x10_reg_write(solo6x10, REG_P2M_CONTROL(id), 0);
	p2m_dev->nr_dma_desc = 0;

	up(&p2m_dev->mutex);

	while(nr_page)
	{
		nr_page--;
		pci_unmap_page(solo6x10->pdev, p2m_dma_desc[nr_page].tar_adr, cp_size, wr ? PCI_DMA_TODEVICE : PCI_DMA_FROMDEVICE);
		if((!wr) && (!PageReserved(pages[nr_page])))
			SetPageDirty(pages[nr_page]);
		page_cache_release(pages[nr_page]);
	}
	kfree(pages);

	return (timeout == 0) ? -1 : 0;
}

int solo6x10_osg_dma_page(struct SOLO6x10 *solo6x10, int id, int alpha, int byte_per_pixel, struct page *page, int offset, u32 ext_addr, u32 width)
{
	struct P2M_DEV *p2m_dev;
	dma_addr_t dma_addr;

	id = 0;	// use id 0 only

	if((offset + (width*byte_per_pixel)) >= PAGE_SIZE)
		return -1;

	p2m_dev = (struct P2M_DEV *)solo6x10->p2m_dev;
	p2m_dev += id;

	dma_addr = pci_map_page(solo6x10->pdev, page, offset, width*byte_per_pixel, PCI_DMA_TODEVICE);

	down(&p2m_dev->mutex);

start_dma:
	INIT_COMPLETION(p2m_dev->completion);
	solo6x10_reg_write(solo6x10, REG_P2M_TAR_ADR(id), dma_addr);
	solo6x10_reg_write(solo6x10, REG_P2M_EXT_ADR(id), ext_addr);
	solo6x10_reg_write(solo6x10, REG_P2M_EXT_CFG(id), P2M_COPY_SIZE((width<<1)>>2));
	solo6x10_reg_write(solo6x10, REG_P2M_CONTROL(id),
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
		p2m_stat = solo6x10_reg_read(solo6x10, REG_P2M_STATUS(id));
		dbg_msg("p2m timeout(p2m_stat : %08x)\n", p2m_stat);
	}
}
#endif

	solo6x10_reg_write(solo6x10, REG_P2M_CONTROL(id), 0);
	if(p2m_dev->error)
	{
		p2m_dev->error = 0;
		goto start_dma;
	}

	up(&p2m_dev->mutex);

	pci_unmap_page(solo6x10->pdev, dma_addr, width*byte_per_pixel, PCI_DMA_TODEVICE);

	return width*byte_per_pixel;
}

int p2m_test(struct SOLO6x10 *solo6x10, u32 base, int size)
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

	solo6x10_p2m_dma(solo6x10, 0, 1, wr_buf, base, size);
	solo6x10_p2m_dma(solo6x10, 0, 0, rd_buf, base, size);

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

int solo6x10_p2m_init(struct SOLO6x10 *solo6x10)
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
		p2m_dev[i].solo6x10 = solo6x10;
		init_MUTEX(&p2m_dev[i].mutex);
		init_completion(&p2m_dev[i].completion);
		solo6x10_reg_write(solo6x10, REG_P2M_CONTROL(i), 0);
		solo6x10_reg_write(solo6x10, REG_P2M_CONFIG(i), P2M_CSC_16BIT_565 | P2M_DMA_INTERVAL(0) | P2M_PCI_MASTER_MODE);
		solo6x10_enable_interrupt(solo6x10, INTR_P2M(i));
	}

	solo6x10->p2m_dev = (void *)p2m_dev;

	return 0;
}

void solo6x10_p2m_exit(struct SOLO6x10 *solo6x10)
{
	struct P2M_DEV *p2m_dev;
	int i;

	if(solo6x10->p2m_dev == NULL)
		return;

	p2m_dev = (struct P2M_DEV *)solo6x10->p2m_dev;

	for(i=0; i<NR_P2M_DEV; i++)
	{
		solo6x10_disable_interrupt(solo6x10, INTR_P2M(i));
	}

	kfree(p2m_dev);
}

