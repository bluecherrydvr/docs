#ifndef __P2M_H
#define __P2M_H

void solo6x10_p2m_interrupt(struct SOLO6x10 *solo6x10, int id);
void solo6x10_p2m_error_interrupt(struct SOLO6x10 *solo6x10);
int solo6x10_p2m_dma(struct SOLO6x10 *solo6x10, int id, int wr, void *sys_addr, u32 ext_addr, u32 size);
int solo6x10_dma_user(struct SOLO6x10 *solo6x10, int id, int wr, const char __user *buf, u32 ext_addr, u32 size);
int solo6x10_osg_dma_page(struct SOLO6x10 *solo6x10, int id, int alpha, int byte_per_pixel, struct page *page, int offset, u32 ext_addr, u32 width);
int solo6x10_p2m_init(struct SOLO6x10 *solo6x10);
void solo6x10_p2m_exit(struct SOLO6x10 *solo6x10);

#endif

