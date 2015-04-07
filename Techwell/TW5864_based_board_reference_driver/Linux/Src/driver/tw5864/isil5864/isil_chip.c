#include    <isil5864/isil_common.h>
#include    <isil5864/tc_common.h>

#ifdef  POWERPC_PLATFORM
extern int  isil_map_init(void);
extern void isil_map_exit(void);
extern void ppc_timer_init(void);
extern void ppc_timer_exit(void);
static	DVMNVS_CHIP_VIDEO_ENCODE_CAPABILITY platform_video_encode_cap[2] = {
    {
        .first_device_number = 0,
        .video_encode_work_mode = ENCODE_4_D1,
        .encode_reg_4 = (DAON|DEBLOCKINGON|INTRAON|CMOSON|INTERON|DDRON),
        .fpga_sync_base_addr = 0,
        .fpga_sync_section_len = 0,
        .fpga_async_base_addr = 0,
        .fpga_async_section_len = 0,
        .ddr_map_mode = DDR_MAP_COMPRESS_DISABLE,
    },
    {
        .first_device_number = 16,
        .video_encode_work_mode = ENCODE_4_D1,
        .encode_reg_4 = (DAON|DEBLOCKINGON|INTRAON|CMOSON|INTERON|DDRON),
        .fpga_sync_base_addr = 0,
        .fpga_sync_section_len = 0,
        .fpga_async_base_addr = 0,
        .fpga_async_section_len = 0,
        .ddr_map_mode = DDR_MAP_COMPRESS_DISABLE,
    }
};
#endif

#if defined(X86_PLATFORM) || defined(HISILICON_PLATFORM) || defined(NXP_PLATFORM)
static	DVMNVS_CHIP_VIDEO_ENCODE_CAPABILITY platform_video_encode_cap[2] = {
    {
        .first_device_number = 0,
        .video_encode_work_mode = ENCODE_4_D1,
        .encode_reg_4 = (DEBLOCKINGON|INTRAON|CMOSON|INTERON|DDRON),
        .fpga_sync_base_addr = 0,
        .fpga_sync_section_len = 0,
        .fpga_async_base_addr = 0,
        .fpga_async_section_len = 0,
        .ddr_map_mode = DDR_MAP_COMPRESS_DISABLE,
    },
    {
        .first_device_number = 16,
        .video_encode_work_mode = ENCODE_4_D1,
        .encode_reg_4 = (DEBLOCKINGON|INTRAON|CMOSON|INTERON|DDRON),
        .fpga_sync_base_addr = 0,
        .fpga_sync_section_len = 0,
        .fpga_async_base_addr = 0,
        .fpga_async_section_len = 0,
        .ddr_map_mode = DDR_MAP_COMPRESS_DISABLE,
    }
};
#endif

void    get_isil_avSync_device(isil_avSync_dev_t **ptr_device, int inode_id)
{
    type_bus_t  *isil_host_bus;
    isil_chip_t	*chip;
    isil_chip_bus_t *chip_bus;

    *ptr_device = NULL;
#if defined(PCI_PLATFORM)
    get_isil_pci_bus(&isil_host_bus);
#else
    get_isil_host_bus(&isil_host_bus); 
#endif

    isil_host_bus->op->find_chip_bus_in_type_bus_table(isil_host_bus, &chip_bus, (inode_id>>4));
    if(chip_bus != NULL){
        chip = to_get_chip_with_chip_bus(chip_bus);
        //if(inode_id < ISIL_PHY_VD_CHAN_NUMBER){
        *ptr_device = &chip->av_avSync_device_chan[inode_id & 0xf]; 
        //} 
    }else{
        printk("%s,%d: No such chip!\n", __FILE__, __LINE__);
    }
}

int chip_request_irq(isil_chip_t *chip, u32 irq, isil_irq_handler_t handler, const char *fnname, void *context)
{
    unsigned long flags;
    struct chip_irq_action *action;
    struct chip_irq_desc   *desc;

    if(!chip || !handler){
        return -EINVAL;
    }

    desc = chip_irq_to_desc(chip, irq);
    if(!desc){
        return -EINVAL;
    }
    if(desc->flags & IRQ_DISABLED) {
        return -EFAULT;
    }
    spin_lock_irqsave(&desc->lock, flags);
    action = &desc->action;
    action->handler = handler;
    action->irq     = irq;
    action->name    = fnname;
    action->context = context;
    action->next    = NULL;

    desc->name           = fnname;
    //desc->irq_count      = 0;
    //desc->irqs_unhandled = 0;
    atomic_inc(&desc->depth);
    spin_unlock_irqrestore(&desc->lock, flags);

    chip_enable_irq(chip, irq);

    return ISIL_OK;
}

void chip_free_irq(isil_chip_t *chip, u32 irq, void *context)
{
    struct chip_irq_desc    *desc;
    struct chip_irq_action  *action;
    unsigned long flags;

    desc = chip_irq_to_desc(chip, irq);
    if(!desc){
        return;
    }
    spin_lock_irqsave(&desc->lock, flags);
    action = &desc->action;

    action->handler = NULL;
    action->irq     = irq;
    action->name    = NULL;
    action->context = NULL;
    action->next    = NULL;
    atomic_dec(&desc->depth);

    spin_unlock_irqrestore(&desc->lock, flags);

    chip_disable_irq(chip, irq);

    return ;
}

void chip_enable_irq(isil_chip_t *chip, u32 irq)
{
    isil_interrupt_control_t *irq_ctrl;

    if(chip && (irq < CHIP_IRQS_NR)){
        irq_ctrl = &chip->isil_int_control;
        irq_ctrl->system_op->enable_interrupt_source(irq_ctrl, irq);
    }
}

void chip_disable_irq(isil_chip_t *chip, u32 irq)
{
    isil_interrupt_control_t *irq_ctrl;

    if(chip && (irq < CHIP_IRQS_NR)){
        irq_ctrl = &chip->isil_int_control;
        irq_ctrl->system_op->disable_interrupt_source(irq_ctrl, irq);
    }
}

void chip_clear_irq(isil_chip_t *chip, u32 irq)
{
    isil_interrupt_control_t *irq_ctrl;

    if(chip && (irq < CHIP_IRQS_NR)){
        irq_ctrl = &chip->isil_int_control;
        irq_ctrl->hardware_op->ack_interrupt(irq_ctrl, irq);
    }
}

static isil_irq_handler_t isil_vlc_irq_handler(int irq, void *context)
{
    isil_chip_t	*chip;

    chip = context;
    if(chip != NULL){
        if(chip->curr_h264_encode_chan != NULL) {
            chip->curr_h264_encode_chan->encode_control.op->irq_func(irq, (void*)&chip->curr_h264_encode_chan->encode_control);
        } else {
            printk("no register isr curr service chan\n");
        }
    }

    return 0;
}

#if defined(HISILICON_PLATFORM)
#include <asm/arch/pci.h>
#endif
static irqreturn_t  isil_top_isr(int irq, void *id)
{
    isil_interrupt_control_t *control = (isil_interrupt_control_t*)id;
    isil_chip_t	*chip = control->chip;
    u32     chip_irq = 0;
    struct chip_irq_desc *desc;
    struct chip_irq_action *action;
    unsigned long   flags;

#if defined(HISILICON_PLATFORM)
    hi3520_bridge_ahb_writel(CPU_ISTATUS, 0x1000000);
#endif

    spin_lock_irqsave(&chip->lock, flags);
    control->hardware_op->get_interrupt_status(control);
    while(control->chip_interrupt_status){
        for(chip_irq = 0; chip_irq < CHIP_IRQS_NR; chip_irq++){
            if(control->chip_interrupt_status & (1 << chip_irq)){
                struct timeval tv_base, tv_curr;
                struct chip_irq_desc *timer_desc;

                timer_desc = chip_irq_to_desc(chip, IRQ_TIMER_TYPE_INTR);
                desc = chip_irq_to_desc(chip, chip_irq);
                desc->irq_count++;
                if(timer_desc->irq_count > desc->base_tick) {
                    desc->base_tick = timer_desc->irq_count;
                    desc->delta_irq_count = desc->irq_count - desc->base_irq_count;
                    desc->base_irq_count = desc->irq_count;
                    if(desc->delta_irq_count >= 20) {
                        desc->irq_overflow_count++;
                        if(desc->irq_overflow_count >= 5000) {
                            printk("!!!!!!!!!!!!!!!!!%s = %2d overflow, delta irq %d/%dms overflow!!!!!!!!!!!!!!!\n", desc->name, chip_irq, desc->delta_irq_count, MS_PER_TICK);
                            /*disable this irq*/
                            desc->flags |= IRQ_DISABLED;
                            chip_disable_irq(chip, chip_irq);
                        }
                    }else{
                        desc->irq_overflow_count = 0;
                    }
                }
                do_gettimeofday(&tv_base);
                action = &desc->action;
                if(action->handler) {
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 18)
                    if(action->handler(action->irq, action->context, NULL)){
                        /*nothing*/
                    }
#else
                    if(action->handler(action->irq, action->context)){
                        /*nothing*/
                    }
#endif
                }else{
                    desc->irqs_unhandled++;
                }
                do_gettimeofday(&tv_curr);
                desc->irq_duration = ((tv_curr.tv_sec - tv_base.tv_sec) * 1000 * 1000) + (tv_curr.tv_usec - tv_base.tv_usec); 
                if(desc->irq_duration > 2000){
                    ISIL_DBG(ISIL_DBG_WARN, "IRQ %s duration %d us, at %pS\n", desc->name, desc->irq_duration, action->handler);
                }
            }
            chip_clear_irq(chip, chip_irq);
            control->hardware_op->get_interrupt_status(control);
        }
    }
    spin_unlock_irqrestore(&chip->lock, flags);

    return IRQ_HANDLED;
}

static int  isil_interrupt_control_system_interface_init(isil_interrupt_control_t *control, isil_chip_t *chip)
{
    int	ret = ISIL_ERR;
    u32 i;

    if(control != NULL){
        spin_lock_init(&control->lock);
        control->interrupt_source = 0x00000000;
        control->interrupt_source_trigger_mode = 0x00000000;
        control->interrupt_source_assert = 0xffffffff;
        control->chip_interrupt_output_assert = 0x0;
        control->chip_interrupt_status = 0x00000000;
        control->chip = chip;
        /*level mode*/
        chip->io_op->chip_write32(chip, INTERRUPT_SOURCE_TRIGGER_REG_L, (control->interrupt_source_trigger_mode & 0xffff));
        chip->io_op->chip_write32(chip, INTERRUPT_SOURCE_TRIGGER_REG_H, ((control->interrupt_source_trigger_mode>>16) & 0xffff));

        chip->io_op->chip_write32(chip, INTERRUPT_ASSERT_REG_L, (control->interrupt_source_assert & 0xffff));
        chip->io_op->chip_write32(chip, INTERRUPT_ASSERT_REG_H, ((control->interrupt_source_assert>>16) & 0xffff));

        chip->io_op->chip_write32(chip, INTERRUPT_OUTPUT_ASSERT_REG, control->chip_interrupt_output_assert);

        /*init irq describe table*/
        for(i = 0; i < CHIP_IRQS_NR; i++)
        {
            struct chip_irq_desc *desc = chip_irq_to_desc(chip, i);
            desc->irq = i;
            desc->irq_count =0;
            desc->irqs_unhandled = 0;
            spin_lock_init(&desc->lock);
            atomic_set(&desc->depth, 0);
            memset(&desc->action, 0, sizeof(struct chip_irq_action));
            switch(desc->irq)
            {
                case IRQ_VLC_TYPE_INTR:
                    desc->name = "VLC";break;
                case IRQ_BURST_TYPE_INTR:
                    desc->name = "BURST";break;
                case IRQ_MV_TYPE_INTR:
                    desc->name = "MV";break;
                case IRQ_FRONT_END_TYPE_INTR:
                    desc->name = "FRONT_END";break;
                case IRQ_JPEG_TYPE_INTR:
                    desc->name = "JPEG";break;
                case IRQ_TIMER_TYPE_INTR:
                    desc->name = "TIMER";break;
                case IRQ_VLC_DONE_TYPE_INTR:
                    desc->name = "VLC_DONE";break;
                case IRQ_AD_SYNC_TYPE_INTR:
                    desc->name = "AD_SYNC";break;
                case IRQ_AUDIO_EOF_TYPE_INTR:
                    desc->name = "AUDIO";break;
                case IRQ_IIC_DONE_INTR:
                    desc->name = "IIC";break;
                case IRQ_AD_EVENT_INTR:
                    desc->name = "AD_EVENT";break;
                default:
                    desc->name = "UNDEFINE";
            }
            chip_disable_irq(chip, i);
        }
        ret = ISIL_OK;
    }
    return ret;
}

