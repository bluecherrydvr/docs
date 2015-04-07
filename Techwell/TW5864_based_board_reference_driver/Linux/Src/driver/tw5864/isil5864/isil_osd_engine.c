#include    <isil5864/isil_common.h>

extern void get_ascii_lib12_addr(unsigned char **addr_ptr, unsigned char* char_id);
extern void get_lib12_addr(unsigned char **addr_ptr, unsigned char* lib16_id);
extern void get_ascii_lib24_addr(unsigned char **addr_ptr, unsigned char* char_id);
extern void get_lib24_addr(unsigned char **addr_ptr, unsigned char* lib16_id);

#define	YEAR_STRING_LEN         (11)
#define	TIME_STRING_LEN         (9)
#define	CHANNEL_STRING_LEN      (10)
#define	YEAR_TIME_STRING_LEN    (YEAR_STRING_LEN+TIME_STRING_LEN+CHANNEL_STRING_LEN+4+40)
#define	UPDATE_OSD_DATA_SECTION (1)
#define	UPDATE_OSD_ATTR_SECTION (2)

#ifdef  USED_STATIC_OSD_CHAR_BUF
static osd_mb_char_entry_t  osd_md_cache[MAX_CHIP_NUM_IN_BOARD][OSD_MB_ENTRY_NUMBER];
#endif

static u32  get_osd_chan_base(int phy_slot_id)
{
    u32 chan_base;
    chan_base = (((phy_slot_id>>2)<<3)+(phy_slot_id&0x3));
    chan_base <<= 20;
    return chan_base;
}

static u32  get_phy_slot_id_by_osd_chan_base(isil_vd_cross_bus_t *vd_bus, u32 chan_base_addr, int osd_slot_id, int IsDdrCompressMode)
{
    int phy_slot_id, ddr_page_id, logic_enc_ch_id;
    isil_h264_phy_video_slot_t *phy_video_slot;
    int i = 0, ch_base = 0;;

    ddr_page_id = chan_base_addr>>19;
    logic_enc_ch_id = ddr_page_id>>4;
    ch_base = 0;
    if(IsDdrCompressMode == DDR_MAP_COMPRESS_ENABLE){
        for(i = 0; i < logic_enc_ch_id; i++) {
            vd_bus->op->get_phy_video_slot_by_phy_id(vd_bus, ch_base, &phy_video_slot);
            switch (phy_video_slot->op->get_video_size(phy_video_slot)) {
                default:
                case ISIL_VIDEO_SIZE_4CIF:
                case ISIL_VIDEO_SIZE_D1:
                    ch_base += 1;
                    break;
                case ISIL_VIDEO_SIZE_2CIF:
                case ISIL_VIDEO_SIZE_HALF_D1:
                    ch_base += 2;
                    break;
                case ISIL_VIDEO_SIZE_CIF:
                    ch_base += 4;
                    break;
            }
        }
        logic_enc_ch_id = ch_base;
    }else{
        logic_enc_ch_id = (osd_slot_id >> 2);
        logic_enc_ch_id <<= 0;
    }
    phy_slot_id = logic_enc_ch_id;
    return phy_slot_id;
}

static u32  get_osd_rectangel_data_base(int rect_id, int videoSizeMode, int MasterOrSub, u32 chan_base)
{
    u32 osd_rectangle_data_base;
    switch(videoSizeMode){
        default:
        case ISIL_VIDEO_SIZE_4CIF:
        case ISIL_VIDEO_SIZE_D1:
            if(rect_id <=2){
                osd_rectangle_data_base = (rect_id<<18) + 0x30000;
            } else {
                osd_rectangle_data_base = 0xc0000 + ((rect_id-3)<<16);
            }
            if(MasterOrSub == ISIL_SUB_BITSTREAM){
                osd_rectangle_data_base += 0x8000;
            }
            break;
        case ISIL_VIDEO_SIZE_2CIF:
        case ISIL_VIDEO_SIZE_HALF_D1:
        case ISIL_VIDEO_SIZE_CIF:
            osd_rectangle_data_base = (rect_id<<17) + 0x10000;
            if(MasterOrSub == ISIL_SUB_BITSTREAM){
                osd_rectangle_data_base += 0xa000;
            } else {
                osd_rectangle_data_base += 0x2000;
            }
            break;
    }
    osd_rectangle_data_base += chan_base;
    return osd_rectangle_data_base;
}

static void get_osd_rectangel_data_base_and_offset(u32 rectangle_base_addr, int osd_mode, int osd_map_mode, int *osd_data_0_8_base_addr, int *osd_data_addr_offset_0_2)
{
    u32 rectangle_base_dword_addr = rectangle_base_addr>>2;
    switch(osd_mode){
        case ENCODE_OSD_MODE0:
        case ENCODE_OSD_MODE1:
            switch(osd_map_mode){
                case Y8MB_X128MB_OSDMODE013:
                    *osd_data_addr_offset_0_2 = ((rectangle_base_dword_addr>>11)&0x7);
                    *osd_data_0_8_base_addr = ((rectangle_base_dword_addr>>14)&0x1ff);
                    break;
                case Y16MB_X64MB_OSDMODE013:
                    *osd_data_addr_offset_0_2 = ((rectangle_base_dword_addr>>11)&0x7);
                    *osd_data_0_8_base_addr = ((rectangle_base_dword_addr>>14)&0x1ff);
                    break;
                case Y32MB_X32MB_OSDMODE013:
                    *osd_data_addr_offset_0_2 = ((rectangle_base_dword_addr>>11)&0x7);
                    *osd_data_0_8_base_addr = ((rectangle_base_dword_addr>>14)&0x1ff);
                    break;
                default:
                case Y64MB_X16MB_OSDMODE013:
                    *osd_data_addr_offset_0_2 = ((rectangle_base_dword_addr>>11)&0x7);
                    *osd_data_0_8_base_addr = ((rectangle_base_dword_addr>>14)&0x1ff);
                    break;
            }
            break;
        case ENCODE_OSD_MODE2:
            switch(osd_map_mode){
                case Y8MB_X128MB_OSDMODE013:
                    *osd_data_0_8_base_addr = ((rectangle_base_dword_addr>>14)&0x1ff);
                    break;
                case Y16MB_X64MB_OSDMODE013:
                    *osd_data_0_8_base_addr = ((rectangle_base_dword_addr>>14)&0x1ff);
                    break;
                case Y32MB_X32MB_OSDMODE013:
                    *osd_data_0_8_base_addr = ((rectangle_base_dword_addr>>15)&0xff);
                    break;
                default:
                case Y64MB_X16MB_OSDMODE013:
                    *osd_data_0_8_base_addr = ((rectangle_base_dword_addr>>15)&0xff);
                    break;
            }
            *osd_data_addr_offset_0_2 = 0;
            break;
        default:
        case ENCODE_OSD_MODE3:
            *osd_data_0_8_base_addr = 0;
            *osd_data_addr_offset_0_2 = 0;
            break;
    }
}

static u32  get_osd_rectangle_mb_data_base(int osd_mode, int osd_map_mode, int mb_x, int mb_y, int rectangle_data_base)
{
    u32 osd_md_data_base, mb_bias;
    osd_md_data_base = (rectangle_data_base>>2);
    switch(osd_mode){
        case ENCODE_OSD_MODE0:
        case ENCODE_OSD_MODE1:
            switch(osd_map_mode){
                case Y8MB_X128MB_OSDMODE013:
                    mb_bias = (((mb_y&0x7)<<11)|((mb_x&0x7f)<<4));
                    break;
                case Y16MB_X64MB_OSDMODE013:
                    mb_bias = (((mb_y&0xf)<<10)|((mb_x&0x3f)<<4));
                    break;
                case Y32MB_X32MB_OSDMODE013:
                    mb_bias = (((mb_y&0x1f)<<9)|((mb_x&0x1f)<<4));
                    break;
                default:
                case Y64MB_X16MB_OSDMODE013:
                    mb_bias = (((mb_y&0x3f)<<8)|((mb_x&0xf)<<4));
                    break;
            }
            break;
        case ENCODE_OSD_MODE2:
            switch(osd_map_mode){
                case Y8MB_X128MB_OSDMODE013:
                    mb_bias = (((mb_y&0x7)<<11)|((mb_x&0xf)<<7));
                    break;
                case Y16MB_X64MB_OSDMODE013:
                    mb_bias = (((mb_y&0xf)<<10)|((mb_x&0x7)<<7));
                    break;
                case Y32MB_X32MB_OSDMODE013:
                    mb_bias = (((mb_y&0xf)<<11)|((mb_x&0xf)<<7));
                    break;
                default:
                case Y64MB_X16MB_OSDMODE013:
                    mb_bias = (((mb_y&0xf)<<11)|((mb_x&0xf)<<7));
                    break;
            }
            break;
        default:
        case ENCODE_OSD_MODE3:
            mb_bias = 0;
            break;
    }
    osd_md_data_base += mb_bias;
    osd_md_data_base <<= 2;
    return osd_md_data_base;
}

static void get_ddr_page_id_and_page_offset_by_md_data_base(u32 osd_md_data_base, int *ddr_page_id, int *ddr_page_offset)
{
    *ddr_page_id = (osd_md_data_base>>19);
    *ddr_page_offset = (osd_md_data_base&0x7ffff);
}

static void osd_timer_attr_reset(encode_osd_timer_attr_t *timer_attr, int chip_id, int chan_id)
{
    timer_attr->timer->need_update_map_table |= (1<<chan_id);
}

static struct encode_osd_timer_attr_operation   osd_timer_attr_op = {
    .reset = osd_timer_attr_reset,
};

static void osd_string_attr_reset(encode_osd_string_attr_t *string_attr)
{
    string_attr->mb_x = 0;
    string_attr->mb_y = 0;
    string_attr->mb_height = 0;
    string_attr->mb_width = 0;
    string_attr->need_update_flag = 0;
    string_attr->data_addr_map_mode = Y8MB_X128MB_OSDMODE013;
    if(string_attr->string != NULL){
        string_attr->string = NULL;
    }
}

static struct encode_osd_string_attr_operation	osd_string_attr_op = {
    .reset = osd_string_attr_reset,
};

static void osd_mask_attr_reset(encode_osd_mask_attr_t *mask_attr)
{
    mask_attr->mask_height = 0;
    mask_attr->mask_width = 0;
    mask_attr->mb_x = 0;
    mask_attr->mb_y = 0;
    mask_attr->y_value = 0;
    mask_attr->u_value = 0;
    mask_attr->v_value = 0;
    mask_attr->need_update_flag = 0;
    mask_attr->is_display = 0;
}

static struct encode_osd_mask_attr_operation    osd_mask_attr_op = {
    .reset = osd_mask_attr_reset,
};

static void osd_attribute_regs_set(osd_attribute_regs_t *attr_regs)
{
    if(attr_regs != NULL){
        attr_regs->reg1_value.reg1.enable = 1;
    }
}

static void osd_attribute_regs_set_no_display(osd_attribute_regs_t *attr_regs)
{
    if(attr_regs != NULL){
        attr_regs->reg1_value.reg1.enable = 0;
    }
}

static void osd_attribute_regs_reset(osd_attribute_regs_t *attr_regs)
{
    if(attr_regs != NULL){
        attr_regs->reg1_value.value = 0;
        attr_regs->reg2_value.value = 0;
        attr_regs->reg3_value.value = 0;
    }
}

static struct osd_attribute_regs_operation  osd_attribute_regs_op = {
    .set = osd_attribute_regs_set,
    .set_no_display = osd_attribute_regs_set_no_display,
    .reset = osd_attribute_regs_reset,
};

static int  osd_attr_burst_done_isr(int irq, void *context)
{
    osd_rectangle_dpram_tcb_t   *osd_dpram_req = (osd_rectangle_dpram_tcb_t*)context;
    int ret = 0;
    if(osd_dpram_req != NULL){
        osd_attribute_regs_group_t  *attr_regs_group = to_osd_attribute_regs_group_with_osd_dpram_req(osd_dpram_req);
        osd_chan_engine_t           *osd_chan_engine = to_osd_chan_engine_with_osd_attribute_regs_group(attr_regs_group);
        isil_chip_t     *chip = osd_chan_engine->encode_chan->chip;
        unsigned long   flags;

        spin_lock_irqsave(&osd_chan_engine->osd_lock, flags);
        chip_free_irq(chip, IRQ_BURST_TYPE_INTR, osd_dpram_req);
        chip->io_op->chip_write32(chip, OSD_ATTRI_BASE_REG, OSD_ATTR_REG_BASE_ADDR);
        chip->io_op->chip_write32(chip, OSD_RECTANGLE_EN_REG, 0xff);
        chip->io_op->chip_write32(chip, OSD_ATTRI_UPDATE_REG, osd_chan_engine->encode_chan->phy_slot_id);
        osd_dpram_req->op->response_write_osd_rectangle(osd_dpram_req, chip);
        spin_unlock_irqrestore(&osd_chan_engine->osd_lock, flags);
        ret = 1;
    }
    return ret;
}

static int  osd_encode_chan_start_send_attr_reg_value(dpram_request_t *dpram_req, void *context)
{
    if((dpram_req!=NULL) && (context!=NULL)){
        osd_rectangle_dpram_tcb_t   *osd_dpram_req = (osd_rectangle_dpram_tcb_t*)context;
        osd_attribute_regs_group_t  *attr_regs_group = to_osd_attribute_regs_group_with_osd_dpram_req(osd_dpram_req);
        osd_chan_engine_t           *osd_chan_engine = to_osd_chan_engine_with_osd_attribute_regs_group(attr_regs_group);
        isil_chip_t                 *chip = osd_chan_engine->encode_chan->chip;
        dpram_control_t             *chip_dpram_controller;

        get_isil_dpram_controller(&chip_dpram_controller);
        if(chip_dpram_controller->op->is_can_submit_move_data_from_host_to_dpram_service_req(chip_dpram_controller, &osd_dpram_req->dpram_page, chip)){
            osd_dpram_req->op->submit_write_osd_rectangle_attr(osd_dpram_req, chip);
            chip->io_op->chip_write32(chip, OSD_ATTRI_BASE_REG, OSD_ATTR_REG_BASE_ADDR);
            chip->io_op->chip_write32(chip, OSD_RECTANGLE_EN_REG, 0xff);
            chip->io_op->chip_write32(chip, OSD_ATTRI_UPDATE_REG, osd_chan_engine->encode_chan->phy_slot_id);
            osd_dpram_req->op->response_write_osd_rectangle(osd_dpram_req, chip);
        } else {
            atomic_set(&osd_chan_engine->sync_osd_param, 1);
            osd_dpram_req->op->response_write_osd_rectangle(osd_dpram_req, chip);
            ISIL_DBG(ISIL_DBG_DEBUG, "retrigger\n");
        }
    }
    return 1;
}

static int  osd_encode_chan_delete_send_attr_reg_value(dpram_request_t *dpram_req, void *context)
{
    int ret = 1;
    if((dpram_req!=NULL) && (context!=NULL)){
        osd_rectangle_dpram_tcb_t   *osd_dpram_req = (osd_rectangle_dpram_tcb_t*)context;
        osd_attribute_regs_group_t  *attr_regs_group = to_osd_attribute_regs_group_with_osd_dpram_req(osd_dpram_req);
        osd_chan_engine_t           *osd_chan_engine = to_osd_chan_engine_with_osd_attribute_regs_group(attr_regs_group);

        if(atomic_read(&osd_chan_engine->osd_working) > 0){
            atomic_dec(&osd_chan_engine->osd_working);
        }
        osd_chan_engine->osd_rectangle_need_transfer_data_flag &= ~(1<<ATTRIBUT_DATA_ID);
        osd_chan_engine->osd_rectangle_need_transfer_data_flag |= (1<<ATTRIBUT_DONE_ID);
        if((atomic_read(&osd_chan_engine->osd_working)==0) &&
                ((osd_chan_engine->osd_rectangle_need_transfer_data_flag&OSD_RECTANGLE_AND_ATTR_MASK)==0)){
            complete_all(&osd_chan_engine->osd_transfering_done);
        }
    }
    return ret;
}

static void osd_rectangle_attr_regs_group_dpram_tcb_init(osd_rectangle_dpram_tcb_t *osd_dpram_req, isil_chip_t *chip)
{
    if(osd_dpram_req != NULL){
        dpram_request_t *dpram_req;
        dpram_req = &osd_dpram_req->osd_data_req;
        dpram_req->chip = chip;
        dpram_req->type = DPRAM_NONBLOCK_TRANSFER_REQUEST;
        dpram_req->context = (void*)osd_dpram_req;
        dpram_req->req_callback = osd_encode_chan_start_send_attr_reg_value;
        dpram_req->delete_req_notify = osd_encode_chan_delete_send_attr_reg_value;
    }
}

static void osd_rectangle_attr_regs_group_dpram_tcb_submit_write_osd_rectangle_data_req(osd_rectangle_dpram_tcb_t *osd_dpram_req, isil_chip_t *chip)
{
    if((osd_dpram_req!=NULL) && (chip!=NULL)){
        osd_attribute_regs_group_t  *attr_regs_group = to_osd_attribute_regs_group_with_osd_dpram_req(osd_dpram_req);
        osd_chan_engine_t           *osd_chan_engine = to_osd_chan_engine_with_osd_attribute_regs_group(attr_regs_group);
        dpram_control_t             *chip_dpram_controller;

        atomic_inc(&osd_chan_engine->osd_working);
        init_completion(&osd_chan_engine->osd_transfering_done);
        get_isil_dpram_controller(&chip_dpram_controller);
        osd_dpram_req->op->init(osd_dpram_req, chip);
        chip_dpram_controller->op->submit_write_video_encode_osd_attr_req(chip_dpram_controller, attr_regs_group);
    }
}

