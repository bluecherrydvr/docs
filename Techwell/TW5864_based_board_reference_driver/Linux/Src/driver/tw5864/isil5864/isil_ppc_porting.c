#include	<isil5864/isil_common.h>

static void __iomem	*isil_va_8313regbase = NULL;

/* 8313 register address */
#define DMAGSR		(isil_va_8313regbase+0x82A8)
#define DMASR0		(isil_va_8313regbase+0x8104)
#define DMASAR0		(isil_va_8313regbase+0x8110)
#define DMADAR0		(isil_va_8313regbase+0x8118)
#define DMABCR0		(isil_va_8313regbase+0x8120)
#define DMAMR0		(isil_va_8313regbase+0x8100)
#define DMACDAR0	(isil_va_8313regbase+0x8108)

#define PTLDR		(isil_va_8313regbase+0x0404)
#define PTCNR		(isil_va_8313regbase+0x0400)
#define PTEVR		(isil_va_8313regbase+0x0410)

#define	GPCFG		(isil_va_8313regbase+0xc00)
#define	GPDAT		(isil_va_8313regbase+0xc08)

#define SICFR		(isil_va_8313regbase+0x0700)
#define SIVCR		(isil_va_8313regbase+0x0704)
#define SIPNR_H		(isil_va_8313regbase+0x0708)
#define SIPNR_L		(isil_va_8313regbase+0x070c)
#define SIPRR_A		(isil_va_8313regbase+0x0710)
#define SIPRR_D		(isil_va_8313regbase+0x071c)
#define SIMSR_H		(isil_va_8313regbase+0x0720)
#define SIMSR_L		(isil_va_8313regbase+0x0724)
#define SICNR		(isil_va_8313regbase+0x0728)
#define SEPNR		(isil_va_8313regbase+0x072c)
#define SMPRR_A		(isil_va_8313regbase+0x0730)
#define SMPRR_B		(isil_va_8313regbase+0x0734)
#define SEMSR		(isil_va_8313regbase+0x0738)
#define SECNR		(isil_va_8313regbase+0x073c)
#define SERSR		(isil_va_8313regbase+0x0740)
#define SERMR		(isil_va_8313regbase+0x0744)
#define SERCR		(isil_va_8313regbase+0x0748)
#define SIFCR_H		(isil_va_8313regbase+0x0750)
#define SIFCR_L		(isil_va_8313regbase+0x0754)
#define SEFCR		(isil_va_8313regbase+0x0758)
#define SERFR		(isil_va_8313regbase+0x075c)
#define SCVCR		(isil_va_8313regbase+0x0760)
#define SMVCR		(isil_va_8313regbase+0x0764)

/* GPCM */
#define OR0 ((void __iomem *)((unsigned int)isil_va_8313regbase+0x5004))
#define OR1 ((void __iomem *)((unsigned int)isil_va_8313regbase+0x500C))
#define OR2 ((void __iomem *)((unsigned int)isil_va_8313regbase+0x5014))
#define OR3 ((void __iomem *)((unsigned int)isil_va_8313regbase+0x501C))

#define	TIMER_INVALIDE	(-1)
#define	TIMER_VALIDE	(1)

#define GPIO_PIN_29     (29)
#define WATCHDOG_BASE   (isil_va_8313regbase + 0x0200)
#define WATCHDOG_CTROL  (WATCHDOG_BASE + 0x04)
#define WATCHDOG_CNT    (WATCHDOG_BASE + 0x08)
#define WATCHDOG_SERV   (WATCHDOG_BASE + 0x0e)


void watchdog_reset(void)
{
    //__raw_writel(0x556C0000, WATCHDOG_SERV);
    //__raw_writel(0xAA390000, WATCHDOG_SERV);
    out_be16(WATCHDOG_SERV, 0x556c);
    out_be16(WATCHDOG_SERV, 0xaa39);
}

static void watchdog_release(void)
{
    __raw_writel((0xffff << 16), WATCHDOG_CTROL);
}

static void watchdog_init(u32 timeout)
{

    //__raw_writel((timeout << 16), WATCHDOG_CTROL);
    //__raw_writel((timeout << 16) | (1 << 2) | (1 << 1) | (1 << 0), WATCHDOG_CTROL);
}

