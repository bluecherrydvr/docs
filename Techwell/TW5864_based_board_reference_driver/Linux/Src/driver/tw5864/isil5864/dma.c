#include	<isil5864/isil_common.h>

/************************************************************************/
/************************************************************************/
#ifdef  NXP_PLATFORM

#endif

#ifdef POWERPC_PLATFORM
extern void	ppc_dma_m2m(u32 src, u32 dest, u32 len);
#endif

#ifdef X86_PLATFORM
#ifdef CONFIG_GENERIC_ISA_DMA
static unsigned int s_dmanr = -1;
int x86_dma_init(void)
{
    unsigned int dmanr;

    for(dmanr = 0; dmanr < MAX_DMA_CHANNELS; dmanr++){
        if(dmanr == 4){/*for cascade*/
            continue;
        }
        if(request_dma(dmanr, "ISIL") == 0){
            s_dmanr = dmanr;
            break;
        }
    }
    if(dmanr == 8){
        ISIL_DBG(ISIL_DBG_ERR, "dma request failed\n");
        return ISIL_ERR;
    }

    return ISIL_OK;
}

int x86_dma_release(void)
{
    if(s_dmanr >= 0){
        free_dma(s_dmanr);
        return ISIL_OK;
    }

    return ISIL_ERR;
}

static void DMA_M2M(u32 src, u32 dest, u32 len)
{
    u32 *vir_src, *vir_dst;

    vir_src = phys_to_virt(src);
    vir_dst = phys_to_virt(dest);

    //memcpy(vir_dst, vir_src, len);
}
#else
static void DMA_M2M(u32 src, u32 dest, u32 len) {}
//#error "NO GENERIC_ISA_DMA SUPPORT!!"
#endif
#endif

#ifdef HISILICON_PLATFORM
#include <asm/arch/pci.h>

#define MAX_DMA_CHANNEL         (8)
#define DMAC_ENBLD_CHNS         IO_ADDRESS(0x100d0000)
#define DMAC_INT_TC_STAT        IO_ADDRESS(0x100d0004)
#define DMAC_INT_TC_CLR         IO_ADDRESS(0x100d0008)
#define DMAC_INT_ERR_STAT       IO_ADDRESS(0x100d000c)
#define DMAC_INT_ERR_CLR        IO_ADDRESS(0x100d0010)

#define DMAC_CONFIG             IO_ADDRESS(0x100d0030)

#define DMAC_CX_SRC_ADDR(x)     IO_ADDRESS(0x100d0100 + ((x) << 5))
#define DMAC_CX_DEST_ADDR(x)    IO_ADDRESS(0x100d0104 + ((x) << 5))
#define DMAC_CX_LLI(x)          IO_ADDRESS(0x100d0108 + ((x) << 5))
#define DMAC_CX_CONTROL(x)      IO_ADDRESS(0x100d010c + ((x) << 5))
#define DMAC_CX_CONFIG(x)       IO_ADDRESS(0x100d0110 + ((x) << 5))

static __used void DMA_M2M(u32 src, u32 dest, u32 len)
{
    u32 flags;
    int dma_chnl = -1, ch;
    int timeout = 1000000;

    if((__raw_readl(DMAC_CONFIG) & 0x7) != 0x1) {
        printk("%s, %d: dma controler not enable or configure\n", __FUNCTION__, __LINE__);
        return;
    }

    flags = __raw_readl(DMAC_ENBLD_CHNS);
    for(ch = 0; ch < MAX_DMA_CHANNEL; ch++) {
        if(flags & (1 << ch)) {
            continue;
        }else{
            dma_chnl = ch;
            break;
        }
    }
    if(dma_chnl == -1) {
        printk("%s, %d: can't get valid dma channel\n", __FUNCTION__, __LINE__);
        return;
    }
    if(__raw_readl(DMAC_CX_CONFIG(dma_chnl)) & (1 << 17)) {
        printk("%s, %d: this dma channel is disabled but already active\n", __FUNCTION__, __LINE__);
        return;
    }
    /*clear irq flags*/
    __raw_writel((1 << dma_chnl), DMAC_INT_ERR_CLR);
    __raw_writel((1 << dma_chnl), DMAC_INT_TC_CLR);
    /*enable dma channel*/
    flags |= (1 << dma_chnl);
    __raw_writel(flags, DMAC_ENBLD_CHNS);
    /*set src*/
    __raw_writel(src, DMAC_CX_SRC_ADDR(dma_chnl));
    /*set dest*/
    __raw_writel(dest, DMAC_CX_DEST_ADDR(dma_chnl));
    /*configure single*/
    __raw_writel(0, DMAC_CX_LLI(dma_chnl));

    /*configure control*/
    __raw_writel((0x0f480 << 12) | (((len + 3)>>2) & ((1 << 12) - 1)), DMAC_CX_CONTROL(dma_chnl));

    flags = (1 << 16) | (0x2 << 11) | 
        __raw_writel((1 << 16), DMAC_CX_CONFIG(dma_chnl));
    /*start dma*/
    __raw_writel((1 << 16) | (1 << 0), DMAC_CX_CONFIG(dma_chnl));

    /*poll wait*/
    while(--timeout) {
        if(__raw_readl(DMAC_INT_ERR_STAT) & (1 << dma_chnl)) {
            printk("%s, %d: dma %d trans error\n", __FUNCTION__, __LINE__, dma_chnl);
            break;
        }
        if(__raw_readl(DMAC_INT_TC_STAT) & (1 << dma_chnl)) {
            /*complate*/
            break;
        }
    }
    if(!timeout) {
        printk("dma %d time out\n", dma_chnl);
    }
    /*clear flags*/
    __raw_writel((1 << dma_chnl), DMAC_INT_ERR_CLR);
    __raw_writel((1 << dma_chnl), DMAC_INT_TC_CLR);
    /*disable this dma channel*/
    flags = __raw_readl(DMAC_ENBLD_CHNS);
    flags &= ~(1 << dma_chnl);
    __raw_writel(flags, DMAC_ENBLD_CHNS);

    return;
}