static void isil_interrupt_control_system_interface_enable_interrupt_source(isil_interrupt_control_t *control, u32 source)
{
    if((control != NULL) && (source < CHIP_IRQS_NR)){
        isil_chip_t  *chip;
        unsigned long	flags;
        spin_lock_irqsave(&control->lock, flags);
        chip = control->chip;

        control->interrupt_source &= ~(INTERRUPT_DISABLE<<source);
        control->interrupt_source |= (INTERRUPT_ENABLE<<source);
        if(source < (CHIP_IRQS_NR / 2)){
            chip->io_op->chip_write32(chip, INTERRUPT_SOURCE_ENABLE_REG_L, (control->interrupt_source & 0xffff));
        } else{
            chip->io_op->chip_write32(chip, INTERRUPT_SOURCE_ENABLE_REG_H, ((control->interrupt_source >> 16) & 0xffff));
        }
        spin_unlock_irqrestore(&control->lock, flags);
    }
}

static void isil_interrupt_control_system_interface_disable_interrupt_source(isil_interrupt_control_t *control, u32 source)
{
    if((control != NULL) && (source < CHIP_IRQS_NR)){
        isil_chip_t  *chip;
        unsigned long	flags;
        spin_lock_irqsave(&control->lock, flags);
        chip = control->chip;
        control->interrupt_source &= ~(INTERRUPT_ENABLE<<source);
        control->interrupt_source |= (INTERRUPT_DISABLE<<source);
        if(source < (CHIP_IRQS_NR / 2)){
            chip->io_op->chip_write32(chip, INTERRUPT_SOURCE_ENABLE_REG_L, (control->interrupt_source & 0xffff));
        } else {
            chip->io_op->chip_write32(chip, INTERRUPT_SOURCE_ENABLE_REG_H, ((control->interrupt_source >> 16) & 0xffff));
        }
        spin_unlock_irqrestore(&control->lock, flags);
    }
}

static void isil_interrupt_control_system_interface_set_interrupt_source_edge_trigger(isil_interrupt_control_t *control, u32 source)
{
    if((control != NULL) && (source < CHIP_IRQS_NR)){
        isil_chip_t  *chip;
        unsigned long	flags;
        spin_lock_irqsave(&control->lock, flags);
        chip = control->chip;
        control->interrupt_source_trigger_mode &= ~(LEVEL_TRIGGER<<source);
        control->interrupt_source_trigger_mode |= (EDGE_TRIGGER<<source);
        if(source < (CHIP_IRQS_NR / 2)){
            chip->io_op->chip_write32(chip, INTERRUPT_SOURCE_TRIGGER_REG_L, (control->interrupt_source_trigger_mode & 0xffff));
        } else{
            chip->io_op->chip_write32(chip, INTERRUPT_SOURCE_TRIGGER_REG_H, ((control->interrupt_source_trigger_mode >> 16) & 0xffff));
        }
        spin_unlock_irqrestore(&control->lock, flags);
    }
}

static void isil_interrupt_control_system_interface_set_interrupt_source_level_trigger(isil_interrupt_control_t *control, u32 source)
{
    if((control != NULL) && (source < CHIP_IRQS_NR)){
        isil_chip_t  *chip;
        unsigned long	flags;
        spin_lock_irqsave(&control->lock, flags);
        chip = control->chip;
        control->interrupt_source_trigger_mode &= ~(EDGE_TRIGGER<<source);
        control->interrupt_source_trigger_mode |= (LEVEL_TRIGGER<<source);
        if(source < (CHIP_IRQS_NR / 2)){
            chip->io_op->chip_write32(chip, INTERRUPT_SOURCE_TRIGGER_REG_L, (control->interrupt_source_trigger_mode & 0xffff));
        } else{
            chip->io_op->chip_write32(chip, INTERRUPT_SOURCE_TRIGGER_REG_H, ((control->interrupt_source_trigger_mode >> 16) & 0xffff));
        }
        spin_unlock_irqrestore(&control->lock, flags);
    }
}

static struct isil_interrupt_control_system_interface_operation	isil_interrupt_control_system_interface_op = {
    .init = isil_interrupt_control_system_interface_init,
    .enable_interrupt_source = isil_interrupt_control_system_interface_enable_interrupt_source,
    .disable_interrupt_source = isil_interrupt_control_system_interface_disable_interrupt_source,
    .set_interrupt_source_edge_trigger = isil_interrupt_control_system_interface_set_interrupt_source_edge_trigger,
    .set_interrupt_source_level_trigger = isil_interrupt_control_system_interface_set_interrupt_source_level_trigger,
};

static void isil_interrupt_control_hardware_interface_get_interrupt_status(isil_interrupt_control_t *control)
{
    if(control != NULL){
        isil_chip_t  *chip;
        unsigned long	flags;
        spin_lock_irqsave(&control->lock, flags);
        chip = control->chip;
        control->chip_interrupt_status = (chip->io_op->chip_read32(chip, INTERRUPT_STATUS_REG_L)) & 0xffff;
        control->chip_interrupt_status |= ((chip->io_op->chip_read32(chip, INTERRUPT_STATUS_REG_H) & 0xffff) << 16);
        spin_unlock_irqrestore(&control->lock, flags);
    }
}

static void isil_interrupt_control_hardware_interface_ack_interrupt(isil_interrupt_control_t *control, u32 source)
{
    if((control != NULL) && (source < CHIP_IRQS_NR)){
        isil_chip_t  *chip;
        unsigned long	flags;
        spin_lock_irqsave(&control->lock, flags);
        chip = control->chip;
        control->chip_interrupt_status &= ~(1<<source);
        if(source < (CHIP_IRQS_NR / 2)){
            chip->io_op->chip_write32(chip, INTERRUPT_CLEAR_REG_L, (1<<source));
        }else{
            chip->io_op->chip_write32(chip, INTERRUPT_CLEAR_REG_H, (1<<(source - 16)));
        }
        spin_unlock_irqrestore(&control->lock, flags);
    }
}

struct isil_interrupt_control_hardware_interface_operation    isil_interrupt_control_hardware_interface_op = {
    .get_interrupt_status = isil_interrupt_control_hardware_interface_get_interrupt_status,
    .ack_interrupt = isil_interrupt_control_hardware_interface_ack_interrupt,
};

static int  init_isil_interrupt_control(isil_interrupt_control_t *control, isil_chip_t *chip)
{
    int	ret = ISIL_ERR;

    if((control!=NULL) && (chip!=NULL)){
        control->system_op = &isil_interrupt_control_system_interface_op;
        control->hardware_op = &isil_interrupt_control_hardware_interface_op;
        ret = control->system_op->init(control, chip);
    }
    return ret;
}

static void remove_isil_interrupt_control(isil_interrupt_control_t *control, isil_chip_t *chip)
{
    if((control!=NULL) && (chip!=NULL)){
    }
}

void    disable_chip_int(isil_chip_t *chip)
{
    disable_irq(chip->isil_int_control.chip_irq);
}

void    enable_chip_int(isil_chip_t *chip)
{
    enable_irq(chip->isil_int_control.chip_irq);
}

static int  chip_polling_task(void *context)
{
    isil_chip_t	*chip = (isil_chip_t*)context;
    if(chip != NULL){
        chip_audio_t    *chip_audio;
        isil_chip_vi_driver_t   *chip_vi_driver;
        isil_vj_bus_t *vj_bus;
        isil_vp_bus_t *vp_bus;

        chip_audio = &chip->chip_audio;
        vj_bus = &chip->chip_vj_bus;
        vp_bus = &chip->chip_vp_bus;
        chip_audio->op->process_audio_decode(chip_audio);
        chip_audio->op->process_audio_encode(chip_audio);

        chip_vi_driver = chip->chip_vi_driver;
        chip_vi_driver->op->polling_hw_video_standard(chip_vi_driver);
        vj_bus->op->polling_task(vj_bus);
        vp_bus->op->polling_task(vp_bus);
    }

    return ISIL_OK;
}

/************************************************************************************/
/************************************************************************************/
static const fpga_reg_addr  ForwardQuantizationTable[QUANTIZATION_TABLE_LEN] =
{
    0x3333, 0x1f82, 0x3333, 0x1F82, 0x1F82, 0x147B, 0x1F82, 0x147B, 0x3333, 0x1F82, 
    0x3333, 0x1F82, 0x1F82, 0x147B, 0x1F82, 0x147B, 0x2E8C, 0x1D42, 0x2E8C, 0x1D42,
    0x1D42, 0x1234, 0x1D42, 0x1234, 0x2E8C, 0x1D42, 0x2E8C, 0x1D42, 0x1D42, 0x1234,
    0x1D42, 0x1234, 0x2762, 0x199A, 0x2762, 0x199A, 0x199A, 0x1062, 0x199A, 0x1062,
    0x2762, 0x199A, 0x2762, 0x199A, 0x199A, 0x1062, 0x199A, 0x1062, 0x2492, 0x16C1,
    0x2492, 0x16C1, 0x16C1, 0x0E3F, 0x16C1, 0x0E3F, 0x2492, 0x16C1, 0x2492, 0x16C1,
    0x16C1, 0x0E3F, 0x16C1, 0x0E3F, 0x2000, 0x147B, 0x2000, 0x147B, 0x147B, 0x0D1B,
    0x147B, 0x0D1B, 0x2000, 0x147B, 0x2000, 0x147B, 0x147B, 0x0D1B, 0x147B, 0x0D1B,
    0x1C72, 0x11CF, 0x1C72, 0x11CF, 0x11CF, 0x0B4D, 0x11CF, 0x0B4D, 0x1C72, 0x11CF,
    0x1C72, 0x11CF, 0x11CF, 0x0B4D, 0x11CF, 0x0B4D
};

static const fpga_reg_addr  InverseQuantizationTable[QUANTIZATION_TABLE_LEN] =
{
    0x800A, 0x800D, 0x800A, 0x800D, 0x800D, 0x8010, 0x800D, 0x8010, 0x800A, 0x800D, 
    0x800A, 0x800D, 0x800D, 0x8010, 0x800D, 0x8010, 0x800B, 0x800E, 0x800B, 0x800E,
    0x800E, 0x8012, 0x800E, 0x8012, 0x800B, 0x800E, 0x800B, 0x800E, 0x800E, 0x8012,
    0x800E, 0x8012, 0x800D, 0x8010, 0x800D, 0x8010, 0x8010, 0x8014, 0x8010, 0x8014,
    0x800D, 0x8010, 0x800D, 0x8010, 0x8010, 0x8014, 0x8010, 0x8014, 0x800E, 0x8012,
    0x800E, 0x8012, 0x8012, 0x8017, 0x8012, 0x8017, 0x800E, 0x8012, 0x800E, 0x8012,
    0x8012, 0x8017, 0x8012, 0x8017, 0x8010, 0x8014, 0x8010, 0x8014, 0x8014, 0x8019,
    0x8014, 0x8019, 0x8010, 0x8014, 0x8010, 0x8014, 0x8014, 0x8019, 0x8014, 0x8019,
    0x8012, 0x8017, 0x8012, 0x8017, 0x8017, 0x801D, 0x8017, 0x801D, 0x8012, 0x8017,
    0x8012, 0x8017, 0x8017, 0x801D, 0x8017, 0x801D
};