static int  osd_rectangle_attr_regs_group_dpram_tcb_delete_write_osd_rectangle_data_req(osd_rectangle_dpram_tcb_t *osd_dpram_req, isil_chip_t *chip)
{
    int ret = 0;

    if((osd_dpram_req!=NULL) && (chip!=NULL)){
        osd_attribute_regs_group_t  *attr_regs_group = to_osd_attribute_regs_group_with_osd_dpram_req(osd_dpram_req);
        dpram_control_t             *chip_dpram_controller;

        get_isil_dpram_controller(&chip_dpram_controller);
        chip_dpram_controller->op->delete_write_video_encode_osd_attr_req(chip_dpram_controller, attr_regs_group);
    }
    return ret;
}

static void osd_rectangle_attr_regs_group_dpram_tcb_submit_write_osd_rectangle_attr(osd_rectangle_dpram_tcb_t *osd_dpram_req, isil_chip_t *chip)
{
    if((osd_dpram_req!=NULL) && (chip!=NULL)){
        osd_attribute_regs_group_t  *attr_regs_group;
        ddr_burst_interface_t       *burst_interface;
        dpram_control_t             *chip_dpram_controller;
        chip_ddr_burst_engine_t     *chip_ddr_burst_interface;
        u32 *reg_value_buf, reg_value_buf_len;

        get_isil_dpram_controller(&chip_dpram_controller);
        attr_regs_group = to_osd_attribute_regs_group_with_osd_dpram_req(osd_dpram_req);
        chip_ddr_burst_interface = &chip->chip_ddr_burst_interface;
        burst_interface = &chip_ddr_burst_interface->burst_interface;
        reg_value_buf =  (u32 *)(attr_regs_group->attribute_regs_group);
        reg_value_buf_len = sizeof(osd_attribute_regs_t)*OSD_CHAN_RECTANGLE_NUMBER;
        burst_interface->op->pio_host_to_sram_write(burst_interface, osd_dpram_req->dpram_page, reg_value_buf, reg_value_buf_len, 0);
        burst_interface->op->start_block_transfer_from_sram_to_ddr(burst_interface, osd_dpram_req->dpram_page, attr_regs_group->osd_chan_attr_reg_base, attr_regs_group->osd_chan_attr_reg_page_id, reg_value_buf_len, DDR_CHIP_A);
    }
}

static int  osd_rectangle_attr_regs_group_dpram_tcb_delete_write_osd_rectangle_attr_req(osd_rectangle_dpram_tcb_t *osd_dpram_req, isil_chip_t *chip)
{
    int ret = 1;
    return ret;
}

static void osd_rectangle_attr_regs_group_dpram_tcb_response_write_osd_rectangle(osd_rectangle_dpram_tcb_t *osd_dpram_req, isil_chip_t *chip)
{
    if((osd_dpram_req!=NULL) && (chip!=NULL)){
        osd_attribute_regs_group_t  *attr_regs_group = to_osd_attribute_regs_group_with_osd_dpram_req(osd_dpram_req);
        osd_chan_engine_t           *osd_chan_engine = to_osd_chan_engine_with_osd_attribute_regs_group(attr_regs_group);
        dpram_control_t             *chip_dpram_controller;

        if(atomic_read(&osd_chan_engine->osd_working) > 0){
            atomic_dec(&osd_chan_engine->osd_working);
        }
        osd_chan_engine->osd_rectangle_need_transfer_data_flag &= ~(1<<ATTRIBUT_DATA_ID);
        osd_chan_engine->osd_rectangle_need_transfer_data_flag |= (1<<ATTRIBUT_DONE_ID);
        if((atomic_read(&osd_chan_engine->osd_working)==0) &&
                ((osd_chan_engine->osd_rectangle_need_transfer_data_flag&OSD_RECTANGLE_AND_ATTR_MASK)==0)){
            complete_all(&osd_chan_engine->osd_transfering_done);
        }
        get_isil_dpram_controller(&chip_dpram_controller);
        chip_dpram_controller->op->ack_write_video_encode_osd_attr_req(chip_dpram_controller, attr_regs_group);
    }
}

static struct osd_rectangle_dpram_tcb_operation osd_rectangle_attr_regs_group_dpram_tcb_op = {
    .init = osd_rectangle_attr_regs_group_dpram_tcb_init,
    .submit_write_osd_rectangle_data_req = osd_rectangle_attr_regs_group_dpram_tcb_submit_write_osd_rectangle_data_req,
    .delete_write_osd_rectangle_data_req = osd_rectangle_attr_regs_group_dpram_tcb_delete_write_osd_rectangle_data_req,
    .submit_write_osd_rectangle_attr = osd_rectangle_attr_regs_group_dpram_tcb_submit_write_osd_rectangle_attr,
    .delete_write_osd_rectangle_attr_req = osd_rectangle_attr_regs_group_dpram_tcb_delete_write_osd_rectangle_attr_req,
    .response_write_osd_rectangle = osd_rectangle_attr_regs_group_dpram_tcb_response_write_osd_rectangle,
};

static void osd_attribute_regs_group_init_osd_chan_rectangle_attr_regs_group(osd_attribute_regs_group_t *attr_regs_group)
{
    if(attr_regs_group != NULL){
        isil_h264_logic_encode_chan_t *encode_chan;
        osd_chan_engine_t       *osd_chan_engine;
        osd_attribute_regs_t    *attr_regs;
        osd_rectangle_dpram_tcb_t   *osd_dpram_req;
        int i, chan_id;

        attr_regs_group->rect_attr_regs_op = &osd_attribute_regs_op;
        for(i=0; i<OSD_CHAN_RECTANGLE_NUMBER; i++){
            attr_regs = &attr_regs_group->attribute_regs_group[i];
            attr_regs_group->rect_attr_regs_op->reset(attr_regs);
        }

        osd_chan_engine = to_osd_chan_engine_with_osd_attribute_regs_group(attr_regs_group);
        encode_chan = to_get_isil_h264_encode_chan_with_osd_engine(osd_chan_engine);
        chan_id = encode_chan->phy_slot_id;
        if(osd_chan_engine->masterOrSub == ISIL_MASTER_BITSTREAM){
            attr_regs_group->osd_chan_attr_reg_base = ((OSD_ATTR_REG_BASE_ADDR<<OSD_ATTR_REG_SHIFT)|(chan_id<<5))<<2;
        } else {
            attr_regs_group->osd_chan_attr_reg_base = ((OSD_ATTR_REG_BASE_ADDR<<OSD_ATTR_REG_SHIFT)|(0x1<<9)|(chan_id<<5))<<2;
        }
        attr_regs_group->osd_chan_attr_reg_page_id = attr_regs_group->osd_chan_attr_reg_base>>19;

        osd_dpram_req = &attr_regs_group->osd_dpram_req;
        osd_dpram_req->op = &osd_rectangle_attr_regs_group_dpram_tcb_op;
    }
}

static void osd_attribute_regs_group_chan_rectangle_attr_regs_set(osd_attribute_regs_group_t *attr_regs_group, int rect_id)
{
    if((attr_regs_group!=NULL) && (rect_id<OSD_CHAN_RECTANGLE_NUMBER)){
        osd_attribute_regs_t    *attr_regs = &attr_regs_group->attribute_regs_group[rect_id];
        attr_regs_group->rect_attr_regs_op->set(attr_regs);
    }
}

static void osd_attribute_regs_group_chan_rectangle_attr_regs_set_no_display(osd_attribute_regs_group_t *attr_regs_group, int rect_id)
{
    if((attr_regs_group!=NULL) && (rect_id<OSD_CHAN_RECTANGLE_NUMBER)){
        osd_attribute_regs_t    *attr_regs = &attr_regs_group->attribute_regs_group[rect_id];
        attr_regs_group->rect_attr_regs_op->set_no_display(attr_regs);
    }
}

static void osd_attribute_regs_group_chan_rectangle_attr_regs_reset(osd_attribute_regs_group_t *attr_regs_group, int rect_id)
{
    if((attr_regs_group!=NULL) && (rect_id<OSD_CHAN_RECTANGLE_NUMBER)){
        osd_attribute_regs_t    *attr_regs = &attr_regs_group->attribute_regs_group[rect_id];
        attr_regs_group->rect_attr_regs_op->reset(attr_regs);
    }
}

static void osd_attribute_regs_group_update_chan_rectangle_attr_regs(osd_attribute_regs_group_t *attr_regs_group, isil_chip_t *chip)
{
    if((attr_regs_group!=NULL) && (chip!=NULL)){
        osd_rectangle_dpram_tcb_t   *osd_dpram_req = &attr_regs_group->osd_dpram_req;
        osd_dpram_req->op->submit_write_osd_rectangle_data_req(osd_dpram_req, chip);
    }
}

static struct osd_attribute_regs_group_operation    osd_attribute_regs_group_op = {
    .init_osd_chan_rectangle_attr_regs_group = osd_attribute_regs_group_init_osd_chan_rectangle_attr_regs_group,
    .chan_rectangle_attr_regs_set = osd_attribute_regs_group_chan_rectangle_attr_regs_set,
    .chan_rectangle_attr_regs_set_no_display = osd_attribute_regs_group_chan_rectangle_attr_regs_set_no_display,
    .chan_rectangle_attr_regs_reset = osd_attribute_regs_group_chan_rectangle_attr_regs_reset,
    .update_chan_rectangle_attr_regs = osd_attribute_regs_group_update_chan_rectangle_attr_regs,
};

static void osd_mb_tcb_reset(osd_mb_char_entry_t *mb)
{
    mb->rectangle = NULL;
    mb->osd_mb_node.op = &tcb_node_op;
    mb->osd_mb_node.op->init(&mb->osd_mb_node);
    mb->osd_mb_node.op->set_priv(&mb->osd_mb_node, mb);

    mb->ddr_page = INVALID_OSD_VALUE_ID;
    mb->ddr_page_offset = INVALID_OSD_VALUE_ID;

    mb->payload = (u32*)mb->mb_char_buf;
    mb->payload_len = 0;
    memset(mb->mb_char_buf, 0, (OSD_MB_BUF_CHAR_BITMAP_LEN<<2));
    mb->ref_mb_x = 0;
    mb->ref_mb_y = 0;
}

static void osd_mb_tcb_release(osd_mb_char_entry_t *mb)
{
    if(mb != NULL){
        osd_mb_pool_t	*pool = mb->pool;
        mb->op->reset(mb);
        pool->op->put(pool, mb);
    }
}

static void osd_mb_tcb_update_mb_virtual_data(osd_mb_char_entry_t *mb, int ref_mb_x, int ref_mb_y)
{
    osd_rectangle_entry_t   *rectangle;
    struct osd_attribute_regs1	*ptr_regs1;
    struct osd_attribute_regs3	*ptr_regs3;

    rectangle = mb->rectangle;
    if(rectangle != NULL){
        ptr_regs1 = &rectangle->attr_regs->reg1_value.reg1;
        ptr_regs3 = &rectangle->attr_regs->reg3_value.reg3;
        mb->osd_data_addr_base = get_osd_rectangle_mb_data_base(ptr_regs1->mode, ptr_regs3->data_base_map_mode, ref_mb_x, ref_mb_y, rectangle->osd_data_addr_base);
        get_ddr_page_id_and_page_offset_by_md_data_base(mb->osd_data_addr_base, &mb->ddr_page, &mb->ddr_page_offset);
    } else {
        mb->ddr_page = INVALID_OSD_VALUE_ID;
        mb->ddr_page_offset = INVALID_OSD_VALUE_ID;
    }
}

static void osd_mb_tcb_update_fpga_osd_data(osd_mb_char_entry_t *mb)
{
    if((mb->ddr_page!=INVALID_OSD_VALUE_ID) && (mb->ddr_page_offset!=INVALID_OSD_VALUE_ID)){
        isil_chip_t  *chip;
        u32 ddr_page_offset, ddr_page, old_ddr_page, i;

        chip = mb->pool->chip;
        ddr_page_offset = mb->ddr_page_offset;
        ddr_page_offset += DDRBASE;
        ddr_page = mb->ddr_page;
        old_ddr_page = chip->io_op->chip_read32(chip, DDRPAGE);
        chip->io_op->chip_write32(chip, DDRPAGE, ddr_page);
        for(i=0; i<mb->payload_len; i++){
            chip->io_op->chip_write32(chip, ddr_page_offset, mb->payload[i]);
            ddr_page_offset += 4;
            if(ddr_page_offset >= (DDRBASE<<1)){
                ddr_page_offset = DDRBASE;
                ddr_page++;
                chip->io_op->chip_write32(chip, DDRPAGE, ddr_page);
            }
        }
        chip->io_op->chip_write32(chip, DDRPAGE, old_ddr_page);
    }
}

static struct osd_mb_char_operation osd_mb_char_tcb_op = 
{
    .reset = osd_mb_tcb_reset,
    .release = osd_mb_tcb_release,
    .update_mb_virtual_data = osd_mb_tcb_update_mb_virtual_data,
    .update_fpga_osd_data = osd_mb_tcb_update_fpga_osd_data,
};

static int osd_encode_mb_pool_create(osd_mb_pool_t *chan_osd_mb_buf_pool, int chip_id)
{
    if(chan_osd_mb_buf_pool != NULL){
        tcb_node_pool_t	    *node_pool;
        osd_mb_char_entry_t *ptr_osd_mb_char_tcb;
        unsigned long buf_size, i;
#ifdef  USED_STATIC_OSD_CHAR_BUF
        chan_osd_mb_buf_pool->mb_char_entry_cache = osd_md_cache[0];
#else
        buf_size = sizeof(osd_mb_char_entry_t)*OSD_MB_ENTRY_NUMBER;
        chan_osd_mb_buf_pool->mb_char_cache_order = get_order(buf_size);
        chan_osd_mb_buf_pool->mb_char_entry_cache = (osd_mb_char_entry_t*)__get_free_pages(GFP_KERNEL, chan_osd_mb_buf_pool->mb_char_cache_order);
        if(chan_osd_mb_buf_pool->mb_char_entry_cache == NULL){
            printk("can't alloc %d pages for video buf pool\n", chan_osd_mb_buf_pool->mb_char_cache_order);
            chan_osd_mb_buf_pool->mb_char_cache_order = 0;
            return -ENOMEM;
        }
#endif
        node_pool = &chan_osd_mb_buf_pool->osd_mb_char_pool_tcb;
        node_pool->op = &tcb_node_pool_op;
        node_pool->op->init(node_pool, OSD_MB_ENTRY_NUMBER);
        for(i=0; i<OSD_MB_ENTRY_NUMBER; i++){
            ptr_osd_mb_char_tcb = &chan_osd_mb_buf_pool->mb_char_entry_cache[i];
            ptr_osd_mb_char_tcb->pool = chan_osd_mb_buf_pool;
            ptr_osd_mb_char_tcb->op = &osd_mb_char_tcb_op;
            ptr_osd_mb_char_tcb->op->release(ptr_osd_mb_char_tcb);
        }
        return 0;
    }
    return -ENOMEM;
}

static void osd_encode_mb_pool_release(osd_mb_pool_t *chan_osd_mb_buf_pool)
{
    if(chan_osd_mb_buf_pool != NULL){
        tcb_node_pool_t	*node_pool;
        node_pool = &chan_osd_mb_buf_pool->osd_mb_char_pool_tcb;
        if(node_pool->op != NULL){
            node_pool->op->release(node_pool);
        }
#ifdef  USED_STATIC_OSD_CHAR_BUF
        chan_osd_mb_buf_pool->mb_char_entry_cache = NULL;
#else
        if(chan_osd_mb_buf_pool->mb_char_entry_cache != NULL){
            free_pages((unsigned long)chan_osd_mb_buf_pool->mb_char_entry_cache, chan_osd_mb_buf_pool->mb_char_cache_order);
            chan_osd_mb_buf_pool->mb_char_entry_cache = NULL;
        }
#endif
        chan_osd_mb_buf_pool->mb_char_cache_order = 0;
    }
}

static void osd_encode_mb_pool_get(osd_mb_pool_t *pool, osd_mb_char_entry_t **mb, osd_rectangle_entry_t *rectangle)
{
    if((pool!=NULL) && (mb!=NULL)){
        tcb_node_pool_t *node_pool = &pool->osd_mb_char_pool_tcb;
        *mb = NULL;
        if(node_pool->op != NULL){
            tcb_node_t	*temp_node;
            node_pool->op->get(node_pool, &temp_node);
            if(temp_node != NULL){
                *mb = to_get_osd_mb_char_entry_with_node(temp_node);
                (*mb)->rectangle = rectangle;
            }
        }
    }
}

static void osd_encode_mb_pool_try_get(osd_mb_pool_t *pool, osd_mb_char_entry_t **mb, osd_rectangle_entry_t *rectangle)
{
    if((pool!=NULL) && (mb!=NULL)){
        tcb_node_pool_t *node_pool = &pool->osd_mb_char_pool_tcb;
        *mb = NULL;
        if(node_pool->op != NULL){
            tcb_node_t	*temp_node;
            node_pool->op->try_get(node_pool, &temp_node);
            if(temp_node != NULL){
                *mb = to_get_osd_mb_char_entry_with_node(temp_node);
                (*mb)->rectangle = rectangle;
            }
        }
    }
}