static irqreturn_t      isil_timer_interrupt(int irq, void *dev_id)
{
    watchdog_reset();
    __raw_writel(0xffffffff, PTEVR);
    __raw_writel(0xffffffff, SIPNR_L);
    __raw_writel(0xffffffff, SIPNR_H);

    return IRQ_HANDLED;
}

void	read_sivcr(void)
{
    printk("SEPNR = 0x%x\n", __raw_readl(SEPNR));
}

void	read_ipic_regs(void)
{
    printk("\n\nSICFR = 0x%x\n", __raw_readl(SICFR));
    printk("SIVCR = 0x%x\n", __raw_readl(SIVCR));
    printk("SIPNR_H = 0x%x\n", __raw_readl(SIPNR_H));
    printk("SIPNR_L = 0x%x\n", __raw_readl(SIPNR_L));
    printk("SIPRR_A = 0x%x\n", __raw_readl(SIPRR_A));
    printk("SIPRR_D = 0x%x\n", __raw_readl(SIPRR_D));
    printk("SIPRR_A = 0x%x\n", __raw_readl(SIMSR_H));
    printk("SIPRR_D = 0x%x\n", __raw_readl(SIMSR_L));
    printk("SICNR = 0x%x\n", __raw_readl(SICNR));
    printk("SEPNR = 0x%x\n", __raw_readl(SEPNR));
    printk("SMPRR_A = 0x%x\n", __raw_readl(SMPRR_A));
    printk("SMPRR_B = 0x%x\n", __raw_readl(SMPRR_B));
    printk("SEMSR = 0x%x\n", __raw_readl(SEMSR));
    printk("SECNR = 0x%x\n", __raw_readl(SECNR));
    printk("SERSR = 0x%x\n", __raw_readl(SERSR));
    printk("SERMR = 0x%x\n", __raw_readl(SERMR));
    printk("SERCR = 0x%x\n", __raw_readl(SERCR));
    printk("SIFCR_H = 0x%x\n", __raw_readl(SIFCR_H));
    printk("SIFCR_L = 0x%x\n", __raw_readl(SIFCR_L));
    printk("SEFCR = 0x%x\n", __raw_readl(SEFCR));
    printk("SERFR = 0x%x\n", __raw_readl(SERFR));
    printk("SCVCR = 0x%x\n", __raw_readl(SCVCR));
    printk("SMVCR = 0x%x\n\n", __raw_readl(SMVCR));
}

void	clear_irq0_pending(void)
{
    u32	irq_flags;

    irq_flags = __raw_readl(SEPNR);
    irq_flags |= 0x80000000;
    __raw_writel(irq_flags, SEPNR);
}

void	set_gpio13_high(void)
{
    u32	gpio;
    gpio = __raw_readl(GPDAT);
    gpio |= 0x40000;
    __raw_writel(gpio, GPDAT);
}

void	set_gpio13_low(void)
{
    u32	gpio;
    gpio = readl(GPDAT);
    gpio &= (~0x40000);
    writel(gpio, GPDAT);
}

static int s_virq = -1;

void	ppc_timer_init(void)
{
    u32 reg_value;
    u32	gpio;
    u32 ret = 0;

    gpio = __raw_readl(GPCFG);
    gpio |= 0x40000;
    __raw_writel(gpio, GPCFG);
    //reg_value = 0xa000000 >> 8;//8 = 4ms ,7==8ms 0==1s
    reg_value = 0xcb800;
    s_virq = irq_create_mapping(NULL, IRQ_SYS_INTERNAL);
    if(s_virq == NO_IRQ)
    {
        printk("map irq failed !\n");
        return ;
    }
    ret = request_irq(s_virq, (void *)isil_timer_interrupt, IRQF_DISABLED, "isil_timer", NULL);
    if(ret) {
        printk("request_irq is err %d, (hw %2d --> vir %2d).\n", ret, IRQ_SYS_INTERNAL, s_virq);
        return ;
    }
#if 1 //disbale pit
    __raw_writel(reg_value, PTLDR);
    __raw_writel(0x81, PTCNR);
#endif
}


void	ppc_timer_exit(void)
{
    disable_irq(s_virq);
    free_irq(s_virq, NULL);
}

void	disable_timer_int(void)
{
    disable_irq(s_virq);
}

