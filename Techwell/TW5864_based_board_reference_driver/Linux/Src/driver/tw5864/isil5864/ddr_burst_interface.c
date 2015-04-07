#include	<isil5864/isil_common.h>

#undef  DDR_BURST_INTERFACE_DEBUG

static void ddr_burst_interface_reg_init(ddr_burst_interface_t *burst_interface)
{
    if(burst_interface != NULL){
        burst_interface->page_mode_reg_value.value = 0;
        burst_interface->access_page_mode_reg_offset = MPI_ACCESS_PAGE_MODE_REG_OFFSET;
        burst_interface->cmd_status_reg_value.value = 0;
        burst_interface->burst_cmd_status_reg_offset = MPI_CMD_STATUS_REG_OFFSET;
        burst_interface->ddr_base_addr_reg_value.value = 0;
        burst_interface->burst_ddr_addr_reg_offset = MPI_BURST_DDR_ADDR_REG_OFFSET;
        burst_interface->sram_base_addr_reg_value.value = 0;
        burst_interface->burst_sram_addr_reg_offset = MPI_BURST_SRAM_ADDR_REG_OFFSET;
    }
}

static void ddr_burst_interface_reg_reset(ddr_burst_interface_t *burst_interface)
{
    if(burst_interface != NULL){
        burst_interface->page_mode_reg_value.value = 0;
        burst_interface->cmd_status_reg_value.value = 0;
        burst_interface->ddr_base_addr_reg_value.value = 0;
        burst_interface->sram_base_addr_reg_value.value = 0;
    }
}

static void ddr_burst_interface_reg_get_page_mode_reg(ddr_burst_interface_t *burst_interface, isil_chip_t *chip)
{
    if((burst_interface!=NULL) && (chip!=NULL)){
        burst_interface->page_mode_reg_value.value = chip->io_op->chip_read32(chip, burst_interface->access_page_mode_reg_offset);
    }
}

static void ddr_burst_interface_reg_update_page_mode_reg(ddr_burst_interface_t *burst_interface, isil_chip_t *chip)
{
    if((burst_interface!=NULL) && (chip!=NULL)){
        chip->io_op->chip_write32(chip, burst_interface->access_page_mode_reg_offset, burst_interface->page_mode_reg_value.value);
#ifdef  DDR_BURST_INTERFACE_DEBUG
        printk("%s.%d:%x=0x%08x\n", __FUNCTION__, __LINE__, burst_interface->access_page_mode_reg_offset, chip->io_op->chip_read32(chip, burst_interface->page_mode_reg_value.value));
#endif
    }
}

static void ddr_burst_interface_reg_get_burst_interface_param(ddr_burst_interface_t *burst_interface, isil_chip_t *chip)
{
    if((burst_interface!=NULL) && (chip!=NULL)){
        burst_interface->cmd_status_reg_value.value = chip->io_op->chip_read32(chip, burst_interface->burst_cmd_status_reg_offset);
        burst_interface->ddr_base_addr_reg_value.value = chip->io_op->chip_read32(chip, burst_interface->burst_ddr_addr_reg_offset);
        burst_interface->sram_base_addr_reg_value.value = chip->io_op->chip_read32(chip, burst_interface->burst_sram_addr_reg_offset);
    }
}

static void ddr_burst_interface_reg_update_burst_interface_param(ddr_burst_interface_t *burst_interface, isil_chip_t *chip)
{
    if((burst_interface!=NULL) && (chip!=NULL)){
        if(!(chip->io_op->chip_read32(chip, MODE) & DDRON)) {
            ISIL_DBG(ISIL_DBG_ERR, "On chip DDR not enable, mode = 0x%08x\n", chip->io_op->chip_read32(chip, MODE));
            return;
        }
        if(!(chip->io_op->chip_read32(chip, MPI_ACCESS_PAGE_MODE_REG_OFFSET) & (1 << 15))) {
            ISIL_DBG(ISIL_DBG_ERR, "hardware burst interface not enable\n");
            return;
        }
        chip->io_op->chip_write32(chip, burst_interface->burst_ddr_addr_reg_offset, burst_interface->ddr_base_addr_reg_value.value);
#ifdef  DDR_BURST_INTERFACE_DEBUG
        printk("%s.%d:%x=0x%08x\n", __FUNCTION__, __LINE__, burst_interface->burst_ddr_addr_reg_offset, chip->io_op->chip_read32(chip, burst_interface->burst_ddr_addr_reg_offset));
#endif
        chip->io_op->chip_write32(chip, burst_interface->burst_sram_addr_reg_offset, burst_interface->sram_base_addr_reg_value.value);
#ifdef  DDR_BURST_INTERFACE_DEBUG
        printk("%s.%d:%x=0x%08x\n", __FUNCTION__, __LINE__, burst_interface->burst_sram_addr_reg_offset, chip->io_op->chip_read32(chip, burst_interface->burst_sram_addr_reg_offset));
#endif
        chip->io_op->chip_write32(chip, burst_interface->burst_cmd_status_reg_offset, burst_interface->cmd_status_reg_value.value);
#ifdef  DDR_BURST_INTERFACE_DEBUG
        printk("%s.%d:%x=0x%08x\n", __FUNCTION__, __LINE__, burst_interface->burst_cmd_status_reg_offset, chip->io_op->chip_read32(chip, burst_interface->burst_cmd_status_reg_offset));
#endif
        burst_interface->op->start_ddr_burst(burst_interface);
        chip->io_op->chip_write32(chip, burst_interface->burst_cmd_status_reg_offset, burst_interface->cmd_status_reg_value.value);
#ifdef  DDR_BURST_INTERFACE_DEBUG
        printk("%s.%d:%x=0x%08x\n", __FUNCTION__, __LINE__, burst_interface->burst_cmd_status_reg_offset, chip->io_op->chip_read32(chip, burst_interface->burst_cmd_status_reg_offset));
#endif
    }
}

static void ddr_burst_interface_reg_enable_access_sram_block(ddr_burst_interface_t *burst_interface)
{
    if(burst_interface != NULL){
        burst_interface->page_mode_reg_value.bit_value.burst_mode_en = 1;
    }
}

static void ddr_burst_interface_reg_enable_access_ddr_block(ddr_burst_interface_t *burst_interface)
{
    if(burst_interface != NULL){
        burst_interface->page_mode_reg_value.bit_value.burst_mode_en = 0;
    }
}

static void ddr_burst_interface_reg_select_ddr_A(ddr_burst_interface_t *burst_interface)
{
    if(burst_interface != NULL){
        burst_interface->page_mode_reg_value.bit_value.ddr_a_b_select = 0;
    }
}

static void ddr_burst_interface_reg_select_ddr_B(ddr_burst_interface_t *burst_interface)
{
    if(burst_interface != NULL){
        burst_interface->page_mode_reg_value.bit_value.ddr_a_b_select = 1;
    }
}

static void ddr_burst_interface_reg_enable_read_4(ddr_burst_interface_t *burst_interface)
{
    if(burst_interface != NULL){
        burst_interface->page_mode_reg_value.bit_value.mpi_burst_read_4_enable = 1;
    }
}

static void ddr_burst_interface_reg_disable_read_4(ddr_burst_interface_t *burst_interface)
{
    if(burst_interface != NULL){
        burst_interface->page_mode_reg_value.bit_value.mpi_burst_read_4_enable = 0;
    }
}

static void ddr_burst_interface_reg_set_ddr_page_number(ddr_burst_interface_t *burst_interface, int page_id)
{
    if(burst_interface != NULL){
        burst_interface->page_mode_reg_value.bit_value.page_id = (page_id&0xff);
    }
}

static void ddr_burst_interface_reg_set_ddr_burst_param(ddr_burst_interface_t *burst_interface, int burst_len, int dir)
{
    if(burst_interface != NULL){
        if(dir == ISIL_READ){
            burst_interface->cmd_status_reg_value.bit_value.burst_dir = 0;
        } else {
            burst_interface->cmd_status_reg_value.bit_value.burst_dir = 1;
        }
        burst_len += 3;
        burst_len >>= 2;
        burst_len <<= 2;
        burst_interface->cmd_status_reg_value.bit_value.burst_len = burst_len;
    }
}

static void ddr_burst_interface_reg_start_ddr_burst(ddr_burst_interface_t *burst_interface)
{
    if(burst_interface != NULL){
        burst_interface->cmd_status_reg_value.bit_value.burst_trigger = 1;
    }
}

static void ddr_burst_interface_reg_int_enable(ddr_burst_interface_t *burst_interface, int ddr_single_access_err_int_enable, int ddr_burst_access_err_int_enable, int burst_end_int_enable)
{
    if(burst_interface != NULL){
        burst_interface->cmd_status_reg_value.bit_value.ddr_single_access_err_int_enable = (ddr_single_access_err_int_enable&0x1);
        burst_interface->cmd_status_reg_value.bit_value.ddr_burst_access_err_int_enable = (ddr_burst_access_err_int_enable&0x1);
        burst_interface->cmd_status_reg_value.bit_value.burst_end_int_enable = (burst_end_int_enable&0x1);
    }
}

static int  ddr_burst_interface_reg_get_single_access_err_flag(ddr_burst_interface_t *burst_interface)
{
    return burst_interface->cmd_status_reg_value.bit_value.ddr_single_access_err_flag;
}

static int  ddr_burst_interface_reg_get_burst_access_err_flag(ddr_burst_interface_t *burst_interface)
{
    return burst_interface->cmd_status_reg_value.bit_value.ddr_burst_err_flag;
}

static int  ddr_burst_interface_reg_get_burst_busy_flag(ddr_burst_interface_t *burst_interface)
{
    return burst_interface->cmd_status_reg_value.bit_value.ddr_burst_busy_flag;
}

static int  ddr_burst_interface_reg_get_burst_end_flag(ddr_burst_interface_t *burst_interface)
{
    return burst_interface->cmd_status_reg_value.bit_value.ddr_burst_end_flag;
}

