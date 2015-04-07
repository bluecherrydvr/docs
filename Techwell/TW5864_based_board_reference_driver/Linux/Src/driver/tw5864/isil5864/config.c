#include   <isil5864/isil_common.h>

#define MPB_CMD_REG	    (0xb800)
#define MPB_DATA_REG	(0xb804)
#define MPB_READ_MODE	0
#define MPB_WRITE_MODE	1

//#define PLATFORM_FPGA_155
#define PLATFORM_FPGA_330

#define PCI_I2C_TIMEOUT  (30000)

static unsigned char tbl_pal_tw2864_common[] __used = {
    0x00,0x00,0x64,0x11,
    0x80,0x80,0x00,0x12,
    0x12,0x20,0x0a,0xD0,
    0x00,0x00,0x07,0x7F,
};

static unsigned char tbl_ntsc_tw2864_common[] __used = {
    0x00,0x00,0x64,0x11,
    0x80,0x80,0x00,0x02,
    0x12,0xF0,0x0C,0xD0,
    0x00,0x00,0x07,0x7F
};

static unsigned char tbl_pal_tw2864_common2[] __used = {
    0x00,0x22,0x00,0x00,
    0x22,0x00,0x00,0x22,
    0x00,0x00,0x22,0x00,
};

static unsigned char tbl_tw2864_other[] __used = {
    0xfb,0x6f,0xfc,0xff,
    0xdb,0xc1,0xd2,0x01,
    0xdd,0x00,0xde,0x00,
    0xe1,0xc0,0xe2,0xaa,
    0xe3,0xaa,0xf8,0x64,
    0xf9,0x11,0xaa,0x00,
    0x9e,0x72,0x9c,0x20,
    0x94,0x14,0xca,0xaa,
    0xcb,0x00,0x89,0x02,
    0xfa,0xc6,0xcf,0x83,
#ifdef PLATFORM_FPGA_155
    0x9f,0x07,0x96,0x02,
#endif
#ifdef PLATFORM_FPGA_330
    0x9f,0x00,0xb1,0x2a,//330
    0x9e,0x7a,
#endif
};

static unsigned char tbl_pal_tw2865_common[] __used  = {
    0x00,0x00,0x64,0x11,
    0x80,0x80,0x00,0x12,
    0x17,0x20,0x0C,0xD0,
    0x00,0x00,0x07,0x7F,
};

static unsigned char  tbl_ntsc_tw2865_common[] __used = {
    0x00,0x00,0x64,0x11,
    0x80,0x80,0x00,0x02,
    0x12,0xF0,0x0C,0xD0,
    0x00,0x00,0x07,0x7F
};

static unsigned char tbl_tw2865_other1[] __used  = {
    0xfa,0x4a,0xfb,0x6f,
#ifdef PLATFORM_FPGA_155
    0xfc,0xff,0x9c,0xa0,
#endif
#ifdef PLATFORM_FPGA_330
    0xfc,0xff,0x9c,0x20,//330
#endif
    0x9e,0x72,0xca,0x02,
    0xf9,0x51,0xaa,0x00,
    0x41,0xd4,0x43,0x08,
    0x6b,0x0f,0x6c,0x0f,
    0x61,0x02,0x96,0xe6,//0xe6
#ifdef PLATFORM_FPGA_155
    0x97,0xc3,0x9f,0x77,
#endif
#ifdef PLATFORM_FPGA_330
    0x97,0xc3,0x9f,0x03,//330
    0xb1,0x2a,0x9e,0x7a,
    0x18,0x19,0x1a,0x06,
    0x28,0x19,0x2a,0x06,
    0x38,0x19,0x3a,0x06,
    0x60,0x15,
#endif
};

static unsigned char tbl_tw2866_other1[] __used = {
    0xfa,0x4a,0xfb,0x6f,
#ifdef PLATFORM_FPGA_155
    0xfc,0xff,0x9c,0xa0,
#endif
#ifdef PLATFORM_FPGA_330
    0xfc,0xff,0x9c,0x20,//330
#endif
    0x9e,0x72,0xca,0x02,
    0xf9,0x51,0xaa,0x00,
    0x41,0xd4,0x43,0x08,
    0x6b,0x0f,0x6c,0x0f,
    0x61,0x02,0x96,0xe6,//0xe6
#ifdef PLATFORM_FPGA_155
    0x97,0xc3,0x9f,0x77,
#endif
#ifdef PLATFORM_FPGA_330
    0x97,0xc3,0x9f,0x00,//330
    0xb1,0x2a,0x9e,0x7a,
    0x5b,0xff,0x08,0x19,
    0x0a,0x06,0x18,0x19,
    0x1a,0x06,0x28,0x19,
    0x2a,0x06,0x38,0x19,
    0x3a,0x06,0x60,0x15,
#endif
};

