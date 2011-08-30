#ifndef __P2M_H
#define __P2M_H

void solo6010_p2m_interrupt(struct SOLO6010 *solo6010, int id);
void solo6010_p2m_error_interrupt(struct SOLO6010 *solo6010, u32 status);
int solo6010_p2m_dma(struct SOLO6010 *solo6010, int id, int wr, void *sys_addr, u32 ext_addr, u32 size);
int solo6010_dma_user(struct SOLO6010 *solo6010, int id, int wr, const char __user *buf, u32 ext_addr, u32 size);
int solo6010_osg_dma_page(struct SOLO6010 *solo6010, int id, int alpha, int byte_per_pixel, struct page *page, int offset, u32 ext_addr, u32 width);
int solo6010_p2m_init(struct SOLO6010 *solo6010);
void solo6010_p2m_exit(struct SOLO6010 *solo6010);

#endif