static void osd_encode_mb_pool_put(osd_mb_pool_t *pool, osd_mb_char_entry_t *mb)
{
    if((pool!=NULL) && (mb!=NULL)){
        tcb_node_pool_t *node_pool = &pool->osd_mb_char_pool_tcb;
        if(node_pool->op != NULL){
            node_pool->op->put(node_pool, &mb->osd_mb_node);
        }
    }
}

static int  osd_encode_mb_pool_entry_number(osd_mb_pool_t *pool)
{
    int	entry_number = 0;
    if(pool != NULL){
        tcb_node_pool_t *node_pool = &pool->osd_mb_char_pool_tcb;
        if(node_pool->op != NULL){
            entry_number = node_pool->op->get_curr_pool_entry_number(node_pool);
        }
    }
    return entry_number;
}

static struct osd_mb_pool_operation osd_encode_mb_pool_op = 
{
    .create = osd_encode_mb_pool_create,
    .release = osd_encode_mb_pool_release,
    .get = osd_encode_mb_pool_get,
    .try_get = osd_encode_mb_pool_try_get,
    .put = osd_encode_mb_pool_put,
    .get_osd_mb_tcb_pool_entry_number = osd_encode_mb_pool_entry_number,
};

void    remove_osd_encode_mb_pool(osd_mb_pool_t *mb_pool)
{
    if(mb_pool->op != NULL){
        mb_pool->op->release(mb_pool);
    }
}

int init_osd_encode_mb_pool(osd_chan_engine_t *osd_engine, int chip_id)
{
    int ret = -1;
    if(osd_engine != NULL){
        osd_mb_pool_t   *mb_pool;
        mb_pool = &osd_engine->osd_chan_pool;
        mb_pool->chip = osd_engine->encode_chan->chip;
        mb_pool->op = &osd_encode_mb_pool_op;
        ret = mb_pool->op->create(mb_pool, chip_id);
    }
    return ret;
}

static int  osd_data_burst_done_isr(int irq, void *context)
{
    osd_rectangle_dpram_tcb_t   *osd_dpram_req = (osd_rectangle_dpram_tcb_t*)context;
    int ret = 0;
    if(osd_dpram_req != NULL){
        osd_rectangle_entry_t   *rectangle = to_osd_rectangle_entry_with_osd_rectangle_dpram_tcb(osd_dpram_req);
        osd_chan_engine_t       *osd_chan_engine = rectangle->osd_chan_engine;
        isil_chip_t             *chip = osd_chan_engine->encode_chan->chip;
        ddr_burst_interface_t   *burst_interface;
        osd_mb_char_entry_t     *mb;
        chip_ddr_burst_engine_t *chip_ddr_burst_interface;
        unsigned long   flags;
        u32 dpram_offset_addr;
        u32 ddr_start_base_addr, ddr_page_id;


        spin_lock_irqsave(&osd_chan_engine->osd_lock, flags);
        if(atomic_read(&osd_chan_engine->osd_encode_start_close)){
            do{
                rectangle->op->try_get_mb_from_rectangle(rectangle, &mb);
                if(mb != NULL){
                    mb->op->release(mb);
                } else {
                    break;
                }
            }while(1);
            chip_free_irq(chip, IRQ_BURST_TYPE_INTR, osd_dpram_req);
            osd_dpram_req->op->response_write_osd_rectangle(osd_dpram_req, chip);
        } else {
            chip_ddr_burst_interface = &chip->chip_ddr_burst_interface;
            burst_interface = &chip_ddr_burst_interface->burst_interface;
            dpram_offset_addr = ddr_start_base_addr = ddr_page_id = 0;
            if(rectangle->op->get_mb_number_at_rectangle(rectangle)){
                rectangle->op->try_get_mb_from_rectangle(rectangle, &mb);
                while(mb != NULL){
                    mb->op->update_mb_virtual_data(mb, mb->ref_mb_x, mb->ref_mb_y);
                    if(dpram_offset_addr == 0){
                        ddr_start_base_addr = mb->osd_data_addr_base;
                        ddr_page_id = mb->ddr_page;
                    }
                    burst_interface->op->pio_host_to_sram_write(burst_interface, osd_dpram_req->dpram_page, mb->payload, (mb->payload_len<<2), dpram_offset_addr);
                    dpram_offset_addr += 64;
                    mb->op->release(mb);
                    if((dpram_offset_addr >= 2048) || (rectangle->op->get_mb_number_at_rectangle(rectangle)==0)){
                        break;
                    }
                    rectangle->op->try_get_mb_from_rectangle(rectangle, &mb);
                }
                if(dpram_offset_addr){
                    burst_interface->op->start_nonblock_transfer_from_sram_to_ddr(burst_interface, osd_dpram_req->dpram_page, ddr_start_base_addr, ddr_page_id, dpram_offset_addr, DDR_CHIP_A);
                } else {
                    chip_free_irq(chip, IRQ_BURST_TYPE_INTR, osd_dpram_req);
                    osd_dpram_req->op->response_write_osd_rectangle(osd_dpram_req, chip);
                }
            } else if(rectangle->op->get_mb24_number_at_rectangle(rectangle)){
                rectangle->op->try_get_mb_from_rectangle(rectangle, &mb);
                while(mb != NULL){
                    mb->op->update_mb_virtual_data(mb, mb->ref_mb_x, mb->ref_mb_y);
                    if(dpram_offset_addr == 0){
                        ddr_start_base_addr = mb->osd_data_addr_base;
                        ddr_page_id = mb->ddr_page;
                    }
                    burst_interface->op->pio_host_to_sram_write(burst_interface, osd_dpram_req->dpram_page, mb->payload, (mb->payload_len<<2), dpram_offset_addr);
                    dpram_offset_addr += 64;
                    mb->op->release(mb);
                    if((dpram_offset_addr >= 2048) || (rectangle->op->get_mb24_number_at_rectangle(rectangle)==0)){
                        break;
                    }
                    rectangle->op->try_get_mb_from_rectangle(rectangle, &mb);
                }
                if(dpram_offset_addr){
                    burst_interface->op->start_nonblock_transfer_from_sram_to_ddr(burst_interface, osd_dpram_req->dpram_page, ddr_start_base_addr, ddr_page_id, dpram_offset_addr, DDR_CHIP_A);
                } else {
                    chip_free_irq(chip, IRQ_BURST_TYPE_INTR, osd_dpram_req);
                    osd_dpram_req->op->response_write_osd_rectangle(osd_dpram_req, chip);
                }
            } else {
                chip_free_irq(chip, IRQ_BURST_TYPE_INTR, osd_dpram_req);
                osd_dpram_req->op->response_write_osd_rectangle(osd_dpram_req, chip);
            }
        }
        spin_unlock_irqrestore(&osd_chan_engine->osd_lock, flags);
        ret = 1;
    }
    return ret;
}

static int osd_encode_chan_start_send_bitstream(dpram_request_t *dpram_req, void *context)
{
    int ret = 1;
    if((dpram_req!=NULL) && (context!=NULL)){
        osd_rectangle_dpram_tcb_t   *osd_dpram_req = (osd_rectangle_dpram_tcb_t*)context;
        osd_rectangle_entry_t   *rectangle = to_osd_rectangle_entry_with_osd_rectangle_dpram_tcb(osd_dpram_req);
        osd_chan_engine_t       *osd_chan_engine = rectangle->osd_chan_engine;
        isil_chip_t             *chip;
        ddr_burst_interface_t   *burst_interface;
        osd_mb_char_entry_t     *mb;
        dpram_control_t         *chip_dpram_controller;
        chip_ddr_burst_engine_t *chip_ddr_burst_interface;
        int free_rectangle_mb_resource = 0;
        u32 dpram_offset_addr;
        u32 ddr_start_base_addr, ddr_page_id;

        get_isil_dpram_controller(&chip_dpram_controller);
        chip = dpram_req->chip;
        chip_ddr_burst_interface = &chip->chip_ddr_burst_interface;
        burst_interface = &chip_ddr_burst_interface->burst_interface;
        if(atomic_read(&osd_chan_engine->osd_encode_start_close)){
            free_rectangle_mb_resource = 2;
        } else {
            if(rectangle->osd_enable && rectangle->need_update_data){
                rectangle->need_update_data = 0;
#ifdef  SUBMIT_OSD_BY_INTERRUPT
                rectangle->op->try_get_mb_from_rectangle(rectangle, &mb);
                if(mb != NULL){
                    if(chip_dpram_controller->op->is_can_submit_move_data_from_host_to_dpram_service_req(chip_dpram_controller, &osd_dpram_req->dpram_page, chip)){
                        dpram_offset_addr = ddr_start_base_addr = ddr_page_id = 0;
                        while(mb != NULL){
                            mb->op->update_mb_virtual_data(mb, mb->ref_mb_x, mb->ref_mb_y);
                            if(dpram_offset_addr == 0){
                                ddr_start_base_addr = mb->osd_data_addr_base;
                                ddr_page_id = mb->ddr_page;
                            }
                            burst_interface->op->pio_host_to_sram_write(burst_interface, osd_dpram_req->dpram_page, mb->payload, (mb->payload_len<<2), dpram_offset_addr);
                            dpram_offset_addr += 64;
                            mb->op->release(mb);
                            if((dpram_offset_addr >= 2048) || (rectangle->op->get_mb_number_at_rectangle(rectangle)==0)){
                                break;
                            }
                            rectangle->op->try_get_mb_from_rectangle(rectangle, &mb);
                        }
                        chip_request_irq(chip, IRQ_BURST_TYPE_INTR, osd_data_burst_done_isr, "BURST", (void*)osd_dpram_req);
                        burst_interface->op->start_nonblock_transfer_from_sram_to_ddr(burst_interface, osd_dpram_req->dpram_page, ddr_start_base_addr, ddr_page_id, dpram_offset_addr, DDR_CHIP_A);
                    } else {
                        free_rectangle_mb_resource = 2;
                    }
                } else {
                    free_rectangle_mb_resource = 1;
                }
#else
                if(chip_dpram_controller->op->is_can_submit_move_data_from_host_to_dpram_service_req(chip_dpram_controller, &osd_dpram_req->dpram_page, chip)){
                    while(rectangle->op->get_mb_number_at_rectangle(rectangle)){
                        dpram_offset_addr = ddr_start_base_addr = ddr_page_id = 0;
                        rectangle->op->try_get_mb_from_rectangle(rectangle, &mb);
                        while(mb != NULL){
                            mb->op->update_mb_virtual_data(mb, mb->ref_mb_x, mb->ref_mb_y);
                            if(dpram_offset_addr == 0){
                                ddr_start_base_addr = mb->osd_data_addr_base;
                                ddr_page_id = mb->ddr_page;
                            }
                            burst_interface->op->pio_host_to_sram_write(burst_interface, osd_dpram_req->dpram_page, mb->payload, (mb->payload_len<<2), dpram_offset_addr);
                            dpram_offset_addr += 64;
                            mb->op->release(mb);
                            if((dpram_offset_addr >= 2048) || (rectangle->op->get_mb_number_at_rectangle(rectangle)==0)){
                                break;
                            }
                            rectangle->op->try_get_mb_from_rectangle(rectangle, &mb);
                        }
                        if(dpram_offset_addr){
                            burst_interface->op->start_block_transfer_from_sram_to_ddr(burst_interface, osd_dpram_req->dpram_page, ddr_start_base_addr, ddr_page_id, dpram_offset_addr, DDR_CHIP_A);
                        }
                    }
                    while(rectangle->op->get_mb24_number_at_rectangle(rectangle)){
                        dpram_offset_addr = ddr_start_base_addr = ddr_page_id = 0;
                        rectangle->op->try_get_mb_from_rectangle(rectangle, &mb);
                        while(mb != NULL){
                            mb->op->update_mb_virtual_data(mb, mb->ref_mb_x, mb->ref_mb_y);
                            if(dpram_offset_addr == 0){
                                ddr_start_base_addr = mb->osd_data_addr_base;
                                ddr_page_id = mb->ddr_page;
                            }
                            burst_interface->op->pio_host_to_sram_write(burst_interface, osd_dpram_req->dpram_page, mb->payload, (mb->payload_len<<2), dpram_offset_addr);
                            dpram_offset_addr += 64;
                            mb->op->release(mb);
                            if((dpram_offset_addr >= 2048) || (rectangle->op->get_mb24_number_at_rectangle(rectangle)==0)){
                                break;
                            }
                            rectangle->op->try_get_mb_from_rectangle(rectangle, &mb);
                        }
                        if(dpram_offset_addr){
                            burst_interface->op->start_block_transfer_from_sram_to_ddr(burst_interface, osd_dpram_req->dpram_page, ddr_start_base_addr, ddr_page_id, dpram_offset_addr, DDR_CHIP_A);
                        }
                    }
                    free_rectangle_mb_resource = 1;
                } else {
                    free_rectangle_mb_resource = 2;
                }
#endif
            } else {
                free_rectangle_mb_resource = 2;
            }
        }

        if(free_rectangle_mb_resource){
            rectangle->need_update_data = 0;
            do{
                rectangle->op->try_get_mb_from_rectangle(rectangle, &mb);
                if(mb != NULL){
                    mb->op->release(mb);
                } else {
                    break;
                }
            }while(1);
            if(free_rectangle_mb_resource > 1){
                osd_attribute_regs_group_t  *attribute_regs;
                attribute_regs = &osd_chan_engine->attribute_regs;
                attribute_regs->chan_rect_attr_regs_group_op->chan_rectangle_attr_regs_reset(attribute_regs, rectangle->rectangle_id);
                atomic_set(&osd_chan_engine->sync_osd_param, 1);
                ISIL_DBG(ISIL_DBG_DEBUG, "retrigger\n");
            }
            osd_dpram_req->op->response_write_osd_rectangle(osd_dpram_req, rectangle->chip);
        }
    }
    return ret;
}

static int  osd_encode_chan_delete_start_send_bitstream(dpram_request_t *dpram_req, void *context)
{
    int ret = 1;
    if((dpram_req!=NULL) && (context!=NULL)){
        osd_rectangle_dpram_tcb_t   *osd_dpram_req = (osd_rectangle_dpram_tcb_t*)context;
        osd_rectangle_entry_t   *rectangle = to_osd_rectangle_entry_with_osd_rectangle_dpram_tcb(osd_dpram_req);
        osd_chan_engine_t       *osd_chan_engine = rectangle->osd_chan_engine;
        osd_mb_char_entry_t     *mb;

        rectangle->need_update_data = 0;
        do{
            rectangle->op->try_get_mb_from_rectangle(rectangle, &mb);
            if(mb != NULL){
                mb->op->release(mb);
            } else {
                break;
            }
        }while(1);

        if(atomic_read(&osd_chan_engine->osd_working) > 0){
            atomic_dec(&osd_chan_engine->osd_working);
        }
        osd_chan_engine->osd_rectangle_need_transfer_data_flag &= ~(1<<rectangle->rectangle_id);
        if((atomic_read(&osd_chan_engine->osd_working)==0) &&
                ((osd_chan_engine->osd_rectangle_need_transfer_data_flag&OSD_RECTANGLE_AND_ATTR_MASK)==0)){
            complete_all(&osd_chan_engine->osd_transfering_done);
        }
    }
    return ret;
}

static void osd_rectangle_dpram_tcb_init(osd_rectangle_dpram_tcb_t *osd_dpram_req, isil_chip_t *chip)
{
    if(osd_dpram_req != NULL){
        dpram_request_t *dpram_req;
        dpram_req = &osd_dpram_req->osd_data_req;
        dpram_req->chip = chip;
        dpram_req->type = DPRAM_NONBLOCK_TRANSFER_REQUEST;
        dpram_req->context = (void*)osd_dpram_req;
        dpram_req->req_callback = osd_encode_chan_start_send_bitstream;
        dpram_req->delete_req_notify = osd_encode_chan_delete_start_send_bitstream;
    }
}

static void osd_rectangle_dpram_tcb_submit_write_osd_rectangle_data_req(osd_rectangle_dpram_tcb_t *osd_dpram_req, isil_chip_t *chip)
{
    if((osd_dpram_req!=NULL) && (chip!=NULL)){
        osd_rectangle_entry_t   *rectangle = to_osd_rectangle_entry_with_osd_rectangle_dpram_tcb(osd_dpram_req);
        osd_chan_engine_t       *osd_chan_engine = rectangle->osd_chan_engine;
        dpram_control_t         *chip_dpram_controller;

        atomic_inc(&osd_chan_engine->osd_working);
        init_completion(&osd_chan_engine->osd_transfering_done);
        get_isil_dpram_controller(&chip_dpram_controller);
        osd_dpram_req->op->init(osd_dpram_req, chip);
        chip_dpram_controller->op->submit_write_video_encode_osd_data_req(chip_dpram_controller, rectangle);
    }
}