static unsigned char tbl_tw2865_other2[] __used = {
    0x73,0x01,0xf8,0xc4,
    0xf9,0x51,0x70,0x08,
    0x7f,0x80,0xcf,0x80
};

static unsigned char tbl_tw2865_other3[] __used = {
    0x89,0x05,0x7e,0xc0,
    0xe0,0x00
};
static unsigned char  audio_tw2865_common[] __used = {
    0x33,0x33,0x03,0x31,
    0x75,0xb9,0xfd,0x20,
    0x64,0xa8,0xec,0xC1,
    0x00,0x00,0x00,0x80,
    0x00,0xC0,0xAA,0xAA
};

static unsigned char audio_tbl_pal_tw2865_8KHz[] __used = {
    0x83,0xB5,0x09,0x00,
    0xA0,0x00
};
static unsigned char audio_tbl_pal_tw2865_16KHz[] __used = {
    0x07,0x6B,0x13,0x00,0x40,0x01
};
static unsigned char audio_tbl_ntsc_tw2865_8KHz[] __used = {
    0x83,0xB5,0x09,0x78,0x85,0x00
};

static unsigned char audio_tbl_ntsc_tw2865_16KHz[] __used = {
    0x07,0x6B,0x13,0xEF,0x0A,0x01
};

int pci_i2c_read(isil_chip_t *chip, u8 devid, u8 devfn, u8 *buf);
int pci_i2c_multi_read(isil_chip_t *chip, u8 devid, u8 devfn, u8 *buf, u32 count)
{
    int i = 0;
    u32 val = 0;
    int timeout = PCI_I2C_TIMEOUT;
    unsigned long   flags;

    local_irq_save(flags);
    for(i = 0; i < count; i++)
    {
        val = (1 << 24) | ((devid | 0x01) << 16) | ((devfn + i) << 8);

        chip->io_op->chip_write32(chip, PCI_I2C, val);

        do{
            val = chip->io_op->chip_read32(chip, PCI_I2C) & (0x01000000);
        }while((!val) && (--timeout));
        if(!timeout){
            local_irq_restore(flags);
            ISIL_DBG(ISIL_DBG_ERR, "dev 0x%x, fn 0x%x\n", devid, devfn);
            return -ETIMEDOUT;
        }
        buf[i] = (u8)chip->io_op->chip_read32(chip, PCI_I2C);
    }
    local_irq_restore(flags);

    return ISIL_OK;
}
int pci_i2c_multi_write(isil_chip_t *chip, u8 devid, u8 devfn, u8 *buf, u32 count)
{
    int i = 0;
    u32 val = 0;
    int timeout = PCI_I2C_TIMEOUT;
    unsigned long   flags;

    local_irq_save(flags);
    for(i = 0; i < count; i++)
    {
        val = (1 << 24) | ((devid & 0xfe) << 16) | ((devfn + i) << 8) | buf[i];
        chip->io_op->chip_write32(chip, PCI_I2C, val);
        do{
            val = chip->io_op->chip_read32(chip, PCI_I2C) & (0x01000000);
        }while((!val) && (--timeout));
        if(!timeout){
            local_irq_restore(flags);
            ISIL_DBG(ISIL_DBG_ERR, "dev 0x%x, fn 0x%x, 0x%x\n", devid, devfn, buf[i]);
            return -ETIMEDOUT;
        }
    }
    local_irq_restore(flags);

    return ISIL_OK;
}
int pci_i2c_read(isil_chip_t *chip, u8 devid, u8 devfn, u8 *buf)
{
    u32 val = 0;
    int timeout = PCI_I2C_TIMEOUT;
    unsigned long   flags;

    local_irq_save(flags);
    val = (1 << 24) | ((devid | 0x01) << 16) | (devfn << 8);

    chip->io_op->chip_write32(chip, PCI_I2C, val);
    do{
        val = chip->io_op->chip_read32(chip, PCI_I2C) & (0x01000000);
    }while((!val) && (--timeout));
    if(!timeout){
        local_irq_restore(flags);
        ISIL_DBG(ISIL_DBG_ERR, "dev 0x%x, fn 0x%x\n", devid, devfn);
        return -ETIMEDOUT;
    }

    *buf = (u8)chip->io_op->chip_read32(chip, PCI_I2C);
    local_irq_restore(flags);

    return ISIL_OK;
}
int pci_i2c_write(isil_chip_t *chip, u8 devid, u8 devfn, u8 buf)
{
    u32 val = 0;
    int timeout = PCI_I2C_TIMEOUT;
    unsigned long   flags;

    local_irq_save(flags);
    val = (1 << 24) + ((devid & 0xfe) << 16) + (devfn << 8) + buf;
    chip->io_op->chip_write32(chip, PCI_I2C, val);
    do{
        val = chip->io_op->chip_read32(chip, PCI_I2C) & (0x01000000);
    }while((!val) && (--timeout));
    local_irq_restore(flags);
    if(!timeout){
        ISIL_DBG(ISIL_DBG_ERR, "dev 0x%x, fn 0x%x, 0x%x\n", devid, devfn, buf);
        return -ETIMEDOUT;
    }

    return ISIL_OK;
}
int pci_i2c_wscatter(isil_chip_t *chip, u8 devid, u8 *buf, u32 count)
{
    int i = 0;
    u32 val = 0;
    int timeout = PCI_I2C_TIMEOUT;
    unsigned long   flags;

    local_irq_save(flags);
    for(i = 0; i < count; i++)
    {
        val = (1 << 24) + ((devid & 0xfe) << 16) + (buf[i * 2+ 0] << 8) + buf[i * 2 +1];
        chip->io_op->chip_write32(chip, PCI_I2C, val);
        do{
            val = chip->io_op->chip_read32(chip, PCI_I2C) & (0x01000000);
        }while((!val) && (--timeout));
        if(!timeout){
            local_irq_restore(flags);
            ISIL_DBG(ISIL_DBG_ERR, "dev 0x%x, fn 0x%x, 0x%x\n", devid, buf[i * 2], buf[i * 2 + 1]);
            return -ETIMEDOUT;
        }
    }
    local_irq_restore(flags);

    return ISIL_OK;
}