void	enable_timer_int(void)
{
    enable_irq(s_virq);
}


void ppc_dma_m2m(u32 src, u32 dest, u32 len)
{
    u32 reg_value;
    u32 timeout = 1000000;

    do{
        reg_value = readl(DMASR0);
    } while(likely(reg_value & 0x4)  && (--timeout));
    if(!timeout){
        printk("DMA time out\n");
    }
    writel(src, DMASAR0);
    writel(dest, DMADAR0);
    writel(len, DMABCR0);	
    reg_value = (0<<26)|(0<<25)|(0<<24)|(0<<22)|(1<<20)|(0<<19)|(0<<16)|(0<<14)|(0<<7)|(0<<3)|(1<<2);
    writel(reg_value, DMAMR0);
    reg_value = (0<<26)|(0<<25)|(0<<24)|(0<<22)|(1<<20)|(0<<19)|(0<<16)|(0<<14)|(0<<7)|(0<<3)|(1<<2)|1;
    writel(reg_value, DMAMR0);
    timeout = 1000000;
    do{
        reg_value = readl(DMASR0);
    } while(likely(reg_value & 0x4) && (--timeout));
    if(readl(DMASR0) & (1 << 7)){
        printk("DMA Tx err!\n");
    }
    if(!timeout){
        if(readl(DMASR0) & (1 << 2)) {
            printk("DMA transfer is in progress, 0x%08x counts left\n", readl(DMABCR0));
        }
        printk("DMA time out, 0x%08x, 0x%08x\n", readl(DMASR0), readl(DMABCR0));
    }
}

static __used void upm_test_fpga(void)
{
    int table1[64]= {
        0x0ffffc00,  0x0ffffc00,  0x0ffffc00,  0xfffffc05, //Words 0 to 3
        0xfffffc00,  0xfffffc00,  0xfffffc00,  0xfffffc00, //Words 4 to 7
        0x0ffffc00,  0x0ffffc08,  0x0ffffc08,  0x0ffffc8c, //Words 8 to 11
        0x0ffffc8c,  0x0ffffc0c,  0x0ffffc04,  0x3ffffc04, //Words 12 to 15
        0xfffffc05,  0xfffffc00,  0xfffffc00,  0xfffffc00, //Words 16 to 19
        0xfffffc00,  0xfffffc00,  0xfffffc00,  0xfffffc00, //Words 20 to 23
        0xefeffc00,  0x0faffc04,  0x3fbffc01,  0xfffffc00, //Words 24 to 27
        0xfffffc00,  0xfffffc00,  0xfffffc00,  0xfffffc00, //Words 28 to 31
        0x0faffc8c,  0x0faffc8c,  0xfffffc00,  0xfffffc00, //Words 32 to 35
        0xfffffc00,  0xfffffc00,  0xfffffc01,  0xfffffc00, //Words 36 to 39
        0xfffffc00,  0xfffffc00,  0xfffffc00,  0xfffffc00, //Words 40 to 43
        0xfffffc00,  0xfffffc00,  0xfffffc00,  0xfffffc00, //Words 44 to 47
        0xfffffc00,  0xfffffc00,  0xfffffc00,  0xfffffc00, //Words 48 to 51
        0xfffffc00,  0xfffffc00,  0xfffffc00,  0xfffffc00, //Words 52 to 55
        0xfffffc00,  0xfffffc00,  0xfffffc00,  0xfffffc01, //Words 56 to 59
        0xfffffc00,  0xfffffc00,  0xfffffc00,  0xfffffc01  //Words 60 to 63
    };

    int temp;
    int i;
    int *mxmra, *mdr, *dummy;
    void	*pfpgaupmcs0;
    //	short int j;
    //	int *pimmbar;isil_va_8313regbase

    //	pimmbar = ioremap(0xe0000000, 0x100000);
    //	*((int *)((int)pimmbar+0x30)) = 0x70000000;
    //	*((int*)((int)pimmbar+0x34) )= 0x80000018;
    //	*((int*)((int)pimmbar+0x5010)) = 0x70001081;
    //	*((int*)((int)pimmbar+0x5014)) = 0xfe000000;
    *((int *)((int)isil_va_8313regbase+0x30)) = 0x70000000;
    *((int*)((int)isil_va_8313regbase+0x34) )= 0x80000018;
    *((int*)((int)isil_va_8313regbase+0x5010)) = 0x70001081;
    *((int*)((int)isil_va_8313regbase+0x5014)) = 0xfe000000;

    //	int *mxmra = (int *)((int)pimmbar+0x5070);
    //	int *mdr =   (int *)((int)pimmbar+0x5088);
    //	int *mar =   (int *)((int)pimmbar+0x5068);

    mxmra = (int *)((int)isil_va_8313regbase+0x5070);
    mdr =   (int *)((int)isil_va_8313regbase+0x5088);
    //	int *mar =   (int *)((int)isil_va_8313regbase+0x5068);


    pfpgaupmcs0 = ioremap(0x70000000, 0x80000);
    /*
       int  pfpgaupmcs1 = ioremap(0x70400000, 0x80000);
       int  pfpgaupmcs2 = ioremap(0x70800000, 0x80000);
       int  pfpgaupmcs3 = ioremap(0x70c00000, 0x80000);
       int  pfpgaupmcs4 = ioremap(0x71000000, 0x80000);
       */
    dummy = (int *)pfpgaupmcs0;
    //	int *dummy = (int *)isil_platdata0.chip_va_sync;

    *mxmra = (*mxmra & 0xCFFFFFC0) | 0x10000000;

    for (i = 0; i < 64; i++)
    {
        *mdr = table1[i];

        __asm__ __volatile__ ("sync");

        *dummy=1;		/* Write the value to memory and increment MAD */

        __asm__ __volatile__ ("sync");
        //printk("i=%d, *mxmra=%08x.\n",i, *mxmra);
    }

    *mxmra = (*mxmra & 0xCFFFFFC0) | 0x0000000;
    *mxmra = 0x0819c400;
    temp = *mxmra;

    //	iounmap(pimmbar);
    iounmap(pfpgaupmcs0);
    /*	
        iounmap(pfpgaupmcs1);
        iounmap(pfpgaupmcs2);	
        iounmap(pfpgaupmcs3);	
        iounmap(pfpgaupmcs4);
        */
}

