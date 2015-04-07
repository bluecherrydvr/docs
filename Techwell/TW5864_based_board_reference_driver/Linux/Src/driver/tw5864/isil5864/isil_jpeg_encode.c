/*
file:isil_jpeg_codec.c
desc:jpeg codec function
create by rxchen <rxchen@techwellinc.com.cn>
*/
#include	<isil5864/isil_common.h>

static isil_mjpeg_encode_param_t default_jpeg_encode_property = {
    .i_image_width_mb_size   = (WIDTH_FRAME_D1_PAL>>4),
    .i_image_height_mb_size  = (HEIGHT_FRAME_D1_PAL>>4),
    .e_image_level           = MJPEG_IMAGE_LEVEL_0,
    .i_capture_frame_stride  = ISIL_MJPEG_MIN_INTERVAL,
    .i_capture_frame_number  = 0xffffffff,
    .i_capture_type          = ISIL_MJPEG_ENCODE_PARAM_CAPTURE_TYPE_TIMER,
    //.i_capture_type          = 0,
    .change_mask_flag        = 0xff,
};

static const char base_q_tbl_y[64] =
{
    8,  6,  6,  7,  6,  5,  8,  7,
    7,  7,  9,  9,  8, 10, 12, 20,
    13, 12, 11, 11, 12, 25, 18, 19,
    15, 20, 29, 26, 31, 30, 29, 26,
    28, 28, 32, 36, 46, 39, 32, 34,
    44, 35, 28, 28, 40, 55, 41, 44,
    48, 49, 52, 52, 52, 31, 39, 57,
    61, 56, 50, 60, 46, 51, 52, 50
};

static const char base_q_tbl_uv[64] =
{
    9,  9,  9, 12, 11, 12, 24, 13,
    13, 24, 50, 33, 28, 33, 50, 50,
    50, 50, 50, 50, 50, 50, 50, 50,
    50, 50, 50, 50, 50, 50, 50, 50,
    50, 50, 50, 50, 50, 50, 50, 50,
    50, 50, 50, 50, 50, 50, 50, 50,
    50, 50, 50, 50, 50, 50, 50, 50,
    50, 50, 50, 50, 50, 50, 50, 50
};

/*DC ID 0x00*/
static const char huffman_DC0_tbl[] = {
    0xFF,0xC4,0x00,0x1F,0x00,0x00,0x01,0x05,
    0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,
    0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,
    0x0B,
};

/*DC ID 0x01*/
static const char huffman_DC1_tbl[] = {
    0xFF,0xC4,0x00,0x1F,0x01,0x00,0x03,0x01,
    0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
    0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,
    0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,
    0x0B,
};

/*AC ID 0x00*/
static const char huffman_AC0_tbl[] = {
    0xFF,0xC4,0x00,0xB5,0x10,0x00,0x02,0x01,
    0x03,0x03,0x02,0x04,0x03,0x05,0x05,0x04,
    0x04,0x00,0x00,0x01,0x7D,0x01,0x02,0x03,
    0x00,0x04,0x11,0x05,0x12,0x21,0x31,0x41,
    0x06,0x13,0x51,0x61,0x07,0x22,0x71,0x14,
    0x32,0x81,0x91,0xA1,0x08,0x23,0x42,0xB1,
    0xC1,0x15,0x52,0xD1,0xF0,0x24,0x33,0x62,
    0x72,0x82,0x09,0x0A,0x16,0x17,0x18,0x19,
    0x1A,0x25,0x26,0x27,0x28,0x29,0x2A,0x34,
    0x35,0x36,0x37,0x38,0x39,0x3A,0x43,0x44,
    0x45,0x46,0x47,0x48,0x49,0x4A,0x53,0x54,
    0x55,0x56,0x57,0x58,0x59,0x5A,0x63,0x64,
    0x65,0x66,0x67,0x68,0x69,0x6A,0x73,0x74,
    0x75,0x76,0x77,0x78,0x79,0x7A,0x83,0x84,
    0x85,0x86,0x87,0x88,0x89,0x8A,0x92,0x93,
    0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0xA2,
    0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,
    0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,
    0xBA,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,
    0xC9,0xCA,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,
    0xD8,0xD9,0xDA,0xE1,0xE2,0xE3,0xE4,0xE5,
    0xE6,0xE7,0xE8,0xE9,0xEA,0xF1,0xF2,0xF3,
    0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,
};
/*AC ID 0x01*/
static const char huffman_AC1_tbl[] = {
    0xFF,0xC4,0x00,0xB5,0x11,0x00,0x02,0x01,
    0x02,0x04,0x04,0x03,0x04,0x07,0x05,0x04,
    0x04,0x00,0x01,0x02,0x77,0x00,0x01,0x02,
    0x03,0x11,0x04,0x05,0x21,0x31,0x06,0x12,
    0x41,0x51,0x07,0x61,0x71,0x13,0x22,0x32,
    0x81,0x08,0x14,0x42,0x91,0xA1,0xB1,0xC1,
    0x09,0x23,0x33,0x52,0xF0,0x15,0x62,0x72,
    0xD1,0x0A,0x16,0x24,0x34,0xE1,0x25,0xF1,
    0x17,0x18,0x19,0x1A,0x26,0x27,0x28,0x29,
    0x2A,0x35,0x36,0x37,0x38,0x39,0x3A,0x43,
    0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x53,
    0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x63,
    0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x73,
    0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x82,
    0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,
    0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,
    0x9A,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,
    0xA9,0xAA,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,
    0xB8,0xB9,0xBA,0xC2,0xC3,0xC4,0xC5,0xC6,
    0xC7,0xC8,0xC9,0xCA,0xD2,0xD3,0xD4,0xD5,
    0xD6,0xD7,0xD8,0xD9,0xDA,0xE2,0xE3,0xE4,
    0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xF2,0xF3,
    0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,
};

static const char sos_table_default[] = {
    0xFF,0xDA,0x00,0x0C,0x03,0x01,0x00,0x02,
    0x11,0x03,0x11,0x00,0x3F,0x00,
};

static int isil_jpeg_schedule_restart(void *context);
static void init_jpeg_service_tcb_with_start_capture(jpeg_service_tcb_t *encode_request_tcb);
static void init_jpeg_service_tcb_with_start_encode(jpeg_service_tcb_t *encode_request_tcb);
static int init_vj_resource_queue(isil_vj_resource_queue_t *queue);
int isil_jpeg_to_file(struct file *jpeg_file, isil_vb_frame_tcb_t  *frame);
int isil_jpeg_save_chn(isil_jpeg_logic_encode_chan_t *logic_chnl, char *path, int cnt);

static int  jpeg_encode_driver_in_unregister_state_recv_close_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        ed_tcb_t    *ed_tcb;
        isil_jpeg_logic_encode_chan_t   *jpeg_logic_encode_chan;
        robust_processing_control_t *robust_processing_control;

        jpeg_logic_encode_chan = (isil_jpeg_logic_encode_chan_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", jpeg_logic_encode_chan->phy_slot_id, jpeg_logic_encode_chan->logic_chan_id, atomic_read(&jpeg_logic_encode_chan->opened_flag));
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        if(atomic_read(&jpeg_logic_encode_chan->opened_flag) == 0){
            ISIL_DBG(ISIL_DBG_ERR, "channel %d already closed\n", jpeg_logic_encode_chan->phy_slot_id);
            ed_tcb->ed.op->all_complete_done(&ed_tcb->ed);
            return ISIL_ERR;
        }
        robust_processing_control = &jpeg_logic_encode_chan->robust_processing_control;
        if(robust_processing_control->op->is_in_robust_processing(robust_processing_control)){
            ed_fsm->op->save_event_for_robust_processing(ed_fsm);
        }
        if(atomic_dec_and_test(&jpeg_logic_encode_chan->opened_flag)){
            ed_fsm->op->reset(ed_fsm);
            ed_tcb->ed.op->all_complete_done(&ed_tcb->ed);
        }
    }
    return ISIL_OK;
}

static int jpeg_encode_driver_in_unregister_state_recv_suspend_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        ed_tcb_t    *ed_tcb;
	isil_register_node_t    *ed;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        ed = &ed_tcb->ed;
        ed->op->suspend_complete_done(ed);
        
        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int jpeg_encode_driver_in_unregister_state_recv_resume_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        ed_tcb_t    *ed_tcb;
	isil_register_node_t    *ed;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        ed = &ed_tcb->ed;
        ed->op->resume_complete_done(ed);
        
        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int  jpeg_encode_driver_in_unregister_state_recv_open_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        ed_tcb_t    *ed_tcb;
        isil_jpeg_logic_encode_chan_t   *jpeg_logic_encode_chan;
        robust_processing_control_t *robust_processing_control;

        jpeg_logic_encode_chan = (isil_jpeg_logic_encode_chan_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", jpeg_logic_encode_chan->phy_slot_id, jpeg_logic_encode_chan->logic_chan_id, atomic_read(&jpeg_logic_encode_chan->opened_flag));
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        if(atomic_read(&jpeg_logic_encode_chan->opened_flag) == 0){
            atomic_set(&jpeg_logic_encode_chan->opened_flag, 1);
            ed_tcb->ed.op->init_complete(&ed_tcb->ed);
            ed_fsm->op->change_state(ed_fsm, ISIL_ED_SUSPEND);
            robust_processing_control = &jpeg_logic_encode_chan->robust_processing_control;
            if(robust_processing_control->op->is_in_robust_processing(robust_processing_control)){
                ed_fsm->op->restore_event_from_robust_processing(ed_fsm);
                if(have_any_pending_event(ed_tcb)){
                    driver_trigger_pending_event(ed_tcb);
                }
            }
        }else{
            atomic_inc(&jpeg_logic_encode_chan->opened_flag);
        }
    }
    return ISIL_OK;
}

static int  jpeg_encode_driver_in_idle_state_recv_close_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_jpeg_logic_encode_chan_t   *jpeg_logic_encode_chan;
        isil_jpeg_encode_control_t      *encode_control;
        ed_tcb_t    *ed_tcb;
	isil_chip_t	*chip;
        isil_chip_timer_controller_t    *chip_timer_cont;
 
        ed_fsm->op->change_state(ed_fsm, ISIL_ED_UNREGISTER);
        jpeg_logic_encode_chan = (isil_jpeg_logic_encode_chan_t*)context;
        chip = jpeg_logic_encode_chan->chip;
        chip_timer_cont = &chip->chip_timer_cont;
        encode_control = &jpeg_logic_encode_chan->encode_control;
        if(encode_control->schedule_time != INVALIDTIMERID){
            ISIL_DBG(ISIL_DBG_DEBUG, "delete schedule timer\n");
            chip_timer_cont->op->DeleteSingleFireTimerJob(chip_timer_cont, encode_control->schedule_time);
            encode_control->schedule_time = INVALIDTIMERID;
        }

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_close_event(ed_tcb, 1);
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", jpeg_logic_encode_chan->phy_slot_id, jpeg_logic_encode_chan->logic_chan_id, atomic_read(&jpeg_logic_encode_chan->opened_flag));
    }
    return ISIL_OK;
}

static int  jpeg_encode_driver_in_idle_state_recv_timeout_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    return ISIL_OK;
}

static int  jpeg_encode_driver_in_idle_state_recv_suspend_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_jpeg_logic_encode_chan_t   *jpeg_logic_encode_chan;
        isil_jpeg_encode_control_t      *encode_control;
        ed_tcb_t    *ed_tcb;
	isil_chip_t	*chip;
        isil_chip_timer_controller_t    *chip_timer_cont;
 
        ed_fsm->op->change_state(ed_fsm, ISIL_ED_SUSPEND);
        jpeg_logic_encode_chan = (isil_jpeg_logic_encode_chan_t*)context;
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        chip = jpeg_logic_encode_chan->chip;
        chip_timer_cont = &chip->chip_timer_cont;
        encode_control = &jpeg_logic_encode_chan->encode_control;
        if(encode_control->schedule_time != INVALIDTIMERID){
            ISIL_DBG(ISIL_DBG_DEBUG, "delete schedule timer\n");
            chip_timer_cont->op->DeleteSingleFireTimerJob(chip_timer_cont, encode_control->schedule_time);
            encode_control->schedule_time = INVALIDTIMERID;
        }
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", jpeg_logic_encode_chan->phy_slot_id, jpeg_logic_encode_chan->logic_chan_id, atomic_read(&jpeg_logic_encode_chan->opened_flag));
        
        ed_tcb->ed.op->suspend_complete_done(&ed_tcb->ed);
    }
    return ISIL_OK;
}

static int jpeg_encode_driver_in_idle_state_recv_resume_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        ed_tcb_t    *ed_tcb;
	isil_register_node_t    *ed;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        ed = &ed_tcb->ed;
        ed->op->resume_complete_done(ed);
        
        return ISIL_OK;
    }

    return ISIL_ERR;
}


static int  jpeg_encode_driver_in_idle_state_recv_deliver_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        //isil_jpeg_logic_encode_chan_t *jpeg_logic_encode_chan = (isil_jpeg_logic_encode_chan_t*)context;
        ed_fsm->op->change_state(ed_fsm, ISIL_ED_STANDBY);
    }
    return ISIL_OK;
}

static int  jpeg_encode_driver_in_standby_state_recv_close_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_jpeg_logic_encode_chan_t   *jpeg_logic_encode_chan;
        isil_chip_t *chip;
	isil_vj_bus_t         *chip_vj_bus;
	jpeg_service_queue_t        *jpeg_encode_service_queue;
	tcb_node_t	*service_tcb;
	jpeg_service_tcb_t    *encode_request_tcb;
        ed_tcb_t    *ed_tcb;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        jpeg_logic_encode_chan = (isil_jpeg_logic_encode_chan_t*)context;
        chip = jpeg_logic_encode_chan->chip;
        chip_vj_bus = &chip->chip_vj_bus;
		encode_request_tcb = &jpeg_logic_encode_chan->encode_request_tcb;
        service_tcb = &encode_request_tcb->service_tcb;
        service_tcb->op->get_priv(service_tcb, (void *)&jpeg_encode_service_queue);
        if(jpeg_encode_service_queue && jpeg_encode_service_queue->op->delete_pending_service_request(jpeg_encode_service_queue, &jpeg_logic_encode_chan->encode_request_tcb)){
            ed_fsm->op->change_state(ed_fsm, ISIL_ED_UNREGISTER);
            driver_gen_close_event(ed_tcb, 1);
        } else {
            driver_gen_close_event(ed_tcb, 0);
            ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", jpeg_logic_encode_chan->phy_slot_id, jpeg_logic_encode_chan->logic_chan_id, atomic_read(&jpeg_logic_encode_chan->opened_flag));
        }
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", jpeg_logic_encode_chan->phy_slot_id, jpeg_logic_encode_chan->logic_chan_id, atomic_read(&jpeg_logic_encode_chan->opened_flag));
    }
    return ISIL_OK;
}

static int  jpeg_encode_driver_in_standby_state_recv_timeout_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    return ISIL_OK;
}

static int  jpeg_encode_driver_in_standby_state_recv_suspend_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_jpeg_logic_encode_chan_t   *jpeg_logic_encode_chan;
        isil_chip_t *chip;
        ed_tcb_t    *ed_tcb;
	isil_vj_bus_t         *chip_vj_bus;
	jpeg_service_queue_t        *jpeg_encode_service_queue;
	tcb_node_t	*service_tcb;
	jpeg_service_tcb_t    *encode_request_tcb;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        jpeg_logic_encode_chan = (isil_jpeg_logic_encode_chan_t*)context;
        chip = jpeg_logic_encode_chan->chip;
        chip_vj_bus = &chip->chip_vj_bus;
		encode_request_tcb = &jpeg_logic_encode_chan->encode_request_tcb;
        service_tcb = &encode_request_tcb->service_tcb;
        service_tcb->op->get_priv(service_tcb, (void *)&jpeg_encode_service_queue);
        if(jpeg_encode_service_queue && jpeg_encode_service_queue->op->delete_pending_service_request(jpeg_encode_service_queue, encode_request_tcb)){
            ed_fsm->op->change_state(ed_fsm, ISIL_ED_SUSPEND);
            ed_tcb->ed.op->suspend_complete_done(&ed_tcb->ed);
        } else {
            ISIL_DBG(ISIL_DBG_ERR, "I'm at standby but not in service queue, curr_consumer = %p, self = %p\n", jpeg_encode_service_queue->curr_consumer, encode_request_tcb);
            driver_gen_suspend_event(ed_tcb, 0);
            ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", jpeg_logic_encode_chan->phy_slot_id, jpeg_logic_encode_chan->logic_chan_id, atomic_read(&jpeg_logic_encode_chan->opened_flag));
        }
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", jpeg_logic_encode_chan->phy_slot_id, jpeg_logic_encode_chan->logic_chan_id, atomic_read(&jpeg_logic_encode_chan->opened_flag));
    }
    return ISIL_OK;
}

static int jpeg_encode_driver_in_standby_state_recv_resume_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        ed_tcb_t    *ed_tcb;
	isil_register_node_t    *ed;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        ed = &ed_tcb->ed;
        ed->op->resume_complete_done(ed);
        
        return ISIL_OK;
    }

    return ISIL_ERR;
}


static int  jpeg_encode_driver_in_standby_state_recv_deliver_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        ed_fsm->op->change_state(ed_fsm, ISIL_ED_RUNNING);
    }
    return ISIL_OK;
}

static int  jpeg_encode_driver_in_suspend_state_recv_close_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_jpeg_logic_encode_chan_t   *jpeg_logic_encode_chan;
        ed_tcb_t    *ed_tcb;

        ed_fsm->op->change_state(ed_fsm, ISIL_ED_UNREGISTER);
        jpeg_logic_encode_chan = (isil_jpeg_logic_encode_chan_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", jpeg_logic_encode_chan->phy_slot_id, jpeg_logic_encode_chan->logic_chan_id, atomic_read(&jpeg_logic_encode_chan->opened_flag));
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_close_event(ed_tcb, 1);
    }
    return ISIL_OK;
}

static int  jpeg_encode_driver_in_suspend_state_recv_timeout_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    return ISIL_OK;
}

static int  jpeg_encode_driver_in_suspend_state_recv_resume_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_jpeg_logic_encode_chan_t   *jpeg_logic_encode_chan;
        isil_jpeg_encode_control_t      *encode_control;
		isil_vb_frame_tcb_queue_t	*jpeg_frame_queue;
        ed_tcb_t    *ed_tcb;

        jpeg_logic_encode_chan = (isil_jpeg_logic_encode_chan_t*)context;
        encode_control = &jpeg_logic_encode_chan->encode_control;
        jpeg_frame_queue = &jpeg_logic_encode_chan->jpeg_frame_queue;
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);

        jpeg_frame_queue->op->try_get_curr_producer_from_pool(jpeg_frame_queue, &jpeg_logic_encode_chan->pool);
        if(!jpeg_frame_queue->curr_producer){
            ISIL_DBG(ISIL_DBG_ERR, "can't get frame!\n");
        }

        ed_tcb->ed.op->resume_complete_done(&ed_tcb->ed);
        ed_fsm->op->change_state(ed_fsm, ISIL_ED_IDLE);
        encode_control->op->first_start(encode_control);
    }
    return ISIL_OK;
}

static int  jpeg_encode_driver_in_running_state_recv_close_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_jpeg_logic_encode_chan_t   *jpeg_logic_encode_chan;
        ed_tcb_t    *ed_tcb;

        jpeg_logic_encode_chan = (isil_jpeg_logic_encode_chan_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", jpeg_logic_encode_chan->phy_slot_id, jpeg_logic_encode_chan->logic_chan_id, atomic_read(&jpeg_logic_encode_chan->opened_flag));
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_close_event(ed_tcb, 0);
 
        return ISIL_OK;
    }
    return ISIL_OK;
}

static int  jpeg_encode_driver_in_running_state_recv_timeout_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_jpeg_logic_encode_chan_t   *jpeg_logic_encode_chan;
        isil_jpeg_encode_control_t *jpeg_encode_control;
        ed_tcb_t    *ed_tcb;
        isil_chip_t *chip;
        chip_driver_t *chip_driver;
        isil_video_bus_t *video_bus;
        isil_vj_bus_t *vj_bus;
        robust_processing_control_t *robust_processing_control;

        jpeg_logic_encode_chan = (isil_jpeg_logic_encode_chan_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", jpeg_logic_encode_chan->phy_slot_id, jpeg_logic_encode_chan->logic_chan_id, atomic_read(&jpeg_logic_encode_chan->opened_flag));
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);

        jpeg_encode_control = &jpeg_logic_encode_chan->encode_control;
        chip = jpeg_logic_encode_chan->chip;
        chip_driver = chip->chip_driver;
        video_bus = &chip_driver->video_bus;
        vj_bus = video_bus->vj_bus_driver;


        robust_processing_control = &chip->chip_robust_processing;
        if(!robust_processing_control->op->is_in_robust_processing(robust_processing_control)){
            robust_processing_control->op->start_robust_processing(robust_processing_control);
            robust_processing_control = &vj_bus->robust_processing_control;
            if(!robust_processing_control->op->is_in_robust_processing(robust_processing_control)){
                robust_processing_control->op->start_robust_processing(robust_processing_control);
                robust_processing_control = &jpeg_logic_encode_chan->robust_processing_control;
                robust_processing_control->op->start_robust_processing(robust_processing_control);
                jpeg_encode_chan_gen_req_msg(jpeg_logic_encode_chan, REQ_ALGO_CHIP_RESET, NONBLOCK_OP);
            }
        }
    }

    return ISIL_OK;
}

static int  jpeg_encode_driver_in_running_state_recv_suspend_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_jpeg_logic_encode_chan_t   *jpeg_logic_encode_chan;
        ed_tcb_t    *ed_tcb;

        jpeg_logic_encode_chan = (isil_jpeg_logic_encode_chan_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", jpeg_logic_encode_chan->phy_slot_id, jpeg_logic_encode_chan->logic_chan_id, atomic_read(&jpeg_logic_encode_chan->opened_flag));
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_suspend_event(ed_tcb, 0);
    }
    return ISIL_OK;
}

static int  jpeg_encode_driver_in_running_state_recv_resume_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_jpeg_logic_encode_chan_t   *jpeg_logic_encode_chan;
        ed_tcb_t    *ed_tcb;

        jpeg_logic_encode_chan = (isil_jpeg_logic_encode_chan_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", jpeg_logic_encode_chan->phy_slot_id, jpeg_logic_encode_chan->logic_chan_id, atomic_read(&jpeg_logic_encode_chan->opened_flag));
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_resume_event(ed_tcb, 0);
    }
    return ISIL_OK;
}

static int  jpeg_encode_driver_in_running_state_recv_deliver_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        ed_fsm->op->change_state(ed_fsm, ISIL_ED_TRANSFERING);
    }
    return ISIL_OK;
}