static void ddr_burst_interface_reg_clear_burst_end_flag(ddr_burst_interface_t *burst_interface)
{
    if(burst_interface != NULL){
        burst_interface->cmd_status_reg_value.bit_value.ddr_burst_end_flag = 1;
    }
}

static void ddr_burst_interface_reg_set_burst_ddr_addr(ddr_burst_interface_t *burst_interface, u32 addr_offset)
{
    if(burst_interface != NULL){
        burst_interface->ddr_base_addr_reg_value.bit_value.ddr_base_addr = addr_offset&0x0fffffff;
    }
}

static void ddr_burst_interface_reg_set_burst_sram_addr(ddr_burst_interface_t *burst_interface, u32 addr_offset)
{
    if(burst_interface != NULL){
        burst_interface->sram_base_addr_reg_value.bit_value.sram_base_addr = addr_offset&0x0000ffff;
    }
}

static int  ddr_burst_interface_reg_pio_host_to_sram_write(ddr_burst_interface_t *burst_interface, dpram_page_node_t *dpram_page, u32 *host_end_addr, int len, int offset)
{
    int ret = 0;
    if((burst_interface!=NULL) && (dpram_page!=NULL)){
        isil_chip_t  *chip;
        u32 dpram_page_base;
        int i;
        chip = burst_interface->chip;
        burst_interface->op->get_page_mode_reg(burst_interface, chip);
        burst_interface->op->enable_access_sram_block(burst_interface);
        burst_interface->op->update_page_mode_reg(burst_interface, chip);

        dpram_page_base = dpram_page->op->get_page_base(dpram_page);
        dpram_page_base += offset;
        ret = len;
        len += 3;
        len >>= 2;
        for(i=0; i<len; i++){
            chip->io_op->chip_write32(chip, dpram_page_base, host_end_addr[i]);
            dpram_page_base += 4;
        }
    }
    return ret;
}

static int  ddr_burst_interface_reg_dma_host_to_sram_write(ddr_burst_interface_t *burst_interface, dpram_page_node_t *dpram_page, dma_addr_t host_end_addr, int len, int offset)
{
    int ret = 0;
    if((burst_interface!=NULL) && (dpram_page!=NULL)){
        isil_chip_t  *chip;
        dma_addr_t  src, dest;
        chip = burst_interface->chip;
        burst_interface->op->get_page_mode_reg(burst_interface, chip);
        burst_interface->op->enable_access_sram_block(burst_interface);
        burst_interface->op->update_page_mode_reg(burst_interface, chip);

        dest = dpram_page->op->get_page_base(dpram_page);
        dest += chip->pa_regs;
        dest += offset;
        src = host_end_addr; 
        ret = len;
        len += 3;
        len >>= 2;
        len <<= 2;
        fpga_dma_receive((u32)src, (u32)dest, len);
    }
    return ret;
}

static int  ddr_burst_interface_reg_pio_host_to_sram_read(ddr_burst_interface_t *burst_interface, dpram_page_node_t *dpram_page, u32 *host_end_addr, int len, int offset)
{
    int ret = 0;
    if((burst_interface!=NULL) && (dpram_page!=NULL)){
        isil_chip_t  *chip;
        u32 dpram_page_base;
        int i;
        chip = burst_interface->chip;
        burst_interface->op->get_page_mode_reg(burst_interface, chip);
        burst_interface->op->enable_access_sram_block(burst_interface);
        burst_interface->op->update_page_mode_reg(burst_interface, chip);

        dpram_page_base = dpram_page->op->get_page_base(dpram_page);
        dpram_page_base += offset;
        ret = len;
        len += 3;
        len >>= 2;
        for(i=0; i<len; i++){
            host_end_addr[i] = chip->io_op->chip_read32(chip, dpram_page_base);
            dpram_page_base += 4;
        }
    }
    return ret;
}

static int  ddr_burst_interface_reg_dma_host_to_sram_read(ddr_burst_interface_t *burst_interface, dpram_page_node_t *dpram_page, dma_addr_t host_end_addr, int len, int offset)
{
    int ret=0;
    if((burst_interface!=NULL) && (dpram_page!=NULL)){
        isil_chip_t  *chip;
        dma_addr_t  src, dest;
        chip = burst_interface->chip;
        burst_interface->op->get_page_mode_reg(burst_interface, chip);
        burst_interface->op->enable_access_sram_block(burst_interface);
        burst_interface->op->update_page_mode_reg(burst_interface, chip);

        src = dpram_page->op->get_page_base(dpram_page);
        src += chip->pa_regs;
        src += offset;
        dest = host_end_addr; 
        ret = len;
        len += 3;
        len >>= 2;
        len <<= 2;
        fpga_dma_receive((u32)src, (u32)dest, len);
        /*printk("%d: len 0x%x, 0x30 ==> 0x%08x, %08x, %08x, src %08x\n", __LINE__, 
          len,
          chip->io_op->chip_read32(chip, 0x30),
          chip->io_op->chip_read32(chip, 0x18004),
          chip->io_op->chip_read32(chip, 0x101c), src);*/
    }
    return ret;
}

static int  ddr_burst_interface_reg_start_block_transfer_from_sram_to_ddr(ddr_burst_interface_t *burst_interface, dpram_page_node_t *dpram_page, u32 ddr_end_addr, int page_id, int len, int ddr_chip_a_or_b)
{
    int count, escape;

    count = escape = 0;
    if((burst_interface!=NULL) && (dpram_page!=NULL)){
        isil_chip_t  *chip;
        u32 dpram_page_base;
        chip = burst_interface->chip;
        burst_interface->op->get_page_mode_reg(burst_interface, chip);
        burst_interface->op->enable_access_sram_block(burst_interface);
        burst_interface->op->update_page_mode_reg(burst_interface, chip);

        count = escape = 0;
        burst_interface->op->get_burst_interface_param(burst_interface, chip);
        while(burst_interface->op->get_burst_busy_flag(burst_interface)){
            //mdelay(1);
            burst_interface->op->get_burst_interface_param(burst_interface, chip);
            count++;
            if(count > BURST_INTERFACE_CHECKUP_TIMEOUT){
                printk("%s.%d: can't access burst interface\n", __FUNCTION__, __LINE__);
                escape = 1;
                break;
            }
        }

        if(!escape){
            burst_interface->op->set_ddr_page_number(burst_interface, page_id);
            if(DDR_CHIP_B == ddr_chip_a_or_b){
                burst_interface->op->select_ddr_B(burst_interface);
            } else {
                burst_interface->op->select_ddr_A(burst_interface); 
            }
#if defined(PCI_PLATFORM) && defined(DMA_INTERFACE) && defined(BURST_INTERFACE)
            burst_interface->op->enable_read_4(burst_interface);
#else
            burst_interface->op->disable_read_4(burst_interface);
#endif
            burst_interface->op->update_page_mode_reg(burst_interface, chip);

            dpram_page_base = dpram_page->op->get_page_offset(dpram_page);
            burst_interface->op->set_burst_sram_addr(burst_interface, dpram_page_base);
            burst_interface->op->set_burst_ddr_addr(burst_interface, ddr_end_addr);
            burst_interface->op->int_enable(burst_interface, 0, 0, 0);
            burst_interface->op->set_ddr_burst_param(burst_interface, len, ISIL_WRITE);
            burst_interface->op->update_burst_interface_param(burst_interface, chip);

            count = escape = 0;
            burst_interface->op->get_burst_interface_param(burst_interface, chip);
            while(burst_interface->op->get_burst_end_flag(burst_interface) == 0){
                //mdelay(1);
                burst_interface->op->get_burst_interface_param(burst_interface, chip);
                count++;
                if(count > BURST_INTERFACE_CHECKUP_TIMEOUT){
                    printk("%s.%d: can't access burst interface\n", __FUNCTION__, __LINE__);
                    escape = 1;
                    break;
                }
            }

            if(escape == 0){
                burst_interface->op->clear_burst_end_flag(burst_interface);
                burst_interface->op->update_burst_interface_param(burst_interface, chip);
            }
        }
        burst_interface->op->clear_burst_done(burst_interface);
    }
    return escape;
}

static int  ddr_burst_interface_reg_start_nonblock_transfer_from_sram_to_ddr(ddr_burst_interface_t *burst_interface, dpram_page_node_t *dpram_page, u32 ddr_end_addr, int page_id, int len, int ddr_chip_a_or_b)
{
    int count, escape;

    count = escape = 0;
    if((burst_interface!=NULL) && (dpram_page!=NULL)){
        isil_chip_t  *chip;
        u32 dpram_page_base;
        int count, escape;

        chip = burst_interface->chip;
        burst_interface->op->get_page_mode_reg(burst_interface, chip);
        burst_interface->op->enable_access_sram_block(burst_interface);
        burst_interface->op->update_page_mode_reg(burst_interface, chip);

        count = escape = 0;
        burst_interface->op->get_burst_interface_param(burst_interface, chip);
        while(burst_interface->op->get_burst_busy_flag(burst_interface)){
            //mdelay(1);
            burst_interface->op->get_burst_interface_param(burst_interface, chip);
            count++;
            if(count > BURST_INTERFACE_CHECKUP_TIMEOUT){
                printk("%s.%d: can't access burst interface\n", __FUNCTION__, __LINE__);
                escape = 1;
                break;
            }
        }

        if(!escape){
            burst_interface->op->set_ddr_page_number(burst_interface, page_id);
            if(DDR_CHIP_B == ddr_chip_a_or_b){
                burst_interface->op->select_ddr_B(burst_interface);
            } else {
                burst_interface->op->select_ddr_A(burst_interface); 
            }
#if defined(PCI_PLATFORM) && defined(DMA_INTERFACE) && defined(BURST_INTERFACE)
            burst_interface->op->enable_read_4(burst_interface);
#else
            burst_interface->op->disable_read_4(burst_interface);
#endif
            burst_interface->op->update_page_mode_reg(burst_interface, chip);

            dpram_page_base = dpram_page->op->get_page_offset(dpram_page);
            burst_interface->op->set_burst_sram_addr(burst_interface, dpram_page_base);
            burst_interface->op->set_burst_ddr_addr(burst_interface, ddr_end_addr);
            burst_interface->op->int_enable(burst_interface, 0, 0, 1);
            burst_interface->op->set_ddr_burst_param(burst_interface, len, ISIL_WRITE);
            burst_interface->op->update_burst_interface_param(burst_interface, chip);
        }
    }
    return escape;
}