static const fpga_reg_addr	encoder_vlc_lookup_table[VLC_LOOKUP_TABLE_LEN] = {
    0x011,0x000,0x000,0x000,0x065,0x021,0x000,0x000,0x087,0x064,0x031,0x000,
    0x097,0x086,0x075,0x053,0x0a7,0x096,0x085,0x063,0x0b7,0x0a6,0x095,0x074,
    0x0df,0x0b6,0x0a5,0x084,0x0db,0x0de,0x0b5,0x094,0x0d8,0x0da,0x0dd,0x0a4,
    0x0ef,0x0ee,0x0d9,0x0b4,0x0eb,0x0ea,0x0ed,0x0dc,0x0ff,0x0fe,0x0e9,0x0ec,
    0x0fb,0x0fa,0x0fd,0x0e8,0x10f,0x0f1,0x0f9,0x0fc,0x10b,0x10e,0x10d,0x0f8,
    0x107,0x10a,0x109,0x10c,0x104,0x106,0x105,0x108,0x023,0x000,0x000,0x000,
    0x06b,0x022,0x000,0x000,0x067,0x057,0x033,0x000,0x077,0x06a,0x069,0x045,
    0x087,0x066,0x065,0x044,0x084,0x076,0x075,0x056,0x097,0x086,0x085,0x068,
    0x0bf,0x096,0x095,0x064,0x0bb,0x0be,0x0bd,0x074,0x0cf,0x0ba,0x0b9,0x094,
    0x0cb,0x0ce,0x0cd,0x0bc,0x0c8,0x0ca,0x0c9,0x0b8,0x0df,0x0de,0x0dd,0x0cc,
    0x0db,0x0da,0x0d9,0x0dc,0x0d7,0x0eb,0x0d6,0x0d8,0x0e9,0x0e8,0x0ea,0x0d1,
    0x0e7,0x0e6,0x0e5,0x0e4,0x04f,0x000,0x000,0x000,0x06f,0x04e,0x000,0x000,
    0x06b,0x05f,0x04d,0x000,0x068,0x05c,0x05e,0x04c,0x07f,0x05a,0x05b,0x04b,
    0x07b,0x058,0x059,0x04a,0x079,0x06e,0x06d,0x049,0x078,0x06a,0x069,0x048,
    0x08f,0x07e,0x07d,0x05d,0x08b,0x08e,0x07a,0x06c,0x09f,0x08a,0x08d,0x07c,
    0x09b,0x09e,0x089,0x08c,0x098,0x09a,0x09d,0x088,0x0ad,0x097,0x099,0x09c,
    0x0a9,0x0ac,0x0ab,0x0aa,0x0a5,0x0a8,0x0a7,0x0a6,0x0a1,0x0a4,0x0a3,0x0a2,
    0x021,0x000,0x000,0x000,0x067,0x011,0x000,0x000,0x064,0x066,0x031,0x000,
    0x063,0x073,0x072,0x065,0x062,0x083,0x082,0x070,0x000,0x000,0x000,0x000,
    0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
    0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
    0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
    0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
    0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
    0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
    0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
    0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
    0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
    0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
    0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
    0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
    0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
    0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
    0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
    0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
    0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
    0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
    0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
    0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
    0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
    0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
    0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
    0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
    0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
    0x011,0x010,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
    0x000,0x000,0x000,0x000,0x011,0x021,0x020,0x000,0x000,0x000,0x000,0x000,
    0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x023,0x022,0x021,0x020,
    0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
    0x023,0x022,0x021,0x031,0x030,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
    0x000,0x000,0x000,0x000,0x023,0x022,0x033,0x032,0x031,0x030,0x000,0x000,
    0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x023,0x030,0x031,0x033,
    0x032,0x035,0x034,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
    0x037,0x036,0x035,0x034,0x033,0x032,0x031,0x041,0x051,0x061,0x071,0x081,
    0x091,0x0a1,0x0b1,0x000,0x002,0x000,0x0e4,0x011,0x0f4,0x002,0x024,0x003,
    0x005,0x012,0x034,0x013,0x065,0x024,0x013,0x063,0x015,0x022,0x075,0x034,
    0x044,0x023,0x023,0x073,0x054,0x033,0x033,0x004,0x043,0x014,0x011,0x043,
    0x014,0x001,0x025,0x015,0x035,0x025,0x064,0x055,0x045,0x035,0x074,0x065,
    0x085,0x0d5,0x012,0x095,0x055,0x045,0x095,0x0e5,0x084,0x075,0x022,0x0a5,
    0x094,0x085,0x032,0x0b5,0x003,0x0c5,0x001,0x044,0x0a5,0x032,0x0b5,0x094,
    0x0c5,0x0a4,0x0a4,0x054,0x0d5,0x0b4,0x0b4,0x064,0x0f5,0x0f5,0x053,0x0d4,
    0x0e5,0x0c4,0x105,0x105,0x0c4,0x074,0x063,0x0e4,0x0d4,0x084,0x073,0x0f4,
    0x004,0x005,0x000,0x053,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
    0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
    0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
    0x000,0x000,0x000,0x000,0x011,0x021,0x031,0x030,0x011,0x021,0x020,0x000,
    0x011,0x010,0x000,0x000,0x011,0x033,0x032,0x043,0x042,0x053,0x052,0x063,
    0x062,0x073,0x072,0x083,0x082,0x093,0x092,0x091,0x037,0x036,0x035,0x034,
    0x033,0x045,0x044,0x043,0x042,0x053,0x052,0x063,0x062,0x061,0x060,0x000,
    0x045,0x037,0x036,0x035,0x044,0x043,0x034,0x033,0x042,0x053,0x052,0x061,
    0x051,0x060,0x000,0x000,0x053,0x037,0x045,0x044,0x036,0x035,0x034,0x043,
    0x033,0x042,0x052,0x051,0x050,0x000,0x000,0x000,0x045,0x044,0x043,0x037,
    0x036,0x035,0x034,0x033,0x042,0x051,0x041,0x050,0x000,0x000,0x000,0x000,
    0x061,0x051,0x037,0x036,0x035,0x034,0x033,0x032,0x041,0x031,0x060,0x000,
    0x000,0x000,0x000,0x000,0x061,0x051,0x035,0x034,0x033,0x023,0x032,0x041,
    0x031,0x060,0x000,0x000,0x000,0x000,0x000,0x000,0x061,0x041,0x051,0x033,
    0x023,0x022,0x032,0x031,0x060,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
    0x061,0x060,0x041,0x023,0x022,0x031,0x021,0x051,0x000,0x000,0x000,0x000,
    0x000,0x000,0x000,0x000,0x051,0x050,0x031,0x023,0x022,0x021,0x041,0x000,
    0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x040,0x041,0x031,0x032,
    0x011,0x033,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
    0x040,0x041,0x021,0x011,0x031,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
    0x000,0x000,0x000,0x000,0x030,0x031,0x011,0x021,0x000,0x000,0x000,0x000,
    0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x020,0x021,0x011,0x000,
    0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
    0x010,0x011,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
    0x000,0x000,0x000,0x000
};

static void WriteEncodeVLCLookupTable(isil_chip_t *chip)
{
    int i;

    chip->io_op->chip_write32(chip, ENABLE_VLC_LOOKUP_TABLE, 0x1);
    for(i=0; i<VLC_LOOKUP_TABLE_LEN; i++){
        chip->io_op->chip_write32(chip, (ENCODE_VLC_LOOKUP_TABLE_BASE+(i<<2)), encoder_vlc_lookup_table[i]);
    }
    chip->io_op->chip_write32(chip, ENABLE_VLC_LOOKUP_TABLE, 0x0);
}

static void WriteForwardQuantizationTable(isil_chip_t *chip)
{
    int i;

    for (i=0; i<QUANTIZATION_TABLE_LEN; i++) {
        chip->io_op->chip_write32(chip, (QUANTIZATION+(i<<2)), ForwardQuantizationTable[i]);
    }
}

static void WriteInverseQuantizationTable(isil_chip_t *chip)
{
    int i;

    for (i=0; i<QUANTIZATION_TABLE_LEN; i++) {
        chip->io_op->chip_write32(chip, (QUANTIZATION+(i<<2)), InverseQuantizationTable[i]);
    }
}

void    start_chip_robust_process(isil_chip_t *chip)
{
    isil_kthread_msg_queue_t    *isil_msg_queue;
    robust_processing_control_t *chip_robust;

    chip_robust = &chip->chip_robust_processing;
    chip_robust->op->start_robust_processing(chip_robust);
    get_msg_queue_header(&isil_msg_queue);
    delete_all_msg_of_this_chip(isil_msg_queue, chip);
}

void    chip_robust_process_done(isil_chip_t *chip)
{
    robust_processing_control_t *chip_robust;

    chip_robust = &chip->chip_robust_processing;
    chip_robust->op->robust_processing_done(chip_robust);
}

void    chip_wait_robust_process_done(isil_chip_t *chip)
{
    robust_processing_control_t *chip_robust_processing;
    chip_robust_processing = &chip->chip_robust_processing;
    chip_robust_processing->op->wait_robust_processing_done(chip_robust_processing);
}

int chip_is_in_robust_processing(isil_chip_t *chip)
{
    robust_processing_control_t *chip_robust_processing;
    chip_robust_processing = &chip->chip_robust_processing;
    return chip_robust_processing->op->is_in_robust_processing(chip_robust_processing);
}

static int  chip_hardware_resource_open(isil_chip_t *chip)
{
    int ret = ISIL_ERR;

    atomic_inc(&chip->open_chan_number);
    /*if(atomic_read(&chip->open_chan_number) == 1){
        chip->chip_set_valid(chip);
    }*/
    ISIL_DBG(ISIL_DBG_INFO, "chip->open_chan_number = %d\n", atomic_read(&chip->open_chan_number));
    ret = ISIL_OK;
    return ret;
} 

static void chip_hardware_resource_close(isil_chip_t *chip)
{
    ISIL_DBG(ISIL_DBG_INFO, "chip->open_chan_number = %d\n", atomic_read(&chip->open_chan_number));
    if(atomic_read(&chip->open_chan_number) > 0){
        if(atomic_dec_and_test(&chip->open_chan_number)){
            /*chip->chip_set_invalid(chip);*/
        }
    } else {
        printk("%s.%d: open_chan_number err\n", __FUNCTION__, __LINE__);
    }
}

static int  timer_on_chip_handler(int irq, void *context)
{
    if(context) {
        u32 flags;
        isil_chip_t *chip = (isil_chip_t *)context;
        isil_chip_timer_controller_t    *chip_timer_cont;

        flags = chip->io_op->chip_read32(chip, INTERRUPT_FLAGS_EXT);
        if(flags & (1 << IRQ_EXT_TIMER_INTR)) {
            chip_timer_cont = &chip->chip_timer_cont;
            chip_timer_cont->op->isr(chip_timer_cont);
            /*clear timer pending*/
            chip->io_op->chip_write32(chip, INTERRUPT_FLAGS_EXT, 1 << IRQ_EXT_TIMER_INTR);
        }
    }

    return 1;
}

static void register_on_chip_timer(isil_chip_t *chip, isil_irq_handler_t handler)
{
    if(chip && handler) {
        if(chip_request_irq(chip, IRQ_TIMER_TYPE_INTR, timer_on_chip_handler, "timer", chip)){
            printk("%s ,%d: register system timer failed\n", __FUNCTION__, __LINE__);
            return;
        }
        chip->io_op->chip_write32(chip, PCI_TIMER_CONTROL, 0x2);/*default is 0->1ms, 1->2ms, 2->4ms, 3->8ms*/
        chip->enable_timer(chip);
    }
}

static void enable_on_chip_timer(isil_chip_t *chip)
{
    if(chip) {
        u32 val;

        val = chip->io_op->chip_read32(chip, PCI_MASTER_CONTROL);
        val |= (1 << IRQ_EXT_TIMER_INTR);
        chip->io_op->chip_write32(chip, PCI_MASTER_CONTROL, val);
    }
}

static void disable_on_chip_timer(isil_chip_t *chip)
{
    if(chip) {
        u32 val;

        val = chip->io_op->chip_read32(chip, PCI_MASTER_CONTROL);
        val &= ~(1 << IRQ_EXT_TIMER_INTR);
        chip->io_op->chip_write32(chip, PCI_MASTER_CONTROL, val);
    }
}

static void unregister_on_chip_timer(isil_chip_t *chip)
{
    if(chip) {
        chip->disable_timer(chip);
        chip_free_irq(chip, IRQ_TIMER_TYPE_INTR, chip);
    }
}

static void chip_phy_invalid(isil_chip_t *chip)
{
    u32 val;
    isil_interrupt_control_t  *isil_int_control;
    isil_chip_vi_driver_t *chip_vi_driver;

    ISIL_DBG(ISIL_DBG_INFO, "chip->open_chan_number = %d\n", atomic_read(&chip->open_chan_number));
    chip_vi_driver = chip->chip_vi_driver;
    isil_int_control = &chip->isil_int_control;

    if(chip->delete_chip_polling_task != NULL){
        chip->delete_chip_polling_task(chip);
    }
#ifdef USE_ON_CHIP_TIMER
    if(chip->unregister_timer) {
        chip->unregister_timer(chip);
    }
#endif

    if(isil_int_control->chip_irq_register_flags){
        free_irq(isil_int_control->chip_irq, isil_int_control);
        isil_int_control->chip_irq_register_flags = 0;
    }

#ifndef FPGA_330_5864_TESTING
    chip->io_op->chip_write32(chip, 0x8018, 0xd000 + ((chip->sys_clock << 1) - 1));
    chip->io_op->chip_write32(chip, 0x8020, 0x284);
    chip->io_op->chip_write32(chip, 0x8024, 0x20);
    chip->io_op->chip_write32(chip, 0x801c, 0x0d);
    mdelay(10);
    /*reset*/
    mpb_write(chip, ISIL_VI_SYSTEM_RESET, 0x00);/*reset*/
    mpb_write(chip, ISIL_VI_SYSTEM_RESET, 0xe0);/*unreset*/
    mdelay(10);
    mpb_write(chip, ISIL_VI_SYSTEM_CLOCK, 0xf0);
    mpb_write(chip, ISIL_VI_SYSTEM_CLOCK_REVERSE, 0xf0);/*155 set to 0xff, 330 set to 0xf0*/

#if (defined(PCI_PLATFORM) && defined(DMA_INTERFACE) && defined(BURST_INTERFACE))
    chip->io_op->chip_write32(chip, MPI_CS_SETUP, 0x1);//set to 0x2 when read ping pong buffer;other set to 0x1
    chip->io_op->chip_write32(chip, MPI_RDY_HOLD, 0x3);
#else
    chip->io_op->chip_write32(chip, MPI_CS_SETUP, 0x1);//set to 0x2 when read ping pong buffer;other set to 0x1
    chip->io_op->chip_write32(chip, MPI_RDY_HOLD, 0x1);
#endif
#endif
    mdelay(10);

#ifndef FPGA_330_5864_TESTING
    if(chip->sys_clock > 110) {
        chip->io_op->chip_write32(chip, 0xa000, 0xc5);//Data valid counter after read command to DDR
        chip->io_op->chip_write32(chip, 0xa800, 0xc5);
    }
    /*enable i2c controler*/
    val = chip->io_op->chip_read32(chip, MCU_IIC_CONF);
    val |= 0x01;
    chip->io_op->chip_write32(chip, MCU_IIC_CONF, val);
    chip->io_op->chip_write32(chip, I2C_PHASE_SHIFT, 0x01);

    /*reset font-end*/
    if(chip_vi_driver && chip_vi_driver->op) {
        chip_vi_driver->op->reset(chip_vi_driver);
    }

    /*confiure ad output 108 clock*/
    chip->io_op->chip_write32(chip, 0x8028, 0x15);
    chip->io_op->chip_write32(chip, 0x801c, 0x18);
    mdelay(5);
    chip->io_op->chip_write32(chip, 0x801c, 0x00);
#endif
}