static int  osd_rectangle_dpram_tcb_delete_write_osd_rectangle_data_req(osd_rectangle_dpram_tcb_t *osd_dpram_req, isil_chip_t *chip)
{
    int ret = 0;
    if((osd_dpram_req!=NULL) && (chip!=NULL)){
        osd_rectangle_entry_t   *rectangle = to_osd_rectangle_entry_with_osd_rectangle_dpram_tcb(osd_dpram_req);
        dpram_control_t         *chip_dpram_controller;

        get_isil_dpram_controller(&chip_dpram_controller);
        chip_dpram_controller->op->delete_wirte_video_encode_osd_data_req(chip_dpram_controller, rectangle);
    }
    return ret;
}

static void osd_rectangle_dpram_tcb_submit_write_osd_rectangle_attr(osd_rectangle_dpram_tcb_t *osd_dpram_req, isil_chip_t *chip)
{

}

static int  osd_rectangle_dpram_tcb_delete_write_osd_rectangle_attr_req(osd_rectangle_dpram_tcb_t *osd_dpram_req, isil_chip_t *chip)
{
    int ret = 0;

    return ret;
}

static void osd_rectangle_dpram_tcb_response_write_osd_rectangle(osd_rectangle_dpram_tcb_t *osd_dpram_req, isil_chip_t *chip)
{
    if((osd_dpram_req!=NULL) && (chip!=NULL)){
        osd_rectangle_entry_t   *rectangle = to_osd_rectangle_entry_with_osd_rectangle_dpram_tcb(osd_dpram_req);
        osd_chan_engine_t       *osd_chan_engine = rectangle->osd_chan_engine;
        dpram_control_t         *chip_dpram_controller;

        if(atomic_read(&osd_chan_engine->osd_working) > 0){
            atomic_dec(&osd_chan_engine->osd_working);
        }
        osd_chan_engine->osd_rectangle_need_transfer_data_flag &= ~(1<<rectangle->rectangle_id);
        if((atomic_read(&osd_chan_engine->osd_working)==0) &&
                ((osd_chan_engine->osd_rectangle_need_transfer_data_flag&OSD_RECTANGLE_AND_ATTR_MASK)==0)){
            complete_all(&osd_chan_engine->osd_transfering_done);
        }
        get_isil_dpram_controller(&chip_dpram_controller);
        chip_dpram_controller->op->ack_write_video_encode_osd_data_req(chip_dpram_controller, rectangle);
    }
}

static struct osd_rectangle_dpram_tcb_operation osd_rectangle_dpram_tcb_op = {
    .init = osd_rectangle_dpram_tcb_init,
    .submit_write_osd_rectangle_data_req = osd_rectangle_dpram_tcb_submit_write_osd_rectangle_data_req,
    .delete_write_osd_rectangle_data_req = osd_rectangle_dpram_tcb_delete_write_osd_rectangle_data_req,
    .submit_write_osd_rectangle_attr = osd_rectangle_dpram_tcb_submit_write_osd_rectangle_attr,
    .delete_write_osd_rectangle_attr_req = osd_rectangle_dpram_tcb_delete_write_osd_rectangle_attr_req,
    .response_write_osd_rectangle = osd_rectangle_dpram_tcb_response_write_osd_rectangle,
};

static void osd_rectangle_reset(osd_rectangle_entry_t *rectangle)
{
    rectangle->osd_enable = 0;
    rectangle->need_update_attr = 0;
    rectangle->need_update_data = 0;
    rectangle->rectangle_mode = EMPTY_RECTANGLE_ENCODE_OSD;
    rectangle->rectangle_left_x = rectangle->rectangle_left_y = 0;
    rectangle->rectangle_right_x = rectangle->rectangle_right_y = 0;
    rectangle->private_attri.empty_rectangle_attribute = NULL;
}

static void osd_rectangle_close_rectangle(osd_rectangle_entry_t *rectangle)
{
    osd_mb_char_entry_t *mb;
    osd_chan_engine_t   *osd_chan_engine;
    osd_attribute_regs_group_t *attribute_regs;

    osd_chan_engine = rectangle->osd_chan_engine;
    osd_chan_engine->op->put_rectangle(osd_chan_engine, rectangle);
    attribute_regs = &osd_chan_engine->attribute_regs;
    attribute_regs->chan_rect_attr_regs_group_op->chan_rectangle_attr_regs_reset(attribute_regs, rectangle->rectangle_id);
    do{
        rectangle->op->try_get_mb_from_rectangle(rectangle, &mb);
        if(mb != NULL){
            mb->op->release(mb);
        } else {
            break;
        }
    }while(1);
    rectangle->op->reset(rectangle);
}

static int  osd_rectangle_get_mb_number_at_rectangle(osd_rectangle_entry_t *rectangle)
{
    tcb_node_queue_t    *mb_char_queue;
    mb_char_queue = &rectangle->mb_char_queue_node;
    return  mb_char_queue->op->get_queue_curr_entry_number(mb_char_queue);
}

static int  osd_rectangle_get_mb24_number_at_rectangle(osd_rectangle_entry_t *rectangle)
{
    tcb_node_queue_t    *mb_char_queue;
    mb_char_queue = &rectangle->mb24_char_queue_node;
    return mb_char_queue->op->get_queue_curr_entry_number(mb_char_queue);
}

static void osd_rectangle_put_mb_into_rectangle(osd_rectangle_entry_t *rectangle, osd_mb_char_entry_t *mb)
{
    if(mb != NULL){
        tcb_node_queue_t    *mb_char_queue;
        mb_char_queue = &rectangle->mb_char_queue_node;
        mb_char_queue->op->put(mb_char_queue, &mb->osd_mb_node);
    }
}

static void osd_rectangle_put_mb24_into_rectangle(osd_rectangle_entry_t *rectangle, osd_mb_char_entry_t *mb)
{
    if(mb != NULL){
        tcb_node_queue_t    *mb_char_queue;
        mb_char_queue = &rectangle->mb24_char_queue_node;
        mb_char_queue->op->put(mb_char_queue, &mb->osd_mb_node);
    }
}

static void osd_rectangle_try_get_mb_from_rectangle(osd_rectangle_entry_t *rectangle, osd_mb_char_entry_t **mb)
{
    tcb_node_queue_t    *mb_char_queue;
    tcb_node_t  *temp_node;

    mb_char_queue = &rectangle->mb_char_queue_node;
    mb_char_queue->op->try_get(mb_char_queue, &temp_node);
    if(temp_node != NULL){
        *mb = to_get_osd_mb_char_entry_with_node(temp_node);
    } else {
        *mb = NULL;
    }
    if(*mb == NULL){
        mb_char_queue = &rectangle->mb24_char_queue_node;
        mb_char_queue->op->try_get(mb_char_queue, &temp_node);
        if(temp_node != NULL){
            *mb = to_get_osd_mb_char_entry_with_node(temp_node);
        } else {
            *mb = NULL;
        }
    }
}

static int  osd_rectangle_update_rectangle_fpga_buf(osd_rectangle_entry_t *rectangle)
{
    int	ret = 0;

    if(rectangle->osd_enable){
        if(rectangle->need_update_data){
            osd_mb_char_entry_t	*mb;
            do{
                rectangle->op->try_get_mb_from_rectangle(rectangle, &mb);
                if(mb != NULL){
                    mb->op->update_mb_virtual_data(mb, mb->ref_mb_x, mb->ref_mb_y);
                    mb->op->update_fpga_osd_data(mb);
                    mb->op->release(mb);
                } else {
                    break;
                }
            }while(1);
            rectangle->need_update_data = 0;
            ret |= UPDATE_OSD_DATA_SECTION;
        }
    }
    return ret;
}

static int  osd_rectangle_update_rectangle_attr_regs(osd_rectangle_entry_t *rectangle)
{
    int	ret = 0;

    if(rectangle->osd_enable){
        if(rectangle->need_update_attr){
            osd_chan_engine_t   *osd_chan_engine;
            osd_attribute_regs_group_t *attribute_regs;

            osd_chan_engine = rectangle->osd_chan_engine;
            attribute_regs = &osd_chan_engine->attribute_regs;
            attribute_regs->chan_rect_attr_regs_group_op->chan_rectangle_attr_regs_set(attribute_regs, rectangle->rectangle_id);
            rectangle->need_update_attr = 0;
            ret |= UPDATE_OSD_ATTR_SECTION;
        }
    }
    return ret;
}

static void osd_rectangle_decide_rectangle_data_addr_map(osd_rectangle_entry_t *rectangle, int video_source_size)
{
    osd_chan_engine_t           *osd_chan_engine = rectangle->osd_chan_engine;
    osd_attribute_regs_group_t  *attribute_regs;
    encode_osd_string_attr_t    *string_attr;
    encode_osd_mask_attr_t      *mask_attr;
    struct osd_attribute_regs1  *ptr_regs1;
    struct osd_attribute_regs2  *ptr_regs2;
    struct osd_attribute_regs3  *ptr_regs3;
    int osd_data_0_8_base_addr, osd_data_addr_offset_0_2;

    attribute_regs = &osd_chan_engine->attribute_regs;
    ptr_regs1 = &rectangle->attr_regs->reg1_value.reg1;
    ptr_regs1->enable = 0;
    ptr_regs1->rectangle_mb_x_width = rectangle->rectangle_right_x - rectangle->rectangle_left_x;
    ptr_regs1->rectangle_mb_y_height = rectangle->rectangle_right_y - rectangle->rectangle_left_y;
    ptr_regs1->rectangle_mb_x = rectangle->rectangle_left_x;
    ptr_regs1->rectangle_mb_y = rectangle->rectangle_left_y;
    ptr_regs2 = &rectangle->attr_regs->reg2_value.reg2;
    ptr_regs2->mode1_front_y = 0;
    ptr_regs2->mode1_front_u = 0;
    ptr_regs2->mode1_front_v = 0;
    ptr_regs3 = &rectangle->attr_regs->reg3_value.reg3;
    ptr_regs3->mode1_background_y = 0;
    ptr_regs3->mode1_background_u = 0;
    ptr_regs3->mode1_background_v = 0;
    ptr_regs3->reserved = 0;
    switch(rectangle->rectangle_mode){
        case TIMER_RECTANGLE_ENCODE_OSD:
            if(rectangle->private_attri.time_rectangle_attribute==NULL){
                attribute_regs->chan_rect_attr_regs_group_op->chan_rectangle_attr_regs_reset(attribute_regs, rectangle->rectangle_id);
                return;
            }
            if(rectangle->private_attri.time_rectangle_attribute->timer_display_mode >= DONT_DISPLAY_TIMER){
                attribute_regs->chan_rect_attr_regs_group_op->chan_rectangle_attr_regs_reset(attribute_regs, rectangle->rectangle_id);
                return;
            }
            ptr_regs1->mode = ENCODE_OSD_MODE0;
            ptr_regs3->data_base_map_mode = Y8MB_X128MB_OSDMODE013;
            break;
        case CHAR_RECTANGLE_ENCODE_OSD:
            string_attr = rectangle->private_attri.string_rectangle_attribute;
            if(string_attr == NULL){
                attribute_regs->chan_rect_attr_regs_group_op->chan_rectangle_attr_regs_reset(attribute_regs, rectangle->rectangle_id);
                return;
            }
            ptr_regs1->mode = ENCODE_OSD_MODE0;
            ptr_regs3->data_base_map_mode = string_attr->data_addr_map_mode;
            break;
        case MASK_RECTANGLE_ENCODE_OSD:
            mask_attr = rectangle->private_attri.mask_rectangle_attribute;
            if(mask_attr == NULL){
                attribute_regs->chan_rect_attr_regs_group_op->chan_rectangle_attr_regs_reset(attribute_regs, rectangle->rectangle_id);
                return;
            }
            ptr_regs1->mode = ENCODE_OSD_MODE3;
            ptr_regs3->data_base_map_mode = mask_attr->data_addr_map_mode;
            ptr_regs2->mode1_front_u = ptr_regs3->mode1_background_u = mask_attr->u_value;
            ptr_regs2->mode1_front_v = ptr_regs3->mode1_background_v = mask_attr->v_value;
            ptr_regs2->mode1_front_y = ptr_regs3->mode1_background_y = mask_attr->y_value;
            break;
        default:
            attribute_regs->chan_rect_attr_regs_group_op->chan_rectangle_attr_regs_reset(attribute_regs, rectangle->rectangle_id);
            return;
    }
    rectangle->osd_data_addr_base = get_osd_rectangel_data_base(rectangle->rectangle_id, video_source_size, osd_chan_engine->masterOrSub, osd_chan_engine->chan_data_base);
    get_osd_rectangel_data_base_and_offset(rectangle->osd_data_addr_base, ptr_regs1->mode, ptr_regs3->data_base_map_mode, &osd_data_0_8_base_addr, &osd_data_addr_offset_0_2);
    ptr_regs1->data_base_addr8 = ((osd_data_0_8_base_addr>>8)&0x1);
    ptr_regs2->data_base_addr_7_0 = (osd_data_0_8_base_addr&0xff);
    ptr_regs3->osd_data_address_offset = osd_data_addr_offset_0_2;
    attribute_regs->chan_rect_attr_regs_group_op->chan_rectangle_attr_regs_set(attribute_regs, rectangle->rectangle_id);
}

static void osd_rectangle_set_rectangle_map_mode(osd_rectangle_entry_t *rectangle, int mode)
{
    struct osd_attribute_regs3	*ptr_regs3;
    ptr_regs3 = &rectangle->attr_regs->reg3_value.reg3;
    ptr_regs3->data_base_map_mode = mode;
}

static int  osd_rectangle_delete_rectangle_data_req(osd_rectangle_entry_t *rectangle)
{
    osd_chan_engine_t           *osd_chan_engine = rectangle->osd_chan_engine;
    osd_rectangle_dpram_tcb_t   *osd_dpram_req;
    isil_chip_t                 *chip;
    int ret;

    chip = osd_chan_engine->encode_chan->chip;
    osd_dpram_req = &rectangle->osd_dpram_req;
    ret = osd_dpram_req->op->delete_write_osd_rectangle_data_req(osd_dpram_req, chip);
    return ret;
}

static struct osd_rectangle_operation   osd_rectangle_op = 
{
    .reset = osd_rectangle_reset,
    .close_rectangle = osd_rectangle_close_rectangle,
    .get_mb_number_at_rectangle = osd_rectangle_get_mb_number_at_rectangle,
    .get_mb24_number_at_rectangle = osd_rectangle_get_mb24_number_at_rectangle,
    .put_mb_into_rectangle = osd_rectangle_put_mb_into_rectangle,
    .put_mb24_into_rectangle = osd_rectangle_put_mb24_into_rectangle,
    .try_get_mb_from_rectangle = osd_rectangle_try_get_mb_from_rectangle,
    .update_rectangle_fpga_buf = osd_rectangle_update_rectangle_fpga_buf,
    .update_rectangle_attr_regs = osd_rectangle_update_rectangle_attr_regs,
    .decide_rectangle_data_addr_map = osd_rectangle_decide_rectangle_data_addr_map,
    .set_rectangle_map_mode = osd_rectangle_set_rectangle_map_mode,
    .delete_rectangle_data_req = osd_rectangle_delete_rectangle_data_req,
};