void pci_init_tw2864(isil_chip_t *chip, u8 iic)
{
    u32 ch;

    for(ch = 0 ; ch < 4 ; ch++)
    {
        pci_i2c_multi_write(chip , iic , ch * 0x10 , tbl_pal_tw2864_common , 16);
    }
#ifdef PLATFORM_FPGA_155
    pci_i2c_wscatter(chip , iic , tbl_tw2864_other ,22);
#else
    pci_i2c_wscatter(chip , iic , tbl_tw2864_other ,23);
#endif
    pci_i2c_write(chip, iic, 0xcf, 0x83);
    pci_i2c_write(chip, iic, 0xe0, 0x00);
}

static __used void pci_init_tw2865(isil_chip_t *chip, u8 iic)
{
    u32 ch;

    for(ch = 0 ; ch < 4 ; ch++)
    {
        pci_i2c_multi_write(chip, iic, ch * 0x10, tbl_pal_tw2865_common, 16);
    }
#ifdef PLATFORM_FPGA_155
    pci_i2c_wscatter(chip , iic , tbl_tw2865_other1 , sizeof(tbl_tw2865_other1)>>1);
#else
    pci_i2c_wscatter(chip , iic , tbl_tw2865_other1 , sizeof(tbl_tw2865_other1)>>1);
#endif
    pci_i2c_multi_write(chip , iic , 0xd0 , audio_tw2865_common , 20);
    pci_i2c_wscatter(chip , iic , tbl_tw2865_other2 , 6);
    pci_i2c_multi_write(chip , iic , 0xf0 , audio_tbl_pal_tw2865_8KHz , 6);
    pci_i2c_wscatter(chip , iic , tbl_tw2865_other3 , 3);
    pci_i2c_write(chip, iic, 0xe0, 0x10);
}

static void pci_init_tw2866(isil_chip_t *chip, u8 iic)
{
    u32 ch;

    for(ch = 0 ; ch < 4 ; ch++)
    {
        pci_i2c_multi_write(chip, iic, ch * 0x10, tbl_pal_tw2865_common, 16);
    }
#ifdef PLATFORM_FPGA_155
    pci_i2c_wscatter(chip , iic, tbl_tw2866_other1 , 17);
#else
    pci_i2c_wscatter(chip , iic , tbl_tw2866_other1 , sizeof(tbl_tw2866_other1)>>1);
#endif
    pci_i2c_multi_write(chip , iic , 0xd0 , audio_tw2865_common , 20);
    pci_i2c_wscatter(chip , iic , tbl_tw2865_other2 , 6);
    pci_i2c_multi_write(chip , iic , 0xf0 , audio_tbl_pal_tw2865_8KHz , 6);
    pci_i2c_wscatter(chip , iic , tbl_tw2865_other3 , 3);
    pci_i2c_write(chip, iic, 0xe0, 0x10);
#ifdef TW5864_ASIC_NEW
    pci_i2c_write(chip, iic, 0x60, 0x15);
    pci_i2c_write(chip, iic, 0x61, 0x3);
#endif
    pci_i2c_write(chip, iic, 0xcf, 0x80);
}