void gpcm_init(void)
{
    u32 reg_value;

    reg_value = __raw_readl(OR0);
    reg_value = __raw_readl(OR1);
    //	reg_value |= 0xfe000d09;
    reg_value = 0xfe000df5;
    //	reg_value |= 0xfe000c09;
    __raw_writel(reg_value, OR1);
}

void isil_clear_irq_pending(int irq)
{
    u32 val;

    val = __raw_readl(isil_va_8313regbase + 0x72c);
    printk("-------->irq pending: 0x%08x\n", val);
    if(val & (0x80000000 >> 1))
    {
        printk("irq1 is pending, now clear it!\n");
        __raw_writel(val | (0x80000000 >> 1), isil_va_8313regbase + 0x72c);
    }
}

int gpio_pin_set(int gpio, int bool)
{
    u32 val;

    bool = !!bool;
    val = __raw_readl(isil_va_8313regbase + 0xc08);
    if(bool)
    {
        val |= (0x80000000 >> gpio);
    }else{
        val &= ~(0x80000000 >> gpio);
    }
    __raw_writel(val, isil_va_8313regbase + 0xc08);

    return 0;
}

int gpio_pin_init(int gpio)
{
    u32 val;

    val = __raw_readl(isil_va_8313regbase + 0x114);
    val |= (0xc0000000 >> 8);
    __raw_writel(val, isil_va_8313regbase + 0x114);

    val = __raw_readl(isil_va_8313regbase + 0xc00);
    val |= (0x80000000 >> gpio);/*set output mode*/
    __raw_writel(val, isil_va_8313regbase + 0xc00);

    return 0;
}

int isil_map_init(void)
{
    isil_va_8313regbase = ioremap(0xe0000000, 0x10000);
    if(isil_va_8313regbase == NULL) {
        printk("8313 regs base err\n");
        return -ENOMEM;
    }
    watchdog_init(2000);//2000ms
    return 0;
}

void isil_map_exit(void)
{
    watchdog_release();
    if(NULL != isil_va_8313regbase) {
        iounmap(isil_va_8313regbase);
        isil_va_8313regbase = NULL;
    }
}