static int  ddr_burst_interface_reg_start_block_transfer_from_ddr_to_sram(ddr_burst_interface_t *burst_interface, dpram_page_node_t *dpram_page, u32 ddr_end_addr, int page_id, int len, int ddr_chip_a_or_b)
{
    int count, escape;

    //printk("%s.%d:%x, %d, %d, %d\n", __FUNCTION__, __LINE__, ddr_end_addr, page_id, len, ddr_chip_a_or_b);

    count = escape = 0;
    if((burst_interface!=NULL) && (dpram_page!=NULL)){
        isil_chip_t  *chip;
        u32 dpram_page_base;

        chip = burst_interface->chip;
        burst_interface->op->get_page_mode_reg(burst_interface, chip);
        burst_interface->op->enable_access_sram_block(burst_interface);
        burst_interface->op->update_page_mode_reg(burst_interface, chip);
        count = escape = 0;
        burst_interface->op->get_burst_interface_param(burst_interface, chip);
        while(burst_interface->op->get_burst_busy_flag(burst_interface)){
            //mdelay(1);
            burst_interface->op->get_burst_interface_param(burst_interface, chip);
            count++;
            if(count > BURST_INTERFACE_CHECKUP_TIMEOUT){
                printk("%s.%d: can't access burst interface\n", __FUNCTION__, __LINE__);
                escape = 1;
                break;
            }
        }

        if(!escape){
            burst_interface->op->set_ddr_page_number(burst_interface, page_id);
            if(DDR_CHIP_B == ddr_chip_a_or_b){
                burst_interface->op->select_ddr_B(burst_interface);
            } else {
                burst_interface->op->select_ddr_A(burst_interface); 
            }
#if defined(PCI_PLATFORM) && defined(DMA_INTERFACE) && defined(BURST_INTERFACE)
            burst_interface->op->enable_read_4(burst_interface);
#else
            burst_interface->op->disable_read_4(burst_interface);
#endif
            burst_interface->op->update_page_mode_reg(burst_interface, chip);

            dpram_page_base = dpram_page->op->get_page_offset(dpram_page);
            burst_interface->op->set_burst_sram_addr(burst_interface, dpram_page_base);
            burst_interface->op->set_burst_ddr_addr(burst_interface, ddr_end_addr);
            burst_interface->op->int_enable(burst_interface, 0, 0, 0);
            burst_interface->op->set_ddr_burst_param(burst_interface, len, ISIL_READ);
            burst_interface->op->update_burst_interface_param(burst_interface, chip);
            count = escape = 0;
            burst_interface->op->get_burst_interface_param(burst_interface, chip);
            while(burst_interface->op->get_burst_end_flag(burst_interface) == 0){
                //mdelay(1);
                burst_interface->op->get_burst_interface_param(burst_interface, chip);
                count++;
                if(count > BURST_INTERFACE_CHECKUP_TIMEOUT){
                    printk("%s.%d: can't access burst interface\n", __FUNCTION__, __LINE__);
                    escape = 1;
                    break;
                }
            }

            if(escape == 0){
                burst_interface->op->clear_burst_end_flag(burst_interface);
                burst_interface->op->update_burst_interface_param(burst_interface, chip);
            }
        }
        burst_interface->op->clear_burst_done(burst_interface);
    }
    return escape;
}

static int  ddr_burst_interface_reg_start_nonblock_transfer_from_ddr_to_sram(ddr_burst_interface_t *burst_interface, dpram_page_node_t *dpram_page, u32 ddr_end_addr, int page_id, int len, int ddr_chip_a_or_b)
{
    int count, escape;

    count = escape = 0;
    if((burst_interface!=NULL) && (dpram_page!=NULL)){
        isil_chip_t  *chip;
        u32 dpram_page_base;
        chip = burst_interface->chip;
        burst_interface->op->get_page_mode_reg(burst_interface, chip);
        burst_interface->op->enable_access_sram_block(burst_interface);
        burst_interface->op->update_page_mode_reg(burst_interface, chip);

        //printk("%s,%d:addr 0x%08x, len %08x, page id %d %s\n", __FUNCTION__, __LINE__, ddr_end_addr, len, page_id, ddr_chip_a_or_b?"B":"A");
        count = escape = 0;
        burst_interface->op->get_burst_interface_param(burst_interface, chip);
        while(burst_interface->cmd_status_reg_value.bit_value.ddr_burst_busy_flag || burst_interface->cmd_status_reg_value.bit_value.ddr_single_access_busy_flag){
            //mdelay(1);
            burst_interface->op->get_burst_interface_param(burst_interface, chip);
            count++;
            if(count > BURST_INTERFACE_CHECKUP_TIMEOUT){
                printk("%s.%d: can't access burst interface\n", __FUNCTION__, __LINE__);
                escape = 1;
                break;
            }
        }

        if(!escape){
            burst_interface->op->set_ddr_page_number(burst_interface, page_id);
            if(DDR_CHIP_B == ddr_chip_a_or_b){
                burst_interface->op->select_ddr_B(burst_interface);
            } else {
                burst_interface->op->select_ddr_A(burst_interface); 
            }
#if defined(PCI_PLATFORM) && defined(DMA_INTERFACE) && defined(BURST_INTERFACE)
            burst_interface->op->enable_read_4(burst_interface);
#else
            burst_interface->op->disable_read_4(burst_interface);
#endif
            burst_interface->op->update_page_mode_reg(burst_interface, chip);

            dpram_page_base = dpram_page->op->get_page_offset(dpram_page);
            burst_interface->op->set_burst_sram_addr(burst_interface, dpram_page_base);
            burst_interface->op->set_burst_ddr_addr(burst_interface, ddr_end_addr);
            burst_interface->op->int_enable(burst_interface, 0, 0, 1);
            burst_interface->op->set_ddr_burst_param(burst_interface, len, ISIL_READ);
            burst_interface->op->update_burst_interface_param(burst_interface, chip);
        }
    }
    return escape;
}

static void ddr_burst_interface_reg_clear_burst_done(ddr_burst_interface_t *burst_interface)
{
    if(burst_interface != NULL){
        isil_chip_t      *chip;
        chip = burst_interface->chip;
        burst_interface->op->get_page_mode_reg(burst_interface, chip);
        burst_interface->op->enable_access_sram_block(burst_interface);
        burst_interface->op->update_page_mode_reg(burst_interface, chip);

        burst_interface->op->get_burst_interface_param(burst_interface, chip);
        burst_interface->op->clear_burst_end_flag(burst_interface);
        chip->io_op->chip_write32(chip, burst_interface->burst_cmd_status_reg_offset, burst_interface->cmd_status_reg_value.value);
        burst_interface->op->get_burst_interface_param(burst_interface, chip);
    }
}

static struct ddr_burst_interface_reg_operation ddr_burst_interface_reg_op = {
    .init = ddr_burst_interface_reg_init,
    .reset = ddr_burst_interface_reg_reset,
    .get_page_mode_reg = ddr_burst_interface_reg_get_page_mode_reg,
    .update_page_mode_reg = ddr_burst_interface_reg_update_page_mode_reg,
    .get_burst_interface_param = ddr_burst_interface_reg_get_burst_interface_param,
    .update_burst_interface_param = ddr_burst_interface_reg_update_burst_interface_param,

    .enable_access_sram_block = ddr_burst_interface_reg_enable_access_sram_block,
    .enable_access_ddr_block = ddr_burst_interface_reg_enable_access_ddr_block,
    .select_ddr_A = ddr_burst_interface_reg_select_ddr_A,
    .select_ddr_B = ddr_burst_interface_reg_select_ddr_B,
    .enable_read_4 = ddr_burst_interface_reg_enable_read_4,
    .disable_read_4 = ddr_burst_interface_reg_disable_read_4,
    .set_ddr_page_number = ddr_burst_interface_reg_set_ddr_page_number,

    .set_ddr_burst_param = ddr_burst_interface_reg_set_ddr_burst_param,
    .start_ddr_burst = ddr_burst_interface_reg_start_ddr_burst,
    .int_enable = ddr_burst_interface_reg_int_enable,
    .get_single_access_err_flag = ddr_burst_interface_reg_get_single_access_err_flag,
    .get_burst_access_err_flag = ddr_burst_interface_reg_get_burst_access_err_flag,
    .get_burst_busy_flag = ddr_burst_interface_reg_get_burst_busy_flag,
    .get_burst_end_flag = ddr_burst_interface_reg_get_burst_end_flag,
    .clear_burst_end_flag = ddr_burst_interface_reg_clear_burst_end_flag,
    .set_burst_ddr_addr = ddr_burst_interface_reg_set_burst_ddr_addr,
    .set_burst_sram_addr = ddr_burst_interface_reg_set_burst_sram_addr,

    .pio_host_to_sram_write = ddr_burst_interface_reg_pio_host_to_sram_write,
    .dma_host_to_sram_write = ddr_burst_interface_reg_dma_host_to_sram_write,
    .pio_host_to_sram_read = ddr_burst_interface_reg_pio_host_to_sram_read,
    .dma_host_to_sram_read = ddr_burst_interface_reg_dma_host_to_sram_read,

    .start_block_transfer_from_sram_to_ddr = ddr_burst_interface_reg_start_block_transfer_from_sram_to_ddr,
    .start_nonblock_transfer_from_sram_to_ddr = ddr_burst_interface_reg_start_nonblock_transfer_from_sram_to_ddr,
    .start_block_transfer_from_ddr_to_sram = ddr_burst_interface_reg_start_block_transfer_from_ddr_to_sram,
    .start_nonblock_transfer_from_ddr_to_sram = ddr_burst_interface_reg_start_nonblock_transfer_from_ddr_to_sram,
    .clear_burst_done = ddr_burst_interface_reg_clear_burst_done,
};