#ifdef	TESTING_TIMESTAMP
static int  osd_chan_engine_create_timer_rectangle_testing_timestamp(osd_chan_engine_t *engine, unsigned int timestamp, int encode_ptr, int ref_ptr, int fps, int fpm)
#else
static int  osd_chan_engine_create_timer_rectangle(osd_chan_engine_t *engine)
#endif
{
    osd_attribute_regs_group_t  *attribute_regs;
    isil_h264_logic_encode_chan_t   *encode_chan;
    isil_h264_encode_delta_t    *delta;
    isil_chip_t         *chip;
    chip_driver_t       *chip_driver;
    isil_video_bus_t    *video_bus;
    isil_vd_cross_bus_t *vd_bus;
    isil_h264_phy_video_slot_t  *phy_video_slot;
    DVMNVS_CHIP_VIDEO_ENCODE_CAPABILITY *video_encode_cap;
    encode_osd_timer_attr_t *timer_attr;
    osd_rectangle_entry_t   *rectangle;
    osd_mb_pool_t           *mb_pool;
    osd_mb_char_entry_t     *mb;
    osd_system_timer_t      *timer;
    u8  *char_lib_addr, *src_buf, *merge_buf, *string_ptr[2], string_len[2], mb_line_width[2], mb_line_height;
    u8  temp_buf[YEAR_TIME_STRING_LEN];
    int mb_left_x, mb_left_y, mb_line_number, merge_buf_offset;
    int i, pixel_x, delta_x, font_right_pixel, font_pixel_width, mb_right_pixel;
    int mb_count, char_len;
    int phy_slot_id, osd_slot_id, cant_get_mb_buf=0;

    attribute_regs = &engine->attribute_regs;
    encode_chan = engine->encode_chan;
    phy_slot_id = encode_chan->phy_slot_id;
    delta = &encode_chan->encode_control.encode_delta;
    engine->op->get_rectangle(engine, &rectangle, TIMER_RECTANGLE_ENCODE_OSD, TIMER_RECTANGLE_ID);
    if(rectangle == NULL){
        return 0;
    }
    timer_attr = rectangle->private_attri.time_rectangle_attribute;
    timer = timer_attr->timer;
    chip = rectangle->chip;
    chip_driver = chip->chip_driver;
    video_bus = &chip_driver->video_bus;
    vd_bus = video_bus->vd_bus_driver;
    video_encode_cap = chip->video_encode_cap;
    mb_pool = &engine->osd_chan_pool;

    if(timer_attr->need_update_flag){
        if(timer->need_update_map_table&(1<<encode_chan->logic_chan_id)){
            engine->osd_rectangle_need_transfer_data_flag |= ((1<<TIMER_RECTANGLE_ID)|(1<<ATTRIBUT_DATA_ID));
#ifdef  TESTING_TIMESTAMP

#else
            timer->need_update_map_table &= ~(1<<encode_chan->logic_chan_id);
#endif
            mb_left_x = timer_attr->mb_x;
            mb_left_y = timer_attr->mb_y;
            string_ptr[0] = string_ptr[1] = temp_buf;
            string_len[0] = string_len[1] = 0;
            mb_line_width[0] = mb_line_width[1] = 0;
            mb_line_number = 1;
            switch(timer_attr->timer_display_mode){
                default:
                    timer_attr->need_update_flag = 0;
                    attribute_regs->chan_rect_attr_regs_group_op->chan_rectangle_attr_regs_reset(attribute_regs, rectangle->rectangle_id);
                    mb_line_number = 0;
                    cant_get_mb_buf = 1;
                    engine->osd_rectangle_need_transfer_data_flag &= ~(1<<TIMER_RECTANGLE_ID);
                    return 0;
                case YEAR_MONTH_DAY_HOUR_MINUTE_SECOND:
#ifdef	TESTING_TIMESTAMP
                    string_len[0] = snprintf(string_ptr[0], YEAR_TIME_STRING_LEN, "[%d_%c(%d,%d,%d)]:%u-%d-%d %d-%d", rectangle->chan_id, ((engine->masterOrSub == ISIL_SUB_BITSTREAM) ? 'S' : 'M'), delta->max_delta, delta->cur_delta, delta->min_delta, timestamp, encode_ptr, ref_ptr, fps, fpm);
#else
                    string_len[0] = snprintf(string_ptr[0], YEAR_TIME_STRING_LEN, "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d", timer->year, timer->month, timer->day, timer->hour, timer->minute, timer->second);
#endif
                    break;
                case YEAR_NONTH_DAY_NEWLINE_HOUR_MINUTE_SECOND_LEFT_ALIGN:
                case YEAR_MONTH_DAY_NEWLINE_HOUR_MINUTE_SECOND_RIGHT_ALIGN:
                    string_len[0] = snprintf(string_ptr[0], YEAR_TIME_STRING_LEN, " %2.2d:%2.2d:%2.2d", timer->hour, timer->minute, timer->second);
                    string_ptr[1] = string_ptr[0] + string_len[0] + 1;
                    string_len[1] = snprintf(string_ptr[1], YEAR_STRING_LEN, "%4.4d-%2.2d-%2.2d", timer->year, timer->month, timer->day);
                    mb_line_number++;
                    break;
            }
            rectangle->rectangle_left_x = mb_left_x;
            rectangle->rectangle_left_y = mb_left_y;
            mb_line_height = 0;
            for(i=0; i<mb_line_number; i++){
                if(string_len[i] == 0){
                    continue;
                }
                src_buf = string_ptr[i];
                pixel_x = delta_x = 0;
                font_right_pixel = font_pixel_width = mb_right_pixel = 0;
                char_lib_addr = merge_buf = NULL;
                mb_count = 0;
                char_len = 0;
                if(timer_attr->font_type == FONT_TYPE_12X12){
                    do{
                        if(mb_right_pixel == 0){
                            mb_pool->op->try_get(mb_pool, &mb, rectangle);
                            if(mb == NULL){
                                cant_get_mb_buf = 1;
                                break;
                            }
                            merge_buf = (u8*)mb->payload;
                            mb->payload_len = OSD_MB_BUF_CHAR_BITMAP_LEN;                                
                            mb_right_pixel = MB_PIXEL_WIDTH;
                        }
                        if((font_right_pixel==0) && (char_len<string_len[i])){
                            get_lib12_addr(&char_lib_addr, src_buf);
                            if(char_lib_addr != NULL){
                                font_right_pixel = font_pixel_width = FONT12_PIXEL_WIDTH;
                                char_len += 2;
                                src_buf += 2;
                            } else {
                                get_ascii_lib12_addr(&char_lib_addr, src_buf);
                                font_right_pixel = font_pixel_width = ASCII12_PIXEL_WIDTH;
                                char_len += 1;
                                src_buf += 1;
                            }
                        }
                        merge_buf_offset = pixel_x%MB_PIXEL_WIDTH;
                        if(font_right_pixel != 0){
                            delta_x = H264_MIN(mb_right_pixel, font_right_pixel);
                        } else {
                            delta_x = mb_right_pixel;
                        }
                        if(merge_buf!=NULL && char_lib_addr!=NULL){
                            memcpy(&merge_buf[merge_buf_offset], &char_lib_addr[font_pixel_width-font_right_pixel], delta_x);//process 123
                            memcpy(&merge_buf[merge_buf_offset+MB_PIXEL_WIDTH], &char_lib_addr[font_pixel_width-font_right_pixel+font_pixel_width], delta_x);
                        }
                        pixel_x += delta_x;
                        font_right_pixel -= delta_x;
                        if(font_right_pixel <= 0){
                            char_lib_addr = NULL;
                            font_right_pixel = 0;
                        }
                        mb_right_pixel -= delta_x;
                        if(mb_right_pixel <= 0){
                            merge_buf = NULL;
                            mb_right_pixel = 0;
                            mb->ref_mb_x = mb_count++;
                            mb->ref_mb_y = 0;
                            rectangle->op->put_mb_into_rectangle(rectangle, mb);
                        }
                    }while(((font_right_pixel!=0) || (mb_right_pixel!=0) || (char_len < string_len[i])) && (pixel_x<WIDTH_FRAME_D1_PAL));
                    mb_line_height++;
                } else {
                    osd_mb_char_entry_t *mb_24;
                    u8	*merge_buf_24=NULL;
                    do{
                        if(mb_right_pixel == 0){
                            mb_pool->op->try_get(mb_pool, &mb, rectangle);
                            if(mb == NULL){
                                cant_get_mb_buf = 1;
                                break;                                
                            }
                            merge_buf = (u8*)mb->payload;
                            mb->payload_len = OSD_MB_BUF_CHAR_BITMAP_LEN;
                            mb_right_pixel = MB_PIXEL_WIDTH;
                            mb_pool->op->try_get(mb_pool, &mb_24, rectangle);
                            if(mb_24 == NULL){
                                cant_get_mb_buf = 1;
                                break;                                
                            }
                            merge_buf_24 = (u8*)mb_24->payload;
                            mb_24->payload_len = OSD_MB_BUF_CHAR_BITMAP_LEN;
                        }
                        if((font_right_pixel==0) && (char_len<string_len[i])){
                            get_lib24_addr(&char_lib_addr, src_buf);
                            if(char_lib_addr != NULL){
                                font_right_pixel = font_pixel_width = FONT24_PIXEL_WIDTH;
                                char_len += 2;
                                src_buf += 2;
                            } else {
                                get_ascii_lib24_addr(&char_lib_addr, src_buf);
                                font_right_pixel = font_pixel_width = ASCII24_PIXEL_WIDTH;
                                char_len += 1;
                                src_buf += 1;
                            }
                        }
                        merge_buf_offset = pixel_x%MB_PIXEL_WIDTH;
                        if(font_right_pixel != 0){
                            delta_x = H264_MIN(mb_right_pixel, font_right_pixel);
                        } else {
                            delta_x = mb_right_pixel;
                        }
                        if(merge_buf!=NULL && char_lib_addr!=NULL){
                            memcpy(&merge_buf[merge_buf_offset], &char_lib_addr[font_pixel_width-font_right_pixel], delta_x);
                            memcpy(&merge_buf[merge_buf_offset+MB_PIXEL_WIDTH], &char_lib_addr[font_pixel_width-font_right_pixel+font_pixel_width], delta_x);
                        }
                        if(merge_buf_24!=NULL && char_lib_addr!=NULL){
                            memcpy(&merge_buf_24[merge_buf_offset], &char_lib_addr[font_pixel_width-font_right_pixel+font_pixel_width+font_pixel_width], delta_x);
                            memcpy(&merge_buf_24[merge_buf_offset+MB_PIXEL_WIDTH], &char_lib_addr[font_pixel_width-font_right_pixel+font_pixel_width+font_pixel_width+font_pixel_width], delta_x);
                        }
                        pixel_x += delta_x;
                        font_right_pixel -= delta_x;
                        if(font_right_pixel <= 0){
                            char_lib_addr = NULL;
                            font_right_pixel = 0;
                        }
                        mb_right_pixel -= delta_x;
                        if(mb_right_pixel <= 0){
                            merge_buf = NULL;
                            mb_right_pixel = 0;
                            mb->ref_mb_x = mb_count;
                            mb->ref_mb_y = 0;
                            rectangle->op->put_mb_into_rectangle(rectangle, mb);
                            mb_24->ref_mb_x = mb_count++;
                            mb_24->ref_mb_y = 1;
                            rectangle->op->put_mb24_into_rectangle(rectangle, mb_24);
                        }
                    }while(((font_right_pixel!=0) || (mb_right_pixel!=0) || (char_len < string_len[i])) && (pixel_x<WIDTH_FRAME_D1_PAL));
                    mb_line_height += 2;
                }
                mb_line_width[i] = mb_count;
                if(cant_get_mb_buf){
                    break;
                }
            }
            if(cant_get_mb_buf){
                rectangle->need_update_attr = 0;
                rectangle->need_update_data = 0;
                rectangle->osd_enable = 0;
                do{
                    rectangle->op->try_get_mb_from_rectangle(rectangle, &mb);
                    if(mb != NULL){
                        mb->op->release(mb);
                    } else {
                        break;
                    }
                }while(1);
                engine->osd_rectangle_need_transfer_data_flag &= ~(1<<TIMER_RECTANGLE_ID);
                attribute_regs->chan_rect_attr_regs_group_op->chan_rectangle_attr_regs_reset(attribute_regs, rectangle->rectangle_id);
                timer->need_update_map_table |= (1<<encode_chan->logic_chan_id);
                ISIL_DBG(ISIL_DBG_ERR, "chan_%d\n", encode_chan->logic_chan_id);
            } else {
                mb_count = H264_MAX(mb_line_width[0], mb_line_width[1]);
                if(mb_count){
                    rectangle->rectangle_right_x = timer_attr->mb_x + mb_count - 1;
                } else {
                    rectangle->rectangle_right_x = rectangle->rectangle_left_x;
                }
                if(mb_line_height){
                    rectangle->rectangle_right_y = timer_attr->mb_y + mb_line_height - 1;
                } else {
                    rectangle->rectangle_right_y = rectangle->rectangle_left_y;
                }
                vd_bus->op->get_phy_video_slot_by_phy_id(vd_bus, phy_slot_id, &phy_video_slot);
                osd_slot_id = get_phy_slot_id_by_osd_chan_base(vd_bus, engine->chan_data_base, phy_slot_id, video_encode_cap->ddr_map_mode);
                vd_bus->op->get_phy_video_slot_by_phy_id(vd_bus, osd_slot_id, &phy_video_slot);
                engine->videoSizeMode = phy_video_slot->video_size;
                rectangle->op->decide_rectangle_data_addr_map(rectangle, engine->videoSizeMode);
                rectangle->need_update_attr = 1;
                rectangle->need_update_data = 1;
                rectangle->osd_enable = 1;
            }
        }
    }
    return 0;
}

