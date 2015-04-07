#ifndef __DMA_H__
#define __DMA_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define DISRC_OFFSET		(0)
#define	DISRCC_OFFSET		(4)
#define	DIDST_OFFSET		(8)
#define	DIDSTC_OFFSET		(0xc)
#define	DCON_OFFSET			(0x10)
#define	DSTAT_OFFSET		(0x14)
#define	DCSRC_OFFSET		(0x18)
#define	DCDST_OFFSET		(0x1c)
#define	DMASKTRIG_OFFSET	(0x20)

#define VLC_ENCODER_PING			(0x2000)
#define	VLC_ENCODER_DMA_LEN			(4096)
#define	VLC_ENCODER_PONG			(0x3000)
#define FPGA_LEN_OFFSET_MASK		(0x7ff)

    typedef struct tagDMA
    {
	volatile u32	DISRC;		/* dma initial source reg */
	volatile u32	DISRCC;		/* dma initial source control reg */
	volatile u32	DIDST;		/* dma initial dest reg */
	volatile u32	DIDSTC;		/* dma initial dest control reg */
	volatile u32	DCON;		/* dma control reg */
	volatile u32	DSTAT;		/* dma status reg */
	volatile u32	DCSRC;		/* dma curr source reg */
	volatile u32	DCDST;		/* dma curr dest reg */
	volatile u32	DMASKTRIG;	/* dma mask trigger reg */
    }DMA_REG;

    typedef struct DMAparam
    {
	u32		srcAddr;
	s32		ifSrcAddrChange;
	u32		destAddr;
	s32		ifDestAddrChange;
	int		count;
	int		dsz;
    }DMA_PARAM;

    extern void init_isil_dma_chan(void);
    extern void	fpga_dma_receive(u32 src, u32 dest, u32 len);

#ifdef __cplusplus
}
#endif

#endif /*__DMA_H__*/