void    init_ddr_burst_interface_t(ddr_burst_interface_t *burst_interface, isil_chip_t *chip)
{
    if(burst_interface != NULL){
        burst_interface->op = &ddr_burst_interface_reg_op;
        burst_interface->chip = chip;
        burst_interface->op->init(burst_interface);
        burst_interface->op->update_page_mode_reg(burst_interface, chip);
        burst_interface->op->update_burst_interface_param(burst_interface, chip);
    }
}

static u32  dpram_pgae_node_get_page_base(dpram_page_node_t *dpram_page)
{
    return (dpram_page->dpram_base + dpram_page->dpram_page_base_offset);
}

static u32  dpram_page_node_get_page_offset(dpram_page_node_t *dpram_page)
{
    return dpram_page->dpram_page_base_offset;
}

static int  dpram_page_node_get_page_size(dpram_page_node_t *dpram_page)
{
    return dpram_page->page_size;
}

static struct dpram_page_node_operation dpram_page_node_op = {
    .get_page_base = dpram_pgae_node_get_page_base,
    .get_page_offset = dpram_page_node_get_page_offset,
    .get_page_size = dpram_page_node_get_page_size,
};

static void dpram_page_resource_manage_init(dpram_page_resource_manage_t *dpram_page_resource)
{
    if(dpram_page_resource){
        dpram_page_node_t   *dpram;
        u32 page_offset;
        int i;
        dpram = dpram_page_resource->dpram;
        dpram_page_resource->dpram_number = DPRAM_PAGE_NUMBER;
        dpram_page_resource->curr_used_dpram_index = 0;
        atomic_set(&dpram_page_resource->curr_can_use_dpram_page_number, dpram_page_resource->dpram_number);
        spin_lock_init(&dpram_page_resource->lock);
        page_offset = DPRAM_PAGE_SIZE;
        for(i=0; i<dpram_page_resource->dpram_number; i++){
            dpram->dpram_base = MPI_DPRAM_BASE_OFFSET;
            dpram->dpram_page_base_offset = i*page_offset;
            dpram->page_size = page_offset;
            dpram->op = &dpram_page_node_op;
            dpram++;
        }
    }
}

static void dpram_page_resource_manage_get_dpram_page(dpram_page_resource_manage_t *dpram_page_resource, dpram_page_node_t **ptr_dpram_page)
{
    if((dpram_page_resource!=NULL) && (ptr_dpram_page!=NULL)){
        unsigned long   flags;
        spin_lock_irqsave(&dpram_page_resource->lock, flags);
        *ptr_dpram_page = NULL;
        if(atomic_read(&dpram_page_resource->curr_can_use_dpram_page_number)){
            atomic_dec(&dpram_page_resource->curr_can_use_dpram_page_number);
            *ptr_dpram_page = &dpram_page_resource->dpram[dpram_page_resource->curr_used_dpram_index];
            dpram_page_resource->curr_used_dpram_index++;
            if(dpram_page_resource->curr_used_dpram_index >= dpram_page_resource->dpram_number){
                dpram_page_resource->curr_used_dpram_index = 0;
            }
        }
        spin_unlock_irqrestore(&dpram_page_resource->lock, flags);
    }
}

static void dpram_page_resource_manage_put_dpram_page(dpram_page_resource_manage_t *dpram_page_resource, dpram_page_node_t **ptr_dpram_page)
{
    if((dpram_page_resource!=NULL) && (ptr_dpram_page!=NULL)){
        unsigned long   flags;
        spin_lock_irqsave(&dpram_page_resource->lock, flags);
        if(*ptr_dpram_page != NULL){
            *ptr_dpram_page = NULL;
            atomic_inc(&dpram_page_resource->curr_can_use_dpram_page_number);
            if(atomic_read(&dpram_page_resource->curr_can_use_dpram_page_number) > dpram_page_resource->dpram_number){
                atomic_set(&dpram_page_resource->curr_can_use_dpram_page_number, dpram_page_resource->dpram_number);
            }
        }
        spin_unlock_irqrestore(&dpram_page_resource->lock, flags);
    }
}

static int  dpram_page_resource_manage_get_can_use_resource_number(dpram_page_resource_manage_t *dpram_page_resource)
{
    return atomic_read(&dpram_page_resource->curr_can_use_dpram_page_number);
}

static struct dpram_page_resource_manage_operation  dpram_page_resource_manage_op = {
    .init = dpram_page_resource_manage_init,
    .get_dpram_page = dpram_page_resource_manage_get_dpram_page,
    .put_dpram_page = dpram_page_resource_manage_put_dpram_page,
    .get_can_use_resource_number = dpram_page_resource_manage_get_can_use_resource_number,
};

void    init_dpram_page_resource_manage(dpram_page_resource_manage_t *dpram_page_resource)
{
    if(dpram_page_resource != NULL){
        dpram_page_resource->op = &dpram_page_resource_manage_op;
        dpram_page_resource->op->init(dpram_page_resource);
    }
}

static int  ddr_video_bitstream_buf_node_get_buf_id(ddr_video_bitstream_buf_node_t *video_bitstream)
{
    return video_bitstream->buf_id;
}

static int  ddr_video_bitstream_buf_node_get_buf_in_chip_a_or_b(ddr_video_bitstream_buf_node_t *video_bitstream)
{
    return video_bitstream->chip_a_or_b;
}

static int  ddr_video_bitstream_buf_node_get_page_id(ddr_video_bitstream_buf_node_t *video_bitstream)
{
    return video_bitstream->ddr_phy_page_id;
}

static u32  ddr_video_bitstream_buf_node_get_buf_base_offset(ddr_video_bitstream_buf_node_t *video_bitstream)
{
    return video_bitstream->buf_base_offset;
}

static struct ddr_video_bitstream_buf_node_operation    ddr_video_bitstream_buf_node_op = {
    .get_buf_id = ddr_video_bitstream_buf_node_get_buf_id,
    .get_buf_in_chip_a_or_b = ddr_video_bitstream_buf_node_get_buf_in_chip_a_or_b,
    .get_page_id = ddr_video_bitstream_buf_node_get_page_id,
    .get_buf_base_offset = ddr_video_bitstream_buf_node_get_buf_base_offset,
};

static int  map_ddr_page_id_from_buf_id(int buf_id)
{
    int page_id;
    switch(buf_id){
        case 0:
            page_id = 12;
            break;
        case 1:
            page_id = 28;
            break;
        case 2:
            page_id = 44;
            break;
        case 3:
            page_id = 60;
            break;
        case 4:
            page_id = 13;
            break;
        case 5:
            page_id = 29;
            break;
        case 6:
            page_id = 45;
            break;
        default:
            page_id = 61;
            break;
    }
    return page_id;
}

static void ddr_video_bitstream_resource_manage_init(ddr_video_bitstream_resource_manage_t *video_bitstream_resource)
{
    if(video_bitstream_resource != NULL){
        ddr_video_bitstream_buf_node_t  *ddr_buf_section;
        int i;

        ddr_buf_section = video_bitstream_resource->ddr_buf_section;
        video_bitstream_resource->ddr_buf_section_number = VIDEO_BITSTREAM_DDR_SECTION_NUMBER;
        video_bitstream_resource->curr_used_ddr_buf_section_index = 0;
        atomic_set(&video_bitstream_resource->curr_can_use_ddr_buf_section_number, video_bitstream_resource->ddr_buf_section_number);
        spin_lock_init(&video_bitstream_resource->lock);
        for(i=0; i<video_bitstream_resource->ddr_buf_section_number; i++){
            ddr_buf_section->buf_id = i;
            ddr_buf_section->chip_a_or_b = DDR_CHIP_B;
            ddr_buf_section->ddr_phy_page_id = map_ddr_page_id_from_buf_id(i);
            ddr_buf_section->buf_base_offset = ddr_buf_section->ddr_phy_page_id<<19;
            ddr_buf_section->op = &ddr_video_bitstream_buf_node_op;
            ddr_buf_section++;
        }
    }
}

static void ddr_video_bitstream_resource_manage_get_video_bitstream(ddr_video_bitstream_resource_manage_t *video_bitstream_resource, ddr_video_bitstream_buf_node_t **ptr_video_bitstream)
{
    if((video_bitstream_resource!=NULL) && (ptr_video_bitstream!=NULL)){
        unsigned long   flags;
        spin_lock_irqsave(&video_bitstream_resource->lock, flags);
        *ptr_video_bitstream = NULL;
        if(atomic_read(&video_bitstream_resource->curr_can_use_ddr_buf_section_number)){
            atomic_dec(&video_bitstream_resource->curr_can_use_ddr_buf_section_number);
            *ptr_video_bitstream  = &video_bitstream_resource->ddr_buf_section[video_bitstream_resource->curr_used_ddr_buf_section_index];
            video_bitstream_resource->curr_used_ddr_buf_section_index++;
            if(video_bitstream_resource->curr_used_ddr_buf_section_index >= video_bitstream_resource->ddr_buf_section_number){
                video_bitstream_resource->curr_used_ddr_buf_section_index = 0;
            }
        }
        spin_unlock_irqrestore(&video_bitstream_resource->lock, flags);
    }
}

static void ddr_video_bitstream_resource_manage_put_video_bitstream(ddr_video_bitstream_resource_manage_t *video_bitstream_resource, ddr_video_bitstream_buf_node_t **ptr_video_bitstream)
{
    if((video_bitstream_resource!=NULL) && (ptr_video_bitstream!=NULL)){
        unsigned long   flags;
        spin_lock_irqsave(&video_bitstream_resource->lock, flags);
        if(*ptr_video_bitstream != NULL){
            *ptr_video_bitstream = NULL;
            atomic_inc(&video_bitstream_resource->curr_can_use_ddr_buf_section_number);
            if(atomic_read(&video_bitstream_resource->curr_can_use_ddr_buf_section_number) > video_bitstream_resource->ddr_buf_section_number){
                atomic_set(&video_bitstream_resource->curr_can_use_ddr_buf_section_number, video_bitstream_resource->ddr_buf_section_number);
            }
        }
        spin_unlock_irqrestore(&video_bitstream_resource->lock, flags);
    }
}