static int  osd_chan_engine_create_char_rectangle(osd_chan_engine_t *engine, int type_id)
{
    osd_attribute_regs_group_t  *attribute_regs;
    isil_h264_logic_encode_chan_t *encode_chan;
    isil_h264_encode_control_t    *encode_control;
    isil_chip_t                  *chip;
    chip_driver_t   *chip_driver;
    isil_video_bus_t  *video_bus;
    isil_vd_cross_bus_t   *vd_bus;
    isil_h264_phy_video_slot_t	*phy_video_slot;
    DVMNVS_CHIP_VIDEO_ENCODE_CAPABILITY *video_encode_cap;
    osd_mb_pool_t               *mb_pool;
    osd_mb_char_entry_t         *mb;
    osd_rectangle_entry_t       *rectangle;
    encode_osd_string_attr_t    *string_attr;
    u8	*char_lib_addr, *merge_buf, *src_buf;
    int mb_number, char_len;
    int	pixel_x, delta_x, font_pixel_width, merge_buf_offset, mb_right_pixel, font_right_pixel, mb_count;
    int phy_slot_id, osd_slot_id, cant_get_buf=0;

    attribute_regs = &engine->attribute_regs;
    encode_chan = engine->encode_chan;
    phy_slot_id = encode_chan->phy_slot_id;
    encode_control = &encode_chan->encode_control;
    engine->op->get_rectangle(engine, &rectangle, CHAR_RECTANGLE_ENCODE_OSD, type_id);
    if(rectangle == NULL){
        return -1;
    }
    string_attr = rectangle->private_attri.string_rectangle_attribute;
    chip = rectangle->chip;
    chip_driver = chip->chip_driver;
    video_bus   = &chip_driver->video_bus;
    vd_bus      = video_bus->vd_bus_driver;
    video_encode_cap = chip->video_encode_cap;
    mb_pool = &engine->osd_chan_pool;

    if(string_attr->need_update_flag){
#ifdef	TESTING_TIMESTAMP
#else
        string_attr->need_update_flag = 0;
#endif
        mb_number = (string_attr->string_len+1)>>1;
        //ISIL_DBG(ISIL_DBG_ERR, "chan_%d, %d\n", encode_chan->logic_chan_id, type_id);
        engine->osd_rectangle_need_transfer_data_flag |= ((1<<type_id)|(1<<ATTRIBUT_DATA_ID));
        if(string_attr->string_len <= 0){
            attribute_regs->chan_rect_attr_regs_group_op->chan_rectangle_attr_regs_reset(attribute_regs, rectangle->rectangle_id);
            engine->osd_rectangle_need_transfer_data_flag &= ~(1<<type_id);
        } else {
            if(string_attr->font_type == FONT_TYPE_12X12){
                mb_number *= 12;
                mb_number += 15;
                mb_number >>= 4;
            } else {
                mb_number *= 24;
                mb_number += 15;
                mb_number >>= 4;
            }
            if((mb_number+string_attr->mb_x) < encode_control->i_mb_x){
                string_attr->mb_width = mb_number - 1;
                string_attr->mb_height = 0;
            } else {
                string_attr->mb_width = encode_control->i_mb_x - 1 - string_attr->mb_x;
                string_attr->mb_height = mb_number/string_attr->mb_width;
                string_attr->mb_height = H264_MIN(string_attr->mb_height, (encode_control->i_mb_y - 1 - string_attr->mb_y));
            }
            if(string_attr->mb_width < 16){
                string_attr->data_addr_map_mode = Y64MB_X16MB_OSDMODE013;
                string_attr->mb_height = H264_MIN(63, string_attr->mb_height);
            } else if(string_attr->mb_width < 32){
                string_attr->data_addr_map_mode = Y32MB_X32MB_OSDMODE013;
                string_attr->mb_height = H264_MIN(31, string_attr->mb_height);
            } else if(string_attr->mb_width < 64){
                string_attr->data_addr_map_mode = Y16MB_X64MB_OSDMODE013;
                string_attr->mb_height = H264_MIN(15, string_attr->mb_height);
            } else if(string_attr->mb_width < 128){
                string_attr->data_addr_map_mode = Y8MB_X128MB_OSDMODE013;
                string_attr->mb_height = H264_MIN(7, string_attr->mb_height);
            } else {
                string_attr->mb_width = 127;
                string_attr->data_addr_map_mode = Y8MB_X128MB_OSDMODE013;
                string_attr->mb_height = H264_MIN(7, string_attr->mb_height);
            }
            rectangle->rectangle_left_x = string_attr->mb_x;
            rectangle->rectangle_left_y = string_attr->mb_y;
            rectangle->rectangle_right_y = string_attr->mb_y;

            src_buf = string_attr->string;
            pixel_x = delta_x = 0;
            font_right_pixel = font_pixel_width = mb_right_pixel = 0;
            char_lib_addr = merge_buf = NULL;
            mb_count = 0;
            char_len = 0;
            if(string_attr->font_type == FONT_TYPE_12X12){
                do{
                    if(mb_right_pixel == 0){
                        mb_pool->op->try_get(mb_pool, &mb, rectangle);
                        mb_right_pixel = MB_PIXEL_WIDTH;
                        if(mb == NULL){
                            cant_get_buf = 1;
                            break;
                        }
                        merge_buf = (u8*)mb->payload;
                        mb->payload_len = OSD_MB_BUF_CHAR_BITMAP_LEN;
                    }
                    if((font_right_pixel==0) && (char_len<string_attr->string_len)){
                        get_lib12_addr(&char_lib_addr, src_buf);
                        if(char_lib_addr != NULL){
                            font_right_pixel = font_pixel_width = FONT12_PIXEL_WIDTH;
                            char_len += 2;
                            src_buf += 2;
                        } else {
                            get_ascii_lib12_addr(&char_lib_addr, src_buf);
                            font_right_pixel = font_pixel_width = ASCII12_PIXEL_WIDTH;
                            char_len += 1;
                            src_buf += 1;
                        }
                    }
                    merge_buf_offset = pixel_x%MB_PIXEL_WIDTH;
                    if(font_right_pixel != 0){
                        delta_x = H264_MIN(mb_right_pixel, font_right_pixel);
                    } else {
                        delta_x = mb_right_pixel;
                    }
                    if(merge_buf!=NULL && char_lib_addr!=NULL){
                        memcpy(&merge_buf[merge_buf_offset], &char_lib_addr[font_pixel_width-font_right_pixel], delta_x);//process 123
                        memcpy(&merge_buf[merge_buf_offset+MB_PIXEL_WIDTH], &char_lib_addr[font_pixel_width-font_right_pixel+font_pixel_width], delta_x);
                    }
                    pixel_x += delta_x;
                    font_right_pixel -= delta_x;
                    if(font_right_pixel <= 0){
                        char_lib_addr = NULL;
                        font_right_pixel = 0;
                    }
                    mb_right_pixel -= delta_x;
                    if(mb_right_pixel <= 0){
                        merge_buf = NULL;
                        mb_right_pixel = 0;
                        mb->ref_mb_x = mb_count++;
                        mb->ref_mb_y = 0;
                        rectangle->op->put_mb_into_rectangle(rectangle, mb);
                    }
                }while(((font_right_pixel!=0) || (mb_right_pixel!=0) || (char_len < string_attr->string_len)) && (pixel_x < WIDTH_FRAME_D1_PAL));
            } else {
                osd_mb_char_entry_t *mb_24;
                u8	*merge_buf_24=NULL;
                do{
                    if(mb_right_pixel == 0){
                        mb_pool->op->try_get(mb_pool, &mb, rectangle);
                        if(mb == NULL){
                            cant_get_buf = 1;
                            break;
                        }
                        mb_right_pixel = MB_PIXEL_WIDTH;
                        merge_buf = (u8*)mb->payload;
                        mb->payload_len = OSD_MB_BUF_CHAR_BITMAP_LEN;
                        mb_pool->op->try_get(mb_pool, &mb_24, rectangle);
                        if(mb_24 == NULL){
                            cant_get_buf = 1;
                            break;
                        }
                        merge_buf_24 = (u8*)mb_24->payload;
                        mb_24->payload_len = OSD_MB_BUF_CHAR_BITMAP_LEN;
                    }
                    if((font_right_pixel==0) && (char_len<string_attr->string_len)){
                        get_lib24_addr(&char_lib_addr, src_buf);
                        if(char_lib_addr != NULL){
                            font_right_pixel = font_pixel_width = FONT24_PIXEL_WIDTH;
                            char_len += 2;
                            src_buf += 2;
                        } else {
                            get_ascii_lib24_addr(&char_lib_addr, src_buf);
                            font_right_pixel = font_pixel_width = ASCII24_PIXEL_WIDTH;
                            char_len += 1;
                            src_buf += 1;
                        }
                    }
                    merge_buf_offset = pixel_x%MB_PIXEL_WIDTH;
                    if(font_right_pixel != 0){
                        delta_x = H264_MIN(mb_right_pixel, font_right_pixel);
                    } else {
                        delta_x = mb_right_pixel;
                    }
                    if(merge_buf!=NULL && char_lib_addr!=NULL){
                        memcpy(&merge_buf[merge_buf_offset], &char_lib_addr[font_pixel_width-font_right_pixel], delta_x);
                        memcpy(&merge_buf[merge_buf_offset+MB_PIXEL_WIDTH], &char_lib_addr[font_pixel_width-font_right_pixel+font_pixel_width], delta_x);
                    }
                    if(merge_buf_24!=NULL && char_lib_addr!=NULL){
                        memcpy(&merge_buf_24[merge_buf_offset], &char_lib_addr[font_pixel_width-font_right_pixel+font_pixel_width+font_pixel_width], delta_x);
                        memcpy(&merge_buf_24[merge_buf_offset+MB_PIXEL_WIDTH], &char_lib_addr[font_pixel_width-font_right_pixel+font_pixel_width+font_pixel_width+font_pixel_width], delta_x);
                    }
                    pixel_x += delta_x;
                    font_right_pixel -= delta_x;
                    if(font_right_pixel <= 0){
                        char_lib_addr = NULL;
                        font_right_pixel = 0;
                    }
                    mb_right_pixel -= delta_x;
                    if(mb_right_pixel <= 0){
                        merge_buf = NULL;
                        mb_right_pixel = 0;
                        mb->ref_mb_x = mb_count;
                        mb->ref_mb_y = 0;
                        rectangle->op->put_mb_into_rectangle(rectangle, mb);
                        mb_24->ref_mb_x = mb_count++;
                        mb_24->ref_mb_y = 1;
                        rectangle->op->put_mb24_into_rectangle(rectangle, mb_24);
                    }
                }while(((font_right_pixel!=0) || (mb_right_pixel!=0) || (char_len < string_attr->string_len)) && (pixel_x < WIDTH_FRAME_D1_PAL));
                rectangle->rectangle_right_y++;
            }

            if(cant_get_buf){
                rectangle->need_update_attr = 0;
                rectangle->need_update_data = 0;
                rectangle->osd_enable = 0;
                do{
                    rectangle->op->try_get_mb_from_rectangle(rectangle, &mb);
                    if(mb != NULL){
                        mb->op->release(mb);
                    } else {
                        break;
                    }
                }while(1);
                engine->osd_rectangle_need_transfer_data_flag &= ~(1<<type_id);
                attribute_regs->chan_rect_attr_regs_group_op->chan_rectangle_attr_regs_reset(attribute_regs, rectangle->rectangle_id);
                ISIL_DBG(ISIL_DBG_ERR, "chan_%d, %d\n", encode_chan->logic_chan_id, type_id);
            } else {
                rectangle->rectangle_right_x = string_attr->mb_x + mb_count - 1;
                vd_bus->op->get_phy_video_slot_by_phy_id(vd_bus, phy_slot_id, &phy_video_slot);
                osd_slot_id = get_phy_slot_id_by_osd_chan_base(vd_bus, engine->chan_data_base, phy_slot_id, video_encode_cap->ddr_map_mode);
                vd_bus->op->get_phy_video_slot_by_phy_id(vd_bus, osd_slot_id, &phy_video_slot);
                engine->videoSizeMode = phy_video_slot->video_size;
                rectangle->op->decide_rectangle_data_addr_map(rectangle, engine->videoSizeMode);
                rectangle->need_update_attr = 1;
                rectangle->need_update_data = 1;
                rectangle->osd_enable = 1;
            }
        }
    }
    return 0;
}

static int  osd_chan_engine_create_mask_rectangle(osd_chan_engine_t *engine, int type_id)
{
    isil_chip_t     *chip;
    chip_driver_t   *chip_driver;
    isil_video_bus_t    *video_bus;
    isil_vd_cross_bus_t *vd_bus;
    isil_h264_phy_video_slot_t  *phy_video_slot;
    DVMNVS_CHIP_VIDEO_ENCODE_CAPABILITY *video_encode_cap;
    osd_attribute_regs_group_t  *attribute_regs;
    isil_h264_logic_encode_chan_t   *encode_chan;
    osd_rectangle_entry_t       *rectangle;
    encode_osd_mask_attr_t      *mask_attr;
    int phy_slot_id, osd_slot_id;

    attribute_regs = &engine->attribute_regs;
    encode_chan = engine->encode_chan;
    chip        = encode_chan->chip;
    chip_driver = chip->chip_driver;
    video_bus   = &chip_driver->video_bus;
    vd_bus      = video_bus->vd_bus_driver;
    video_encode_cap = chip->video_encode_cap;
    phy_slot_id = encode_chan->phy_slot_id;
    engine->op->get_rectangle(engine, &rectangle, MASK_RECTANGLE_ENCODE_OSD, type_id);
    if(rectangle == NULL){
        return -1;
    }
    mask_attr = rectangle->private_attri.mask_rectangle_attribute;
    if(mask_attr->need_update_flag){
#ifdef	TESTING_TIMESTAMP
#else
        mask_attr->need_update_flag = 0;
#endif
        engine->osd_rectangle_need_transfer_data_flag |= ((1<<type_id)|(1<<ATTRIBUT_DATA_ID));
        rectangle->osd_enable = 1;
        rectangle->need_update_attr = 1;
        rectangle->need_update_data = 1;
        if(mask_attr->is_display == 0){
            attribute_regs->chan_rect_attr_regs_group_op->chan_rectangle_attr_regs_reset(attribute_regs, rectangle->rectangle_id);
            engine->osd_rectangle_need_transfer_data_flag &= ~(1<<type_id);
        } else {
            mask_attr->data_addr_map_mode = Y32MB_X32MB_OSDMODE013;
            rectangle->rectangle_left_x = mask_attr->mb_x;
            rectangle->rectangle_left_y = mask_attr->mb_y;
            rectangle->rectangle_right_x = mask_attr->mb_x + mask_attr->mask_width;
            rectangle->rectangle_right_y = mask_attr->mb_y + mask_attr->mask_height;
            vd_bus->op->get_phy_video_slot_by_phy_id(vd_bus, phy_slot_id, &phy_video_slot);
            osd_slot_id = get_phy_slot_id_by_osd_chan_base(vd_bus, engine->chan_data_base, phy_slot_id, video_encode_cap->ddr_map_mode);
            vd_bus->op->get_phy_video_slot_by_phy_id(vd_bus, osd_slot_id, &phy_video_slot);
            engine->videoSizeMode = phy_video_slot->video_size;
            rectangle->op->decide_rectangle_data_addr_map(rectangle, engine->videoSizeMode);
        }
    }
    return 0;
}

#ifdef	TESTING_TIMESTAMP
static int  osd_chan_engine_create_rectangle_testing_timestamp(osd_chan_engine_t *engine, ISIL_OSD_PARAM *osd_running_param, int masterOrSub, int encodeSize, unsigned int timestamp, int encode_ptr, int ref_ptr, int fps, int fpm)
#else
static int  osd_chan_engine_create_rectangle(osd_chan_engine_t *engine, ISIL_OSD_PARAM *osd_running_param, int masterOrSub, int encodeSize)
#endif
{
    encode_osd_string_attr_t    *string_attr;
    encode_osd_timer_attr_t     *timer_attr;
    encode_osd_mask_attr_t      *mask_attr;
    isil_h264_logic_encode_chan_t *encode_chan;
    isil_h264_encode_property_t   *encode_property;
    isil_h264_phy_video_slot_t    *master_slot;
    isil_h264_encode_control_t    *encode_control;
    isil_video_bus_t              *video_bus;
    unsigned long       flags;
    int font_type;

    spin_lock_irqsave(&engine->osd_lock, flags);
    encode_chan = engine->encode_chan;
    if(atomic_read(&encode_chan->first_start_flag)){
        complete_all(&engine->osd_transfering_done);
        spin_unlock_irqrestore(&engine->osd_lock, flags);
        return 0;
    }
    if(atomic_read(&engine->osd_encode_start_close) &&
            (atomic_read(&engine->osd_working)==0) &&
            ((engine->osd_rectangle_need_transfer_data_flag&OSD_RECTANGLE_AND_ATTR_MASK)==0)){
        complete_all(&engine->osd_transfering_done);
        spin_unlock_irqrestore(&engine->osd_lock, flags);
        return 0;
    }
    if(atomic_read(&engine->osd_encode_start_close) == 0){
        if(atomic_read(&engine->sync_osd_param)){
            atomic_set(&engine->sync_osd_param, 0);
            encode_control = &encode_chan->encode_control;
            master_slot = encode_chan->master_slot;
            encode_property = &encode_chan->encode_property;
            video_bus = &encode_chan->chip->chip_driver->video_bus;
            engine->masterOrSub = masterOrSub;

            if(master_slot){
                if(encode_chan->type == ISIL_MASTER_BITSTREAM){
                    font_type = FONT_TYPE_24X24;
                } else {
                    font_type = FONT_TYPE_12X12;
                }
                switch(encodeSize){
                    case ISIL_VIDEO_SIZE_4CIF:
                    case ISIL_VIDEO_SIZE_D1:
                    case ISIL_VIDEO_SIZE_2CIF:
                    case ISIL_VIDEO_SIZE_HALF_D1:
                    case ISIL_VIDEO_SIZE_USER:
                        font_type = FONT_TYPE_24X24;
                        break;
                    default:
                    case ISIL_VIDEO_SIZE_CIF:
                        font_type = FONT_TYPE_12X12;
                        break;
                }

                timer_attr = &engine->timer_attr;
                if((osd_running_param->time_attrib&OSD_ATTR_DISPLAY_ON) == OSD_ATTR_DISPLAY_ON){
                    timer_attr->mb_x = osd_running_param->time_pos_x>>4;
                    timer_attr->mb_y = osd_running_param->time_pos_y>>4;
                    timer_attr->mb_x = H264_MIN(timer_attr->mb_x, encode_control->i_mb_x-1-CHAR_RECTANGLE_X_LEFT);
                    timer_attr->mb_y = H264_MIN(timer_attr->mb_y, encode_control->i_mb_y-1-CHAR_RECTANGLE_Y_LEFT);
                    timer_attr->timer_display_mode = YEAR_MONTH_DAY_HOUR_MINUTE_SECOND;
                    timer_attr->font_type = font_type;
                    timer_attr->op->reset(timer_attr, encode_chan->chip->chip_id, encode_chan->logic_chan_id);
                } else {
                    timer_attr->mb_x = 0;
                    timer_attr->mb_y = 0;
                    timer_attr->timer_display_mode = DONT_DISPLAY_TIMER;
                }
                timer_attr->need_update_flag = 1;

                string_attr = &engine->string_attr[CHAN_NAME_RECTANGLE_ID-1];
                if((osd_running_param->name_attrib&OSD_ATTR_DISPLAY_ON) == OSD_ATTR_DISPLAY_ON){
                    string_attr->font_type = font_type;
                    string_attr->mb_x = osd_running_param->name_pos_x>>4;
                    string_attr->mb_y = osd_running_param->name_pos_y>>4;
                    string_attr->mb_x = H264_MIN(string_attr->mb_x, encode_control->i_mb_x-1-CHAR_RECTANGLE_X_LEFT);
                    string_attr->mb_y = H264_MIN(string_attr->mb_y, encode_control->i_mb_y-1-CHAR_RECTANGLE_Y_LEFT);
                    string_attr->mb_height = 0;
                    string_attr->string_len = strlen(osd_running_param->name);
                    if(string_attr->string_len>0){
                        string_attr->string_len = H264_MIN(string_attr->string_len, NAME_LEN);
                        string_attr->string = osd_running_param->name;
                        string_attr->mb_width = ((string_attr->string_len+1)>>1) - 1;
                    } else {
                        string_attr->string_len = 0;
                    }
                } else {
                    string_attr->string = NULL;
                    string_attr->string_len = 0;
                }
                string_attr->need_update_flag = 1;

                string_attr = &engine->string_attr[SUBTILTE1_RECTANGLE_ID-1];
                if((osd_running_param->subtitle1_attrib&OSD_ATTR_DISPLAY_ON) == OSD_ATTR_DISPLAY_ON){
                    string_attr->font_type = font_type;
                    string_attr->mb_x = osd_running_param->subtitle1_pos_x>>4;
                    string_attr->mb_y = osd_running_param->subtitle1_pos_y>>4;
                    string_attr->mb_x = H264_MIN(string_attr->mb_x, encode_control->i_mb_x-1-CHAR_RECTANGLE_X_LEFT);
                    string_attr->mb_y = H264_MIN(string_attr->mb_y, encode_control->i_mb_y-1-CHAR_RECTANGLE_Y_LEFT);
                    string_attr->mb_height = 0;
                    string_attr->string_len = strlen(osd_running_param->subtitle1);
                    if(string_attr->string_len > 0){
                        string_attr->string_len = H264_MIN(string_attr->string_len, SUB_LEN);
                        string_attr->string = osd_running_param->subtitle1;
                        string_attr->mb_width = ((string_attr->string_len+1)>>1) - 1;
                    } else {
                        string_attr->string_len = 0;
                    }
                } else {
                    string_attr->string = NULL;
                    string_attr->string_len = 0;
                }
                string_attr->need_update_flag = 1;

                string_attr = &engine->string_attr[SUBTILTE2_RECTANGLE_ID-1];
                if((osd_running_param->subtitle2_attrib&OSD_ATTR_DISPLAY_ON) == OSD_ATTR_DISPLAY_ON){
                    string_attr->font_type = font_type;
                    string_attr->mb_x = osd_running_param->subtitle2_pos_x>>4;
                    string_attr->mb_y = osd_running_param->subtitle2_pos_y>>4;
                    string_attr->mb_x = H264_MIN(string_attr->mb_x, encode_control->i_mb_x-1-CHAR_RECTANGLE_X_LEFT);
                    string_attr->mb_y = H264_MIN(string_attr->mb_y, encode_control->i_mb_y-1-CHAR_RECTANGLE_Y_LEFT);
                    string_attr->mb_height = 0;
                    string_attr->string_len = strlen(osd_running_param->subtitle2);
                    if(string_attr->string_len > 0){
                        string_attr->string_len = H264_MIN(string_attr->string_len, SUB_LEN);
                        string_attr->string = osd_running_param->subtitle2;
                        string_attr->mb_width = ((string_attr->string_len+1)>>1) - 1;
                    } else {
                        string_attr->string_len = 0;
                    }
                } else {
                    string_attr->string_len = 0;
                    string_attr->string = NULL;
                }
                string_attr->need_update_flag = 1;

                string_attr = &engine->string_attr[SUBTILTE3_RECTANGLE_ID-1];
                if((osd_running_param->subtitle3_attrib&OSD_ATTR_DISPLAY_ON) == OSD_ATTR_DISPLAY_ON){
                    string_attr->font_type = font_type;
                    string_attr->mb_x = osd_running_param->subtitle3_pos_x>>4;
                    string_attr->mb_y = osd_running_param->subtitle3_pos_y>>4;
                    string_attr->mb_x = H264_MIN(string_attr->mb_x, encode_control->i_mb_x-1-CHAR_RECTANGLE_X_LEFT);
                    string_attr->mb_y = H264_MIN(string_attr->mb_y, encode_control->i_mb_y-1-CHAR_RECTANGLE_Y_LEFT);
                    string_attr->mb_height = 0;
                    string_attr->string_len = strlen(osd_running_param->subtitle3);
                    if(string_attr->string_len > 0){
                        string_attr->string_len = H264_MIN(string_attr->string_len, SUB_LEN);
                        string_attr->string = osd_running_param->subtitle3;
                        string_attr->mb_width = ((string_attr->string_len+1)>>1) - 1;
                    } else {
                        string_attr->string_len = 0;
                    }
                } else {
                    string_attr->string_len = 0;
                    string_attr->string = NULL;
                }
                string_attr->need_update_flag = 1;

                string_attr = &engine->string_attr[SUBTILTE4_RECTANGLE_ID-1];
                if((osd_running_param->subtitle4_attrib&OSD_ATTR_DISPLAY_ON) == OSD_ATTR_DISPLAY_ON){
                    string_attr->font_type = font_type;
                    string_attr->mb_x = osd_running_param->subtitle4_pos_x>>4;
                    string_attr->mb_y = osd_running_param->subtitle4_pos_y>>4;
                    string_attr->mb_x = H264_MIN(string_attr->mb_x, encode_control->i_mb_x-1-CHAR_RECTANGLE_X_LEFT);
                    string_attr->mb_y = H264_MIN(string_attr->mb_y, encode_control->i_mb_y-1-CHAR_RECTANGLE_Y_LEFT);
                    string_attr->mb_height = 0;
                    string_attr->string_len = strlen(osd_running_param->subtitle4);
                    if(string_attr->string_len > 0){
                        string_attr->string_len = H264_MIN(string_attr->string_len, SUB_LEN);
                        string_attr->string = osd_running_param->subtitle4;
                        string_attr->mb_width = ((string_attr->string_len+1)>>1) - 1;
                    } else {
                        string_attr->string_len = 0;
                    }
                } else {
                    string_attr->string_len = 0;
                    string_attr->string = NULL;
                }
                string_attr->need_update_flag = 1;
            }

            mask_attr = &engine->mask_attr[SHELTER1_RECTANGLE_ID-1];
            if((osd_running_param->shelter1_attrib&OSD_ATTR_DISPLAY_ON) == OSD_ATTR_DISPLAY_ON){
                mask_attr->mb_x = osd_running_param->shelter1_pos_x>>4;
                mask_attr->mb_y = osd_running_param->shelter1_pos_y>>4;
                mask_attr->mb_x = H264_MIN(mask_attr->mb_x, encode_control->i_mb_x-1);
                mask_attr->mb_y = H264_MIN(mask_attr->mb_y, encode_control->i_mb_y-1);
                mask_attr->mask_width = osd_running_param->shelter1_width>>4;
                mask_attr->mask_height = osd_running_param->shelter1_height>>4;
                mask_attr->mask_width = H264_MIN(mask_attr->mask_width, encode_control->i_mb_x-mask_attr->mb_x);
                mask_attr->mask_height = H264_MIN(mask_attr->mask_height, encode_control->i_mb_y-mask_attr->mb_y);
                mask_attr->u_value = 0x80;
                mask_attr->v_value = 0x80;
                mask_attr->y_value = 0x10;
                mask_attr->is_display = 1;
            } else {
                mask_attr->is_display = 0;
            }
            mask_attr->need_update_flag = 1;

            mask_attr = &engine->mask_attr[SHELTER2_RECTANGLE_ID-1];
            if((osd_running_param->shelter2_attrib&OSD_ATTR_DISPLAY_ON) == OSD_ATTR_DISPLAY_ON){
                mask_attr->mb_x = osd_running_param->shelter2_pos_x>>4;
                mask_attr->mb_y = osd_running_param->shelter2_pos_y>>4;
                mask_attr->mb_x = H264_MIN(mask_attr->mb_x, encode_control->i_mb_x-1);
                mask_attr->mb_y = H264_MIN(mask_attr->mb_y, encode_control->i_mb_y-1);
                mask_attr->mask_width = osd_running_param->shelter2_width>>4;
                mask_attr->mask_height = osd_running_param->shelter2_height>>4;
                mask_attr->mask_width = H264_MIN(mask_attr->mask_width, encode_control->i_mb_x-mask_attr->mb_x);
                mask_attr->mask_height = H264_MIN(mask_attr->mask_height, encode_control->i_mb_y-mask_attr->mb_y);
                mask_attr->u_value = 0x80;
                mask_attr->v_value = 0x80;
                mask_attr->y_value = 0x10;
                mask_attr->is_display = 1;
            } else {
                mask_attr->is_display = 0;
            }
            mask_attr->need_update_flag = 1;
        }
    }
    if((engine->osd_rectangle_need_transfer_data_flag & (1<<ATTRIBUT_DONE_ID))){
        if((engine->osd_rectangle_need_transfer_data_flag & (1<<TIMER_RECTANGLE_ID)) == 0){
#ifdef	TESTING_TIMESTAMP
            engine->op->create_timer_rectangle(engine, timestamp, encode_ptr, ref_ptr, fps, fpm);
#else
            engine->op->create_timer_rectangle(engine);
#endif
        }
        if((engine->osd_rectangle_need_transfer_data_flag & (1<<CHAN_NAME_RECTANGLE_ID)) == 0){
            engine->op->create_char_rectangle(engine, CHAN_NAME_RECTANGLE_ID);
        }
        if((engine->osd_rectangle_need_transfer_data_flag & (1<<SHELTER1_RECTANGLE_ID)) == 0){
            engine->op->create_mask_rectangle(engine, SHELTER1_RECTANGLE_ID);
        }
        if((engine->osd_rectangle_need_transfer_data_flag & (1<<SHELTER2_RECTANGLE_ID)) == 0){
            engine->op->create_mask_rectangle(engine, SHELTER2_RECTANGLE_ID);
        }
        if((engine->osd_rectangle_need_transfer_data_flag & (1<<SUBTILTE1_RECTANGLE_ID)) == 0){
            engine->op->create_char_rectangle(engine, SUBTILTE1_RECTANGLE_ID);
        }
        if((engine->osd_rectangle_need_transfer_data_flag & (1<<SUBTILTE2_RECTANGLE_ID)) == 0){
            engine->op->create_char_rectangle(engine, SUBTILTE2_RECTANGLE_ID);
        }
        if((engine->osd_rectangle_need_transfer_data_flag & (1<<SUBTILTE3_RECTANGLE_ID)) == 0){
            engine->op->create_char_rectangle(engine, SUBTILTE3_RECTANGLE_ID);
        }
        if((engine->osd_rectangle_need_transfer_data_flag & (1<<SUBTILTE4_RECTANGLE_ID)) == 0){
            engine->op->create_char_rectangle(engine, SUBTILTE4_RECTANGLE_ID);
        }
    }
    spin_unlock_irqrestore(&engine->osd_lock, flags);
    return 0;
}