static void	chip_phy_valid(isil_chip_t *chip)
{
    u32 val;
    int ret;
    chip_driver_t  *chip_driver;
    isil_interrupt_control_t  *isil_int_control;


    ISIL_DBG(ISIL_DBG_INFO, "chip->open_chan_number = %d\n", atomic_read(&chip->open_chan_number));
    //printk("\n%s.%d: chip->open_chan_number = %d\n", __FUNCTION__, __LINE__, atomic_read(&chip->open_chan_number));
    chip_driver = chip->chip_driver;
    isil_int_control = &chip->isil_int_control;

    /*do not reset*/
    chip->chip_set_invalid(chip);
    chip->io_op->chip_write32(chip, INTRAENABLE, 0);
    chip->io_op->chip_write32(chip, INTRANDMECONTROL, 0);
    chip->io_op->chip_write32(chip, WINDOWSIZE, SEARCH_WINDOW_SIZE_VALUE);
    chip->io_op->chip_write32(chip, REFERENCEFRAME, 0x0440);
    chip->io_op->chip_write32(chip, ORIGINALSEQUENCE, 0x0440);
    chip->io_op->chip_write32(chip, INTRA4X4_THRESHOLD_I, 0x18);
    chip->io_op->chip_write32(chip, INTRA4X4_THRESHOLD_IP, 0x0);
    chip->io_op->chip_write32(chip, CHIPSCOPE, 0x0070);
    chip->io_op->chip_write32(chip, 0x0008, 0x0000);
    chip->io_op->chip_write32(chip, MODE, (DAON|DDRON|CMOSON|INTRAON|INTERON|DEBLOCKINGON));
    chip->io_op->chip_write32(chip, 0x0008, 0x0800);
    chip->io_op->chip_write32(chip, CHANNEL_SELECT, 0);
    chip->io_op->chip_write32(chip, CHANNEL_ID, 0);

    WriteForwardQuantizationTable(chip);
    WriteInverseQuantizationTable(chip);
    WriteEncodeVLCLookupTable(chip);
    ISIL_DBG(ISIL_DBG_INFO, "\nFPGA version: %x:%x, chip_id=%d\n", chip->io_op->chip_read32(chip, 0), chip->io_op->chip_read32(chip, 8), chip->chip_id);

    /*enable i2c controler*/
    val = chip->io_op->chip_read32(chip, MCU_IIC_CONF);
    val |= 0x01;
    chip->io_op->chip_write32(chip, MCU_IIC_CONF, val);
    chip->io_op->chip_write32(chip, I2C_PHASE_SHIFT, 0x01);
    if(chip_driver && chip_driver->op){
        chip_driver->op->reset(chip_driver);
    }

    if(chip->add_chip_polling_task != NULL){
        chip->add_chip_polling_task(chip);
    }
#ifdef USE_ON_CHIP_TIMER
    if(chip->register_timer) {
        chip->register_timer(chip, timer_on_chip_handler);
    }
#endif

#if defined(PCI_PLATFORM) && defined(DMA_INTERFACE) && defined(BURST_INTERFACE)
    /*enable burst*/
    val = chip->io_op->chip_read32(chip, PCI_MASTER_CONTROL);
    val |= (0x7 << 26);
    chip->io_op->chip_write32(chip, PCI_MASTER_CONTROL, val);
    /*val = chip->io_op->chip_read32(chip, DDRPAGE);
      val |= (1<<13) | (1 << 15);
      chip->io_op->chip_write32(chip, DDRPAGE, val);*/
#else
    /*disable burst*/
    val = chip->io_op->chip_read32(chip, PCI_MASTER_CONTROL);
    val &= ~(0x7 << 26);
    chip->io_op->chip_write32(chip, PCI_MASTER_CONTROL, val);
    /*val = chip->io_op->chip_read32(chip, DDRPAGE);
      val &= ~((1<<13) | (1 << 15));
      chip->io_op->chip_write32(chip, DDRPAGE, val);*/
#endif

#if defined(X86_PLATFORM) || defined(NXP_PLATFORM)
    ret = request_irq(isil_int_control->chip_irq, 
            isil_top_isr, IRQF_SHARED, chip->name, isil_int_control);
#else
    ret = request_irq(isil_int_control->chip_irq, 
            isil_top_isr, IRQF_DISABLED, chip->name, isil_int_control);
#endif
    if(ret) {
        printk("%s,%d:install irq failed!, %d\n", __FILE__, __LINE__, ret);
    } else {
        isil_int_control->chip_irq_register_flags = 1;    
    }
}

static void chip_phy_reset(isil_chip_t *chip)
{
    if(chip) {
        chip_driver_t       *chip_driver = chip->chip_driver;
        isil_chip_ai_driver_t *chip_ai_driver = chip->chip_ai_driver;
        isil_chip_vi_driver_t *chip_vi_driver = chip->chip_vi_driver;

        if(chip_ai_driver && chip_ai_driver->op) {
            chip_ai_driver->op->reset(chip_ai_driver);
        }
        if(chip_vi_driver && chip_vi_driver->op) {
            chip_vi_driver->op->reset(chip_vi_driver);
        }
        if(chip_driver && chip_driver->op) {
            chip_driver->op->reset(chip_driver);
        }
    }
}

static void add_chip_polling_task(isil_chip_t *chip)
{
    if(chip != NULL){
        isil_chip_timer_controller_t    *chip_timer_cont = &chip->chip_timer_cont;
        chip_timer_cont->op->DeleteForFireTimerJob(chip_timer_cont, chip->chip_polling_timeid);
        chip->chip_polling_timeid = INVALIDTIMERID;
        chip->chip_polling_timeid = chip_timer_cont->op->AddForFireTimerJob(chip_timer_cont, 1, chip_polling_task, chip);
        if(chip->chip_polling_timeid == ADDJOBERROR){
            printk("%s.%d: add timer hook err\n", __FUNCTION__, __LINE__);
        }
    }
}

static void delete_chip_polling_task(isil_chip_t *chip)
{
    if(chip != NULL){
        isil_chip_timer_controller_t    *chip_timer_cont = &chip->chip_timer_cont;
        chip_timer_cont->op->DeleteForFireTimerJob(chip_timer_cont, chip->chip_polling_timeid);
        chip->chip_polling_timeid = INVALIDTIMERID;
    }
}

static void register_curr_h264_encode_chan(isil_chip_t *chip, isil_h264_logic_encode_chan_t *h264_logic_encode_chan)
{
    if((chip!=NULL) && (h264_logic_encode_chan!=NULL)){
        chip->vlc_ping_pong_index = 0;
        chip_request_irq(chip, IRQ_VLC_TYPE_INTR, h264_logic_encode_chan->encode_control.op->irq_func, "VLC", (void*)&h264_logic_encode_chan->encode_control);
    }
}

static void	unregister_curr_h264_encode_chan(isil_chip_t *chip, isil_h264_logic_encode_chan_t *h264_logic_encode_chan)
{
    if((chip!=NULL) && (h264_logic_encode_chan!=NULL)){
        chip_free_irq(chip, IRQ_VLC_TYPE_INTR, &h264_logic_encode_chan->encode_control);
    }
}

static void register_curr_h264_encode_push_chan(isil_chip_t *chip, isil_h264_logic_encode_chan_t *h264_logic_encode_chan)
{
    if((chip!=NULL) && (h264_logic_encode_chan!=NULL)){
        chip_request_irq(chip, IRQ_VLC_DONE_TYPE_INTR, h264_logic_encode_chan->encode_control.op->irq_func, "VLC PUSH", (void*)&h264_logic_encode_chan->encode_control);
    }
}

static void	unregister_curr_h264_encode_push_chan(isil_chip_t *chip, isil_h264_logic_encode_chan_t *h264_logic_encode_chan)
{
    if((chip!=NULL) && (h264_logic_encode_chan!=NULL)){
        chip_free_irq(chip, IRQ_VLC_DONE_TYPE_INTR, &h264_logic_encode_chan->encode_control);
    }
}


static void chip_encode_chan_service_queue_init(chip_encode_chan_service_queue_t *chip_encode_service_queue)
{
    if(chip_encode_service_queue != NULL){
        tcb_node_queue_t    *queue_node = &chip_encode_service_queue->service_queue_node;
        spin_lock_init(&chip_encode_service_queue->lock);
        atomic_set(&chip_encode_service_queue->h264_master_can_recv_number, 2);
        atomic_set(&chip_encode_service_queue->h264_ddr_cache_number, 8);
        queue_node->op = &tcb_node_queue_op;
        queue_node->op->init(queue_node);
        chip_encode_service_queue->curr_consumer = NULL;
    }
}

static int  chip_encode_chan_service_queue_get_queue_curr_entry_number(chip_encode_chan_service_queue_t *chip_encode_service_queue)
{
    int ret = 0;
    if(chip_encode_service_queue != NULL){
        tcb_node_queue_t    *queue_node = &chip_encode_service_queue->service_queue_node;
        ret = queue_node->op->get_queue_curr_entry_number(queue_node);
    }
    return ret;
}

static void chip_encode_chan_service_queue_put_service_request_into_queue(chip_encode_chan_service_queue_t *chip_encode_service_queue, encode_chan_service_tcb_t *chan_service_tcb)
{
    if((chip_encode_service_queue!=NULL) && (chan_service_tcb!=NULL)){
        tcb_node_queue_t    *queue_node = &chip_encode_service_queue->service_queue_node;
        queue_node->op->put(queue_node, &chan_service_tcb->service_tcb);
        chip_encode_service_queue->op->trigger_chip_pending_service_request(chip_encode_service_queue);
    }
}

static void chip_encode_chan_service_queue_put_service_request_into_queue_header(chip_encode_chan_service_queue_t *chip_encode_service_queue, encode_chan_service_tcb_t *chan_service_tcb)
{
    if((chip_encode_service_queue!=NULL) && (chan_service_tcb!=NULL)){
        tcb_node_queue_t    *queue_node = &chip_encode_service_queue->service_queue_node;
        queue_node->op->put_header(queue_node, &chan_service_tcb->service_tcb);
        chip_encode_service_queue->op->trigger_chip_pending_service_request(chip_encode_service_queue);
    }
}

static int  chip_encode_chan_service_queue_delete_service_request_from_queue(chip_encode_chan_service_queue_t *chip_encode_service_queue, encode_chan_service_tcb_t *chan_service_tcb)
{
    int ret = 0;
    if((chip_encode_service_queue!=NULL) && (chan_service_tcb!=NULL)){
        tcb_node_queue_t    *queue_node = &chip_encode_service_queue->service_queue_node;
        unsigned long       flags;
        spin_lock_irqsave(&chip_encode_service_queue->lock, flags);
        ret = queue_node->op->delete(queue_node, &chan_service_tcb->service_tcb);
        if(ret == 1){
            if(chan_service_tcb->delete_req_notify != NULL){
                chan_service_tcb->delete_req_notify(chan_service_tcb->context);
            }
        } 
        spin_unlock_irqrestore(&chip_encode_service_queue->lock, flags);
    }
    return ret;
}