static int  jpeg_encode_driver_in_transfering_state_recv_close_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_jpeg_logic_encode_chan_t   *jpeg_logic_encode_chan;
        ed_tcb_t    *ed_tcb;

        jpeg_logic_encode_chan = (isil_jpeg_logic_encode_chan_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", jpeg_logic_encode_chan->phy_slot_id, jpeg_logic_encode_chan->logic_chan_id, atomic_read(&jpeg_logic_encode_chan->opened_flag));
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_close_event(ed_tcb, 0);
    }
    return ISIL_OK;
}

static int  jpeg_encode_driver_in_transfering_state_recv_timeout_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    return ISIL_OK;
}

static int  jpeg_encode_driver_in_transfering_state_recv_suspend_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_jpeg_logic_encode_chan_t   *jpeg_logic_encode_chan;
        ed_tcb_t    *ed_tcb;

        jpeg_logic_encode_chan = (isil_jpeg_logic_encode_chan_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", jpeg_logic_encode_chan->phy_slot_id, jpeg_logic_encode_chan->logic_chan_id, atomic_read(&jpeg_logic_encode_chan->opened_flag));
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_suspend_event(ed_tcb, 0);
    }
    return ISIL_OK;
}

static int  jpeg_encode_driver_in_transfering_state_recv_resume_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_jpeg_logic_encode_chan_t   *jpeg_logic_encode_chan;
        ed_tcb_t    *ed_tcb;

        jpeg_logic_encode_chan = (isil_jpeg_logic_encode_chan_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", jpeg_logic_encode_chan->phy_slot_id, jpeg_logic_encode_chan->logic_chan_id, atomic_read(&jpeg_logic_encode_chan->opened_flag));
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_resume_event(ed_tcb, 0);
    }
    return ISIL_OK;
}


static int  jpeg_encode_driver_in_transfering_state_recv_deliver_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        ed_tcb_t    *ed_tcb;

        ed_fsm->op->change_state(ed_fsm, ISIL_ED_DONE);
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        if(have_any_pending_event(ed_tcb)){
            ISIL_DBG(ISIL_DBG_DEBUG, "trigger pending ed_fsm = %p\n", ed_fsm);
            driver_trigger_pending_event(ed_tcb);
        } else {
            driver_gen_deliver_event(ed_tcb, 1);
        }
    }
    return ISIL_OK;
}

static int  jpeg_encode_driver_in_done_state_recv_close_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_jpeg_logic_encode_chan_t   *jpeg_logic_encode_chan;
        ed_tcb_t    *ed_tcb;

        ed_fsm->op->change_state(ed_fsm, ISIL_ED_UNREGISTER);
        jpeg_logic_encode_chan = (isil_jpeg_logic_encode_chan_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", jpeg_logic_encode_chan->phy_slot_id, jpeg_logic_encode_chan->logic_chan_id, atomic_read(&jpeg_logic_encode_chan->opened_flag));
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_close_event(ed_tcb, 1);
    }
    return ISIL_OK;
}

static int  jpeg_encode_driver_in_done_state_recv_timeout_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    return ISIL_OK;
}

static int  jpeg_encode_driver_in_done_state_recv_suspend_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_jpeg_logic_encode_chan_t   *jpeg_logic_encode_chan;
        ed_tcb_t    *ed_tcb;

        ed_fsm->op->change_state(ed_fsm, ISIL_ED_SUSPEND);
        jpeg_logic_encode_chan = (isil_jpeg_logic_encode_chan_t*)context;
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);

        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", jpeg_logic_encode_chan->phy_slot_id, jpeg_logic_encode_chan->logic_chan_id, atomic_read(&jpeg_logic_encode_chan->opened_flag));

        ed_tcb->ed.op->suspend_complete_done(&ed_tcb->ed);
    }
    return ISIL_OK;
}

static int jpeg_encode_driver_in_done_state_recv_resume_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        ed_tcb_t    *ed_tcb;
	isil_register_node_t    *ed;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        ed = &ed_tcb->ed;
        ed->op->resume_complete_done(ed);
        driver_gen_deliver_event(ed_tcb, 1);
        
        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int  jpeg_encode_driver_in_done_state_recv_deliver_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        ed_fsm->op->change_state(ed_fsm, ISIL_ED_IDLE);
    }
    return ISIL_OK;
}

static int  jpeg_encode_driver_sync_hook(isil_ed_fsm_t *ed_fsm, void *context)
{
    
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_jpeg_logic_encode_chan_t *jpeg_logic_encode_chan;
		isil_jpeg_encode_property_t   *encode_property;
		isil_mjpeg_encode_param_t     *config_encode_property;
		isil_mjpeg_encode_param_t     *running_encode_property;

        jpeg_logic_encode_chan = (isil_jpeg_logic_encode_chan_t*)context;
        encode_property = &jpeg_logic_encode_chan->encode_property;
        config_encode_property = &encode_property->encode_property;
        running_encode_property = &encode_property->running_encode_property;

        running_encode_property->change_mask_flag = config_encode_property->change_mask_flag;
        if(running_encode_property->change_mask_flag & ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_IMAGE_LEVEL_MASK){
            running_encode_property->e_image_level = config_encode_property->e_image_level;
        }

        if((running_encode_property->change_mask_flag & ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_IMAGE_WIDTH_MASK) ||
            (running_encode_property->change_mask_flag & ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_IMAGE_HEIGHT_MASK)){
            isil_video_bus_t *video_bus;
            enum ISIL_VIDEO_SIZE video_size;
            enum ISIL_VIDEO_STANDARD video_standard;

            if(config_encode_property->i_image_width_mb_size == (WIDTH_FRAME_4CIF_PAL >> 4)){
                running_encode_property->i_image_width_mb_size = (WIDTH_FRAME_D1_PAL >> 4);
            }else{
                running_encode_property->i_image_width_mb_size = config_encode_property->i_image_width_mb_size;
            }
            running_encode_property->i_image_height_mb_size = config_encode_property->i_image_height_mb_size;
            /*adjust with current video norm*/
            video_bus = &jpeg_logic_encode_chan->chip->chip_driver->video_bus;
            video_standard = video_bus->op->get_video_standard(video_bus);
            video_size = VIDEO_SIZE_FROM_WIDTH_HEIGHT(running_encode_property->i_image_width_mb_size << 4, running_encode_property->i_image_height_mb_size << 4, video_standard);
            running_encode_property->i_image_width_mb_size = VIDEO_SIZE_TO_WIDTH(video_size, video_standard) >> 4;
            running_encode_property->i_image_height_mb_size = VIDEO_SIZE_TO_HEIGHT(video_size, video_standard) >> 4;
        }
        if(running_encode_property->change_mask_flag & ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_CAPTURE_FRAME_NUMBER_MASK){
            running_encode_property->i_capture_frame_number = config_encode_property->i_capture_frame_number;
        }
        if(running_encode_property->change_mask_flag & ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_CAPTURE_FRAME_STRIDE_MASK){
            running_encode_property->i_capture_frame_stride = config_encode_property->i_capture_frame_stride;
        }
        if(running_encode_property->change_mask_flag & ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_CAPTURE_TYPE_MASK){
            running_encode_property->i_capture_type = config_encode_property->i_capture_type;
        }
        running_encode_property->change_mask_flag = config_encode_property->change_mask_flag = 0;
    }
    return ISIL_OK;
}

static int  jpeg_encode_driver_sync_running_to_config_hook(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_jpeg_logic_encode_chan_t *jpeg_logic_encode_chan;
        isil_jpeg_encode_property_t   *encode_property;
        isil_mjpeg_encode_param_t     *config_encode_property;
        isil_mjpeg_encode_param_t     *running_encode_property;

        jpeg_logic_encode_chan = (isil_jpeg_logic_encode_chan_t*)context;
        encode_property = &jpeg_logic_encode_chan->encode_property;
        config_encode_property = &encode_property->encode_property;
        running_encode_property = &encode_property->running_encode_property;

        memcpy(config_encode_property, running_encode_property, sizeof(isil_mjpeg_encode_param_t));
        running_encode_property->change_mask_flag = config_encode_property->change_mask_flag = 0;

        return ISIL_OK;
    }
    
    return ISIL_ERR;
}
static fsm_state_transfer_matrix_table_t    jpeg_encode_driver_fsm_state_transfer_matrix_table = {
    .action = {
        {
            jpeg_encode_driver_in_unregister_state_recv_close_event, 
            no_op,
            jpeg_encode_driver_in_unregister_state_recv_suspend_event,
            jpeg_encode_driver_in_unregister_state_recv_resume_event,
            jpeg_encode_driver_in_unregister_state_recv_open_event,
            no_op
        },
        {
            jpeg_encode_driver_in_idle_state_recv_close_event, 
            jpeg_encode_driver_in_idle_state_recv_timeout_event,
            jpeg_encode_driver_in_idle_state_recv_suspend_event,
            jpeg_encode_driver_in_idle_state_recv_resume_event,
            no_op,
            jpeg_encode_driver_in_idle_state_recv_deliver_event
        },
        {
            jpeg_encode_driver_in_standby_state_recv_close_event, 
            jpeg_encode_driver_in_standby_state_recv_timeout_event,
            jpeg_encode_driver_in_standby_state_recv_suspend_event,
            jpeg_encode_driver_in_standby_state_recv_resume_event,
            no_op,
            jpeg_encode_driver_in_standby_state_recv_deliver_event
        },
        {
            jpeg_encode_driver_in_suspend_state_recv_close_event,
            jpeg_encode_driver_in_suspend_state_recv_timeout_event,
            no_op,
            jpeg_encode_driver_in_suspend_state_recv_resume_event,
            no_op,
            no_op
        },
        {
            jpeg_encode_driver_in_running_state_recv_close_event,
            jpeg_encode_driver_in_running_state_recv_timeout_event,
            jpeg_encode_driver_in_running_state_recv_suspend_event,
            jpeg_encode_driver_in_running_state_recv_resume_event,
            no_op,
            jpeg_encode_driver_in_running_state_recv_deliver_event
        },
        {
            jpeg_encode_driver_in_transfering_state_recv_close_event,
            jpeg_encode_driver_in_transfering_state_recv_timeout_event,
            jpeg_encode_driver_in_transfering_state_recv_suspend_event,
            jpeg_encode_driver_in_transfering_state_recv_resume_event,
            no_op,
            jpeg_encode_driver_in_transfering_state_recv_deliver_event
        },
        {
            jpeg_encode_driver_in_done_state_recv_close_event,
            jpeg_encode_driver_in_done_state_recv_timeout_event,
            jpeg_encode_driver_in_done_state_recv_suspend_event,
            jpeg_encode_driver_in_done_state_recv_resume_event,
            no_op,
            jpeg_encode_driver_in_done_state_recv_deliver_event
        },
    },
    .sync_hook = jpeg_encode_driver_sync_hook,
    .sync_config_to_running_hook = jpeg_encode_driver_sync_hook,
    .sync_running_to_config_hook = jpeg_encode_driver_sync_running_to_config_hook,
};



static void init_jpeg_service_tcb_with_start_capture(jpeg_service_tcb_t *encode_request_tcb)
{
    if(encode_request_tcb){
        isil_vj_bus_t *vj_bus;
        jpeg_service_queue_t   *target_service_queue;
        jpeg_service_queue_t   *service_queue;
        isil_jpeg_logic_encode_chan_t *jpeg_logic;
        tcb_node_t	*service_tcb;
        isil_jpeg_encode_control_t    *encode_control;

        jpeg_logic = to_get_isil_jpeg_encode_chan_with_encode_request_tcb(encode_request_tcb);
        encode_control = &jpeg_logic->encode_control;
        service_tcb = &encode_request_tcb->service_tcb;

        vj_bus = &jpeg_logic->chip->chip_vj_bus;
        target_service_queue = service_queue = vj_bus->jpeg_capture_service_queue;
        for(; (service_queue - vj_bus->jpeg_capture_service_queue) < JPEG_MAX_RESOURCE_POOL; service_queue++){
            if((service_queue->op->get_queue_curr_entry_number(service_queue) == 0) && (service_queue->curr_consumer == NULL)){
                target_service_queue = service_queue;
                break;
            }

            if((target_service_queue->op->get_queue_curr_entry_number(target_service_queue) > service_queue->op->get_queue_curr_entry_number(service_queue))){
                target_service_queue = service_queue;
            }
        }

        service_queue = target_service_queue;
        service_tcb->op = &tcb_node_op;
        service_tcb->op->init(service_tcb);
        service_tcb->op->set_priv(service_tcb, service_queue);
        encode_request_tcb->context = encode_control;
        encode_request_tcb->type = JPEG_SERVICE_CAPTURE;
        encode_request_tcb->req_callback = encode_control->op->start_capture;

        service_queue->op->put_service_request_into_queue(service_queue, encode_request_tcb);
    }

}

static void init_jpeg_service_tcb_with_start_encode(jpeg_service_tcb_t *encode_request_tcb)
{
    if(encode_request_tcb){
        isil_vj_bus_t *vj_bus;
        jpeg_service_queue_t   *service_queue;
        isil_jpeg_logic_encode_chan_t *jpeg_logic;
        tcb_node_t	*service_tcb;
        isil_jpeg_encode_control_t    *encode_control;

        jpeg_logic = to_get_isil_jpeg_encode_chan_with_encode_request_tcb(encode_request_tcb);
        encode_control = &jpeg_logic->encode_control;
        service_tcb = &encode_request_tcb->service_tcb;

        service_tcb->op = &tcb_node_op;
        service_tcb->op->init(service_tcb);
        service_tcb->op->set_priv(service_tcb, encode_request_tcb);
        encode_request_tcb->context = encode_control;
        encode_request_tcb->type = JPEG_SERVICE_ENCODE;
        encode_request_tcb->req_callback = encode_control->op->start_encode;

        vj_bus = &jpeg_logic->chip->chip_vj_bus;
        service_queue = &vj_bus->jpeg_encode_service_queue;
        service_queue->op->put_service_request_into_queue(service_queue, encode_request_tcb);
    }

}

static void init_jpeg_service_tcb_with_null(jpeg_service_tcb_t *encode_request_tcb)
{
    if(encode_request_tcb){
        isil_jpeg_logic_encode_chan_t *jpeg_logic;
        tcb_node_t	*service_tcb;

        jpeg_logic = to_get_isil_jpeg_encode_chan_with_encode_request_tcb(encode_request_tcb);
        service_tcb = &encode_request_tcb->service_tcb;

        service_tcb->op = &tcb_node_op;
        service_tcb->op->init(service_tcb);
        service_tcb->op->set_priv(service_tcb, encode_request_tcb);
        encode_request_tcb->context = NULL;
        encode_request_tcb->type = JPEG_SERVICE_RESERVED;
        encode_request_tcb->req_callback = NULL;
    }
}

static void jpeg_service_queue_init(jpeg_service_queue_t *jpeg_service_queue)
{
    if(jpeg_service_queue != NULL){
        tcb_node_queue_t    *service_queue = &jpeg_service_queue->service_queue;
        spin_lock_init(&jpeg_service_queue->lock);
        atomic_set(&jpeg_service_queue->service_count, 0);
        service_queue->op = &tcb_node_queue_op;
        service_queue->op->init(service_queue);
    }
}

static int  jpeg_service_queue_get_queue_curr_entry_number(jpeg_service_queue_t *jpeg_service_queue)
{
    int ret = 0;
    if(jpeg_service_queue != NULL){
        tcb_node_queue_t    *service_queue = &jpeg_service_queue->service_queue;
        ret = service_queue->op->get_queue_curr_entry_number(service_queue);
    }
    return ret;
}

static void jpeg_service_queue_put_service_request_into_queue(jpeg_service_queue_t *jpeg_service_queue, struct jpeg_service_tcb *service_tcb)
{
    if((jpeg_service_queue!=NULL) && (service_tcb!=NULL)){
        tcb_node_queue_t    *service_queue = &jpeg_service_queue->service_queue;
        unsigned long   flags;
        int     curr_entry_number;

        spin_lock_irqsave(&jpeg_service_queue->lock, flags);
        curr_entry_number = jpeg_service_queue->op->get_queue_curr_entry_number(jpeg_service_queue);
        if((curr_entry_number==0) && (jpeg_service_queue->curr_consumer == NULL)){
            jpeg_service_queue->curr_consumer = service_tcb;
            if(service_tcb->req_callback) {
                if(service_tcb->req_callback(service_tcb->context)){
                }
                atomic_inc(&jpeg_service_queue->service_count);
            }
        } else {
            service_queue->op->put(service_queue, &service_tcb->service_tcb);
        }

        spin_unlock_irqrestore(&jpeg_service_queue->lock, flags);
    }
}

static void jpeg_service_queue_put_service_request_into_queue_header(jpeg_service_queue_t *jpeg_service_queue, jpeg_service_tcb_t *service_tcb)
{
    if((jpeg_service_queue!=NULL) && (service_tcb!=NULL)){
        tcb_node_queue_t *service_queue = &jpeg_service_queue->service_queue;
        unsigned long	flags;
        int	curr_entry_number;

        spin_lock_irqsave(&jpeg_service_queue->lock, flags);
        curr_entry_number = jpeg_service_queue->op->get_queue_curr_entry_number(jpeg_service_queue);
        if((curr_entry_number==0) && (jpeg_service_queue->curr_consumer == NULL)){
            jpeg_service_queue->curr_consumer = service_tcb;
            if(service_tcb->req_callback) {
                if(service_tcb->req_callback(service_tcb->context)){
                }
                atomic_inc(&jpeg_service_queue->service_count);
            }
        } else {
            service_queue->op->put_header(service_queue, &service_tcb->service_tcb);
        }
        spin_unlock_irqrestore(&jpeg_service_queue->lock, flags);
    }
}

static void jpeg_service_queue_try_get_curr_consumer_from_queue(jpeg_service_queue_t *jpeg_service_queue)
{
    if(jpeg_service_queue != NULL){
        tcb_node_queue_t    *service_queue = &jpeg_service_queue->service_queue;

        if((service_queue->op != NULL) && (jpeg_service_queue->curr_consumer == NULL)){
            tcb_node_t  *temp_node;
            service_queue->op->try_get(service_queue, &temp_node);
            if(temp_node != NULL){
                jpeg_service_queue->curr_consumer = to_get_jpeg_service_tcb_with_service_tcb(temp_node);
            }
        }
    }
}

static void jpeg_service_queue_release_curr_consumer(jpeg_service_queue_t *jpeg_service_queue)
{
    if(jpeg_service_queue != NULL){
        unsigned long	flags;
        spin_lock_irqsave(&jpeg_service_queue->lock, flags);
        jpeg_service_queue->curr_consumer = NULL;
        spin_unlock_irqrestore(&jpeg_service_queue->lock, flags);
    }
}

static void jpeg_trigger_pending_service_request(jpeg_service_queue_t *jpeg_service_queue)
{
    if(jpeg_service_queue != NULL){
        jpeg_service_tcb_t   *service_tcb;
        unsigned long	flags;

        spin_lock_irqsave(&jpeg_service_queue->lock, flags);
        jpeg_service_queue->op->try_get_curr_consumer_from_queue(jpeg_service_queue);
        service_tcb = jpeg_service_queue->curr_consumer;
        if(service_tcb != NULL){
            if(service_tcb->req_callback) {
                if(service_tcb->req_callback(service_tcb->context)){
                }
                atomic_inc(&jpeg_service_queue->service_count);
            }
        }
        spin_unlock_irqrestore(&jpeg_service_queue->lock, flags);
    }
}

static int jpeg_delete_pending_service_request(jpeg_service_queue_t *jpeg_service_queue, jpeg_service_tcb_t *service_tcb)
{
    int ret = 0;

    if(jpeg_service_queue){
        if(service_tcb){
            tcb_node_queue_t    *service_queue;
            tcb_node_t *node;

            service_queue = &jpeg_service_queue->service_queue;
            node = &service_tcb->service_tcb;
            ret = service_queue->op->delete(service_queue, node);
        }
    }

    return ret;
}

static struct jpeg_service_queue_operation jpeg_service_queue_op = {
    .init = jpeg_service_queue_init,
    .get_queue_curr_entry_number = jpeg_service_queue_get_queue_curr_entry_number,
    .put_service_request_into_queue = jpeg_service_queue_put_service_request_into_queue,
    .put_service_request_into_queue_header = jpeg_service_queue_put_service_request_into_queue_header,
    .try_get_curr_consumer_from_queue = jpeg_service_queue_try_get_curr_consumer_from_queue,
    .release_curr_consumer = jpeg_service_queue_release_curr_consumer,
    .trigger_pending_service_request = jpeg_trigger_pending_service_request,
    .delete_pending_service_request = jpeg_delete_pending_service_request,
};

static void init_jpeg_service_queue(jpeg_service_queue_t * jpeg_service_queue)
{
    if(jpeg_service_queue != NULL){
        jpeg_service_queue->op = &jpeg_service_queue_op;
        jpeg_service_queue->op->init(jpeg_service_queue);
    }
}

static void remove_jpeg_service_queue(jpeg_service_queue_t * jpeg_service_queue)
{
    if(jpeg_service_queue != NULL){
        jpeg_service_queue->op->release_curr_consumer(jpeg_service_queue);
        while(jpeg_service_queue->op->get_queue_curr_entry_number(jpeg_service_queue)){
            jpeg_service_queue->op->try_get_curr_consumer_from_queue(jpeg_service_queue);
            ISIL_DBG(ISIL_DBG_DEBUG, "release service tcb %p\n", jpeg_service_queue->curr_consumer);
            jpeg_service_queue->op->release_curr_consumer(jpeg_service_queue);
        }
    }
}