/*auto detect CLKP_DEL delay*/
static int tw28xx_clkp_delay(isil_chip_t *chip, u8 devid, u32 base_ch, u32 limit)
{
    if(chip && (base_ch < ISIL_PHY_VD_CHAN_NUMBER) && (limit <= (ISIL_PHY_VD_CHAN_NUMBER>>2))) {
        int delay;
        u8 flags;

        delay = -1;
        pci_i2c_read(chip, devid, 0x9f, &flags); 
        while((++delay) < 0x10) {
            pci_i2c_write(chip, devid, 0x9f, delay);
            /*only bus0 can detect colume and line*/
            chip->io_op->chip_write32(chip, VD_BUS_MAP(0), base_ch);
            chip->io_op->chip_write32(chip, 0x9218, 0x1);/*clear error flags*/
            mdelay(100);
            if(chip->io_op->chip_read32(chip, 0x9218)) {
                continue;
            }
            printk("auto detect CLKP_DEL = %02x\n", delay);
            break;
        }
        if(delay >= 0x10) {
            printk("can't find suitable clkp_del for devid 0x%02x\n", devid);
            pci_i2c_write(chip, devid, 0x9f, flags);

            return -EFAULT;
        }
        return ISIL_OK;
    }

    return ISIL_ERR;
}

void pci_init_ad(isil_chip_t *chip)
{
    unsigned int val;

    val = chip->io_op->chip_read32(chip, MCU_IIC_CONF);
    val |= 0x01;
    chip->io_op->chip_write32(chip, MCU_IIC_CONF, val);
    chip->io_op->chip_write32(chip, I2C_PHASE_SHIFT, 0x01);

#if defined(FPGA_330_5864_TESTING) //FPGA 330
#if defined(TW5864_CHIP) //TW5864
    pci_init_tw2864(chip, 0x52);
    tw28xx_clkp_delay(chip, 0x52, 4, 4);
    pci_init_tw2864(chip, 0x54);
    tw28xx_clkp_delay(chip, 0x54, 8, 4);
    pci_init_tw2864(chip, 0x56);
    tw28xx_clkp_delay(chip, 0x56, 12, 4);
    pci_init_tw2865(chip, 0x50);
#elif defined(TW5866_CHIP)
    pci_init_tw2866(chip, 0x50);
    pci_init_tw2866(chip, 0x52);
    pci_init_tw2866(chip, 0x54);
    pci_init_tw2866(chip, 0x56);
#else
#error "No chip module defined!"
#endif
#else
    pci_init_tw2866(chip, 0x52);
    tw28xx_clkp_delay(chip, 0x52, 4, 4);
    pci_init_tw2866(chip, 0x54);
    tw28xx_clkp_delay(chip, 0x54, 8, 4);
    pci_init_tw2866(chip, 0x56);
    tw28xx_clkp_delay(chip, 0x56, 12, 4);
#endif
}

//#endif //POWERPC_PCI_PLATFORM

static void mpb_operation(isil_chip_t *chip, u32 addr,u32 *data,u32 mode)
{
    u32 reg_value;
    u32 timeout = 1000;

    switch(mode)
    {
        case MPB_READ_MODE:
            while((chip->io_op->chip_read32(chip, MPB_CMD_REG)>>31) && (timeout--));
            if(!timeout) {
                ISIL_DBG(ISIL_DBG_ERR, "read addr 0x%03x timeout\n", addr);
            }
            reg_value = ((1<<25)|(mode<<24)|(addr << 2));
            //reg_value = ((1<<25)|(mode<<24)|(addr));
            chip->io_op->chip_write32(chip, MPB_CMD_REG, reg_value);
            timeout = 1000;
            while(((chip->io_op->chip_read32(chip, MPB_CMD_REG))>>31) && (timeout--));
            if(!timeout) {
                ISIL_DBG(ISIL_DBG_ERR, "read addr 0x%03x timeout\n", addr);
            }
            *data = chip->io_op->chip_read32(chip, MPB_DATA_REG);
            break;
        case MPB_WRITE_MODE:
            while((chip->io_op->chip_read32(chip, MPB_CMD_REG)>>31) && (timeout--));
            if(!timeout) {
                ISIL_DBG(ISIL_DBG_ERR, "write addr 0x%03x timeout\n", addr);
            }
            chip->io_op->chip_write32(chip, MPB_DATA_REG, *data);
            reg_value = ((1<<25)|(mode<<24)|(addr << 2));
            //reg_value = ((1<<25)|(mode<<24)|(addr));
            chip->io_op->chip_write32(chip, MPB_CMD_REG, reg_value);
            break;
    }
}

void mpb_write(isil_chip_t *chip, u32 addr,u32 data)
{
#ifdef PLATFORM_FPGA_155
    return;
#endif

    mpb_operation(chip, addr,&data,MPB_WRITE_MODE);
}

u32 mpb_read(isil_chip_t *chip, u32 addr)
{
    u32 data;

    mpb_operation(chip, addr,&data,MPB_READ_MODE);
    return data;
}