static int  ddr_video_bitstream_resource_manage_get_can_use_resource_number(ddr_video_bitstream_resource_manage_t *video_bitstream_resource)
{
    return atomic_read(&video_bitstream_resource->curr_can_use_ddr_buf_section_number);
}

static struct ddr_video_bitstream_resource_manage_operation ddr_video_bitstream_resource_manage_op = {
    .init = ddr_video_bitstream_resource_manage_init,
    .get_video_bitstream = ddr_video_bitstream_resource_manage_get_video_bitstream,
    .put_video_bitstream = ddr_video_bitstream_resource_manage_put_video_bitstream,
    .get_can_use_resource_number = ddr_video_bitstream_resource_manage_get_can_use_resource_number,
};

void    init_ddr_video_bitstream_resource_manage(ddr_video_bitstream_resource_manage_t *video_bitstream_resource)
{
    if(video_bitstream_resource != NULL){
        video_bitstream_resource->op = &ddr_video_bitstream_resource_manage_op;
        video_bitstream_resource->op->init(video_bitstream_resource);
    }
}

void    init_dpram_request_service_tcb(dpram_request_t *request_tcb)
{
    if(request_tcb != NULL){
        tcb_node_t  *node = &request_tcb->request_node;
        request_tcb->chip = NULL;
        request_tcb->type = DPRAM_BLOCK_TRANSFER_REQUEST;
        request_tcb->context = NULL;
        request_tcb->req_callback = NULL;
        node->op = &tcb_node_op;
        node->op->init(node);
        node->op->set_priv(node, request_tcb);
    }
}

static void dpram_request_queue_init(dpram_request_queue_t *request_queue)
{
    if(request_queue != NULL){
        tcb_node_queue_t    *request_queue_node = &request_queue->request_queue;
        request_queue->curr_service_req = NULL;
        spin_lock_init(&request_queue->lock);
        request_queue_node->op = &tcb_node_queue_op;
        request_queue_node->op->init(request_queue_node);
    }
}

static int  dpram_request_queue_get_queue_curr_entry_number(dpram_request_queue_t *request_queue)
{
    if(request_queue != NULL) {
        tcb_node_queue_t    *request_queue_node = &request_queue->request_queue;
        return request_queue_node->op->get_queue_curr_entry_number(request_queue_node);
    } else {
        return 0;
    }
}

static void dpram_request_queue_put_service_request_into_queue(dpram_request_queue_t *request_queue, dpram_request_t *request)
{
    if((request_queue!=NULL) && (request!=NULL)){
        tcb_node_queue_t    *request_queue_node = &request_queue->request_queue;
        request_queue_node->op->put(request_queue_node, &request->request_node);
        request_queue->op->trigger_chip_pending_service_request(request_queue);
    }
}

static void dpram_request_queue_put_service_request_into_queue_header(dpram_request_queue_t *request_queue, dpram_request_t *request)
{
    if((request_queue!=NULL) && (request!=NULL)){
        tcb_node_queue_t    *request_queue_node = &request_queue->request_queue;
        request_queue_node->op->put_header(request_queue_node, &request->request_node);
        request_queue->op->trigger_chip_pending_service_request(request_queue);
    }
}

static int  dpram_request_queue_delete_service_request_from_queue(dpram_request_queue_t *request_queue, dpram_request_t *request)
{
    int ret = 0;

    if((request_queue!=NULL) && (request!=NULL)){
        tcb_node_queue_t    *request_queue_node = &request_queue->request_queue;
        unsigned long   flags;
        spin_lock_irqsave(&request_queue->lock, flags);
        if(request_queue->op->get_queue_curr_entry_number(request_queue)){
            ret = request_queue_node->op->delete(request_queue_node, &request->request_node);
            if(ret == 1){
                if(request->delete_req_notify != NULL){
                    request->delete_req_notify(request, request->context);
                }
            }
        }
        spin_unlock_irqrestore(&request_queue->lock, flags);
    }
    return ret;
}

static void dpram_request_queue_try_get_curr_consumer_from_queue(dpram_request_queue_t *request_queue)
{
    if(request_queue != NULL){
        tcb_node_queue_t    *request_queue_node = &request_queue->request_queue;
        if(request_queue->curr_service_req == NULL){
            if(request_queue_node->op != NULL){
                tcb_node_t  *temp_node;
                request_queue_node->op->try_get(request_queue_node, &temp_node);
                if(temp_node != NULL){
                    request_queue->curr_service_req = to_get_dpram_request_with_request_node(temp_node);
                }
            }
        }
    }
}

static void dpram_request_queue_release_curr_consumer(dpram_request_queue_t *request_queue)
{
    if(request_queue != NULL){
        unsigned long	flags;
        spin_lock_irqsave(&request_queue->lock, flags);
        request_queue->curr_service_req = NULL;
        spin_unlock_irqrestore(&request_queue->lock, flags);
    }
}

static void dpram_request_queue_trigger_chip_pending_service_request(dpram_request_queue_t *request_queue)
{
    if(request_queue != NULL){
        isil_chip_t     *chip;
        dpram_request_t *curr_request;
        unsigned long   flags;
        int curr_entry_number;

again:
        spin_lock_irqsave(&request_queue->lock, flags);
        curr_entry_number = request_queue->op->get_queue_curr_entry_number(request_queue);
        if((curr_entry_number==0) && (request_queue->curr_service_req==NULL)){
            spin_unlock_irqrestore(&request_queue->lock, flags);
            return;
        } else if(request_queue->curr_service_req != NULL){
            spin_unlock_irqrestore(&request_queue->lock, flags);
            return;
        }
        request_queue->op->try_get_curr_consumer_from_queue(request_queue);
        curr_request = request_queue->curr_service_req;
        spin_unlock_irqrestore(&request_queue->lock, flags);

        if(curr_request != NULL){
            chip = curr_request->chip;
            if(chip != NULL){
                if(chip_is_in_robust_processing(chip)){
                    if(curr_request->delete_req_notify != NULL){
                        curr_request->delete_req_notify(curr_request, curr_request->context);
                    }
                    request_queue->op->release_curr_consumer(request_queue);
                    goto again;
                } else {
                    if(curr_request->req_callback != NULL){
                        curr_request->req_callback(curr_request, curr_request->context);
                    } else {
                        if(curr_request->delete_req_notify != NULL){
                            curr_request->delete_req_notify(curr_request, curr_request->context);
                        }
                        request_queue->op->release_curr_consumer(request_queue);                    
                        printk("\n\n&&&&&&&&&&&%s.%d&&&&&&&&&&&&,%d\n\n", __FUNCTION__, __LINE__, curr_entry_number);
                    }
                }
            } else {
                request_queue->op->release_curr_consumer(request_queue);
                printk("\n\n%s.%d: curr bus schedule request's chip is null\n\n", __FUNCTION__, __LINE__);
                goto again;
            }
        }
    }
}

static int  dpram_request_queue_service_request_queue_is_empty(dpram_request_queue_t *request_queue)
{
    if(request_queue != NULL) {
        unsigned long   flags;
        int curr_entry_number;

        spin_lock_irqsave(&request_queue->lock, flags);
        curr_entry_number = request_queue->op->get_queue_curr_entry_number(request_queue);
        if((curr_entry_number==0) && (request_queue->curr_service_req==NULL)){
            curr_entry_number = 1;
        } else {
            curr_entry_number = 0;
        }
        spin_unlock_irqrestore(&request_queue->lock, flags);
        return curr_entry_number;
    } else {
        return 0;
    }
}

static struct dpram_request_queue_operation dpram_request_queue_op = {
    .init = dpram_request_queue_init,
    .get_queue_curr_entry_number = dpram_request_queue_get_queue_curr_entry_number,
    .put_service_request_into_queue = dpram_request_queue_put_service_request_into_queue,
    .put_service_request_into_queue_header = dpram_request_queue_put_service_request_into_queue_header,
    .delete_service_request_from_queue = dpram_request_queue_delete_service_request_from_queue,
    .try_get_curr_consumer_from_queue = dpram_request_queue_try_get_curr_consumer_from_queue,
    .release_curr_consumer = dpram_request_queue_release_curr_consumer,
    .trigger_chip_pending_service_request = dpram_request_queue_trigger_chip_pending_service_request,
    .service_request_queue_is_empty = dpram_request_queue_service_request_queue_is_empty,
};

void    init_dpram_request_queue(dpram_request_queue_t *request_queue)
{
    if(request_queue != NULL){
        request_queue->op = &dpram_request_queue_op;
        request_queue->op->init(request_queue);
    }
}

static void chip_ddr_burst_engine_init(chip_ddr_burst_engine_t *chip_ddr_burst_interface, isil_chip_t *chip)
{
    if((chip_ddr_burst_interface!=NULL) && (chip!=NULL)){
        atomic_set(&chip_ddr_burst_interface->bus_state, BUS_IDLE);
        init_ddr_burst_interface_t(&chip_ddr_burst_interface->burst_interface, chip);
        init_dpram_page_resource_manage(&chip_ddr_burst_interface->dpram_resource);
        init_ddr_video_bitstream_resource_manage(&chip_ddr_burst_interface->video_bitstream_resource);
    }
}

static void chip_ddr_burst_engine_get_ddr_burst_interface(chip_ddr_burst_engine_t *chip_ddr_burst_interface, ddr_burst_interface_t **ptr_burst_interface)
{
    if(chip_ddr_burst_interface != NULL){
        *ptr_burst_interface = &chip_ddr_burst_interface->burst_interface;
    }
}

static void chip_ddr_burst_engine_get_dpram_page_resource_manage(chip_ddr_burst_engine_t *chip_ddr_burst_interface, dpram_page_resource_manage_t **ptr_dpram_resource)
{
    if(chip_ddr_burst_interface != NULL){
        *ptr_dpram_resource = &chip_ddr_burst_interface->dpram_resource;
    }    
}