static int isil_jifi_header(char *buf, isil_vb_frame_tcb_t *frame)
{
    char  *char_p;
    struct APP_mark *app0_mark;
    struct DQT_table *dqt_table;
    struct SOF_table *sof_table;
    int width, height;
    /*
       struct DHT_table *dht_table[4] = {(struct DHT_table *)huffman_DC0_tbl, 
       (struct DHT_table *)huffman_DC1_tbl, 
       (struct DHT_table *)huffman_AC0_tbl, 
       (struct DHT_table *)huffman_AC1_tbl};

       struct DRI_table *dri_table;
       struct SOS_table *sos_table;
       struct COM_segment *com_seg;
       */

    if(!buf || !frame) {
        return 0;
    }

    width = frame->i_mb_x<<4;
    height= frame->i_mb_y<<4;

#if defined(__BIG_ENDIAN_BITFIELD)
#undef SWAP16
#define SWAP16(x) (x)
#endif

    char_p   = buf;
    /*SOI*/
    *(short *)char_p = SWAP16(SOI);
    char_p += 2;
    *char_p = 0xff;
    char_p++;
    /*only use app0 mark*/
    app0_mark = (struct APP_mark *)char_p;
    app0_mark->flag = SWAP16(APP(0));
    app0_mark->segment_len = SWAP16(0x0010);
    strcpy(app0_mark->string, JFIF_ID);
    app0_mark->version = SWAP16(0x0101);/*version 1.01*/
    app0_mark->unit    = 0x01; /*inch*/
    app0_mark->width   = SWAP16(0x0001);
    app0_mark->height   = SWAP16(0x0001);
    /*no scale*/
    app0_mark->scale_width = 0x00;
    app0_mark->scale_height= 0x00;
    char_p += SWAP16(app0_mark->segment_len) + 2;
#if 0
    /*add comment*/
    comment = (struct COM_segment *)char_p;
    comment->flag = SWAP16(COM);
    strcpy(comment->string, "intersil JPEG module");
    comment->segment_len = SWAP16(strlen(comment->string));
    char_p += SWAP16(comment->segment_len) + 2;
#endif
    /*quantization table Y*/
    dqt_table = (struct DQT_table *)char_p;
    dqt_table->flag = SWAP16(DQT);
    dqt_table->segment_len = SWAP16(0x43);
    dqt_table->table.bits = 0x0; /*8 bits*/
    dqt_table->table.id   = 0x0; /*Y*/
    memcpy(dqt_table->table.DQT_TABLE, base_q_tbl_y, 0x40);
    char_p += SWAP16(dqt_table->segment_len) + 2;

    /*quantization table UV*/
    dqt_table = (struct DQT_table *)char_p;
    dqt_table->flag = SWAP16(DQT);
    dqt_table->segment_len = SWAP16(0x43);
    dqt_table->table.bits = 0x0; /*8 bits*/
    dqt_table->table.id   = 0x1; /*UV*/
    memcpy(dqt_table->table.DQT_TABLE, base_q_tbl_uv, 0x40);
    char_p += SWAP16(dqt_table->segment_len) + 2;

    /*start of frame*/
    sof_table = (struct SOF_table *)char_p;
    sof_table->flag = SWAP16(SOF(0));//only one frame
    sof_table->segment_len = SWAP16(0x11);
    sof_table->bits = 0x08; /*8 bits*/
    sof_table->height = SWAP16(height);
    sof_table->width  = SWAP16(width);
    sof_table->color  = 0x03; /*YCrCb*/
    /*Y*/
    sof_table->comp[0].color_id = 0x01;
    sof_table->comp[0].HV       = 0x22;
    sof_table->comp[0].QDT_id   = 0x00; /*use Y*/
    /*Cr*/
    sof_table->comp[1].color_id = 0x02;
    sof_table->comp[1].HV       = 0x11;
    sof_table->comp[1].QDT_id   = 0x01; /*use UV*/
    /*Cb*/
    sof_table->comp[2].color_id = 0x03;
    sof_table->comp[2].HV       = 0x11;
    sof_table->comp[2].QDT_id   = 0x01; /*use UV*/

    char_p += SWAP16(sof_table->segment_len) + 2;

    /*Huffman Table*/
    memcpy(char_p, huffman_DC0_tbl, sizeof(huffman_DC0_tbl));
    char_p += sizeof(huffman_DC0_tbl);
    memcpy(char_p, huffman_DC1_tbl, sizeof(huffman_DC1_tbl));
    char_p += sizeof(huffman_DC1_tbl);
    memcpy(char_p, huffman_AC0_tbl, sizeof(huffman_AC0_tbl));
    char_p += sizeof(huffman_AC0_tbl);
    memcpy(char_p, huffman_AC1_tbl, sizeof(huffman_AC1_tbl));
    char_p += sizeof(huffman_AC1_tbl);

    /*start of scan*/
#if 0
    sos_table = char_p;
    sos_table->flag = SWAP16(SOS);
    sos_table->segment_len = SWAP16(0x0c);
    sos_table->color_comp_id = 0x03;
    sos_table->start_info[0].DC_table   = 0x01;
    sos_table->start_info[0].AC_table   = 0x00;
    sos_table->start_info[0].data_start = 0x00;
    sos_table->start_info[0].data_end   = 0x02;
    sos_table->start_info[0].data_select= 0x11;
#else
    memcpy(char_p, sos_table_default, sizeof(sos_table_default));
#endif
    char_p += sizeof(sos_table_default);

#if defined(__BIG_ENDIAN_BITFIELD)
#undef SWAP16
#endif

    return (char_p - buf);
}

int jpeg_encode_hcd_interface_open(ed_tcb_t *ed_tcb)
{
    if(ed_tcb){
        isil_jpeg_logic_encode_chan_t *jpeg_logic_chan;
        robust_processing_control_t *robust_processing_control;

        jpeg_logic_chan = to_get_jpeg_channel_with_opened_logic_chan_ed(ed_tcb);
        robust_processing_control = &jpeg_logic_chan->robust_processing_control;
        robust_processing_control->op->wait_robust_processing_done(robust_processing_control);
        if(atomic_read(&jpeg_logic_chan->opened_flag) == 0){
            isil_chip_t *chip;

            chip = jpeg_logic_chan->chip;
            chip->chip_open(chip);
        }
        driver_gen_open_event(ed_tcb, 1);
    }

    return ISIL_OK;
}

int jpeg_encode_hcd_interface_close(ed_tcb_t *ed_tcb)
{
    if(ed_tcb){
        isil_register_node_t	*ed;
        isil_jpeg_logic_encode_chan_t *jpeg_logic_chan;
        robust_processing_control_t *robust_processing_control;

        jpeg_logic_chan = to_get_jpeg_channel_with_opened_logic_chan_ed(ed_tcb);
        ed = &ed_tcb->ed;

        robust_processing_control = &jpeg_logic_chan->robust_processing_control;
        robust_processing_control->op->wait_robust_processing_done(robust_processing_control);

        driver_gen_close_event(ed_tcb, 1);
        ISIL_DBG(ISIL_DBG_INFO, "wait channel %d close\n", jpeg_logic_chan->phy_slot_id);
        ed->op->wait_complete(ed);
        ISIL_DBG(ISIL_DBG_INFO, "wait channel %d close complete\n", jpeg_logic_chan->phy_slot_id);
        if(atomic_read(&jpeg_logic_chan->opened_flag) == 0){
            isil_chip_t *chip;

            chip = jpeg_logic_chan->chip;
            chip->chip_close(chip);
        }
    }

    return ISIL_OK;
}

void jpeg_encode_hcd_interface_suspend(ed_tcb_t *ed_tcb)
{
    if(ed_tcb) {
        isil_register_node_t	*ed;
        isil_jpeg_logic_encode_chan_t *jpeg_logic_chan;

        jpeg_logic_chan = to_get_jpeg_channel_with_opened_logic_chan_ed(ed_tcb);
        ed = &ed_tcb->ed;

        ed->op->init_wait_suspend_complete(ed);
        driver_gen_suspend_event(ed_tcb, 1);
        ISIL_DBG(ISIL_DBG_INFO, "wait channel %d suspend\n", jpeg_logic_chan->phy_slot_id);
        ed->op->wait_suspend_complete(ed);
        ISIL_DBG(ISIL_DBG_INFO, "wait channel %d suspend complete\n", jpeg_logic_chan->phy_slot_id);
    }
}

void jpeg_encode_hcd_interface_resume(ed_tcb_t *ed_tcb)
{
    if(ed_tcb) {
	isil_register_node_t	*ed;
        isil_jpeg_logic_encode_chan_t *jpeg_logic_chan;

        jpeg_logic_chan = to_get_jpeg_channel_with_opened_logic_chan_ed(ed_tcb);
        ed = &ed_tcb->ed;

        ed->op->init_wait_resume_complete(ed);
        driver_gen_resume_event(ed_tcb, 1);
        ISIL_DBG(ISIL_DBG_INFO, "wait channel %d resume\n", jpeg_logic_chan->phy_slot_id);
        ed->op->wait_resume_complete(ed);
        ISIL_DBG(ISIL_DBG_INFO, "wait channel %d resume complete\n", jpeg_logic_chan->phy_slot_id);
    }
}

int jpeg_encode_hcd_interface_ioctl(ed_tcb_t *ed_tcb, unsigned int cmd, unsigned long arg)
{
    int ret = ISIL_OK;
    isil_jpeg_logic_encode_chan_t *jpeg_logic_encode_chan;
    isil_jpeg_encode_control_t *encode_control;
    isil_jpeg_encode_property_t *encode_property;
    isil_mjpeg_encode_param_t encode_param, *config_param = &encode_param;
    isil_video_bus_t           *video_bus;
    chip_driver_t            *chip_driver;
    isil_ed_fsm_t              *ed_fsm;
    robust_processing_control_t *robust_processing_control;
    isil_chip_t     *chip;
    isil_chip_timer_controller_t    *chip_timer_cont;

    if(!ed_tcb) {
        ISIL_DBG(ISIL_DBG_ERR, "null pointer\n");
        return -EINVAL;
    }

    ISIL_DBG(ISIL_DBG_INFO, "%c, %d\n", _IOC_TYPE(cmd), _IOC_NR(cmd));
    jpeg_logic_encode_chan  = to_get_isil_jpeg_encode_chan_with_opened_logic_chan_ed(ed_tcb);
    chip_driver             = jpeg_logic_encode_chan->chip->chip_driver;
    video_bus               = &chip_driver->video_bus;
    encode_property         = &jpeg_logic_encode_chan->encode_property;
    encode_control            = &jpeg_logic_encode_chan->encode_control;
    ed_fsm                  = &ed_tcb->ed_fsm;
    chip = jpeg_logic_encode_chan->chip;
    chip_timer_cont = &chip->chip_timer_cont;

    robust_processing_control = &jpeg_logic_encode_chan->robust_processing_control;
    robust_processing_control->op->wait_robust_processing_done(robust_processing_control);

    if(atomic_read(&jpeg_logic_encode_chan->opened_flag) == 0){
        ISIL_DBG(ISIL_DBG_ERR, "channel have not be opend\n");
        return -ENODEV;
    }

    switch(cmd) {
        case ISIL_CODEC_CHAN_FLUSH:
            {
                unsigned long   flags;

                spin_lock_irqsave(&jpeg_logic_encode_chan->fsm_matrix_call_lock, flags);
                encode_control->op->flush_all_frame(encode_control);
                spin_unlock_irqrestore(&jpeg_logic_encode_chan->fsm_matrix_call_lock, flags);
            }
            break;
        case ISIL_MJPEG_ENCODE_PARAM_GET:
            if(!arg) {
                ISIL_DBG(ISIL_DBG_ERR, "invalid arg\n");
                return -EINVAL;
            }
            if(copy_from_user(&encode_param, (void *)arg, sizeof(struct isil_mjpeg_encode_param)) != 0){
                printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
            }
            if(config_param->change_mask_flag & ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_IMAGE_LEVEL_MASK) {
                encode_param.e_image_level = encode_property->op->get_image_level(encode_property);
            }
            if(config_param->change_mask_flag & ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_IMAGE_WIDTH_MASK) {
                encode_param.i_image_width_mb_size = encode_property->op->get_mb_width(encode_property);
            }
            if(config_param->change_mask_flag & ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_IMAGE_HEIGHT_MASK) {
                config_param->i_image_height_mb_size = encode_property->op->get_mb_height(encode_property);
            }
            if(encode_param.change_mask_flag & ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_CAPTURE_FRAME_NUMBER_MASK) {
                config_param->i_capture_frame_number = encode_property->op->get_capture_number(encode_property);
            }
            if(encode_param.change_mask_flag & ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_CAPTURE_FRAME_STRIDE_MASK) {
                config_param->i_capture_frame_stride = encode_property->op->get_capture_stride(encode_property);
            }
            if(encode_param.change_mask_flag & ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_CAPTURE_TYPE_MASK) {
                config_param->i_capture_type = encode_property->op->get_capture_type(encode_property);
            }

            if(copy_to_user((void *)arg, &encode_param, sizeof(isil_mjpeg_encode_param_t)) != 0){
                printk("%s, %d: copy_to_user failed\n", __FUNCTION__, __LINE__);
            }
            break;
        case ISIL_MJPEG_ENCODE_PARAM_SET:
            if(!arg) {
                ISIL_DBG(ISIL_DBG_ERR, "invalid arg\n");
                return -EINVAL;
            }
            if(copy_from_user(&encode_param, (void *)arg, sizeof(struct isil_mjpeg_encode_param)) != 0){
                printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
            }
            if(encode_param.change_mask_flag & ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_IMAGE_LEVEL_MASK) {
                if((encode_param.e_image_level < MJPEG_IMAGE_LEVEL_0) || (encode_param.e_image_level >= MJPEG_IMAGE_LEVEL_RESERVE)){
                    ISIL_DBG(ISIL_DBG_ERR, "image level %d unsupport\n", encode_param.e_image_level);
                    ret = -EINVAL;
                }

            }
            if(encode_param.change_mask_flag & ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_CAPTURE_FRAME_STRIDE_MASK) {
                if(encode_param.i_capture_frame_stride < ISIL_MJPEG_MIN_INTERVAL){
                    ISIL_DBG(ISIL_DBG_ERR, "jpeg interval %d error, >=%dms\n", encode_param.i_capture_frame_stride, ISIL_MJPEG_MIN_INTERVAL);
                    ret = -EINVAL;
                }
            }
            if(encode_param.change_mask_flag & ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_IMAGE_WIDTH_MASK) {
                if(encode_param.change_mask_flag & ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_IMAGE_HEIGHT_MASK){
                    if((encode_param.i_image_width_mb_size < (MIN_FRAME_WIDTH>>4)) || (encode_param.i_image_height_mb_size < (MIN_FRAME_HEIGHT>>4))
                            || (encode_param.i_image_width_mb_size > (MAX_FRAME_WIDTH>>4))
                            || (encode_param.i_image_height_mb_size > (MAX_FRAME_HEIGHT>>4))) {
                        ISIL_DBG(ISIL_DBG_ERR, "width or hight error(%d, %d)\n", encode_param.i_image_width_mb_size, encode_param.i_image_height_mb_size);
                        ret = -EINVAL;
                    }
                }else{
                    ISIL_DBG(ISIL_DBG_ERR, "hight should be set\n");
                    ret = -EINVAL;
                }
            }


            if(ret == ISIL_OK) {
                if(encode_param.change_mask_flag & ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_IMAGE_LEVEL_MASK) {
                    encode_property->op->set_image_level(encode_property, encode_param.e_image_level);
                }
                if(encode_param.change_mask_flag & ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_IMAGE_WIDTH_MASK) {
                    if(encode_param.change_mask_flag & ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_IMAGE_HEIGHT_MASK){
                        encode_property->op->set_mb_height(encode_property, encode_param.i_image_height_mb_size);
                        encode_property->op->set_mb_width(encode_property, encode_param.i_image_width_mb_size);
                    }
                }
                if(encode_param.change_mask_flag & ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_CAPTURE_FRAME_STRIDE_MASK) {
                    encode_property->op->set_capture_stride(encode_property, encode_param.i_capture_frame_stride);
                }
                if(encode_param.change_mask_flag & ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_CAPTURE_FRAME_NUMBER_MASK) {
                    encode_property->op->set_capture_number(encode_property, encode_param.i_capture_frame_number);
                }
                if(encode_param.change_mask_flag & ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_CAPTURE_TYPE_MASK) {
                    encode_property->op->set_capture_type(encode_property, encode_param.i_capture_type);
                }
                encode_property->encode_property.change_mask_flag = encode_param.change_mask_flag;
                driver_sync_config_to_running(ed_tcb);
                if(encode_param.i_capture_type & (ISIL_MJPEG_ENCODE_PARAM_CAPTURE_TYPE_USER | ISIL_MJPEG_ENCODE_PARAM_CAPTURE_TYPE_TIMER)){
                    if(encode_control->schedule_time == INVALIDTIMERID){
                        encode_control->schedule_time = chip_timer_cont->op->AddSingleFireTimerJob(chip_timer_cont, TIMER_5MS, isil_jpeg_schedule_restart, encode_control);
                        if(encode_control->schedule_time == INVALIDTIMERID){
                            ISIL_DBG(ISIL_DBG_FATAL, "add timer error\n");
                        }
                    }
                }
            }
            break;
        case ISIL_LOGIC_CHAN_ENABLE_SET:
            ed_tcb->op->resume(ed_tcb);
            ret = ISIL_OK;
            break;
        case ISIL_LOGIC_CHAN_DISABLE_SET:
            ed_tcb->op->suspend(ed_tcb);
            ret = ISIL_OK;
            break;
        default:
            ISIL_DBG(ISIL_DBG_ERR, "no such cmd %d\n", _IOC_NR(cmd));
            ret = -EBADRQC;
    }

    return ret;
}

int jpeg_encode_hcd_interface_get_state(ed_tcb_t *ed_tcb)
{
    int	ret = ISIL_ED_UNREGISTER;

    if(ed_tcb != NULL){
        ret = atomic_read(&ed_tcb->state);
    }

    return ret;
}

static isil_hcd_interface_operation jpeg_encode_hcd_interface_op = {
    .open = jpeg_encode_hcd_interface_open,
    .close = jpeg_encode_hcd_interface_close,
    .suspend = jpeg_encode_hcd_interface_suspend,
    .resume = jpeg_encode_hcd_interface_resume,
    .ioctl = jpeg_encode_hcd_interface_ioctl,
    .get_state = jpeg_encode_hcd_interface_get_state,
};

static void jpeg_logic_encode_chan_driver_notify(isil_register_node_t *node, void *priv, isil_notify_msg *msg)
{
    if((node!=NULL) && (priv!=NULL)){

    }
}

static int jpeg_logic_encode_chan_driver_match_id(isil_register_node_t *node, void *priv, unsigned long logic_chan_id)
{
    int	ret = ISIL_ERR;
    if((node!=NULL) && (priv!=NULL)){
        isil_jpeg_logic_encode_chan_t *jpeg_logic = (isil_jpeg_logic_encode_chan_t*)priv;
        if(jpeg_logic->logic_chan_id == logic_chan_id){
            ret = ISIL_OK;
        }
    }

    return ret;
}

int isil_jpeg_encode_property_init(isil_jpeg_encode_property_t *encode_property)
{
    if(encode_property) {
        isil_mjpeg_encode_param_t *encode_param = &encode_property->encode_property;

        encode_param->e_image_level          = MJPEG_IMAGE_LEVEL_0;
        encode_param->i_capture_frame_number = 0;
        encode_param->i_capture_frame_stride = 0;
        encode_param->i_capture_type         = 0;
        encode_param->i_image_width_mb_size  = 0;
        encode_param->i_image_height_mb_size = 0;
        encode_param->change_mask_flag       = 0;
    }

    return 0;
}

int isil_jpeg_encode_property_reset(isil_jpeg_encode_property_t *encode_property)
{
    if(encode_property) {
        isil_mjpeg_encode_param_t *encode_param = &encode_property->encode_property;

        encode_param->e_image_level          = MJPEG_IMAGE_LEVEL_0;
        encode_param->i_capture_frame_number = 0;
        encode_param->i_capture_frame_stride = 0;
        encode_param->i_capture_type         = 0;
        encode_param->i_image_width_mb_size  = 0;
        encode_param->i_image_height_mb_size = 0;
        encode_param->change_mask_flag       = 0;
    }

    return 0;
}

u32 isil_jpeg_encode_property_get_image_level(isil_jpeg_encode_property_t *encode_property)
{
    if(encode_property) {
        isil_mjpeg_encode_param_t *encode_param = &encode_property->encode_property;

        return encode_param->e_image_level;
    }

    return 0;
}
u32 isil_jpeg_encode_property_get_mb_width(isil_jpeg_encode_property_t *encode_property)
{
    if(encode_property) {
        isil_mjpeg_encode_param_t *encode_param = &encode_property->encode_property;

        return encode_param->i_image_width_mb_size;
    }

    return 0;
}
u32 isil_jpeg_encode_property_get_mb_height(isil_jpeg_encode_property_t *encode_property)
{
    if(encode_property) {
        isil_mjpeg_encode_param_t *encode_param = &encode_property->encode_property;

        return encode_param->i_image_height_mb_size;
    }

    return 0;
}
u32 isil_jpeg_encode_property_get_capture_number(isil_jpeg_encode_property_t *encode_property)
{
    if(encode_property) {
        isil_mjpeg_encode_param_t *encode_param = &encode_property->encode_property;

        return encode_param->i_capture_frame_number;
    }

    return 0;
}
u32 isil_jpeg_encode_property_get_capture_stride(isil_jpeg_encode_property_t *encode_property)
{
    if(encode_property) {
        isil_mjpeg_encode_param_t *encode_param = &encode_property->encode_property;

        return encode_param->i_capture_frame_stride;
    }

    return 0;
}
u32 isil_jpeg_encode_property_get_capture_type(isil_jpeg_encode_property_t *encode_property)
{
    if(encode_property) {
        isil_mjpeg_encode_param_t *encode_param = &encode_property->encode_property;

        return encode_param->i_capture_type;
    }

    return 0;
}

int isil_jpeg_encode_property_set_image_level(isil_jpeg_encode_property_t *encode_property, enum ISIL_MJPEG_IMAGE_LEVEL_E level)
{
    if(encode_property) {
        isil_mjpeg_encode_param_t *encode_param = &encode_property->encode_property;

        encode_param->e_image_level = level;
    }

    return 0;
}
int isil_jpeg_encode_property_set_mb_width(isil_jpeg_encode_property_t *encode_property, u32 width)
{
    if(encode_property) {
        isil_mjpeg_encode_param_t *encode_param = &encode_property->encode_property;

        if(width == (WIDTH_FRAME_4CIF_PAL>>4)) {
            ISIL_DBG(ISIL_DBG_INFO, "adjust width from %d to %d\n", width, WIDTH_FRAME_D1_PAL>>4);
            width = WIDTH_FRAME_D1_PAL>>4;
        }

        if(width == (WIDTH_FRAME_4CIF_NTSC>>4)) {
            ISIL_DBG(ISIL_DBG_INFO, "adjust width from %d to %d\n", width, WIDTH_FRAME_D1_NTSC>>4);
            width = WIDTH_FRAME_D1_NTSC>>4;
        }

        encode_param->i_image_width_mb_size = width;
    }

    return 0;
}
int isil_jpeg_encode_property_set_mb_height(isil_jpeg_encode_property_t *encode_property, u32 height)
{
    if(encode_property) {
        isil_mjpeg_encode_param_t *encode_param = &encode_property->encode_property;

        encode_param->i_image_height_mb_size = height;
    }

    return 0;
}
int isil_jpeg_encode_property_set_capture_number(isil_jpeg_encode_property_t *encode_property, u32 number)
{
    if(encode_property) {
        isil_mjpeg_encode_param_t *encode_param = &encode_property->encode_property;

        encode_param->i_capture_frame_number = number;
    }

    return 0;
}
int isil_jpeg_encode_property_set_capture_stride(isil_jpeg_encode_property_t *encode_property, u32 stride)
{
    if(encode_property) {
        isil_mjpeg_encode_param_t *encode_param = &encode_property->encode_property;

        encode_param->i_capture_frame_stride = stride;
    }

    return 0;
}
int isil_jpeg_encode_property_set_capture_type(isil_jpeg_encode_property_t *encode_property, u32 type)
{
    if(encode_property) {
        isil_mjpeg_encode_param_t *encode_param = &encode_property->encode_property;

        encode_param->i_capture_type = type;
    }

    return 0;
}