int hi3520_dma_write( void *pci_addr_phy, void *ahb_addr_phy, int len)
{
    unsigned int control;
    int i;

    if(len>0xffffff){
        printk("transfer length is too long\n");
        goto out;
    }

    //write ahb_addr, pci_addr, and length to ahb registers
    hi3520_bridge_ahb_writel( WDMA_AHB_ADDR,(unsigned long)ahb_addr_phy);
    hi3520_bridge_ahb_writel(WDMA_PCI_ADDR, (unsigned long)(pci_addr_phy));
    control = (len<<8 | DMAW_MEM);
    hi3520_bridge_ahb_writel(WDMA_CONTROL, control);
    control |= DMAC_START;
    PCI_HAL_DEBUG(0, "dma control is 0x%x", control);
    hi3520_bridge_ahb_writel(WDMA_CONTROL, control);

    for(i = 0; i < 5000 && (hi3520_bridge_ahb_readl(WDMA_CONTROL) & 0x00000001); i++){
        udelay(1000);
    };
    if(hi3520_bridge_ahb_readl(WDMA_CONTROL) & 0x00000001)
        printk("pci_config_dma_read timeout!\n");

    return 0;
out:
    return -1;
}


int hi3520_dma_read(void *ahb_addr_phy, void *pci_addr_phy, int len)
{
    unsigned int control;
    int timeout = 5000000;

    if(len > 0xffffff){
        printk("transfer length is too long\n");
        goto out;
    }

    /*write ahb_addr, pci_addr, and length to ahb registers*/
    hi3520_bridge_ahb_writel(RDMA_AHB_ADDR, (unsigned long)ahb_addr_phy);
    hi3520_bridge_ahb_writel(RDMA_PCI_ADDR, (unsigned long)(pci_addr_phy));
    control = (len << 8 | DMAR_MEM);
    hi3520_bridge_ahb_writel(RDMA_CONTROL, control);
    control |= DMAC_START;
    hi3520_bridge_ahb_writel(RDMA_CONTROL, control);

    while((hi3520_bridge_ahb_readl(RDMA_CONTROL) & 0x00000001) && (--timeout)){
    }

    if(hi3520_bridge_ahb_readl(RDMA_CONTROL) & 0x00000001)
        printk("pci_config_dma_read timeout!\n");

    return 0;
out:
    return -1;
}


#endif


static spinlock_t   isil_dma_lock;
void    init_isil_dma_chan(void)
{
    spin_lock_init(&isil_dma_lock);
}

void    fpga_dma_receive(u32 src, u32 dest, u32 len)
{
    unsigned long flags;

    spin_lock_irqsave(&isil_dma_lock, flags);
    {
#if defined(POWERPC_PLATFORM)
        len += 3;
        len >>= 2;
        len <<= 2;
        ppc_dma_m2m(src, dest, len);
#elif defined(X86_PLATFORM)
        DMA_M2M(src, dest, len);
#elif defined(HISILICON_PLATFORM)
        if((src >= 0xb0000000) && (src < 0xc0000000) && ((dest < 0xb0000000) || (dest >= 0xc0000000))) {
            //hi3520_dma_read((void *)dest, (void *)src, len);
            hi3520_pci_dma_read(src, dest, len);
        }else if((dest >= 0xb0000000) && (dest < 0xc0000000) && ((src < 0xb0000000) || (src >= 0xc0000000))){
            //hi3520_dma_write((void *)dest, (void *)src, len);
            hi3520_pci_dma_write(dest, src, len);
            //hi3520_dma_read((void *)dest, (void *)src, len);
        }else{
            printk("src = 0x%08x, dst = 0x%08x, PCI DMA address error\n", src, dest);
        }
#elif defined(NXP_PLATFORM)
#warning    "don't implement nxp_platform dma function!"
#endif
    }
    spin_unlock_irqrestore(&isil_dma_lock, flags);

}