static void chip_ddr_burst_engine_get_ddr_video_bitstream_resource_manage(chip_ddr_burst_engine_t *chip_ddr_burst_interface, ddr_video_bitstream_resource_manage_t **ptr_video_bitstream_resource)
{
    if(chip_ddr_burst_interface != NULL){
        *ptr_video_bitstream_resource = &chip_ddr_burst_interface->video_bitstream_resource;
    } 
}

static int  chip_ddr_burst_engine_is_can_submit_h264_encode_service_req(chip_ddr_burst_engine_t *chip_ddr_burst_interface, ddr_video_bitstream_buf_node_t **ptr_video_bitstream_buf)
{
    int ret = 0;
    if(chip_ddr_burst_interface != NULL){
        ddr_video_bitstream_resource_manage_t   *video_bitstream_resource;

        video_bitstream_resource = &chip_ddr_burst_interface->video_bitstream_resource;
        ret = video_bitstream_resource->op->get_can_use_resource_number(video_bitstream_resource);
        video_bitstream_resource->op->get_video_bitstream(video_bitstream_resource, ptr_video_bitstream_buf);
    }
    return ret;
}

static void chip_ddr_burst_engine_release_h264_ddr_page_resource(chip_ddr_burst_engine_t *chip_ddr_burst_interface, ddr_video_bitstream_buf_node_t **ptr_video_bitstream_buf)
{
    if(chip_ddr_burst_interface != NULL){
        ddr_video_bitstream_resource_manage_t   *video_bitstream_resource;

        video_bitstream_resource = &chip_ddr_burst_interface->video_bitstream_resource;
        video_bitstream_resource->op->put_video_bitstream(video_bitstream_resource, ptr_video_bitstream_buf);
    }
}

static int  chip_ddr_burst_engine_is_can_submit_move_data_from_host_to_dpram_service_req(chip_ddr_burst_engine_t *chip_ddr_burst_interface, dpram_page_node_t **ptr_dpram_page)
{
    int ret = 0;
    if(chip_ddr_burst_interface != NULL){
        dpram_page_resource_manage_t *dpram_resource;
        int bus_state;

        bus_state = atomic_read(&chip_ddr_burst_interface->bus_state);
        if(!(bus_state&DDR_END_BUS_BUSY)){
            dpram_resource = &chip_ddr_burst_interface->dpram_resource;
            ret = dpram_resource->op->get_can_use_resource_number(dpram_resource);
            if(ret){
                bus_state |= DDR_END_BUS_BUSY;
                atomic_set(&chip_ddr_burst_interface->bus_state, bus_state);
                dpram_resource->op->get_dpram_page(dpram_resource, ptr_dpram_page);
            }
        }
    }
    return ret;
}

static void chip_ddr_burst_engine_notify_end_move_data_from_dpram_to_ddr_service_req(chip_ddr_burst_engine_t *chip_ddr_burst_interface, dpram_page_node_t **ptr_dpram_page)
{
    if(chip_ddr_burst_interface != NULL){
        dpram_page_resource_manage_t    *dpram_resource;
        int bus_state;

        dpram_resource = &chip_ddr_burst_interface->dpram_resource;
        dpram_resource->op->put_dpram_page(dpram_resource, ptr_dpram_page);
        bus_state = atomic_read(&chip_ddr_burst_interface->bus_state);
        bus_state &= ~DDR_END_BUS_BUSY;
        atomic_set(&chip_ddr_burst_interface->bus_state, bus_state);
    }
}

static void chip_ddr_burst_engine_release_end_move_data_from_dpram_to_ddr_service_req(chip_ddr_burst_engine_t *chip_ddr_burst_interface, dpram_page_node_t **ptr_dpram_page)
{
    if(chip_ddr_burst_interface != NULL){
        dpram_page_resource_manage_t    *dpram_resource;
        int bus_state;

        dpram_resource = &chip_ddr_burst_interface->dpram_resource;
        dpram_resource->op->put_dpram_page(dpram_resource, ptr_dpram_page);
        bus_state = atomic_read(&chip_ddr_burst_interface->bus_state);
        bus_state &= ~DDR_END_BUS_BUSY;
        atomic_set(&chip_ddr_burst_interface->bus_state, bus_state);
    }
}

static int  chip_ddr_burst_engine_is_can_submit_move_data_from_ddr_to_dpram_service_req(chip_ddr_burst_engine_t *chip_ddr_burst_interface, dpram_page_node_t **ptr_dpram_page)
{
    int ret = 0;
    if(chip_ddr_burst_interface != NULL){
        dpram_page_resource_manage_t *dpram_resource;
        int bus_state;

        bus_state = atomic_read(&chip_ddr_burst_interface->bus_state);
        if(!(bus_state&DDR_END_BUS_BUSY)){
            dpram_resource = &chip_ddr_burst_interface->dpram_resource;
            ret = dpram_resource->op->get_can_use_resource_number(dpram_resource);
            if(ret){
                bus_state |= DDR_END_BUS_BUSY;
                atomic_set(&chip_ddr_burst_interface->bus_state, bus_state);
                dpram_resource->op->get_dpram_page(dpram_resource, ptr_dpram_page);
            }
        }
    }
    return ret;
}

static void chip_ddr_burst_engine_notify_end_move_data_from_dpram_to_host_service_req(chip_ddr_burst_engine_t *chip_ddr_burst_interface, dpram_page_node_t **ptr_dpram_page)
{
    if((chip_ddr_burst_interface!=NULL) && (ptr_dpram_page!=NULL)){
        dpram_page_resource_manage_t *dpram_resource;
        int bus_state;

        dpram_resource = &chip_ddr_burst_interface->dpram_resource;
        dpram_resource->op->put_dpram_page(dpram_resource, ptr_dpram_page);
        bus_state = atomic_read(&chip_ddr_burst_interface->bus_state);
        bus_state &= ~DDR_END_BUS_BUSY;
        atomic_set(&chip_ddr_burst_interface->bus_state, bus_state);
    }
}

static void chip_ddr_burst_engine_release_end_move_data_from_dpram_to_host_service_req(chip_ddr_burst_engine_t *chip_ddr_burst_interface, dpram_page_node_t **ptr_dpram_page)
{
    if((chip_ddr_burst_interface!=NULL) && (ptr_dpram_page!=NULL)){
        dpram_page_resource_manage_t *dpram_resource;
        int bus_state;

        dpram_resource = &chip_ddr_burst_interface->dpram_resource;
        dpram_resource->op->put_dpram_page(dpram_resource, ptr_dpram_page);
        bus_state = atomic_read(&chip_ddr_burst_interface->bus_state);
        bus_state &= ~DDR_END_BUS_BUSY;
        atomic_set(&chip_ddr_burst_interface->bus_state, bus_state);
    }
}

static struct chip_ddr_burst_engine_operation   chip_ddr_burst_engine_op = {
    .init = chip_ddr_burst_engine_init,
    .get_ddr_burst_interface = chip_ddr_burst_engine_get_ddr_burst_interface,
    .get_dpram_page_resource_manage = chip_ddr_burst_engine_get_dpram_page_resource_manage,
    .get_ddr_video_bitstream_resource_manage = chip_ddr_burst_engine_get_ddr_video_bitstream_resource_manage,

    .is_can_submit_h264_encode_service_req = chip_ddr_burst_engine_is_can_submit_h264_encode_service_req,
    .release_h264_ddr_page_resource = chip_ddr_burst_engine_release_h264_ddr_page_resource,
    .is_can_submit_move_data_from_host_to_dpram_service_req = chip_ddr_burst_engine_is_can_submit_move_data_from_host_to_dpram_service_req,
    .notify_end_move_data_from_dpram_to_ddr_service_req = chip_ddr_burst_engine_notify_end_move_data_from_dpram_to_ddr_service_req,
    .release_end_move_data_from_dpram_to_ddr_service_req = chip_ddr_burst_engine_release_end_move_data_from_dpram_to_ddr_service_req,
    .is_can_submit_move_data_from_ddr_to_dpram_service_req = chip_ddr_burst_engine_is_can_submit_move_data_from_ddr_to_dpram_service_req,
    .notify_end_move_data_from_dpram_to_host_service_req = chip_ddr_burst_engine_notify_end_move_data_from_dpram_to_host_service_req,
    .release_end_move_data_from_dpram_to_host_service_req = chip_ddr_burst_engine_release_end_move_data_from_dpram_to_host_service_req,
};

void    init_chip_ddr_burst_engine(chip_ddr_burst_engine_t *chip_ddr_burst_interface, isil_chip_t *chip)
{
    if((chip_ddr_burst_interface!=NULL) && (chip!=NULL)){
        chip_ddr_burst_interface->op = &chip_ddr_burst_engine_op;
        chip_ddr_burst_interface->op->init(chip_ddr_burst_interface, chip);
    }
}

static void dpram_control_init(dpram_control_t *dpram)
{
    spin_lock_init(&dpram->lock);
    init_dpram_request_queue(&dpram->request_queue);
}

static int  dpram_control_is_can_submit_h264_encode_service_req(dpram_control_t *dpram, ddr_video_bitstream_buf_node_t **ptr_video_bitstream_buf, isil_chip_t *chip)
{
    int ret = 0;
    if(dpram != NULL){
        chip_ddr_burst_engine_t *chip_ddr_burst_interface;
        unsigned long   flags;

        spin_lock_irqsave(&dpram->lock, flags);
        chip_ddr_burst_interface = &chip->chip_ddr_burst_interface;
        ret = chip_ddr_burst_interface->op->is_can_submit_h264_encode_service_req(chip_ddr_burst_interface, ptr_video_bitstream_buf);
        spin_unlock_irqrestore(&dpram->lock, flags);
    }
    return ret;
}