static struct isil_jpeg_encode_property_operation jpeg_encode_property_op = {
    .init   = isil_jpeg_encode_property_init,
    .reset  = isil_jpeg_encode_property_reset,

    .get_capture_number = isil_jpeg_encode_property_get_image_level,
    .get_capture_stride = isil_jpeg_encode_property_get_capture_stride,
    .get_capture_type   = isil_jpeg_encode_property_get_capture_type,
    .get_image_level    = isil_jpeg_encode_property_get_image_level,
    .get_mb_height      = isil_jpeg_encode_property_get_mb_height,
    .get_mb_width       = isil_jpeg_encode_property_get_mb_width,

    .set_capture_number = isil_jpeg_encode_property_set_capture_number,
    .set_capture_stride = isil_jpeg_encode_property_set_capture_stride,
    .set_capture_type   = isil_jpeg_encode_property_set_capture_type,
    .set_image_level    = isil_jpeg_encode_property_set_image_level,
    .set_mb_height      = isil_jpeg_encode_property_set_mb_height,
    .set_mb_width       = isil_jpeg_encode_property_set_mb_width,
};

/*init_jpeg_encode_property:
desc:init jpeg encoder paramter
encode_property:
config_param: NULL->configure default parameter
*/
static void init_jpeg_encode_property(isil_jpeg_encode_property_t *encode_property, isil_mjpeg_encode_param_t *config_param)
{
    if(encode_property && config_param){
        spin_lock_init(&encode_property->lock);
        encode_property->op = &jpeg_encode_property_op;
        encode_property->op->init(encode_property);

        encode_property->op->set_capture_number(encode_property, config_param->i_capture_frame_number);
        encode_property->op->set_capture_stride(encode_property, config_param->i_capture_frame_stride);
        encode_property->op->set_capture_type(encode_property, config_param->i_capture_type);
        encode_property->op->set_image_level(encode_property, config_param->e_image_level);
        encode_property->op->set_mb_width(encode_property, config_param->i_image_width_mb_size);
        encode_property->op->set_mb_height(encode_property, config_param->i_image_height_mb_size);
        encode_property->encode_property.change_mask_flag = config_param->change_mask_flag;

        /*update defaut running configure*/
        memcpy(&encode_property->running_encode_property, &encode_property->encode_property, sizeof(isil_mjpeg_encode_param_t));
    }

    return ;
}

static int  jpeg_logic_chan_response_ddr_burst_done_isr(int irq, void *context)
{
    isil_vb_frame_tcb_queue_t  *frame_queue;
    isil_vb_frame_tcb_t        *frame;
    isil_vb_packet_tcb_queue_t *packet_queue;
    isil_vb_packet_tcb_t       *packet;
    dpram_page_node_t        *dpram_page;
    ddr_burst_interface_t    *burst_interface; 
        isil_jpeg_encode_control_t  *encode_control; 
    isil_chip_t             *chip;
        dpram_control_t             *chip_dpram_controller;
    chip_ddr_burst_engine_t     *chip_ddr_burst_interface;
    isil_jpeg_logic_encode_chan_t *jpeg_logic_encode_chan;
    
    jpeg_logic_encode_chan = (isil_jpeg_logic_encode_chan_t *)context;
    chip = jpeg_logic_encode_chan->chip;
    if(jpeg_logic_encode_chan){
#if defined(PIO_INTERFACE)
        void *temp_pio_addr;
#else
        dma_addr_t  temp_dma_addr;
#endif

       get_isil_dpram_controller(&chip_dpram_controller);
       chip_ddr_burst_interface = &chip->chip_ddr_burst_interface;
       chip_ddr_burst_interface->op->get_ddr_burst_interface(chip_ddr_burst_interface, &burst_interface);
       dpram_page            = jpeg_logic_encode_chan->dpram_page;
       encode_control        = &jpeg_logic_encode_chan->encode_control;

       /*clear burst done irq*/
       burst_interface->op->clear_burst_done(burst_interface);

       frame_queue = &jpeg_logic_encode_chan->jpeg_frame_queue;
       frame = frame_queue->curr_producer;
       packet_queue = &frame->vb_packet_queue;
       if(packet_queue->curr_producer == NULL){
           packet_queue->op->try_get_curr_producer_from_pool(packet_queue, &jpeg_logic_encode_chan->pool);
       }
       packet = packet_queue->curr_producer;
       if(packet == NULL){
           frame->frame_is_err = 1;
           goto burst_read_end;
       }
#if defined(PIO_INTERFACE)
       temp_pio_addr = packet->data + packet->payload_len;
       burst_interface->op->pio_host_to_sram_read(burst_interface, dpram_page, temp_pio_addr, encode_control->trans_len, 0);
#else
       packet->op->dma_map(packet, DMA_FROM_DEVICE);
       temp_dma_addr = packet->dma_addr + packet->payload_len;
       burst_interface->op->dma_host_to_sram_read(burst_interface, dpram_page, temp_dma_addr, encode_control->trans_len, 0);
#endif

       packet->payload_len += encode_control->trans_len;
       encode_control->frame_base_addr += encode_control->trans_len;

       encode_control->have_recv_section_number++;
       if(!(encode_control->have_recv_section_number&0x1)){
           if(packet_queue->curr_producer != NULL){
               packet_queue->curr_producer->op->dma_unmap(packet_queue->curr_producer, DMA_FROM_DEVICE);
           }
           packet_queue->op->put_curr_producer_into_queue(packet_queue);
       }
       encode_control->section_number--;

       if(encode_control->section_number > 0){
           if(encode_control->section_number == 1){
               encode_control->trans_len = encode_control->tailer_len;
           }else{
               encode_control->trans_len = encode_control->section_len;
           }

           burst_interface->op->start_nonblock_transfer_from_ddr_to_sram(burst_interface, 
                   dpram_page, encode_control->frame_base_addr, 
                   encode_control->frame_base_addr >> 19, encode_control->trans_len, DDR_CHIP_B);

           return IRQ_HANDLED;
       } else {
           if(encode_control->have_recv_section_number&0x1){
               packet_queue = &frame->vb_packet_queue;
               if(packet_queue->curr_producer != NULL){
                   packet_queue->curr_producer->op->dma_unmap(packet_queue->curr_producer, DMA_FROM_DEVICE);
               }
               packet_queue->op->put_curr_producer_into_queue(packet_queue);
           }
       }

burst_read_end:
       //notify data move req queue to update
       encode_control->op->notify(encode_control);
       chip_free_irq(jpeg_logic_encode_chan->chip, IRQ_BURST_TYPE_INTR, jpeg_logic_encode_chan);
		get_isil_dpram_controller(&chip_dpram_controller);
       chip_dpram_controller->op->ack_read_mjpeg_req(chip_dpram_controller, jpeg_logic_encode_chan);
    }

    return IRQ_HANDLED;
}

static int  jpeg_logic_chan_start_read_bitstream(dpram_request_t *dpram_req, void *context)
{
    int ret=1;

    if((dpram_req!=NULL) && (context!=NULL)){
        isil_jpeg_logic_encode_chan_t *jpeg_logic_chan;
        isil_jpeg_encode_control_t  *encode_control;
        isil_vb_frame_tcb_queue_t   *frame_queue;
        isil_vb_frame_tcb_t         *frame;
        isil_vb_packet_tcb_t        *packet;
        isil_vb_packet_tcb_queue_t  *packet_queue;
        dpram_page_node_t           *dpram_page;
        dpram_control_t             *chip_dpram_controller;
            chip_ddr_burst_engine_t     *chip_ddr_burst_interface;
        ddr_burst_interface_t       *burst_interface;
        isil_timestamp_t            *timestamp;
        isil_chip_t                 *chip;
        isil_vj_resource_tcb_t      *resource; 
        isil_vj_resource_queue_t    *resource_queue; 
        u32 header_len = 0, align = sizeof(int) - (sizeof(ext_h264_nal_bitstream_t) % 4);
        int encode_len = 0;


        jpeg_logic_chan = (isil_jpeg_logic_encode_chan_t*)context;
        encode_control = &jpeg_logic_chan->encode_control;
            get_isil_dpram_controller(&chip_dpram_controller);
        chip = jpeg_logic_chan->chip;
        timestamp = &jpeg_logic_chan->timestamp;
        encode_control = &jpeg_logic_chan->encode_control;
        frame_queue = &jpeg_logic_chan->jpeg_frame_queue;
        resource_queue = &jpeg_logic_chan->encode_resource_queue;
        resource_queue->op->try_get_curr_consumer(resource_queue);
        resource = resource_queue->curr_consumer;

        timestamp->op->set_timestamp(timestamp, resource->op->get_timestamp(resource), __FILE__);
        frame = frame_queue->curr_producer;
        frame->frame_type   = MJPEG_FRAME_TYPE;
        frame->frame_number = encode_control->i_frame_serial;
        frame->timestamp    = timestamp->op->get_timestamp(timestamp);

        chip_ddr_burst_interface = &chip->chip_ddr_burst_interface;
        chip_ddr_burst_interface->op->get_ddr_burst_interface(chip_ddr_burst_interface, &burst_interface);

        if(!chip_dpram_controller->op->is_can_submit_move_data_from_ddr_to_dpram_service_req(chip_dpram_controller, &jpeg_logic_chan->dpram_page, chip)){
            ISIL_DBG(ISIL_DBG_ERR, "no dpram_page!\n");
            frame->frame_is_err = 1;
            encode_control->op->notify(encode_control);

            return -EBUSY;
        }

        dpram_page = jpeg_logic_chan->dpram_page;
        packet_queue = &frame->vb_packet_queue;
        packet_queue->op->try_get_curr_producer_from_pool(packet_queue, &jpeg_logic_chan->pool);
        packet = packet_queue->curr_producer;
        if (packet == NULL) {
            frame->frame_is_err = 1;
            chip_dpram_controller->op->ack_read_mjpeg_req(chip_dpram_controller, jpeg_logic_chan);
            encode_control->op->notify(encode_control);
        }else{
            encode_len = resource->op->get_data_size(resource);
            if(encode_len <= 0){
                ISIL_DBG(ISIL_DBG_ERR, "jpeg encode length error\n");
                frame->frame_is_err = 1;
                encode_control->op->notify(encode_control);
                return -EFAULT;
            }
            header_len = isil_jifi_header(packet->data + sizeof(ext_h264_nal_bitstream_t) + align, frame);
            gen_ext_mjpeg_header(packet->data + align, encode_len + header_len);
            packet->consumer_len = align;
            packet->payload_len = header_len + sizeof(ext_h264_nal_bitstream_t) + align;
            packet->op->dma_map(packet, DMA_TO_DEVICE);
            encode_control->total_frame_len = encode_len + header_len + sizeof(ext_h264_nal_bitstream_t) + align;
            encode_control->frame_base_addr = resource->op->get_ddr_phy_addr(resource);
            encode_control->have_recv_section_number = 0;
            encode_control->section_len = dpram_page->op->get_page_size(dpram_page);
            encode_control->section_number = (encode_control->total_frame_len) / encode_control->section_len;
            encode_control->tailer_len = (encode_control->total_frame_len) % encode_control->section_len;

            frame->frame_len = encode_control->total_frame_len;
            if(encode_control->tailer_len){
                encode_control->section_number++;
            }
            encode_control->trans_len = encode_control->section_len - header_len - sizeof(ext_h264_nal_bitstream_t) - align;
            if(encode_control->trans_len > encode_len){
                encode_control->trans_len = encode_len;
            }
            /*burst*/
            chip_request_irq(jpeg_logic_chan->chip, IRQ_BURST_TYPE_INTR, jpeg_logic_chan_response_ddr_burst_done_isr, "BURST", jpeg_logic_chan);
            burst_interface->op->start_nonblock_transfer_from_ddr_to_sram(burst_interface, dpram_page, 
                    encode_control->frame_base_addr, encode_control->frame_base_addr >> 19, encode_control->trans_len, DDR_CHIP_B);

        }
    }else{
        ISIL_DBG(ISIL_DBG_ERR, "null pointer\n");
    }

    return ret;
}

static void init_submit_recv_jpeg_done_service(dpram_request_t *dpram_req, isil_jpeg_logic_encode_chan_t *jpeg_logic_encode_chan)
{
    if((dpram_req!=NULL) || (jpeg_logic_encode_chan!=NULL)){
        dpram_req->chip = jpeg_logic_encode_chan->chip;
        dpram_req->type = DPRAM_NONBLOCK_TRANSFER_REQUEST;
        dpram_req->context = (void*)jpeg_logic_encode_chan;
        dpram_req->req_callback = jpeg_logic_chan_start_read_bitstream;
    }
}