static void osd_chan_engine_release_char_rectangle(osd_chan_engine_t *engine, int type_id)
{
    osd_attribute_regs_group_t      *attribute_regs;
    isil_h264_logic_encode_chan_t   *encode_chan;
    osd_rectangle_entry_t       *rectangle;
    encode_osd_string_attr_t    *string_attr;
    osd_mb_char_entry_t         *mb;
    unsigned long           flags;

    spin_lock_irqsave(&engine->osd_lock, flags);
    if(engine->osd_rectangle_need_transfer_data_flag & (1<<type_id)){
        attribute_regs = &engine->attribute_regs;
        encode_chan = engine->encode_chan;
        engine->op->get_rectangle(engine, &rectangle, CHAR_RECTANGLE_ENCODE_OSD, type_id);
        if(rectangle != NULL){
            do{
                rectangle->op->try_get_mb_from_rectangle(rectangle, &mb);
                if(mb != NULL){
                    mb->op->release(mb);
                } else {
                    break;
                }
            }while(1);
            rectangle->need_update_attr = 0;
            rectangle->need_update_data = 0;
            rectangle->osd_enable = 0;
            attribute_regs->chan_rect_attr_regs_group_op->chan_rectangle_attr_regs_reset(attribute_regs, rectangle->rectangle_id);
            string_attr = rectangle->private_attri.string_rectangle_attribute;
            if(string_attr){
                string_attr->need_update_flag = 0;
            }
        }
        engine->osd_rectangle_need_transfer_data_flag &= ~(1<<type_id);
        if(!(OSD_RECTANGLE_MASK&engine->osd_rectangle_need_transfer_data_flag)){
            complete_all(&engine->osd_transfering_done);
        }
        ISIL_DBG(ISIL_DBG_ERR, "chan_%d\n", encode_chan->logic_chan_id);
    }
    spin_unlock_irqrestore(&engine->osd_lock, flags);
}

static void osd_chan_engine_release_mask_rectangle(osd_chan_engine_t *engine, int type_id)
{
    osd_attribute_regs_group_t      *attribute_regs;
    isil_h264_logic_encode_chan_t   *encode_chan;
    osd_rectangle_entry_t       *rectangle;
    encode_osd_mask_attr_t      *mask_attr;
    unsigned long           flags;

    spin_lock_irqsave(&engine->osd_lock, flags);
    if(engine->osd_rectangle_need_transfer_data_flag & (1<<type_id)){
        attribute_regs = &engine->attribute_regs;
        encode_chan = engine->encode_chan;
        engine->op->get_rectangle(engine, &rectangle, MASK_RECTANGLE_ENCODE_OSD, type_id);
        if(rectangle != NULL){
            rectangle->need_update_attr = 0;
            rectangle->need_update_data = 0;
            rectangle->osd_enable = 0;
            attribute_regs->chan_rect_attr_regs_group_op->chan_rectangle_attr_regs_reset(attribute_regs, rectangle->rectangle_id);
            mask_attr = rectangle->private_attri.mask_rectangle_attribute;
            if(mask_attr){
                mask_attr->need_update_flag = 0;
            }
        }
        engine->osd_rectangle_need_transfer_data_flag &= ~(1<<type_id);
        if(!(OSD_RECTANGLE_MASK&engine->osd_rectangle_need_transfer_data_flag)){
            complete_all(&engine->osd_transfering_done);
        }
        ISIL_DBG(ISIL_DBG_ERR, "chan_%d\n", encode_chan->logic_chan_id);
    }
    spin_unlock_irqrestore(&engine->osd_lock, flags);
}

static void osd_chan_engine_release_timer_rectangle(osd_chan_engine_t *engine)
{
    osd_attribute_regs_group_t      *attribute_regs;
    isil_h264_logic_encode_chan_t   *encode_chan;
    osd_rectangle_entry_t   *rectangle;
    encode_osd_timer_attr_t *timer_attr;
    osd_system_timer_t      *timer;
    osd_mb_char_entry_t     *mb;
    unsigned long           flags;

    spin_lock_irqsave(&engine->osd_lock, flags);
    if(engine->osd_rectangle_need_transfer_data_flag & (1<<TIMER_RECTANGLE_ID)){
        attribute_regs = &engine->attribute_regs;
        encode_chan = engine->encode_chan;
        engine->op->get_rectangle(engine, &rectangle, TIMER_RECTANGLE_ENCODE_OSD, TIMER_RECTANGLE_ID);
        if(rectangle != NULL){
            do{
                rectangle->op->try_get_mb_from_rectangle(rectangle, &mb);
                if(mb != NULL){
                    mb->op->release(mb);
                } else {
                    break;
                }
            }while(1);
            rectangle->need_update_attr = 0;
            rectangle->need_update_data = 0;
            rectangle->osd_enable = 0;
            timer_attr = rectangle->private_attri.time_rectangle_attribute;
            if(timer_attr){
                timer_attr->need_update_flag = 0;
            }
            timer = timer_attr->timer;
            timer->need_update_map_table |= (1<<encode_chan->logic_chan_id);
            attribute_regs->chan_rect_attr_regs_group_op->chan_rectangle_attr_regs_reset(attribute_regs, rectangle->rectangle_id);
        }
        engine->osd_rectangle_need_transfer_data_flag &= ~(1<<TIMER_RECTANGLE_ID);
        if(!(OSD_RECTANGLE_MASK&engine->osd_rectangle_need_transfer_data_flag)){
            complete_all(&engine->osd_transfering_done);
        }
        ISIL_DBG(ISIL_DBG_ERR, "chan_%d\n", encode_chan->logic_chan_id);
    }
    spin_unlock_irqrestore(&engine->osd_lock, flags);
}

static void osd_chan_engine_release_rectangle(osd_chan_engine_t *engine)
{
    engine->op->release_timer_rectangle(engine);
    engine->op->release_char_rectangle(engine, CHAN_NAME_RECTANGLE_ID);
    engine->op->release_char_rectangle(engine, SUBTILTE1_RECTANGLE_ID);
    engine->op->release_char_rectangle(engine, SUBTILTE2_RECTANGLE_ID);
    engine->op->release_char_rectangle(engine, SUBTILTE3_RECTANGLE_ID);
    engine->op->release_char_rectangle(engine, SUBTILTE4_RECTANGLE_ID);
    engine->op->release_mask_rectangle(engine, SHELTER1_RECTANGLE_ID);
    engine->op->release_mask_rectangle(engine, SHELTER2_RECTANGLE_ID);
}

static int  osd_chan_engine_close_rectangles(osd_chan_engine_t *engine)
{
    osd_rectangle_entry_t   *rectangle;
    int	i;
    unsigned long	flags;

    spin_lock_irqsave(&engine->osd_lock, flags);
    rectangle = engine->rectangle;
    for(i=0; i<OSD_CHAN_RECTANGLE_NUMBER; i++){
        rectangle->op->close_rectangle(rectangle);
        rectangle++;
    }
    spin_unlock_irqrestore(&engine->osd_lock, flags);
    return 0;
}

static void  osd_chan_engine_update_encode_osd_property(osd_chan_engine_t *engine, ISIL_OSD_PARAM *osd_config_param)
{
    ISIL_OSD_PARAM    *osd_running_param = &engine->osd_running_param;
    if(osd_config_param != NULL){
        unsigned long	flags;
        spin_lock_irqsave(&engine->osd_lock, flags);
        memcpy((char*)osd_running_param, (char*)osd_config_param, sizeof(ISIL_OSD_PARAM));
        atomic_set(&engine->sync_osd_param, 1);
        spin_unlock_irqrestore(&engine->osd_lock, flags);
    }
}

static void osd_chan_engine_update_fpga_osd_encode(osd_chan_engine_t *engine)
{
    isil_chip_t  *chip;
    osd_attribute_regs_group_t  *attribute_regs;

    chip = engine->encode_chan->chip;
    attribute_regs = &engine->attribute_regs;
    attribute_regs->chan_rect_attr_regs_group_op->update_chan_rectangle_attr_regs(attribute_regs, chip);
}

static void osd_chan_engine_get_rectangle(osd_chan_engine_t *engine, osd_rectangle_entry_t **rectangle, int type, int type_id)
{
    *rectangle = NULL;
    if(type_id>OSD_MAX_RECTANGLE_ID || type_id<OSD_MIN_RECTANGLE_ID){
        return;
    }
    engine->chan_rectangle_used_map_table |= 1<<type_id;
    *rectangle = &engine->rectangle[type_id];
    (*rectangle)->rectangle_mode = type;
    switch(type){
        case TIMER_RECTANGLE_ENCODE_OSD:
            (*rectangle)->private_attri.time_rectangle_attribute = &engine->timer_attr;
            break;
        case CHAR_RECTANGLE_ENCODE_OSD:
            (*rectangle)->private_attri.string_rectangle_attribute = &engine->string_attr[type_id-1];
            break;
        case MASK_RECTANGLE_ENCODE_OSD:
            (*rectangle)->private_attri.mask_rectangle_attribute = &engine->mask_attr[type_id-1];
            break;
        default:
            engine->chan_rectangle_used_map_table &= ~(1<<type_id);
            (*rectangle)->private_attri.empty_rectangle_attribute = NULL;
            ISIL_DBG(ISIL_DBG_ERR, "rectangle's type isn't know\n");
            break;
    }
}

static void osd_chan_engine_put_rectangle(osd_chan_engine_t *engine, osd_rectangle_entry_t *rectangle)
{
    encode_osd_timer_attr_t     *timer_attr;
    encode_osd_string_attr_t    *string_attr;
    encode_osd_mask_attr_t      *mask_attr;

    if(rectangle->rectangle_id != INVALID_OSD_VALUE_ID){
        switch(rectangle->rectangle_mode){
            case TIMER_RECTANGLE_ENCODE_OSD:
                timer_attr = rectangle->private_attri.time_rectangle_attribute;
                if(timer_attr != NULL){
                    timer_attr->op->reset(timer_attr, engine->encode_chan->chip->chip_id, engine->encode_chan->logic_chan_id);
                }
                break;
            case CHAR_RECTANGLE_ENCODE_OSD:
                string_attr = rectangle->private_attri.string_rectangle_attribute;
                if(string_attr != NULL){
                    string_attr->op->reset(string_attr);
                }
                break;
            case MASK_RECTANGLE_ENCODE_OSD:
                mask_attr = rectangle->private_attri.mask_rectangle_attribute;
                if(mask_attr != NULL){
                    mask_attr->op->reset(mask_attr);
                }
                break;
            default:
                break;
        }
        engine->chan_rectangle_used_map_table &= ~(1<<rectangle->rectangle_id);
    }
}