static void chip_encode_chan_service_queue_try_get_curr_consumer_from_queue(chip_encode_chan_service_queue_t *chip_encode_service_queue)
{
    if(chip_encode_service_queue != NULL){
        tcb_node_queue_t    *queue_node = &chip_encode_service_queue->service_queue_node;

        if(chip_encode_service_queue->curr_consumer == NULL){
            if(queue_node->op != NULL){
                tcb_node_t  *temp_node;
                queue_node->op->try_get(queue_node, &temp_node);
                if(temp_node != NULL){
                    chip_encode_service_queue->curr_consumer = to_get_encode_chan_service_tcb_with_service_tcb(temp_node);
                }
            }
        }
    }
}

static void chip_encode_chan_service_queue_release_curr_consumer(chip_encode_chan_service_queue_t *chip_encode_service_queue)
{
    if(chip_encode_service_queue != NULL){
        unsigned long	flags;
        spin_lock_irqsave(&chip_encode_service_queue->lock, flags);
        chip_encode_service_queue->curr_consumer = NULL;
        spin_unlock_irqrestore(&chip_encode_service_queue->lock, flags);
    }
}

static void chip_encode_chan_service_queue_trigger_chip_pending_service_request(chip_encode_chan_service_queue_t *chip_encode_service_queue)
{
    if(chip_encode_service_queue != NULL){
        isil_chip_t *chip = to_get_chip_with_chip_encode_service_queue(chip_encode_service_queue);
        encode_chan_service_tcb_t   *curr_consumer;
        unsigned long	flags;
        int curr_entry_number;

again:
        spin_lock_irqsave(&chip_encode_service_queue->lock, flags);
        curr_entry_number = chip_encode_service_queue->op->get_queue_curr_entry_number(chip_encode_service_queue);
        if((curr_entry_number==0) && (chip_encode_service_queue->curr_consumer==NULL)){
            spin_unlock_irqrestore(&chip_encode_service_queue->lock, flags);
            return;
        } else if(chip_encode_service_queue->curr_consumer != NULL){
            spin_unlock_irqrestore(&chip_encode_service_queue->lock, flags);
            return;
        }
        chip_encode_service_queue->op->try_get_curr_consumer_from_queue(chip_encode_service_queue);
        curr_consumer = chip_encode_service_queue->curr_consumer;
        spin_unlock_irqrestore(&chip_encode_service_queue->lock, flags);

        if(curr_consumer != NULL){
            if(chip_is_in_robust_processing(chip)){
                if(curr_consumer->delete_req_notify != NULL){
                    curr_consumer->delete_req_notify(curr_consumer->context);
                }
                chip_encode_service_queue->op->release_curr_consumer(chip_encode_service_queue);
                goto again;
            } else {
                if(curr_consumer->req_callback != NULL){
                    curr_consumer->req_callback(curr_consumer->context);                
                } else {
                    if(curr_consumer->delete_req_notify != NULL){
                        curr_consumer->delete_req_notify(curr_consumer->context);
                    }
                    chip_encode_service_queue->op->release_curr_consumer(chip_encode_service_queue);
                    printk("\n\n&&&&&&&&&&&&%s.%d&&&&&&&&&&&&&,%d\n\n", __FUNCTION__, __LINE__, curr_entry_number);
                    goto again;
                }

                spin_lock_irqsave(&chip_encode_service_queue->lock, flags);
                if(curr_consumer->type == DVM_CHIP_REQ_FIRST_STRAT_CHAN){
                    chip_encode_service_queue->curr_consumer = NULL;
                    spin_unlock_irqrestore(&chip_encode_service_queue->lock, flags);
                    goto again;
                } else {
                    spin_unlock_irqrestore(&chip_encode_service_queue->lock, flags);
                }
            }
        }
    }
}

struct chip_encode_chan_service_queue_operation chip_encode_chan_service_queue_op = {
    .init = chip_encode_chan_service_queue_init,
    .get_queue_curr_entry_number = chip_encode_chan_service_queue_get_queue_curr_entry_number,
    .put_service_request_into_queue = chip_encode_chan_service_queue_put_service_request_into_queue,
    .put_service_request_into_queue_header = chip_encode_chan_service_queue_put_service_request_into_queue_header,
    .delete_service_request_from_queue = chip_encode_chan_service_queue_delete_service_request_from_queue,
    .try_get_curr_consumer_from_queue = chip_encode_chan_service_queue_try_get_curr_consumer_from_queue,
    .release_curr_consumer = chip_encode_chan_service_queue_release_curr_consumer,
    .trigger_chip_pending_service_request = chip_encode_chan_service_queue_trigger_chip_pending_service_request,
};

static void init_chip_encode_chan_service_queue(chip_encode_chan_service_queue_t * chip_encode_service_queue)
{
    if(chip_encode_service_queue != NULL){
        chip_encode_service_queue->op = &chip_encode_chan_service_queue_op;
        chip_encode_service_queue->op->init(chip_encode_service_queue);
    }
}

static u32  isil_host_io_chip_read32(isil_chip_t *chip, u32 offset)
{
    return __raw_readl((chip->regs+offset));
}

static u16  isil_host_io_chip_read16(isil_chip_t *chip, u32 offset)
{
    return __raw_readw((chip->regs+offset));
}

static u8  isil_host_io_chip_read8(isil_chip_t *chip, u32 offset)
{
    return __raw_readb((chip->regs+offset));
}

static void  isil_host_io_chip_read_block(isil_chip_t *chip, u32 offset, int len, u32 *dest)
{
    int i;
    len += 3;
    len >>= 2;
    for(i=0; i<len; i++){
        dest[i] = __raw_readl((chip->regs+offset));
        offset += 4;
    }
}

static void isil_host_io_chip_write32(isil_chip_t *chip, u32 offset, u32 value)
{
    __raw_writel(value, (chip->regs+offset));
#if 0
    if(offset < 0x2000){
        printk("264: %x=0x%x, 0x%x\n", offset, value, __raw_readl(chip->regs+offset));
    } else if(offset < 0x3000){
        //printk("bitstream: %x=0x%x, 0x%x\n", offset, value, __raw_readl(chip->regs+offset));
    } else if(offset < 0x4000){
        printk("reserver space %x\n", offset);
    } else if(offset < 0x5000){
        printk("audio: %x=0x%x, 0x%x\n", offset, value, __raw_readl(chip->regs+offset));
    } else if(offset < 0x802c){
        printk("system_config: %x=0x%x, 0x%x\n", offset, value, __raw_readl(chip->regs+offset));
    } else if(offset < 0x8800){
        printk("reserver space %x\n", offset);
    } else if(offset < 0x8900){
        printk("Int: %x=0x%x, 0x%x\n", offset, value, __raw_readl(chip->regs+offset));
    } else if(offset < 0x9000){
        printk("reserver space %x\n", offset);
    } else if(offset < 0x9210){
        printk("VIF: %x=0x%x, 0x%x\n", offset, value, __raw_readl(chip->regs+offset));
    } else if(offset < 0xa000){
        printk("reserver space %x\n", offset);
    } else if(offset < 0xa900){
        printk("DDR: %x=0x%x, 0x%x\n", offset, value, __raw_readl(chip->regs+offset));
    } else if(offset < 0xb800){
        printk("reserver space %x\n", offset);
    } else if(offset < 0xb810){
        printk("IAR: %x=0x%x, 0x%x\n", offset, value, __raw_readl(chip->regs+offset));
    } else if(offset < 0xc000){
        printk("reserver space %x\n", offset);
    } else if(offset < 0xc800){
        printk("preview: %x=0x%x, 0x%x\n", offset, value, __raw_readl(chip->regs+offset));
    } else if(offset < 0xc808){
        printk("MJPEG_Capture: %x=0x%x, 0x%x\n", offset, value, __raw_readl(chip->regs+offset));
    } else if(offset < 0xd000){
        printk("reserver space %x\n", offset);
    } else if(offset < 0xd100){
        printk("MJPEG_Control: %x=0x%x, 0x%x\n", offset, value, __raw_readl(chip->regs+offset));
    } else if(offset < 0xe000){
        printk("reserver space %x\n", offset);
    } else if(offset < 0xfc00){
        printk("Motion_vector: %x=0x%x, 0x%x\n", offset, value, __raw_readl(chip->regs+offset));
    } else if(offset < 0x18000){
        printk("reserver space %x\n", offset);
    } else if(offset < 0x18200){
        printk("PCI_Control_map: %x=0x%x, 0x%x\n", offset, value, __raw_readl(chip->regs+offset));
    } else if(offset < 0x80000){
        printk("reserver space %x\n", offset);
    } else if(offset < 0x88000){
        printk("ddr_burst_map: %x=0x%x, 0x%x\n", offset, value, __raw_readl(chip->regs+offset));
    } else {
        printk("reserver space %x\n", offset);    
    }
#endif
}

static void isil_host_io_chip_write16(isil_chip_t *chip, u32 offset, u16 value)
{
    __raw_writew(value, (chip->regs+offset));
}

static void isil_host_io_chip_write8(isil_chip_t *chip, u32 offset, u8 value)
{
    __raw_writeb(value, (chip->regs+offset));
}

static void isil_host_io_chip_write_block(isil_chip_t *chip, u32 offset, int len, u32 *src)
{
    int i;
    len += 3;
    len >>= 2;
    for(i=0; i<len; i++){
        __raw_writel(src[i], (chip->regs+offset));
        offset += 4;
    }
}

static struct chip_io_operation isil_host_io = {
    .chip_read32 = isil_host_io_chip_read32,
    .chip_read16 = isil_host_io_chip_read16,
    .chip_read8 = isil_host_io_chip_read8,
    .chip_read_block = isil_host_io_chip_read_block,
    .chip_write32 = isil_host_io_chip_write32,
    .chip_write16 = isil_host_io_chip_write16,
    .chip_write8 = isil_host_io_chip_write8,
    .chip_write_block = isil_host_io_chip_write_block,
};

#ifdef POWERPC_PLATFORM
static u32  isil_pci_io_chip_read32(isil_chip_t *chip, u32 offset)
{
    return in_le32((chip->regs+offset));
}

static u16  isil_pci_io_chip_read16(isil_chip_t *chip, u32 offset)
{
    return in_le16((chip->regs+offset));
}

static u8  isil_pci_io_chip_read8(isil_chip_t *chip, u32 offset)
{
    return in_8((chip->regs+offset));
}

static void  isil_pci_io_chip_read_block(isil_chip_t *chip, u32 offset, int len, u32 *dest)
{
    int i;

    len += 3;
    len >>= 2;
    for(i=0; i<len; i++){
        dest[i] = in_le32((chip->regs+offset));
        offset += 4;
    }
}

static void isil_pci_io_chip_write32(isil_chip_t *chip, u32 offset, u32 value)
{
    out_le32((chip->regs+offset), value);
}

static void isil_pci_io_chip_write16(isil_chip_t *chip, u32 offset, u16 value)
{
    out_le16((chip->regs+offset), value);
}

static void isil_pci_io_chip_write8(isil_chip_t *chip, u32 offset, u8 value)
{
    out_8((chip->regs+offset), value);
}

static void isil_pci_io_chip_write_block(isil_chip_t *chip, u32 offset, int len, u32 *src)
{
    int i;

    len += 3;
    len >>= 2;
    for(i=0; i<len; i++){
        out_le32((chip->regs+offset), src[i]);
        offset += 4;
    }
}

static struct chip_io_operation isil_pci_io = {
    .chip_read32 = isil_pci_io_chip_read32,
    .chip_read16 = isil_pci_io_chip_read16,
    .chip_read8 = isil_pci_io_chip_read8,
    .chip_read_block = isil_pci_io_chip_read_block,
    .chip_write32 = isil_pci_io_chip_write32,
    .chip_write16 = isil_pci_io_chip_write16,
    .chip_write8 = isil_pci_io_chip_write8,
    .chip_write_block = isil_pci_io_chip_write_block,
};
#endif