static void dpram_control_release_h264_ddr_page_resource(dpram_control_t *dpram, ddr_video_bitstream_buf_node_t **ptr_video_bitstream_buf, isil_chip_t *chip)
{
    if(dpram != NULL){
        chip_ddr_burst_engine_t *chip_ddr_burst_interface;
        unsigned long   flags;

        spin_lock_irqsave(&dpram->lock, flags);
        chip_ddr_burst_interface = &chip->chip_ddr_burst_interface;
        chip_ddr_burst_interface->op->release_h264_ddr_page_resource(chip_ddr_burst_interface, ptr_video_bitstream_buf);
        spin_unlock_irqrestore(&dpram->lock, flags);
    }
}

static int  dpram_control_is_can_submit_move_data_from_host_to_dpram_service_req(dpram_control_t *dpram, dpram_page_node_t **ptr_dpram_page, isil_chip_t *chip)
{
    int ret = 0;
    if(dpram != NULL){
        chip_ddr_burst_engine_t *chip_ddr_burst_interface;
        unsigned long   flags;

        spin_lock_irqsave(&dpram->lock, flags);
        chip_ddr_burst_interface = &chip->chip_ddr_burst_interface;
        ret = chip_ddr_burst_interface->op->is_can_submit_move_data_from_host_to_dpram_service_req(chip_ddr_burst_interface, ptr_dpram_page);
        spin_unlock_irqrestore(&dpram->lock, flags);
    }
    return ret;
}

static void dpram_control_notify_end_move_data_from_dpram_to_ddr_service_req(dpram_control_t *dpram, dpram_page_node_t **ptr_dpram_page, isil_chip_t *chip)
{
    if((dpram!=NULL) && (ptr_dpram_page!=NULL)){
        chip_ddr_burst_engine_t *chip_ddr_burst_interface;
        dpram_request_queue_t   *request_queue;
        unsigned long   flags;

        spin_lock_irqsave(&dpram->lock, flags);
        chip_ddr_burst_interface = &chip->chip_ddr_burst_interface;
        chip_ddr_burst_interface->op->notify_end_move_data_from_dpram_to_ddr_service_req(chip_ddr_burst_interface, ptr_dpram_page);
        spin_unlock_irqrestore(&dpram->lock, flags);

        request_queue = &dpram->request_queue;
        request_queue->op->release_curr_consumer(request_queue);
        request_queue->op->trigger_chip_pending_service_request(request_queue);
    }
}

static void dpram_control_release_end_move_data_from_dpram_to_ddr_service_req(dpram_control_t *dpram, dpram_page_node_t **ptr_dpram_page, isil_chip_t *chip)
{
    if((dpram!=NULL) && (ptr_dpram_page!=NULL)){
        chip_ddr_burst_engine_t *chip_ddr_burst_interface;
        dpram_request_queue_t   *request_queue;
        unsigned long   flags;

        spin_lock_irqsave(&dpram->lock, flags);
        chip_ddr_burst_interface = &chip->chip_ddr_burst_interface;
        chip_ddr_burst_interface->op->release_end_move_data_from_dpram_to_ddr_service_req(chip_ddr_burst_interface, ptr_dpram_page);
        spin_unlock_irqrestore(&dpram->lock, flags);

        request_queue = &dpram->request_queue;
        request_queue->op->release_curr_consumer(request_queue);
    }
}

static int  dpram_control_is_can_submit_move_data_from_ddr_to_dpram_service_req(dpram_control_t *dpram, dpram_page_node_t **ptr_dpram_page, isil_chip_t *chip)
{
    int ret = 0;
    if(dpram != NULL){
        chip_ddr_burst_engine_t *chip_ddr_burst_interface;
        unsigned long   flags;

        spin_lock_irqsave(&dpram->lock, flags);
        chip_ddr_burst_interface = &chip->chip_ddr_burst_interface;
        ret = chip_ddr_burst_interface->op->is_can_submit_move_data_from_ddr_to_dpram_service_req(chip_ddr_burst_interface, ptr_dpram_page);
        spin_unlock_irqrestore(&dpram->lock, flags);
    }
    return ret;
}

static void dpram_control_notify_end_move_data_from_dpram_to_host_service_req(dpram_control_t *dpram, dpram_page_node_t **ptr_dpram_page, isil_chip_t *chip)
{
    if((dpram!=NULL) && (ptr_dpram_page!=NULL)){
        chip_ddr_burst_engine_t *chip_ddr_burst_interface;
        dpram_request_queue_t   *request_queue;
        unsigned long   flags;

        spin_lock_irqsave(&dpram->lock, flags);
        chip_ddr_burst_interface = &chip->chip_ddr_burst_interface;
        chip_ddr_burst_interface->op->notify_end_move_data_from_dpram_to_host_service_req(chip_ddr_burst_interface, ptr_dpram_page);
        spin_unlock_irqrestore(&dpram->lock, flags);

        request_queue = &dpram->request_queue;
        request_queue->op->release_curr_consumer(request_queue);
        request_queue->op->trigger_chip_pending_service_request(request_queue);
    }
}

static void dpram_control_release_end_move_data_from_dpram_to_host_service_req(dpram_control_t *dpram, dpram_page_node_t **ptr_dpram_page, isil_chip_t *chip)
{
    if((dpram!=NULL) && (ptr_dpram_page!=NULL)){
        chip_ddr_burst_engine_t *chip_ddr_burst_interface;
        dpram_request_queue_t   *request_queue;
        unsigned long   flags;

        spin_lock_irqsave(&dpram->lock, flags);
        chip_ddr_burst_interface = &chip->chip_ddr_burst_interface;
        chip_ddr_burst_interface->op->release_end_move_data_from_dpram_to_host_service_req(chip_ddr_burst_interface, ptr_dpram_page);
        spin_unlock_irqrestore(&dpram->lock, flags);

        request_queue = &dpram->request_queue;
        request_queue->op->release_curr_consumer(request_queue);
    }
}

static void dpram_control_submit_read_h264_bitstream_req(dpram_control_t *dpram, isil_h264_logic_encode_chan_t *h264_logic_encode_chan)
{
    if((dpram!=NULL) && (h264_logic_encode_chan!=NULL)){
        dpram_request_queue_t   *request_queue;
        request_queue = &dpram->request_queue;
        request_queue->op->put_service_request_into_queue(request_queue, &h264_logic_encode_chan->read_video_bitstream_req);
    }
}

static void dpram_control_ack_read_h264_bitstream_req(dpram_control_t *dpram, isil_h264_logic_encode_chan_t *h264_logic_encode_chan)
{
    if((dpram!=NULL) && (h264_logic_encode_chan!=NULL)){
        //dpram_request_queue_t   *request_queue;
        //request_queue = &dpram->request_queue;
        dpram->op->release_h264_ddr_page_resource(dpram, &h264_logic_encode_chan->video_bitstream_buf, h264_logic_encode_chan->chip);
        //request_queue->op->release_curr_consumer(request_queue);
        dpram->op->notify_end_move_data_from_dpram_to_host_service_req(dpram, &h264_logic_encode_chan->dpram_page, h264_logic_encode_chan->chip);
    }
}

static int  dpram_control_delete_read_h264_bitstream_req(dpram_control_t *dpram, isil_h264_logic_encode_chan_t *h264_logic_encode_chan)
{
    int ret = 0;

    if((dpram!=NULL) && (h264_logic_encode_chan!=NULL)){
        dpram_request_queue_t   *request_queue;
        request_queue = &dpram->request_queue;
        ret = request_queue->op->delete_service_request_from_queue(request_queue, &h264_logic_encode_chan->read_video_bitstream_req);
    }
    return ret;
}


static void dpram_control_submit_write_video_encode_osd_req(dpram_control_t *dpram, osd_rectangle_entry_t *rectangle)
{
    if((dpram!=NULL) && (rectangle!=NULL)){
        dpram_request_queue_t       *request_queue;
        osd_rectangle_dpram_tcb_t   *osd_dpram_req;

        request_queue = &dpram->request_queue;
        osd_dpram_req = &rectangle->osd_dpram_req;
        request_queue->op->put_service_request_into_queue(request_queue, &osd_dpram_req->osd_data_req);
    }
}

static void dpram_control_ack_write_video_encode_osd_req(dpram_control_t *dpram, osd_rectangle_entry_t *rectangle)
{
    if((dpram!=NULL) && (rectangle!=NULL)){
        osd_rectangle_dpram_tcb_t   *osd_dpram_req;

        osd_dpram_req = &rectangle->osd_dpram_req;
        dpram->op->notify_end_move_data_from_dpram_to_ddr_service_req(dpram, &osd_dpram_req->dpram_page, rectangle->chip);
    }
}

static int  dpram_control_delete_wirte_video_encode_osd_data_req(dpram_control_t *dpram, osd_rectangle_entry_t *rectangle)
{
    int ret = 0;
    if((dpram!=NULL) && (rectangle!=NULL)){
        dpram_request_queue_t       *request_queue;
        osd_rectangle_dpram_tcb_t   *osd_dpram_req;

        request_queue = &dpram->request_queue;
        osd_dpram_req = &rectangle->osd_dpram_req;
        ret = request_queue->op->delete_service_request_from_queue(request_queue, &osd_dpram_req->osd_data_req);
    }
    return ret;
}

static void dpram_control_submit_write_video_encode_osd_attr_req(dpram_control_t *dpram, osd_attribute_regs_group_t *attr_group)
{
    if((dpram!=NULL) && (attr_group!=NULL)){
        dpram_request_queue_t       *request_queue;
        osd_rectangle_dpram_tcb_t   *osd_dpram_req;

        request_queue = &dpram->request_queue;
        osd_dpram_req = &attr_group->osd_dpram_req;
        request_queue->op->put_service_request_into_queue(request_queue, &osd_dpram_req->osd_data_req);
    }
}

static void dpram_control_ack_write_video_encode_osd_attr_req(dpram_control_t *dpram, osd_attribute_regs_group_t *attr_group)
{
    if((dpram!=NULL) && (attr_group!=NULL)){
        osd_rectangle_dpram_tcb_t   *osd_dpram_req;
        osd_chan_engine_t           *osd_chan_engine;

        osd_dpram_req = &attr_group->osd_dpram_req;
        osd_chan_engine = to_osd_chan_engine_with_osd_attribute_regs_group(attr_group);
        dpram->op->notify_end_move_data_from_dpram_to_ddr_service_req(dpram, &osd_dpram_req->dpram_page, osd_chan_engine->encode_chan->chip);
    }
}