static int  osd_chan_engine_delete_chan_engine_rectangles(osd_chan_engine_t *engine)
{
    osd_attribute_regs_group_t  *attribute_regs;
    osd_rectangle_dpram_tcb_t   *osd_dpram_req;
    isil_chip_t                 *chip;
    osd_rectangle_entry_t   *rectangle;
    int ret = 0, i;
    unsigned long	flags;

    spin_lock_irqsave(&engine->osd_lock, flags);
    atomic_set(&engine->osd_encode_start_close, 1);
    rectangle = engine->rectangle;
    for(i=0; i<OSD_CHAN_RECTANGLE_NUMBER; i++){
        ret |= rectangle->op->delete_rectangle_data_req(rectangle);
        rectangle++;
    }
    chip = engine->encode_chan->chip;
    attribute_regs = &engine->attribute_regs;
    osd_dpram_req = &attribute_regs->osd_dpram_req;
    ret |= osd_dpram_req->op->delete_write_osd_rectangle_data_req(osd_dpram_req, chip);
    //chip->io_op->chip_write32(chip, OSD_RECTANGLE_EN_REG, 0x0);
    //chip->io_op->chip_write32(chip, OSD_ATTRI_UPDATE_REG, engine->encode_chan->phy_slot_id);
    spin_unlock_irqrestore(&engine->osd_lock, flags);
    return ret;
}

static void osd_chan_engine_reset_chan_engine(osd_chan_engine_t *engine)
{
    unsigned long   flags;

    spin_lock_irqsave(&engine->osd_lock, flags);
    atomic_set(&engine->sync_osd_param, 1);
    atomic_set(&engine->osd_working, 0);
    atomic_set(&engine->osd_encode_start_close, 0);
    engine->osd_rectangle_need_transfer_data_flag = (1<<ATTRIBUT_DONE_ID);
    init_completion(&engine->osd_transfering_done);
    complete_all(&engine->osd_transfering_done);
    spin_unlock_irqrestore(&engine->osd_lock, flags);
}

static void osd_chan_engine_submit_osd_data(osd_chan_engine_t *engine)
{
    osd_rectangle_entry_t       *rectangle;
    osd_rectangle_dpram_tcb_t   *osd_dpram_req;
    isil_chip_t         *chip;
    unsigned long       flags;

    spin_lock_irqsave(&engine->osd_lock, flags);
    chip        = engine->encode_chan->chip;
    if(engine->osd_rectangle_need_transfer_data_flag & (1<<TIMER_RECTANGLE_ID)){
        engine->op->get_rectangle(engine, &rectangle, TIMER_RECTANGLE_ENCODE_OSD, TIMER_RECTANGLE_ID);
        if(rectangle != NULL){
            osd_dpram_req = &rectangle->osd_dpram_req;
            osd_dpram_req->op->submit_write_osd_rectangle_data_req(osd_dpram_req, chip);                
        }
    }
    if(engine->osd_rectangle_need_transfer_data_flag & (1<<CHAN_NAME_RECTANGLE_ID)){
        engine->op->get_rectangle(engine, &rectangle, CHAR_RECTANGLE_ENCODE_OSD, CHAN_NAME_RECTANGLE_ID);
        if(rectangle != NULL){
            osd_dpram_req = &rectangle->osd_dpram_req;
            osd_dpram_req->op->submit_write_osd_rectangle_data_req(osd_dpram_req, chip);                
        }
    }
    if(engine->osd_rectangle_need_transfer_data_flag & (1<<SHELTER1_RECTANGLE_ID)){
        engine->op->get_rectangle(engine, &rectangle, MASK_RECTANGLE_ENCODE_OSD, SHELTER1_RECTANGLE_ID);
        if(rectangle != NULL){
            osd_dpram_req = &rectangle->osd_dpram_req;
            osd_dpram_req->op->submit_write_osd_rectangle_data_req(osd_dpram_req, chip);                
        }
    }
    if(engine->osd_rectangle_need_transfer_data_flag & (1<<SHELTER2_RECTANGLE_ID)){
        engine->op->get_rectangle(engine, &rectangle, MASK_RECTANGLE_ENCODE_OSD, SHELTER2_RECTANGLE_ID);
        if(rectangle != NULL){
            osd_dpram_req = &rectangle->osd_dpram_req;
            osd_dpram_req->op->submit_write_osd_rectangle_data_req(osd_dpram_req, chip);                
        }
    }
    if(engine->osd_rectangle_need_transfer_data_flag & (1<<SUBTILTE1_RECTANGLE_ID)){
        engine->op->get_rectangle(engine, &rectangle, CHAR_RECTANGLE_ENCODE_OSD, SUBTILTE1_RECTANGLE_ID);
        if(rectangle != NULL){
            osd_dpram_req = &rectangle->osd_dpram_req;
            osd_dpram_req->op->submit_write_osd_rectangle_data_req(osd_dpram_req, chip);                
        }
    }
    if(engine->osd_rectangle_need_transfer_data_flag & (1<<SUBTILTE2_RECTANGLE_ID)){
        engine->op->get_rectangle(engine, &rectangle, CHAR_RECTANGLE_ENCODE_OSD, SUBTILTE2_RECTANGLE_ID);
        if(rectangle != NULL){
            osd_dpram_req = &rectangle->osd_dpram_req;
            osd_dpram_req->op->submit_write_osd_rectangle_data_req(osd_dpram_req, chip);                
        }
    }
    if(engine->osd_rectangle_need_transfer_data_flag & (1<<SUBTILTE3_RECTANGLE_ID)){
        engine->op->get_rectangle(engine, &rectangle, CHAR_RECTANGLE_ENCODE_OSD, SUBTILTE3_RECTANGLE_ID);
        if(rectangle != NULL){
            osd_dpram_req = &rectangle->osd_dpram_req;
            osd_dpram_req->op->submit_write_osd_rectangle_data_req(osd_dpram_req, chip);                
        }
    }
    if(engine->osd_rectangle_need_transfer_data_flag & (1<<SUBTILTE4_RECTANGLE_ID)){
        engine->op->get_rectangle(engine, &rectangle, CHAR_RECTANGLE_ENCODE_OSD, SUBTILTE4_RECTANGLE_ID);
        if(rectangle != NULL){
            osd_dpram_req = &rectangle->osd_dpram_req;
            osd_dpram_req->op->submit_write_osd_rectangle_data_req(osd_dpram_req, chip);                
        }
    }
    spin_unlock_irqrestore(&engine->osd_lock, flags);
}

static void osd_chan_engine_submit_osd_attribute(osd_chan_engine_t *engine)
{
    if((engine->osd_rectangle_need_transfer_data_flag & OSD_ATTR_MASK) == OSD_ATTR_MASK){
        engine->osd_rectangle_need_transfer_data_flag &= ~(1<<ATTRIBUT_DONE_ID);
        engine->op->update_fpga_osd_encode(engine);
    }
}

static struct osd_chan_engine_operation	osd_chan_engine_op = 
{
    .create_char_rectangle = osd_chan_engine_create_char_rectangle,
    .create_mask_rectangle = osd_chan_engine_create_mask_rectangle,
#ifdef	TESTING_TIMESTAMP
    .create_timer_rectangle = osd_chan_engine_create_timer_rectangle_testing_timestamp,
    .create_rectangle = osd_chan_engine_create_rectangle_testing_timestamp,
#else
    .create_timer_rectangle = osd_chan_engine_create_timer_rectangle,
    .create_rectangle = osd_chan_engine_create_rectangle,
#endif

    .release_char_rectangle = osd_chan_engine_release_char_rectangle,
    .release_mask_rectangle = osd_chan_engine_release_mask_rectangle,
    .release_timer_rectangle = osd_chan_engine_release_timer_rectangle,
    .release_rectangle = osd_chan_engine_release_rectangle,

    .close_chan_engine_rectangles = osd_chan_engine_close_rectangles,
    .update_encode_osd_property = osd_chan_engine_update_encode_osd_property,

    .update_fpga_osd_encode = osd_chan_engine_update_fpga_osd_encode,
    .get_rectangle = osd_chan_engine_get_rectangle,
    .put_rectangle = osd_chan_engine_put_rectangle,

    .delete_chan_engine_rectangles = osd_chan_engine_delete_chan_engine_rectangles,
    .reset_chan_engine = osd_chan_engine_reset_chan_engine,

    .submit_osd_data = osd_chan_engine_submit_osd_data,
    .submit_osd_attribute = osd_chan_engine_submit_osd_attribute,
};


void    close_encode_osd_chan_engine(osd_chan_engine_t *engine)
{
    ISIL_DBG(ISIL_DBG_ERR, "chan_%d, %d, %d, %x\n", engine->encode_chan->logic_chan_id, atomic_read(&engine->osd_encode_start_close), atomic_read(&engine->osd_working), engine->osd_rectangle_need_transfer_data_flag);
    wait_for_completion(&engine->osd_transfering_done);
    engine->op->close_chan_engine_rectangles(engine);
    complete_all(&engine->osd_transfering_done);
}

void    remove_encode_osd_chan_engine(osd_chan_engine_t *engine)
{
    close_encode_osd_chan_engine(engine);
    remove_osd_encode_mb_pool(&engine->osd_chan_pool);
}

void    osd_testing_init(osd_chan_engine_t *engine)
{
    ISIL_OSD_PARAM    *osd_config_param, config_param;

    osd_config_param = &config_param;
    memset((char*)osd_config_param, 0, sizeof(ISIL_OSD_PARAM));

#if 1
    sprintf(osd_config_param->name, "Video chan name");
    printk("%s, len=%d\n", osd_config_param->name, strlen(osd_config_param->name));
    osd_config_param->name_attrib = OSD_ATTR_DISPLAY_ON;
    osd_config_param->name_pos_x = 50;
    osd_config_param->name_pos_y = 50;
#endif
    osd_config_param->time_attrib = OSD_ATTR_DISPLAY_ON;
    osd_config_param->time_pos_x = 0;
    osd_config_param->time_pos_y = 0;
#if 1
    osd_config_param->shelter1_attrib = OSD_ATTR_DISPLAY_ON;
    osd_config_param->shelter1_pos_x = 0;
    osd_config_param->shelter1_pos_y = 50;
    osd_config_param->shelter1_width = 20;
    osd_config_param->shelter1_height = 20;
 
    osd_config_param->shelter2_attrib = OSD_ATTR_DISPLAY_ON;
    osd_config_param->shelter2_pos_x = 0;
    osd_config_param->shelter2_pos_y = 100;
    osd_config_param->shelter2_width = 50;
    osd_config_param->shelter2_height = 50;
    sprintf(osd_config_param->subtitle1, "subtitle1 window test");
    printk("%s, len=%d\n", osd_config_param->subtitle1, strlen(osd_config_param->subtitle1));
    osd_config_param->subtitle1_attrib = OSD_ATTR_DISPLAY_ON;
    osd_config_param->subtitle1_pos_x = 100;
    osd_config_param->subtitle1_pos_y = 100;
    sprintf(osd_config_param->subtitle2, "subtitle2 window test");
    printk("%s, len=%d\n", osd_config_param->subtitle2, strlen(osd_config_param->subtitle2));
    osd_config_param->subtitle2_attrib = OSD_ATTR_DISPLAY_ON;
    osd_config_param->subtitle2_pos_x = 150;
    osd_config_param->subtitle2_pos_y = 150;
    sprintf(osd_config_param->subtitle3, "subtitle3 window test");
    printk("%s, len=%d\n", osd_config_param->subtitle3, strlen(osd_config_param->subtitle3));
    osd_config_param->subtitle3_attrib = OSD_ATTR_DISPLAY_ON;
    osd_config_param->subtitle3_pos_x = 200;
    osd_config_param->subtitle3_pos_y = 200;
    sprintf(osd_config_param->subtitle4, "subtitle4 window test");
    printk("%s, len=%d\n", osd_config_param->subtitle4, strlen(osd_config_param->subtitle4));
    osd_config_param->subtitle4_attrib = OSD_ATTR_DISPLAY_ON;
    osd_config_param->subtitle4_pos_x = 0;
    osd_config_param->subtitle4_pos_y = 240;
#endif
    engine->op->update_encode_osd_property(engine, osd_config_param);
}

int init_encode_osd_chan_engine(osd_chan_engine_t *engine, int masterOrSub)
{
    isil_h264_logic_encode_chan_t   *encode_chan;
    ISIL_OSD_PARAM  *osd_running_param;
    isil_chip_t     *chip;
    chip_driver_t   *chip_driver;
    isil_video_bus_t    *video_bus;
    isil_vd_cross_bus_t *vd_bus;
    isil_h264_phy_video_slot_t  *phy_video_slot;
    DVMNVS_CHIP_VIDEO_ENCODE_CAPABILITY *video_encode_cap;
    osd_rectangle_entry_t       *rectangle;
    osd_rectangle_dpram_tcb_t   *osd_dpram_req;
    encode_osd_string_attr_t    *string_attr;
    encode_osd_mask_attr_t      *mask_attr;
    int	i, phy_slot_id, osd_slot_id, ret = -1;

    encode_chan = to_get_isil_h264_encode_chan_with_osd_engine(engine);
    phy_slot_id = encode_chan->phy_slot_id;
    chip = encode_chan->chip;
    osd_running_param = &engine->osd_running_param;

    chip_driver = chip->chip_driver;
    video_bus = &chip_driver->video_bus;
    vd_bus = video_bus->vd_bus_driver;
    video_encode_cap = chip->video_encode_cap;
    spin_lock_init(&engine->osd_lock);
    engine->encode_chan = encode_chan;
    memset((char*)osd_running_param, 0, sizeof(ISIL_OSD_PARAM));
    init_completion(&engine->osd_transfering_done);
    complete_all(&engine->osd_transfering_done);

    engine->chan_rectangle_used_map_table = 0;
    engine->chan_data_base = get_osd_chan_base(phy_slot_id);
    engine->masterOrSub = masterOrSub;

    vd_bus->op->get_phy_video_slot_by_phy_id(vd_bus, phy_slot_id, &phy_video_slot);
    osd_slot_id = get_phy_slot_id_by_osd_chan_base(vd_bus, engine->chan_data_base, phy_slot_id, video_encode_cap->ddr_map_mode);
    vd_bus->op->get_phy_video_slot_by_phy_id(vd_bus, osd_slot_id, &phy_video_slot);
    engine->videoSizeMode = phy_video_slot->video_size;
    ret = init_osd_encode_mb_pool(engine, chip->chip_id);
    if(ret < 0){
        return ret;
    }

    engine->op = &osd_chan_engine_op;
    engine->attribute_regs.chan_rect_attr_regs_group_op = &osd_attribute_regs_group_op;
    engine->attribute_regs.chan_rect_attr_regs_group_op->init_osd_chan_rectangle_attr_regs_group(&engine->attribute_regs);
    for(i=0; i<OSD_CHAN_RECTANGLE_NUMBER; i++){
        rectangle = &engine->rectangle[i];
        rectangle->chip = chip;
        rectangle->osd_chan_engine = engine;
        rectangle->mb_char_queue_node.op = &tcb_node_queue_op;
        rectangle->mb_char_queue_node.op->init(&rectangle->mb_char_queue_node);
        rectangle->mb24_char_queue_node.op = &tcb_node_queue_op;
        rectangle->mb24_char_queue_node.op->init(&rectangle->mb24_char_queue_node);
        rectangle->rectangle_id = i;
        rectangle->chan_id = phy_slot_id;
        rectangle->attr_regs = &engine->attribute_regs.attribute_regs_group[i];
        rectangle->op = &osd_rectangle_op;
        osd_dpram_req = &rectangle->osd_dpram_req;
        osd_dpram_req->op = &osd_rectangle_dpram_tcb_op;
        rectangle->op->reset(rectangle);
    }

    get_chip_osd_timer(&engine->timer_attr.timer, chip);
    engine->timer_attr.op = &osd_timer_attr_op;
    engine->timer_attr.mb_x = 0;
    engine->timer_attr.mb_y = 0;
    engine->timer_attr.timer_display_mode = YEAR_MONTH_DAY_HOUR_MINUTE_SECOND;
    engine->timer_attr.op->reset(&engine->timer_attr, chip->chip_id, encode_chan->logic_chan_id);
    string_attr = engine->string_attr;
    mask_attr = engine->mask_attr;
    for(i=0; i<OSD_CHAN_MAX_STRING_RECTANGLE_NUMBER; i++){
        string_attr->op = &osd_string_attr_op;
        mask_attr->op = &osd_mask_attr_op;

        string_attr->op->reset(string_attr);
        mask_attr->op->reset(mask_attr);
        string_attr++;
        mask_attr++;
    }
    engine->op->reset_chan_engine(engine);

    //osd_testing_init(engine);
    return ret;
}