static int isil_jpeg_schedule_restart(void *context)
{
    isil_jpeg_encode_control_t *encode_control = (isil_jpeg_encode_control_t *)context;
    if(encode_control){
        isil_jpeg_logic_encode_chan_t *jpeg_logic_chan;
        isil_vb_frame_tcb_queue_t *jpeg_frame_queue;
        ed_tcb_t *ed;
        isil_ed_fsm_t *fsm;
        isil_chip_t *chip;
        isil_chip_timer_controller_t    *chip_timer_cont;

        jpeg_logic_chan = to_get_jpeg_channel_with_encode_control(encode_control);
        jpeg_frame_queue = &jpeg_logic_chan->jpeg_frame_queue;
        ed = &jpeg_logic_chan->opened_logic_chan_ed;
        fsm = &ed->ed_fsm;
        chip = jpeg_logic_chan->chip;
        chip_timer_cont = &chip->chip_timer_cont;

        if(fsm->op->get_curr_state(fsm) != ISIL_ED_IDLE){
            return ISIL_OK;
        }

        encode_control->schedule_time = INVALIDTIMERID;
        jpeg_frame_queue->op->try_get_curr_producer_from_pool(jpeg_frame_queue, &jpeg_logic_chan->pool);
        if(jpeg_frame_queue->curr_producer){
            driver_gen_deliver_event(ed, 1);
            init_jpeg_service_tcb_with_start_capture(&jpeg_logic_chan->encode_request_tcb);
            //ISIL_DBG(ISIL_DBG_DEBUG, "ed_fsm = %p\n", fsm);
        }else{
            encode_control->schedule_time = chip_timer_cont->op->AddSingleFireTimerJob(chip_timer_cont, TIMER_5MS, isil_jpeg_schedule_restart, encode_control);
            if(encode_control->schedule_time == INVALIDTIMERID){
                ISIL_DBG(ISIL_DBG_ERR, "add timer error\n");
            }
        }

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_jpeg_encode_control_init(isil_jpeg_encode_control_t *jpeg_encode_control)
{
    if(jpeg_encode_control){
        
        jpeg_encode_control->total_frame_len = 0;
        jpeg_encode_control->frame_base_addr = 0;
        jpeg_encode_control->section_number = 0;
        jpeg_encode_control->have_recv_section_number = 0;
        jpeg_encode_control->section_len = 0;
        jpeg_encode_control->tailer_len = 0;
        jpeg_encode_control->last_jiffies = 0;
        jpeg_encode_control->int_time = 0;
        jpeg_encode_control->lost_frame = 0;
        jpeg_encode_control->i_frame_serial = 0;
        jpeg_encode_control->b_flush_all_frame = 0;

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_jpeg_encode_control_reset(isil_jpeg_encode_control_t *jpeg_encode_control)
{
    if(jpeg_encode_control){
        
        jpeg_encode_control->total_frame_len = 0;
        jpeg_encode_control->frame_base_addr = 0;
        jpeg_encode_control->section_number = 0;
        jpeg_encode_control->have_recv_section_number = 0;
        jpeg_encode_control->section_len = 0;
        jpeg_encode_control->tailer_len = 0;
        jpeg_encode_control->last_jiffies = 0;
        jpeg_encode_control->int_time = 0;
        jpeg_encode_control->lost_frame = 0;
        jpeg_encode_control->i_frame_serial = 0;
        jpeg_encode_control->b_flush_all_frame = 0;

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_jpeg_encode_control_release(isil_jpeg_encode_control_t *jpeg_encode_control)
{
    if(jpeg_encode_control){
        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_jpeg_encode_control_first_start(isil_jpeg_encode_control_t *jpeg_encode_control)
{
    if(jpeg_encode_control){
        isil_jpeg_logic_encode_chan_t   *jpeg_logic_encode_chan;
	isil_vb_frame_tcb_queue_t	*jpeg_frame_queue;
	isil_jpeg_encode_property_t   *encode_property;
	isil_mjpeg_encode_param_t     *running_encode_property;
	ed_tcb_t *opened_logic_chan_ed;
        isil_ed_fsm_t *fsm;

        jpeg_logic_encode_chan  = to_get_jpeg_channel_with_encode_control(jpeg_encode_control);
        jpeg_frame_queue = &jpeg_logic_encode_chan->jpeg_frame_queue;
        encode_property = &jpeg_logic_encode_chan->encode_property;
        running_encode_property = &encode_property->running_encode_property;

        ISIL_DBG(ISIL_DBG_DEBUG, "channel %d first start\n", jpeg_logic_encode_chan->phy_slot_id);
        jpeg_frame_queue->op->try_get_curr_producer_from_pool(jpeg_frame_queue, &jpeg_logic_encode_chan->pool);
        if(!jpeg_frame_queue->curr_producer){
            ISIL_DBG(ISIL_DBG_ERR, "can't get frame!\n");
            return ISIL_ERR;
        }

        if(running_encode_property->i_capture_type & ISIL_MJPEG_ENCODE_PARAM_CAPTURE_TYPE_TIMER){
            opened_logic_chan_ed = &jpeg_logic_encode_chan->opened_logic_chan_ed;
            fsm = &opened_logic_chan_ed->ed_fsm;
            if(fsm->op->get_curr_state(fsm) == ISIL_ED_IDLE){
                driver_gen_deliver_event(&jpeg_logic_encode_chan->opened_logic_chan_ed, 1);
                init_jpeg_service_tcb_with_start_capture(&jpeg_logic_encode_chan->encode_request_tcb);
            }else{
                ISIL_DBG(ISIL_DBG_ERR, "curr_state(%d) != ISIL_ED_IDLE\n", fsm->op->get_curr_state(fsm));
            }
        }
 
        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_jpeg_encode_control_start_capture(void *context)
{
    isil_jpeg_encode_control_t *jpeg_encode_control = (isil_jpeg_encode_control_t *)context;
    if(jpeg_encode_control){
        isil_jpeg_logic_encode_chan_t *jpeg_logic_chan;
        isil_jpeg_phy_video_slot_t *phy_video_slot;
        isil_chip_t *chip;
        chip_driver_t *chip_driver;
        isil_video_bus_t *video_bus;
        isil_vj_bus_t *vj_bus;
	jpeg_service_queue_t        *jpeg_capture_service_queue;
	tcb_node_t	*service_tcb;
	jpeg_service_tcb_t    *encode_request_tcb;
        isil_jpeg_encode_property_t   *encode_property;
	isil_mjpeg_encode_param_t     *running_encode_property;
        isil_vj_resource_pool_t *pool;
        isil_vj_resource_tcb_t *resource; 
        isil_vj_resource_queue_t *resource_queue; 
        isil_vb_frame_tcb_queue_t *jpeg_frame_queue;
        isil_vb_frame_tcb_t *frame;
        ed_tcb_t *ed_tcb;
        isil_ed_fsm_t *fsm;
        int video_size = 0;

        jpeg_logic_chan = to_get_jpeg_channel_with_encode_control(jpeg_encode_control);
        phy_video_slot = jpeg_logic_chan->phy_video_slot;
        chip = jpeg_logic_chan->chip;
        chip_driver = chip->chip_driver;
        video_bus = &chip_driver->video_bus;
        vj_bus = video_bus->vj_bus_driver;
        resource_queue = &jpeg_logic_chan->capture_resource_queue;
        encode_property = &jpeg_logic_chan->encode_property;
        jpeg_frame_queue = &jpeg_logic_chan->jpeg_frame_queue;
        running_encode_property = &encode_property->running_encode_property;
		encode_request_tcb = &jpeg_logic_chan->encode_request_tcb;
        ed_tcb = &jpeg_logic_chan->opened_logic_chan_ed;
        fsm = &ed_tcb->ed_fsm;
        service_tcb = &encode_request_tcb->service_tcb;

        service_tcb->op->get_priv(service_tcb, (void *)&jpeg_capture_service_queue);

        if(fsm->op->get_curr_state(fsm) != ISIL_ED_STANDBY){
            ISIL_DBG(ISIL_DBG_ERR, "jpeg_logic_chan %d state %d error\n", jpeg_logic_chan->logic_chan_id, fsm->op->get_curr_state(fsm));
            jpeg_capture_service_queue->op->release_curr_consumer(jpeg_capture_service_queue);
            jpeg_capture_service_queue->op->trigger_pending_service_request(jpeg_capture_service_queue);

            return -EFAULT;
        }

        video_size = VIDEO_SIZE_FROM_WIDTH_HEIGHT(running_encode_property->i_image_width_mb_size<<4,
                        running_encode_property->i_image_height_mb_size<<4, video_bus->op->get_video_standard(video_bus));
        pool = &vj_bus->resource_pool[jpeg_capture_service_queue - vj_bus->jpeg_capture_service_queue];
        pool->op->try_get_resource_tcb(pool, &resource);
        frame = jpeg_frame_queue->curr_producer;
        if(resource){
            u32 value;
            
            driver_gen_deliver_event(ed_tcb, 1);
            if(resource->op->is_ready(resource) && (resource->op->get_video_size(resource) < ISIL_VIDEO_SIZE_VGA)){
                ISIL_DBG(ISIL_DBG_FATAL, "why this buffer is ready?\n");
                return ISIL_ERR;
            }

            resource->op->set_image_level(resource, running_encode_property->e_image_level);
            resource->op->set_video_size(resource, video_size);

            if(resource->op->get_video_size(resource) == ISIL_VIDEO_SIZE_USER){
                ISIL_DBG(ISIL_DBG_ERR, "width = %d, height = %d, norm = %d\n", running_encode_property->i_image_width_mb_size<<4,
                        running_encode_property->i_image_height_mb_size<<4, video_bus->op->get_video_standard(video_bus));
            }
            frame->i_mb_x = running_encode_property->i_image_width_mb_size;
	    frame->i_mb_y = running_encode_property->i_image_height_mb_size;
	    frame->frame_flags = running_encode_property->i_capture_type;

            if(resource->pool->pool_id == 0){
                value = chip->io_op->chip_read32(chip, VIDEO_JPEG_CMD_PATH0);
            }else{
                value = chip->io_op->chip_read32(chip, VIDEO_JPEG_CMD_PATH1);
            }
            if(value & 0x1){
                ISIL_DBG(ISIL_DBG_ERR, "channel %d capture cmd issue failed, hardware is busy, entry_number %d\n", resource->pool->pool_id, jpeg_capture_service_queue->op->get_queue_curr_entry_number(jpeg_capture_service_queue));
                return -EBUSY;
            }

            value = 0;
            value = (jpeg_logic_chan->phy_slot_id & 0xf) << 2;
            switch(resource->op->get_video_size(resource)){
                case ISIL_VIDEO_SIZE_D1:
                case ISIL_VIDEO_SIZE_4CIF:
                    value |= 0x0 << 6;
                    break;
                case ISIL_VIDEO_SIZE_HALF_D1:
                case ISIL_VIDEO_SIZE_2CIF:
                    value |= (0x0 << 6) | (1 << 9);
                    break;
                case ISIL_VIDEO_SIZE_CIF:
                    value |= (0x3 << 6) | (1 << 9);
                    break;
                default:
                    ISIL_DBG(ISIL_DBG_ERR, "unsupport this format %d\n", resource->op->get_video_size(resource));
            }
            /*start hardware capture*/

            value |= 0x1;
            if(resource->pool->pool_id == 0){
                chip->io_op->chip_write32(chip, VIDEO_JPEG_CMD_PATH0, value);
                chip->io_op->chip_write32(chip, JPEG_CAP_PATH0_ENABLE, 1);
            }else{
                chip->io_op->chip_write32(chip, VIDEO_JPEG_CMD_PATH1, value);
                chip->io_op->chip_write32(chip, JPEG_CAP_PATH1_ENABLE, 1);
            }
            //ISIL_DBG(ISIL_DBG_DEBUG, "ch %d capture pool %d, resource %d\n", jpeg_logic_chan->phy_slot_id, resource->pool->pool_id, resource->resource_id);
            resource_queue->op->put(resource_queue, resource);
            /*register complate notify*/
            resource->op->register_event_notify(resource, jpeg_encode_control->op->capture_notify, (void *)jpeg_encode_control);
            /*start polling check*/
            resource->op->start_polling_check(resource);
        }else{
            ISIL_DBG(ISIL_DBG_DEBUG, "why channel %d can't get resource from pool %d ? jpeg_frame_queue->curr_producer = %p\n", jpeg_logic_chan->phy_slot_id, pool->pool_id, jpeg_frame_queue->curr_producer);

            return ISIL_ERR;
        }

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_jpeg_encode_control_capture_notify(void *context)
{
    isil_jpeg_encode_control_t *jpeg_encode_control = (isil_jpeg_encode_control_t *)context;
    if(jpeg_encode_control){
        isil_jpeg_logic_encode_chan_t *jpeg_logic_chan;
        isil_chip_t *chip;
        chip_driver_t *chip_driver;
        isil_video_bus_t *video_bus;
        isil_vj_bus_t *vj_bus;
	jpeg_service_queue_t        *jpeg_capture_service_queue;
	tcb_node_t	*service_tcb;
	jpeg_service_tcb_t    *encode_request_tcb;
        isil_vj_resource_tcb_t *resource; 
        isil_vj_resource_queue_t *resource_queue; 
        ed_tcb_t *ed_tcb;
        isil_ed_fsm_t *ed_fsm;

        jpeg_logic_chan = to_get_jpeg_channel_with_encode_control(jpeg_encode_control);
        chip = jpeg_logic_chan->chip;
        chip_driver = chip->chip_driver;
        video_bus = &chip_driver->video_bus;
        vj_bus = video_bus->vj_bus_driver;
        resource_queue = &jpeg_logic_chan->capture_resource_queue;
        ed_tcb = &jpeg_logic_chan->opened_logic_chan_ed;
        ed_fsm = &ed_tcb->ed_fsm;

        encode_request_tcb = &jpeg_logic_chan->encode_request_tcb;
        service_tcb = &encode_request_tcb->service_tcb;
        service_tcb->op->get_priv(service_tcb, (void *)&jpeg_capture_service_queue);
        resource_queue->op->try_get_curr_consumer(resource_queue);
        resource = resource_queue->curr_consumer;

        if(ed_fsm->op->get_curr_state(ed_fsm) == ISIL_ED_RUNNING){
            if(resource){
                u32 value;

                resource->op->unregister_event_notify(resource);
                //ISIL_DBG(ISIL_DBG_DEBUG, "ch %d capture pool %d, resource %d complate\n", jpeg_logic_chan->phy_slot_id, resource->pool->pool_id, resource->resource_id);
                if(resource->pool->pool_id == 0){
                    value = chip->io_op->chip_read32(chip, JPEG_FRAME_VLD_0);
                }else{
                    value = chip->io_op->chip_read32(chip, JPEG_FRAME_VLD_1);
                }
                if(((value >> 4) & 0x3) != resource->resource_id){
                    ISIL_DBG(ISIL_DBG_ERR, "error hardware return %d, but we expect %d\n", ((value >> 4) & 0x3) , resource->resource_id);
                    return -EFAULT;
                }
                resource_queue->curr_consumer = NULL;
                resource_queue = &jpeg_logic_chan->encode_resource_queue;
                resource_queue->op->put(resource_queue, resource);
                //driver_gen_deliver_event(ed_tcb, 1);
                init_jpeg_service_tcb_with_start_encode(encode_request_tcb);
            }else{
                ISIL_DBG(ISIL_DBG_ERR, "resource == null, why ?");
            }
        }

        /*trigger next capture service*/
        jpeg_capture_service_queue->op->release_curr_consumer(jpeg_capture_service_queue);
        jpeg_capture_service_queue->op->trigger_pending_service_request(jpeg_capture_service_queue);

        return ISIL_OK;
    }

    return ISIL_OK;
}

static int isil_jpeg_encode_control_start_encode(void *context)
{
    isil_jpeg_encode_control_t *jpeg_encode_control = (isil_jpeg_encode_control_t *)context;
    if(jpeg_encode_control){
        isil_jpeg_logic_encode_chan_t *jpeg_logic_chan;
        isil_chip_t *chip;
        chip_driver_t *chip_driver;
        isil_video_bus_t *video_bus;
        isil_vj_bus_t *vj_bus;
        isil_vb_frame_tcb_queue_t *jpeg_frame_queue;
        isil_vb_frame_tcb_t       *frame;
        isil_vj_resource_tcb_t *resource; 
        isil_vj_resource_queue_t *resource_queue; 
        ed_tcb_t *ed_tcb;

        jpeg_logic_chan = to_get_jpeg_channel_with_encode_control(jpeg_encode_control);
        chip = jpeg_logic_chan->chip;
        chip_driver = chip->chip_driver;
        video_bus = &chip_driver->video_bus;
        vj_bus = video_bus->vj_bus_driver;
        resource_queue = &jpeg_logic_chan->encode_resource_queue;
        ed_tcb = &jpeg_logic_chan->opened_logic_chan_ed;
        jpeg_frame_queue = &jpeg_logic_chan->jpeg_frame_queue;
        frame = jpeg_frame_queue->curr_producer;

        resource_queue->op->try_get_curr_consumer(resource_queue);
        resource = resource_queue->curr_consumer;
        if(resource){
            u32 qp = 0;

            switch(resource->op->get_image_level(resource)){
                default:
                case MJPEG_IMAGE_LEVEL_0:
                    qp = 0x10;break;
                case MJPEG_IMAGE_LEVEL_1:
                    qp = 0x11;break;
                case MJPEG_IMAGE_LEVEL_2:
                    qp = 0x12;break;
                case MJPEG_IMAGE_LEVEL_3:
                    qp = 0x13;break;
                case MJPEG_IMAGE_LEVEL_4:
                    qp = 0x14;break;
            }

            //driver_gen_deliver_event(ed_tcb, 1);
            if(chip->io_op->chip_read32(chip, JPEG_ENCODE_CONTROL) & 0x1){
                ISIL_DBG(ISIL_DBG_ERR, "channel encode cmd issue failed, hardware is busy\n");
                return -EBUSY;
            }

            chip_request_irq(chip, IRQ_JPEG_TYPE_INTR, jpeg_encode_control->op->irq_encode_fun, "JPEG", jpeg_encode_control);

            if(video_bus->op->get_video_standard(video_bus) == ISIL_VIDEO_STANDARD_PAL){
                chip->io_op->chip_write32(chip, JPEG_VIDEO_FORMAT, 0);
            }else{
                chip->io_op->chip_write32(chip, JPEG_VIDEO_FORMAT, 1);
            }

            chip->io_op->chip_write32(chip, JPEG_ENCODE_QSCALE, qp);
            chip->io_op->chip_write32(chip, JPEG_ENCODE_CONTROL, (resource->pool->pool_id << 3) | ((resource->resource_id & 0x3) << 1) | 0x1);

            //ISIL_DBG(ISIL_DBG_DEBUG, "ch %d start encode pool %d, resource %d\n", jpeg_logic_chan->phy_slot_id, resource->pool->pool_id, resource->resource_id);
        }else{
            printk("%s, %d: resource == NULL\n", __FUNCTION__, __LINE__);
            return ISIL_ERR;
        }

       return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_jpeg_encode_control_start_encode_by_push(void *context)
{
    isil_jpeg_encode_control_t *jpeg_encode_control = (isil_jpeg_encode_control_t *)context;
    if(jpeg_encode_control){
        isil_jpeg_logic_encode_chan_t *jpeg_logic_chan;
        isil_chip_t *chip;
        chip_driver_t *chip_driver;
        isil_video_bus_t *video_bus;
        isil_vj_bus_t *vj_bus;
        isil_vb_frame_tcb_queue_t *jpeg_frame_queue;
        isil_vb_frame_tcb_t *frame;
        isil_vj_resource_tcb_t *resource; 
        isil_vj_resource_queue_t *resource_queue; 
        ed_tcb_t *ed_tcb;
        u32 val;

        jpeg_logic_chan = to_get_jpeg_channel_with_encode_control(jpeg_encode_control);
        chip = jpeg_logic_chan->chip;
        chip_driver = chip->chip_driver;
        video_bus = &chip_driver->video_bus;
        vj_bus = video_bus->vj_bus_driver;
        resource_queue = &jpeg_logic_chan->encode_resource_queue;
        ed_tcb = &jpeg_logic_chan->opened_logic_chan_ed;
        jpeg_frame_queue = &jpeg_logic_chan->jpeg_frame_queue;
        frame = jpeg_frame_queue->curr_producer;

        resource_queue->op->try_get_curr_consumer(resource_queue);
        resource = resource_queue->curr_consumer;
        if(resource){
            u32 qp = 0;

            switch(resource->op->get_image_level(resource)){
                default:
                case MJPEG_IMAGE_LEVEL_0:
                    qp = 0x10;break;
                case MJPEG_IMAGE_LEVEL_1:
                    qp = 0x11;break;
                case MJPEG_IMAGE_LEVEL_2:
                    qp = 0x12;break;
                case MJPEG_IMAGE_LEVEL_3:
                    qp = 0x13;break;
                case MJPEG_IMAGE_LEVEL_4:
                    qp = 0x14;break;
            }

            //driver_gen_deliver_event(ed_tcb, 1);
            if(chip->io_op->chip_read32(chip, JPEG_ENCODE_CONTROL) & 0x1){
                ISIL_DBG(ISIL_DBG_ERR, "channel encode cmd issue failed, hardware is busy\n");
                return -EBUSY;
            }

            chip->io_op->chip_write32(chip, JPEG_PCI_MASTER, 0x1);

            jpeg_logic_chan->push_buf = get_push_buf(BUF_TYPE_JPEG, 0, chip);
            if(jpeg_logic_chan->push_buf == NULL){
                ISIL_DBG(ISIL_DBG_ERR, "channel %d can't get jpeg push buffer\n", jpeg_logic_chan->phy_slot_id);
                return -EFAULT;
            }
            chip->io_op->chip_write32(chip, PCI_JPEG_BUFFER0_BASE_ADDR_REG + (((resource->pool->pool_id * resource->pool->entry_number) + resource->resource_id) << 2), virt_to_phys(jpeg_logic_chan->push_buf));

            val = chip->io_op->chip_read32(chip, PCI_MASTER_CONTROL);
            val |=  (1 << IRQ_EXT_PCI_MASTER) | (1 << IRQ_EXT_JPEG_INTR);
            chip->io_op->chip_write32(chip, PCI_MASTER_CONTROL, val);
            val = chip->io_op->chip_read32(chip, PCI_MASTER_INTR_ENABLE_REG);

            val = chip->io_op->chip_read32(chip, PCI_MASTER_INTR_ENABLE_REG);
            val |=  1 << IRQ_EXT_JPEG_INTR;
            chip->io_op->chip_write32(chip, PCI_MASTER_INTR_ENABLE_REG, val);
            chip->io_op->chip_write32(chip, PCI_JPEG_MAX_LEN, JPEG_BUF_SIZE - 4);

            chip_request_irq(chip, IRQ_JPEG_TYPE_INTR, jpeg_encode_control->op->irq_encode_fun, "JPEG", jpeg_encode_control);

            if(video_bus->op->get_video_standard(video_bus) == ISIL_VIDEO_STANDARD_PAL){
                chip->io_op->chip_write32(chip, JPEG_VIDEO_FORMAT, 0);
            }else{
                chip->io_op->chip_write32(chip, JPEG_VIDEO_FORMAT, 1);
            }

            chip->io_op->chip_write32(chip, JPEG_ENCODE_QSCALE, qp);
            chip->io_op->chip_write32(chip, JPEG_ENCODE_CONTROL, (resource->pool->pool_id << 3) | ((resource->resource_id & 0x3) << 1) | 0x1);

            //ISIL_DBG(ISIL_DBG_DEBUG, "ch %d start encode pool %d, resource %d\n", jpeg_logic_chan->phy_slot_id, resource->pool->pool_id, resource->resource_id);
        }else{
            printk("%s, %d: resource == NULL\n", __FUNCTION__, __LINE__);
            return ISIL_ERR;
        }

       return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_jpeg_encode_control_irq_encode_fun(int irq, void *context) 
{
    isil_jpeg_encode_control_t *jpeg_encode_control;

    jpeg_encode_control = (isil_jpeg_encode_control_t *)context;

    if(jpeg_encode_control){
        isil_jpeg_logic_encode_chan_t *jpeg_logic_chan;
        isil_chip_t *chip;
        chip_driver_t *chip_driver;
        isil_video_bus_t *video_bus;
        isil_vj_bus_t *vj_bus;
        isil_vj_resource_tcb_t *resource; 
        isil_vj_resource_queue_t *resource_queue; 
        ed_tcb_t *ed_tcb;
        isil_ed_fsm_t *ed_fsm;

        jpeg_logic_chan = to_get_jpeg_channel_with_encode_control(jpeg_encode_control);
        chip = jpeg_logic_chan->chip;
        chip_driver = chip->chip_driver;
        video_bus = &chip_driver->video_bus;
        vj_bus = video_bus->vj_bus_driver;

        ed_tcb = &jpeg_logic_chan->opened_logic_chan_ed;
        ed_fsm = &ed_tcb->ed_fsm;
        if(ed_fsm->op->get_curr_state(ed_fsm) != ISIL_ED_RUNNING){
            ISIL_DBG(ISIL_DBG_FATAL, "bad irq!\n");
            chip->io_op->chip_write32(chip, JPEG_IRQ, 0xff);
            return ISIL_ERR;
        }

        resource_queue = &jpeg_logic_chan->encode_resource_queue;
        resource_queue->op->try_get_curr_consumer(resource_queue);
        resource = resource_queue->curr_consumer;
        if(resource){
            /*move data*/
            jpeg_encode_control->op->move_data(jpeg_encode_control);
            /*clear irq*/
            chip->io_op->chip_write32(chip, JPEG_IRQ, 1 << (resource->pool->pool_id * resource->pool->entry_number + resource->resource_id));
        }else{
            printk("%s, %d: resource == NULL\n", __FUNCTION__, __LINE__);
        }

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_jpeg_encode_control_irq_encode_fun_by_push(int irq, void *context) 
{
    isil_jpeg_encode_control_t *jpeg_encode_control;

    jpeg_encode_control = (isil_jpeg_encode_control_t *)context;

    if(jpeg_encode_control){
        isil_jpeg_logic_encode_chan_t *jpeg_logic_chan;
        isil_chip_t *chip;
        chip_driver_t *chip_driver;
        isil_video_bus_t *video_bus;
        isil_vj_bus_t *vj_bus;
        isil_vj_resource_tcb_t *resource; 
        isil_vj_resource_queue_t *resource_queue; 
        ed_tcb_t *ed_tcb;
        isil_ed_fsm_t *ed_fsm;

        jpeg_logic_chan = to_get_jpeg_channel_with_encode_control(jpeg_encode_control);
        chip = jpeg_logic_chan->chip;
        chip_driver = chip->chip_driver;
        video_bus = &chip_driver->video_bus;
        vj_bus = video_bus->vj_bus_driver;

        ed_tcb = &jpeg_logic_chan->opened_logic_chan_ed;
        ed_fsm = &ed_tcb->ed_fsm;
        if(ed_fsm->op->get_curr_state(ed_fsm) != ISIL_ED_RUNNING){
            ISIL_DBG(ISIL_DBG_FATAL, "channel %d bad irq! fsm_curr_state = %d, irq flags %02x\n", jpeg_logic_chan->logic_chan_id, ed_fsm->op->get_curr_state(ed_fsm), chip->io_op->chip_read32(chip, JPEG_IRQ));
            chip->io_op->chip_write32(chip, JPEG_IRQ, 0xff);
            chip->io_op->chip_write32(chip, PCI_JPEG_BUF_INTR, 0xffff);
            chip->io_op->chip_write32(chip, INTERRUPT_FLAGS_EXT, 1 << IRQ_EXT_JPEG_INTR);
            return ISIL_ERR;
        }
        resource_queue = &jpeg_logic_chan->encode_resource_queue;
        resource_queue->op->try_get_curr_consumer(resource_queue);
        resource = resource_queue->curr_consumer;
        if(resource){
            /*move data*/
            jpeg_encode_control->op->move_data(jpeg_encode_control);
            /*clear irq*/
            chip->io_op->chip_write32(chip, JPEG_IRQ, 1 << (resource->pool->pool_id * resource->pool->entry_number + resource->resource_id));
            chip->io_op->chip_write32(chip, PCI_JPEG_BUF_INTR, 1 << (resource->pool->pool_id * resource->pool->entry_number + resource->resource_id));
            chip->io_op->chip_write32(chip, INTERRUPT_FLAGS_EXT, 1 << IRQ_EXT_JPEG_INTR);
        }else{
            ISIL_DBG(ISIL_DBG_FATAL, "resource == NULL\n");
            ISIL_DBG(ISIL_DBG_FATAL, "clear all jpeg irq flags!\n");
            chip->io_op->chip_write32(chip, JPEG_IRQ, 0xff);
            chip->io_op->chip_write32(chip, PCI_JPEG_BUF_INTR, 0xffff);
            chip->io_op->chip_write32(chip, INTERRUPT_FLAGS_EXT, 1 << IRQ_EXT_JPEG_INTR);
        }

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_jpeg_encode_control_move_data(isil_jpeg_encode_control_t *jpeg_encode_control)
{
    if(jpeg_encode_control){
        isil_jpeg_logic_encode_chan_t *jpeg_logic_chan;
        isil_chip_t *chip;
	dpram_control_t *chip_dpram_controller;
        dpram_request_t *read_video_bitstream_req;
        ed_tcb_t *ed_tcb;

        jpeg_logic_chan = to_get_jpeg_channel_with_encode_control(jpeg_encode_control);
        chip = jpeg_logic_chan->chip;
       
        ed_tcb = &jpeg_logic_chan->opened_logic_chan_ed;
        get_isil_dpram_controller(&chip_dpram_controller);
        read_video_bitstream_req = &jpeg_logic_chan->read_video_bitstream_req;
        driver_gen_deliver_event(ed_tcb, 1);
        init_submit_recv_jpeg_done_service(read_video_bitstream_req, jpeg_logic_chan);
        chip_dpram_controller->op->submit_read_mjpeg_req(chip_dpram_controller, jpeg_logic_chan);
         
        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_jpeg_encode_control_move_data_by_push(isil_jpeg_encode_control_t *jpeg_encode_control)
{
    if(jpeg_encode_control){
        isil_jpeg_logic_encode_chan_t *jpeg_logic_chan;
        isil_chip_t                 *chip;
        isil_vb_frame_tcb_queue_t   *frame_queue;
        isil_vb_frame_tcb_t         *frame;
        isil_vb_packet_tcb_t        *packet;
        isil_vb_packet_tcb_queue_t  *packet_queue;
        isil_timestamp_t            *timestamp;
        isil_vj_resource_tcb_t      *resource; 
        isil_vj_resource_queue_t    *resource_queue; 
        u32 header_len = 0, align = sizeof(int) - (sizeof(ext_h264_nal_bitstream_t) % 4);
        int encode_len = 0;
        ed_tcb_t *ed_tcb;

        jpeg_logic_chan = to_get_jpeg_channel_with_encode_control(jpeg_encode_control);
       
        ed_tcb = &jpeg_logic_chan->opened_logic_chan_ed;
        driver_gen_deliver_event(ed_tcb, 1);

        chip = jpeg_logic_chan->chip;
        timestamp = &jpeg_logic_chan->timestamp;
        frame_queue = &jpeg_logic_chan->jpeg_frame_queue;
        resource_queue = &jpeg_logic_chan->encode_resource_queue;
        resource_queue->op->try_get_curr_consumer(resource_queue);
        resource = resource_queue->curr_consumer;

        timestamp->op->set_timestamp(timestamp, resource->op->get_timestamp(resource), __FILE__);
        frame = frame_queue->curr_producer;
        frame->frame_type   = MJPEG_FRAME_TYPE;
        frame->frame_number = jpeg_encode_control->i_frame_serial;
        frame->timestamp    = timestamp->op->get_timestamp(timestamp);

        packet_queue = &frame->vb_packet_queue;
        packet_queue->op->try_get_curr_producer_from_pool(packet_queue, &jpeg_logic_chan->pool);
        packet = packet_queue->curr_producer;
        if ((packet == NULL) || (jpeg_logic_chan->push_buf == NULL)) {
            if(packet == NULL){
                ISIL_DBG(ISIL_DBG_ERR, "can't get packet\n");
            }else{
                ISIL_DBG(ISIL_DBG_ERR, "jpeg push_buf = null\n");
            }
            frame->frame_is_err = 1;
        }else{
            encode_len = resource->op->get_data_size(resource);
            if(encode_len > 0){
                header_len = isil_jifi_header(packet->data + sizeof(ext_h264_nal_bitstream_t) + align, frame);
                gen_ext_mjpeg_header(packet->data + align, encode_len + header_len);
                packet->consumer_len = align;
                packet->payload_len = header_len + sizeof(ext_h264_nal_bitstream_t) + align;
                packet->op->dma_map(packet, DMA_TO_DEVICE);
                jpeg_encode_control->total_frame_len = encode_len + header_len + sizeof(ext_h264_nal_bitstream_t) + align;
                jpeg_encode_control->frame_base_addr = 0; 
                jpeg_encode_control->have_recv_section_number = 0;
                jpeg_encode_control->section_len = packet->packet_size;
                jpeg_encode_control->section_number = (jpeg_encode_control->total_frame_len) / jpeg_encode_control->section_len;
                jpeg_encode_control->tailer_len = (jpeg_encode_control->total_frame_len) % jpeg_encode_control->section_len;

                frame->frame_len = jpeg_encode_control->total_frame_len;
                if(jpeg_encode_control->tailer_len){
                    jpeg_encode_control->section_number++;
                }
                jpeg_encode_control->trans_len = jpeg_encode_control->section_len - header_len - sizeof(ext_h264_nal_bitstream_t) - align;
                if(jpeg_encode_control->trans_len > encode_len){
                    jpeg_encode_control->trans_len = encode_len;
                }
                while(jpeg_encode_control->trans_len){
                    packet_queue->op->try_get_curr_producer_from_pool(packet_queue, &jpeg_logic_chan->pool);
                    packet = packet_queue->curr_producer;
                    if (packet == NULL) {
                        ISIL_DBG(ISIL_DBG_ERR, "can't get packet, frame->len = 0x%08x\n", jpeg_encode_control->total_frame_len);
                        frame->frame_is_err = 1;
                        break;
                    }else{
                        memcpy(packet->data + packet->payload_len, jpeg_logic_chan->push_buf + jpeg_encode_control->frame_base_addr, jpeg_encode_control->trans_len);
                        packet->payload_len += jpeg_encode_control->trans_len;
                        jpeg_encode_control->frame_base_addr += jpeg_encode_control->trans_len;
                        jpeg_encode_control->total_frame_len -= packet->payload_len;
                        jpeg_encode_control->trans_len = (jpeg_encode_control->total_frame_len > packet->packet_size) ? packet->packet_size:jpeg_encode_control->total_frame_len;
                        packet_queue->op->put_curr_producer_into_queue(packet_queue);
                    }
                }
            }else{
                frame->frame_is_err = 1;
                ISIL_DBG(ISIL_DBG_ERR, "jpeg frame len error\n");
            }
        }
        if (jpeg_logic_chan->push_buf) {
            release_push_buf(BUF_TYPE_JPEG, 0, jpeg_logic_chan->push_buf, chip);
            jpeg_logic_chan->push_buf = NULL;
        }

        jpeg_encode_control->op->notify(jpeg_encode_control);
        return ISIL_OK;
    }

    return ISIL_ERR;
}
static int isil_jpeg_encode_control_notify(isil_jpeg_encode_control_t *jpeg_encode_control)
{
    if(jpeg_encode_control){
        isil_jpeg_logic_encode_chan_t *jpeg_logic_chan;
        isil_chip_t *chip;
        chip_driver_t *chip_driver;
        isil_video_bus_t *video_bus;
        isil_vj_bus_t *vj_bus;
        jpeg_service_queue_t        *jpeg_encode_service_queue;
        isil_jpeg_encode_property_t *encode_property;
        isil_mjpeg_encode_param_t   *running_encode_property;
        isil_vj_resource_tcb_t *resource; 
        isil_vj_resource_queue_t *resource_queue; 
        ed_tcb_t *ed_tcb;
        isil_ed_fsm_t *fsm;
        isil_vb_frame_tcb_queue_t	*jpeg_frame_queue;
        u32 restart_time;
        isil_chip_timer_controller_t    *chip_timer_cont;

        jpeg_logic_chan = to_get_jpeg_channel_with_encode_control(jpeg_encode_control);
        chip = jpeg_logic_chan->chip;
        chip_driver = chip->chip_driver;
        video_bus = &chip_driver->video_bus;
        vj_bus = video_bus->vj_bus_driver;
        encode_property = &jpeg_logic_chan->encode_property;
        running_encode_property = &encode_property->running_encode_property;
        resource_queue = &jpeg_logic_chan->encode_resource_queue;
        jpeg_frame_queue = &jpeg_logic_chan->jpeg_frame_queue;
        ed_tcb = &jpeg_logic_chan->opened_logic_chan_ed;
        fsm = &ed_tcb->ed_fsm;
        chip_timer_cont = &chip->chip_timer_cont;


        vj_bus->i_temp_fps_counter++;
        resource_queue->op->try_get_curr_consumer(resource_queue);
        resource = resource_queue->curr_consumer;
        if(resource){
            //ISIL_DBG(ISIL_DBG_DEBUG, "ch %d encode pool %d, resource %d complate\n",  jpeg_logic_chan->phy_slot_id, resource->pool->pool_id, resource->resource_id);
            resource->op->release(resource);
            resource_queue->curr_consumer = NULL;
        }else{
            ISIL_DBG(ISIL_DBG_ERR, "why resource == NULL\n");
        }
        if(jpeg_frame_queue->curr_producer && (jpeg_frame_queue->curr_producer->frame_is_err == 0)){
            jpeg_encode_control->op->flush_curr_producer_frame(jpeg_encode_control);
            if(jpeg_frame_queue->curr_producer){
                jpeg_frame_queue->op->put_curr_producer_into_queue(jpeg_frame_queue);
                jpeg_encode_control->i_frame_serial++;
            }
            if(running_encode_property->i_capture_type & ISIL_MJPEG_ENCODE_PARAM_CAPTURE_TYPE_USER){
                if(running_encode_property->i_capture_frame_number > 0){
                    running_encode_property->i_capture_frame_number--;
                }else{
                    running_encode_property->i_capture_frame_number = 0;
                    running_encode_property->i_capture_type &= ~ISIL_MJPEG_ENCODE_PARAM_CAPTURE_TYPE_USER;
                }
            }
        }else{
            if(jpeg_frame_queue->curr_producer){
                ISIL_DBG(ISIL_DBG_ERR, "frame_is_err = %d, discard\n", jpeg_frame_queue->curr_producer->frame_is_err);
            }else{
                ISIL_DBG(ISIL_DBG_FATAL, "frame == NULL\n");
            }
            jpeg_frame_queue->op->release_curr_producer(jpeg_frame_queue, &jpeg_logic_chan->pool);
            jpeg_encode_control->lost_frame++;
        }

        driver_gen_deliver_event(ed_tcb, 1);

        if(fsm->op->get_curr_state(fsm) == ISIL_ED_IDLE){
            jpeg_encode_control->int_time = H264_MAX(jiffies_to_msecs(jiffies) , jiffies_to_msecs(jpeg_encode_control->last_jiffies)) - 
                H264_MIN(jiffies_to_msecs(jiffies) , jiffies_to_msecs(jpeg_encode_control->last_jiffies));
            jpeg_encode_control->last_jiffies = jiffies;
            if((running_encode_property->i_capture_frame_number > 0) || (running_encode_property->i_capture_type & ISIL_MJPEG_ENCODE_PARAM_CAPTURE_TYPE_TIMER)){
                if(jpeg_encode_control->int_time > running_encode_property->i_capture_frame_stride){
                    restart_time = 0;
                }else{
                    restart_time = running_encode_property->i_capture_frame_stride - jpeg_encode_control->int_time;
                }
                restart_time = restart_time / MS_PER_TICK;
                restart_time = restart_time?restart_time:1;
                jpeg_encode_control->schedule_time = chip_timer_cont->op->AddSingleFireTimerJob(chip_timer_cont, restart_time, isil_jpeg_schedule_restart, jpeg_encode_control);
                if(jpeg_encode_control->schedule_time == INVALIDTIMERID){
                    ISIL_DBG(ISIL_DBG_ERR, "add timer error\n");
                }
            }
        }

        /*trigger next encode service*/
        jpeg_encode_service_queue = &vj_bus->jpeg_encode_service_queue;
        jpeg_encode_service_queue->op->release_curr_consumer(jpeg_encode_service_queue);
        jpeg_encode_service_queue->op->trigger_pending_service_request(jpeg_encode_service_queue);

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static void isil_jpeg_encode_control_flush_all_frame(isil_jpeg_encode_control_t *jpeg_encode_control)
{
    jpeg_encode_control->b_flush_all_frame = ((1<<FLUSH_CURR_PRODUCER_FRAME)|(1<<FLUSH_CURR_CONSUMER_FRAME));
    jpeg_encode_control->op->flush_frame_queue(jpeg_encode_control);
}

static void isil_jpeg_encode_control_flush_curr_producer_frame(isil_jpeg_encode_control_t *jpeg_encode_control)
{
    if(jpeg_encode_control->b_flush_all_frame & (1<<FLUSH_CURR_PRODUCER_FRAME)){


        jpeg_encode_control->op->flush_frame_queue(jpeg_encode_control);
        jpeg_encode_control->b_flush_all_frame &= ~(1<<FLUSH_CURR_PRODUCER_FRAME);
    }
}

static void isil_jpeg_encode_control_flush_curr_consumer_frame(isil_jpeg_encode_control_t *jpeg_encode_control)
{
    if(jpeg_encode_control->b_flush_all_frame & (1<<FLUSH_CURR_CONSUMER_FRAME)){


        jpeg_encode_control->b_flush_all_frame &= ~(1<<FLUSH_CURR_CONSUMER_FRAME);
    }
}

static void isil_jpeg_encode_control_flush_frame_queue(isil_jpeg_encode_control_t *jpeg_encode_control)
{
    if(jpeg_encode_control){
        isil_jpeg_logic_encode_chan_t   *jpeg_logic_encode_chan;
        isil_vb_frame_tcb_queue_t    *video_frame_queue;
        isil_vb_pool_t      *video_chan_buf_pool;
        isil_vb_frame_tcb_t          *temp_frame;
        unsigned long   flags;

        jpeg_logic_encode_chan = to_get_isil_jpeg_encode_chan_with_encode_control(jpeg_encode_control);
        video_frame_queue = &jpeg_logic_encode_chan->jpeg_frame_queue;
        video_chan_buf_pool = &jpeg_logic_encode_chan->pool;
        spin_lock_irqsave(&video_frame_queue->lock, flags);
        while(video_frame_queue->op->get_curr_queue_entry_number(video_frame_queue)){
            video_frame_queue->op->try_get(video_frame_queue, &temp_frame);
            if(temp_frame != NULL){
                temp_frame->op->release(&temp_frame, video_chan_buf_pool);
            } else {
                break;
            }
        }
        spin_unlock_irqrestore(&video_frame_queue->lock, flags);
    }
}
static struct isil_jpeg_encode_control_operation isil_jpeg_encode_control_operation_op_by_ddr = {
    .init               = isil_jpeg_encode_control_init,
    .reset              = isil_jpeg_encode_control_reset,
    .release            = isil_jpeg_encode_control_release,
    .first_start        = isil_jpeg_encode_control_first_start,
    .start_capture      = isil_jpeg_encode_control_start_capture,
    .capture_notify     = isil_jpeg_encode_control_capture_notify,
    .start_encode       = isil_jpeg_encode_control_start_encode,
    .irq_encode_fun     = isil_jpeg_encode_control_irq_encode_fun,
    .move_data          = isil_jpeg_encode_control_move_data,
    .notify             = isil_jpeg_encode_control_notify,
    .flush_all_frame            = isil_jpeg_encode_control_flush_all_frame,
    .flush_curr_producer_frame  = isil_jpeg_encode_control_flush_curr_producer_frame,
    .flush_curr_consumer_frame  = isil_jpeg_encode_control_flush_curr_consumer_frame,
    .flush_frame_queue          = isil_jpeg_encode_control_flush_frame_queue,
};

static struct isil_jpeg_encode_control_operation isil_jpeg_encode_control_operation_op_by_pci_push = {
    .init               = isil_jpeg_encode_control_init,
    .reset              = isil_jpeg_encode_control_reset,
    .release            = isil_jpeg_encode_control_release,
    .first_start        = isil_jpeg_encode_control_first_start,
    .start_capture      = isil_jpeg_encode_control_start_capture,
    .capture_notify     = isil_jpeg_encode_control_capture_notify,
    .start_encode       = isil_jpeg_encode_control_start_encode_by_push,
    .irq_encode_fun     = isil_jpeg_encode_control_irq_encode_fun_by_push,
    .move_data          = isil_jpeg_encode_control_move_data_by_push,
    .notify             = isil_jpeg_encode_control_notify,
    .flush_all_frame            = isil_jpeg_encode_control_flush_all_frame,
    .flush_curr_producer_frame  = isil_jpeg_encode_control_flush_curr_producer_frame,
    .flush_curr_consumer_frame  = isil_jpeg_encode_control_flush_curr_consumer_frame,
    .flush_frame_queue          = isil_jpeg_encode_control_flush_frame_queue,
};

static int init_jpeg_encode_control(isil_jpeg_encode_control_t *jpeg_control)
{
    int ret = ISIL_ERR;

    if(jpeg_control){
        isil_jpeg_logic_encode_chan_t *jpeg_logic_chan;
        isil_chip_t *chip;

        jpeg_logic_chan = to_get_jpeg_channel_with_encode_control(jpeg_control);
        chip = jpeg_logic_chan->chip;
        if(chip->io_op->chip_read32(chip, 0x0) <= 0xe004){
            jpeg_control->op = &isil_jpeg_encode_control_operation_op_by_ddr;
        }else{
            jpeg_control->op = &isil_jpeg_encode_control_operation_op_by_pci_push;
        }
        ret = jpeg_control->op->init(jpeg_control);
        if(ret){
            ISIL_DBG(ISIL_DBG_ERR, "init jpeg control failed %d\n", ret);
            return ret;
        }
    }

    return ret;
}



static int jpeg_proc_read(struct seq_file *seq, void *data)
{
    isil_mjpeg_encode_param_t *jpeg_config;
    isil_vj_bus_t       *vj_bus;
    jpeg_service_queue_t *queue;
    isil_jpeg_logic_encode_chan_t *base, *jpeg;
    isil_chip_t *chip;
    ed_tcb_t	*opened_logic_chan_ed;
    isil_ed_fsm_t     *ed_fsm;
    isil_jpeg_encode_control_t    *encode_control;
    isil_vb_pool_t	*pool;
    isil_vb_frame_tcb_queue_t	*jpeg_frame_queue;
    isil_jpeg_encode_property_t *encode_property;
    isil_mjpeg_encode_param_t   *running_encode_property;
	
    chip = (isil_chip_t *)seq->private;
    base = jpeg = chip->jpeg_encode_chan;
    vj_bus = &jpeg->chip->chip_vj_bus;
    
    seq_printf(seq, "curr_fps: %8d\n", vj_bus->current_fps);
    seq_printf(seq, "capture overflow: %8d\n", vj_bus->overflow_times);
    seq_printf(seq, "capture queue entry: ");
    for(queue = vj_bus->jpeg_capture_service_queue; (queue - vj_bus->jpeg_capture_service_queue) < JPEG_MAX_RESOURCE_POOL; queue++){
        seq_printf(seq, "(%d, %8d) ", queue->op->get_queue_curr_entry_number(queue), atomic_read(&queue->service_count));
    }

    seq_printf(seq, "\n");
    queue = &vj_bus->jpeg_encode_service_queue;
    seq_printf(seq, "encode queue entry: (%d, %d) \n", queue->op->get_queue_curr_entry_number(queue), atomic_read(&queue->service_count));

    seq_printf(seq, "%8s %8s %8s %8s %8s %8s %8s", "ch", "phy", "logic", "width", "height", "Level", "interval\n");
    for(jpeg = base; (jpeg - base) < ISIL_PHY_VJ_CHAN_NUMBER; jpeg++)
    {
        jpeg_config = &jpeg->encode_property.running_encode_property;
        if(atomic_read(&jpeg->opened_flag) <= 0)
        {
            continue;
        }

        seq_printf(seq, "%8d %8d %8d %8d %8d %8d %8d\n",jpeg->phy_slot_id, 
                jpeg->phy_slot_id,
                jpeg->logic_chan_id,
                jpeg_config->i_image_width_mb_size,
                jpeg_config->i_image_height_mb_size,
                jpeg_config->e_image_level,
                jpeg_config->i_capture_frame_stride);
    }

    seq_printf(seq, "encoder status:\n");
    seq_printf(seq, "%8s %8s %8s %8s %8s %8s %8s %8s", "BlkCnt", "FrmCnt", "Seq", "int(ms)", "lost", "state", "left", "flags\n");
    for(jpeg = base; (jpeg - base) < ISIL_PHY_VJ_CHAN_NUMBER; jpeg++) {
        encode_control = &jpeg->encode_control;
        jpeg_frame_queue = &jpeg->jpeg_frame_queue;
        pool = &jpeg->pool;
        opened_logic_chan_ed = &jpeg->opened_logic_chan_ed;
        ed_fsm = &opened_logic_chan_ed->ed_fsm;
        encode_property = &jpeg->encode_property;
        running_encode_property = &encode_property->running_encode_property;
        if(atomic_read(&jpeg->opened_flag) <= 0)
        {
            continue;
        }
        seq_printf(seq, "%8d %8d %8d %8d %8d %8d %08x %08x\n", pool->op->get_packet_tcb_pool_entry_number(pool),
                jpeg_frame_queue->op->get_curr_queue_entry_number(jpeg_frame_queue),
                encode_control->i_frame_serial, encode_control->int_time, encode_control->lost_frame, 
                ed_fsm->op->get_curr_state(ed_fsm),running_encode_property->i_capture_frame_number, running_encode_property->i_capture_type
                );
    }

    return 0;
}

int isil_read_jpeg_yuv(isil_chip_t *chip, int page, int format, char *buf, int isB);
int isil_write_jpeg_yuv(isil_chip_t *chip, int page, int format, char *buf, int isB);
static ssize_t jpeg_proc_write(struct file *file, const char __user *buffer,
        size_t count, loff_t *offset)
{
    isil_chip_t *chip = (isil_chip_t *)isil_file_get_seq_private(file);
    isil_jpeg_logic_encode_chan_t *jpeg = chip->jpeg_encode_chan;
    char cmdbuf[128];
    char **argv;
    int argc, i;

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
    if(!jpeg){
        goto write_release;
    }

    if((argv[0] != NULL) && (argc >= 4) && (strcmp(argv[0], "cap") == 0))
    {
        jpeg += atoi(argv[2]);
        isil_jpeg_save_chn(jpeg, argv[1], atoi(argv[3]));
    }else if((argv[0] != NULL) && (strcmp(argv[0], "close") == 0)) {
        isil_jpeg_logic_encode_chan_t *base;

        for(base = jpeg; (base - jpeg) < ISIL_PHY_VD_CHAN_NUMBER; base++) {
            if(atomic_read(&base->opened_flag) == 0) {
                continue;
            }
            printk("close jpeg %d\n", base->logic_chan_id);
            base->opened_logic_chan_ed.op->close(&base->opened_logic_chan_ed);
        }
    } else if((argv[0] != NULL) && (strcmp(argv[0], "read") == 0)) {
        struct file *yuv_file;
        mm_segment_t fs; 
        char *buff;
        int size;

        //chip->io_op->chip_write32(chip, MODE, 0x5f);
        size = 0x97e00;
        buff = (char *)__get_free_pages(GFP_KERNEL, get_order(size));
        memset(buff, 0, size);
        yuv_file = isil_kernel_file_open(argv[2]);
        fs=get_fs();
        set_fs(KERNEL_DS);
        isil_read_jpeg_yuv(chip, atoi(argv[1]), 0, buff, 1);
        yuv_file->f_op->write(yuv_file, buff, size, &(yuv_file->f_pos));
        set_fs(fs);
        isil_kernel_file_close(yuv_file);
        free_pages((u32 )buff, get_order(size));

    } else if((argv[0] != NULL) && (strcmp(argv[0], "write") == 0)) {
        struct file *yuv_file;
        mm_segment_t fs; 
        char *buff;
        int size;

        //chip->io_op->chip_write32(chip, MODE, 0x5f);
        size = 0x97e00;
        buff = (char *)__get_free_pages(GFP_KERNEL, get_order(size));
        memset(buff, 0, size);
        yuv_file = isil_kernel_file_open(argv[2]);
        fs=get_fs();
        set_fs(KERNEL_DS);
        yuv_file->f_op->read(yuv_file, buff, size, &(yuv_file->f_pos));
        isil_write_jpeg_yuv(chip, atoi(argv[1]), 0, buff, 1);
        set_fs(fs);
        isil_kernel_file_close(yuv_file);
        free_pages((u32 )buff, get_order(size));
    } else{
    }

write_release:
    argv_free(argv);
    return count;
}

void    start_isil_jpeg_encode_chan_robust_processing_control_process(isil_jpeg_logic_encode_chan_t *jpeg_logic_encode_chan)
{
    robust_processing_control_t *robust_processing_control;

    robust_processing_control = &jpeg_logic_encode_chan->robust_processing_control;
    if(robust_processing_control->op){
        robust_processing_control->op->start_robust_processing(robust_processing_control);
    }
}

void    isil_jpeg_encode_chan_robust_processing_control_process_done(isil_jpeg_logic_encode_chan_t *jpeg_logic_encode_chan)
{
    robust_processing_control_t *robust_processing_control;
    robust_processing_control = &jpeg_logic_encode_chan->robust_processing_control;
    if(robust_processing_control->op){
        robust_processing_control->op->robust_processing_done(robust_processing_control);
    }
}

void    isil_jpeg_encode_chan_wait_robust_process_done(isil_jpeg_logic_encode_chan_t *jpeg_logic_encode_chan)
{
    robust_processing_control_t *robust_processing_control;
    robust_processing_control = &jpeg_logic_encode_chan->robust_processing_control;
    if(robust_processing_control->op){
        robust_processing_control->op->wait_robust_processing_done(robust_processing_control);
    }
}

int isil_jpeg_encode_chan_is_in_robust_processing(isil_jpeg_logic_encode_chan_t *jpeg_logic_encode_chan)
{
    robust_processing_control_t *robust_processing_control;
    robust_processing_control = &jpeg_logic_encode_chan->robust_processing_control;
    if(robust_processing_control->op){
        return robust_processing_control->op->is_in_robust_processing(robust_processing_control);
    }else{
        return 0;
    }
}

int init_isil_jpeg_encode_chan(isil_jpeg_logic_encode_chan_t *jpeg_logic, int bus_id, int chip_id, int phy_slot_id, isil_chip_t *chip)
{
    isil_vj_bus_t *chip_vj_bus;

    if(jpeg_logic && chip && (phy_slot_id < ISIL_PHY_VJ_CHAN_NUMBER))
    {
        ed_tcb_t *ed;
        isil_proc_register_s *jpeg_proc;
        isil_jpeg_phy_video_slot_t *phy_video_slot;
        robust_processing_control_t *robust_processing_control;

        chip_vj_bus = &chip->chip_vj_bus;
        jpeg_logic->phy_slot_id = phy_slot_id;
        jpeg_logic->logic_chan_id = (jpeg_logic - chip->jpeg_encode_chan);
        jpeg_logic->chip = chip;

        ed = &jpeg_logic->opened_logic_chan_ed;
        robust_processing_control = &jpeg_logic->robust_processing_control;
        init_robust_processing_control(robust_processing_control);
        jpeg_logic->push_buf = NULL; 
        init_isil_encode_timestamp(&jpeg_logic->timestamp);
        spin_lock_init(&jpeg_logic->fsm_matrix_call_lock);
        init_isil_send_msg_controller(&jpeg_logic->send_msg_contr, ISIL_MJPEG_ENCODE_CHAN, jpeg_logic->logic_chan_id, jpeg_logic);
        /*init encoder parameter*/
        init_jpeg_encode_property(&jpeg_logic->encode_property, &default_jpeg_encode_property);//use default parameter
        init_jpeg_encode_control(&jpeg_logic->encode_control);
        init_vj_resource_queue(&jpeg_logic->capture_resource_queue);
        init_vj_resource_queue(&jpeg_logic->encode_resource_queue);
        /*default single mode*/
        init_jpeg_service_tcb_with_null(&jpeg_logic->encode_request_tcb);
        /*node TCB*/
        init_endpoint_tcb(&jpeg_logic->logic_chan_ed, bus_id, chip_id, 
                ISIL_ED_VIDEO_ENCODE_IN, jpeg_logic->phy_slot_id, 
                jpeg_logic->logic_chan_id, jpeg_logic, jpeg_logic_encode_chan_driver_notify, 
                jpeg_logic_encode_chan_driver_match_id, &jpeg_encode_driver_fsm_state_transfer_matrix_table);
        init_endpoint_tcb(&jpeg_logic->opened_logic_chan_ed, bus_id, chip_id, 
                ISIL_ED_VIDEO_ENCODE_IN, jpeg_logic->phy_slot_id, 
                jpeg_logic->logic_chan_id, jpeg_logic, jpeg_logic_encode_chan_driver_notify, 
                jpeg_logic_encode_chan_driver_match_id, &jpeg_encode_driver_fsm_state_transfer_matrix_table);
        /*create jpeg buffer pool*/
        isil_create_pool(&jpeg_logic->pool, VIDEO_JPEG_BUF_POOL_LEN);
        isil_create_frame_tcb_queue(&jpeg_logic->jpeg_frame_queue);
        /*regist HCD operation*/
        ed->op = &jpeg_encode_hcd_interface_op;

        /*static allocation phy_slot_id*/
        phy_video_slot = &chip_vj_bus->phy_video_slot[phy_slot_id];
        phy_video_slot->op->register_logic_chan(phy_video_slot, jpeg_logic);
        jpeg_logic->phy_video_slot = phy_video_slot; 

        jpeg_proc = &chip->jpeg_proc;
        strcpy(jpeg_proc->name, "jpeg");
        jpeg_proc->read  = jpeg_proc_read;
        jpeg_proc->write = jpeg_proc_write;
        jpeg_proc->private = chip;
        isil_module_register(chip, jpeg_proc);

        return 0;
    }

    return -EINVAL;
}

void remove_isil_jpeg_encode_chan(isil_jpeg_logic_encode_chan_t *logic_chan)
{
    isil_chip_t	*chip;

    if(logic_chan){
        ed_tcb_t *ed;
        isil_jpeg_phy_video_slot_t *phy_video_slot;
	isil_vb_frame_tcb_queue_t	*jpeg_frame_queue;

        jpeg_frame_queue = &logic_chan->jpeg_frame_queue;
        ed = &logic_chan->opened_logic_chan_ed;
        chip = logic_chan->chip;
        if(ed->op != NULL){
            ed->op->close(&logic_chan->opened_logic_chan_ed);
        }

        phy_video_slot = logic_chan->phy_video_slot;
        if(phy_video_slot){
            phy_video_slot->op->unregister_logic_chan(phy_video_slot, logic_chan);
            logic_chan->phy_video_slot = NULL; 
        }

        jpeg_frame_queue->op->release(jpeg_frame_queue, &logic_chan->pool);
        isil_destroy_pool(&logic_chan->pool);
        isil_module_unregister(chip, &chip->jpeg_proc);
        ISIL_DBG(ISIL_DBG_INFO, "remove channel %d complete\n", logic_chan->phy_slot_id);
    }
}

void    reset_isil_jpeg_encode_chan_preprocessing(isil_jpeg_logic_encode_chan_t *jpeg_logic_encode_chan)
{
    if(jpeg_logic_encode_chan){
        ed_tcb_t *opened_logic_chan_ed;
        isil_ed_fsm_t   *ed_fsm;

        ISIL_DBG(ISIL_DBG_DEBUG, "%d\n", jpeg_logic_encode_chan->logic_chan_id);
        if(isil_jpeg_encode_chan_is_in_robust_processing(jpeg_logic_encode_chan)){

        }else{
            start_isil_jpeg_encode_chan_robust_processing_control_process(jpeg_logic_encode_chan);
        }

        opened_logic_chan_ed = &jpeg_logic_encode_chan->opened_logic_chan_ed;
        ed_fsm = &opened_logic_chan_ed->ed_fsm;
        ed_fsm->op->change_state_for_robust(ed_fsm);
        opened_logic_chan_ed->op->suspend(opened_logic_chan_ed);
        driver_gen_close_event(opened_logic_chan_ed, 1);
    }
}

void    reset_isil_jpeg_encode_chan_postprocessing(isil_jpeg_logic_encode_chan_t *jpeg_logic_encode_chan)
{
    if(jpeg_logic_encode_chan){
        ed_tcb_t *opened_logic_chan_ed;

        isil_jpeg_encode_chan_robust_processing_control_process_done(jpeg_logic_encode_chan);

        if(atomic_read(&jpeg_logic_encode_chan->opened_flag) == 0){
            isil_chip_t *chip;

            chip = jpeg_logic_encode_chan->chip;
            chip->chip_open(chip);
        }
        opened_logic_chan_ed = &jpeg_logic_encode_chan->opened_logic_chan_ed;
        driver_gen_open_event(opened_logic_chan_ed, 1);
        driver_gen_resume_event(opened_logic_chan_ed, 1);
    }
}

void    reset_isil_jpeg_encode_chan_processing(isil_jpeg_logic_encode_chan_t *jpeg_logic_encode_chan)
{
    if(jpeg_logic_encode_chan){
        ed_tcb_t *opened_logic_chan_ed;
        isil_jpeg_encode_control_t  *jpeg_encode_control;
        isil_chip_t *chip;
        chip_driver_t *chip_driver;
        isil_video_bus_t *video_bus;
        isil_vj_bus_t *vj_bus;
        isil_vj_resource_tcb_t *resource; 
        isil_vj_resource_queue_t *resource_queue; 
 

        ISIL_DBG(ISIL_DBG_DEBUG, "%d\n", jpeg_logic_encode_chan->logic_chan_id);
        jpeg_encode_control = &jpeg_logic_encode_chan->encode_control;
        jpeg_encode_control->op->reset(jpeg_encode_control);
        chip =  jpeg_logic_encode_chan->chip;
        chip_driver = chip->chip_driver;
        video_bus = &chip_driver->video_bus;
        vj_bus = video_bus->vj_bus_driver;
        opened_logic_chan_ed = &jpeg_logic_encode_chan->opened_logic_chan_ed;

        resource_queue = &jpeg_logic_encode_chan->capture_resource_queue;
        resource_queue->op->try_get_curr_consumer(resource_queue);
        resource = resource_queue->curr_consumer;
        if(resource){
            ISIL_DBG(ISIL_DBG_ERR, "ch %d capture pool %d, resource %d timeout\n", jpeg_logic_encode_chan->phy_slot_id, resource->pool->pool_id, resource->resource_id);
            resource->op->stop_polling_check(resource);
            resource->op->unregister_event_notify(resource);
            resource->op->release(resource);
            resource_queue->curr_consumer = NULL;
        }
        resource_queue = &jpeg_logic_encode_chan->encode_resource_queue;
        resource_queue->op->try_get_curr_consumer(resource_queue);
        resource = resource_queue->curr_consumer;
        if(resource){
            ISIL_DBG(ISIL_DBG_ERR, "ch %d encode pool %d, resource %d timeout\n", jpeg_logic_encode_chan->phy_slot_id, resource->pool->pool_id, resource->resource_id);
            resource->op->release(resource);
            resource_queue->curr_consumer = NULL;
        }
        if(jpeg_logic_encode_chan->push_buf){
            release_push_buf(BUF_TYPE_JPEG, 0, jpeg_logic_encode_chan->push_buf, chip);
            jpeg_logic_encode_chan->push_buf = NULL;
        }

        if(atomic_read(&jpeg_logic_encode_chan->opened_flag) == 0){
            chip->chip_close(chip);
        }
    }
}

void isil_chip_jpeg_start_robust_processing(isil_vj_bus_t *vj_bus)
{
    if(vj_bus){
        robust_processing_control_t *robust_processing_control;

        robust_processing_control = &vj_bus->robust_processing_control;
        robust_processing_control->op->start_robust_processing(robust_processing_control);
    }
}

void isil_chip_jpeg_wait_robust_processing_done(isil_vj_bus_t *vj_bus)
{
    if(vj_bus){
        robust_processing_control_t *robust_processing_control;

        robust_processing_control = &vj_bus->robust_processing_control;
        robust_processing_control->op->wait_robust_processing_done(robust_processing_control);
    }
}

int  isil_chip_jpeg_is_in_robust_processing(isil_vj_bus_t *vj_bus)
{
    if(vj_bus){
        robust_processing_control_t *robust_processing_control;

        robust_processing_control = &vj_bus->robust_processing_control;
        return robust_processing_control->op->is_in_robust_processing(robust_processing_control);
    }

    return ISIL_FALSE;
}

void isil_chip_jpeg_robust_process_done(isil_vj_bus_t *vj_bus)
{
    if(vj_bus){
        robust_processing_control_t *robust_processing_control;

        robust_processing_control = &vj_bus->robust_processing_control;
        robust_processing_control->op->robust_processing_done(robust_processing_control);
    }
}

static int isil_vj_resource_queue_init(isil_vj_resource_queue_t *queue)
{
    if(queue){
        tcb_node_queue_t *node_queue;

        node_queue = &queue->queue;
        node_queue->op = &tcb_node_queue_op;
        node_queue->op->init(node_queue);

        queue->curr_consumer = NULL;
        queue->curr_producer = NULL;

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_vj_resource_queue_reset(isil_vj_resource_queue_t *queue)
{
    if(queue){
        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_vj_resource_queue_release(isil_vj_resource_queue_t *queue)
{
    if(queue){
        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_vj_resource_queue_get(isil_vj_resource_queue_t *queue, isil_vj_resource_tcb_t **resource)
{
    if(queue && resource){
        tcb_node_queue_t *node_queue;
        tcb_node_t       *node;

        node_queue = &queue->queue;
        node_queue->op->get(node_queue, &node);
        if(node == NULL){
            *resource = NULL;
        }else{
            *resource = to_get_vj_resource_tcb_with_node(node);
        }

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_vj_resource_queue_try_get(isil_vj_resource_queue_t *queue, isil_vj_resource_tcb_t **resource)
{
    if(queue && resource){
        tcb_node_queue_t *node_queue;
        tcb_node_t       *node;

        node_queue = &queue->queue;
        node_queue->op->try_get(node_queue, &node);
        if(node == NULL){
            *resource = NULL;
        }else{
            *resource = to_get_vj_resource_tcb_with_node(node);
        }

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_vj_resource_queue_try_get_curr_consumer(isil_vj_resource_queue_t *queue)
{
    if(queue && (queue->curr_consumer == NULL)){
        tcb_node_queue_t *node_queue;
        tcb_node_t       *node;

        node_queue = &queue->queue;
        node_queue->op->try_get(node_queue, &node);
        if(node == NULL){
            queue->curr_consumer = NULL;
        }else{
            queue->curr_consumer = to_get_vj_resource_tcb_with_node(node);
        }

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_vj_resource_queue_put(isil_vj_resource_queue_t *queue, isil_vj_resource_tcb_t *resource)
{
    if(queue && resource){
        tcb_node_queue_t *node_queue;
        tcb_node_t       *node;

        node_queue = &queue->queue;
        node = &resource->node;
        node_queue->op->put(node_queue, node);

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static u32 isil_vj_resource_queue_get_curr_entry_number(isil_vj_resource_queue_t *queue)
{
    if(queue){
        tcb_node_queue_t *node_queue;
        u32 entry_number;

        node_queue = &queue->queue;
        entry_number = node_queue->op->get_queue_curr_entry_number(node_queue);

        return entry_number;
    }

    return 0;
}


static struct isil_vj_resource_queue_operation isil_vj_resource_queue_operation_op = {
    .init                   = isil_vj_resource_queue_init,
    .reset                  = isil_vj_resource_queue_reset,
    .release                = isil_vj_resource_queue_release,

    .get                    = isil_vj_resource_queue_get,
    .try_get                = isil_vj_resource_queue_try_get,
    .try_get_curr_consumer= isil_vj_resource_queue_try_get_curr_consumer,
    .put                    = isil_vj_resource_queue_put,
    .get_curr_entry_number  = isil_vj_resource_queue_get_curr_entry_number,
};

static int init_vj_resource_queue(isil_vj_resource_queue_t *queue)
{
    int ret = ISIL_ERR;

    if(queue){
        queue->op = &isil_vj_resource_queue_operation_op;
        ret = queue->op->init(queue);
        if(ret){
            ISIL_DBG(ISIL_DBG_ERR, "init resource queue failed %d\n", ret);
            return ret;
        }
        ret = ISIL_OK;
    }

    return ret;
}

static int isil_vj_resource_tcb_init(isil_vj_resource_tcb_t *resource)
{
    if(resource){
        tcb_node_t *node;

        node = &resource->node;
        node->op = &tcb_node_op;
        node->op->init(node);

        resource->timer_id = INVALIDTIMERID;
        resource->video_size = ISIL_VIDEO_SIZE_USER;
        resource->image_level = MJPEG_IMAGE_LEVEL_0;
        resource->timestamp = 0;
        resource->data_size = 0;
        
        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_vj_resource_tcb_reset(isil_vj_resource_tcb_t *resource)
{
    if(resource){
        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_vj_resource_tcb_release(isil_vj_resource_tcb_t *resource)
{
    if(resource){
        tcb_node_pool_t *node_pool;
        tcb_node_t *node;

        //resource->timer_id = INVALIDTIMERID;
        resource->video_size = ISIL_VIDEO_SIZE_USER;
        resource->image_level = MJPEG_IMAGE_LEVEL_0;
        resource->timestamp = 0;
        resource->data_size = 0;

        resource->callback = NULL;
        resource->context = NULL;

        node = &resource->node;
        node_pool = &resource->pool->pool;
        node->op->release(node, node_pool);

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_vj_resource_tcb_reference(isil_vj_resource_tcb_t *resource, isil_vj_resource_tcb_t **target)
{
    if(resource && target){
        return ISIL_OK;
    }

    return ISIL_ERR;
}

static u32 isil_vj_resource_tcb_get_video_size(isil_vj_resource_tcb_t *resource)
{
    if(resource){

        return resource->video_size;
    }

    return ISIL_ERR;
}

static int isil_vj_resource_tcb_set_video_size(isil_vj_resource_tcb_t *resource, enum ISIL_VIDEO_SIZE video_size)
{
    if(resource){
        resource->video_size = video_size;

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static u32 isil_vj_resource_tcb_get_image_level(isil_vj_resource_tcb_t *resource)
{
    if(resource){
        return resource->image_level;
    }

    return ISIL_ERR;
}

static int isil_vj_resource_tcb_set_image_level(isil_vj_resource_tcb_t *resource, enum ISIL_MJPEG_IMAGE_LEVEL_E image_level)
{
    if(resource){
        resource->image_level = image_level;

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static u32 isil_vj_resource_tcb_get_ddr_phy_addr(isil_vj_resource_tcb_t *resource)
{
    if(resource){
        isil_vj_resource_pool_t *pool;
        isil_vj_bus_t *vj_bus;
        isil_chip_t *chip;
        u32 page_id, ddr_phy_addr;

        pool = resource->pool;
        chip = pool->chip;
        vj_bus = &chip->chip_vj_bus;
#if 0
        page_id = vj_bus->op->get_ddr_page_base(vj_bus) + ((pool->entry_number * pool->pool_id + resource->resource_id) << 1);
#else
        switch(pool->entry_number * pool->pool_id + resource->resource_id)
        {
            default:
            case 0:page_id = 0x09;break;
            case 1:page_id = 0x0b;break;
            case 2:page_id = 0x19;break;
            case 3:page_id = 0x1b;break;
            case 4:page_id = 0x29;break;
            case 5:page_id = 0x2b;break;
            case 6:page_id = 0x39;break;
            case 7:page_id = 0x3b;break;
        }

#endif
        ddr_phy_addr = (page_id << 19) + (VIDEO_JPEG_DDR_BASE << 2) - 0x80000;

        return ddr_phy_addr;
    }

    return 0;
}

static u32 isil_vj_resource_tcb_get_data_size(isil_vj_resource_tcb_t *resource)
{
    if(resource){
        isil_vj_resource_pool_t *pool;
        isil_chip_t *chip;
        u32 datalen;

        pool = resource->pool;
        chip = pool->chip;

        datalen = chip->io_op->chip_read32(chip, JPEG_LENGTH(pool->entry_number * pool->pool_id + resource->resource_id));

        return datalen; 
    }

    return ISIL_ERR;
}

static u32 isil_vj_resource_tcb_get_timestamp(isil_vj_resource_tcb_t *resource)
{
    if(resource){
        isil_vj_resource_pool_t *pool;
        isil_chip_t *chip;
        u32 timestamp, shift;

        pool = resource->pool;
        chip = pool->chip;

        timestamp = chip->io_op->chip_read32(chip, JPEG_TIMESTAMP(pool->entry_number * pool->pool_id + resource->resource_id));
        if(resource->resource_id & 0x1){
            shift = 16;
        }else{
            shift = 0;
        }
        timestamp >>= shift;
        timestamp &= 0xffff;

        return timestamp;
    }

    return ISIL_ERR;
}

static int isil_vj_resource_tcb_is_ready(isil_vj_resource_tcb_t *resource)
{
    if(resource){
        /*check ready*/
        isil_vj_resource_pool_t *pool;
        isil_chip_t *chip;
        u32 value, line, height;

        pool = resource->pool;
        chip = pool->chip; 

        resource->timer_id = INVALIDTIMERID;
        if(pool->pool_id == 0){
            value = chip->io_op->chip_read32(chip, JPEG_FRAME_VLD_0);
            line = chip->io_op->chip_read32(chip, 0xd084);
        }else{
            value = chip->io_op->chip_read32(chip, JPEG_FRAME_VLD_1);
            line = chip->io_op->chip_read32(chip, 0xd084);
        }
        if(value & (1 << resource->resource_id)){
            value = chip->io_op->chip_read32(chip, JPEG_DBG_OVERFLOW);
            if(value & 0x1){
                printk("%s, %d: jpeg module error, buffer overflow!\n", __FUNCTION__, __LINE__);
                chip->chip_vj_bus.overflow_times++;
                chip->io_op->chip_write32(chip, JPEG_DBG_OVERFLOW, value);
            }
			height = VIDEO_SIZE_TO_HEIGHT(resource->op->get_video_size(resource), 
			chip->chip_driver->video_bus.op->get_video_standard(&chip->chip_driver->video_bus));
            if((line < (height - 16)) || (line > (height + 16))){
                ISIL_DBG(ISIL_DBG_ERR, "line error %d, should be %d\n", line, height);
            }
            return ISIL_TRUE;
        }else{
            return ISIL_FALSE;
        }
    }

    return ISIL_ERR;
}


static int isil_vj_resource_tcb_register_event_notify(isil_vj_resource_tcb_t *resource, int (*fun)(void *), void *context)
{
    if(resource){

        if(!resource->callback){
            resource->callback =  fun;
            resource->context = context;
        }

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_vj_resource_tcb_unregister_event_notify(isil_vj_resource_tcb_t *resource)
{
    if(resource){
        resource->callback = NULL;
        resource->context = NULL;

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_vj_resource_tcb_start_polling_check(isil_vj_resource_tcb_t *resource)
{
    if(resource){
        /*check ready*/
        isil_vj_resource_pool_t *pool;
        isil_chip_t *chip;
        isil_chip_timer_controller_t    *chip_timer_cont;
        u32 value, value1;

        pool = resource->pool;
        chip = pool->chip;
        chip_timer_cont = &chip->chip_timer_cont;

        resource->timer_id = INVALIDTIMERID;
        if(pool->pool_id == 0){
            value = chip->io_op->chip_read32(chip, JPEG_FRAME_VLD_0);
            value1 = chip->io_op->chip_read32(chip, VIDEO_JPEG_CMD_PATH0);
        }else{
            value = chip->io_op->chip_read32(chip, JPEG_FRAME_VLD_1);
            value1 = chip->io_op->chip_read32(chip, VIDEO_JPEG_CMD_PATH1);
        }
        if((value & (1 << resource->resource_id)) && (!(value1 & 0x1))){
        //if((value & (1 << resource->resource_id))){
            //ISIL_DBG(ISIL_DBG_DEBUG, "resource %d ready\n", resource->resource_id);
            if(resource->callback){
                if(resource->callback(resource->context) == -EFAULT){
                    resource->op->release(resource);
                }
            }else{
                ISIL_DBG(ISIL_DBG_DEBUG, "nobody care this event, discard\n");
            }
            resource->op->unregister_event_notify(resource);
            return ISIL_OK;
        }

        resource->timer_id = chip_timer_cont->op->AddSingleFireTimerJob(chip_timer_cont, TIMER_5MS, (timerHook )resource->op->start_polling_check, resource);
        if(resource->timer_id == INVALIDTIMERID){
            ISIL_DBG(ISIL_DBG_FATAL, "add timer failed\n");
        }

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_vj_resource_tcb_stop_polling_check(isil_vj_resource_tcb_t *resource)
{
    if(resource){
        isil_vj_resource_pool_t *pool;
        isil_chip_t *chip;
        isil_chip_timer_controller_t    *chip_timer_cont;
 
        pool = resource->pool;
        chip = pool->chip;
        chip_timer_cont = &chip->chip_timer_cont;

        if(resource->timer_id != INVALIDTIMERID){
            chip_timer_cont->op->DeleteSingleFireTimerJob(chip_timer_cont, resource->timer_id);
        }

        return ISIL_OK;
    }

    return ISIL_ERR;
}
static struct isil_vj_resource_tcb_operation isil_vj_resource_tcb_op = {
    .init               = isil_vj_resource_tcb_init,
    .reset              = isil_vj_resource_tcb_reset,
    .release            = isil_vj_resource_tcb_release,

    .reference          = isil_vj_resource_tcb_reference,

    .get_video_size     = isil_vj_resource_tcb_get_video_size,
    .set_video_size     = isil_vj_resource_tcb_set_video_size,
    .get_image_level    = isil_vj_resource_tcb_get_image_level,
    .set_image_level    = isil_vj_resource_tcb_set_image_level,
    .get_ddr_phy_addr   = isil_vj_resource_tcb_get_ddr_phy_addr,
    .get_data_size      = isil_vj_resource_tcb_get_data_size,
    .get_timestamp      = isil_vj_resource_tcb_get_timestamp,
    .is_ready           = isil_vj_resource_tcb_is_ready,
    .register_event_notify = isil_vj_resource_tcb_register_event_notify,
    .unregister_event_notify = isil_vj_resource_tcb_unregister_event_notify,
    .start_polling_check = isil_vj_resource_tcb_start_polling_check,
    .stop_polling_check = isil_vj_resource_tcb_stop_polling_check,
};

static int isil_vj_resource_pool_init(isil_vj_resource_pool_t *pool)
{
    int ret = ISIL_ERR;

    if(pool){
        if(pool->entry_number <= JPEG_MAX_RESOURCE_SIZE){
            isil_chip_t *chip;
            tcb_node_pool_t  *node_pool;
            isil_vj_resource_tcb_t *resource_tcb_pool;

            chip = pool->chip;

            if(pool->pool_id == 0){
                pool->rd = pool->wr = (chip->io_op->chip_read32(chip, JPEG_FRAME_VLD_0) >> 4) & 0x3;
            }else{
                pool->rd = pool->wr = (chip->io_op->chip_read32(chip, JPEG_FRAME_VLD_1) >> 4) & 0x3;
            }

            spin_lock_init(&pool->lock);
            node_pool = &pool->pool;
            node_pool->op = &tcb_node_pool_op;
            node_pool->op->init(node_pool, pool->entry_number);
            
            pool->resource_tcb_pool = (isil_vj_resource_tcb_t *)kmalloc(sizeof(isil_vj_resource_tcb_t) * pool->entry_number, GFP_KERNEL);
            if(pool->resource_tcb_pool == NULL){
                ISIL_DBG(ISIL_DBG_ERR, "no memory for resorce_tcb_pool\n");
                return -ENOMEM;
            }
            for(resource_tcb_pool = pool->resource_tcb_pool; (resource_tcb_pool - pool->resource_tcb_pool) < pool->entry_number; resource_tcb_pool++){
                resource_tcb_pool->resource_id = (resource_tcb_pool - pool->resource_tcb_pool);
                resource_tcb_pool->pool = pool;
                resource_tcb_pool->op = &isil_vj_resource_tcb_op;
                ret = resource_tcb_pool->op->init(resource_tcb_pool);
                if(ret){
                    ISIL_DBG(ISIL_DBG_ERR, "init resource_tcb failed %d\n", ret);
                    break;
                }
                resource_tcb_pool->op->release(resource_tcb_pool);
            }
            if(ret){
                for(--resource_tcb_pool; (resource_tcb_pool - pool->resource_tcb_pool) >= 0; resource_tcb_pool++){
                    if(resource_tcb_pool->op){
                        resource_tcb_pool->op->release(resource_tcb_pool);
                    }
                }
                if(pool->resource_tcb_pool){
                    kfree(pool->resource_tcb_pool);
                }
            }else{
                ret = ISIL_OK;
            }
        }
    }

    return ret;
}

static int isil_vj_resource_pool_reset(isil_vj_resource_pool_t *pool)
{
    if(pool){
        isil_chip_t *chip;

        chip = pool->chip;

        if(pool->pool_id == 0){
            pool->rd = pool->wr = (chip->io_op->chip_read32(chip, JPEG_FRAME_VLD_0) >> 4) & 0x3;
        }else{
            pool->rd = pool->wr = (chip->io_op->chip_read32(chip, JPEG_FRAME_VLD_1) >> 4) & 0x3;
        }
        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_vj_resource_pool_release(isil_vj_resource_pool_t *pool)
{
    if(pool){
        if(pool->entry_number <= JPEG_MAX_RESOURCE_SIZE){
            isil_vj_resource_tcb_t *resource_tcb_pool;

            if(pool->resource_tcb_pool){
                for(resource_tcb_pool = pool->resource_tcb_pool; (resource_tcb_pool - pool->resource_tcb_pool) < pool->entry_number; resource_tcb_pool++){
                    resource_tcb_pool->op->release(resource_tcb_pool);
                }
                kfree(pool->resource_tcb_pool);
                pool->resource_tcb_pool = NULL;
            }
        }
 
        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_vj_resource_pool_try_get_resource_tcb(isil_vj_resource_pool_t *pool, isil_vj_resource_tcb_t **resource)
{
    if(pool && resource){
        isil_vj_resource_tcb_t *tmp;
        tcb_node_pool_t *node_pool;
        tcb_node_t *node;
        unsigned long flags;
        int i;

        *resource = NULL;
        spin_lock_irqsave(&pool->lock, flags);
        node_pool = &pool->pool;
        for(i = 0; i < pool->entry_number; i++){
            node_pool->op->try_get(node_pool, &node);
            tmp = to_get_vj_resource_tcb_with_node(node);
            if(node && (tmp->resource_id == pool->wr)){
                pool->wr++;
                if(pool->wr >= JPEG_MAX_RESOURCE_SIZE){
                    pool->wr = 0;
                }
                *resource = tmp;
                break;
            }else{
                node_pool->op->put(node_pool, node);
            }
        }
        if(*resource == NULL){
            ISIL_DBG(ISIL_DBG_ERR, "get resource %d from pool %d failed\n", pool->wr, pool->pool_id);
        }
        spin_unlock_irqrestore(&pool->lock, flags);

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_vj_resource_pool_get_resource_tcb(isil_vj_resource_pool_t *pool, isil_vj_resource_tcb_t **resource)
{
    if(pool && resource){
        tcb_node_pool_t *node_pool;
        tcb_node_t *node;

        node_pool = &pool->pool;
        node_pool->op->get(node_pool, &node);
        if(node){
            *resource = to_get_vj_resource_tcb_with_node(node);
        }else{
            *resource = NULL;
        }
        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_vj_resource_pool_put_resource_tcb(isil_vj_resource_pool_t *pool, isil_vj_resource_tcb_t *resource)
{
    if(pool){
        tcb_node_pool_t *node_pool;
        tcb_node_t *node;

        node = &resource->node;
        node_pool = &pool->pool;
        node_pool->op->put(node_pool, node);
        
        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_vj_resource_pool_get_curr_entry_number(isil_vj_resource_pool_t *pool)
{
    if(pool){
        
        tcb_node_pool_t *node_pool;

        node_pool = &pool->pool;

        return node_pool->op->get_curr_pool_entry_number(node_pool);
    }

    return ISIL_ERR;
}

static struct isil_vj_resource_pool_operation isil_vj_resource_pool_operation_op = {
    .init                       = isil_vj_resource_pool_init,
    .reset                      = isil_vj_resource_pool_reset,
    .release                    = isil_vj_resource_pool_release,

    .try_get_resource_tcb       = isil_vj_resource_pool_try_get_resource_tcb,
    .get_resource_tcb           = isil_vj_resource_pool_get_resource_tcb,
    .put_resource_tcb           = isil_vj_resource_pool_put_resource_tcb,

    .get_curr_entry_number   = isil_vj_resource_pool_get_curr_entry_number,
};

static int isil_jpeg_phy_video_slot_init(isil_jpeg_phy_video_slot_t *phy_video_slot)
{
    if(phy_video_slot){
	isil_register_table_t *logic_chan_table;

        phy_video_slot->video_size = ISIL_VIDEO_SIZE_D1;
        logic_chan_table = &phy_video_slot->logic_chan_table;
        init_register_table_node(logic_chan_table);

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_jpeg_phy_video_slot_reset(isil_jpeg_phy_video_slot_t *phy_video_slot)
{
    if(phy_video_slot){
        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_jpeg_phy_video_slot_release(isil_jpeg_phy_video_slot_t *phy_video_slot)
{
    if(phy_video_slot){
        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_jpeg_phy_video_slot_set_video_size(isil_jpeg_phy_video_slot_t *phy_video_slot, enum ISIL_VIDEO_SIZE video_size)
{
    if(phy_video_slot){
        if((video_size >= ISIL_VIDEO_SIZE_QCIF) || (video_size <= ISIL_VIDEO_SIZE_USER)){
            phy_video_slot->video_size = video_size;

            return ISIL_OK;
        }
    }

    return ISIL_ERR;
}

static int isil_jpeg_phy_video_slot_get_video_size(isil_jpeg_phy_video_slot_t *phy_video_slot)
{
    if(phy_video_slot){
        return phy_video_slot->video_size;
    }

    return ISIL_ERR;
}

static int isil_jpeg_phy_video_slot_register_logic_chan(isil_jpeg_phy_video_slot_t *phy_video_slot, isil_jpeg_logic_encode_chan_t *jpeg_logic_encode_chan)
{
    if(phy_video_slot && jpeg_logic_encode_chan){
	isil_register_table_t *logic_chan_table;
        isil_register_node_t *logic_chan_node;
        ed_tcb_t *logic_chan_ed;

        logic_chan_ed = &jpeg_logic_encode_chan->logic_chan_ed;
        logic_chan_table = &phy_video_slot->logic_chan_table;
        logic_chan_node = &logic_chan_ed->ed;

        logic_chan_table->op->register_node_into_table(logic_chan_table, logic_chan_node);

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_jpeg_phy_video_slot_find_register_logic_chan(isil_jpeg_phy_video_slot_t *phy_video_slot, int phy_slot_id, isil_jpeg_logic_encode_chan_t **jpeg_logic_encode_chan)
{
    if(phy_video_slot && jpeg_logic_encode_chan){
	isil_register_table_t *logic_chan_table;
        isil_register_node_t *logic_chan_node;
        ed_tcb_t *logic_chan_ed;

        logic_chan_table = &phy_video_slot->logic_chan_table;

        logic_chan_table->op->find_register_node_in_table(logic_chan_table, &logic_chan_node, phy_slot_id);
        if(logic_chan_node){
            logic_chan_ed = to_get_endpoint_tcb_with_register_node(logic_chan_node);
            *jpeg_logic_encode_chan = to_get_jpeg_channel_with_logic_chan_ed(logic_chan_ed);
        }else{
            *jpeg_logic_encode_chan = NULL;
        }

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_jpeg_phy_video_slot_unregister_logic_chan(isil_jpeg_phy_video_slot_t *phy_video_slot, isil_jpeg_logic_encode_chan_t *jpeg_logic_encode_chan)
{
    if(phy_video_slot && jpeg_logic_encode_chan){
	isil_register_table_t *logic_chan_table;
        isil_register_node_t *logic_chan_node;
        ed_tcb_t *logic_chan_ed;

        logic_chan_ed = &jpeg_logic_encode_chan->logic_chan_ed;
        logic_chan_table = &phy_video_slot->logic_chan_table;
        logic_chan_node = &logic_chan_ed->ed;

        logic_chan_node->op->all_complete_done(logic_chan_node);
        logic_chan_table->op->unregister_node_from_table(logic_chan_table, logic_chan_node);

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_jpeg_phy_video_slot_touch_each_register_logic_chan(isil_jpeg_phy_video_slot_t *phy_video_slot, void (*func)(void *))
{
    if(phy_video_slot && func){
        isil_jpeg_logic_encode_chan_t *jpeg_logic_encode_chan;
	isil_register_table_t *logic_chan_table;
        isil_register_node_t *logic_chan_node;
        ed_tcb_t *logic_chan_ed;
        int entry_number, pos;

        logic_chan_table = &phy_video_slot->logic_chan_table;

        entry_number = logic_chan_table->op->get_curr_register_node_number(logic_chan_table);
        for(pos = 0; pos < entry_number; pos++){
            logic_chan_table->op->get_node_from_table(logic_chan_table, &logic_chan_node, pos);
            if(logic_chan_node){
                logic_chan_ed = to_get_endpoint_tcb_with_register_node(logic_chan_node);
                jpeg_logic_encode_chan = to_get_jpeg_channel_with_logic_chan_ed(logic_chan_ed);
                func(jpeg_logic_encode_chan);
            }
        }

        return ISIL_OK;
    }

    return ISIL_ERR;
}


struct isil_jpeg_phy_video_slot_operation isil_jpeg_phy_video_slot_operation_op = {
    .init                    = isil_jpeg_phy_video_slot_init,
    .reset                   = isil_jpeg_phy_video_slot_reset,
    .release                 = isil_jpeg_phy_video_slot_release,

    .set_video_size          = isil_jpeg_phy_video_slot_set_video_size,
    .get_video_size          = isil_jpeg_phy_video_slot_get_video_size,
    .register_logic_chan     = isil_jpeg_phy_video_slot_register_logic_chan,
    .find_register_logic_chan= isil_jpeg_phy_video_slot_find_register_logic_chan,
    .unregister_logic_chan   = isil_jpeg_phy_video_slot_unregister_logic_chan,
    .touch_each_register_logic_chan = isil_jpeg_phy_video_slot_touch_each_register_logic_chan,
};

static int isil_vj_bus_init(isil_vj_bus_t *vj_bus)
{
    int ret = ISIL_ERR;
    if(vj_bus){
        isil_vj_resource_pool_t *pool;
        isil_jpeg_phy_video_slot_t *phy_video_slot;
        int i;

        init_robust_processing_control(&vj_bus->robust_processing_control);
        init_jpeg_service_queue(&vj_bus->jpeg_encode_service_queue);
        for(i = 0; i < JPEG_MAX_RESOURCE_POOL; i++){
            pool = &vj_bus->resource_pool[i];
            pool->pool_id = i;
            pool->chip = to_get_chip_with_chip_vj_bus(vj_bus);
            pool->entry_number = JPEG_MAX_RESOURCE_SIZE;
            pool->op = &isil_vj_resource_pool_operation_op;
            ret = pool->op->init(pool);
            if(ret){
                ISIL_DBG(ISIL_DBG_ERR, "init resource pool failed %d\n", ret);
                return ret;
            }

            init_jpeg_service_queue(&vj_bus->jpeg_capture_service_queue[i]);
        }

        for(i = 0; i < ISIL_PHY_VJ_CHAN_NUMBER; i++){
            phy_video_slot = &vj_bus->phy_video_slot[i];
            phy_video_slot->phy_slot_id = i;
            phy_video_slot->op = &isil_jpeg_phy_video_slot_operation_op;
            phy_video_slot->op->init(phy_video_slot);
        }

        vj_bus->overflow_times = 0;
        vj_bus->ddr_page_base = 0;
        vj_bus->i_static_second_counter = 0;
        vj_bus->i_temp_fps_counter = 0;
        vj_bus->current_fps = 0;

        vj_bus->op->reset(vj_bus);

        return ISIL_OK;
    }

    return ret;
}

static int isil_vj_bus_reset(isil_vj_bus_t *vj_bus)
{
    if(vj_bus){
        isil_chip_t *chip;
        isil_vj_resource_pool_t *pool;
        jpeg_service_queue_t *jpeg_service_queue;
        int i;

        chip = to_get_chip_with_chip_vj_bus(vj_bus);
        chip->io_op->chip_write32(chip, JPEG_MODULE_REST, 0xf8000001);
        mdelay(1);
        chip->io_op->chip_write32(chip, JPEG_MODULE_REST, 0xf8000000);
        chip->io_op->chip_write32(chip, 0xd0f0, 0xffffffff);/*unknown register, resolved on cap_cmd two frame valid*/
        chip->io_op->chip_write32(chip, JPEG_CAP_TIMEOUT, 0x01000000);

        for(i = 0; i < JPEG_MAX_RESOURCE_POOL; i++){
            pool = &vj_bus->resource_pool[i];
            pool->op->reset(pool);

            ISIL_DBG(ISIL_DBG_DEBUG, "pool %d current entry number %d\n", pool->pool_id, pool->op->get_curr_entry_number(pool));
            jpeg_service_queue = &vj_bus->jpeg_capture_service_queue[i];
            ISIL_DBG(ISIL_DBG_DEBUG, "remove capture service queue %d\n", i);
            remove_jpeg_service_queue(jpeg_service_queue);
        }
        ISIL_DBG(ISIL_DBG_DEBUG, "remove encode service queue\n");
        jpeg_service_queue = &vj_bus->jpeg_encode_service_queue;
        remove_jpeg_service_queue(jpeg_service_queue);

        vj_bus->i_static_second_counter = 0;
        vj_bus->i_temp_fps_counter = 0;
        vj_bus->current_fps = 0;

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_vj_bus_release(isil_vj_bus_t *vj_bus)
{
    if(vj_bus){
        isil_vj_resource_pool_t *pool;
        int i;

        remove_jpeg_service_queue(&vj_bus->jpeg_encode_service_queue);
        for(i = 0; i < JPEG_MAX_RESOURCE_POOL; i++){
            pool = &vj_bus->resource_pool[i];
            pool->op->release(pool);

            remove_jpeg_service_queue(&vj_bus->jpeg_capture_service_queue[i]);
        }
    }

    return ISIL_OK;
}

static int isil_vj_bus_get_ddr_pages(isil_vj_bus_t *vj_bus)
{
    if(vj_bus) {

        vj_bus->ddr_page_base = 0;

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_vj_bus_free_ddr_pages(isil_vj_bus_t *vj_bus)
{
    if(vj_bus) {
        return ISIL_OK;
    }

    return ISIL_ERR;
}

static u32 isil_vj_bus_get_ddr_page_base(isil_vj_bus_t *vj_bus)
{
    if(vj_bus){
        return vj_bus->ddr_page_base;
    }

    ISIL_DBG(ISIL_DBG_FATAL, "vj ddr page not exist!!\n");

    return 0;
}

static void isil_vj_bus_polling_task(isil_vj_bus_t *vj_bus)
{
    if(vj_bus){
        isil_chip_t *chip;
        isil_jpeg_logic_encode_chan_t *jpeg_logic_encode_chan;


        vj_bus->i_static_second_counter++;
        if(vj_bus->i_static_second_counter >= TIMER_1_SECOND){
            vj_bus->current_fps = vj_bus->i_temp_fps_counter;
            vj_bus->i_temp_fps_counter = 0;
            vj_bus->i_static_second_counter = 0;
        }

        chip = to_get_chip_with_chip_vj_bus(vj_bus);
        for(jpeg_logic_encode_chan = chip->jpeg_encode_chan; (jpeg_logic_encode_chan - chip->jpeg_encode_chan) < ISIL_PHY_VJ_CHAN_NUMBER; 
                jpeg_logic_encode_chan++){
            if(atomic_read(&jpeg_logic_encode_chan->opened_flag)){
                ed_tcb_t    *opened_logic_chan_ed;
                isil_ed_fsm_t *ed_fsm;

                opened_logic_chan_ed = &jpeg_logic_encode_chan->opened_logic_chan_ed;
                ed_fsm = &opened_logic_chan_ed->ed_fsm;
                if(ed_fsm->op){
                    ed_fsm->op->update_state_timeout_counter(ed_fsm);
                }
            }
        }

    }
}

static struct isil_vj_bus_operation	isil_vj_bus_op = {
    .init               = isil_vj_bus_init,
    .reset              = isil_vj_bus_reset,
    .release            = isil_vj_bus_release,

    .get_ddr_pages      = isil_vj_bus_get_ddr_pages,
    .free_ddr_pages     = isil_vj_bus_free_ddr_pages,
    .get_ddr_page_base  = isil_vj_bus_get_ddr_page_base,
    .polling_task       = isil_vj_bus_polling_task,
};




int init_isil_vj_bus(isil_vj_bus_t *vj_bus)
{
    if(vj_bus != NULL){
        vj_bus->op = &isil_vj_bus_op;

        return vj_bus->op->init(vj_bus);
    }

    return ISIL_ERR;
}

void remove_isil_vj_bus(isil_vj_bus_t *vj_bus)
{
    if(vj_bus != NULL){
        vj_bus->op->release(vj_bus);
    }

    return ;
}

int isil_jpeg_save_chn(isil_jpeg_logic_encode_chan_t *logic_chnl, char *path, int cnt)
{
    struct file *jpeg_file = NULL;
    isil_vb_frame_tcb_t  *frame;
    isil_vb_frame_tcb_queue_t *frame_queue;
    isil_jpeg_encode_control_t    *encode_control;
    char name[128];
    int i = 0;
    if(cnt == 0){
        cnt = 0x0fffffff;
    }

    encode_control = &logic_chnl->encode_control;
    while((i++) < cnt) {        

        frame_queue = &logic_chnl->jpeg_frame_queue;

        while(!frame_queue->op->get_curr_queue_entry_number(frame_queue)){
            schedule();
            if (signal_pending(current)) {
                ISIL_DBG(ISIL_DBG_DEBUG, "receive signal\n");
                goto jpeg_out;
            }
        }

        memset(name, 0, 128);
        sprintf(name, "%s/video_chn_%02d_%02d_seq%d.jpg", path, logic_chnl->logic_chan_id, i, encode_control->i_frame_serial);
        jpeg_file = isil_kernel_file_open(name);
        if(IS_ERR(jpeg_file) || !jpeg_file){
            ISIL_DBG(ISIL_DBG_ERR, "create file %s, failed\n", name);
            return -EINVAL;
        }
        frame_queue->op->get_curr_consumer_from_queue(frame_queue);
        frame = frame_queue->curr_consumer;
        ISIL_DBG(ISIL_DBG_DEBUG, "receive (%d/%d) jpeg stream, seq %d!\n", (i+1), cnt, frame->frame_number);
        if(!frame->frame_is_err){
            isil_jpeg_to_file(jpeg_file, frame);
        }
        frame_queue->op->release_curr_consumer(frame_queue, &logic_chnl->pool);
jpeg_out:
        if(jpeg_file) {
            isil_kernel_file_close(jpeg_file);
        }
        jpeg_file = NULL;
    }
    //logic_chnl->op->close(&logic_chnl->opened_logic_chan_ed);
    return 0;
}

int isil_jpeg_to_file(struct file *jpeg_file, isil_vb_frame_tcb_t  *frame)
{
    mm_segment_t fs; 
    isil_vb_packet_tcb_t *packet;
    isil_vb_packet_tcb_queue_t *packet_queue;
    int len, first = 0;

    if(!jpeg_file || !frame)
    {
        return -EINVAL;
    }

    fs=get_fs();
    set_fs(KERNEL_DS);

    packet_queue = &frame->vb_packet_queue;
    first = 0;
    while(packet_queue->op->get_curr_queue_entry_number(packet_queue)){
        packet_queue->op->get_curr_consumer_from_queue(packet_queue);
        packet = packet_queue->curr_consumer;
        if(first == 0) {
            len = jpeg_file->f_op->write(jpeg_file, packet->data + sizeof(ext_h264_nal_bitstream_t) + packet->consumer_len, packet->payload_len - packet->consumer_len - sizeof(ext_h264_nal_bitstream_t), &(jpeg_file->f_pos));
            first = 1;
        }else{

            len = jpeg_file->f_op->write(jpeg_file, packet->data, packet->payload_len, &(jpeg_file->f_pos));
        }
        packet_queue->op->release_curr_consumer(packet_queue, frame->pool);
    }

    set_fs(fs);

    return 0;
}