static int chip_proc_read(struct seq_file *seq, void *data)
{
    u32 i;
    DVMNVS_CHIP_VIDEO_ENCODE_CAPABILITY *video_encode_cap;

    isil_chip_t *chip = (isil_chip_t *)seq->private;

    video_encode_cap = chip->video_encode_cap;

    seq_printf(seq, "Version: %d.%d.%d-%d\nGCC Version: %s\nARCH: %s\nBuild at: %s:%s\n", 
            get_version_major(__ISIL__CODE__VERSION__),
            get_version_submajor(__ISIL__CODE__VERSION__),
            get_version_minor(__ISIL__CODE__VERSION__), 
            __ISIL_SVN_VERSION__,__VERSION__, __ISIL_ARCH__ , __DATE__, __TIME__);
    seq_printf(seq, "device name \"%s\", bus %d, chip_id %d, sys_clk = %dMhz, map_mode = %d\n", chip->name, chip->bus_id, chip->chip_id, chip->sys_clock, video_encode_cap->ddr_map_mode);
    seq_printf(seq, "base: 0x%08x, size: 0x%08x\n", video_encode_cap->fpga_async_base_addr, video_encode_cap->fpga_async_section_len);

    seq_printf(seq, "\n");
    seq_printf(seq, "%8s %8s %8s %8s %8s %8s %8s %8s\n",
            "irq", "name", "n_irq", "lost", "depth", "irq_base", "irq_delta", "duration(us)");
    for(i = 0; i < CHIP_IRQS_NR; i++)
    {
        struct chip_irq_desc *desc = chip_irq_to_desc(chip, i);
        if(desc && desc->action.handler){
            seq_printf(seq, "%8d %8s %8u %8u %8d %8u %8u %8u\n",
                    desc->irq, desc->name, desc->irq_count,
                    desc->irqs_unhandled, atomic_read(&desc->depth),
                    desc->base_irq_count, desc->delta_irq_count,
                    desc->irq_duration);
        }
    }

    return 0;
}


void isil_read_encode_orignal_frame(isil_chip_t *chip, u16 channel, u16 FrameIdx, u8 *buf);
void isil_read_encode_rebuild_frame(isil_chip_t *chip, u16 channel, u16 FrameIdx, u8 *buf);

static int inline gen_verify_data(int test_case, u32 data)
{
    switch(test_case) {
        default:
        case 0:return data;
        case 1:return ~data;
        case 2:return ((data & 0x1) ? 0xaa55aa55 : 0x55aa55aa);
    }
}