static int  dpram_control_delete_write_video_encode_osd_attr_req(dpram_control_t *dpram, osd_attribute_regs_group_t *attr_group)
{
    int ret = 0;

    if((dpram!=NULL) && (attr_group!=NULL)){
        dpram_request_queue_t       *request_queue;
        osd_rectangle_dpram_tcb_t   *osd_dpram_req;

        request_queue = &dpram->request_queue;
        osd_dpram_req = &attr_group->osd_dpram_req;
        ret = request_queue->op->delete_service_request_from_queue(request_queue, &osd_dpram_req->osd_data_req);
    }
    return ret;
}

static void dpram_control_submit_read_audio_req(dpram_control_t *dpram, isil_audio_driver_t *audio_driver)
{
    if((dpram!=NULL) && (audio_driver!=NULL)){
        dpram_request_queue_t   *request_queue;
        request_queue = &dpram->request_queue;
        request_queue->op->put_service_request_into_queue(request_queue, &audio_driver->audio_bitstream_req);
        //printk("%s.%d\n", __FUNCTION__, __LINE__);
    }
}

static void dpram_control_ack_read_audio_req(dpram_control_t *dpram, isil_audio_driver_t *audio_driver)
{
    if((dpram!=NULL) && (audio_driver!=NULL)){
        //dpram_request_queue_t   *request_queue;
        //request_queue = &dpram->request_queue;
        //request_queue->op->release_curr_consumer(request_queue);
        dpram->op->notify_end_move_data_from_dpram_to_host_service_req(dpram, &audio_driver->dpram_page, audio_driver->chip_audio->chip);
    }
}

static int  dpram_control_delete_read_audio_req(dpram_control_t *dpram, isil_audio_driver_t *audio_driver)
{
    int ret = 0;

    if((dpram!=NULL) && (audio_driver!=NULL)){
        dpram_request_queue_t       *request_queue;
        request_queue = &dpram->request_queue;
        ret = request_queue->op->delete_service_request_from_queue(request_queue, &audio_driver->audio_bitstream_req);
    }   
    return ret;
}

static void dpram_control_submit_write_audio_req(dpram_control_t *dpram, isil_audio_driver_t *audio_driver)
{
    if((dpram!=NULL) && (audio_driver!=NULL)){
        dpram_request_queue_t   *request_queue;
        request_queue = &dpram->request_queue;
        request_queue->op->put_service_request_into_queue(request_queue, &audio_driver->audio_bitstream_req);
    }
}

static void dpram_control_ack_write_audio_req(dpram_control_t *dpram, isil_audio_driver_t *audio_driver)
{
    if((dpram!=NULL) && (audio_driver!=NULL)){
        //dpram_request_queue_t   *request_queue;
        //request_queue = &dpram->request_queue;
        //request_queue->op->release_curr_consumer(request_queue);
        dpram->op->notify_end_move_data_from_dpram_to_ddr_service_req(dpram, &audio_driver->dpram_page, audio_driver->chip_audio->chip);
    }
}

static int  dpram_control_delete_write_audio_req(dpram_control_t *dpram, isil_audio_driver_t *audio_driver)
{
    int ret = 0;

    if((dpram!=NULL) && (audio_driver!=NULL)){
        dpram_request_queue_t       *request_queue;
        request_queue = &dpram->request_queue;
        ret = request_queue->op->delete_service_request_from_queue(request_queue, &audio_driver->audio_bitstream_req);
    }   
    return ret;
}

static void dpram_control_submit_read_mjpeg_req(dpram_control_t *dpram, isil_jpeg_logic_encode_chan_t *jpeg_logic_chan)
{
    if((dpram!=NULL) && (jpeg_logic_chan!=NULL)){
        dpram_request_queue_t   *request_queue;
        request_queue = &dpram->request_queue;
        request_queue->op->put_service_request_into_queue(request_queue, &jpeg_logic_chan->read_video_bitstream_req);
    }
}

static void dpram_control_ack_read_mjpeg_req(dpram_control_t *dpram, isil_jpeg_logic_encode_chan_t *jpeg_logic_chan)
{
    if((dpram!=NULL) && (jpeg_logic_chan!=NULL)){
        //dpram_request_queue_t   *request_queue;
        //request_queue = &dpram->request_queue;
        //request_queue->op->release_curr_consumer(request_queue);
        dpram->op->notify_end_move_data_from_dpram_to_host_service_req(dpram, &jpeg_logic_chan->dpram_page, jpeg_logic_chan->chip);
    }
}

static int  dpram_control_delete_read_mjpeg_req(dpram_control_t *dpram, isil_jpeg_logic_encode_chan_t *jpeg_logic_chan)
{
    int ret = 0;

    if((dpram!=NULL) && (jpeg_logic_chan!=NULL)){
        dpram_request_queue_t   *request_queue;
        request_queue = &dpram->request_queue;
        request_queue->op->delete_service_request_from_queue(request_queue, &jpeg_logic_chan->read_video_bitstream_req);
    }
    return ret;
}

#ifdef  MV_MODULE
static void dpram_control_submit_read_mvflag_req(dpram_control_t *dpram, isil_h264_logic_encode_chan_t *h264_logic_chan)
{
    if((dpram!=NULL) && (h264_logic_chan!=NULL)){
        dpram_request_queue_t   *request_queue;
        request_queue = &dpram->request_queue;
        request_queue->op->put_service_request_into_queue(request_queue, &h264_logic_chan->read_mvflag_req);
    }
}

static void dpram_control_ack_read_mvflag_req(dpram_control_t *dpram, isil_h264_logic_encode_chan_t *h264_logic_chan)
{
    if((dpram!=NULL) && (h264_logic_chan!=NULL)){
        //dpram_request_queue_t   *request_queue;
        //request_queue = &dpram->request_queue;
        //request_queue->op->release_curr_consumer(request_queue);
        dpram->op->notify_end_move_data_from_dpram_to_host_service_req(dpram, &h264_logic_chan->dpram_page);
    }
}

static int  dpram_control_delete_read_mvflag_req(dpram_control_t *dpram, isil_h264_logic_encode_chan_t *h264_logic_chan)
{
    int ret = 0;

    if((dpram!=NULL) && (h264_logic_chan!=NULL)){
        dpram_request_queue_t   *request_queue;
        request_queue = &dpram->request_queue;
        if (request_queue->op->delete_service_request_from_queue(request_queue, &h264_logic_chan->read_mvflag_req)) {
            complete_all(&h264_logic_chan->mvflag_transfer_done);
        }
    }
    return ret;
}
#endif

static struct dpram_control_operation   dpram_control_op = {
    .init = dpram_control_init,
    .is_can_submit_h264_encode_service_req = dpram_control_is_can_submit_h264_encode_service_req,
    .release_h264_ddr_page_resource = dpram_control_release_h264_ddr_page_resource,
    .is_can_submit_move_data_from_host_to_dpram_service_req = dpram_control_is_can_submit_move_data_from_host_to_dpram_service_req,
    .notify_end_move_data_from_dpram_to_ddr_service_req = dpram_control_notify_end_move_data_from_dpram_to_ddr_service_req,
    .release_end_move_data_from_dpram_to_ddr_service_req = dpram_control_release_end_move_data_from_dpram_to_ddr_service_req,
    .is_can_submit_move_data_from_ddr_to_dpram_service_req = dpram_control_is_can_submit_move_data_from_ddr_to_dpram_service_req,
    .notify_end_move_data_from_dpram_to_host_service_req = dpram_control_notify_end_move_data_from_dpram_to_host_service_req,
    .release_end_move_data_from_dpram_to_host_service_req = dpram_control_release_end_move_data_from_dpram_to_host_service_req,

    .submit_read_h264_bitstream_req = dpram_control_submit_read_h264_bitstream_req,
    .ack_read_h264_bitstream_req = dpram_control_ack_read_h264_bitstream_req,
    .delete_read_h264_bitstream_req = dpram_control_delete_read_h264_bitstream_req,

    .submit_write_video_encode_osd_data_req = dpram_control_submit_write_video_encode_osd_req,
    .ack_write_video_encode_osd_data_req = dpram_control_ack_write_video_encode_osd_req,
    .delete_wirte_video_encode_osd_data_req = dpram_control_delete_wirte_video_encode_osd_data_req,

    .submit_write_video_encode_osd_attr_req = dpram_control_submit_write_video_encode_osd_attr_req,
    .ack_write_video_encode_osd_attr_req = dpram_control_ack_write_video_encode_osd_attr_req,
    .delete_write_video_encode_osd_attr_req = dpram_control_delete_write_video_encode_osd_attr_req,

    .submit_read_audio_req = dpram_control_submit_read_audio_req,
    .ack_read_audio_req = dpram_control_ack_read_audio_req,
    .delete_read_audio_req = dpram_control_delete_read_audio_req,

    .submit_write_audio_req = dpram_control_submit_write_audio_req,
    .ack_write_audio_req = dpram_control_ack_write_audio_req,
    .delete_write_audio_req = dpram_control_delete_write_audio_req,

    .submit_read_mjpeg_req = dpram_control_submit_read_mjpeg_req,
    .ack_read_mjpeg_req = dpram_control_ack_read_mjpeg_req,
    .delete_read_mjpeg_req = dpram_control_delete_read_mjpeg_req,
#ifdef  MV_MODULE
    .submit_read_mvflag_req = dpram_control_submit_read_mvflag_req,
    .ack_read_mvflag_req = dpram_control_ack_read_mvflag_req,
    .delete_read_mvflag_req = dpram_control_delete_read_mvflag_req,
#endif    
};

void    init_dpram_control(dpram_control_t *dpram)
{
    if(dpram != NULL){
        dpram->op = &dpram_control_op;
        dpram->op->init(dpram);
    }
}