static ssize_t chip_proc_write(struct file *file, const char __user *buffer, size_t count, loff_t *offset)
{
    isil_chip_t *chip = (isil_chip_t *)isil_file_get_seq_private(file);
    char cmdbuf[128];
    char **argv;
    int argc, i,ch,cnt;
    u32 val = 0;

    if(!chip){
        ISIL_DBG(ISIL_DBG_INFO, "internal error!\n");
        return 0;
    }
    if(count > 128){
        if(copy_from_user(cmdbuf, buffer, 128) != 0){
            printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);                    
        }
    }
    else{
        if(copy_from_user(cmdbuf, buffer, count) != 0){
            printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);                    
        }
    }
    argv = argv_split(GFP_KERNEL, cmdbuf, &argc);
    for(i = 0; i < (argc - 1); i++)
    {
        ISIL_DBG(ISIL_DBG_INFO, "cmd %d: %s\n", i, argv[i]);
    }
    if((argv[0] != NULL) && (strcmp(argv[0], "burst") == 0))
    {
        chip->io_op->chip_write32(chip, 0x18004, 0x7c000000);
        val = chip->io_op->chip_read32(chip, ENABLE_VLC_LOOKUP_TABLE) | 0x2;
        chip->io_op->chip_write32(chip, ENABLE_VLC_LOOKUP_TABLE, val);
    }
    else if((argv[0] != NULL) && (strcmp(argv[0], "single") == 0))
    {
        chip->io_op->chip_write32(chip, 0x18004, 0x7c000000);
        chip->io_op->chip_write32(chip, ENABLE_VLC_LOOKUP_TABLE, chip->io_op->chip_read32(chip, ENABLE_VLC_LOOKUP_TABLE) | 0x2);
    }
    else if((argv[0] != NULL) && (strcmp(argv[0], "h264_burst") == 0))
    {
        //chip->io_op->chip_write32(chip,MODE, DDRON);//enable ddr
        while(1)isil_vlc_irq_handler(0, chip);
    }
    /*close ad*/
    else if((argv[0] != NULL) && (strcmp(argv[0], "poweroff") == 0)){
        pci_i2c_write(chip, 0x50, 0xce, 0x3f);
        pci_i2c_write(chip, 0x52, 0xce, 0x3f);
        pci_i2c_write(chip, 0x54, 0xce, 0x3f);
        pci_i2c_write(chip, 0x56, 0xce, 0x3f);
    }
    else if((argv[0] != NULL) && (strcmp(argv[0], "poweron") == 0)){
        pci_i2c_write(chip, 0x50, 0xce, 0x00);
        pci_i2c_write(chip, 0x52, 0xce, 0x00);
        pci_i2c_write(chip, 0x54, 0xce, 0x00);
        pci_i2c_write(chip, 0x56, 0xce, 0x00);
    }
    else if((argv[0] != NULL) && (strcmp(argv[0], "mpb_read") == 0)){
        printk("\nread 0x%08x == >0x%08x\n", atoi(argv[1]), mpb_read(chip, atoi(argv[1])));
    }
    else if((argv[0] != NULL) && (strcmp(argv[0], "mpb_write") == 0)){
        mpb_write(chip, atoi(argv[1]), atoi(argv[2]));
        printk("\nwrite 0x%08x == >0x%08x\n", atoi(argv[1]), atoi(argv[2]));
    }
    else if((argv[0] != NULL) && (strcmp(argv[0], "mpb_dump") == 0)){
        u32 base;
        u32 length;

        if(argc >= 3){
            base = atoi(argv[1]);
            length = atoi(argv[2]);
            for(i = 0; i < length; i++){
                if((i % 16) == 0){
                    printk("\n0x%08x:", base + i);
                }
                printk(" %02x", mpb_read(chip, base + i));
            }
            printk("\n");
        }
    }
    else if((argv[0] != NULL) && (strcmp(argv[0], "i2c_read") == 0)){
        u8 temp = 0;
        pci_i2c_read(chip, atoi(argv[1]), atoi(argv[2]), &temp);
        printk("\nread 0x%08x, 0x%08x == >0x%x\n", atoi(argv[1]), atoi(argv[2]), temp);
    }
    else if((argv[0] != NULL) && (strcmp(argv[0], "i2c_write") == 0)){
        pci_i2c_write(chip, atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
        printk("\nwrite 0x%08x, 0x%08x == >0x%08x\n", atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
    }
    else if((argv[0] != NULL) && (strcmp(argv[0], "i2c_dump") == 0)){
        int iic = atoi(argv[1]);
        u8 val = 0;

        printk("0x%02x", iic);
        for(i = 0; i < 256; i++) {
            if((i % 16) == 0) {
                printk("\n%08x:", i);
            }
            pci_i2c_read(chip, iic, i, &val);
            printk("%02x ", val);
        }
        printk("\n");
    }else if(strcmp(argv[0], "read_ec_yuv") == 0){ // useage: read_ec_yuv chan_id frame_id file_name   ....read encoder frame(constrction format)
        u32 len, size;
        struct file *yuv_file = NULL;
        mm_segment_t fs;
        char *yuv_buf = NULL;
        if((argv[1] == NULL) || (argv[2] == NULL))
        {
            printk("no channel id and buffer id\n");
            goto chip_write_end;
        }
        ch = atoi(argv[1]);
        cnt = atoi(argv[2]);

        size = (704 * 576 * 3)>>1;
        yuv_buf = (char*)__get_free_pages(GFP_KERNEL, get_order(size));
        if(!yuv_buf) {
            printk("no free buffer\n");
            goto chip_write_end;
        }
        isil_read_encode_rebuild_frame(chip, ch, cnt, yuv_buf);

        yuv_file = filp_open(argv[3], O_CREAT | O_RDWR, 655);
        if(IS_ERR(yuv_file))
        {
            printk("open file \"%s\" faild, %d\n", argv[3], (int )yuv_file);
            return IS_ERR(yuv_file);
        }
        fs=get_fs();
        set_fs(KERNEL_DS);
        len = yuv_file->f_op->write(yuv_file, yuv_buf, size, &(yuv_file->f_pos));
        set_fs(fs);
        if(yuv_buf)
            free_pages((unsigned long)yuv_buf, get_order(size));
        filp_close(yuv_file, NULL);        

    }else if(strcmp(argv[0], "read_eo_yuv") == 0){// useage: read_eo_yuv chan_id frame_id file_name   ....read encoder frame(orignal format)
        u32 len, size;
        struct file *yuv_file = NULL;
        mm_segment_t fs;
        char *yuv_buf = NULL;
        if((argv[1] == NULL) || (argv[2] == NULL))
        {
            printk("no channel id and buffer id\n");
            goto chip_write_end;
        }
        ch = atoi(argv[1]);
        cnt = atoi(argv[2]);

        size = (704 * 576 * 3)>>1;
        yuv_buf = (char*)__get_free_pages(GFP_KERNEL, get_order(size));
        if(!yuv_buf) {
            printk("no free buffer\n");
            goto chip_write_end;
        }
        isil_read_encode_orignal_frame(chip, ch, cnt, yuv_buf);

        yuv_file = filp_open(argv[3], O_CREAT | O_RDWR, 655);
        if(IS_ERR(yuv_file))
        {
            printk("open file \"%s\" faild, %d\n", argv[3], (int )yuv_file);
            return IS_ERR(yuv_file);
        }
        fs=get_fs();
        set_fs(KERNEL_DS);
        len = yuv_file->f_op->write(yuv_file, yuv_buf, size, &(yuv_file->f_pos));
        set_fs(fs);
        if(yuv_buf)
            free_pages((unsigned long)yuv_buf, get_order(size));
        filp_close(yuv_file, NULL);



    }else if((argv[0] != NULL) && (strcmp(argv[0], "dump") == 0) && (argc > 4)){
        struct file *mem_file;
        mm_segment_t fs; 
        int ab_switch = 0;
        u32 ddr_start, size, page_id, offset, page_cnt, len;
        char *buff, *temp_buf, *p_buff;
        if((argv[1][0] == 'A') || (argv[1][0] == 'a')){
            ab_switch = 0;
        }else{
            ab_switch = 1;
        }
        temp_buf = (char*)__get_free_pages(GFP_KERNEL, get_order(0x80000));
        p_buff = temp_buf;
        ddr_start = atoi(argv[2]);
        size      = atoi(argv[3]);
        page_id = (ddr_start>>19) & 0xff;
        offset  = ddr_start & 0x0007ffff;
        page_cnt = (size / 0x80000);
        printk("save ddr %s: begin 0x%08x size 0x%08x , start page id %d offset 0x%08x...\n", ab_switch?"B":"A", ddr_start, size, page_id, offset);
        mem_file = isil_kernel_file_open(argv[4]);
        fs=get_fs();
        set_fs(KERNEL_DS);
#if 0 //reg dump
        p_buff = temp_buf;
        temp_len = 0x0;
        while(temp_len < 0x80000) {
            *(u32 *)p_buff = chip->io_op->chip_read32(chip, temp_len);
#ifdef POWERPC_PLATFORM
            *(u32 *)p_buff = in_le32(p_buff);
#endif
            p_buff += sizeof(int);
            temp_len += sizeof(int);
        }
        mem_file->f_op->write(mem_file, temp_buf, 0x80000, &(mem_file->f_pos));
#else

        chip->io_op->chip_write32(chip, 0x04, 0x1);
        for(i = 0; i < page_cnt; i++){
            /*switch page*/
            chip->io_op->chip_write32(chip, 0x30, (ab_switch << 14) | (page_id + i));
            if(i == 0){
                buff = chip->regs + 0x80000 + offset;
                len = 0x80000 - offset;
            }else{
                buff = chip->regs + 0x80000;
                len = 0x80000;
            }
            printk("--->write size %08x\n", len);
            /*write to file*/
            mem_file->f_op->write(mem_file, buff, len, &(mem_file->f_pos));
        }
        len = (size % 0x80000);
        if(len){
            chip->io_op->chip_write32(chip, 0x30, (ab_switch << 14) | (page_id + i));
            if(page_cnt == 0){
                buff = chip->regs + 0x80000 + offset;
            }else{
                buff = chip->regs + 0x80000;
            }
            printk("--->write size %08x\n", len);
            /*write tailer*/
            mem_file->f_op->write(mem_file, buff, len, &(mem_file->f_pos));
        }
#endif
        set_fs(fs);
        isil_kernel_file_close(mem_file);
        free_pages((unsigned long)temp_buf, get_order(0x80000));

    }
    else if((argv[0] != NULL) && (strcmp(argv[0], "ddr_test") == 0))
    {
        u32 *dma_phy = NULL, *dma_vir = NULL;
        u32 *dma_phy_temp = NULL, *dma_vir_temp = NULL;
        int error = 0;
        int test_flags = 0;
        int retry_cnt = 0, data_base = 0;
        int page_id = 0, ab_switch = 0;
        int test_case = 0;

        u32 section_size = 0x800;
        u32 section_cnt;
        dpram_control_t         *chip_dpram_controller;
        chip_ddr_burst_engine_t *chip_ddr_burst_interface;
        ddr_burst_interface_t   *burst_interface;
        dpram_page_node_t       *dpram_page;

        get_isil_dpram_controller(&chip_dpram_controller);
        chip_ddr_burst_interface = &chip->chip_ddr_burst_interface;
        chip_ddr_burst_interface->op->get_ddr_burst_interface(chip_ddr_burst_interface, &burst_interface);
#define DDR_PIO    0x0001
#define DDR_BURST  0x0002
#define SRAM_PIO   0x0004
#define SRAM_BURST 0x0008


        for(i = 2; i < argc; i++){
            if(strcmp(argv[i], "ddr_pio") == 0){
                test_flags |= DDR_PIO;
            }
            if(strcmp(argv[i], "ddr_burst") == 0){
                test_flags |= DDR_BURST;
            }
            if(strcmp(argv[i], "sram_pio") == 0){
                test_flags |= SRAM_PIO;
            }
            if(strcmp(argv[i], "sram_burst") == 0){
                test_flags |= SRAM_BURST;
            }
        }
#if 0
        i = 0;
        while(1) {
            if(i == 200000) {
                printk("\b/");
            }
            if(i == 400000) {
                printk("\b-");
            }
            if(i == 600000) {
                printk("\b\\");
                schedule();
                if (signal_pending(current)) {
                    return count;
                }
                i = 0;
            }
            i++;
            chip->io_op->chip_write32(chip, 0x9004, chip->io_op->chip_read32(chip, 0x9004));
        }
#endif
        retry_cnt = atoi(argv[1]);
        printk("--->0x%08x<---retry %d\n", test_flags, retry_cnt);
        dma_vir = (u32 *)__get_free_pages(GFP_KERNEL, 0);
        if(!dma_vir)
        {
            printk("no memory!\n");
            goto ddr_test_end;
        }
        dma_phy = (u32 *)virt_to_phys(dma_vir);
        printk("dst dma phy address: 0x%8p\n", dma_phy);
        memset(dma_vir, 0, PAGE_SIZE);
        dma_vir_temp = (u32 *)__get_free_pages(GFP_KERNEL, 0);
        if(!dma_vir)
        {
            printk("no memory!\n");
            goto ddr_test_end;
        }
        dma_phy_temp = (u32 *)virt_to_phys(dma_vir_temp);
        printk("src dma phy address: 0x%8p\n", dma_phy_temp);
        memset(dma_vir_temp, 0, PAGE_SIZE);
        /*test all pages*/
        //chip->io_op->chip_write32(chip, MODE, DDRON);//enable ddr
        //chip->io_op->chip_write32(chip, PCI_MASTER_CONTROL, 0x1c000000);
        chip->io_op->chip_write32(chip, MPI_CS_SETUP, 0x1);//set to 0x2 when read ping pong buffer;other set to 0x1
        chip->io_op->chip_write32(chip, MPI_RDY_HOLD, 0x3);
        data_base = 0;
        page_id = 0;
        ab_switch = 0;
        test_case = 0;
        while(1) {
            if(page_id >= 512) {
                page_id = 0;
            }
            data_base = (1 << 19) * page_id;
            if(test_flags & DDR_BURST) {
                u32 ddr_end_addr;
                u32 page_size;
                u32 temp_pio_addr;
                u32 temp_dma_addr;
                u32 *gen_data;

                page_size = 0x80000;
                section_size = 0x800;
                cnt = 0;
                section_cnt = page_size/section_size;
                while((cnt < section_cnt) && chip_dpram_controller->op->is_can_submit_move_data_from_host_to_dpram_service_req(chip_dpram_controller, &dpram_page,chip)){
                    ddr_end_addr = data_base + section_size * cnt;

                    gen_data = dma_vir;
                    while((gen_data - dma_vir) < (section_size >> 2)) {
                        *gen_data = gen_verify_data(test_case, ddr_end_addr + (gen_data - dma_vir));
                        gen_data++;
                    }
                    dma_phy = (u32 *)dma_map_single(NULL, dma_vir, PAGE_SIZE, DMA_TO_DEVICE);
                    dma_vir_temp = (u32 *)dma_map_single(NULL, dma_vir_temp, PAGE_SIZE, DMA_FROM_DEVICE);
                    temp_pio_addr = (u32 )dma_vir;
                    temp_dma_addr = (u32 )dma_phy;

#if defined(PIO_INTERFACE)
                    burst_interface->op->pio_host_to_sram_write(burst_interface, dpram_page, (u32*)temp_pio_addr, section_size, 0);
#elif defined(DMA_INTERFACE)
                    burst_interface->op->dma_host_to_sram_write(burst_interface, dpram_page, temp_dma_addr, section_size, 0);
#endif

                    burst_interface->op->start_block_transfer_from_sram_to_ddr(burst_interface, dpram_page, 
                            ddr_end_addr, page_id, section_size, DDR_CHIP_A);
                    chip_dpram_controller->op->notify_end_move_data_from_dpram_to_ddr_service_req(chip_dpram_controller, &dpram_page, chip);
                    dma_unmap_single(NULL, (u32 )dma_phy, PAGE_SIZE, DMA_TO_DEVICE);

                    cnt++;
                }

                cnt = 0;
                while((cnt < section_cnt) && chip_dpram_controller->op->is_can_submit_move_data_from_ddr_to_dpram_service_req(chip_dpram_controller, &dpram_page, chip)){
                    ddr_end_addr = data_base + section_size * cnt;
                    temp_pio_addr = (u32 )dma_vir_temp;
                    temp_dma_addr = (u32 )dma_phy_temp;

                    burst_interface->op->start_block_transfer_from_ddr_to_sram(burst_interface, dpram_page, 
                            ddr_end_addr, page_id, section_size, DDR_CHIP_A);
#if defined(PIO_INTERFACE)
                    burst_interface->op->pio_host_to_sram_read(burst_interface, dpram_page, (u32*)temp_pio_addr, section_size, 0);
#elif defined(DMA_INTERFACE)
                    burst_interface->op->dma_host_to_sram_read(burst_interface, dpram_page, temp_dma_addr, section_size, 0);
#endif
                    chip_dpram_controller->op->notify_end_move_data_from_dpram_to_host_service_req(chip_dpram_controller, &dpram_page, chip);
                    cnt++;
                    dma_unmap_single(NULL, (u32 )dma_phy_temp, PAGE_SIZE, DMA_FROM_DEVICE);
                    gen_data = dma_vir_temp;
                    while((gen_data - dma_vir_temp) < (section_size >> 2)) {
                        if(*gen_data != gen_verify_data(test_case, ddr_end_addr + (gen_data - dma_vir_temp))){
                            error = 1;
                            printk("verfiy data failed! expect=0x%08x, readback = 0x%08x\n", gen_verify_data(test_case, ddr_end_addr + (gen_data - dma_vir_temp)), *gen_data);
                            schedule();
                            if (signal_pending(current)) {
                                goto ddr_test_end;
                            }
                        }
                        gen_data++;
                    }
                }

            }
            test_case++;
            if(test_case > 2) {
                page_id++;
                test_case = 0;
                printk("%s page %3d %s\n", ab_switch?"DDRB":"DDRA", page_id, error?"FAILED":"OK");
                error = 0;
            }
            schedule();
            if (signal_pending(current)) {
                goto ddr_test_end;
            }
        }
ddr_test_end:
        if(dma_vir) {
            free_page((int )dma_vir);
        }
        if(dma_vir_temp) {
            free_page((int )dma_vir_temp);
        }
    }
chip_write_end:
    argv_free(argv);

    return count;
}

//struct isil_chip_device * tcd = NULL;

/************************************************************************************/
/************************************************************************************/


int register_all_tc_device(void)
{
    /*all the functions blew have handle error, so the return value can be ignore*/
    isil_chip_ai_init();
    isil_chip_vi_init();
    isil_audio_en_init();
    isil_audio_de_init();
    isil_jpeg_en_init();
    isil_videoh264_en_init();
    isil_videoh264s_en_init();
    return ISIL_OK;
}

void unregister_all_tc_devices(void)
{
    isil_chip_vi_remove();
    isil_chip_ai_remove();
    isil_audio_en_remove();
    isil_audio_de_remove();
    isil_jpeg_en_remove();
    isil_videoh264_en_remove();
    isil_videoh264s_en_remove();
}


int register_all_tc_driver(void)
{
    int ret = -1;

    if(isil_chip_ai_driver_init()){
        return ret;
    }
    if(isil_chip_vi_driver_init()){
        goto err1;
    }
    if(isil_audio_en_driver_init()) {  
        goto err2;
    }
    if(isil_audio_de_driver_init()){
        goto err3;
    }
    if(isil_video_en_driver_init()){
        goto err4;
    }
    if(isil_videosub_en_driver_init()){
        goto err5;
    }
    if(isil_jpeg_en_driver_init()){
        goto err6;
    }

    return 0;

err6:
    isil_videosub_en_driver_remove();
err5:
    isil_video_en_driver_remove();
err4:
    isil_audio_de_driver_remove();
err3:
    isil_audio_en_driver_remove();
err2:
    isil_chip_vi_driver_remove();
err1:
    isil_chip_ai_driver_remove();

    return -1;
}

void unregister_all_tc_driver(void)
{
    isil_chip_ai_driver_remove();
    isil_chip_vi_driver_remove();
    isil_audio_de_driver_remove();
    isil_audio_en_driver_remove();
    isil_video_en_driver_remove();
    isil_videosub_en_driver_remove();
    isil_jpeg_en_driver_remove();
}




static void isil_chip_remove(struct isil_chip_device * tcd)
{
    isil_chip_t *chip = tcd_priv(tcd);

    if(chip == NULL){
        return;
    }

    if(chip->chip_set_invalid){
        chip->chip_set_invalid(chip);
        chip->chip_set_invalid = NULL;
    }
    if(chip->unregister_curr_h264_encode_chan){
        chip->unregister_curr_h264_encode_chan(chip, chip->curr_h264_encode_chan);
        chip->unregister_curr_h264_encode_chan = NULL;
    }

    remove_isil_interrupt_control(&chip->isil_int_control, chip);
    if(chip->regs != NULL){
        iounmap(chip->regs);
        chip->regs=NULL;
    }
    if(chip->resource != NULL){
        release_resource(chip->resource);
        kfree(chip->resource);
        chip->resource = NULL;
    }
    if(chip->sync_regs != NULL){
        iounmap(chip->sync_regs);
        chip->sync_regs = NULL;
    }
    if(chip->sync_resource != NULL){
        release_resource(chip->sync_resource);
        kfree(chip->sync_resource);
        chip->sync_resource = NULL;
    }

    //remove chip_bus
    remove_chip_bus(&chip->isil_chip_bus, chip->bus_id);

    isil_module_unregister(chip, &chip->chip_proc);

    push_buf_pool_release(chip);
}

void remove_chip_device(struct isil_chip_device * tcd)
{
    printk("before isil_chip_exit tcd[0x%p]\n",tcd);
    tc_chip_exit(tcd); 
}

void release_chip_memory(struct isil_chip_device * tcd)
{
    tc_chipdev_free(tcd);
    printk("after tc_chip_dev_free\n");

}

void pci_init_ad(isil_chip_t *);
static int sys_clk = 0;
module_param(sys_clk, int, S_IRUGO);
MODULE_PARM_DESC(sys_clk, "On chip sys_clk rate, available sys_clk(108,110,120,150,166,172,175,180,185)");
static int  isil_chip_init(int bus_id, int chip_id, DVMNVS_CHIP_VIDEO_ENCODE_CAPABILITY *platform_encode_cap, int irq_id)
{
    isil_chip_t	*chip;

    DVMNVS_CHIP_VIDEO_ENCODE_CAPABILITY	*video_encode_cap;
    isil_proc_register_s *chip_proc;
    int	ret = 0;
    struct cmd_arg data;
    struct isil_chip_device *tcd;

    tcd = tc_chipdev_alloc(sizeof(*chip));
    if(!tcd){
        ret = -ENOMEM;
        return ret;	
    }
    chip = tcd_priv(tcd);

    sprintf(chip->name, "isil_chip_%d_%d", bus_id, chip_id);
    chip->bus_id = bus_id;
    chip->chip_id = chip_id;
    if(sys_clk == 0) {
        chip->sys_clock = DEFAULT_SYS_CLK;
    }else{
        chip->sys_clock = sys_clk;
    }
    atomic_set(&chip->open_chan_number, 0);
    init_robust_processing_control(&chip->chip_robust_processing);
    chip->video_encode_cap = platform_encode_cap;
    video_encode_cap = chip->video_encode_cap;
    chip->first_device_id = video_encode_cap->first_device_number;

    chip->isil_int_control.chip_irq_register_flags = 0;
    chip->isil_int_control.chip_irq = irq_id;
    set_irq_type(irq_id, IRQT_LOW);
 
    if(video_encode_cap->fpga_async_section_len != 0){
        chip->resource = request_mem_region(video_encode_cap->fpga_async_base_addr, video_encode_cap->fpga_async_section_len, chip->name);
        if(chip->resource == NULL){
            printk("requeset mem regions err\n");
            goto release;
        }
        chip->regs = ioremap_nocache(video_encode_cap->fpga_async_base_addr, video_encode_cap->fpga_async_section_len);
        chip->pa_regs = video_encode_cap->fpga_async_base_addr;
    } else {
        chip->regs = NULL;
        chip->pa_regs = 0;
    }
    if(video_encode_cap->fpga_sync_section_len != 0){
        chip->sync_resource = request_mem_region(video_encode_cap->fpga_sync_base_addr, video_encode_cap->fpga_sync_section_len, chip->name);
        if(chip->sync_resource == NULL){
            printk("requeset mem regions err\n");
            goto release;
        }
        chip->sync_regs = ioremap_nocache(video_encode_cap->fpga_sync_base_addr, video_encode_cap->fpga_sync_section_len);
        chip->pa_sync_regs = video_encode_cap->fpga_sync_base_addr;
    } else {
        chip->sync_regs = NULL;
        chip->pa_sync_regs = 0;
    }

    chip->chip_open = chip_hardware_resource_open;
    chip->chip_close = chip_hardware_resource_close;
    chip->chip_set_invalid = chip_phy_invalid;
    chip->chip_set_valid = chip_phy_valid;
    chip->chip_reset = chip_phy_reset;
#if defined(PLATFORM_ENDIAN_SAME)
    chip->io_op = &isil_host_io;
#elif defined(PLATFORM_ENDIAN_DIFFERENT)
    chip->io_op = &isil_pci_io;
#else
#warning "ENDIAN error!"
#endif

#if defined(PCI_PLATFORM)
    /*configure AD*/
#ifndef USER_FILE_IO_REQ
    pci_init_ad(chip);
#endif
#endif

    chip->register_timer   = register_on_chip_timer;
    chip->enable_timer     = enable_on_chip_timer;
    chip->disable_timer    = disable_on_chip_timer;
    chip->unregister_timer = unregister_on_chip_timer;

    chip->chip_polling_timeid = INVALIDTIMERID;
    chip->add_chip_polling_task = add_chip_polling_task;
    chip->delete_chip_polling_task = delete_chip_polling_task;

    chip_proc = &chip->chip_proc;
    chip_proc->entry = NULL;
    strcpy(chip_proc->name, "chip");
    chip_proc->read  = chip_proc_read;
    chip_proc->write = chip_proc_write;
    chip_proc->private = chip;
    isil_module_register(chip, chip_proc);

    init_chip_encode_chan_service_queue(&chip->chip_encode_service_queue);
    init_chip_ddr_burst_engine(&chip->chip_ddr_burst_interface, chip);
    init_chip_bus(&chip->isil_chip_bus, chip->bus_id, chip->chip_id);//never fail
    if(init_isil_interrupt_control(&chip->isil_int_control, chip) != ISIL_OK){
        ISIL_DBG(ISIL_DBG_FATAL, "irq controler init failed\n");
        goto remove;
    }
    spin_lock_init(&chip->lock);
    chip->vlc_ping_pong_index = 0;
    chip->curr_h264_encode_chan = NULL;
    chip->register_curr_h264_encode_chan = register_curr_h264_encode_chan;
    chip->unregister_curr_h264_encode_chan = unregister_curr_h264_encode_chan;
    chip->register_curr_h264_encode_push_chan = register_curr_h264_encode_push_chan;
    chip->unregister_curr_h264_encode_push_chan = unregister_curr_h264_encode_push_chan; 
    init_isil_timer(&chip->chip_timer_cont);
    if(tc_chip_init(tcd,bus_id,chip_id))
        goto remove;


    data.type = 5;//ISIL_AUDIO_IN
    data.channel_idx = 0;
    data.algorithm = 0;
    data.stream = 0;
    if(create_service_device(tcd, (unsigned long)&data))
        goto err;
    data.type = 3;//ISIL_VIDEO_IN
    if(create_service_device(tcd, (unsigned long)&data))
        goto err;

    if (push_buf_pool_init(chip))
        goto err;

    chip->chip_set_valid(chip);

    return ISIL_OK;    

err:

    release_all_service_device_ext_on_chip(tcd);
    remove_chip_device(tcd);
remove:
    isil_chip_remove(tcd);
release:
    release_chip_memory(tcd);
    printk("%s_probe fail\n", chip->name);
    return ret;
}


#if defined(PCI_PLATFORM)
static int __devinit isil_pci_probe(struct pci_dev *pci_dev, const struct pci_device_id *id)
{
    int slot, bus_id, devfn, i;
    type_bus_t      *isil_pci_bus;
    isil_chip_bus_t *isil_chip_bus;
    isil_chip_t     *chip;
    u32     chip_id;
    int     ret;
    DVMNVS_CHIP_VIDEO_ENCODE_CAPABILITY *platform_encode_cap;

    bus_id = pci_dev->bus->number;
    devfn  = PCI_FUNC(pci_dev->devfn);
    slot   = PCI_SLOT(pci_dev->devfn);

    ret = pci_enable_device(pci_dev);
    if(ret){
        ISIL_DBG(ISIL_DBG_ERR, "enable pci device failed %d\n", ret);
        return ret;
    }

    ISIL_DBG(ISIL_DBG_INFO, "detect tw%x pci device, bus %d, slot %d, fn %d, irq %d\n", pci_dev->device, bus_id, slot, devfn, pci_dev->irq);
    for(i = 0; i < DEVICE_COUNT_RESOURCE; i++)
    {
        if(pci_dev->resource[i].start){
            ISIL_DBG(ISIL_DBG_INFO, "BAR%d  %s Addr 0x%08x, Size 0x%08x\n", 
                    i, (pci_dev->resource[i].flags & IORESOURCE_IO) ? "I/O":"MEM",
                    pci_resource_start(pci_dev, i),
                    pci_resource_len(pci_dev, i));
        }
    }

    get_isil_pci_bus(&isil_pci_bus);
    chip_id = isil_pci_bus->op->get_device_id(isil_pci_bus);
    platform_encode_cap = (DVMNVS_CHIP_VIDEO_ENCODE_CAPABILITY *)kmalloc(sizeof(DVMNVS_CHIP_VIDEO_ENCODE_CAPABILITY), GFP_KERNEL);
    if(!platform_encode_cap) {
        return -ENOMEM;
    }
    memcpy(platform_encode_cap, &platform_video_encode_cap[0], sizeof(DVMNVS_CHIP_VIDEO_ENCODE_CAPABILITY));
    platform_encode_cap->first_device_number = chip_id * ISIL_PHY_VD_CHAN_NUMBER;
    platform_encode_cap->fpga_async_base_addr =  pci_resource_start(pci_dev, 0);//TW5864 only use BAR0
    platform_encode_cap->fpga_async_section_len = pci_resource_len(pci_dev, 0);

    ret = isil_chip_init(ISIL_ED_PCI_BUS, chip_id, platform_encode_cap, pci_dev->irq);
    if(ret){
        ISIL_DBG(ISIL_DBG_FATAL, "init chip failed %d\n", ret);
        pci_disable_device(pci_dev);
        return ret;
    }

    get_isil_pci_bus(&isil_pci_bus);
    isil_pci_bus->op->find_chip_bus_in_type_bus_table(isil_pci_bus, &isil_chip_bus, chip_id);
    if(isil_chip_bus != NULL){
        chip = to_get_chip_with_chip_bus(isil_chip_bus);
        chip->dev = &pci_dev->dev;
        /*enable bus control*/
        pci_set_master(pci_dev);
        pci_set_drvdata(pci_dev, chip);
    }else{
        ISIL_DBG(ISIL_DBG_ERR, "No such chip %d\n", chip_id);
        pci_disable_device(pci_dev);
    }
    return 0;
} 

static void __devexit isil_pci_remove(struct pci_dev *pci_dev)
{
    type_bus_t      *isil_pci_bus;
    isil_chip_bus_t *isil_chip_bus;
    isil_chip_t     *chip;
    struct isil_chip_device *tcd;
    DVMNVS_CHIP_VIDEO_ENCODE_CAPABILITY *video_encode_cap;

    chip = pci_get_drvdata(pci_dev);
    get_isil_pci_bus(&isil_pci_bus); 
    isil_pci_bus->op->find_chip_bus_in_type_bus_table(isil_pci_bus, &isil_chip_bus, chip->chip_id);
    if(isil_chip_bus != NULL){
	isil_interrupt_control_t  *isil_int_control;

        isil_int_control = &chip->isil_int_control;
        //free_irq(chip->isil_int_control.chip_irq, &chip->isil_int_control);
        if(isil_int_control->chip_irq_register_flags){
            free_irq(isil_int_control->chip_irq, isil_int_control);
            isil_int_control->chip_irq_register_flags = 0;
        }

        tcd = priv_2_tcd((void*)chip);
        release_all_service_device_ext_on_chip(tcd);
        remove_chip_device(tcd);
        video_encode_cap = chip->video_encode_cap;

        isil_chip_remove(tcd);
        release_chip_memory(tcd);
        if(video_encode_cap) {
            kfree(video_encode_cap);
        }
    }else{
        printk("%s, %d: chip not register!!\n", __FUNCTION__, __LINE__);
    }
    pci_disable_device(pci_dev);
}

static struct pci_device_id isil_pci_tbl[] __initdata = {
    {
        .vendor = PCI_ISIL_VENDOR_ID,
        .device = PCI_DEVICE_ID_TW5864,
        .subvendor = PCI_ANY_ID, 
        .subdevice = PCI_ANY_ID, 
        .class = 0,
        .class_mask = 0,
        .driver_data = 0,
    },
    {
        .vendor = PCI_ISIL_VENDOR_ID,
        .device = PCI_DEVICE_ID_TW5866,
        .subvendor = PCI_ANY_ID, 
        .subdevice = PCI_ANY_ID, 
        .class = 0,
        .class_mask = 0,
        .driver_data = 0,
    },{},
};

static struct pci_driver isil_pci_driver = {
    .name     = "TW5864",
    .id_table = isil_pci_tbl,
    .probe    = isil_pci_probe,
    .remove   = isil_pci_remove,
};
#endif

static int __init   isil_init(void)
{
    int ret = ISIL_OK;

    printk("\nVersion: %d.%d.%d-%d\nGCC Version: %s\nARCH: %s\nBuild at: %s:%s'\n", 
            get_version_major(__ISIL__CODE__VERSION__),
            get_version_submajor(__ISIL__CODE__VERSION__),
            get_version_minor(__ISIL__CODE__VERSION__), 
            __ISIL_SVN_VERSION__,__VERSION__, __ISIL_ARCH__ , __DATE__, __TIME__);
    init_system_walltimer();
    isil_core_init();
    register_all_tc_device();
    register_all_tc_driver();
    init_isil_dma_chan();
    ret = isil_chip_driver_init();
    if(ret != ISIL_OK){
        printk("chip driver register failed!\n");
        return ret;
    }

    proc_module_init();
    init_isil_root_bus();
#ifdef  POWERPC_PLATFORM
    isil_map_init();
#endif
    start_isil_kthread();
#if defined(PCI_PLATFORM)
    if (pci_register_driver(&isil_pci_driver)) {
        stop_isil_kthread();
#if defined(POWERPC_PLATFORM)
        isil_map_exit();
#endif
        remove_isil_root_bus();
        printk("Faild register pci driver!\n");
        return -ENODEV;
    }
#else
#endif

    return ret;
}

static void __exit  isil_exit(void)
{
#if  defined(PCI_PLATFORM)
    pci_unregister_driver(&isil_pci_driver);
#else
    get_isil_host_bus(&isil_host_bus);
    isil_host_bus->op->find_chip_bus_in_type_bus_table(isil_host_bus, &isil_chip_bus, 0);
    if(isil_chip_bus != NULL){
        chip = to_get_chip_with_chip_bus(isil_chip_bus);

        tcd = priv_2_tcd((void*)chip);
        printk("tcd = %p \n",tcd);
        release_all_service_device_ext_on_chip(tcd);
        remove_chip_device(tcd);
        isil_chip_remove(tcd);
        release_chip_memory(tcd);
    }
#endif
    stop_isil_kthread();

    remove_isil_root_bus();
    isil_chip_driver_remove();
    unregister_all_tc_driver();
    unregister_all_tc_devices();
    proc_module_release();
    isil_core_exit();
#ifdef  POWERPC_PLATFORM
    isil_map_exit();
#endif
}

module_init(isil_init);
module_exit(isil_exit);

MODULE_DESCRIPTION("DVMicro h264 chip driver");
MODULE_AUTHOR("tang shang chang, email:tang.sc@163.com");
MODULE_LICENSE("GPL");



