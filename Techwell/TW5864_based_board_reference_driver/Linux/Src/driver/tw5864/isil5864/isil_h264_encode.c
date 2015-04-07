#include    <isil5864/isil_common.h>

static isil_h264_encode_configuration_t	default_h264_master_encode_property = {
    .changedMask = ISIL_ENCODE_CONFIG_MASKALL,
    .width = WIDTH_FRAME_D1_PAL,
    .height = HEIGHT_FRAME_D1_PAL,
    .bitrate = DEFAULT_BITRATE_D1SIZE,
    .fps = DEFAULT_FRAMERATE_PAL,
    .keyFrameIntervals = DEFAULT_I_P_STRIDE,
    .gopIntervals = DEFAULT_GOP_VALUE,
    .imageLevel = 0,
};
static isil_h264_encode_feature_param_t default_main_encode_feature = {
    .b_enable_deinterlace   = ISIL_H264_FEATURE_ON,
    .b_enable_skip          = ISIL_H264_FEATURE_ON,
    .b_enable_half_pixel    = ISIL_H264_FEATURE_ON,
    .b_enable_I_4X4         = ISIL_H264_FEATURE_ON,
    .b_enable_quarter_pixel = ISIL_H264_FEATURE_ON,
    .i_mb_delay_value       = MB_DELAY_VALUE,
    .change_mask_flag       = 0xff,
};
static isil_h264_encode_rc_config_t default_main_encode_rc = {
    .e_image_priority = ISIL_RC_IMAGE_QUALITY_FIRST,
    .e_rc_type        = ISIL_H264_CBR,
    .i_qpi     = DEFAULT_QP,
    .i_qpp     = DEFAULT_QP,
    .i_qpb     = DEFAULT_QP,
    .change_mask_flag = 0xf,
};

static isil_h264_encode_configuration_t	default_h264_sub_encode_property = {
    .changedMask = ISIL_ENCODE_CONFIG_MASKALL,
    .width = WIDTH_FRAME_CIF_PAL,
    .height = HEIGHT_FRAME_CIF_PAL,
    .bitrate = DEFAULT_BITRATE_CIFSIZE,
    .fps = DEFAULT_FRAMERATE_PAL,
    .keyFrameIntervals = DEFAULT_I_P_STRIDE,
    .gopIntervals = DEFAULT_GOP_VALUE,
    .imageLevel = 0,
};
static isil_h264_encode_feature_param_t default_sub_encode_feature = {
    .b_enable_deinterlace   = ISIL_H264_FEATURE_ON,
    .b_enable_skip          = ISIL_H264_FEATURE_ON,
    .b_enable_half_pixel    = ISIL_H264_FEATURE_ON,
    .b_enable_I_4X4         = ISIL_H264_FEATURE_ON,
    .b_enable_quarter_pixel = ISIL_H264_FEATURE_ON,
    .i_mb_delay_value       = MB_DELAY_VALUE,
    .change_mask_flag       = 0xff,
};
static isil_h264_encode_rc_config_t default_sub_encode_rc = {
    .e_image_priority = ISIL_RC_IMAGE_QUALITY_FIRST,
    .e_rc_type        = ISIL_H264_CBR,
    .i_qpi = DEFAULT_QP,
    .i_qpp = DEFAULT_QP,
    .i_qpb = DEFAULT_QP,
    .change_mask_flag = 0xf,
};

static const u32 default_nonreal_time_fps_table[ISIL_PHY_VD_CHAN_NUMBER]= {
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
};

static const u32 default_nonreal_time_fmt_table[ISIL_PHY_VD_CHAN_NUMBER]= {
    ISIL_VIDEO_SIZE_D1,  ISIL_VIDEO_SIZE_D1,  ISIL_VIDEO_SIZE_D1,  ISIL_VIDEO_SIZE_D1,
    ISIL_VIDEO_SIZE_D1,  ISIL_VIDEO_SIZE_D1,  ISIL_VIDEO_SIZE_D1,  ISIL_VIDEO_SIZE_D1,
    ISIL_VIDEO_SIZE_D1,  ISIL_VIDEO_SIZE_D1,  ISIL_VIDEO_SIZE_D1,  ISIL_VIDEO_SIZE_D1,
    ISIL_VIDEO_SIZE_D1,  ISIL_VIDEO_SIZE_D1,  ISIL_VIDEO_SIZE_D1,  ISIL_VIDEO_SIZE_D1,
};

static const unsigned int   Lambda_lookup_table[52] =
{
    0x0020,0x0020,0x0020,0x0020,
    0x0020,0x0020,0x0020,0x0020,
    0x0020,0x0020,0x0020,0x0020,
    0x0020,0x0020,0x0020,0x0020,
    0x0040,0x0040,0x0040,0x0040,
    0x0060,0x0060,0x0060,0x0080,
    0x0080,0x0080,0x00a0,0x00c0,
    0x00c0,0x00e0,0x0100,0x0120,
    0x0140,0x0160,0x01a0,0x01c0,
    0x0200,0x0240,0x0280,0x02e0,
    0x0320,0x03a0,0x0400,0x0480,
    0x0500,0x05a0,0x0660,0x0720,
    0x0800,0x0900,0x0a20,0x0b60
};
static const unsigned int Intra4X4_Lambda3[52] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 3, 3, 3, 4,
    4, 4, 5, 6, 6, 7, 8, 9,
    10, 11, 13, 14, 16, 18, 20, 23,
    25, 29, 32, 36, 40, 45, 51, 57,
    64, 72, 81, 91
};

int isil_h264_save_chn(isil_h264_logic_encode_chan_t *logic_chnl, char *path);
int isil_h264_handle_user_q(isil_h264_logic_encode_chan_t *logic_chnl, char *name, char *save, int cnt);
int isil_h264_to_file(struct file *h264_file, isil_video_frame_tcb_t  *frame);
static void register_opened_logic_encode_chan_into_master_slot(isil_h264_logic_encode_chan_t *h264_logic_encode_chan, isil_h264_phy_video_slot_t *master_slot);
static int  h264_encode_driver_in_unregister_state_recv_close_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_h264_logic_encode_chan_t   *h264_logic_encode_chan;
        isil_kthread_msg_queue_t        *isil_msg_queue;
        isil_send_msg_controller_t      *send_msg_contr;

        h264_logic_encode_chan = (isil_h264_logic_encode_chan_t*)context;
        if(atomic_read(&h264_logic_encode_chan->opened_flag) > 0){
            send_msg_contr = &h264_logic_encode_chan->send_msg_contr;
            send_msg_contr->op->delete_send_msg_controller(send_msg_contr);
            get_msg_queue_header(&isil_msg_queue);
            delete_all_msg_of_this_h264_encode_chan(isil_msg_queue, h264_logic_encode_chan);
            if(isil_h264_encode_chan_is_in_robust_processing(h264_logic_encode_chan) == 1){
                ed_fsm->op->save_event_for_robust_processing(ed_fsm);
            }

            ed_fsm->op->reset(ed_fsm);
            atomic_dec(&h264_logic_encode_chan->opened_flag);
        }

        if(atomic_read(&h264_logic_encode_chan->opened_flag) == 0){
            ed_tcb_t    *ed_tcb;
            ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
            ed_tcb->op->close_done(ed_tcb);
        }
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id, atomic_read(&h264_logic_encode_chan->opened_flag));
    }
    return ISIL_OK;
}

static int  h264_encode_driver_in_unregister_state_recv_suspend_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_h264_logic_encode_chan_t   *h264_logic_encode_chan;
        ed_tcb_t    *ed_tcb;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        ed_tcb->op->suspend_done(ed_tcb);
        h264_logic_encode_chan = (isil_h264_logic_encode_chan_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id, atomic_read(&h264_logic_encode_chan->opened_flag));
    }
    return ISIL_OK;
}

static int  h264_encode_driver_in_unregister_state_recv_resume_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_h264_logic_encode_chan_t   *h264_logic_encode_chan;
        ed_tcb_t    *ed_tcb;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        ed_tcb->op->resume_done(ed_tcb);
        h264_logic_encode_chan = (isil_h264_logic_encode_chan_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id, atomic_read(&h264_logic_encode_chan->opened_flag));
    }
    return ISIL_OK;
}

static int  h264_encode_driver_in_unregister_state_recv_open_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_h264_logic_encode_chan_t   *h264_logic_encode_chan;

        h264_logic_encode_chan = (isil_h264_logic_encode_chan_t*)context;
        if(atomic_read(&h264_logic_encode_chan->opened_flag) == 0){
            register_opened_logic_encode_chan_into_master_slot(h264_logic_encode_chan, h264_logic_encode_chan->master_slot);
            h264_logic_encode_chan->encode_control.last_jiffies = jiffies;
            ed_fsm->op->change_state(ed_fsm, ISIL_ED_SUSPEND);

            if(isil_h264_encode_chan_is_in_robust_processing(h264_logic_encode_chan) == 1){
                ed_tcb_t    *ed_tcb;
                ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
                ed_fsm->op->restore_event_from_robust_processing(ed_fsm);
                if(have_any_pending_event(ed_tcb)){
                    driver_trigger_pending_event(ed_tcb);
                }
            }
        }
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id, atomic_read(&h264_logic_encode_chan->opened_flag)); 
    }
    return ISIL_OK;
}

static int  h264_encode_driver_in_idle_state_recv_close_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_h264_logic_encode_chan_t   *h264_logic_encode_chan;
        ed_tcb_t    *ed_tcb;

        ed_fsm->op->change_state(ed_fsm, ISIL_ED_UNREGISTER);
        h264_logic_encode_chan = (isil_h264_logic_encode_chan_t*)context;
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_close_event(ed_tcb, 1);
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id, atomic_read(&h264_logic_encode_chan->opened_flag));
    }
    return ISIL_OK;
}

static int  h264_encode_driver_in_idle_state_recv_timeout_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_h264_logic_encode_chan_t   *h264_logic_encode_chan;
   
        h264_logic_encode_chan = (isil_h264_logic_encode_chan_t*)context;
        start_isil_h264_encode_chan_robust_process(h264_logic_encode_chan);
        encode_chan_gen_req_msg(h264_logic_encode_chan, REQ_ALGO_CHIP_RESET, NONBLOCK_OP);
        ISIL_DBG(ISIL_DBG_INFO, "\n%d.%d.%d\n", h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id, atomic_read(&h264_logic_encode_chan->opened_flag));
    }
    return ISIL_OK;
}

static int  h264_encode_driver_in_idle_state_recv_suspend_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_h264_logic_encode_chan_t   *h264_logic_encode_chan;
        ed_tcb_t    *ed_tcb;

        ed_fsm->op->change_state(ed_fsm, ISIL_ED_SUSPEND);
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        ed_tcb->op->suspend_done(ed_tcb);
        h264_logic_encode_chan = (isil_h264_logic_encode_chan_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id, atomic_read(&h264_logic_encode_chan->opened_flag));
    }
    return ISIL_OK;
}

static int  h264_encode_driver_in_idle_state_recv_resume_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_h264_logic_encode_chan_t   *h264_logic_encode_chan;
        ed_tcb_t    *ed_tcb;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        ed_tcb->op->resume_done(ed_tcb);
        h264_logic_encode_chan = (isil_h264_logic_encode_chan_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id, atomic_read(&h264_logic_encode_chan->opened_flag));
    }
    return ISIL_OK;
}

static int  h264_encode_driver_in_idle_state_recv_deliver_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_h264_logic_encode_chan_t *h264_logic_encode_chan = (isil_h264_logic_encode_chan_t*)context;
        ed_fsm->op->change_state(ed_fsm, ISIL_ED_STANDBY);
        init_encode_chan_service_tcb_with_start_encode(&h264_logic_encode_chan->encode_request_tcb);
    }
    return ISIL_OK;
}

static int  h264_encode_driver_in_standby_state_recv_close_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_h264_logic_encode_chan_t       *h264_logic_encode_chan;
        chip_encode_chan_service_queue_t    *chip_encode_service_queue;
        ed_tcb_t    *ed_tcb;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        h264_logic_encode_chan = (isil_h264_logic_encode_chan_t*)context;
        chip_encode_service_queue = &h264_logic_encode_chan->chip->chip_encode_service_queue;
        if(chip_encode_service_queue->op->delete_service_request_from_queue(chip_encode_service_queue, &h264_logic_encode_chan->encode_request_tcb)){
            ed_fsm->op->change_state(ed_fsm, ISIL_ED_UNREGISTER);
            driver_gen_close_event(ed_tcb, 1);
            ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id, atomic_read(&h264_logic_encode_chan->opened_flag));
        } else {
            driver_gen_close_event(ed_tcb, 0);
        }
    }
    return ISIL_OK;
}

static int  h264_encode_driver_in_standby_state_recv_timeout_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_h264_logic_encode_chan_t       *h264_logic_encode_chan;
        chip_encode_chan_service_queue_t    *chip_encode_service_queue;

        h264_logic_encode_chan = (isil_h264_logic_encode_chan_t*)context;
        start_isil_h264_encode_chan_robust_process(h264_logic_encode_chan);
        chip_encode_service_queue = &h264_logic_encode_chan->chip->chip_encode_service_queue;
        if(chip_encode_service_queue->op->delete_service_request_from_queue(chip_encode_service_queue, &h264_logic_encode_chan->encode_request_tcb)){
            encode_chan_gen_req_msg(h264_logic_encode_chan, REQ_ALGO_CHIP_RESET, NONBLOCK_OP);
            ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id, atomic_read(&h264_logic_encode_chan->opened_flag));
        } else {
            ed_tcb_t    *ed_tcb;
            ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
            driver_gen_timeout_event(ed_tcb, 0);
        }
    }
    return ISIL_OK;
}

static int  h264_encode_driver_in_standby_state_recv_suspend_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_h264_logic_encode_chan_t       *h264_logic_encode_chan;
        chip_encode_chan_service_queue_t    *chip_encode_service_queue;
        ed_tcb_t    *ed_tcb;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        h264_logic_encode_chan = (isil_h264_logic_encode_chan_t*)context;
        chip_encode_service_queue = &h264_logic_encode_chan->chip->chip_encode_service_queue;
        if(chip_encode_service_queue->op->delete_service_request_from_queue(chip_encode_service_queue, &h264_logic_encode_chan->encode_request_tcb)){
            ed_fsm->op->change_state(ed_fsm, ISIL_ED_SUSPEND);
            ed_tcb->op->suspend_done(ed_tcb);
            ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id, atomic_read(&h264_logic_encode_chan->opened_flag));
        } else {
            driver_gen_suspend_event(ed_tcb, 0);
        }
    }
    return ISIL_OK;
}

static int  h264_encode_driver_in_standby_state_recv_resume_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_h264_logic_encode_chan_t   *h264_logic_encode_chan;
        ed_tcb_t    *ed_tcb;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        ed_tcb->op->resume_done(ed_tcb);
        h264_logic_encode_chan = (isil_h264_logic_encode_chan_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id, atomic_read(&h264_logic_encode_chan->opened_flag));
    }
    return ISIL_OK;
}

static int  h264_encode_driver_in_standby_state_recv_deliver_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        ed_fsm->op->change_state(ed_fsm, ISIL_ED_RUNNING);
    }
    return ISIL_OK;
}

static int  h264_encode_driver_in_suspend_state_recv_close_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_h264_logic_encode_chan_t   *h264_logic_encode_chan;
        ed_tcb_t    *ed_tcb;

        ed_fsm->op->change_state(ed_fsm, ISIL_ED_UNREGISTER);
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_close_event(ed_tcb, 1);
        h264_logic_encode_chan = (isil_h264_logic_encode_chan_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id, atomic_read(&h264_logic_encode_chan->opened_flag));
    }
    return ISIL_OK;
}

static int  h264_encode_driver_in_suspend_state_recv_suspend_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_h264_logic_encode_chan_t   *h264_logic_encode_chan;
        ed_tcb_t    *ed_tcb;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        ed_tcb->op->suspend_done(ed_tcb);
        h264_logic_encode_chan = (isil_h264_logic_encode_chan_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id, atomic_read(&h264_logic_encode_chan->opened_flag));
    }
    return ISIL_OK;
}

static int  h264_encode_driver_in_suspend_state_recv_resume_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_h264_logic_encode_chan_t   *h264_logic_encode_chan;
        ed_tcb_t    *ed_tcb;

        h264_logic_encode_chan = (isil_h264_logic_encode_chan_t*)context;
        ed_fsm->op->change_state(ed_fsm, ISIL_ED_IDLE);
        encode_chan_gen_req_msg(h264_logic_encode_chan, REQ_ALGO_SLICE_HEAD, NONBLOCK_OP);
        init_encode_chan_service_tcb_with_first_start(&h264_logic_encode_chan->encode_request_tcb);
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        ed_tcb->op->resume_done(ed_tcb);
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id, atomic_read(&h264_logic_encode_chan->opened_flag));
    }
    return ISIL_OK;
}

static int  h264_encode_driver_in_running_state_recv_close_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_h264_logic_encode_chan_t   *h264_logic_encode_chan;
        ed_tcb_t    *ed_tcb;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_close_event(ed_tcb, 0);
        h264_logic_encode_chan = (isil_h264_logic_encode_chan_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id, atomic_read(&h264_logic_encode_chan->opened_flag));
    }
    return ISIL_OK;
}

static int  h264_encode_driver_in_running_state_recv_timeout_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_h264_logic_encode_chan_t   *h264_logic_encode_chan;
        isil_h264_encode_control_t      *encode_control;
 
        h264_logic_encode_chan = (isil_h264_logic_encode_chan_t*)context;
        start_isil_h264_encode_chan_robust_process(h264_logic_encode_chan);
        encode_control = &h264_logic_encode_chan->encode_control;
        encode_control->op->notify_timeout(encode_control, h264_logic_encode_chan, h264_logic_encode_chan->chip);
        encode_chan_gen_req_msg(h264_logic_encode_chan, REQ_ALGO_CHIP_RESET, NONBLOCK_OP);
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id, atomic_read(&h264_logic_encode_chan->opened_flag));
    }
    return ISIL_OK;
}

static int  h264_encode_driver_in_running_state_recv_suspend_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_h264_logic_encode_chan_t   *h264_logic_encode_chan;
        ed_tcb_t    *ed_tcb;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_suspend_event(ed_tcb, 0);
        h264_logic_encode_chan = (isil_h264_logic_encode_chan_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id, atomic_read(&h264_logic_encode_chan->opened_flag));
    }
    return ISIL_OK;
}

static int  h264_encode_driver_in_running_state_recv_resume_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_h264_logic_encode_chan_t   *h264_logic_encode_chan;
        ed_tcb_t    *ed_tcb;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_resume_event(ed_tcb, 0);
        h264_logic_encode_chan = (isil_h264_logic_encode_chan_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id, atomic_read(&h264_logic_encode_chan->opened_flag));
    }
    return ISIL_OK;
}

static int  h264_encode_driver_in_running_state_recv_deliver_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        ed_fsm->op->change_state(ed_fsm, ISIL_ED_TRANSFERING);
    }
    return ISIL_OK;
}

static int  h264_encode_driver_in_transfering_state_recv_close_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_h264_logic_encode_chan_t   *h264_logic_encode_chan;
        ed_tcb_t    *ed_tcb;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_close_event(ed_tcb, 0);
        h264_logic_encode_chan = (isil_h264_logic_encode_chan_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id, atomic_read(&h264_logic_encode_chan->opened_flag));
    }
    return ISIL_OK;
}

static int  h264_encode_driver_in_transfering_state_recv_timeout_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_h264_logic_encode_chan_t   *h264_logic_encode_chan;
        isil_h264_encode_control_t      *encode_control;
 
        h264_logic_encode_chan = (isil_h264_logic_encode_chan_t*)context;
        start_isil_h264_encode_chan_robust_process(h264_logic_encode_chan);
        encode_control = &h264_logic_encode_chan->encode_control;
        encode_control->op->notify_timeout(encode_control, h264_logic_encode_chan, h264_logic_encode_chan->chip);
        encode_chan_gen_req_msg(h264_logic_encode_chan, REQ_ALGO_CHIP_RESET, NONBLOCK_OP);
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id, atomic_read(&h264_logic_encode_chan->opened_flag));
    }
    return ISIL_OK;
}

static int  h264_encode_driver_in_transfering_state_recv_suspend_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_h264_logic_encode_chan_t   *h264_logic_encode_chan;
        ed_tcb_t    *ed_tcb;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_suspend_event(ed_tcb, 0);
        h264_logic_encode_chan = (isil_h264_logic_encode_chan_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id, atomic_read(&h264_logic_encode_chan->opened_flag));
    }
    return ISIL_OK;
}

static int  h264_encode_driver_in_transfering_state_recv_resume_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_h264_logic_encode_chan_t   *h264_logic_encode_chan;
        ed_tcb_t    *ed_tcb;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_resume_event(ed_tcb, 0);
        h264_logic_encode_chan = (isil_h264_logic_encode_chan_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id, atomic_read(&h264_logic_encode_chan->opened_flag));
    }
    return ISIL_OK;
}


static int  h264_encode_driver_in_transfering_state_recv_deliver_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_h264_logic_encode_chan_t   *h264_logic_encode_chan;
        ed_tcb_t        *ed_tcb;

        h264_logic_encode_chan = (isil_h264_logic_encode_chan_t*)context;
        ed_fsm->op->change_state(ed_fsm, ISIL_ED_DONE);
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        if(have_any_pending_event(ed_tcb)){
            driver_trigger_pending_event(ed_tcb);
        } else {
            driver_gen_deliver_event(ed_tcb, 1);
        }
    }
    return ISIL_OK;
}

static int  h264_encode_driver_in_done_state_recv_close_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_h264_logic_encode_chan_t   *h264_logic_encode_chan;
        ed_tcb_t    *ed_tcb;

        ed_fsm->op->change_state(ed_fsm, ISIL_ED_UNREGISTER);
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_close_event(ed_tcb, 1);
        h264_logic_encode_chan = (isil_h264_logic_encode_chan_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id, atomic_read(&h264_logic_encode_chan->opened_flag));
    }
    return ISIL_OK;
}

static int  h264_encode_driver_in_done_state_recv_timeout_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_h264_logic_encode_chan_t   *h264_logic_encode_chan;

        h264_logic_encode_chan = (isil_h264_logic_encode_chan_t*)context;
        start_isil_h264_encode_chan_robust_process(h264_logic_encode_chan);
        encode_chan_gen_req_msg(h264_logic_encode_chan, REQ_ALGO_CHIP_RESET, NONBLOCK_OP);
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id, atomic_read(&h264_logic_encode_chan->opened_flag));
    }
    return ISIL_OK;
}

static int  h264_encode_driver_in_done_state_recv_suspend_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_h264_logic_encode_chan_t   *h264_logic_encode_chan;
        ed_tcb_t    *ed_tcb;

        ed_fsm->op->change_state(ed_fsm, ISIL_ED_SUSPEND);
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        ed_tcb->op->suspend_done(ed_tcb);
        h264_logic_encode_chan = (isil_h264_logic_encode_chan_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id, atomic_read(&h264_logic_encode_chan->opened_flag));
    }
    return ISIL_OK;
}

static int  h264_encode_driver_in_done_state_recv_resume_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_h264_logic_encode_chan_t   *h264_logic_encode_chan;
        ed_tcb_t    *ed_tcb;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_deliver_event(ed_tcb, 1);
        ed_tcb->op->resume_done(ed_tcb);
        h264_logic_encode_chan = (isil_h264_logic_encode_chan_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d.%d.%d\n", h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id, atomic_read(&h264_logic_encode_chan->opened_flag));
    }
    return ISIL_OK;
}

static int  h264_encode_driver_in_done_state_recv_deliver_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_h264_logic_encode_chan_t   *h264_logic_encode_chan;
        h264_logic_encode_chan = (isil_h264_logic_encode_chan_t*)context;
        ed_fsm->op->change_state(ed_fsm, ISIL_ED_IDLE);
        encode_chan_gen_req_msg(h264_logic_encode_chan, REQ_ALGO_SLICE_HEAD, NONBLOCK_OP);
    }
    return ISIL_OK;
}

static int  h264_encode_driver_sync_hook(isil_ed_fsm_t *ed_fsm, void *context)
{

    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_h264_encode_feature_param_t *config_feature_param, *running_feature_param;
        isil_h264_encode_feature_t *encode_feature;
        isil_h264_encode_rc_config_t *config_rc, *running_rc;
        isil_h264_encode_rc_t *encode_rc;
        isil_h264_logic_encode_chan_t   *h264_logic_encode_chan;
        isil_h264_encode_property_t   *encode_property;
        isil_h264_encode_configuration_t      *config_encode_property;
        isil_h264_encode_configuration_t      *running_encode_property;
        unsigned long   flags;
 
        h264_logic_encode_chan = (isil_h264_logic_encode_chan_t*)context;
        /*sync h264 attribute*/
        encode_property = &h264_logic_encode_chan->encode_property;
        spin_lock_irqsave(&encode_property->lock, flags);
        config_encode_property = &encode_property->config_encode_property;
        running_encode_property = &encode_property->running_encode_property;

        if(config_encode_property->changedMask & ISIL_ENCODE_CONFIG_ENCODE_SIZE_CHANGED){
            config_encode_property->changedMask &= ~ISIL_ENCODE_CONFIG_ENCODE_SIZE_CHANGED;
            if(config_encode_property->width && config_encode_property->height){
                running_encode_property->width = config_encode_property->width;
                running_encode_property->height = config_encode_property->height;
                running_encode_property->changedMask |= ISIL_ENCODE_CONFIG_ENCODE_SIZE_CHANGED;
                ISIL_DBG(ISIL_DBG_INFO, "(chip_id=%d, chan_id=%d):width = %d, height = %d\n", h264_logic_encode_chan->chip->chip_id,
                h264_logic_encode_chan->logic_chan_id, running_encode_property->width, 
                        running_encode_property->height);
             }else{
                ISIL_DBG(ISIL_DBG_ERR, "configure parameter error, width = %d, height = %d\n", config_encode_property->width, 
                        config_encode_property->height);
            }
        }
        if(config_encode_property->changedMask & ISIL_ENCODE_CONFIG_BITRATE_CHANGED){
            config_encode_property->changedMask &= ~ISIL_ENCODE_CONFIG_BITRATE_CHANGED;
            running_encode_property->bitrate = config_encode_property->bitrate;
            running_encode_property->changedMask |= ISIL_ENCODE_CONFIG_BITRATE_CHANGED;
            ISIL_DBG(ISIL_DBG_INFO, "(chip_id=%d, chan_id=%d):bps = %d\n", h264_logic_encode_chan->chip->chip_id,
                h264_logic_encode_chan->logic_chan_id, running_encode_property->bitrate);
        }
        if(config_encode_property->changedMask & ISIL_ENCODE_CONFIG_FPS_CHANGED){
            config_encode_property->changedMask &= ~ISIL_ENCODE_CONFIG_FPS_CHANGED;
            running_encode_property->fps = config_encode_property->fps;
            running_encode_property->changedMask |= ISIL_ENCODE_CONFIG_FPS_CHANGED;
            ISIL_DBG(ISIL_DBG_INFO, "(chip_id=%d, chan_id=%d):fps = %d\n", h264_logic_encode_chan->chip->chip_id,
                h264_logic_encode_chan->logic_chan_id, running_encode_property->fps);
        }
        if(config_encode_property->changedMask & ISIL_ENCODE_CONFIG_KEYFRAME_INTERVALS_CHANGED){
            config_encode_property->changedMask &= ~ISIL_ENCODE_CONFIG_KEYFRAME_INTERVALS_CHANGED;
            running_encode_property->keyFrameIntervals = config_encode_property->keyFrameIntervals;
            running_encode_property->changedMask |= ISIL_ENCODE_CONFIG_KEYFRAME_INTERVALS_CHANGED;
            ISIL_DBG(ISIL_DBG_INFO, "(chip_id=%d, chan_id=%d):i_p = %d\n", h264_logic_encode_chan->chip->chip_id,
                h264_logic_encode_chan->logic_chan_id, running_encode_property->keyFrameIntervals);
        }
        if(config_encode_property->changedMask & ISIL_ENCODE_CONFIG_GOP_INTERVALS_CHANGED){
            config_encode_property->changedMask &= ~ISIL_ENCODE_CONFIG_GOP_INTERVALS_CHANGED;
            running_encode_property->gopIntervals = config_encode_property->gopIntervals;
            running_encode_property->changedMask |= ISIL_ENCODE_CONFIG_GOP_INTERVALS_CHANGED;
            ISIL_DBG(ISIL_DBG_INFO, "(chip_id=%d, chan_id=%d):gop = %d\n", h264_logic_encode_chan->chip->chip_id,
                h264_logic_encode_chan->logic_chan_id, running_encode_property->gopIntervals);
        }
        if(config_encode_property->changedMask & ISIL_ENCODE_CONFIG_IMAGE_LEVEL_CHANGED){
            config_encode_property->changedMask &= ~ISIL_ENCODE_CONFIG_IMAGE_LEVEL_CHANGED;
            running_encode_property->imageLevel = config_encode_property->imageLevel;
            running_encode_property->changedMask |= ISIL_ENCODE_CONFIG_IMAGE_LEVEL_CHANGED;
            ISIL_DBG(ISIL_DBG_INFO, "(chip_id=%d, chan_id=%d):level = %d\n", h264_logic_encode_chan->chip->chip_id,
                h264_logic_encode_chan->logic_chan_id, running_encode_property->imageLevel);
        }

        /*sync feature*/
        encode_feature = &h264_logic_encode_chan->encode_property.encode_feature;
        config_feature_param = &encode_feature->config_feature_param;
        running_feature_param = &encode_feature->running_feature_param;
        if(config_feature_param->change_mask_flag & ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_DEINTERLACE_MASK) {
            config_feature_param->change_mask_flag &= ~ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_DEINTERLACE_MASK;
            running_feature_param->b_enable_deinterlace = config_feature_param->b_enable_deinterlace;
            running_feature_param->change_mask_flag |= ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_DEINTERLACE_MASK;
            /*ISIL_DBG(ISIL_DBG_INFO, "(chip_id=%d, chan_id=%d):deinterlace = %d\n", h264_logic_encode_chan->chip->chip_id,
                h264_logic_encode_chan->logic_chan_id, running_feature_param->b_enable_deinterlace);*/
        }
        if(config_feature_param->change_mask_flag & ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_SKIP_MASK) {
            config_feature_param->change_mask_flag &= ~ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_SKIP_MASK;
            running_feature_param->b_enable_skip = config_feature_param->b_enable_skip;
            running_feature_param->change_mask_flag |= ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_SKIP_MASK;
            ISIL_DBG(ISIL_DBG_INFO, "(chip_id=%d, chan_id=%d):skip = %d\n", h264_logic_encode_chan->chip->chip_id,
                h264_logic_encode_chan->logic_chan_id, running_feature_param->b_enable_skip);
        }
        if(config_feature_param->change_mask_flag & ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_I_4X4_MASK) {
            config_feature_param->change_mask_flag &= ~ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_I_4X4_MASK;
            running_feature_param->b_enable_I_4X4 = config_feature_param->b_enable_I_4X4;
            running_feature_param->change_mask_flag |= ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_I_4X4_MASK;
            ISIL_DBG(ISIL_DBG_INFO, "(chip_id=%d, chan_id=%d):intra4X4 = %d\n", h264_logic_encode_chan->chip->chip_id,
                h264_logic_encode_chan->logic_chan_id, running_feature_param->b_enable_I_4X4);
        }
        if(config_feature_param->change_mask_flag & ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_HALF_PIXEL_MASK) {
            config_feature_param->change_mask_flag &= ~ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_HALF_PIXEL_MASK;
            running_feature_param->b_enable_half_pixel = config_feature_param->b_enable_half_pixel;
            running_feature_param->change_mask_flag |= ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_HALF_PIXEL_MASK;
            ISIL_DBG(ISIL_DBG_INFO, "(chip_id=%d, chan_id=%d):1/2 pixel = %d\n", h264_logic_encode_chan->chip->chip_id,
                h264_logic_encode_chan->logic_chan_id, running_feature_param->b_enable_half_pixel);
        }
        if(config_feature_param->change_mask_flag & ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_QUARTER_PIXEL_MASK) {
            config_feature_param->change_mask_flag &= ~ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_QUARTER_PIXEL_MASK;
            running_feature_param->b_enable_quarter_pixel = config_feature_param->b_enable_quarter_pixel;
            running_feature_param->change_mask_flag |= ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_QUARTER_PIXEL_MASK;
            ISIL_DBG(ISIL_DBG_INFO, "(chip_id=%d, chan_id=%d):1/4 pixel = %d\n", h264_logic_encode_chan->chip->chip_id,
                h264_logic_encode_chan->logic_chan_id, running_feature_param->b_enable_quarter_pixel);
        }
        if(config_feature_param->change_mask_flag & ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_MB_DELAY_MASK) {
            config_feature_param->change_mask_flag &= ~ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_MB_DELAY_MASK;
            running_feature_param->i_mb_delay_value = config_feature_param->i_mb_delay_value;
            running_feature_param->change_mask_flag |= ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_MB_DELAY_MASK;
            ISIL_DBG(ISIL_DBG_INFO, "(chip_id=%d, chan_id=%d):mb_delay = %d\n", h264_logic_encode_chan->chip->chip_id,
                h264_logic_encode_chan->logic_chan_id, running_feature_param->i_mb_delay_value);
        }

        /*configure rc*/
        encode_rc = &h264_logic_encode_chan->encode_property.encode_rc;
        config_rc = &encode_rc->config_rc;
        running_rc = &encode_rc->running_rc;
        if(config_rc->change_mask_flag & ISIL_H264_ENCODE_RC_ENABLE_CHANGE_IMAGE_PRIORITY_MASK) {
            config_rc->change_mask_flag &= ~ISIL_H264_ENCODE_RC_ENABLE_CHANGE_IMAGE_PRIORITY_MASK;
            running_rc->e_image_priority = config_rc->e_image_priority;
            running_rc->change_mask_flag |= ISIL_H264_ENCODE_RC_ENABLE_CHANGE_IMAGE_PRIORITY_MASK;
            ISIL_DBG(ISIL_DBG_INFO, "(chip_id=%d, chan_id=%d):pri = %d\n", h264_logic_encode_chan->chip->chip_id,
                h264_logic_encode_chan->logic_chan_id, running_rc->e_image_priority);
        }
        if(config_rc->change_mask_flag & ISIL_H264_ENCODE_RC_ENABLE_CHANGE_RC_TYPE_MASK) {
            config_rc->change_mask_flag &= ~ISIL_H264_ENCODE_RC_ENABLE_CHANGE_RC_TYPE_MASK;
            running_rc->e_rc_type = config_rc->e_rc_type;
            running_rc->change_mask_flag |= ISIL_H264_ENCODE_RC_ENABLE_CHANGE_RC_TYPE_MASK;
            ISIL_DBG(ISIL_DBG_INFO, "(chip_id=%d, chan_id=%d):type = %d\n", h264_logic_encode_chan->chip->chip_id,
                h264_logic_encode_chan->logic_chan_id, running_rc->e_rc_type);
        }
        if(config_rc->change_mask_flag & ISIL_H264_ENCODE_RC_ENABLE_CHANGE_QPI_MASK) {
            config_rc->change_mask_flag &= ~ISIL_H264_ENCODE_RC_ENABLE_CHANGE_QPI_MASK;
            running_rc->i_qpi = config_rc->i_qpi;
            running_rc->change_mask_flag |= ISIL_H264_ENCODE_RC_ENABLE_CHANGE_QPI_MASK;
            ISIL_DBG(ISIL_DBG_INFO, "(chip_id=%d, chan_id=%d):qpi = %d\n", h264_logic_encode_chan->chip->chip_id,
                h264_logic_encode_chan->logic_chan_id, running_rc->i_qpi);
        }
        if(config_rc->change_mask_flag & ISIL_H264_ENCODE_RC_ENABLE_CHANGE_QPP_MASK) {
            config_rc->change_mask_flag &= ~ISIL_H264_ENCODE_RC_ENABLE_CHANGE_QPP_MASK;
            running_rc->i_qpp = config_rc->i_qpp;
            running_rc->change_mask_flag |= ISIL_H264_ENCODE_RC_ENABLE_CHANGE_QPP_MASK;
            ISIL_DBG(ISIL_DBG_INFO, "(chip_id=%d, chan_id=%d):qpp = %d\n", h264_logic_encode_chan->chip->chip_id,
                h264_logic_encode_chan->logic_chan_id, running_rc->i_qpp);
        }
        if(config_rc->change_mask_flag & ISIL_H264_ENCODE_RC_ENABLE_CHANGE_QPB_MASK) {
            config_rc->change_mask_flag &= ~ISIL_H264_ENCODE_RC_ENABLE_CHANGE_QPB_MASK;
            running_rc->i_qpb = config_rc->i_qpb;
            running_rc->change_mask_flag |= ISIL_H264_ENCODE_RC_ENABLE_CHANGE_QPB_MASK;
            ISIL_DBG(ISIL_DBG_INFO, "(chip_id=%d, chan_id=%d):qpb = %d\n", h264_logic_encode_chan->chip->chip_id,
                h264_logic_encode_chan->logic_chan_id, running_rc->i_qpb);
        }
        spin_unlock_irqrestore(&encode_property->lock, flags);
     }

    return ISIL_OK;
}

static int h264_encode_driver_sync_running_to_config_hook(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_h264_encode_feature_param_t *config_feature_param, *running_feature_param;
        isil_h264_encode_feature_t *encode_feature;
        isil_h264_encode_rc_config_t *config_rc, *running_rc;
        isil_h264_encode_rc_t *encode_rc;
        isil_h264_logic_encode_chan_t   *h264_logic_encode_chan;
        isil_h264_encode_configuration_t      *config_encode_property;
        isil_h264_encode_configuration_t      *running_encode_property;

        h264_logic_encode_chan = (isil_h264_logic_encode_chan_t*)context;

        /*sync h264 attribute*/
        config_encode_property = &h264_logic_encode_chan->encode_property.config_encode_property;
        running_encode_property = &h264_logic_encode_chan->encode_property.running_encode_property;

        memcpy(config_encode_property, running_encode_property, sizeof(isil_h264_encode_configuration_t));
        config_encode_property->changedMask = 0;

        /*sync feature*/
        encode_feature = &h264_logic_encode_chan->encode_property.encode_feature;
        config_feature_param = &encode_feature->config_feature_param;
        running_feature_param = &encode_feature->running_feature_param;

        memcpy(config_feature_param, running_feature_param, sizeof(isil_h264_encode_feature_param_t));
        config_feature_param->change_mask_flag = 0;

        /*configure rc*/
        encode_rc = &h264_logic_encode_chan->encode_property.encode_rc;
        config_rc = &encode_rc->config_rc;
        running_rc = &encode_rc->running_rc;

        memcpy(config_rc, running_rc, sizeof(isil_h264_encode_rc_config_t));
        config_rc->change_mask_flag = 0;

        return ISIL_OK;
    }

    return ISIL_ERR; 
}

static fsm_state_transfer_matrix_table_t    h264_encode_driver_fsm_state_transfer_matrix_table = {
    .action = {
        {
            h264_encode_driver_in_unregister_state_recv_close_event, 
            no_op,
            h264_encode_driver_in_unregister_state_recv_suspend_event,
            h264_encode_driver_in_unregister_state_recv_resume_event,
            h264_encode_driver_in_unregister_state_recv_open_event,
            no_op
        },
        {
            h264_encode_driver_in_idle_state_recv_close_event, 
            h264_encode_driver_in_idle_state_recv_timeout_event,
            h264_encode_driver_in_idle_state_recv_suspend_event,
            h264_encode_driver_in_idle_state_recv_resume_event,
            no_op,
            h264_encode_driver_in_idle_state_recv_deliver_event
        },
        {
            h264_encode_driver_in_standby_state_recv_close_event, 
            h264_encode_driver_in_standby_state_recv_timeout_event,
            h264_encode_driver_in_standby_state_recv_suspend_event,
            h264_encode_driver_in_standby_state_recv_resume_event,
            no_op,
            h264_encode_driver_in_standby_state_recv_deliver_event
        },
        {
            h264_encode_driver_in_suspend_state_recv_close_event,
            no_op,
            h264_encode_driver_in_suspend_state_recv_suspend_event,
            h264_encode_driver_in_suspend_state_recv_resume_event,
            no_op,
            no_op
        },
        {
            h264_encode_driver_in_running_state_recv_close_event,
            h264_encode_driver_in_running_state_recv_timeout_event,
            h264_encode_driver_in_running_state_recv_suspend_event,
            h264_encode_driver_in_running_state_recv_resume_event,
            no_op,
            h264_encode_driver_in_running_state_recv_deliver_event
        },
        {
            h264_encode_driver_in_transfering_state_recv_close_event,
            h264_encode_driver_in_transfering_state_recv_timeout_event,
            h264_encode_driver_in_transfering_state_recv_suspend_event,
            h264_encode_driver_in_transfering_state_recv_resume_event,
            no_op,
            h264_encode_driver_in_transfering_state_recv_deliver_event
        },
        {
            h264_encode_driver_in_done_state_recv_close_event,
            h264_encode_driver_in_done_state_recv_timeout_event,
            h264_encode_driver_in_done_state_recv_suspend_event,
            h264_encode_driver_in_done_state_recv_resume_event,
            no_op,
            h264_encode_driver_in_done_state_recv_deliver_event
        },
    },
    .sync_hook = NULL,
    .sync_config_to_running_hook = h264_encode_driver_sync_hook,
    .sync_running_to_config_hook = h264_encode_driver_sync_running_to_config_hook,
};


static void init_isil_h264_phy_video_slot(isil_h264_phy_video_slot_t *, int slot_id, enum ISIL_VIDEO_SIZE video_size);

static void register_logic_encode_chan_into_master_slot(isil_h264_logic_encode_chan_t *h264_logic_encode_chan, isil_h264_phy_video_slot_t *master_slot)
{
    if((h264_logic_encode_chan!=NULL) && (master_slot!=NULL)){
        isil_register_table_t   *logic_chan_table = &master_slot->logic_chan_table;
        ed_tcb_t                *logic_chan_ed = &h264_logic_encode_chan->logic_chan_ed;
        isil_register_node_t    *register_node = &logic_chan_ed->ed;

        logic_chan_table->op->register_node_into_table(logic_chan_table, register_node);
    }
}

static __used void find_register_logic_encode_chan_in_master_slot(isil_h264_phy_video_slot_t *master_slot, int logic_chan_id, isil_h264_logic_encode_chan_t **ptr_h264_logic_encode_chan) 
{
    if((master_slot!=NULL) && (ptr_h264_logic_encode_chan!=NULL)){
        isil_register_table_t *logic_chan_table = &master_slot->logic_chan_table;
        isil_register_node_t  *register_node;

        *ptr_h264_logic_encode_chan = NULL;
        logic_chan_table->op->find_register_node_in_table(logic_chan_table, &register_node, logic_chan_id);
        if(register_node != NULL){
            ed_tcb_t    *logic_chan_ed;
            logic_chan_ed = to_get_endpoint_tcb_with_register_node(register_node);
            *ptr_h264_logic_encode_chan = to_get_isil_h264_encode_chan_with_logic_chan_ed(logic_chan_ed);
        }
    }
}

static __used void unregister_logic_encode_chan_into_master_slot(isil_h264_logic_encode_chan_t *h264_logic_encode_chan, isil_h264_phy_video_slot_t *master_slot)
{
    if((h264_logic_encode_chan!=NULL) && (master_slot!=NULL)){
        isil_register_table_t   *logic_chan_table = &master_slot->logic_chan_table;
        ed_tcb_t                *logic_chan_ed = &h264_logic_encode_chan->logic_chan_ed;
        isil_register_node_t    *register_node = &logic_chan_ed->ed;

        register_node->op->all_complete_done(register_node);
        logic_chan_table->op->unregister_node_from_table(logic_chan_table, register_node);
    }
}

static void register_opened_logic_encode_chan_into_master_slot(isil_h264_logic_encode_chan_t *h264_logic_encode_chan, isil_h264_phy_video_slot_t *master_slot)
{
    if((h264_logic_encode_chan!=NULL) && (master_slot!=NULL)){
        isil_register_table_t   *opened_logic_chan_table = &master_slot->opened_logic_chan_table;
        ed_tcb_t                *opened_logic_chan_ed = &h264_logic_encode_chan->opened_logic_chan_ed;
        isil_register_node_t    *register_node = &opened_logic_chan_ed->ed;

        h264_logic_encode_chan->encode_control.op->reset(&h264_logic_encode_chan->encode_control);
        h264_logic_encode_chan->rc_driver.rc_op->init_rc(h264_logic_encode_chan);
        atomic_inc(&h264_logic_encode_chan->opened_flag);
        opened_logic_chan_table->op->register_node_into_table(opened_logic_chan_table, register_node);
    }
}

void find_register_opened_logic_encode_chan_in_master_slot(isil_h264_phy_video_slot_t *master_slot, int logic_chan_id, isil_h264_logic_encode_chan_t **ptr_h264_logic_encode_chan)
{
    if((master_slot!=NULL) && (ptr_h264_logic_encode_chan!=NULL)){
        isil_register_table_t *opened_logic_chan_table = &master_slot->opened_logic_chan_table;
        isil_register_node_t  *register_node;
        ed_tcb_t            *opened_logic_chan_ed;

        *ptr_h264_logic_encode_chan = NULL;
        opened_logic_chan_table->op->find_register_node_in_table(opened_logic_chan_table, &register_node, logic_chan_id);
        if(register_node != NULL){
            opened_logic_chan_ed = to_get_endpoint_tcb_with_register_node(register_node);
            *ptr_h264_logic_encode_chan = to_get_isil_h264_encode_chan_with_opened_logic_chan_ed(opened_logic_chan_ed);
        }
    }
}

static void unregister_opened_logic_encode_chan_into_master_slot(isil_h264_logic_encode_chan_t *h264_logic_encode_chan, isil_h264_phy_video_slot_t *master_slot)
{
    if((h264_logic_encode_chan!=NULL) && (master_slot!=NULL)){
        isil_chip_t             *chip;
        isil_h264_encode_control_t  *encode_control;
        isil_register_table_t   *opened_logic_chan_table = &master_slot->opened_logic_chan_table;
        ed_tcb_t                *opened_logic_chan_ed = &h264_logic_encode_chan->opened_logic_chan_ed;
        isil_register_node_t    *register_node = &opened_logic_chan_ed->ed;
        isil_ed_fsm_t           *ed_fsm;

        opened_logic_chan_table->op->unregister_node_from_table(opened_logic_chan_table, register_node);
        ed_fsm = &opened_logic_chan_ed->ed_fsm;
        ed_fsm->op->change_state(ed_fsm, ISIL_ED_UNREGISTER);
        chip = h264_logic_encode_chan->chip;
        encode_control = &h264_logic_encode_chan->encode_control;
        if(master_slot->op->slot_ad_is_ready(master_slot, chip, h264_logic_encode_chan)
                || encode_control->op->get_ad_status(encode_control)){
            master_slot->op->slot_clear_ad_ready(master_slot, chip, h264_logic_encode_chan);
            master_slot->op->slot_software_discard_frame(master_slot, chip, h264_logic_encode_chan);
        }
    }
}

static void isil_encode_timestamp_init(isil_timestamp_t *isil_encode_timestamp)
{
    atomic_set(&isil_encode_timestamp->first_sync, 1);
}

static void isil_encode_timestamp_reset(isil_timestamp_t *isil_encode_timestamp)
{
    atomic_set(&isil_encode_timestamp->first_sync, 1);
}

static void isil_encode_timestamp_set_timestamp(isil_timestamp_t *isil_encode_timestamp, int timestamp, char *call_fn_name)
{
    isil_encode_timestamp->total_delta_timer_count = get_system_walltimer();
}

static u32  isil_encode_timestamp_get_timestamp(isil_timestamp_t *isil_encode_timestamp)
{
    return isil_encode_timestamp->total_delta_timer_count;
}

static struct isil_timestamp_operation    isil_encode_timestamp_op = {
    .init = isil_encode_timestamp_init,
    .reset = isil_encode_timestamp_reset,
    .set_timestamp = isil_encode_timestamp_set_timestamp,
    .get_timestamp = isil_encode_timestamp_get_timestamp,
};

void    init_isil_encode_timestamp(isil_timestamp_t *isil_encode_timestamp)
{
    isil_encode_timestamp->op = &isil_encode_timestamp_op;
    isil_encode_timestamp->op->init(isil_encode_timestamp);
}

static int  isil_h264_encode_property_init(isil_h264_encode_property_t *encode_property)
{
    if(encode_property){
        isil_h264_encode_configuration_t *encode_config;

        encode_config = &encode_property->running_encode_property;
        encode_config->changedMask = 0;
        encode_config->enableSubmitMotionVector = 0;
        encode_config->enableMotionVectorAnalyses = 0;
        encode_config->width = WIDTH_FRAME_CIF_PAL;
        encode_config->height = HEIGHT_FRAME_CIF_PAL;
        encode_config->bitrate = DEFAULT_BITRATE_CIFSIZE;
        encode_config->fps = MAX_FRAME_RATE_PAL;
        encode_config->keyFrameIntervals =  MIN_I_P_STRIDE;
        encode_config->gopIntervals = MIN_GOP;
        encode_config->pri = 0;

        encode_config->left_pixel_x = 0;
        encode_config->left_pixel_y = 0;
        encode_config->right_pixel_x = 0;
        encode_config->right_pixel_y = 0;
        encode_config->imageLevel = 0;

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int  isil_h264_encode_property_reset(isil_h264_encode_property_t *encode_property)
{
    if(encode_property){
        isil_h264_encode_configuration_t *encode_config;

        encode_config = &encode_property->running_encode_property;
        encode_config->changedMask = 0;
        encode_config->enableSubmitMotionVector = 0;
        encode_config->enableMotionVectorAnalyses = 0;
        encode_config->width = WIDTH_FRAME_CIF_PAL;
        encode_config->height = HEIGHT_FRAME_CIF_PAL;
        encode_config->bitrate = DEFAULT_BITRATE_CIFSIZE;
        encode_config->fps = MAX_FRAME_RATE_PAL;
        encode_config->keyFrameIntervals =  MIN_I_P_STRIDE;
        encode_config->gopIntervals = MIN_GOP;
        encode_config->pri = 0;

        encode_config->left_pixel_x = 0;
        encode_config->left_pixel_y = 0;
        encode_config->right_pixel_x = 0;
        encode_config->right_pixel_y = 0;
        encode_config->imageLevel = 0;

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_h264_encode_property_is_encodeSize_changed(isil_h264_encode_property_t *encode_property)
{
    isil_h264_encode_configuration_t  *running_param;
    unsigned long   flags;
    int ret = ISIL_ENCODE_PROPERTY_NO_CHANGED;
    spin_lock_irqsave(&encode_property->lock, flags);
    running_param = &encode_property->running_encode_property;
    if(running_param->changedMask & ISIL_ENCODE_CONFIG_ENCODE_SIZE_CHANGED){
        running_param->changedMask &= ~ISIL_ENCODE_CONFIG_ENCODE_SIZE_CHANGED;
        ret = ISIL_ENCODE_PROPERTY_CHANGED;
    }
    spin_unlock_irqrestore(&encode_property->lock, flags);
    return ret;
}

static int  isil_h264_encode_property_get_encodeSize_width(isil_h264_encode_property_t *encode_property)
{
    return encode_property->running_encode_property.width;
}

static int  isil_h264_encode_property_get_encodeSize_height(isil_h264_encode_property_t *encode_property)
{
    return encode_property->running_encode_property.height;
}

static int  isil_h264_encode_property_is_target_bitrate_changed(isil_h264_encode_property_t *encode_property)
{
    isil_h264_encode_configuration_t  *running_param;
    unsigned long   flags;
    int ret = ISIL_ENCODE_PROPERTY_NO_CHANGED;
    spin_lock_irqsave(&encode_property->lock, flags);
    running_param = &encode_property->running_encode_property;
    if(running_param->changedMask & ISIL_ENCODE_CONFIG_BITRATE_CHANGED){
        running_param->changedMask &= ~ISIL_ENCODE_CONFIG_BITRATE_CHANGED;
        ret = ISIL_ENCODE_PROPERTY_CHANGED;
    }
    spin_unlock_irqrestore(&encode_property->lock, flags);
    return ret;
}

static int  isil_h264_encode_property_get_target_bitrate(isil_h264_encode_property_t *encode_property)
{
    return encode_property->running_encode_property.bitrate;
}

static int  isil_h264_encode_property_is_target_fps_changed(isil_h264_encode_property_t *encode_property)
{
    isil_h264_encode_configuration_t  *running_param;
    unsigned long   flags;
    int ret = ISIL_ENCODE_PROPERTY_NO_CHANGED;
    spin_lock_irqsave(&encode_property->lock, flags);
    running_param = &encode_property->running_encode_property;
    if(running_param->changedMask & ISIL_ENCODE_CONFIG_FPS_CHANGED){
        running_param->changedMask &= ~ISIL_ENCODE_CONFIG_FPS_CHANGED;
        ret = ISIL_ENCODE_PROPERTY_CHANGED;
    }
    spin_unlock_irqrestore(&encode_property->lock, flags);
    return ret;
}

static int  isil_h264_encode_property_get_target_fps(isil_h264_encode_property_t *encode_property)
{
    return H264_MEDIAN(1, MAX_FRAME_RATE_NTSC, encode_property->running_encode_property.fps);
}

static int  isil_h264_encode_property_is_keyFrameIntervals_changed(isil_h264_encode_property_t *encode_property)
{
    isil_h264_encode_configuration_t  *running_param;
    unsigned long   flags;
    int ret = ISIL_ENCODE_PROPERTY_NO_CHANGED;
    spin_lock_irqsave(&encode_property->lock, flags);
    running_param = &encode_property->running_encode_property;
    if(running_param->changedMask & ISIL_ENCODE_CONFIG_KEYFRAME_INTERVALS_CHANGED){
        running_param->changedMask &= ~ISIL_ENCODE_CONFIG_KEYFRAME_INTERVALS_CHANGED;
        ret = ISIL_ENCODE_PROPERTY_CHANGED;
    }
    spin_unlock_irqrestore(&encode_property->lock, flags);
    return ret;
}

static int  isil_h264_encode_property_get_keyFrameIntervals(isil_h264_encode_property_t *encode_property)
{
    return encode_property->running_encode_property.keyFrameIntervals;
}

static int  isil_h264_encode_property_is_gopIntervals_changed(isil_h264_encode_property_t *encode_property)
{
    isil_h264_encode_configuration_t  *running_param;
    unsigned long   flags;
    int ret = ISIL_ENCODE_PROPERTY_NO_CHANGED;
    spin_lock_irqsave(&encode_property->lock, flags);
    running_param = &encode_property->running_encode_property;
    if(running_param->changedMask & ISIL_ENCODE_CONFIG_GOP_INTERVALS_CHANGED){
        running_param->changedMask &= ~ISIL_ENCODE_CONFIG_GOP_INTERVALS_CHANGED;
        ret = ISIL_ENCODE_PROPERTY_CHANGED;
    }
    spin_unlock_irqrestore(&encode_property->lock, flags);
    return ret;
}

static int  isil_h264_encode_property_get_gopIntervals(isil_h264_encode_property_t *encode_property)
{
    return encode_property->running_encode_property.gopIntervals;
}

static int  isil_h264_encode_property_is_iamgeLevel_changed(isil_h264_encode_property_t *encode_property)
{
    isil_h264_encode_configuration_t  *running_param;
    unsigned long   flags;
    int ret = ISIL_ENCODE_PROPERTY_NO_CHANGED;
    spin_lock_irqsave(&encode_property->lock, flags);
    running_param = &encode_property->running_encode_property;
    if(running_param->changedMask & ISIL_ENCODE_CONFIG_IMAGE_LEVEL_CHANGED){
        running_param->changedMask &= ~ISIL_ENCODE_CONFIG_IMAGE_LEVEL_CHANGED;
        ret = ISIL_ENCODE_PROPERTY_CHANGED;
    }
    spin_unlock_irqrestore(&encode_property->lock, flags);
    return ret;
}

static int  isil_h264_encode_property_get_iamgeLevel(isil_h264_encode_property_t *encode_property)
{
    return encode_property->running_encode_property.imageLevel;
}

static struct isil_h264_encode_property_operation	isil_h264_master_encode_property_op = {
    .init = isil_h264_encode_property_init,
    .reset = isil_h264_encode_property_reset,

    .is_encodeSize_changed = isil_h264_encode_property_is_encodeSize_changed,
    .get_encodeSize_width = isil_h264_encode_property_get_encodeSize_width,
    .get_encodeSize_height = isil_h264_encode_property_get_encodeSize_height,
    .is_target_bitrate_changed = isil_h264_encode_property_is_target_bitrate_changed,
    .get_target_bitrate = isil_h264_encode_property_get_target_bitrate,
    .is_target_fps_changed = isil_h264_encode_property_is_target_fps_changed,
    .get_target_fps = isil_h264_encode_property_get_target_fps,
    .is_keyFrameIntervals_changed = isil_h264_encode_property_is_keyFrameIntervals_changed,
    .get_keyFrameIntervals = isil_h264_encode_property_get_keyFrameIntervals,
    .is_gopIntervals_changed = isil_h264_encode_property_is_gopIntervals_changed,
    .get_gopIntervals = isil_h264_encode_property_get_gopIntervals,
    .is_iamgeLevel_changed = isil_h264_encode_property_is_iamgeLevel_changed,
    .get_iamgeLevel = isil_h264_encode_property_get_iamgeLevel,
};

static int isil_h264_encode_feature_init(isil_h264_encode_feature_t *encode_feature)
{
    if(encode_feature) {
        isil_h264_encode_feature_param_t *feature_param = &encode_feature->running_feature_param;

        feature_param->b_enable_deinterlace = ISIL_H264_FEATURE_OFF;
        feature_param->b_enable_half_pixel  = ISIL_H264_FEATURE_OFF;
        feature_param->b_enable_I_4X4       = ISIL_H264_FEATURE_OFF;
        feature_param->b_enable_quarter_pixel = ISIL_H264_FEATURE_OFF;
        feature_param->b_enable_skip        = ISIL_H264_FEATURE_OFF;
        feature_param->i_mb_delay_value     = MB_DELAY_VALUE;
        feature_param->change_mask_flag     = 0x0;
    }
    return 0;
}

static int isil_h264_encode_feature_reset(isil_h264_encode_feature_t *encode_feature)
{
    if(encode_feature) {
        isil_h264_encode_feature_param_t *feature_param = &encode_feature->running_feature_param;

        feature_param->b_enable_deinterlace = ISIL_H264_FEATURE_OFF;
        feature_param->b_enable_half_pixel  = ISIL_H264_FEATURE_OFF;
        feature_param->b_enable_I_4X4       = ISIL_H264_FEATURE_OFF;
        feature_param->b_enable_quarter_pixel = ISIL_H264_FEATURE_OFF;
        feature_param->b_enable_skip        = ISIL_H264_FEATURE_OFF;
        feature_param->i_mb_delay_value     = MB_DELAY_VALUE;
        feature_param->change_mask_flag     = 0x0;
    }
    return 0;
}

static int isil_h264_encode_feature_get_deinterlace(isil_h264_encode_feature_t *encode_feature)
{
    if(encode_feature) {
        isil_h264_encode_feature_param_t *feature_param = &encode_feature->running_feature_param;
        return feature_param->b_enable_deinterlace;
    }
    return ISIL_H264_FEATURE_OFF;
}

static int isil_h264_encode_feature_get_half_pixel(isil_h264_encode_feature_t *encode_feature)
{
    if(encode_feature) {
        isil_h264_encode_feature_param_t *feature_param = &encode_feature->running_feature_param;
        return feature_param->b_enable_half_pixel;
    }
    return ISIL_H264_FEATURE_OFF;
}

static int isil_h264_encode_feature_get_i_4x4(isil_h264_encode_feature_t *encode_feature)
{
    if(encode_feature) {
        isil_h264_encode_feature_param_t *feature_param = &encode_feature->running_feature_param;
        return feature_param->b_enable_I_4X4;
    }
    return ISIL_H264_FEATURE_OFF;
}

static u32 isil_h264_encode_feature_get_mb_delay(isil_h264_encode_feature_t *encode_feature)
{
    if(encode_feature) {
        isil_h264_encode_feature_param_t *feature_param = &encode_feature->running_feature_param;
        return feature_param->i_mb_delay_value;
    }
    return 0x430;
}

static int isil_h264_encode_feature_get_quarter_pixel(isil_h264_encode_feature_t *encode_feature)
{
    if(encode_feature) {
        isil_h264_encode_feature_param_t *feature_param = &encode_feature->running_feature_param;
        return feature_param->b_enable_quarter_pixel;
    }

    return ISIL_H264_FEATURE_OFF;
}

static int isil_h264_encode_feature_get_skip(isil_h264_encode_feature_t *encode_feature)
{
    if(encode_feature) {
        isil_h264_encode_feature_param_t *feature_param = &encode_feature->running_feature_param;
        return feature_param->b_enable_skip;
    }
    return ISIL_H264_FEATURE_OFF;
}

static isil_h264_encode_feature_operation_t isil_h264_encode_feature_operation_op = {
    .init                  = isil_h264_encode_feature_init,
    .reset                 = isil_h264_encode_feature_reset,

    .get_deinterlace       = isil_h264_encode_feature_get_deinterlace,
    .get_half_pixel        = isil_h264_encode_feature_get_half_pixel,
    .get_i_4x4             = isil_h264_encode_feature_get_i_4x4,
    .get_mb_delay          = isil_h264_encode_feature_get_mb_delay,
    .get_quarter_pixel     = isil_h264_encode_feature_get_quarter_pixel,
    .get_skip              = isil_h264_encode_feature_get_skip,
};

static int isil_h264_encode_rc_init(isil_h264_encode_rc_t *rc)
{
    if(rc){
        isil_h264_encode_rc_config_t *running_rc;

        running_rc = &rc->running_rc;
        running_rc->change_mask_flag = 0;
        running_rc->e_rc_type = ISIL_H264_CBR;
        running_rc->e_image_priority = ISIL_RC_IMAGE_QUALITY_FIRST;
        running_rc->i_qpi = DEFAULT_QP;
        running_rc->i_qpp = DEFAULT_QP;
        running_rc->i_qpb = DEFAULT_QP;

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_h264_encode_rc_reset(isil_h264_encode_rc_t *rc)
{
    if(rc){
        isil_h264_encode_rc_config_t *running_rc;

        running_rc = &rc->running_rc;
        running_rc->change_mask_flag = 0;
        running_rc->e_rc_type = ISIL_H264_CBR;
        running_rc->e_image_priority = ISIL_RC_IMAGE_QUALITY_FIRST;
        running_rc->i_qpi = DEFAULT_QP;
        running_rc->i_qpp = DEFAULT_QP;
        running_rc->i_qpb = DEFAULT_QP;

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static u32 isil_h264_encode_rc_get_rc_type(isil_h264_encode_rc_t *rc)
{
    if(rc){
        isil_h264_encode_rc_config_t *running_rc;

        running_rc = &rc->running_rc;
        return running_rc->e_rc_type;
    }

    return ISIL_ERR;
}

static u32 isil_h264_encode_rc_get_image_priority(isil_h264_encode_rc_t *rc)
{
    if(rc){
        isil_h264_encode_rc_config_t *running_rc;

        running_rc = &rc->running_rc;
        return running_rc->e_image_priority;
    }

    return ISIL_ERR;
}

static u32 isil_h264_encode_rc_get_qpi(isil_h264_encode_rc_t *rc)
{
    if(rc){
        isil_h264_encode_rc_config_t *running_rc;

        running_rc = &rc->running_rc;
        return running_rc->i_qpi;
    }

    return ISIL_ERR;
}

static u32 isil_h264_encode_rc_get_qpp(isil_h264_encode_rc_t *rc)
{
    if(rc){
        isil_h264_encode_rc_config_t *running_rc;

        running_rc = &rc->running_rc;
        return running_rc->i_qpp;
    }

    return ISIL_ERR;
}

static u32 isil_h264_encode_rc_get_qpb(isil_h264_encode_rc_t *rc)
{
    if(rc){
        isil_h264_encode_rc_config_t *running_rc;

        running_rc = &rc->running_rc;
        return running_rc->i_qpb;
    }

    return ISIL_ERR;
}

static int isil_h264_encode_rc_is_qpi_changed(isil_h264_encode_rc_t *rc)
{
    if(rc){
        isil_h264_encode_rc_config_t *running_rc;

        running_rc = &rc->running_rc;
        if(running_rc->change_mask_flag & ISIL_H264_ENCODE_RC_ENABLE_CHANGE_QPI_MASK){
            running_rc->change_mask_flag &= ~ISIL_H264_ENCODE_RC_ENABLE_CHANGE_QPI_MASK;
            return ISIL_TRUE;
        }else{
            return ISIL_FALSE;
        }
    }

    return ISIL_ERR;
}

static int isil_h264_encode_rc_is_qpp_changed(isil_h264_encode_rc_t *rc)
{
    if(rc){
        isil_h264_encode_rc_config_t *running_rc;

        running_rc = &rc->running_rc;
        if(running_rc->change_mask_flag & ISIL_H264_ENCODE_RC_ENABLE_CHANGE_QPP_MASK){
            running_rc->change_mask_flag &= ~ISIL_H264_ENCODE_RC_ENABLE_CHANGE_QPP_MASK;
            return ISIL_TRUE;
        }else{
            return ISIL_FALSE;
        }
    }

    return ISIL_ERR;
}

static int isil_h264_encode_rc_is_qpb_changed(isil_h264_encode_rc_t *rc)
{
    if(rc){
        isil_h264_encode_rc_config_t *running_rc;

        running_rc = &rc->running_rc;
        if(running_rc->change_mask_flag & ISIL_H264_ENCODE_RC_ENABLE_CHANGE_QPB_MASK){
            running_rc->change_mask_flag &= ~ISIL_H264_ENCODE_RC_ENABLE_CHANGE_QPB_MASK;
            return ISIL_TRUE;
        }else{
            return ISIL_FALSE;
        }
    }

    return ISIL_ERR;
}

static int isil_h264_encode_rc_is_rc_type_changed(isil_h264_encode_rc_t *rc)
{
    if(rc){
        isil_h264_encode_rc_config_t *running_rc;

        running_rc = &rc->running_rc;
        if(running_rc->change_mask_flag & ISIL_H264_ENCODE_RC_ENABLE_CHANGE_RC_TYPE_MASK){
            running_rc->change_mask_flag &= ~ISIL_H264_ENCODE_RC_ENABLE_CHANGE_RC_TYPE_MASK;
            return ISIL_TRUE;
        }else{
            return ISIL_FALSE;
        }
    }

    return ISIL_ERR;
}

static int isil_h264_encode_rc_is_image_priority_changed(isil_h264_encode_rc_t *rc)
{
    if(rc){
        isil_h264_encode_rc_config_t *running_rc;

        running_rc = &rc->running_rc;
        if(running_rc->change_mask_flag & ISIL_H264_ENCODE_RC_ENABLE_CHANGE_IMAGE_PRIORITY_MASK){
            running_rc->change_mask_flag &= ~ISIL_H264_ENCODE_RC_ENABLE_CHANGE_IMAGE_PRIORITY_MASK;
            return ISIL_TRUE;
        }else{
            return ISIL_FALSE;
        }
    }

    return ISIL_ERR;
}

static struct isil_h264_encode_rc_operation isil_h264_encode_rc_operation_op = {
    .init = isil_h264_encode_rc_init,
    .reset = isil_h264_encode_rc_reset,

    .get_rc_type = isil_h264_encode_rc_get_rc_type,
    .get_image_priority = isil_h264_encode_rc_get_image_priority,
    .get_qpi = isil_h264_encode_rc_get_qpi,
    .get_qpp = isil_h264_encode_rc_get_qpp,
    .get_qpb = isil_h264_encode_rc_get_qpb,
    .is_qpi_changed = isil_h264_encode_rc_is_qpi_changed,
    .is_qpp_changed = isil_h264_encode_rc_is_qpp_changed,
    .is_qpb_changed = isil_h264_encode_rc_is_qpb_changed,
    .is_rc_type_changed = isil_h264_encode_rc_is_rc_type_changed,
    .is_image_priority_changed = isil_h264_encode_rc_is_image_priority_changed,
};

static void init_isil_h264_master_encode_property(isil_h264_encode_property_t *encode_property, isil_h264_encode_configuration_t *config_param)
{
    if(encode_property){
        isil_h264_encode_feature_t  *pfeature;
        isil_h264_encode_rc_t *p_rc;

        /*configure h264 attribute*/
        spin_lock_init(&encode_property->lock);
        encode_property->op = &isil_h264_master_encode_property_op;
        encode_property->op->init(encode_property);
        if(config_param == NULL){
            memcpy(&encode_property->config_encode_property, &default_h264_master_encode_property, sizeof(isil_h264_encode_configuration_t));
        }else{
            memcpy(&encode_property->config_encode_property, config_param, sizeof(isil_h264_encode_configuration_t));
        }

        /*configure feature*/
        pfeature = &encode_property->encode_feature;
        pfeature->op = &isil_h264_encode_feature_operation_op;
        pfeature->op->init(pfeature);
        memcpy(&pfeature->config_feature_param, &default_main_encode_feature, sizeof(isil_h264_encode_feature_param_t));

        /*configure rc*/
        p_rc = &encode_property->encode_rc;
        p_rc->op = &isil_h264_encode_rc_operation_op;
        p_rc->op->init(p_rc);
        memcpy(&p_rc->config_rc, &default_main_encode_rc, sizeof(isil_h264_encode_rc_config_t));
    }
}

static void init_isil_h264_sub_encode_property(isil_h264_encode_property_t *encode_property, isil_h264_encode_configuration_t *config_param)
{
    if(encode_property){
        isil_h264_encode_feature_t  *pfeature;
        isil_h264_encode_rc_t *p_rc;

        /*configure h264 attribute*/
        spin_lock_init(&encode_property->lock);
        encode_property->op = &isil_h264_master_encode_property_op;
        encode_property->op->init(encode_property);
        if(config_param == NULL){
            memcpy(&encode_property->config_encode_property, &default_h264_sub_encode_property, sizeof(isil_h264_encode_configuration_t));
        }else{
            memcpy(&encode_property->config_encode_property, config_param, sizeof(isil_h264_encode_configuration_t));
        }

        /*configure feature*/
        pfeature = &encode_property->encode_feature;
        pfeature->op = &isil_h264_encode_feature_operation_op;
        pfeature->op->init(pfeature);
        memcpy(&pfeature->config_feature_param, &default_sub_encode_feature, sizeof(isil_h264_encode_feature_param_t));

        /*configure rc*/
        p_rc = &encode_property->encode_rc;
        p_rc->op = &isil_h264_encode_rc_operation_op;
        p_rc->op->init(p_rc);
        memcpy(&p_rc->config_rc, &default_sub_encode_rc, sizeof(isil_h264_encode_rc_config_t));
    }
}

static void isil_h264_logic_video_slot_init(isil_h264_logic_video_slot_t *logic_video_slot, int logic_slot_id, enum ISIL_VIDEO_SIZE video_size)
{
    if(logic_video_slot != NULL){
        logic_video_slot->logicSlotId = logic_slot_id;
        logic_video_slot->mapPhySlotId = ISIL_LOGIC_VD_INVALID;
        logic_video_slot->discardTable = 0;
        logic_video_slot->roundTableStride = 0;
        logic_video_slot->video_size = video_size;
        logic_video_slot->enableSlot = 0;
        logic_video_slot->slotFps    = 0;
        logic_video_slot->hor_reverse= 0;
        logic_video_slot->ver_reverse= 0;
        atomic_set(&logic_video_slot->needUpdateFlag, 0);
    }
}

static void isil_h264_logic_video_slot_set_mapPhySlotId(isil_h264_logic_video_slot_t *logic_video_slot, int phy_slot_id)
{
    if(logic_video_slot != NULL) {
        logic_video_slot->mapPhySlotId = phy_slot_id;
    }
}

static int  isil_h264_logic_video_slot_get_mapPhySlotId(isil_h264_logic_video_slot_t *logic_video_slot)
{
    if(logic_video_slot)
    {
        return logic_video_slot->mapPhySlotId;
    }


    return 0;

}

static void isil_h264_logic_video_slot_set_discardTable(isil_h264_logic_video_slot_t *logic_video_slot, u32 discardTable)
{
    if(logic_video_slot)
    {
        logic_video_slot->discardTable = discardTable;
    }

    return ;
}

static u32  isil_h264_logic_video_slot_get_discardTable(isil_h264_logic_video_slot_t *logic_video_slot)
{
    if(logic_video_slot)
    {
        return logic_video_slot->discardTable;
    }


    return 0;

}

static void isil_h264_logic_video_slot_set_roundTableStride(isil_h264_logic_video_slot_t *logic_video_slot, int roundTableStride)
{
    if(logic_video_slot)
    {
        logic_video_slot->roundTableStride = roundTableStride;
    }

    return ;
}

static int  isil_h264_logic_video_slot_get_roundTableStride(isil_h264_logic_video_slot_t *logic_video_slot)
{
    if(logic_video_slot)
    {
        return logic_video_slot->roundTableStride;
    }


    return 0;

}

static void isil_h264_logic_video_slot_clear_needUpdateFlag(isil_h264_logic_video_slot_t *logic_video_slot)
{
    if(logic_video_slot)
    {
        atomic_set(&logic_video_slot->needUpdateFlag, 0);
    }

    return ;
}

static int  isil_h264_logic_video_slot_get_needUpdateFlag(isil_h264_logic_video_slot_t *logic_video_slot)
{
    if(logic_video_slot)
    {
        return atomic_read(&logic_video_slot->needUpdateFlag);
    }


    return 0;

}

static int	isil_h264_logic_video_slot_get_video_size(isil_h264_logic_video_slot_t *logic_video_slot)
{
    if(logic_video_slot) {
        return logic_video_slot->video_size;
    }

    ISIL_DBG(ISIL_DBG_ERR, "null pointer\n");
    return ISIL_VIDEO_SIZE_D1;
}

static void	isil_h264_logic_video_slot_set_video_size(isil_h264_logic_video_slot_t *logic_video_slot, enum ISIL_VIDEO_SIZE video_size)
{
    if(logic_video_slot) {
        logic_video_slot->video_size = video_size;
    }
}

static u32 isil_h264_logic_video_slot_get_enableslot(isil_h264_logic_video_slot_t *logic_video_slot)
{
    if(logic_video_slot) {
        return logic_video_slot->enableSlot;
    }

    return 0;
}

static void isil_h264_logic_video_slot_set_enableslot(isil_h264_logic_video_slot_t *logic_video_slot, u32 enable)
{
    if(logic_video_slot) {
        logic_video_slot->enableSlot = enable;
    }
}

static u32 isil_h264_logic_video_slot_get_fps(isil_h264_logic_video_slot_t *logic_video_slot)
{
    if(logic_video_slot) {
        return logic_video_slot->slotFps;
    }

    return 0;
}

static void isil_h264_logic_video_slot_set_fps(isil_h264_logic_video_slot_t *logic_video_slot, u32 fps)
{
    if(logic_video_slot) {
        logic_video_slot->slotFps = fps;
    }
}

static void	isil_h264_logic_video_slot_set_chip_end_logic_video_slot_param(isil_h264_logic_video_slot_t *logic_video_slot, isil_chip_t *chip)
{
    return ;
}

static u32	isil_h264_logic_video_slot_get_chip_end_logic_video_slot_param(isil_h264_logic_video_slot_t *logic_video_slot, isil_chip_t *chip)
{


    return 0;

}

static struct isil_h264_logic_video_slot_operation	isil_h264_logic_video_slot_op = {
    .init = isil_h264_logic_video_slot_init,
    .set_mapPhySlotId = isil_h264_logic_video_slot_set_mapPhySlotId,
    .get_mapPhySlotId = isil_h264_logic_video_slot_get_mapPhySlotId,
    .set_discardTable = isil_h264_logic_video_slot_set_discardTable,
    .get_discardTable = isil_h264_logic_video_slot_get_discardTable,
    .set_roundTableStride = isil_h264_logic_video_slot_set_roundTableStride,
    .get_roundTableStride = isil_h264_logic_video_slot_get_roundTableStride,
    .clear_needUpdateFlag = isil_h264_logic_video_slot_clear_needUpdateFlag,
    .get_needUpdateFlag = isil_h264_logic_video_slot_get_needUpdateFlag,
    .get_video_size = isil_h264_logic_video_slot_get_video_size,
    .set_video_size = isil_h264_logic_video_slot_set_video_size,
    .get_enableslot = isil_h264_logic_video_slot_get_enableslot,
    .set_enableslot = isil_h264_logic_video_slot_set_enableslot,
    .get_fps        = isil_h264_logic_video_slot_get_fps,
    .set_fps        = isil_h264_logic_video_slot_set_fps,

    .set_chip_end_logic_video_slot_param = isil_h264_logic_video_slot_set_chip_end_logic_video_slot_param,
    .get_chip_end_logic_video_slot_param = isil_h264_logic_video_slot_get_chip_end_logic_video_slot_param,
};

static int config_senif_bus(isil_vd_cross_bus_t *vd_bus, int bus_id, int mode)
{
    u32 val = 0, i = 0, full_size = 0, partab_select = 0;
    u32 norm;
    isil_chip_t *chip;
    chip_driver_t *driver;
    isil_video_bus_t *video_bus;
    isil_h264_logic_video_slot_t *logic_video_slot;
    isil_h264_phy_video_slot_t   *phy_video_slot;
    u32 temp, width_parta, width_partb, height_parta, height_partb;

    if((bus_id < 0) || (bus_id >= ISIL_MAX_GRP))
    {
        ISIL_DBG(ISIL_DBG_ERR, "invalid argument\n");
        return -EINVAL;
    }

    chip = to_get_chip_with_chip_cross_bus(vd_bus);
    driver = chip->chip_driver;
    video_bus = &driver->video_bus;
    norm = video_bus->op->get_video_standard(video_bus);
    switch(mode)
    {
        default:
        case MODE_D1:
        case MODE_D1_HALFD1:
        case MODE_HALFD1:
            temp = 0x1c;
            width_parta = VIDEO_SIZE_TO_WIDTH(ISIL_VIDEO_SIZE_D1, norm) - 1;
            width_partb = VIDEO_SIZE_TO_WIDTH(ISIL_VIDEO_SIZE_D1, norm) - 1;
            height_parta= VIDEO_SIZE_TO_HEIGHT(ISIL_VIDEO_SIZE_D1, norm) - 1;
            height_partb= VIDEO_SIZE_TO_HEIGHT(ISIL_VIDEO_SIZE_HALF_D1, norm) - 1;
            break;
        case MODE_HALFD1_CIF:
        case MODE_D1_CIF:
            temp = 0xe;
            width_parta = VIDEO_SIZE_TO_WIDTH(ISIL_VIDEO_SIZE_D1, norm) - 1;
            width_partb = VIDEO_SIZE_TO_WIDTH(ISIL_VIDEO_SIZE_CIF, norm) - 1;
            height_parta= VIDEO_SIZE_TO_HEIGHT(ISIL_VIDEO_SIZE_D1, norm) - 1;
            height_partb= VIDEO_SIZE_TO_HEIGHT(ISIL_VIDEO_SIZE_CIF, norm) - 1;
            break;
        case MODE_CIF:
            temp = 0x7;
            width_parta = VIDEO_SIZE_TO_WIDTH(ISIL_VIDEO_SIZE_CIF, norm) - 1;
            width_partb = VIDEO_SIZE_TO_WIDTH(ISIL_VIDEO_SIZE_CIF, norm) - 1;
            height_parta= VIDEO_SIZE_TO_HEIGHT(ISIL_VIDEO_SIZE_CIF, norm) - 1;
            height_partb= VIDEO_SIZE_TO_HEIGHT(ISIL_VIDEO_SIZE_CIF, norm) - 1;
            break;
    }
    if(bus_id < 2){
        val = chip->io_op->chip_read32(chip, SENIF_BUS01_MODE);
        if(bus_id%2){
            val &= 0x1f;
            val |= temp<<8;
        }
        else{
            val &= 0xffe0;
            val |= temp;
        }
        chip->io_op->chip_write32(chip, SENIF_BUS01_MODE, val);
    }
    else {
        val = chip->io_op->chip_read32(chip, SENIF_BUS23_MODE);
        if(bus_id%2){
            val &= 0x1f;
            val |= temp<<8;
        }
        else{
            val &= 0xffe00;
            val |= temp;
        }
        chip->io_op->chip_write32(chip, SENIF_BUS23_MODE, val);
    }

    chip->io_op->chip_write32(chip, SENIF_WIDTH_PARTA(bus_id), width_parta);
    chip->io_op->chip_write32(chip, SENIF_WIDTH_PARTB(bus_id), width_partb);
    chip->io_op->chip_write32(chip, SENIF_HEIGHT_PARTA(bus_id), height_parta);
    chip->io_op->chip_write32(chip, SENIF_HEIGHT_PARTB(bus_id), height_partb);
    temp = chip->io_op->chip_read32(chip, SENIF_HOR_MIR);
    val = chip->io_op->chip_read32(chip, SENIF_VER_MIR);
    full_size = chip->io_op->chip_read32(chip, SENIF_FULL_FLAGS);
    partab_select = chip->io_op->chip_read32(chip, SENIF_PARTAB_SELECT);
    phy_video_slot = vd_bus->phy_video_slot;
    for(i = 0; i < ISIL_PHY_VD_CHAN_NUMBER; i++, phy_video_slot++) {
        logic_video_slot = phy_video_slot->cross_bus_logic_video_slot;
        if(logic_video_slot->hor_reverse) {
            temp |= (1 << logic_video_slot->logicSlotId);
        }else{
            temp &= ~(1 << logic_video_slot->logicSlotId);
        }
        if(logic_video_slot->ver_reverse) {
            val |= (1 << logic_video_slot->logicSlotId);
        }else{
            val &= ~(1 << logic_video_slot->logicSlotId);
        }
        switch(logic_video_slot->video_size) {
            default:
            case ISIL_VIDEO_SIZE_4CIF:
            case ISIL_VIDEO_SIZE_D1:
                full_size |=  (1 << logic_video_slot->logicSlotId);
                partab_select &= ~(1 << logic_video_slot->logicSlotId);
                break;
            case ISIL_VIDEO_SIZE_CIF:
                full_size |=  (1 << logic_video_slot->logicSlotId);
                if(mode == MODE_CIF){
                    partab_select &= ~(1 << logic_video_slot->logicSlotId);
                }else{
                    partab_select |= (1 << logic_video_slot->logicSlotId);
                }
                break;
            case ISIL_VIDEO_SIZE_HALF_D1:
            case ISIL_VIDEO_SIZE_2CIF:
                partab_select &= ~(1 << logic_video_slot->logicSlotId);
                full_size &=  ~(1 << logic_video_slot->logicSlotId);
                break;
        }
    }
    chip->io_op->chip_write32(chip, SENIF_HOR_MIR, temp);
    chip->io_op->chip_write32(chip, SENIF_VER_MIR, val);
    chip->io_op->chip_write32(chip, SENIF_FULL_FLAGS, full_size);
    chip->io_op->chip_write32(chip, SENIF_PARTAB_SELECT, partab_select);

    ISIL_DBG(ISIL_DBG_INFO, "senif: bus%d mode %d, (0x%x, 0x%x, 0x%x, 0x%x), partab 0x%08x, full 0x%08x\n", bus_id, mode, 
            width_parta, width_partb, height_parta, height_partb, partab_select, full_size);

    return 0;
}

static void	init_isil_h264_logic_video_slot(isil_h264_logic_video_slot_t *logic_video_slot, int logic_id, enum ISIL_VIDEO_SIZE video_size)
{
    logic_video_slot->op = &isil_h264_logic_video_slot_op;
    logic_video_slot->op->init(logic_video_slot, logic_id, video_size);
}

static int	isil_vd_cross_bus_init(isil_vd_cross_bus_t *vd_cross_bus, enum ISIL_CROSS_BUS_WORK_MODE mode, enum ISIL_VIDEO_STANDARD	video_standard)
{

    int phy_bus_id[ISIL_PHY_VD_CHAN_NUMBER];

    memset(phy_bus_id, 0, sizeof(int) * ISIL_PHY_VD_CHAN_NUMBER);

    if(vd_cross_bus != NULL){
        isil_h264_logic_video_slot_t	*logic_video_slot;
        isil_h264_phy_video_slot_t	*phy_video_slot;
        vd_chan_map_info_t	chan_map_table[ISIL_PHY_VD_CHAN_NUMBER], *chan_map;
        isil_chip_t	*chip = NULL;
        enum ISIL_VIDEO_SIZE video_size;
        u32  local_fps = 0;
        int	i;

        chip = to_get_chip_with_chip_cross_bus(vd_cross_bus);
        vd_cross_bus->op->update_work_mode(vd_cross_bus, mode);
        chan_map = chan_map_table;
        if(video_standard == ISIL_VIDEO_STANDARD_PAL) {
            local_fps = 25;
        }else{
            local_fps = 30;
        }
        switch(vd_cross_bus->op->get_work_mode(vd_cross_bus)){
            case ISIL_CROSS_BUS_4D1_REALTIME:
                video_size = ISIL_VIDEO_SIZE_D1;
                for(i=0; i<4; i++, chan_map++){
                    chan_map->phy_slot_id = i;
                    chan_map->fps = local_fps;
                    chan_map->video_size = ISIL_VIDEO_SIZE_D1;
                }

                for(;i<ISIL_PHY_VD_CHAN_NUMBER; i++, chan_map++){
                    chan_map->phy_slot_id = ISIL_LOGIC_VD_INVALID;
                }
                break;
            case ISIL_CROSS_BUS_8HalfD1_REALTIME:
                video_size = ISIL_VIDEO_SIZE_HALF_D1;
                for(i=0; i<8; i++, chan_map++){
                    chan_map->phy_slot_id = i;
                    chan_map->fps = local_fps;
                    chan_map->video_size = ISIL_VIDEO_SIZE_HALF_D1;
                }
                for(; i < ISIL_PHY_VD_CHAN_NUMBER; i++, chan_map++){
                    chan_map->phy_slot_id = ISIL_LOGIC_VD_INVALID;
                }
                break;
            case ISIL_CROSS_BUS_16CIF_REALTIME:
                video_size = ISIL_VIDEO_SIZE_CIF;
                for(i=0; i<ISIL_PHY_VD_CHAN_NUMBER; i++, chan_map++){
                    chan_map->phy_slot_id = i;
                    chan_map->fps = local_fps;
                    chan_map->video_size = ISIL_VIDEO_SIZE_CIF;
                }
                break;
            default:
            case ISIL_CROSS_BUS_UNREALTIME:
                video_size = ISIL_VIDEO_SIZE_D1;
                for(i=0; i<ISIL_PHY_VD_CHAN_NUMBER; i++, chan_map++){
                    chan_map->phy_slot_id = i;
                    chan_map->fps = default_nonreal_time_fps_table[i];
                    chan_map->video_size = default_nonreal_time_fmt_table[i];
                }
                break;
        }

        phy_video_slot = vd_cross_bus->phy_video_slot;
        logic_video_slot = vd_cross_bus->logic_video_slot;
        for(i=0; i<ISIL_PHY_VD_CHAN_NUMBER; i++){
            init_isil_h264_phy_video_slot(phy_video_slot, i, ISIL_VIDEO_SIZE_D1);
            phy_video_slot->op->set_video_size(phy_video_slot, video_size);
            init_isil_h264_logic_video_slot(logic_video_slot, i, ISIL_VIDEO_SIZE_D1);
            logic_video_slot->op->set_video_size(logic_video_slot, video_size);
            phy_video_slot++;
            logic_video_slot++;
        }

        if(vd_cross_bus->op->calculate_cross_bus_map_table(vd_cross_bus, chan_map_table)){
            ISIL_DBG(ISIL_DBG_ERR, "calculate map table failed, please check parameter!\n");
            return -EINVAL;
        }
        chan_map = chan_map_table;
        for(i = 0; i < ISIL_CROSS_BUS_LOGIC_VD_CHAN_NUMBER; i++, chan_map++) {
            if(chan_map->map_logic_slot_id >= ISIL_CROSS_BUS_LOGIC_VD_CHAN_NUMBER) {
                ISIL_DBG(ISIL_DBG_ERR, "logic channel number %d overflow\n", chan_map->map_logic_slot_id);
                return -ISIL_ERR;
            }
            logic_video_slot = &vd_cross_bus->logic_video_slot[chan_map->map_logic_slot_id];
            logic_video_slot->op->set_mapPhySlotId(logic_video_slot, chan_map->phy_slot_id);
            logic_video_slot->op->set_discardTable(logic_video_slot, chan_map->logic_slot_discard_table);
            logic_video_slot->op->set_roundTableStride(logic_video_slot, chan_map->roundTableStride);
            logic_video_slot->op->set_video_size(logic_video_slot, chan_map->video_size);
            logic_video_slot->op->set_enableslot(logic_video_slot, chan_map->enable);
            logic_video_slot->op->set_fps(logic_video_slot, chan_map->fps);
            atomic_set(&logic_video_slot->needUpdateFlag, 1);
            vd_cross_bus->op->get_phy_video_slot_by_phy_id(vd_cross_bus, chan_map->phy_slot_id, &phy_video_slot);
            phy_video_slot->op->update_map_logic_slot(phy_video_slot, logic_video_slot->logicSlotId, logic_video_slot);
        }

        return ISIL_OK;
    }

    ISIL_DBG(ISIL_DBG_ERR, "null pointer\n");

    return ISIL_ERR;
}

static void	isil_vd_cross_bus_get_logic_video_slot_by_logic_id(isil_vd_cross_bus_t *vd_cross_bus, int logic_slot_id, isil_h264_logic_video_slot_t **ptr_logic_video_slot)
{
    if((ptr_logic_video_slot!=NULL) && (logic_slot_id<ISIL_CROSS_BUS_LOGIC_VD_CHAN_NUMBER)){
        *ptr_logic_video_slot = &vd_cross_bus->logic_video_slot[logic_slot_id];
    }
}

static void	isil_vd_cross_bus_get_logic_video_slot_by_phy_id(isil_vd_cross_bus_t *vd_cross_bus, int phy_slot_id, isil_h264_logic_video_slot_t **ptr_logic_video_slot)
{
    if(vd_cross_bus) {
        isil_h264_phy_video_slot_t	*phy_video_slot;
        if((phy_slot_id < 0) || (phy_slot_id >= ISIL_PHY_VD_CHAN_NUMBER)) {
            *ptr_logic_video_slot = NULL;
            return;
        }
        phy_video_slot = &vd_cross_bus->phy_video_slot[phy_slot_id];
        *ptr_logic_video_slot = phy_video_slot->cross_bus_logic_video_slot;
    }
}

static void	isil_vd_cross_bus_get_phy_video_slot_by_logic_id(isil_vd_cross_bus_t *vd_cross_bus, int logic_slot_id, isil_h264_phy_video_slot_t **ptr_phy_video_slot)
{
    if((ptr_phy_video_slot!=NULL) && (logic_slot_id<ISIL_CROSS_BUS_LOGIC_VD_CHAN_NUMBER)){
        isil_h264_phy_video_slot_t	*phy_video_slot;
        int	i;

        for(i=0; i<ISIL_PHY_VD_CHAN_NUMBER; i++){
            phy_video_slot = &vd_cross_bus->phy_video_slot[i];
            if(phy_video_slot->map_logic_id != ISIL_LOGIC_VD_INVALID){
                if(phy_video_slot->map_logic_id == logic_slot_id){
                    *ptr_phy_video_slot = phy_video_slot;
                    return;
                }
            }
        }
    }
}

static void	isil_vd_cross_bus_get_phy_video_slot_by_phy_id(isil_vd_cross_bus_t *vd_cross_bus, int phy_slot_id, isil_h264_phy_video_slot_t **ptr_phy_video_slot)
{
    if((ptr_phy_video_slot!=NULL) && (phy_slot_id<ISIL_PHY_VD_CHAN_NUMBER)){
        *ptr_phy_video_slot = &vd_cross_bus->phy_video_slot[phy_slot_id];
    }
}


static int	isil_vd_cross_bus_get_work_mode(isil_vd_cross_bus_t *vd_cross_bus)
{
    return vd_cross_bus->mode;
}

static int isil_vd_cross_bus_update_work_mode(isil_vd_cross_bus_t *vd_cross_bus, enum ISIL_CROSS_BUS_WORK_MODE mode)
{
    if(!vd_cross_bus) {
        ISIL_DBG(ISIL_DBG_ERR, "null pointer\n");
        return -EINVAL;
    }

    if((mode < ISIL_CROSS_BUS_4D1_REALTIME) || (mode > ISIL_CROSS_BUS_UNREALTIME)){
        ISIL_DBG(ISIL_DBG_ERR, "unsupport work mode %d\n", mode);
        return -EINVAL;
    }

    ISIL_DBG(ISIL_DBG_INFO, "update vd bus work mode %d\n", mode);
    vd_cross_bus->mode = mode;

    return ISIL_OK;
}

static int	isil_vd_cross_bus_calculate_cross_bus_map_table(isil_vd_cross_bus_t *vd_cross_bus, vd_chan_map_info_t *chan_map_info)

{
    VGROUP_S vgroup[ISIL_MAX_GRP];
    int chn_no[ISIL_MAX_VI];
    int chn_cnt = 0;
    int format[ISIL_MAX_VI];
    int frate[ISIL_MAX_VI];
    int norm, last_video_size = ISIL_VIDEO_SIZE_D1;
    int i = 0, j = 0, consistency = ISIL_TRUE;
    vd_chan_map_info_t *map_info;
    isil_chip_t *chip;
    chip_driver_t *driver;
    isil_video_bus_t *video_bus;

    if(!vd_cross_bus)
    {
        ISIL_DBG(ISIL_DBG_ERR, "null pointer\n");
        return ISIL_ERR;
    }

    map_info = chan_map_info;
    memset(vgroup, 0x0, sizeof(VGROUP_S) * ISIL_MAX_GRP);
    memset(chn_no, 0x0, sizeof(int)*ISIL_MAX_VI);
    memset(format, 0x0, sizeof(int)*ISIL_MAX_VI);
    memset(frate,  0x0, sizeof(int)*ISIL_MAX_VI);

    for(i = 0; i < ISIL_MAX_GRP; i++)
    {
        vgroup[i].grp_id = i;
    }
    chip = to_get_chip_with_chip_cross_bus(vd_cross_bus);
    driver = chip->chip_driver;
    video_bus = &driver->video_bus;
    norm = video_bus->op->get_video_standard(video_bus);
    chn_cnt = 0;
    for(i = 0; i < ISIL_PHY_VD_CHAN_NUMBER; i++, map_info++)
    {
        if((map_info->phy_slot_id != ISIL_LOGIC_VD_INVALID) && (map_info->enable != 0) && (map_info->fps != 0)) {
            chn_no[chn_cnt] = map_info->phy_slot_id;
            switch(map_info->video_size) {
                case ISIL_VIDEO_SIZE_4CIF:
                case ISIL_VIDEO_SIZE_D1:
                case ISIL_VIDEO_SIZE_2CIF:
                case ISIL_VIDEO_SIZE_HALF_D1:
                case ISIL_VIDEO_SIZE_CIF:
                    break;
                default:
                case ISIL_VIDEO_SIZE_QCIF:
                case ISIL_VIDEO_SIZE_USER:
                    map_info->video_size = ISIL_VIDEO_SIZE_CIF;
                    break;
            }
            format[chn_cnt] = map_info->video_size;
            frate[chn_cnt]  = map_info->fps;
            chn_cnt++;
        }
    }

    if(cb_ctrl_calculate(chn_no, format, frate, norm, vgroup, chn_cnt, 1))
    {
        ISIL_DBG(ISIL_DBG_ERR, "Unable to calculate map table!\n");

        return -EINVAL;
    }

    ISIL_DBG(ISIL_DBG_INFO, "%8s %8s %8s %8s %8s %8s %8s\n", "phy", "logic", 
            "size", "fps", "stride", "enable", "maptable");
    map_info = chan_map_info;
    for(i = 0; i < ISIL_MAX_GRP; i++)
    {
        for(j = 0; j < ISIL_CHN_NO; j++)
        {
            map_info->phy_slot_id               = vgroup[i].in_select[j];
            map_info->map_logic_slot_id         = i * ISIL_CHN_NO + j;
            map_info->video_size                = vgroup[i].in_fmt[j];
            map_info->fps                       = vgroup[i].out_rate[j];
            map_info->logic_slot_discard_table  = vgroup[i].in_map[j];
            map_info->roundTableStride = (norm == ISIL_VIDEO_STANDARD_PAL) ?0x18:0x1d;
            map_info->enable           = (map_info->fps == 0)?0:1;
            if(map_info->enable) {
                if(consistency == ISIL_TRUE ) {
                    if((i == 0) && (j == 0)) {
                        last_video_size = map_info->video_size;
                    }
                    if(map_info->video_size != last_video_size) {
                        consistency = ISIL_FALSE;
                    }
                }
            }
            ISIL_DBG(ISIL_DBG_INFO, "%8d %8d %8d %8d %8d %8d 0x%08x\n", map_info->phy_slot_id,
                    map_info->map_logic_slot_id, map_info->video_size, map_info->fps,
                    map_info->roundTableStride, map_info->enable, map_info->logic_slot_discard_table);
            map_info++;
        }
        vd_cross_bus->bus_mode[vgroup[i].grp_id] = vgroup[i].mode;
    }

    if(consistency == ISIL_FALSE) {
        chip->video_encode_cap->ddr_map_mode = DDR_MAP_COMPRESS_DISABLE;
        ISIL_DBG(ISIL_DBG_INFO, "change video_encode_work_mode to DDR_MAP_COMPRESS_DISABLE\n");
    }else{
        chip->video_encode_cap->ddr_map_mode = DDR_MAP_COMPRESS_ENABLE;
        ISIL_DBG(ISIL_DBG_INFO, "change video_encode_work_mode to DDR_MAP_COMPRESS_ENABLE\n");
    }

    return ISIL_OK;
}

/*apply all cross bus configuration*/
static int isil_vd_cross_bus_apply(isil_vd_cross_bus_t *bus, isil_chip_t *chip)
{
    int i, halfd1_cnt[ISIL_MAX_GRP], d1_cnt[ISIL_MAX_GRP], cif_cnt[ISIL_MAX_GRP];
    int downsample = 0;
    int valid      = 0;
    int format     = 0;
    int val        = 0;
    int timeout = 0;
    int bus_select[ISIL_MAX_GRP];
    int bus_map[ISIL_MAX_GRP];
    chip_driver_t *driver;
    isil_video_bus_t *video_bus;
    isil_h264_logic_video_slot_t *logic_video_slot;
    isil_h264_phy_video_slot_t   *phy_video_slot;

    if(!bus || !chip) {
        ISIL_DBG(ISIL_DBG_ERR, "NULL pointer\n");
        return ISIL_ERR;
    }
    ISIL_DBG(ISIL_DBG_INFO, "update vd configure to hardware\n");
    driver = chip->chip_driver;
    video_bus = &driver->video_bus;
    memset(bus_select, 0, sizeof(int) * ISIL_MAX_GRP);
    /*disable all video input*/
    chip->io_op->chip_write32(chip, VD_CHANNEL_SWITCH, 0x0000);
    timeout = 100;
    while(chip->io_op->chip_read32(chip, VD_CHANNEL_VALID) && (--timeout)) {
        mdelay(1);
    }
    if(timeout == 0){
        ISIL_DBG(ISIL_DBG_FATAL, "disable channel failed\n");
    }
    memset(bus_map, 0, sizeof(bus_map));
    memset(d1_cnt, 0, sizeof(d1_cnt));
    memset(halfd1_cnt, 0, sizeof(halfd1_cnt));
    memset(cif_cnt, 0, sizeof(cif_cnt));
    for(i = 0; i < ISIL_CROSS_BUS_LOGIC_VD_CHAN_NUMBER; i++){
        logic_video_slot = &bus->logic_video_slot[i];
        if(logic_video_slot->mapPhySlotId >= ISIL_PHY_VD_CHAN_NUMBER){
            ISIL_DBG(ISIL_DBG_ERR, "logic_video_slot = %d map to phy_video_slot = %d failed\n", logic_video_slot->logicSlotId,
                    logic_video_slot->mapPhySlotId);
            return -EINVAL;
        }
        phy_video_slot = &bus->phy_video_slot[logic_video_slot->mapPhySlotId];
        /*down sample*/
        if((logic_video_slot->video_size == ISIL_VIDEO_SIZE_CIF) || (logic_video_slot->video_size == ISIL_VIDEO_SIZE_QCIF)){
            //downsample |= 1 << logic_video_slot->logicSlotId;
            downsample |= 1 << phy_video_slot->phy_slot_id;
        }
        /*configure video format*/
        switch(logic_video_slot->video_size)
        {
            default:
                ISIL_DBG(ISIL_DBG_ERR, "channle %d unsupport video size %d\n", phy_video_slot->phy_slot_id, logic_video_slot->video_size);
                return -EINVAL;
            case ISIL_VIDEO_SIZE_4CIF:
            case ISIL_VIDEO_SIZE_D1:
                d1_cnt[logic_video_slot->logicSlotId>>2]++;
                val = 0x00;
                break;
            case ISIL_VIDEO_SIZE_HALF_D1:
            case ISIL_VIDEO_SIZE_2CIF:
                halfd1_cnt[logic_video_slot->logicSlotId>>2]++;
                val = 0x00;
                break;
            case ISIL_VIDEO_SIZE_CIF:
                cif_cnt[logic_video_slot->logicSlotId>>2]++;
                val = 0x03;
                break;
            case ISIL_VIDEO_SIZE_QCIF:
                cif_cnt[logic_video_slot->logicSlotId>>2]++;
                val = 0x01;
                break;
        }
        format |= val << (phy_video_slot->phy_slot_id * 2);
        if(logic_video_slot->enableSlot){
            valid |= 1<<logic_video_slot->mapPhySlotId;
        }

        /*bus channel select*/
        bus_select[(logic_video_slot->logicSlotId / 4)] |= (logic_video_slot->mapPhySlotId << 16);
        bus_select[(logic_video_slot->logicSlotId / 4)] >>= 4;
        /*configure map table*/
        chip->io_op->chip_write32(chip, VD_CHANNEL_LMAP(((logic_video_slot->logicSlotId / 4) * 4) + bus_map[logic_video_slot->logicSlotId / 4]),logic_video_slot->discardTable);
        chip->io_op->chip_write32(chip, VD_CHANNEL_HMAP(((logic_video_slot->logicSlotId / 4) * 4) + bus_map[logic_video_slot->logicSlotId / 4]), logic_video_slot->discardTable>>16);
        bus_map[logic_video_slot->logicSlotId / 4]++;
    }

    for(i = 0; i < ISIL_MAX_GRP; i++){
        chip->io_op->chip_write32(chip, VD_BUS_MAP(i), bus_select[i]);
        if(cif_cnt[i] == 0){
            bus->bus_mode[i] = MODE_D1;
        }else if((d1_cnt[i] == 0) && (halfd1_cnt[i] == 0)){
            bus->bus_mode[i] = MODE_CIF;
        }else if(halfd1_cnt[i] == 0){
            bus->bus_mode[i] = MODE_D1_CIF;
        }else if(d1_cnt[i] == 0){
            bus->bus_mode[i] = MODE_HALFD1_CIF;
        }else{
            ISIL_DBG(ISIL_DBG_FATAL, "bus %d map error\n", i);
        }
        config_senif_bus(bus, i, bus->bus_mode[i]);
    }

    chip->io_op->chip_write32(chip, VD_CHANNEL_DOWNSAMPLE, downsample);
    /*configure channel format*/
    chip->io_op->chip_write32(chip, VD_CHANNEL_FORMAT0, format & 0xffff);
    chip->io_op->chip_write32(chip, VD_CHANNEL_FORMAT1, (format >> 16) & 0xffff);

    /*max line*/
    if(video_bus->op->get_video_standard(video_bus) == ISIL_VIDEO_STANDARD_PAL){//PAL
        chip->io_op->chip_write32(chip, VD_BUS_MAX_LINE0, 0x318);//every bus 25fps
        chip->io_op->chip_write32(chip, VD_BUS_MAX_LINE1, 0x318);
    }
    else{
        chip->io_op->chip_write32(chip, VD_BUS_MAX_LINE0, 0x3bd);//every bus 30fps
        chip->io_op->chip_write32(chip, VD_BUS_MAX_LINE1, 0x3bd);
    }

    /*bus limit*/
    chip->io_op->chip_write32(chip, VD_BUS_MAX_CHANNEL, 0x0000);//always allowed 4chn
    chip->io_op->chip_write32(chip, VD_CHANNEL_INTERLACE, 0x0000);//always interlace 

    /*enable all channel*/
    chip->io_op->chip_write32(chip, VD_CHANNEL_SWITCH, valid);
    timeout = 100;
    while((chip->io_op->chip_read32(chip, VD_CHANNEL_VALID) != valid) && (--timeout)) {
        mdelay(1);
    }
    if(timeout == 0){
        ISIL_DBG(ISIL_DBG_FATAL, "enable channel failed\n");
    }

    return ISIL_OK;
}

static int	isil_vd_cross_bus_reset(isil_vd_cross_bus_t *vd_cross_bus)
{
    if(vd_cross_bus) {
        isil_h264_logic_video_slot_t	*logic_video_slot;
#if 0
        isil_h264_phy_video_slot_t	*phy_video_slot;
#endif
        vd_chan_map_info_t	chan_map_table[ISIL_PHY_VD_CHAN_NUMBER], *chan_map;
        isil_chip_t	*chip = NULL;
        isil_video_bus_t *video_bus;
        u32  local_fps = 0;
        int	i,ret;

        //ISIL_DBG(ISIL_DBG_INFO, "ignore vd bus reset\n");
        //return 0;

        ISIL_DBG(ISIL_DBG_INFO, "vd_bus reset to mode %d\n", vd_cross_bus->op->get_work_mode(vd_cross_bus));
        chip = to_get_chip_with_chip_cross_bus(vd_cross_bus);
        video_bus = &chip->chip_driver->video_bus;
        chan_map = chan_map_table;
        if(video_bus->op->get_video_standard(video_bus) == ISIL_VIDEO_STANDARD_PAL) {
            local_fps = 25;
        }else{
            local_fps = 30;
        }
#if 0
        switch(vd_cross_bus->op->get_work_mode(vd_cross_bus)){
            case ISIL_CROSS_BUS_4D1_REALTIME:
                for(i=0; i<4; i++, chan_map++){
                    chan_map->phy_slot_id = i;
                    chan_map->fps = local_fps;
                    chan_map->video_size = ISIL_VIDEO_SIZE_D1;
                    chan_map->enable = 1;
                }

                for(;i<ISIL_PHY_VD_CHAN_NUMBER; i++, chan_map++){
                    chan_map->phy_slot_id = ISIL_LOGIC_VD_INVALID;
                }
                break;
            case ISIL_CROSS_BUS_8HalfD1_REALTIME:
                for(i=0; i<8; i++, chan_map++){
                    chan_map->phy_slot_id = i;
                    chan_map->fps = local_fps;
                    chan_map->video_size = ISIL_VIDEO_SIZE_HALF_D1;
                    chan_map->enable = 1;
                }
                for(; i < ISIL_PHY_VD_CHAN_NUMBER; i++, chan_map++){
                    chan_map->phy_slot_id = ISIL_LOGIC_VD_INVALID;
                }
                break;
            case ISIL_CROSS_BUS_16CIF_REALTIME:
                for(i=0; i<ISIL_PHY_VD_CHAN_NUMBER; i++, chan_map++){
                    chan_map->phy_slot_id = i;
                    chan_map->fps = local_fps;
                    chan_map->video_size = ISIL_VIDEO_SIZE_CIF;
                    chan_map->enable = 1;
                }
                break;
            default:
            case ISIL_CROSS_BUS_UNREALTIME:
                for(i=0; i<ISIL_PHY_VD_CHAN_NUMBER; i++, chan_map++){
                    logic_video_slot = &vd_cross_bus->logic_video_slot[i];
                    chan_map->phy_slot_id = logic_video_slot->op->get_mapPhySlotId(logic_video_slot, i);
                    chan_map->fps = logic_video_slot->op->get_fps(logic_video_slot);
                    chan_map->enable = logic_video_slot->op->get_enableslot(logic_video_slot);
                    chan_map->video_size = logic_video_slot->op->get_video_size(logic_video_slot);
                }
        }
#else
        for(i=0; i<ISIL_PHY_VD_CHAN_NUMBER; i++, chan_map++){
            logic_video_slot = &vd_cross_bus->logic_video_slot[i];
            chan_map->phy_slot_id = logic_video_slot->op->get_mapPhySlotId(logic_video_slot);
            chan_map->fps = logic_video_slot->op->get_fps(logic_video_slot);
            chan_map->enable = logic_video_slot->op->get_enableslot(logic_video_slot);
            chan_map->video_size = logic_video_slot->op->get_video_size(logic_video_slot);
        }
#endif
#if 0
        if(vd_cross_bus->op->calculate_cross_bus_map_table(vd_cross_bus, chan_map_table)){
            ISIL_DBG(ISIL_DBG_ERR, "calculate map table failed, please check parameter!\n");
            return -EINVAL;
        }
        chan_map = chan_map_table;
#endif
        video_bus->op->notify_suspend_logic_chan(video_bus);
#if 0
        for(i = 0; i < ISIL_CROSS_BUS_LOGIC_VD_CHAN_NUMBER; i++, chan_map++) {
            if(chan_map->map_logic_slot_id >= ISIL_CROSS_BUS_LOGIC_VD_CHAN_NUMBER) {
                ISIL_DBG(ISIL_DBG_ERR, "logic channel number %d overflow\n", chan_map->map_logic_slot_id);
                return -EINVAL;
            }
            logic_video_slot = &vd_cross_bus->logic_video_slot[chan_map->map_logic_slot_id];
            logic_video_slot->op->set_mapPhySlotId(logic_video_slot, chan_map->phy_slot_id);
            logic_video_slot->op->set_discardTable(logic_video_slot, chan_map->logic_slot_discard_table);
            logic_video_slot->op->set_roundTableStride(logic_video_slot, chan_map->roundTableStride);
            logic_video_slot->op->set_video_size(logic_video_slot, chan_map->video_size);
            logic_video_slot->op->set_enableslot(logic_video_slot, chan_map->enable);
            logic_video_slot->op->set_fps(logic_video_slot, chan_map->fps);
            atomic_set(&logic_video_slot->needUpdateFlag, 1);
            vd_cross_bus->op->get_phy_video_slot_by_phy_id(vd_cross_bus, chan_map->phy_slot_id, &phy_video_slot);
            phy_video_slot->op->set_video_size(phy_video_slot, logic_video_slot->op->get_video_size(logic_video_slot));
            phy_video_slot->op->update_map_logic_slot(phy_video_slot, logic_video_slot->logicSlotId, logic_video_slot);
        }
#endif
        vd_cross_bus->op->set_chip_end_cross_bus(vd_cross_bus, chip);
        video_bus->op->notify_resume_logic_chan(video_bus);
        /*notify logic video channel*/
        ret = video_bus->op->notify_logic_chan_change(video_bus);

        return ret;
    }

    return ISIL_ERR;
}

static struct isil_vd_cross_bus_operation	isil_vd_cross_bus_op = {
    .init = isil_vd_cross_bus_init,
    .reset = isil_vd_cross_bus_reset,
    .get_logic_video_slot_by_logic_id = isil_vd_cross_bus_get_logic_video_slot_by_logic_id,
    .get_logic_video_slot_by_phy_id = isil_vd_cross_bus_get_logic_video_slot_by_phy_id,
    .get_phy_video_slot_by_logic_id = isil_vd_cross_bus_get_phy_video_slot_by_logic_id,
    .get_phy_video_slot_by_phy_id = isil_vd_cross_bus_get_phy_video_slot_by_phy_id,
    .get_work_mode = isil_vd_cross_bus_get_work_mode,
    .update_work_mode = isil_vd_cross_bus_update_work_mode,

    .calculate_cross_bus_map_table = isil_vd_cross_bus_calculate_cross_bus_map_table,
    .set_chip_end_cross_bus = isil_vd_cross_bus_apply,

};

int	init_isil_vd_cross_bus(isil_vd_cross_bus_t *vd_cross_bus, enum ISIL_CROSS_BUS_WORK_MODE mode, enum ISIL_VIDEO_STANDARD	video_standard)
{
    if(vd_cross_bus != NULL){
        vd_cross_bus->op = &isil_vd_cross_bus_op;
        return vd_cross_bus->op->init(vd_cross_bus, mode, video_standard);
    }

    return ISIL_ERR;
}

void	remove_isil_vd_cross_bus(isil_vd_cross_bus_t *vd_cross_bus)	//close all cross bus config
{

}

static void isil_vd_slot_orig_buf_info_init(isil_vd_orig_buf_info_t *isil_vd_orig_buf, int phy_slot_id)
{
    atomic_set(&isil_vd_orig_buf->stop_check_ad, 0);
    spin_lock_init(&isil_vd_orig_buf->lock);
    isil_vd_orig_buf->phy_slot_id = phy_slot_id;
    isil_vd_orig_buf->first_start_discard_frame_number = FIRST_START_DISCARD_FRAME_NUMBER;
    isil_vd_orig_buf->vd_orig_ad_phy_ptr = 0;
    isil_vd_orig_buf->vd_orig_ad_last_phy_ptr = 0;
    isil_vd_orig_buf->vd_toggle_orig_ad_phy_ptr = 0;
    isil_vd_orig_buf->vd_update_orig_ad_phy_ptr =0;
    isil_vd_orig_buf->vd_encode_ptr = 0;
    isil_vd_orig_buf->vd_toggle_encode_ptr = 0;
    init_isil_encode_timestamp(&isil_vd_orig_buf->vd_encode_timestamp);
    isil_vd_orig_buf->vd_ptr_check_count = 0;
    isil_vd_orig_buf->logic_chan_ad_ready_map_table = 0;
    isil_vd_orig_buf->logic_chan_encode_over_map_table = 0;
}

static void isil_vd_slot_orig_buf_info_first_sync_orig_buf_info(isil_vd_orig_buf_info_t *isil_vd_orig_buf, isil_chip_t *chip)
{
    if((chip!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&isil_vd_orig_buf->lock, flags);
        isil_vd_orig_buf->first_start_discard_frame_number = FIRST_START_DISCARD_FRAME_NUMBER;
        isil_vd_orig_buf->vd_encode_timestamp.op->reset(&isil_vd_orig_buf->vd_encode_timestamp);
        isil_vd_orig_buf->vd_orig_ad_last_phy_ptr = isil_vd_orig_buf->op->get_vd_ad_phy_ptr(isil_vd_orig_buf, chip);
        isil_vd_orig_buf->vd_toggle_orig_ad_phy_ptr = isil_vd_orig_buf->vd_orig_ad_last_phy_ptr;
        isil_vd_orig_buf->vd_encode_ptr = isil_vd_orig_buf->vd_orig_ad_last_phy_ptr;
        isil_vd_orig_buf->vd_toggle_encode_ptr = isil_vd_orig_buf->vd_orig_ad_last_phy_ptr;
        chip->io_op->chip_write32(chip, DSP_CUR_RAM_BASE, (((isil_vd_orig_buf->vd_encode_ptr+3)&3)<<12));
        isil_vd_orig_buf->op->set_vd_update_orig_buf_reg(isil_vd_orig_buf, chip);
        spin_unlock_irqrestore(&isil_vd_orig_buf->lock, flags);
    }
}

static int  isil_vd_slot_orig_buf_info_vd_orig_buf_ready(isil_vd_orig_buf_info_t *isil_vd_orig_buf, isil_chip_t *chip, isil_h264_logic_encode_chan_t *logic_encode_chan)
{
    int	ret = 0, ad_is_ready = 0;
    if((chip!=NULL) && (logic_encode_chan!=NULL)){
        ed_tcb_t        *opened_logic_chan_ed;
        isil_ed_fsm_t   *ed_fsm;
        unsigned long	flags;

        spin_lock_irqsave(&isil_vd_orig_buf->lock, flags);
        opened_logic_chan_ed = &logic_encode_chan->opened_logic_chan_ed;
        ed_fsm = &opened_logic_chan_ed->ed_fsm;
        if(chip_is_in_robust_processing(chip)
                || isil_h264_encode_chan_is_in_robust_processing(logic_encode_chan)){
            ed_fsm->op->feed_state_watchdog(ed_fsm);
            isil_vd_orig_buf->vd_ptr_check_count = 0;
        } else {
            isil_vd_orig_buf->op->get_vd_ad_phy_ptr(isil_vd_orig_buf, chip);
            if(isil_vd_orig_buf->vd_orig_ad_last_phy_ptr != isil_vd_orig_buf->vd_orig_ad_phy_ptr){
                isil_vd_orig_buf->vd_orig_ad_last_phy_ptr = isil_vd_orig_buf->vd_orig_ad_phy_ptr;
                isil_vd_orig_buf->vd_toggle_orig_ad_phy_ptr++;
                isil_vd_orig_buf->vd_toggle_orig_ad_phy_ptr &= 7;
            }

            if(atomic_read(&isil_vd_orig_buf->stop_check_ad) == 0){
                if(isil_vd_orig_buf->vd_encode_ptr != isil_vd_orig_buf->vd_orig_ad_phy_ptr){
                    ad_is_ready = 1;
                } else {
                    int vd_orig_ad_phy_ptr, vd_toggle_orig_ad_phy_ptr, vd_encode_ptr, vd_toggle_encode_ptr;
                    vd_orig_ad_phy_ptr = isil_vd_orig_buf->vd_toggle_orig_ad_phy_ptr;
                    vd_encode_ptr = isil_vd_orig_buf->vd_toggle_encode_ptr;
                    vd_toggle_orig_ad_phy_ptr = vd_orig_ad_phy_ptr>>2;
                    vd_toggle_encode_ptr = vd_encode_ptr>>2;
                    vd_orig_ad_phy_ptr &= 3;
                    vd_encode_ptr &= 3;
                    if((vd_orig_ad_phy_ptr == vd_encode_ptr) && (vd_toggle_orig_ad_phy_ptr != vd_toggle_encode_ptr)){
                        ad_is_ready = 1;
                    }
                }
                if(ad_is_ready){
                    ed_fsm->op->feed_state_watchdog(ed_fsm);
                    isil_vd_orig_buf->vd_ptr_check_count = 0;
                    if(isil_vd_orig_buf->first_start_discard_frame_number){
                        isil_vd_orig_buf->first_start_discard_frame_number--;
                        isil_vd_orig_buf->op->update_vd_encode_ptr(isil_vd_orig_buf);
                        isil_vd_orig_buf->op->set_vd_update_orig_buf_reg(isil_vd_orig_buf, chip);
                        ISIL_DBG(ISIL_DBG_INFO, "chan_%d_%d: first discard frame %d\n",
                                logic_encode_chan->phy_slot_id,
                                logic_encode_chan->logic_chan_id,
                                isil_vd_orig_buf->first_start_discard_frame_number);
                    } else {
                        isil_vd_orig_buf->logic_chan_ad_ready_map_table = LOGIC_CHAN_NUMBER_MASK;
                        isil_vd_orig_buf->logic_chan_encode_over_map_table = 0;
                        isil_vd_orig_buf->op->update_timestamp(isil_vd_orig_buf, chip);
                        atomic_set(&isil_vd_orig_buf->stop_check_ad, 1);
                        ret = 1;
                    }
                } else {
                    isil_vd_orig_buf->vd_ptr_check_count++;
                    if(isil_vd_orig_buf->vd_ptr_check_count > 250){
                        isil_vd_orig_buf->vd_ptr_check_count = 0;
                        isil_vd_orig_buf->op->update_vd_encode_ptr(isil_vd_orig_buf);
                    }
                }
            } else {
                if((isil_vd_orig_buf->logic_chan_ad_ready_map_table & (1<<(logic_encode_chan->logic_chan_id&VIDEO_LOGIC_MASTER_OR_SUB_MASK)))){
                    ed_fsm->op->feed_state_watchdog(ed_fsm);
                    ret = 1;
                }
            }
        }
        spin_unlock_irqrestore(&isil_vd_orig_buf->lock, flags);

        if(atomic_read(&logic_encode_chan->first_start_flag)){
            ed_fsm->op->feed_state_watchdog(ed_fsm);
            isil_vd_orig_buf->vd_ptr_check_count = 0;
            if(ret){
                isil_h264_phy_video_slot_t *master_slot = logic_encode_chan->master_slot;
                master_slot->op->slot_software_discard_frame(master_slot, chip, logic_encode_chan);
                ret = 0;
            }
        }
    }
    return ret;
}

static int  isil_vd_slot_orig_buf_info_clear_vd_orig_buf_ready(isil_vd_orig_buf_info_t *isil_vd_orig_buf, isil_chip_t *chip, isil_h264_logic_encode_chan_t *logic_encode_chan)
{
    int	ret = 0;
    if((chip!=NULL) && (logic_encode_chan!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&isil_vd_orig_buf->lock, flags);
        if((isil_vd_orig_buf->logic_chan_ad_ready_map_table & (1<<(logic_encode_chan->logic_chan_id&VIDEO_LOGIC_MASTER_OR_SUB_MASK)))){
            isil_vd_orig_buf->logic_chan_ad_ready_map_table &= ~(1<<(logic_encode_chan->logic_chan_id&VIDEO_LOGIC_MASTER_OR_SUB_MASK));
            ret = 1;
        }
        spin_unlock_irqrestore(&isil_vd_orig_buf->lock, flags);
    }
    return ret;
}

static int  isil_vd_slot_orig_buf_info_get_vd_ad_phy_ptr(isil_vd_orig_buf_info_t *isil_vd_orig_buf, isil_chip_t *chip)
{
    unsigned int    mask=0x3;
    int             shift_number;

    if(isil_vd_orig_buf->phy_slot_id < 8){
        isil_vd_orig_buf->vd_orig_ad_phy_ptr = chip->io_op->chip_read32(chip, AD_ORIG_RAM_BASE);
        shift_number = (isil_vd_orig_buf->phy_slot_id<<1);
    } else {
        isil_vd_orig_buf->vd_orig_ad_phy_ptr = chip->io_op->chip_read32(chip, AD_CHAN_8_15_ORIG_BASE);
        shift_number = (isil_vd_orig_buf->phy_slot_id-8)<<1;
    }
    mask <<= shift_number;
    isil_vd_orig_buf->vd_orig_ad_phy_ptr &= mask;
    isil_vd_orig_buf->vd_orig_ad_phy_ptr >>= shift_number;
    isil_vd_orig_buf->vd_orig_ad_phy_ptr &= 0x3;
    return isil_vd_orig_buf->vd_orig_ad_phy_ptr;
}

static void isil_vd_slot_orig_buf_info_set_vd_update_orig_buf_reg(isil_vd_orig_buf_info_t *isil_vd_orig_buf, isil_chip_t *chip)
{
    unsigned int    mask=0x3;
    int shift_number;

    if(isil_vd_orig_buf->phy_slot_id < 8){
        isil_vd_orig_buf->vd_update_orig_ad_phy_ptr = chip->io_op->chip_read32(chip, TOTAL_CHAN_CUR_PTR);
        shift_number = (isil_vd_orig_buf->phy_slot_id<<1);
    } else {
        isil_vd_orig_buf->vd_update_orig_ad_phy_ptr = chip->io_op->chip_read32(chip, TOTAL_CHAN_8_15_CUR_PTR);
        shift_number = (isil_vd_orig_buf->phy_slot_id-8)<<1;
    }
    mask <<= shift_number;
    isil_vd_orig_buf->vd_update_orig_ad_phy_ptr &= ~mask;
    isil_vd_orig_buf->vd_update_orig_ad_phy_ptr |= (((isil_vd_orig_buf->vd_encode_ptr + 0x3) & (0x3))<<shift_number);
    if(isil_vd_orig_buf->phy_slot_id < 8){
        chip->io_op->chip_write32(chip, TOTAL_CHAN_CUR_PTR, isil_vd_orig_buf->vd_update_orig_ad_phy_ptr);
    } else {
        chip->io_op->chip_write32(chip, TOTAL_CHAN_8_15_CUR_PTR, isil_vd_orig_buf->vd_update_orig_ad_phy_ptr);
    }
}

static void isil_vd_slot_orig_buf_info_set_vd_curr_encode_ptr(isil_vd_orig_buf_info_t *isil_vd_orig_buf, isil_chip_t *chip, isil_h264_logic_encode_chan_t *logic_encode_chan)
{
    if((chip!=NULL) && (logic_encode_chan!=NULL)){
        unsigned long	flags;
        //u32 data;

        spin_lock_irqsave(&isil_vd_orig_buf->lock, flags);
        /*data = chip->io_op->chip_read32(chip, CHANNEL_ID);
        data = (data & (~0xf)) | isil_vd_orig_buf->phy_slot_id;
        chip->io_op->chip_write32(chip, CHANNEL_ID, data);*/
        chip->io_op->chip_write32(chip, DSP_CUR_RAM_BASE, (isil_vd_orig_buf->vd_encode_ptr<<12));
        spin_unlock_irqrestore(&isil_vd_orig_buf->lock, flags);
    }
}

static void isil_vd_slot_orig_buf_info_update_vd_encode_ptr(isil_vd_orig_buf_info_t *isil_vd_orig_buf)
{
    isil_vd_orig_buf->vd_encode_ptr++;
    isil_vd_orig_buf->vd_encode_ptr &= 0x3;
    isil_vd_orig_buf->vd_toggle_encode_ptr++;
    isil_vd_orig_buf->vd_toggle_encode_ptr &= 0x7;
}

static void isil_vd_slot_orig_buf_info_update_timestamp(isil_vd_orig_buf_info_t *isil_vd_orig_buf, isil_chip_t *chip)
{
    if((chip!=NULL)){
        isil_timestamp_t  *vd_encode_timestamp = &isil_vd_orig_buf->vd_encode_timestamp;
        unsigned int    video_timestamp_reg_offset;
        video_timestamp_reg_offset = CHAN_VIDEO_TIMESTAMP_BASE + 0x10*isil_vd_orig_buf->phy_slot_id + (isil_vd_orig_buf->vd_encode_ptr<<2);
        vd_encode_timestamp->op->set_timestamp(vd_encode_timestamp, chip->io_op->chip_read32(chip, video_timestamp_reg_offset), __FILE__);
    }
}

static u32  isil_vd_slot_orig_buf_info_get_timestamp(isil_vd_orig_buf_info_t *isil_vd_orig_buf)
{
    isil_timestamp_t  *vd_encode_timestamp = &isil_vd_orig_buf->vd_encode_timestamp;
    return vd_encode_timestamp->op->get_timestamp(vd_encode_timestamp);
}

static void isil_vd_slot_orig_buf_info_software_discard_frame(isil_vd_orig_buf_info_t *isil_vd_orig_buf, isil_chip_t *chip, isil_h264_logic_encode_chan_t *logic_encode_chan)
{
    if((chip!=NULL) && (logic_encode_chan!=NULL)){
        isil_h264_phy_video_slot_t  *master_slot;
	isil_register_table_t       *opened_logic_chan_table;
        int have_been_encoded_chan_number, encode_over_map_table, entry_number_in_table;
        unsigned long	flags;

        spin_lock_irqsave(&isil_vd_orig_buf->lock, flags);
        isil_vd_orig_buf->logic_chan_encode_over_map_table |= (1<<(logic_encode_chan->logic_chan_id&VIDEO_LOGIC_MASTER_OR_SUB_MASK));
        isil_vd_orig_buf->logic_chan_encode_over_map_table &= LOGIC_CHAN_NUMBER_MASK;
        have_been_encoded_chan_number = 0;
        encode_over_map_table = isil_vd_orig_buf->logic_chan_encode_over_map_table;
        while(encode_over_map_table){
            if(encode_over_map_table&1){
                have_been_encoded_chan_number++;
            }
            encode_over_map_table >>= 1;
        }
        master_slot = logic_encode_chan->master_slot;
        opened_logic_chan_table = &master_slot->opened_logic_chan_table;
        entry_number_in_table = opened_logic_chan_table->op->get_curr_register_node_number(opened_logic_chan_table);
        if((have_been_encoded_chan_number>=entry_number_in_table) || (entry_number_in_table==0)){
            isil_vd_orig_buf->vd_ptr_check_count = 0;
            isil_vd_orig_buf->logic_chan_ad_ready_map_table = 0;
            isil_vd_orig_buf->logic_chan_encode_over_map_table = 0;
            isil_vd_orig_buf->op->update_vd_encode_ptr(isil_vd_orig_buf);
            isil_vd_orig_buf->op->set_vd_update_orig_buf_reg(isil_vd_orig_buf, chip);
            atomic_set(&isil_vd_orig_buf->stop_check_ad, 0);
        }
        spin_unlock_irqrestore(&isil_vd_orig_buf->lock, flags);
    }
}

static void isil_vd_slot_orig_buf_info_encode_endofslice_notify(isil_vd_orig_buf_info_t *isil_vd_orig_buf, isil_h264_logic_encode_chan_t *logic_encode_chan)
{
    if((logic_encode_chan!=NULL)){
        isil_h264_phy_video_slot_t  *master_slot;
	isil_register_table_t       *opened_logic_chan_table;
        int have_been_encoded_chan_number, encode_over_map_table, entry_number_in_table;
        unsigned long	flags;

        spin_lock_irqsave(&isil_vd_orig_buf->lock, flags);
        isil_vd_orig_buf->logic_chan_encode_over_map_table |= (1<<(logic_encode_chan->logic_chan_id&VIDEO_LOGIC_MASTER_OR_SUB_MASK));
        isil_vd_orig_buf->logic_chan_encode_over_map_table &= LOGIC_CHAN_NUMBER_MASK;
        have_been_encoded_chan_number = 0;
        encode_over_map_table = isil_vd_orig_buf->logic_chan_encode_over_map_table;
        while(encode_over_map_table){
            if(encode_over_map_table&1){
                have_been_encoded_chan_number++;
            }
            encode_over_map_table >>= 1;
        }
        master_slot = logic_encode_chan->master_slot;
        opened_logic_chan_table = &master_slot->opened_logic_chan_table;
        entry_number_in_table = opened_logic_chan_table->op->get_curr_register_node_number(opened_logic_chan_table);
        if((have_been_encoded_chan_number>=entry_number_in_table) || (entry_number_in_table==0)){
            isil_vd_orig_buf->vd_ptr_check_count = 0;
            isil_vd_orig_buf->logic_chan_ad_ready_map_table = 0;
            isil_vd_orig_buf->logic_chan_encode_over_map_table = 0;
            isil_vd_orig_buf->op->update_vd_encode_ptr(isil_vd_orig_buf);
            atomic_set(&isil_vd_orig_buf->stop_check_ad, 0);
        }
        spin_unlock_irqrestore(&isil_vd_orig_buf->lock, flags);
     }
}

static void isil_vd_slot_orig_buf_info_encode_unregister_notify(isil_vd_orig_buf_info_t *isil_vd_orig_buf, isil_h264_logic_encode_chan_t *logic_encode_chan)
{
    if((logic_encode_chan!=NULL)){
        isil_h264_phy_video_slot_t  *master_slot;
	isil_register_table_t       *opened_logic_chan_table;
        int have_been_encoded_chan_number, encode_over_map_table, entry_number_in_table;
        unsigned long	flags;

        spin_lock_irqsave(&isil_vd_orig_buf->lock, flags);
        isil_vd_orig_buf->logic_chan_encode_over_map_table &= ~(1<<(logic_encode_chan->logic_chan_id&VIDEO_LOGIC_MASTER_OR_SUB_MASK));
        isil_vd_orig_buf->logic_chan_encode_over_map_table &= LOGIC_CHAN_NUMBER_MASK;
        have_been_encoded_chan_number = 0;
        encode_over_map_table = isil_vd_orig_buf->logic_chan_encode_over_map_table;
        while(encode_over_map_table){
            if(encode_over_map_table&1){
                have_been_encoded_chan_number++;
            }
            encode_over_map_table >>= 1;
        }
        master_slot = logic_encode_chan->master_slot;
        opened_logic_chan_table = &master_slot->opened_logic_chan_table;
        entry_number_in_table = opened_logic_chan_table->op->get_curr_register_node_number(opened_logic_chan_table);
        if((have_been_encoded_chan_number>=entry_number_in_table) || (entry_number_in_table==0)){
            isil_vd_orig_buf->vd_ptr_check_count = 0;
            isil_vd_orig_buf->logic_chan_ad_ready_map_table = 0;
            isil_vd_orig_buf->logic_chan_encode_over_map_table = 0;
            isil_vd_orig_buf->op->update_vd_encode_ptr(isil_vd_orig_buf);
            atomic_set(&isil_vd_orig_buf->stop_check_ad, 0);
        }
        spin_unlock_irqrestore(&isil_vd_orig_buf->lock, flags);
    }
}

static struct isil_vd_orig_buf_info_operation isil_vd_slot_orig_buf_info_op = {
    .init = isil_vd_slot_orig_buf_info_init,
    .first_sync_orig_buf_info = isil_vd_slot_orig_buf_info_first_sync_orig_buf_info,
    .vd_orig_buf_ready = isil_vd_slot_orig_buf_info_vd_orig_buf_ready,
    .clear_vd_orig_buf_ready = isil_vd_slot_orig_buf_info_clear_vd_orig_buf_ready,
    .get_vd_ad_phy_ptr = isil_vd_slot_orig_buf_info_get_vd_ad_phy_ptr,
    .set_vd_update_orig_buf_reg = isil_vd_slot_orig_buf_info_set_vd_update_orig_buf_reg,
    .set_vd_curr_encode_ptr = isil_vd_slot_orig_buf_info_set_vd_curr_encode_ptr,
    .update_vd_encode_ptr = isil_vd_slot_orig_buf_info_update_vd_encode_ptr,
    .update_timestamp = isil_vd_slot_orig_buf_info_update_timestamp,
    .get_timestamp = isil_vd_slot_orig_buf_info_get_timestamp,
    .software_discard_frame = isil_vd_slot_orig_buf_info_software_discard_frame,
    .encode_endofslice_notify = isil_vd_slot_orig_buf_info_encode_endofslice_notify,
    .encode_unregister_notify = isil_vd_slot_orig_buf_info_encode_unregister_notify,
};

static void init_isil_vd_slot_orig_buf_info(isil_vd_orig_buf_info_t *isil_vd_orig_buf, int phy_slot_id)
{
    if(isil_vd_orig_buf != NULL){
        isil_vd_orig_buf->op = &isil_vd_slot_orig_buf_info_op;
        isil_vd_orig_buf->op->init(isil_vd_orig_buf, phy_slot_id);
    }
}

void    isil_h264_logic_encode_chan_monitor(isil_h264_logic_encode_chan_t *h264_logic_encode_chan, isil_h264_phy_video_slot_t *phy_video_slot)
{
    isil_video_frame_tcb_queue_t    *encode_frame_queue;
    isil_video_chan_buf_pool_t      *encode_chan_buf_pool;
    isil_h264_encode_control_t      *encode_control;
    isil_h264_logic_video_slot_t    *logic_video_slot;
    isil_chip_t *chip;
    ed_tcb_t    *opened_logic_chan_ed;
    int logic_chan_id, ad_is_ready=0;
    isil_ed_fsm_t *ed_fsm; 
    isil_h264_encode_delta_t *delta;
    unsigned long   flags;

#ifdef USER_FILE_IO_REQ
    return;
#endif

    spin_lock_irqsave(&h264_logic_encode_chan->fsm_matrix_call_lock, flags);
    encode_frame_queue = &h264_logic_encode_chan->encode_frame_queue;
    encode_chan_buf_pool = &h264_logic_encode_chan->encode_chan_buf_pool;
    encode_control = &h264_logic_encode_chan->encode_control;
    chip = h264_logic_encode_chan->chip;
    delta  = &encode_control->encode_delta;
    opened_logic_chan_ed = &h264_logic_encode_chan->opened_logic_chan_ed;
    logic_chan_id = h264_logic_encode_chan->logic_chan_id;
    ed_fsm = &opened_logic_chan_ed->ed_fsm;

    if(phy_video_slot && phy_video_slot->cross_bus_logic_video_slot){
        isil_h264_encode_property_t *encode_property;
        u32 target=0, phy_fps=0;

        encode_property = &h264_logic_encode_chan->encode_property;
        logic_video_slot = phy_video_slot->cross_bus_logic_video_slot;
        target  = encode_property->op->get_target_fps(encode_property);
        phy_fps = logic_video_slot->op->get_fps(logic_video_slot);
        encode_control->op->update_fps_discard_table(encode_control, phy_fps, target);
        if((phy_fps==0) || (target==0)){
            ed_fsm->op->feed_state_watchdog(ed_fsm);
        }
    }
    if(phy_video_slot->op->slot_ad_is_ready(phy_video_slot, chip, h264_logic_encode_chan)){
        ad_is_ready = 1;
    }
    switch(ed_fsm->op->get_curr_state(ed_fsm)){
        case ISIL_ED_IDLE:
            if(ad_is_ready){
                phy_video_slot->op->slot_clear_ad_ready(phy_video_slot, chip, h264_logic_encode_chan);
                if((encode_control->i_software_discard_frame_table & (1<<encode_control->i_frame_per_second))){
                    encode_control->op->update_ad_ready(encode_control);
                } else {
                    phy_video_slot->op->slot_software_discard_frame(phy_video_slot, chip, h264_logic_encode_chan);
                }
                encode_control->i_frame_per_second++;
                if(encode_control->i_frame_per_second >= encode_control->i_crossbus_fps){
                    encode_control->i_frame_per_second = 0;
                }
            }
            if(encode_control->op->get_slice_head_and_ad_status(encode_control)){
                driver_gen_deliver_event(opened_logic_chan_ed, 1);
            }
            break;
        case ISIL_ED_UNREGISTER:
        case ISIL_ED_SUSPEND:
            if(ad_is_ready || encode_control->op->get_ad_status(encode_control)){
                phy_video_slot->op->slot_software_discard_frame(phy_video_slot, chip, h264_logic_encode_chan);
            }
            break;
        default:
        case ISIL_ED_STANDBY:
        case ISIL_ED_RUNNING:
        case ISIL_ED_TRANSFERING:
            break;
    }
    ed_fsm->op->update_state_timeout_counter(ed_fsm);
    spin_unlock_irqrestore(&h264_logic_encode_chan->fsm_matrix_call_lock, flags);        

    encode_control->i_statistic_minuter_counter++;
    if(encode_control->i_statistic_minuter_counter >= (TIMER_1_MINUTE)){
        dpram_control_t *chip_dpram_controller;
        chip_ddr_burst_engine_t *chip_ddr_burst_interface;
        ddr_video_bitstream_resource_manage_t   *video_bitstream_resource;
        u32 sec;

        get_isil_dpram_controller(&chip_dpram_controller);
        chip_ddr_burst_interface = &chip->chip_ddr_burst_interface;
        chip_ddr_burst_interface->op->get_ddr_video_bitstream_resource_manage(chip_ddr_burst_interface, &video_bitstream_resource);
        encode_control->last_jiffies = jiffies - encode_control->last_jiffies;
        sec = jiffies_to_msecs(encode_control->last_jiffies) / 1000;
        sec = encode_control->i_fpm / (sec ? sec:1);
        encode_control->i_curr_fps = encode_control->i_fpm;
#if 0
        ISIL_DBG(ISIL_DBG_INFO, "chan_%d_%d: fpm=%d, %dms, %dfps\n", phy_video_slot->phy_slot_id,
                logic_chan_id, encode_control->i_fpm,
                jiffies_to_msecs(encode_control->last_jiffies), sec);
        ISIL_DBG(ISIL_DBG_INFO, "(%d), (%d, %d, %x), %d\n", encode_control->ad_ready,
                ed_fsm->op->get_curr_state(ed_fsm),
                ed_fsm->op->get_last_state(ed_fsm),
                *((unsigned int*)&ed_fsm->transfer_pending),
                video_bitstream_resource->op->get_can_use_resource_number(video_bitstream_resource));
#endif
        delta->max_delta = 0;
        delta->min_delta = 0;
        encode_control->i_fps = sec;
        encode_control->ad_ready  = 0;
        encode_control->i_fpm = 0;
        encode_control->i_statistic_minuter_counter = 0;
        encode_control->last_jiffies = jiffies;
    }
}

static int isil_adaptive_deinterlace_control_init(struct isil_adaptive_deinterlace_control *deinterlace_control)
{
    if(deinterlace_control) {
        deinterlace_control->last_flags = ISIL_TRUE;
        deinterlace_control->curr_flags = ISIL_TRUE;
        deinterlace_control->enable     = ISIL_TRUE;
        deinterlace_control->sensitive  = 0;

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_adaptive_deinterlace_control_reset(struct isil_adaptive_deinterlace_control *deinterlace_control)
{
    if(deinterlace_control) {
        deinterlace_control->last_flags = ISIL_TRUE;
        deinterlace_control->curr_flags = ISIL_TRUE;
        deinterlace_control->enable     = ISIL_TRUE;
        deinterlace_control->sensitive  = 0;

        return ISIL_OK;
    }

    return ISIL_ERR;
}
static int isil_adaptive_deinterlace_control_adjust_sensitive(struct isil_adaptive_deinterlace_control *deinterlace_control, u32 sensitive)
{
    if(deinterlace_control) {
        deinterlace_control->sensitive  = sensitive;

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_adaptive_deinterlace_control_update(struct isil_adaptive_deinterlace_control *deinterlace_control)
{
    if(deinterlace_control) {
        deinterlace_control->last_flags = deinterlace_control->curr_flags;

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_adaptive_deinterlace_control_detect(struct isil_adaptive_deinterlace_control *deinterlace_control, isil_chip_t *chip, u32 phy_slot_id)
{
    int ret = ISIL_ERR;

    if(deinterlace_control && chip && (phy_slot_id < ISIL_PHY_VD_CHAN_NUMBER)) {
        mpb_write(chip, RGR_MOTION_SEL, phy_slot_id);
        if(mpb_read(chip, MD_DI_CNTL)){
            deinterlace_control->curr_flags = ISIL_TRUE;
            deinterlace_control->enable     = ISIL_TRUE;
        }else{
            deinterlace_control->curr_flags = ISIL_FALSE;
            deinterlace_control->enable     = ISIL_FALSE;
        }
        ret = ISIL_OK;
    }else{
        ISIL_DBG(ISIL_DBG_ERR, "invalid parameter\n");
    }

    return ret;
}
static int isil_adaptive_deinterlace_control_ischange(struct isil_adaptive_deinterlace_control *deinterlace_control)
{
    if(deinterlace_control) {
        if(deinterlace_control->last_flags != deinterlace_control->curr_flags){
            //ISIL_DBG(ISIL_DBG_DEBUG, "phy deinterlace is changed %d\n", deinterlace_control->curr_flags);
            return ISIL_TRUE;
        }else{
            return ISIL_FALSE;
        }
    }

    return ISIL_ERR;
}

struct isil_adaptive_deinterlace_control_operation isil_adaptive_deinterlace_control_operation_op = {
    .init             = isil_adaptive_deinterlace_control_init,
    .reset            = isil_adaptive_deinterlace_control_reset,

    .adjust_sensitive = isil_adaptive_deinterlace_control_adjust_sensitive,
    .update           = isil_adaptive_deinterlace_control_update,
    .detect           = isil_adaptive_deinterlace_control_detect,
    .ischange         = isil_adaptive_deinterlace_control_ischange,
};

static int  isil_h264_phy_video_slot_monitor(void *context)
{
    isil_h264_phy_video_slot_t *phy_video_slot = (isil_h264_phy_video_slot_t*)context;
    if(phy_video_slot != NULL){
        isil_h264_logic_encode_chan_t *master_h264_logic_encode_chan, *sub_h264_logic_encode_chan;
        isil_chip_t *chip=NULL;
        int logic_chan_id;
        struct isil_adaptive_deinterlace_control *deinterlace;

        logic_chan_id = phy_video_slot->phy_slot_id;
        logic_chan_id <<= VIDEO_MASTER_OR_SUB_FLAG_LEFT_SHIFT_NUMBER;
        logic_chan_id |= ISIL_MASTER_BITSTREAM;
        find_register_opened_logic_encode_chan_in_master_slot(phy_video_slot, logic_chan_id, &master_h264_logic_encode_chan);
        if(master_h264_logic_encode_chan != NULL){
            chip = master_h264_logic_encode_chan->chip;
            isil_h264_logic_encode_chan_monitor(master_h264_logic_encode_chan, phy_video_slot);
        }

        logic_chan_id = phy_video_slot->phy_slot_id;
        logic_chan_id <<= VIDEO_MASTER_OR_SUB_FLAG_LEFT_SHIFT_NUMBER;
        logic_chan_id |= ISIL_SUB_BITSTREAM;
        find_register_opened_logic_encode_chan_in_master_slot(phy_video_slot, logic_chan_id, &sub_h264_logic_encode_chan);
        if(sub_h264_logic_encode_chan != NULL){
            chip = sub_h264_logic_encode_chan->chip;
            isil_h264_logic_encode_chan_monitor(sub_h264_logic_encode_chan, phy_video_slot);
        }

        deinterlace = &phy_video_slot->deinterlace;
        if(chip != NULL){
            deinterlace->op->detect(deinterlace, chip, phy_video_slot->phy_slot_id);
            if(deinterlace->op->ischange(deinterlace)){
                isil_h264_encode_feature_t  *pconfig;
                isil_h264_encode_property_t *encode_property;

                if(master_h264_logic_encode_chan != NULL){
                    encode_property = &master_h264_logic_encode_chan->encode_property;
                    pconfig = &encode_property->encode_feature;
                    if(deinterlace->enable == ISIL_TRUE) {
                        pconfig->config_feature_param.b_enable_deinterlace = ISIL_H264_FEATURE_ON;
                    }else{
                        pconfig->config_feature_param.b_enable_deinterlace = ISIL_H264_FEATURE_OFF;
                    }
                    pconfig->config_feature_param.change_mask_flag |= ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_DEINTERLACE_MASK;
                }
                if(sub_h264_logic_encode_chan != NULL){
                    encode_property = &sub_h264_logic_encode_chan->encode_property;
                    pconfig = &encode_property->encode_feature;
                    if(deinterlace->enable == ISIL_TRUE) {
                        pconfig->config_feature_param.b_enable_deinterlace = ISIL_H264_FEATURE_ON;
                    }else{
                        pconfig->config_feature_param.b_enable_deinterlace = ISIL_H264_FEATURE_OFF;
                    }
                    pconfig->config_feature_param.change_mask_flag |= ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_DEINTERLACE_MASK;
                }
            }
        }
        deinterlace->op->update(deinterlace);
    }
    return 0;
}

static void start_encode_chan_monitor_hook(isil_h264_phy_video_slot_t *phy_video_slot, isil_chip_t *chip)
{
    if(phy_video_slot != NULL){
        isil_chip_timer_controller_t    *chip_timer_cont = &chip->chip_timer_cont;
         phy_video_slot->encode_chan_monitor_timeid = chip_timer_cont->op->AddForFireTimerJob(chip_timer_cont, ENCODE_CHAN_MONITOR_COUNTER, isil_h264_phy_video_slot_monitor, phy_video_slot);
        if(phy_video_slot->encode_chan_monitor_timeid == ADDJOBERROR){
            printk("%s.%d: add timer hook err\n", __FUNCTION__, __LINE__);
        }
    }
}

static void delete_encode_chan_monitor_hook(isil_h264_phy_video_slot_t *phy_video_slot, isil_chip_t *chip)
{
    if(phy_video_slot != NULL){
        if(phy_video_slot->encode_chan_monitor_timeid != INVALIDTIMERID){
            isil_chip_timer_controller_t    *chip_timer_cont = &chip->chip_timer_cont;
            
            chip_timer_cont->op->DeleteForFireTimerJob(chip_timer_cont, phy_video_slot->encode_chan_monitor_timeid);
            phy_video_slot->encode_chan_monitor_timeid = INVALIDTIMERID;
        }
    }
}

static void isil_h264_phy_video_slot_init(isil_h264_phy_video_slot_t *phy_video_slot, int phy_slot_id, enum ISIL_VIDEO_SIZE video_size)
{

    if(phy_video_slot != NULL){
        struct isil_adaptive_deinterlace_control *deinterlace;
        atomic_set(&phy_video_slot->first_start_flag, 1);
        phy_video_slot->phy_slot_id = phy_slot_id;
        phy_video_slot->phy_bus_id = phy_slot_id / 4;
        phy_video_slot->map_logic_id = ISIL_LOGIC_VD_INVALID;
        phy_video_slot->cross_bus_logic_video_slot = NULL;
        phy_video_slot->video_size = video_size;
        phy_video_slot->capability = video_size;
        phy_video_slot->interlace = ISIL_FRAME_MODE_INTERLACE;
        init_isil_vd_slot_orig_buf_info(&phy_video_slot->vd_orig_buf, phy_slot_id);
        init_register_table_node(&phy_video_slot->logic_chan_table);
        init_register_table_node(&phy_video_slot->opened_logic_chan_table);
        phy_video_slot->encode_chan_monitor_timeid = INVALIDTIMERID;
        phy_video_slot->start_encode_chan_monitor_hook = start_encode_chan_monitor_hook;
        phy_video_slot->delete_encode_chan_monitor_hook = delete_encode_chan_monitor_hook;

        deinterlace = &phy_video_slot->deinterlace;
        deinterlace->op = &isil_adaptive_deinterlace_control_operation_op;
        deinterlace->op->init(deinterlace);
    }
}

static void	isil_h264_phy_video_slot_reset(isil_h264_phy_video_slot_t *phy_video_slot, isil_chip_t *chip)
{
    struct isil_adaptive_deinterlace_control *deinterlace;

    deinterlace = &phy_video_slot->deinterlace;
    deinterlace->op->reset(deinterlace);

}

static void isil_h264_phy_video_slot_software_discard_framer(isil_h264_phy_video_slot_t *phy_video_slot, isil_chip_t *chip, isil_h264_logic_encode_chan_t *h264_logic_encode_chan)
{
    if((chip!=NULL)){
        isil_vd_orig_buf_info_t   *vd_orig_buf = &phy_video_slot->vd_orig_buf;
        vd_orig_buf->op->software_discard_frame(vd_orig_buf, chip, h264_logic_encode_chan);
    }
}

static int  isil_h264_phy_video_slot_ad_is_ready(isil_h264_phy_video_slot_t *phy_video_slot, isil_chip_t *chip, isil_h264_logic_encode_chan_t *h264_logic_encode_chan)
{
    isil_vd_orig_buf_info_t   *vd_orig_buf = &phy_video_slot->vd_orig_buf;
    return vd_orig_buf->op->vd_orig_buf_ready(vd_orig_buf, chip, h264_logic_encode_chan);
}

static void isil_h264_phy_video_slot_clear_ad_ready(isil_h264_phy_video_slot_t *phy_video_slot, isil_chip_t *chip, isil_h264_logic_encode_chan_t *h264_logic_encode_chan)
{
    isil_vd_orig_buf_info_t   *vd_orig_buf = &phy_video_slot->vd_orig_buf;
    vd_orig_buf->op->clear_vd_orig_buf_ready(vd_orig_buf, chip, h264_logic_encode_chan);
}

static void isil_h264_phy_video_slot_first_start(isil_h264_phy_video_slot_t *phy_video_slot, isil_chip_t *chip)
{
    if(atomic_read(&phy_video_slot->first_start_flag)){
        isil_vd_orig_buf_info_t	*vd_orig_buf;
        int  select_chan;
        //u32 data;

        atomic_set(&phy_video_slot->first_start_flag, 0);
        select_chan = chip->io_op->chip_read32(chip, CHANNEL_SELECT);
        select_chan |= (1<<phy_video_slot->phy_slot_id);
        chip->io_op->chip_write32(chip, CHANNEL_SELECT, select_chan);
        //data = chip->io_op->chip_read32(chip, CHANNEL_ID);
        //data = (data & (~0xf)) | phy_video_slot->phy_slot_id;
        //chip->io_op->chip_write32(chip, CHANNEL_ID, data);
        //chip->io_op->chip_write32(chip, REFERENCEFRAME, 0x0440);
        //chip->io_op->chip_write32(chip, INTRANDMECONTROL, 0x0000);
        //chip->io_op->chip_write32(chip, WINDOWSIZE, SEARCH_WINDOW_SIZE_VALUE);
        //chip->io_op->chip_write32(chip, INTRAENABLE, 0x0060);
        //chip->io_op->chip_write32(chip, INTRA4X4_THRESHOLD_I, 0x18);
        //chip->io_op->chip_write32(chip, INTRA4X4_THRESHOLD_IP, 0x20);
        //chip->io_op->chip_write32(chip, INTRA4X4_THRESHOLD_IP, 0x0);
        //chip->io_op->chip_write32(chip, CHIPSCOPE, 0x0070);
        //chip->io_op->chip_write32(chip, ORIGINALSEQUENCE, 0x0440);

        vd_orig_buf = &phy_video_slot->vd_orig_buf;
        if(vd_orig_buf->op != NULL){
            vd_orig_buf->op->first_sync_orig_buf_info(vd_orig_buf, chip);
        } else {
            isil_vd_slot_orig_buf_info_first_sync_orig_buf_info(vd_orig_buf, chip);
        }
        chip->video_encode_cap->encode_reg_4 = (DEBLOCKINGON|INTRAON|CMOSON|INTERON|DDRON);

        //chip->io_op->chip_write32(chip, 0x0008, 0x0000);
        //chip->io_op->chip_write32(chip, MODE, chip->video_encode_cap->encode_reg_4);
        //chip->io_op->chip_write32(chip, 0x0008, 0x0800);
        phy_video_slot->start_encode_chan_monitor_hook(phy_video_slot, chip);
    }
}

static void isil_h264_phy_video_slot_end_stop(isil_h264_phy_video_slot_t *phy_video_slot, isil_h264_logic_encode_chan_t *h264_logic_encode_chan)
{
    if((h264_logic_encode_chan!=NULL)){
        isil_chip_t  *chip = h264_logic_encode_chan->chip;
        int  select_chan;
        select_chan = chip->io_op->chip_read32(chip, CHANNEL_SELECT);
        select_chan &= ~(1<<phy_video_slot->phy_slot_id);
        chip->io_op->chip_write32(chip, CHANNEL_SELECT, select_chan);
        atomic_set(&phy_video_slot->first_start_flag, 1);
    }
}

static void isil_h264_phy_video_slot_set_slot_encode_ptr(isil_h264_phy_video_slot_t *phy_video_slot, isil_chip_t *chip, isil_h264_logic_encode_chan_t *h264_logic_encode_chan)
{
    if((chip!=NULL) && (h264_logic_encode_chan!=NULL)){
        phy_video_slot->vd_orig_buf.op->set_vd_curr_encode_ptr(&phy_video_slot->vd_orig_buf, chip, h264_logic_encode_chan);
    }
}

static u32  isil_h264_phy_video_slot_get_timestamp(isil_h264_phy_video_slot_t *phy_video_slot)
{
    isil_vd_orig_buf_info_t   *vd_orig_buf;
    vd_orig_buf = &phy_video_slot->vd_orig_buf;
    return vd_orig_buf->op->get_timestamp(vd_orig_buf);
}

static void isil_h264_phy_video_slot_encode_endofslice_notify(isil_h264_phy_video_slot_t *phy_video_slot, isil_h264_logic_encode_chan_t *h264_logic_encode_chan)
{
    if((h264_logic_encode_chan!=NULL)){
        phy_video_slot->vd_orig_buf.op->encode_endofslice_notify(&phy_video_slot->vd_orig_buf, h264_logic_encode_chan);
    }
}

static void isil_h264_phy_video_slot_encode_unregister_notify(isil_h264_phy_video_slot_t *phy_video_slot, isil_h264_logic_encode_chan_t *h264_logic_encode_chan)
{
    if((h264_logic_encode_chan!=NULL)){
        phy_video_slot->vd_orig_buf.op->encode_unregister_notify(&phy_video_slot->vd_orig_buf, h264_logic_encode_chan);
    }
}

static void isil_h264_phy_video_slot_update_map_logic_slot(isil_h264_phy_video_slot_t *phy_video_slot, int map_logic_id, isil_h264_logic_video_slot_t *logic_video_slot)
{
    if(phy_video_slot != NULL){
        phy_video_slot->map_logic_id = map_logic_id;
        if(map_logic_id == ISIL_LOGIC_VD_INVALID){
            phy_video_slot->cross_bus_logic_video_slot = NULL;
        } else {
            phy_video_slot->cross_bus_logic_video_slot = logic_video_slot;
        }
    }
}

static int  isil_h264_phy_video_slot_get_video_size(isil_h264_phy_video_slot_t *phy_video_slot)
{
    if(phy_video_slot != NULL){
        return phy_video_slot->video_size;
    } else {
        return ISIL_VIDEO_SIZE_D1;
    }
}

static void isil_h264_phy_video_slot_set_video_size(isil_h264_phy_video_slot_t *phy_video_slot, enum ISIL_VIDEO_SIZE video_size)
{
    if(phy_video_slot != NULL){
        phy_video_slot->video_size = video_size;
    }
}

static void isil_h264_get_phy_slot_by_phy_slot_id(isil_chip_t *chip, int phy_slot_id, isil_h264_phy_video_slot_t **phy_video_slot)
{
    if(chip && phy_video_slot && (phy_slot_id >= 0) && (phy_slot_id < ISIL_PHY_VD_CHAN_NUMBER)) {
        *phy_video_slot = &chip->chip_driver->video_bus.vd_bus_driver->phy_video_slot[phy_slot_id];
    }else{
        *phy_video_slot = NULL;
        ISIL_DBG(ISIL_DBG_ERR, "invalid phy_slot_id %d\n", phy_slot_id);
    }
}

static int isil_h264_phy_video_slot_get_interlace(isil_h264_phy_video_slot_t *phy_video_slot)
{
    if(phy_video_slot){
            return phy_video_slot->interlace;
    }

    return ISIL_ERR;
}
static int isil_h264_phy_video_slot_set_interlace(isil_h264_phy_video_slot_t *phy_video_slot, enum ISIL_FRAME_MODE mode)
{
    if(phy_video_slot){
        if((mode > 0) && (mode < ISIL_FRAME_MODE_RESERVED)){
            phy_video_slot->interlace = mode;
            return ISIL_OK;
        }
    }

    return ISIL_ERR;
}
static struct isil_h264_phy_video_slot_operation	isil_h264_phy_video_slot_op = {
    .init = isil_h264_phy_video_slot_init,
    .slot_reset = isil_h264_phy_video_slot_reset,
    .slot_software_discard_frame = isil_h264_phy_video_slot_software_discard_framer,
    .slot_ad_is_ready = isil_h264_phy_video_slot_ad_is_ready,
    .slot_clear_ad_ready = isil_h264_phy_video_slot_clear_ad_ready,
    .slot_first_start = isil_h264_phy_video_slot_first_start,
    .slot_end_stop = isil_h264_phy_video_slot_end_stop,
    .set_slot_encode_ptr = isil_h264_phy_video_slot_set_slot_encode_ptr,
    .get_timestamp = isil_h264_phy_video_slot_get_timestamp,
    .encode_endofslice_notify = isil_h264_phy_video_slot_encode_endofslice_notify,
    .encode_unregister_notify = isil_h264_phy_video_slot_encode_unregister_notify,
    .update_map_logic_slot = isil_h264_phy_video_slot_update_map_logic_slot,
    .get_video_size = isil_h264_phy_video_slot_get_video_size,
    .set_video_size = isil_h264_phy_video_slot_set_video_size,
    .set_interlace = isil_h264_phy_video_slot_set_interlace,
    .get_interlace = isil_h264_phy_video_slot_get_interlace,
    .get_phy_slot_by_phy_slot_id = isil_h264_get_phy_slot_by_phy_slot_id,
};

static void init_isil_h264_phy_video_slot(isil_h264_phy_video_slot_t *phy_video_slot, int slot_id, enum ISIL_VIDEO_SIZE video_size)
{
    if(phy_video_slot != NULL){
        phy_video_slot->op = &isil_h264_phy_video_slot_op;
        phy_video_slot->op->init(phy_video_slot, slot_id, video_size);
    }
}

static void remove_isil_h264_phy_video_slot(isil_h264_phy_video_slot_t *phy_video_slot, isil_h264_logic_encode_chan_t *h264_logic_encode_chan)
{
    if(phy_video_slot != NULL){
        isil_register_table_t	*opened_logic_chan_table = &phy_video_slot->opened_logic_chan_table;
        if(opened_logic_chan_table->op->get_curr_register_node_number(opened_logic_chan_table) == 0){
            phy_video_slot->delete_encode_chan_monitor_hook(phy_video_slot, h264_logic_encode_chan->chip);
            phy_video_slot->op->slot_end_stop(phy_video_slot, h264_logic_encode_chan);
        }
    }
}

static void isil_h264_encode_control_set_sps(isil_h264_encode_control_t *h264_encode_control)
{
    isil_h264_logic_encode_chan_t *h264_logic_encode_chan;
    isil_h264_encode_property_t	*encode_property;
    h264_sps_t	*sps;
    int	gopIntervals;

    sps = h264_encode_control->sps;
    h264_logic_encode_chan = to_get_isil_h264_encode_chan_with_encode_control(h264_encode_control);
    encode_property = &h264_logic_encode_chan->encode_property;
    if(encode_property->op != NULL){
        gopIntervals = encode_property->op->get_gopIntervals(encode_property);
    } else {
        gopIntervals = isil_h264_encode_property_get_gopIntervals(encode_property);
    }
    sps->i_id = 0;
    sps->i_profile_idc  = PROFILE_BASELINE;
    sps->i_level_idc = DEFAULT_LEVEL_IDC;
    sps->b_constraint_set0  = 0;
    sps->b_constraint_set1  = 0;
    sps->b_constraint_set2  = 0;
    sps->i_log2_max_frame_num = 4;  /* at least 4 */
    while( (1 << sps->i_log2_max_frame_num) < gopIntervals){
        sps->i_log2_max_frame_num++;
    }
    sps->i_poc_type = 0;
    if( sps->i_poc_type == 0 ){
        sps->i_log2_max_poc_lsb = sps->i_log2_max_frame_num;
    } else if ( sps->i_poc_type == 1 ) {
        int i;
        sps->b_delta_pic_order_always_zero = 1;
        sps->i_offset_for_non_ref_pic = 0;
        sps->i_offset_for_top_to_bottom_field = 0;
        sps->i_num_ref_frames_in_poc_cycle = 0;
        for ( i = 0; i < sps->i_num_ref_frames_in_poc_cycle; i++ ){
            sps->i_offset_for_ref_frame[i] = 0;
        }
    }
    sps->i_num_ref_frames = 1;
    sps->b_gaps_in_frame_num_value_allowed = 0;
    sps->i_mb_width = h264_encode_control->i_mb_x;
    sps->i_mb_height = h264_encode_control->i_mb_y;
    sps->b_frame_mbs_only = 1;
    sps->b_mb_adaptive_frame_field = 0;
    sps->b_direct8x8_inference = 0;
    if( sps->b_frame_mbs_only == 0){
        sps->b_direct8x8_inference = 1;
    }
    sps->b_crop = 0;
    sps->b_vui = 0;
}

static void isil_h264_encode_control_set_pps(isil_h264_encode_control_t *h264_encode_control)
{
    isil_h264_logic_encode_chan_t *h264_logic_encode_chan;
    isil_h264_encode_property_t	*encode_property;
    h264_sps_t	*sps = h264_encode_control->sps;
    h264_pps_t	*pps = h264_encode_control->pps;

    h264_logic_encode_chan = to_get_isil_h264_encode_chan_with_encode_control(h264_encode_control);
    encode_property = &h264_logic_encode_chan->encode_property;
    pps->i_id = 0;
    pps->i_sps_id = sps->i_id;
    pps->b_cabac = 0;
    pps->b_pic_order = 0;
    pps->i_num_slice_groups = 1;
    pps->i_num_ref_idx_l0_active = 1;
    pps->i_num_ref_idx_l1_active = 1;
    pps->b_weighted_pred = 0;
    pps->b_weighted_bipred =  0;
    pps->i_pic_init_qp = DEFAULT_QP;
    pps->i_pic_init_qs = pps->i_pic_init_qp;
    pps->i_chroma_qp_index_offset = 0;
    pps->b_deblocking_filter_control = 0;
    pps->b_constrained_intra_pred = 0;
    pps->b_redundant_pic_cnt = 0;
}

static void isil_h264_encode_control_set_slice_head(isil_h264_encode_control_t *h264_encode_control)
{
    h264_slice_header_t *slice = h264_encode_control->slice_head;
    h264_sps_t	*sps = h264_encode_control->sps;
    h264_pps_t	*pps = h264_encode_control->pps;

    slice->sps = sps;
    slice->pps = pps;
    slice->i_first_mb = h264_encode_control->i_first_mb;
    slice->i_last_mb = sps->i_mb_width * sps->i_mb_height;
    slice->i_pps_id = pps->i_id;
    slice->i_frame_num = h264_encode_control->i_slice_frame_num;
    slice->b_field_pic = 0;
    slice->b_bottom_field = 1;
    slice->i_poc_lsb = 0;
    slice->i_delta_poc_bottom = 0;
    slice->i_delta_poc[0] = 0;
    slice->i_delta_poc[1] = 0;
    slice->i_redundant_pic_cnt = 0;
    slice->b_num_ref_idx_override = 0;
    slice->i_num_ref_idx_l0_active = 1;
    slice->i_num_ref_idx_l1_active = 1;
    if(slice->i_frame_num<=slice->i_num_ref_idx_l0_active){
        slice->i_redundant_pic_cnt = 1;
    } else {
        slice->i_redundant_pic_cnt = 0;
    }
    slice->i_qp_delta = (h264_encode_control->i_curr_qp - pps->i_pic_init_qp);
    slice->b_sp_for_swidth = 0;
    slice->i_qs_delta = 0;
    slice->i_disable_deblocking_filter_idc = 0;
    slice->i_alpha_c0_offset = 0;
    slice->i_beta_offset = 0;
}

static void isil_h264_encode_control_calculate_frame_type(isil_h264_encode_control_t *h264_encode_control)
{
    if(h264_encode_control != NULL){
        h264_slice_header_t *slice_head = h264_encode_control->slice_head;
        if(h264_encode_control->i_gop_stride == 0) {
            h264_encode_control->frame_type = H264_FRAME_TYPE_IDR;
        } else {
            if((h264_encode_control->i_p_stride==0) || (h264_encode_control->b_force_i_frame)){
                h264_encode_control->frame_type = H264_FRAME_TYPE_IDR;
            } else {
                h264_encode_control->frame_type = H264_FRAME_TYPE_P;
            }
        }
        if(h264_encode_control->frame_type == H264_FRAME_TYPE_IDR){
            h264_encode_control->b_force_i_frame = 0;
            h264_encode_control->i_p_stride = 0;
            h264_encode_control->i_slice_frame_num = 0;
            h264_encode_control->i_frame_per_second = 0;        
        }
        switch (h264_encode_control->frame_type) {
            case H264_FRAME_TYPE_P:
                slice_head->i_type = SLICE_TYPE_P;
                break;
            case H264_FRAME_TYPE_I:
                slice_head->i_type = SLICE_TYPE_I;
                break;
            default:
            case H264_FRAME_TYPE_IDR:
                slice_head->i_type = SLICE_TYPE_I;
                h264_encode_control->i_idr_pic_id++;
                h264_encode_control->i_idr_pic_id &= 0xffff;
                slice_head->i_idr_pic_id = h264_encode_control->i_idr_pic_id;
                break;
        }
    }
}

static int  isil_h264_encode_control_get_frame_type(isil_h264_encode_control_t *h264_encode_control)
{
    if(h264_encode_control != NULL){
        return h264_encode_control->frame_type;
    } else {
        return H264_FRAME_TYPE_IDR;
    }
}

static void isil_h264_encode_control_generator_head(isil_h264_encode_control_t *h264_encode_control, isil_video_frame_tcb_t *frame)
{
    if((h264_encode_control!=NULL) && (frame!=NULL)){
        int frame_type;
        frame_type = h264_encode_control->op->get_frame_type(h264_encode_control);
        switch(frame_type){
            default:
            case H264_FRAME_TYPE_IDR:
                gen_h264_sps(h264_encode_control, frame);
                gen_h264_pps(h264_encode_control, frame);
                gen_h264_slicehead(h264_encode_control, frame, H264_FRAME_TYPE_IDR);
                break;
            case H264_FRAME_TYPE_I:
                gen_h264_slicehead(h264_encode_control, frame, H264_FRAME_TYPE_I);
                break;
            case H264_FRAME_TYPE_P:
                gen_h264_slicehead(h264_encode_control, frame, H264_FRAME_TYPE_P);
                break;
        }
    }
}

static void isil_h264_encode_control_force_I_frame(isil_h264_encode_control_t *h264_encode_control)
{
    h264_encode_control->b_force_i_frame = 1;
}

static void isil_h264_encode_control_flush_all_frame(isil_h264_encode_control_t *h264_encode_control)
{
    h264_encode_control->b_flush_all_frame = ((1<<FLUSH_CURR_PRODUCER_FRAME)|(1<<FLUSH_CURR_CONSUMER_FRAME));
    h264_encode_control->op->force_I_frame(h264_encode_control);
    h264_encode_control->op->flush_frame_queue(h264_encode_control);
}

static void isil_h264_encode_control_flush_curr_producer_frame(isil_h264_encode_control_t *h264_encode_control)
{
    isil_h264_logic_encode_chan_t   *h264_logic_encode_chan;
    isil_video_frame_tcb_queue_t    *video_frame_queue;
    isil_video_chan_buf_pool_t      *video_chan_buf_pool;

    h264_logic_encode_chan = to_get_isil_h264_encode_chan_with_encode_control(h264_encode_control);
    video_frame_queue = &h264_logic_encode_chan->encode_frame_queue;
    video_chan_buf_pool = &h264_logic_encode_chan->encode_chan_buf_pool;
    if(h264_encode_control->b_flush_all_frame & (1<<FLUSH_CURR_PRODUCER_FRAME)){
        video_frame_queue->op->release_curr_producer(video_frame_queue, video_chan_buf_pool);
        h264_encode_control->b_flush_all_frame &= ~(1<<FLUSH_CURR_PRODUCER_FRAME);
    }
}

static void isil_h264_encode_control_flush_curr_consumer_frame(isil_h264_encode_control_t *h264_encode_control)
{
    isil_h264_logic_encode_chan_t   *h264_logic_encode_chan;
    isil_video_frame_tcb_queue_t    *video_frame_queue;
    isil_video_chan_buf_pool_t      *video_chan_buf_pool;
    unsigned long   flags;

    h264_logic_encode_chan = to_get_isil_h264_encode_chan_with_encode_control(h264_encode_control);
    spin_lock_irqsave(&h264_logic_encode_chan->fsm_matrix_call_lock, flags);
    video_frame_queue = &h264_logic_encode_chan->encode_frame_queue;
    video_chan_buf_pool = &h264_logic_encode_chan->encode_chan_buf_pool;
    if(h264_encode_control->b_flush_all_frame & (1<<FLUSH_CURR_CONSUMER_FRAME)){
        video_frame_queue->op->release_curr_consumer(video_frame_queue, video_chan_buf_pool);
        h264_encode_control->op->flush_frame_queue(h264_encode_control);        
        h264_encode_control->b_flush_all_frame &= ~(1<<FLUSH_CURR_CONSUMER_FRAME);
    }
    spin_unlock_irqrestore(&h264_logic_encode_chan->fsm_matrix_call_lock, flags);
}

static void isil_h264_encode_control_flush_frame_queue(isil_h264_encode_control_t *h264_encode_control)
{
    isil_h264_logic_encode_chan_t   *h264_logic_encode_chan;
    isil_video_frame_tcb_queue_t    *video_frame_queue;
    isil_video_chan_buf_pool_t      *video_chan_buf_pool;
    isil_video_frame_tcb_t          *temp_frame;
    unsigned long   flags;

    h264_logic_encode_chan = to_get_isil_h264_encode_chan_with_encode_control(h264_encode_control);
    video_frame_queue = &h264_logic_encode_chan->encode_frame_queue;
    video_chan_buf_pool = &h264_logic_encode_chan->encode_chan_buf_pool;
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

static void isil_h264_encode_control_update_at_encode_frame_ok(isil_h264_encode_control_t *h264_encode_control)
{
    if(h264_encode_control != NULL){
        isil_h264_logic_encode_chan_t *h264_logic_encode_chan;
        isil_h264_encode_property_t   *encode_property;
        int	keyFrameIntervals, gopIntervals;

        h264_logic_encode_chan = to_get_isil_h264_encode_chan_with_encode_control(h264_encode_control);
        encode_property = &h264_logic_encode_chan->encode_property;
        h264_encode_control->i_frameNumber++;
        h264_encode_control->i_frame_serial++;
        h264_encode_control->i_slice_frame_num++;
        h264_encode_control->i_gop_stride++;
        h264_encode_control->i_p_stride++;
        keyFrameIntervals = encode_property->op->get_keyFrameIntervals(encode_property);
        gopIntervals = encode_property->op->get_gopIntervals(encode_property);
        if(h264_encode_control->i_p_stride >= keyFrameIntervals){
            h264_encode_control->i_p_stride = 0;
        }
        //        if(h264_encode_control->i_slice_frame_num >= gopIntervals){
        //            h264_encode_control->i_slice_frame_num = 0;
        //        }
        if(h264_encode_control->i_gop_stride >= gopIntervals){
            h264_encode_control->i_gop_stride = 0;
        }
        h264_encode_control->i_vd_reference_ptr++;
        h264_encode_control->i_vd_reference_ptr &= 3;
    }
}

static void isil_h264_encode_control_update_at_discard_frame(isil_h264_encode_control_t *h264_encode_control)
{
    if(h264_encode_control != NULL){
        isil_h264_logic_encode_chan_t *h264_logic_encode_chan;
        isil_h264_encode_property_t   *encode_property;
        int keyFrameIntervals, gopIntervals;

        h264_logic_encode_chan = to_get_isil_h264_encode_chan_with_encode_control(h264_encode_control);
        encode_property = &h264_logic_encode_chan->encode_property;
        //h264_encode_control->i_frame_serial++;
        h264_encode_control->i_gop_stride++;
        //h264_encode_control->i_slice_frame_num++;
        h264_encode_control->i_p_stride++;
        if(encode_property->op != NULL){
            keyFrameIntervals = encode_property->op->get_keyFrameIntervals(encode_property);
            gopIntervals = encode_property->op->get_gopIntervals(encode_property);
        } else {
            keyFrameIntervals = isil_h264_encode_property_get_keyFrameIntervals(encode_property);
            gopIntervals = isil_h264_encode_property_get_gopIntervals(encode_property);
        }
        if(h264_encode_control->i_p_stride >= keyFrameIntervals){
            h264_encode_control->i_p_stride = 0;
        }
        //if(h264_encode_control->i_slice_frame_num >= gopIntervals){
        //    h264_encode_control->i_slice_frame_num = 0;
        //}
        if(h264_encode_control->i_gop_stride >= gopIntervals){
            h264_encode_control->i_gop_stride = 0;
        }
    }
}

static void isil_h264_encode_control_update_at_encode_frame_err(isil_h264_encode_control_t *h264_encode_control)
{
    if(h264_encode_control != NULL){
        isil_h264_logic_encode_chan_t *h264_logic_encode_chan;
        isil_h264_encode_property_t	*encode_property;
        int	keyFrameIntervals, gopIntervals;

        h264_logic_encode_chan = to_get_isil_h264_encode_chan_with_encode_control(h264_encode_control);
        encode_property = &h264_logic_encode_chan->encode_property;
        //h264_encode_control->i_frame_serial++;
        //h264_encode_control->i_slice_frame_num++;
        h264_encode_control->i_gop_stride++;
        h264_encode_control->i_p_stride++;
        keyFrameIntervals = encode_property->op->get_keyFrameIntervals(encode_property);
        gopIntervals = encode_property->op->get_gopIntervals(encode_property);
        if(h264_encode_control->i_p_stride >= keyFrameIntervals){
            h264_encode_control->i_p_stride = 0;
        }
        //if(h264_encode_control->i_slice_frame_num >= gopIntervals){
        //    h264_encode_control->i_slice_frame_num = 0;
        //}
        if(h264_encode_control->i_gop_stride >= gopIntervals){
            h264_encode_control->i_gop_stride = 0;
        }
    }
}

static void isil_h264_encode_control_update_image_resolution(isil_h264_encode_control_t *h264_encode_control)
{
    if(h264_encode_control != NULL){
        isil_h264_logic_encode_chan_t *h264_logic_encode_chan;
        isil_h264_encode_property_t   *encode_property;
        osd_chan_engine_t *osd_engine;
        int i_mb_number;

        h264_logic_encode_chan = to_get_isil_h264_encode_chan_with_encode_control(h264_encode_control);
        encode_property = &h264_logic_encode_chan->encode_property;
        osd_engine = &h264_logic_encode_chan->encode_osd_engine;
        i_mb_number = encode_property->op->get_encodeSize_width(encode_property);
        //h264_encode_control->i_mb_x = ((i_mb_number+15)>>4);
        h264_encode_control->i_mb_x = ((i_mb_number)>>4);
        i_mb_number = encode_property->op->get_encodeSize_height(encode_property);
        //h264_encode_control->i_mb_y = ((i_mb_number+15)>>4);
        h264_encode_control->i_mb_y = ((i_mb_number)>>4);
        h264_encode_control->i_first_mb = 0;
        h264_encode_control->i_first_mb_x_and_y = 0;
        ISIL_DBG(ISIL_DBG_INFO, "channel %d video size changed, resync to osd\n", h264_logic_encode_chan->logic_chan_id);
        atomic_set(&osd_engine->sync_osd_param, 1);
    }
}

static void isil_h264_encode_control_update_fps_discard_table(isil_h264_encode_control_t *h264_encode_control, u32 phy_fps, u32 target_fps)
{
    if(h264_encode_control) {
        u32 mask, i;

        h264_encode_control->i_software_discard_frame_table = get_bits_map(phy_fps, target_fps);
        mask = 0;
        for(i = 0; i < phy_fps; i++) {
            if(h264_encode_control->i_software_discard_frame_table & (1<<i)) {
                mask++;
            }
        }
    }
}

static int  isil_h264_encode_control_get_slice_head_and_ad_status(isil_h264_encode_control_t *h264_encode_control)
{
    int	ret = 0;
    if(h264_encode_control != NULL){
        unsigned long   flags;

        spin_lock_irqsave(&h264_encode_control->slice_head_and_ad_status_lock, flags);
        ret = atomic_read(&h264_encode_control->slice_head_and_ad_status);
        if((ret&DEV_CAN_SERVICE) == DEV_CAN_SERVICE){
            h264_encode_control->op->clear_slice_head_and_ad_status(h264_encode_control);
            ret = 1;
        } else {
            ret = 0;
        }
        spin_unlock_irqrestore(&h264_encode_control->slice_head_and_ad_status_lock, flags);
    }
    return ret;
}

static void isil_h264_encode_control_clear_slice_head_and_ad_status(isil_h264_encode_control_t *h264_encode_control)
{
    if(h264_encode_control != NULL){
        atomic_set(&h264_encode_control->slice_head_and_ad_status, 0);
    }
}

static void isil_h264_encode_control_update_slice_head_ready(isil_h264_encode_control_t *h264_encode_control)
{
    if(h264_encode_control != NULL){
        unsigned long	flags;
        int	ret;

        spin_lock_irqsave(&h264_encode_control->slice_head_and_ad_status_lock, flags);
        ret = atomic_read(&h264_encode_control->slice_head_and_ad_status);
        ret |= SLICEHEAD_IS_READY;
        atomic_set(&h264_encode_control->slice_head_and_ad_status, ret);
        spin_unlock_irqrestore(&h264_encode_control->slice_head_and_ad_status_lock, flags);
    }
}

static int isil_h264_encode_control_get_slice_head_status(isil_h264_encode_control_t *h264_encode_control)
{
    int	ret = 0;
    if(h264_encode_control != NULL){
        unsigned long	flags;

        spin_lock_irqsave(&h264_encode_control->slice_head_and_ad_status_lock, flags);
        ret = atomic_read(&h264_encode_control->slice_head_and_ad_status);
        ret = (ret & SLICEHEAD_IS_READY ? 1 : 0);
        spin_unlock_irqrestore(&h264_encode_control->slice_head_and_ad_status_lock, flags);
    }

    return ret;
}

static void isil_h264_encode_control_clear_slice_head_ready(isil_h264_encode_control_t *h264_encode_control)
{
    if(h264_encode_control != NULL){
        unsigned long	flags;
        int	ret;

        spin_lock_irqsave(&h264_encode_control->slice_head_and_ad_status_lock, flags);
        ret = atomic_read(&h264_encode_control->slice_head_and_ad_status);
        ret &= ~SLICEHEAD_IS_READY;
        atomic_set(&h264_encode_control->slice_head_and_ad_status, ret);
        spin_unlock_irqrestore(&h264_encode_control->slice_head_and_ad_status_lock, flags);
    }
}

static void isil_h264_encode_control_update_ad_ready(isil_h264_encode_control_t *h264_encode_control)
{
    if(h264_encode_control != NULL){
        unsigned long	flags;
        int	ret;

        spin_lock_irqsave(&h264_encode_control->slice_head_and_ad_status_lock, flags);
        ret = atomic_read(&h264_encode_control->slice_head_and_ad_status);
        ret |= AD_IS_READY;
        atomic_set(&h264_encode_control->slice_head_and_ad_status, ret);
        h264_encode_control->ad_ready++;
        spin_unlock_irqrestore(&h264_encode_control->slice_head_and_ad_status_lock, flags);
    }
}

static void isil_h264_encode_control_clear_ad_ready(isil_h264_encode_control_t *h264_encode_control)
{
    if(h264_encode_control != NULL){
        unsigned long	flags;
        int	ret;

        spin_lock_irqsave(&h264_encode_control->slice_head_and_ad_status_lock, flags);
        ret = atomic_read(&h264_encode_control->slice_head_and_ad_status);
        ret &= ~AD_IS_READY;
        atomic_set(&h264_encode_control->slice_head_and_ad_status, ret);
        spin_unlock_irqrestore(&h264_encode_control->slice_head_and_ad_status_lock, flags);
    }
}

static int isil_h264_encode_control_get_ad_status(isil_h264_encode_control_t *h264_encode_control)
{
    int	ret = 0, status;
    if(h264_encode_control != NULL){
        unsigned long   flags;

        spin_lock_irqsave(&h264_encode_control->slice_head_and_ad_status_lock, flags);
        status = atomic_read(&h264_encode_control->slice_head_and_ad_status);
        if((status&AD_IS_READY)){
            ret = 1;
            status &= ~AD_IS_READY;
        }
        atomic_set(&h264_encode_control->slice_head_and_ad_status, status);
        spin_unlock_irqrestore(&h264_encode_control->slice_head_and_ad_status_lock, flags);
    }
    return ret;
}

static void isil_h264_encode_control_first_start(isil_h264_encode_control_t *h264_encode_control)
{
    isil_h264_logic_encode_chan_t *h264_logic_encode_chan;
    isil_h264_phy_video_slot_t	*master_slot;
    isil_chip_t	*chip;

    h264_logic_encode_chan = to_get_isil_h264_encode_chan_with_encode_control(h264_encode_control);
    if(atomic_read(&h264_logic_encode_chan->first_start_flag)){
        master_slot = h264_logic_encode_chan->master_slot;
        chip = h264_logic_encode_chan->chip;
        if((master_slot!=NULL) && (chip!=NULL)){
            master_slot->op->slot_first_start(master_slot, chip);
            h264_encode_control->i_vd_reference_ptr = master_slot->vd_orig_buf.vd_encode_ptr;
        }
        atomic_set(&h264_logic_encode_chan->first_start_flag, 0);
    }
}

static void isil_h264_encode_control_stop_encode(isil_h264_encode_control_t *h264_encode_control)
{
    isil_h264_logic_encode_chan_t *h264_logic_encode_chan;

    h264_logic_encode_chan = to_get_isil_h264_encode_chan_with_encode_control(h264_encode_control);
    atomic_set(&h264_logic_encode_chan->first_start_flag, 1);
}

static void isil_h264_encode_control_init(isil_h264_encode_control_t *h264_encode_control)
{
    isil_h264_logic_encode_chan_t *h264_logic_encode_chan;
    isil_h264_encode_property_t	*encode_property;
    isil_h264_encode_delta_t      *delta;
    int count;
    isil_h264_phy_video_slot_t *phy_video_slot;
    isil_h264_logic_video_slot_t *logic_video_slot;

    h264_logic_encode_chan = to_get_isil_h264_encode_chan_with_encode_control(h264_encode_control);
    encode_property = &h264_logic_encode_chan->encode_property;
    spin_lock_init(&h264_encode_control->slice_head_and_ad_status_lock);
    atomic_set(&h264_encode_control->slice_head_and_ad_status, 0);
    h264_encode_control->frame_type = H264_FRAME_TYPE_IDR;
    delta = &h264_encode_control->encode_delta;
    phy_video_slot   = h264_logic_encode_chan->master_slot;
    logic_video_slot = phy_video_slot->cross_bus_logic_video_slot;

    h264_encode_control->op->force_I_frame(h264_encode_control);
    h264_encode_control->b_flush_all_frame = 0;
    h264_encode_control->i_frame_serial = 0;
    h264_encode_control->i_frameNumber = 0;
    h264_encode_control->i_slice_frame_num = 0;
    h264_encode_control->i_crossbus_fps = logic_video_slot->op->get_fps(logic_video_slot);
    h264_encode_control->i_gop_stride = 0;
    h264_encode_control->i_p_stride = 0;
    h264_encode_control->i_statistic_minuter_counter = 0;
    h264_encode_control->last_jiffies = 0;
    h264_encode_control->i_curr_fps   = 0;
    h264_encode_control->i_old_fps    = 0;
    h264_encode_control->i_fpm = 0;
    h264_encode_control->i_fps = 0;
    h264_encode_control->ad_ready = 0;
    h264_encode_control->i_last_idr = -encode_property->op->get_keyFrameIntervals(encode_property);
    h264_encode_control->i_idr_pic_id = 0;
    h264_encode_control->i_curr_qp = encode_property->encode_rc.op->get_qpi(&encode_property->encode_rc);
    h264_encode_control->i_curr_lamda = Lambda_lookup_table[h264_encode_control->i_curr_qp];
    h264_encode_control->op->update_image_resolution(h264_encode_control);

    count = h264_encode_control->i_crossbus_fps;
    h264_encode_control->i_frame_per_second = 0;
    h264_encode_control->i_osd_submit_frame_position = -1;
    h264_encode_control->i_software_discard_frame_table = 0;
    h264_encode_control->op->update_fps_discard_table(h264_encode_control, h264_encode_control->i_crossbus_fps, encode_property->op->get_target_fps(encode_property));

    h264_encode_control->sps = h264_encode_control->sps_array;
    h264_encode_control->pps = h264_encode_control->pps_array;
    h264_encode_control->slice_head = h264_encode_control->slice_head_array;
    h264_encode_control->op->set_sps(h264_encode_control);
    h264_encode_control->op->set_pps(h264_encode_control);
    h264_encode_control->op->set_slice_head(h264_encode_control);

    h264_encode_control->overflow = 0;
    h264_encode_control->vlc_status = 0;
    h264_encode_control->vlcIntrLen = 0;

    delta->max_delta = 0;
    delta->min_delta = 0;
    memset(&delta->curr_time, 0, sizeof(struct timeval));
    memset(&delta->last_time, 0, sizeof(struct timeval));
}

static void isil_h264_encode_control_reset(isil_h264_encode_control_t *h264_encode_control)
{
    isil_h264_logic_encode_chan_t   *h264_logic_encode_chan;
    isil_h264_encode_property_t     *encode_property;
    isil_h264_encode_delta_t        *delta;
    int  count;
    isil_h264_phy_video_slot_t      *phy_video_slot;
    isil_h264_logic_video_slot_t    *logic_video_slot;

    h264_logic_encode_chan = to_get_isil_h264_encode_chan_with_encode_control(h264_encode_control);
    ISIL_DBG(ISIL_DBG_DEBUG, "reset channel %d\n", h264_logic_encode_chan->logic_chan_id);
    atomic_set(&h264_logic_encode_chan->opened_flag, 0);
    encode_property = &h264_logic_encode_chan->encode_property;
    delta = &h264_encode_control->encode_delta;
    spin_lock_init(&h264_encode_control->slice_head_and_ad_status_lock);
    atomic_set(&h264_encode_control->slice_head_and_ad_status, 0);
    phy_video_slot   = h264_logic_encode_chan->master_slot;
    logic_video_slot = phy_video_slot->cross_bus_logic_video_slot;

    h264_encode_control->frame_type = H264_FRAME_TYPE_IDR;
    h264_encode_control->op->force_I_frame(h264_encode_control);
    h264_encode_control->i_frameNumber = 0;
    h264_encode_control->i_frame_serial = 0;
    h264_encode_control->i_gop_stride = 0;
    h264_encode_control->i_slice_frame_num = 0;
    h264_encode_control->i_crossbus_fps = logic_video_slot->op->get_fps(logic_video_slot);
    h264_encode_control->i_p_stride = 0;
    h264_encode_control->i_statistic_minuter_counter = 0;
    h264_encode_control->last_jiffies = 0;
    h264_encode_control->i_curr_fps   = 0;
    h264_encode_control->i_old_fps    = 0;
    h264_encode_control->i_fpm = 0;
    h264_encode_control->i_fps = 0;
    h264_encode_control->ad_ready = 0;
    h264_encode_control->i_last_idr = -encode_property->op->get_keyFrameIntervals(encode_property);
    h264_encode_control->i_idr_pic_id = 0;
    h264_encode_control->i_curr_qp = encode_property->encode_rc.op->get_qpi(&encode_property->encode_rc);
    h264_encode_control->i_curr_lamda = Lambda_lookup_table[h264_encode_control->i_curr_qp];

    count = h264_encode_control->i_crossbus_fps;
    h264_encode_control->i_frame_per_second = 0;
    h264_encode_control->i_osd_submit_frame_position = -1;
    h264_encode_control->i_software_discard_frame_table = 0;
    h264_encode_control->op->update_fps_discard_table(h264_encode_control, h264_encode_control->i_crossbus_fps, encode_property->op->get_target_fps(encode_property));
    h264_encode_control->op->update_image_resolution(h264_encode_control);
    h264_encode_control->overflow = 0;
    h264_encode_control->vlc_status = 0;
    h264_encode_control->vlcIntrLen = 0;
    delta->max_delta = 0;
    delta->min_delta = 0;
    memset(&delta->curr_time, 0, sizeof(struct timeval));
    memset(&delta->last_time, 0, sizeof(struct timeval));
}

#ifdef  MV_MODULE
static int  mvflag_response_ddr_burst_done_isr(int irq, void *context)
{
    isil_h264_logic_encode_chan_t   *logic_encode_chan;
    dpram_control_t     *chip_dpram_controller;
    dpram_page_node_t   *dpram_page;
    void    *pio_addr;
    ddr_burst_interface_t   *burst_interface;
    isil_video_mv_frame_tcb_queue_t *video_mv_frame_queue;
    isil_video_mv_frame_tcb_t       *mv_frame;
    int i;

    printk("mvflag ddr burst done.\n");

    logic_encode_chan = (isil_h264_logic_encode_chan_t *)context;
    chip_dpram_controller = &logic_encode_chan->chip->chip_dpram_controller;
    burst_interface = &chip_dpram_controller->burst_interface;
    dpram_page = logic_encode_chan->dpram_page;

    burst_interface->op->clear_burst_done(burst_interface);

    video_mv_frame_queue = &logic_encode_chan->encode_mv_frame_queue;
    video_mv_frame_queue->op->try_get_curr_producer_from_pool(video_mv_frame_queue, 
            &logic_encode_chan->encode_chan_buf_pool);
    mv_frame = video_mv_frame_queue->curr_producer;
    if (mv_frame == NULL)
        printk("can't get mv frame\n");
    else {
        pio_addr = mv_frame->mvFlag_buf;		
        burst_interface->op->pio_host_to_sram_read(burst_interface, dpram_page, 
                pio_addr, MotionVectorBitFlag_BUF_LEN, 0);
        mv_frame->mvFlag_len = MotionVectorBitFlag_BUF_LEN;
    }

    chip_free_irq(logic_encode_chan->chip, IRQ_BURST_TYPE_INTR, logic_encode_chan);
    chip_dpram_controller = &logic_encode_chan->chip->chip_dpram_controller;
    chip_dpram_controller->op->ack_read_mvflag_req(chip_dpram_controller, logic_encode_chan);

    complete_all(&logic_encode_chan->mvflag_transfer_done);

    return IRQ_HANDLED;
}

static int  mvflag_start_read_bitstream(dpram_request_t *dpram_req, void *context)
{
    int ret=1;
    isil_h264_logic_encode_chan_t   *logic_encode_chan;
    isil_h264_encode_control_t      *encode_control;
    dpram_page_node_t               *dpram_page;
    dpram_control_t                 *chip_dpram_controller;
    ddr_burst_interface_t           *burst_interface;
    int page_id;
    int frame_ptr;

#define MVFLAG_DDR_PAGE_ID 73

    //printk("mvflag_start_read_bitstream.\n");

    if((dpram_req!=NULL) && (context!=NULL)){
        logic_encode_chan = (isil_h264_logic_encode_chan_t *)context;
        encode_control = &logic_encode_chan->encode_control;

        /*burst*/
        chip_request_irq(logic_encode_chan->chip, IRQ_BURST_TYPE_INTR, 
                mvflag_response_ddr_burst_done_isr, "BURST", logic_encode_chan);

        chip_dpram_controller = &logic_encode_chan->chip->chip_dpram_controller;
        if(!chip_dpram_controller->op->is_can_submit_move_data_from_ddr_to_dpram_service_req(chip_dpram_controller, &logic_encode_chan->dpram_page)){
            printk("%s,%d:no dpram_page!\n", __FILE__, __LINE__);
            return -ENOMEM;
        }
        burst_interface = &chip_dpram_controller->burst_interface;
        dpram_page = logic_encode_chan->dpram_page;
        page_id = MVFLAG_DDR_PAGE_ID; 
        frame_ptr = encode_control->i_vd_reference_ptr; 

        burst_interface->op->start_nonblock_transfer_from_ddr_to_sram(burst_interface,
                dpram_page, (page_id << 19) + MotionVectorBitFlag_BUF_LEN*frame_ptr,
                page_id, MotionVectorBitFlag_BUF_LEN, DDR_CHIP_B);
    } 
    else {
        ISIL_DBG(ISIL_DBG_ERR, "null pointer\n");
    }

    return ret;
}

static void init_submit_recv_mvflag_done_service(dpram_request_t *dpram_req, isil_h264_logic_encode_chan_t *h264_logic_encode_chan)
{
    if((dpram_req!=NULL) || (h264_logic_encode_chan!=NULL)){
        dpram_req->chip = h264_logic_encode_chan->chip;
        dpram_req->type = DPRAM_NONBLOCK_TRANSFER_REQUEST;
        dpram_req->context = (void*)h264_logic_encode_chan;
        dpram_req->req_callback = mvflag_start_read_bitstream;
    }
}
#endif

static void isil_h264_encode_control_through_pci_start_encode(isil_h264_encode_control_t *h264_encode_control)
{
    if((h264_encode_control!=NULL)){
        isil_h264_logic_encode_chan_t   *h264_logic_encode_chan;
        ed_tcb_t                        *opened_logic_chan_ed;
        isil_h264_encode_property_t     *encode_property;
        isil_h264_encode_feature_t      *encode_feature;
        isil_h264_phy_video_slot_t      *master_slot;
        isil_chip_t                     *chip;
        chip_driver_t                   *chip_driver;
        isil_video_bus_t                *video_bus;
        isil_video_frame_tcb_queue_t    *video_frame_queue;
        isil_video_frame_tcb_t          *frame;
		isil_video_chan_buf_pool_t      *encode_chan_buf_pool;
        chip_encode_chan_service_queue_t    *chip_encode_service_queue;
        int             video_mode = 0;
        isil_ed_fsm_t   *ed_fsm;
        u32             value;
#ifdef  MV_MODULE
        dpram_control_t *chip_dpram_controller;
        dpram_request_t *read_mvflag_req;
#endif

        h264_logic_encode_chan = to_get_isil_h264_encode_chan_with_encode_control(h264_encode_control);
        opened_logic_chan_ed = &h264_logic_encode_chan->opened_logic_chan_ed;
        driver_gen_deliver_event(opened_logic_chan_ed, 1);  //standby change to running

        ed_fsm = &opened_logic_chan_ed->ed_fsm;
        encode_property = &h264_logic_encode_chan->encode_property;
        encode_feature  = &encode_property->encode_feature;
        master_slot = h264_logic_encode_chan->master_slot;
        chip = h264_logic_encode_chan->chip;
        chip_driver = chip->chip_driver;
        video_bus = &chip_driver->video_bus;
        video_frame_queue = &h264_logic_encode_chan->encode_frame_queue;
        encode_chan_buf_pool = &h264_logic_encode_chan->encode_chan_buf_pool;

        h264_encode_control->op->flush_curr_producer_frame(h264_encode_control);
        frame = video_frame_queue->curr_producer;
        if(frame == NULL){
state_err_escape:
            chip_encode_service_queue = &chip->chip_encode_service_queue;
            chip_encode_service_queue->op->release_curr_consumer(chip_encode_service_queue);
            chip_encode_service_queue->op->trigger_chip_pending_service_request(chip_encode_service_queue);
            video_frame_queue->op->release_curr_producer(video_frame_queue, encode_chan_buf_pool);
            if(master_slot != NULL){
                master_slot->op->slot_software_discard_frame(master_slot, chip, h264_logic_encode_chan);
            }
            h264_logic_encode_chan->rc_driver.rc_op->jump_rc(h264_logic_encode_chan);
            h264_encode_control->op->update_at_encode_frame_err(h264_encode_control);
            h264_encode_control->op->force_I_frame(h264_encode_control);
            ed_fsm->op->change_state(ed_fsm, ISIL_ED_DONE);
            if(have_any_pending_event(opened_logic_chan_ed)){
                driver_trigger_pending_event(opened_logic_chan_ed);
            } else {
                driver_gen_deliver_event(opened_logic_chan_ed, 1);
            }
        } else {
            if(ed_fsm->op->get_curr_state(ed_fsm) == ISIL_ED_RUNNING){
                master_slot->op->set_slot_encode_ptr(master_slot, chip, h264_logic_encode_chan);
                frame->timestamp = master_slot->op->get_timestamp(master_slot);
                frame->duration = 40;

                if(((h264_logic_encode_chan->push_buf = get_push_buf(BUF_TYPE_H264, 0, chip)) != NULL)
                        && ((h264_logic_encode_chan->mv_push_buf = get_push_buf(BUF_TYPE_MV, 0, chip)) != NULL)){
                    h264_nal_t  *nal = &frame->nal;
                    int mode_select, mbdelay;
                    u32 bitalign_32;

                    mode_select = chip->io_op->chip_read32(chip, CIF_MAP_D1_MODE);
                    switch(master_slot->op->get_video_size(master_slot)){
                        case ISIL_VIDEO_SIZE_CIF:
                            if(chip->video_encode_cap->ddr_map_mode == DDR_MAP_COMPRESS_DISABLE){
                                mode_select |= 0x40;
                            } else {
                                mode_select &= ~0x40;
                            }
                            video_mode = 0x00;
                            break;
                        case ISIL_VIDEO_SIZE_2CIF:
                        case ISIL_VIDEO_SIZE_HALF_D1:
                            if(chip->video_encode_cap->ddr_map_mode == DDR_MAP_COMPRESS_DISABLE){
                                mode_select |= 0x80;
                            } else {
                                mode_select &= ~0x80;
                            }
                            video_mode = 0x03;
                            break;
                        default:
                        case ISIL_VIDEO_SIZE_4CIF:
                        case ISIL_VIDEO_SIZE_D1:
                            mode_select |= 0xc0;
                            video_mode = 0x01;
                            break;
                    }
                    chip->io_op->chip_write32(chip, CIF_MAP_D1_MODE, mode_select);
                    chip->io_op->chip_write32(chip, FRAMERESOLUTION, ((frame->i_mb_x<<8)|frame->i_mb_y));
                    chip->io_op->chip_write32(chip, REFFRAMEINDEX, ((h264_encode_control->i_vd_reference_ptr<<12)|((h264_encode_control->i_vd_reference_ptr+0x3)&0x3)));
                    chip->io_op->chip_write32(chip, MBADDR, h264_encode_control->i_first_mb_x_and_y);

                    if(frame->frame_len % 4){
                        bitalign_32 = nal->i_temp_bitalign_number + 16;
                    } else {
                        bitalign_32 = nal->i_temp_bitalign_number;
                    }
                    chip->io_op->chip_write32(chip, ENCODER_VLC_PARAM, ((bitalign_32&0x1f)<<8) | h264_encode_control->i_curr_qp | ENABLE_VLC_PCI_SEL);

                    chip->io_op->chip_write32(chip, QP, frame->i_curr_qp);
                    h264_encode_control->i_curr_lamda = Lambda_lookup_table[frame->i_curr_qp];
                    chip->io_op->chip_write32(chip, LAMDA, h264_encode_control->i_curr_lamda);
                    chip->io_op->chip_write32(chip, INTERSTART, frame->sen_type);


                    chip->io_op->chip_write32(chip, 0x0008, 0x0000);
                    chip->io_op->chip_write32(chip, MODE, chip->video_encode_cap->encode_reg_4 | (video_mode << 6));
                    chip->io_op->chip_write32(chip, 0x0008, 0x0800);

                    switch (frame->frame_type) {
                        case H264_FRAME_TYPE_P:
                            mode_select = 0x8c;
                            if(encode_feature->op->get_skip(encode_feature)) {
                                mode_select |= 0x30;
                            }else{
                                mode_select |= 0x40;
                            }
                            if(encode_feature->op->get_half_pixel(encode_feature)){
                                mode_select |= 0x02;
                            }else{
                                mode_select &= 0xfd;
                            }
                            if(encode_feature->op->get_quarter_pixel(encode_feature)) {
                                mode_select |= 0x03;
                            }else{
                                mode_select &= 0xfc;
                            }
                            break;
                        default:
                        case H264_FRAME_TYPE_IDR:
                        case H264_FRAME_TYPE_I:
                            mode_select = 0x8;
                            break;
                    }
                    if(encode_feature->op->get_i_4x4(encode_feature)) {
                        chip->io_op->chip_write32(chip, MB4X4LAMDA, Intra4X4_Lambda3[h264_encode_control->i_curr_qp]);
                        chip->io_op->chip_write32(chip, INTRAENABLE, 0x0070);
                    }else{
                        chip->io_op->chip_write32(chip, INTRAENABLE, 0x0060);                            
                    }
 
                    mbdelay = encode_feature->op->get_mb_delay(encode_feature);
                    if(frame->need_large_mbdelay){
                        mbdelay += 1024;
                    }
                    if(mbdelay <= 240) {
                        mbdelay >>= 4;
                        mbdelay <<= 8;
                        mbdelay |= (0 << 4); //DSP_MB_WAIT = 0
                    }else{
                        mbdelay >>= 7;
                        mbdelay <<= 8;
                        mbdelay |= (1 << 4); //DSP_MB_WAIT = 1
                    }
                    chip->io_op->chip_write32(chip, CHANNEL_ID, (master_slot->phy_slot_id| mbdelay | DDR_B_CHIP));
                    chip->io_op->chip_write32(chip, INTRANDMECONTROL, mode_select);

                    h264_logic_encode_chan->h264_push_dma_addr = dma_map_single(NULL, h264_logic_encode_chan->push_buf, H264_BUF_SIZE, DMA_FROM_DEVICE);
                    h264_logic_encode_chan->mv_push_dma_addr = dma_map_single(NULL, h264_logic_encode_chan->mv_push_buf, MV_BUF_SIZE, DMA_FROM_DEVICE);
                    dma_sync_single_for_device(NULL, h264_logic_encode_chan->h264_push_dma_addr, H264_BUF_SIZE, DMA_FROM_DEVICE);
                    chip->io_op->chip_write32(chip, PCI_VLC_BASE_ADDR, h264_logic_encode_chan->h264_push_dma_addr);
                    chip->io_op->chip_write32(chip, PCI_MV_BASE_ADDR, h264_logic_encode_chan->mv_push_dma_addr);

                    value = chip->io_op->chip_read32(chip, PCI_MASTER_CONTROL);
#ifdef  MV_MODULE
                    value |=  (1 << IRQ_EXT_PCI_MASTER) | (1 << IRQ_EXT_VLC_INTR) | (1 << IRQ_EXT_VLC_MVD);
#else
                    value |=  (1 << IRQ_EXT_PCI_MASTER) | (1 << IRQ_EXT_VLC_INTR);
#endif
                    chip->io_op->chip_write32(chip, PCI_MASTER_CONTROL, value);
                    value = chip->io_op->chip_read32(chip, PCI_MASTER_INTR_ENABLE_REG);
                    value |=  1 << IRQ_EXT_VLC_INTR;
                    chip->io_op->chip_write32(chip, PCI_MASTER_INTR_ENABLE_REG, value);

#ifdef MV_MODULE
                    value = chip->io_op->chip_read32(chip, ENCODER_VLC_PARAM);
                    value |=  ENABLE_VLC_MVD;
                    chip->io_op->chip_write32(chip, ENCODER_VLC_PARAM, value);

                    init_completion(&h264_logic_encode_chan->mvflag_transfer_done);
                    chip_dpram_controller = &chip->chip_dpram_controller;
                    read_mvflag_req = &h264_logic_encode_chan->read_mvflag_req;
                    init_submit_recv_mvflag_done_service(read_mvflag_req, h264_logic_encode_chan);
                    chip_dpram_controller->op->submit_read_mvflag_req(chip_dpram_controller, h264_logic_encode_chan);
#endif

                    {
                        osd_chan_engine_t   *encode_osd_engine = &h264_logic_encode_chan->encode_osd_engine;
                        encode_osd_engine->op->submit_osd_data(encode_osd_engine);
                        encode_osd_engine->op->submit_osd_attribute(encode_osd_engine);
                    }

                    if(ed_fsm->op->get_curr_state(ed_fsm) == ISIL_ED_RUNNING){
                        isil_h264_encode_delta_t    *delta;

                        chip->register_curr_h264_encode_push_chan(chip, h264_logic_encode_chan);
                        chip->io_op->chip_write32(chip, CHIPSCOPE, 0x8000);
                        chip->io_op->chip_write32(chip, CHIPSCOPE, 0x0000);
                        delta = &h264_encode_control->encode_delta;
                        do_gettimeofday(&delta->last_time);
                    } else {
                        if(h264_logic_encode_chan->push_buf) {
                            release_push_buf(BUF_TYPE_H264, 0, h264_logic_encode_chan->push_buf, chip);
                            h264_logic_encode_chan->push_buf = NULL;
                        }
                        if(h264_logic_encode_chan->mv_push_buf) {
                            release_push_buf(BUF_TYPE_MV, 0, h264_logic_encode_chan->mv_push_buf, chip);
                            h264_logic_encode_chan->mv_push_buf = NULL;
                        }
                        printk("\n%s.%d, state is not running, restart encode frame. %d, %d--%d", __FUNCTION__, __LINE__, h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id, ed_fsm->op->get_curr_state(ed_fsm));
                        goto state_err_escape;
                    }
                } else {
                    printk("\n%s.%d, can't get push buf, restart encode frame. %d, %d--%d", __FUNCTION__, __LINE__, h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id, ed_fsm->op->get_curr_state(ed_fsm));
                    goto state_err_escape;
                }
            } else {
                printk("\n%s.%d, state is not running, why?????, %d, %d--%d\n", __FUNCTION__, __LINE__, h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id, ed_fsm->op->get_curr_state(ed_fsm));
                goto state_err_escape;
            }
        }
    }
}

#ifdef  USE_CRC_CHECK 
static u32 crc_check_sum(u32 *data, int len){
    u32 val, count_len=len;

    val = *data++;
    while(((count_len>>2) - 1) > 0) {
        val ^= *data++;
        count_len -= 4;
    }
#if defined(PLATFORM_ENDIAN_SAME)
    val ^= SWAP32((len >> 2));
#else
    val ^= (len >> 2);
#endif
    return val;
}
#endif

static int isil_h264_encode_control_through_pci_irq_func(int irq, void *context)
{
    isil_h264_encode_control_t      *h264_encode_control = (isil_h264_encode_control_t*)context;
    isil_h264_logic_encode_chan_t   *h264_logic_encode_chan;
    isil_chip_t                     *chip;
    isil_video_frame_tcb_queue_t    *video_frame_queue;
    isil_video_frame_tcb_t          *frame;
    h264_nal_t      *nal;
    unsigned int    head_value;
    isil_video_packet_tcb_queue_t   *video_packet_queue;
    isil_video_chan_buf_pool_t      *video_chan_buf_pool;
    isil_video_packet_tcb_t         *packet;
    u8  *head_buf_ptr;
    u8  *push_buf;
    u32 encode_len;
#ifdef  MV_MODULE
    isil_video_mv_frame_tcb_queue_t     *video_mv_frame_queue;
    isil_video_mv_frame_tcb_t           *mv_frame;
    isil_video_mv_packet_tcb_queue_t    *video_mv_packet_queue;
    isil_video_mv_packet_tcb_t          *mv_packet;
    int mv_len;
    int push_data_pos;
#endif
    u32 first_encode;
    u32 bitalign_32;
#ifdef  USE_CRC_CHECK
    u32 vlc_crc, soft_crc, vlc_intra_crc_i, vlc_intra_crc_o;
#endif
    int i;
    isil_h264_encode_delta_t    *delta;
    u32 val;
    ed_tcb_t                    *opened_logic_chan_ed;
    unsigned long   flags;

    delta = &h264_encode_control->encode_delta;
    do_gettimeofday(&delta->curr_time);
    val = timeval_to_ns(&delta->curr_time) - timeval_to_ns(&delta->last_time);
    val = val / 1000;
    if(delta->max_delta == 0) {
        delta->max_delta = val;
    }
    if(delta->min_delta == 0) {
        delta->min_delta = val;
    }
    if(delta->max_delta < val) {
        delta->max_delta = val;
    }
    if(delta->min_delta > val) {
        delta->min_delta = val;
    }
    delta->cur_delta = val;

    h264_logic_encode_chan = to_get_isil_h264_encode_chan_with_encode_control(h264_encode_control);
    spin_lock_irqsave(&h264_logic_encode_chan->fsm_matrix_call_lock, flags);
    opened_logic_chan_ed = &h264_logic_encode_chan->opened_logic_chan_ed;
    driver_gen_deliver_event(opened_logic_chan_ed, 1);      //change state to transfering if state correct.
    chip = h264_logic_encode_chan->chip;
#if defined(PCI_MASTER_OLD)
    chip->io_op->chip_write32(chip, PCI_MASTER_REG_SWITCH, 0x4);
    encode_len = chip->io_op->chip_read32(chip, PCI_VLC_LEN);
    encode_len <<= 2;
#ifdef MV_MODULE
    mv_len = chip->io_op->chip_read32(chip, PCI_VLC_MV_LEN);
    mv_len <<= 2;
#endif
#ifdef  USE_CRC_CHECK
    vlc_crc = chip->io_op->chip_read32(chip, PCI_VLC_CRC);
    vlc_intra_crc_i = chip->io_op->chip_read32(chip, PCI_VLC_INTRA_CRC_I);
    vlc_intra_crc_o = chip->io_op->chip_read32(chip, PCI_VLC_INTRA_CRC_O);
#endif
    chip->io_op->chip_write32(chip, PCI_MASTER_REG_SWITCH, 0x0);
#else
    encode_len = chip->io_op->chip_read32(chip, PCI_VLC_LEN);
    encode_len <<= 2;
#ifdef MV_MODULE
    mv_len = chip->io_op->chip_read32(chip, PCI_VLC_MV_LEN);
    mv_len <<= 2;
#endif
#ifdef  USE_CRC_CHECK
    vlc_crc = chip->io_op->chip_read32(chip, PCI_VLC_CRC);
    vlc_intra_crc_i = chip->io_op->chip_read32(chip, PCI_VLC_INTRA_CRC_I);
    vlc_intra_crc_o = chip->io_op->chip_read32(chip, PCI_VLC_INTRA_CRC_O);
#endif
#endif

    chip->io_op->chip_write32(chip, VLC_ENCODER_INTERRUPT, 1<<IRQ_VLC_TYPE_INTR);
    chip->io_op->chip_write32(chip, INTERRUPT_FLAGS_EXT, 1 << IRQ_EXT_VLC_INTR);
    h264_encode_control->op->notify(h264_encode_control, h264_logic_encode_chan, chip);
    h264_encode_control->op->flush_curr_producer_frame(h264_encode_control);
    video_frame_queue = &h264_logic_encode_chan->encode_frame_queue;
    video_chan_buf_pool = &h264_logic_encode_chan->encode_chan_buf_pool;
    frame = video_frame_queue->curr_producer;
    if(frame){
        video_packet_queue = &frame->video_packet_queue;
        packet = video_packet_queue->curr_producer;
        head_buf_ptr = packet->data;
        nal = &frame->nal;

        if((encode_len > (H264_BUF_SIZE - 64)) || (encode_len == 0)) {
#ifdef  USE_CRC_CHECK
            printk("%s, %d: ----->vlc stream overflow!, len = 0x%08x, crc = 0x%08x<---------\n", __FUNCTION__, __LINE__, encode_len, vlc_crc);
#else
            printk("%s, %d: ----->vlc stream overflow!, len = 0x%08x<---------\n", __FUNCTION__, __LINE__, encode_len);
#endif
            video_frame_queue->op->release_curr_producer(video_frame_queue, video_chan_buf_pool);
            h264_encode_control->op->force_I_frame(h264_encode_control);
            if (h264_logic_encode_chan->push_buf) {
                release_push_buf(BUF_TYPE_H264, 0, h264_logic_encode_chan->push_buf, chip);
                h264_logic_encode_chan->push_buf = NULL;
            }
            if (h264_logic_encode_chan->mv_push_buf) {
                release_push_buf(BUF_TYPE_MV, 0, h264_logic_encode_chan->mv_push_buf, chip);
                h264_logic_encode_chan->mv_push_buf = NULL;
            }
            driver_gen_deliver_event(opened_logic_chan_ed, 1);      //change state to done if state correct.
            spin_unlock_irqrestore(&h264_logic_encode_chan->fsm_matrix_call_lock, flags);        
            return 0;
        }

        push_buf = h264_logic_encode_chan->push_buf;
        dma_sync_single_for_cpu(NULL, h264_logic_encode_chan->h264_push_dma_addr, H264_BUF_SIZE, DMA_FROM_DEVICE);
#ifdef  USE_CRC_CHECK
        soft_crc = crc_check_sum((u32 *)push_buf, encode_len);
#if defined(PLATFORM_ENDIAN_SAME)
        if(vlc_crc != soft_crc){
            printk("%s, %d: PCI transfer error, 0x%08x, 0x%08x, encode_len = 0x%08x, intra_crc_i = 0x%08x, intra_crc_o = 0x%08x\n", __FUNCTION__, __LINE__, vlc_crc, soft_crc, encode_len, vlc_intra_crc_i, vlc_intra_crc_o);
            frame->frame_is_err = 1;
        }
#else
        if(vlc_crc != SWAP32(soft_crc)){
            printk("%s, %d: PCI transfer error, 0x%08x, 0x%08x, encode_len = 0x%08x, intra_crc_i = 0x%08x, intra_crc_o = 0x%08x\n", __FUNCTION__, __LINE__, vlc_crc, SWAP32(soft_crc), encode_len, vlc_intra_crc_i, vlc_intra_crc_o);
            frame->frame_is_err = 1;
        }
#endif
        if(vlc_crc != vlc_intra_crc_i){
            printk("%s, %d: vlc vlc_intra_crc_i error, 0x%08x, 0x%08x\n", __FUNCTION__, __LINE__, vlc_crc, vlc_intra_crc_i);
            frame->frame_is_err = 1;
        }
        if(vlc_crc != vlc_intra_crc_o){
            printk("%s, %d: vlc vlc_intra_crc_o error, 0x%08x, 0x%08x\n", __FUNCTION__, __LINE__, vlc_crc, vlc_intra_crc_o);
            frame->frame_is_err = 1;
        }
        if(vlc_intra_crc_i != vlc_intra_crc_o){
            printk("%s, %d: vlc_intra_crc_i != vlc_intra_crc_o error, 0x%08x, 0x%08x\n", __FUNCTION__, __LINE__, vlc_intra_crc_i, vlc_intra_crc_o);
            frame->frame_is_err = 1;
        }
#endif

        head_value = 0;
        if(frame->frame_len % 4){
            frame->frame_len -= 2;
            head_value = head_buf_ptr[frame->frame_len];
            head_value <<= 8;
            head_value |= head_buf_ptr[frame->frame_len+1];
            head_value <<= 16;
            if(nal->i_temp_bitalign_number) {
                head_value |= nal->i_temp_bitalign_content;
            }
            bitalign_32 = nal->i_temp_bitalign_number + 16;
        } else {
            if(nal->i_temp_bitalign_number) {
                head_value = nal->i_temp_bitalign_content << 16;
            }
            bitalign_32 = nal->i_temp_bitalign_number;
        }

        first_encode = 0;
        for (i=0; i<4; i++){
            first_encode <<= 8;
            first_encode |= push_buf[i];
        }
        first_encode <<= bitalign_32;
        first_encode >>= bitalign_32;
        first_encode |= head_value;

#if defined(POWERPC_PLATFORM)
        *(u32 *)push_buf = first_encode;
#else
        *(u32 *)push_buf = SWAP32(first_encode);
#endif//POWERPC_PLATFORM

        packet->payload_len = frame->frame_len;
        if(frame->frame_is_err == 0){
            u8  *src_ptr, *dest_ptr;
            int transfer_state, dest_count;
            transfer_state = 0;
            src_ptr = h264_logic_encode_chan->push_buf;
            while(encode_len){
                dest_ptr = &packet->data[packet->payload_len];
                dest_count = packet->payload_len;

                while((dest_count<PAGE_SIZE) && (encode_len)){
                    if (transfer_state == 2 && (src_ptr[0]<=0x03)) {
                        *dest_ptr++ = 0x03;
                        dest_count++;
                        packet->payload_len++;
                        frame->frame_len++;
                        transfer_state = 0;
                        if(dest_count >= PAGE_SIZE){
                            break;
                        }
                    }
                    if(src_ptr[0] == 0) {
                        if(transfer_state < 2) {
                            transfer_state++;
                        }
                    } else {
                        transfer_state = 0;
                    }
                    *dest_ptr++ = *src_ptr++;
                    packet->payload_len++;
                    frame->frame_len++;
                    dest_count++;
                    encode_len--;
                }

                if(encode_len){
                    video_packet_queue->op->put_curr_producer_into_queue(video_packet_queue);
                    video_packet_queue->op->try_get_curr_producer_from_pool(video_packet_queue, &h264_logic_encode_chan->encode_chan_buf_pool);
                    packet = video_packet_queue->curr_producer;
                    if (packet == NULL) {
                        frame->frame_is_err = 1;
                        break;
                    }
                } else {
                    video_packet_queue->op->put_curr_producer_into_queue(video_packet_queue);
                }
            }
            if(frame->frame_is_err || h264_encode_control->b_flush_all_frame){
                video_frame_queue->op->release_curr_producer(video_frame_queue, video_chan_buf_pool);
                h264_encode_control->op->force_I_frame(h264_encode_control);
            } else {
                video_frame_queue->op->put_curr_producer_into_queue(video_frame_queue);
            }
        } else {
            video_frame_queue->op->release_curr_producer(video_frame_queue, video_chan_buf_pool);
            h264_encode_control->op->force_I_frame(h264_encode_control);
        }
        dma_sync_single_for_device(NULL, h264_logic_encode_chan->h264_push_dma_addr, H264_BUF_SIZE, DMA_FROM_DEVICE);
        dma_unmap_single(NULL, h264_logic_encode_chan->h264_push_dma_addr, H264_BUF_SIZE, DMA_FROM_DEVICE);

#ifdef MV_MODULE
        video_mv_frame_queue = &h264_logic_encode_chan->encode_mv_frame_queue;
        video_mv_frame_queue->op->try_get_curr_producer_from_pool(video_mv_frame_queue, &h264_logic_encode_chan->encode_chan_buf_pool);
        mv_frame = video_mv_frame_queue->curr_producer;
        dma_sync_single_for_cpu(NULL, h264_logic_encode_chan->mv_push_dma_addr, MV_BUF_SIZE, DMA_FROM_DEVICE);
        if (mv_frame != NULL) {
            mv_frame->mvBuf_payLoadLen = 0;
            push_data_pos = 0;

            video_mv_packet_queue = &mv_frame->mv_queue;
            video_mv_packet_queue->op->try_get_curr_producer_from_pool(video_mv_packet_queue,
                    &h264_logic_encode_chan->encode_chan_buf_pool);
            mv_packet = video_mv_packet_queue->curr_producer;
            if (mv_packet == NULL) {
                printk("can't get mv frame packet\n");
                mv_frame->frame_is_err = 1;
            }
            while (mv_len > 0 && !mv_frame->frame_is_err) {
                if (mv_len > VLC_ENCODER_DMA_LEN) {
                    memcpy(&mv_packet->data[0], &h264_logic_encode_chan->mv_push_buf[push_data_pos], VLC_ENCODER_DMA_LEN);
                    mv_packet->data_size = VLC_ENCODER_DMA_LEN;
                    mv_frame->mvBuf_payLoadLen += VLC_ENCODER_DMA_LEN;
                    mv_len -= VLC_ENCODER_DMA_LEN;
                    push_data_pos += VLC_ENCODER_DMA_LEN;
                    video_mv_packet_queue->op->put_curr_producer_into_queue(video_mv_packet_queue);
                    video_mv_packet_queue->op->try_get_curr_producer_from_pool(video_mv_packet_queue, 
                            &h264_logic_encode_chan->encode_chan_buf_pool);
                    mv_packet = video_mv_packet_queue->curr_producer;
                    if (mv_packet == NULL) {
                        printk("can't get mv frame packet\n");
                        mv_frame->frame_is_err = 1;
                    }
                }
                else {
                    memcpy(&mv_packet->data[0], &h264_logic_encode_chan->mv_push_buf[push_data_pos], mv_len);
                    mv_packet->data_size = mv_len;
                    mv_frame->mvBuf_payLoadLen += mv_len;
                    mv_len = 0;
                    video_mv_packet_queue->op->put_curr_producer_into_queue(video_mv_packet_queue);
                }
            }
        }
        dma_sync_single_for_device(NULL, h264_logic_encode_chan->mv_push_dma_addr, MV_BUF_SIZE, DMA_FROM_DEVICE);
#endif
        dma_unmap_single(NULL, h264_logic_encode_chan->mv_push_dma_addr, MV_BUF_SIZE, DMA_FROM_DEVICE);
#ifdef MV_MODULE
        if(mv_frame != NULL){
            if(mv_frame->frame_is_err)
                video_mv_frame_queue->op->release_curr_producer(video_mv_frame_queue, &h264_logic_encode_chan->encode_chan_buf_pool);
            else
                video_mv_frame_queue->op->put_curr_producer_into_queue(video_mv_frame_queue);
        }
#endif
    } else {
        h264_encode_control->op->force_I_frame(h264_encode_control);
    }

    if (h264_logic_encode_chan->push_buf) {
        release_push_buf(BUF_TYPE_H264, 0, h264_logic_encode_chan->push_buf, chip);
        h264_logic_encode_chan->push_buf = NULL;
    }
    if (h264_logic_encode_chan->mv_push_buf) {
        release_push_buf(BUF_TYPE_MV, 0, h264_logic_encode_chan->mv_push_buf, chip);
        h264_logic_encode_chan->mv_push_buf = NULL;
    }

    driver_gen_deliver_event(opened_logic_chan_ed, 1);
    spin_unlock_irqrestore(&h264_logic_encode_chan->fsm_matrix_call_lock, flags);        
    return 1;
}

static void isil_h264_encode_control_through_pci_read_vlc_param(isil_h264_encode_control_t *h264_encode_control, isil_chip_t *chip)
{
}

static void isil_h264_encode_control_through_pci_calculate_mov_param(isil_h264_encode_control_t *h264_encode_control, isil_chip_t *chip)
{
}

static void isil_h264_encode_control_through_pci_mov_vlc_coding(isil_h264_encode_control_t *h264_encode_control, isil_h264_logic_encode_chan_t *h264_logic_encode_chan, isil_chip_t *chip)
{
}

static int  isil_h264_encode_control_through_pci_chip_ping_pong_update(isil_h264_encode_control_t *h264_encode_control, isil_h264_logic_encode_chan_t *h264_logic_encode_chan, isil_chip_t *chip)
{
    return 0;
}

static int isil_h264_encode_control_through_pci_notify(isil_h264_encode_control_t *h264_encode_control, isil_h264_logic_encode_chan_t *h264_logic_encode_chan, isil_chip_t *chip)
{
    isil_video_frame_tcb_queue_t        *encode_frame_queue;
    isil_video_frame_tcb_t              *frame;
    chip_encode_chan_service_queue_t    *chip_encode_service_queue;

    h264_encode_control->vlc_status = 0;
    h264_logic_encode_chan->rc_driver.rc_op->update_rc(h264_logic_encode_chan);
    if(h264_logic_encode_chan->master_slot != NULL){
        h264_logic_encode_chan->master_slot->op->encode_endofslice_notify(h264_logic_encode_chan->master_slot, h264_logic_encode_chan);
    }

    chip->unregister_curr_h264_encode_push_chan(chip, h264_logic_encode_chan);
    encode_frame_queue = &h264_logic_encode_chan->encode_frame_queue;
    frame = encode_frame_queue->curr_producer;
    if(frame != NULL){
        h264_encode_control->i_fpm++;
        h264_encode_control->op->update_at_encode_frame_ok(h264_encode_control);
    } else {
        printk("%d:chan_%d_%d discard\n", __LINE__, h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id);
        h264_logic_encode_chan->rc_driver.rc_op->jump_rc(h264_logic_encode_chan);
        h264_encode_control->op->update_at_encode_frame_err(h264_encode_control);
    }

    chip_encode_service_queue = &chip->chip_encode_service_queue;
    chip_encode_service_queue->op->release_curr_consumer(chip_encode_service_queue);
    chip_encode_service_queue->op->trigger_chip_pending_service_request(chip_encode_service_queue);
    return 1;
}

static int  isil_h264_encode_control_through_pci_notify_timeout(isil_h264_encode_control_t *h264_encode_control, isil_h264_logic_encode_chan_t *h264_logic_encode_chan, isil_chip_t *chip)
{
    isil_video_frame_tcb_queue_t        *encode_frame_queue;
    chip_encode_chan_service_queue_t    *chip_encode_service_queue;

    if(h264_logic_encode_chan->master_slot != NULL){
        h264_logic_encode_chan->master_slot->op->encode_endofslice_notify(h264_logic_encode_chan->master_slot, h264_logic_encode_chan);
    }
    if(h264_logic_encode_chan->push_buf) {
        release_push_buf(BUF_TYPE_H264, 0, h264_logic_encode_chan->push_buf, chip);
        h264_logic_encode_chan->push_buf = NULL;
    }
    if(h264_logic_encode_chan->mv_push_buf) {
        release_push_buf(BUF_TYPE_MV, 0, h264_logic_encode_chan->mv_push_buf, chip);
        h264_logic_encode_chan->mv_push_buf = NULL;
    }
    encode_frame_queue = &h264_logic_encode_chan->encode_frame_queue;
    encode_frame_queue->op->release_curr_producer(encode_frame_queue, &h264_logic_encode_chan->encode_chan_buf_pool);
    h264_encode_control->op->force_I_frame(h264_encode_control);

    chip->unregister_curr_h264_encode_push_chan(chip, h264_logic_encode_chan);
    chip_encode_service_queue = &chip->chip_encode_service_queue;
    chip_encode_service_queue->op->release_curr_consumer(chip_encode_service_queue);
    printk("%s.%d: %d_%d\n", __FUNCTION__, __LINE__, h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id);
    return 1;
}

static struct isil_h264_encode_control_operation isil_h264_encode_control_through_pci_op = {
    .init = isil_h264_encode_control_init,
    .reset = isil_h264_encode_control_reset,

    .set_sps = isil_h264_encode_control_set_sps,
    .set_pps = isil_h264_encode_control_set_pps,
    .set_slice_head = isil_h264_encode_control_set_slice_head,
    .generator_head = isil_h264_encode_control_generator_head,

    .calculate_frame_type = isil_h264_encode_control_calculate_frame_type,
    .get_frame_type = isil_h264_encode_control_get_frame_type,
    .force_I_frame = isil_h264_encode_control_force_I_frame,
    .flush_all_frame = isil_h264_encode_control_flush_all_frame,
    .flush_curr_producer_frame = isil_h264_encode_control_flush_curr_producer_frame,
    .flush_curr_consumer_frame = isil_h264_encode_control_flush_curr_consumer_frame,
    .flush_frame_queue = isil_h264_encode_control_flush_frame_queue,
    .update_at_encode_frame_ok = isil_h264_encode_control_update_at_encode_frame_ok,
    .update_at_discard_frame = isil_h264_encode_control_update_at_discard_frame,
    .update_at_encode_frame_err = isil_h264_encode_control_update_at_encode_frame_err,
    .update_image_resolution = isil_h264_encode_control_update_image_resolution,
    .update_fps_discard_table = isil_h264_encode_control_update_fps_discard_table,

    .get_slice_head_and_ad_status = isil_h264_encode_control_get_slice_head_and_ad_status,
    .clear_slice_head_and_ad_status = isil_h264_encode_control_clear_slice_head_and_ad_status,
    .update_slice_head_ready = isil_h264_encode_control_update_slice_head_ready,
    .get_slice_head_status = isil_h264_encode_control_get_slice_head_status,
    .clear_slice_head_ready = isil_h264_encode_control_clear_slice_head_ready,
    .update_ad_ready = isil_h264_encode_control_update_ad_ready,
    .clear_ad_ready = isil_h264_encode_control_clear_ad_ready,
    .get_ad_status = isil_h264_encode_control_get_ad_status,

    .first_start = isil_h264_encode_control_first_start,
    .start_encode = isil_h264_encode_control_through_pci_start_encode,
    .stop_encode = isil_h264_encode_control_stop_encode,

    .irq_func = isil_h264_encode_control_through_pci_irq_func,
    .read_vlc_param = isil_h264_encode_control_through_pci_read_vlc_param,
    .calculate_mov_param = isil_h264_encode_control_through_pci_calculate_mov_param,
    .mov_vlc_coding = isil_h264_encode_control_through_pci_mov_vlc_coding,
    .chip_ping_pong_update = isil_h264_encode_control_through_pci_chip_ping_pong_update,
    .notify = isil_h264_encode_control_through_pci_notify,
    .notify_timeout = isil_h264_encode_control_through_pci_notify_timeout,
};

static void init_isil_h264_encode_control(isil_h264_encode_control_t *h264_encode_control, int read_bitstream_mode)
{
    if(h264_encode_control != NULL){
        h264_encode_control->op = &isil_h264_encode_control_through_pci_op;
        h264_encode_control->op->init(h264_encode_control);
    }
}

static void remove_isil_h264_encode_control(isil_h264_encode_control_t *h264_encode_control)
{
    if(h264_encode_control != NULL){
        h264_encode_control->op->stop_encode(h264_encode_control);
    }
}

static void h264_logic_encode_chan_first_start(void *context)
{
    isil_h264_logic_encode_chan_t *h264_logic_encode_chan = (isil_h264_logic_encode_chan_t*)context;
    if(h264_logic_encode_chan != NULL){
        h264_logic_encode_chan->encode_control.op->first_start(&h264_logic_encode_chan->encode_control);
        ISIL_DBG(ISIL_DBG_INFO, "%d-%d\n", h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id);
    }
}

static void h264_logic_encode_chan_delete_first_start_req_notify(void *context)
{
    isil_h264_logic_encode_chan_t *h264_logic_encode_chan = (isil_h264_logic_encode_chan_t*)context;
    if(h264_logic_encode_chan != NULL){
        h264_logic_encode_chan->encode_control.op->stop_encode(&h264_logic_encode_chan->encode_control);
        ISIL_DBG(ISIL_DBG_INFO, "%d-%d\n", h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id);
    }
}

static void h264_logic_encode_chan_start_encode(void *context)
{
    isil_h264_logic_encode_chan_t *h264_logic_encode_chan = (isil_h264_logic_encode_chan_t*)context;
    if(h264_logic_encode_chan != NULL){
        h264_logic_encode_chan->encode_control.op->start_encode(&h264_logic_encode_chan->encode_control);
    }
}

static void h264_logic_encode_chan_delete_start_encode_req_notify(void *context)
{
    isil_h264_logic_encode_chan_t *h264_logic_encode_chan = (isil_h264_logic_encode_chan_t*)context;
    if(h264_logic_encode_chan != NULL){
        isil_h264_phy_video_slot_t *master_slot;
        h264_logic_encode_chan->encode_control.op->stop_encode(&h264_logic_encode_chan->encode_control);
        master_slot = h264_logic_encode_chan->master_slot;
        if(master_slot != NULL){
            master_slot->op->slot_software_discard_frame(master_slot, h264_logic_encode_chan->chip, h264_logic_encode_chan);
        }
        ISIL_DBG(ISIL_DBG_INFO, "%d-%d\n", h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id);
    }
}

void    init_encode_chan_service_tcb_with_null(encode_chan_service_tcb_t *encode_request_tcb)
{
    isil_h264_logic_encode_chan_t *h264_logic_encode_chan = to_get_isil_h264_encode_chan_with_encode_request_tcb(encode_request_tcb);
    tcb_node_t	*service_tcb = &encode_request_tcb->service_tcb;

    service_tcb->op = &tcb_node_op;
    service_tcb->op->init(service_tcb);
    service_tcb->op->set_priv(service_tcb, encode_request_tcb);
    encode_request_tcb->context = h264_logic_encode_chan;
    encode_request_tcb->type = DVM_CHIP_REQ_FIRST_STRAT_CHAN;
    encode_request_tcb->req_callback = h264_logic_encode_chan_first_start;
    encode_request_tcb->delete_req_notify = h264_logic_encode_chan_delete_first_start_req_notify;
}

void    init_encode_chan_service_tcb_with_first_start(encode_chan_service_tcb_t *encode_request_tcb)
{
    isil_h264_logic_encode_chan_t       *h264_logic_encode_chan = to_get_isil_h264_encode_chan_with_encode_request_tcb(encode_request_tcb);
    isil_h264_encode_control_t          *h264_encode_control;
    osd_chan_engine_t                   *encode_osd_engine;
    //chip_encode_chan_service_queue_t    *chip_encode_service_queue;
    isil_h264_phy_video_slot_t          *master_slot;

    encode_osd_engine = &h264_logic_encode_chan->encode_osd_engine;
    h264_encode_control = &h264_logic_encode_chan->encode_control;
    if(encode_osd_engine->op != NULL){
        encode_osd_engine->op->reset_chan_engine(encode_osd_engine);
    }
    ISIL_DBG(ISIL_DBG_INFO, "%d-%d\n", h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id);
    master_slot = h264_logic_encode_chan->master_slot;
    if(atomic_read(&master_slot->first_start_flag)){
        /*init_encode_chan_service_tcb_with_null(encode_request_tcb);
        chip_encode_service_queue = &h264_logic_encode_chan->chip->chip_encode_service_queue;
        chip_encode_service_queue->op->put_service_request_into_queue_header(chip_encode_service_queue, encode_request_tcb);*/
        h264_encode_control->op->first_start(h264_encode_control);
    } else {
        ISIL_DBG(ISIL_DBG_INFO, "%d-%d\n", h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id);
        h264_encode_control->i_vd_reference_ptr = master_slot->vd_orig_buf.vd_encode_ptr;
        atomic_set(&h264_logic_encode_chan->first_start_flag, 0);    
    }
}

void    init_encode_chan_service_tcb_with_start_encode(encode_chan_service_tcb_t *encode_request_tcb)
{
    isil_h264_logic_encode_chan_t *h264_logic_encode_chan = to_get_isil_h264_encode_chan_with_encode_request_tcb(encode_request_tcb);
    tcb_node_t	*service_tcb = &encode_request_tcb->service_tcb;
    chip_encode_chan_service_queue_t    *chip_encode_service_queue;

    service_tcb->op = &tcb_node_op;
    service_tcb->op->init(service_tcb);
    service_tcb->op->set_priv(service_tcb, encode_request_tcb);
    encode_request_tcb->context = h264_logic_encode_chan;
    encode_request_tcb->type = DVM_CHIP_REQ_START_CHAN;
    encode_request_tcb->req_callback = h264_logic_encode_chan_start_encode;
    encode_request_tcb->delete_req_notify = h264_logic_encode_chan_delete_start_encode_req_notify;
    chip_encode_service_queue = &h264_logic_encode_chan->chip->chip_encode_service_queue;
    chip_encode_service_queue->op->put_service_request_into_queue(chip_encode_service_queue, encode_request_tcb);
}

static void h264_logic_encode_chan_gen_open_event_in_thread_context(isil_h264_logic_encode_chan_t  *h264_logic_encode_chan)
{
    if(h264_logic_encode_chan != NULL){
        ed_tcb_t    *opened_logic_chan_ed;
        unsigned long   flags;

        spin_lock_irqsave(&h264_logic_encode_chan->fsm_matrix_call_lock, flags);
        opened_logic_chan_ed = &h264_logic_encode_chan->opened_logic_chan_ed;
        driver_gen_open_event(opened_logic_chan_ed, 1);
        spin_unlock_irqrestore(&h264_logic_encode_chan->fsm_matrix_call_lock, flags);        
    }
}

static void h264_logic_encode_chan_gen_close_event_in_thread_context(isil_h264_logic_encode_chan_t  *h264_logic_encode_chan)
{
    if(h264_logic_encode_chan != NULL){
        ed_tcb_t        *opened_logic_chan_ed;
        unsigned long   flags;

        spin_lock_irqsave(&h264_logic_encode_chan->fsm_matrix_call_lock, flags);
        opened_logic_chan_ed = &h264_logic_encode_chan->opened_logic_chan_ed;
        driver_gen_close_event(opened_logic_chan_ed, 1);
        spin_unlock_irqrestore(&h264_logic_encode_chan->fsm_matrix_call_lock, flags);        
   }
}

static void h264_logic_encode_chan_gen_suspend_event_in_thread_context(isil_h264_logic_encode_chan_t  *h264_logic_encode_chan)
{
    if(h264_logic_encode_chan != NULL){
        ed_tcb_t        *opened_logic_chan_ed;
        unsigned long   flags;

        spin_lock_irqsave(&h264_logic_encode_chan->fsm_matrix_call_lock, flags);
        opened_logic_chan_ed = &h264_logic_encode_chan->opened_logic_chan_ed;
        driver_gen_suspend_event(opened_logic_chan_ed, 1);
        spin_unlock_irqrestore(&h264_logic_encode_chan->fsm_matrix_call_lock, flags);        
   }
}

static void h264_logic_encode_chan_gen_resume_event_in_thread_context(isil_h264_logic_encode_chan_t  *h264_logic_encode_chan)
{
    if(h264_logic_encode_chan != NULL){
        ed_tcb_t        *opened_logic_chan_ed;
        unsigned long   flags;

        spin_lock_irqsave(&h264_logic_encode_chan->fsm_matrix_call_lock, flags);
        opened_logic_chan_ed = &h264_logic_encode_chan->opened_logic_chan_ed;
        driver_gen_resume_event(opened_logic_chan_ed, 1);
        spin_unlock_irqrestore(&h264_logic_encode_chan->fsm_matrix_call_lock, flags);        
   }
}

static int  h264_encode_hcd_interface_open(ed_tcb_t *opened_logic_chan_ed)
{
    int	ret = ISIL_ERR;

    if(opened_logic_chan_ed != NULL){
        isil_h264_logic_encode_chan_t   *h264_logic_encode_chan = to_get_isil_h264_encode_chan_with_opened_logic_chan_ed(opened_logic_chan_ed);

        if(atomic_read(&h264_logic_encode_chan->opened_flag) == 0){
            isil_chip_t *chip;

            isil_h264_encode_chan_wait_robust_process_done(h264_logic_encode_chan);
            chip = h264_logic_encode_chan->chip;
            chip->chip_open(chip);
            ISIL_DBG(ISIL_DBG_INFO, "%d, %d\n", h264_logic_encode_chan->logic_chan_id, h264_logic_encode_chan->master_slot->phy_slot_id);
            h264_logic_encode_chan_gen_open_event_in_thread_context(h264_logic_encode_chan);
            ret = ISIL_OK;
        } else {
            ISIL_DBG(ISIL_DBG_ERR, "%d, channel have been opened\n\n", h264_logic_encode_chan->logic_chan_id);
        }
    }

    return ret;
}

static int  h264_encode_hcd_interface_close(ed_tcb_t *opened_logic_chan_ed)
{
    int	ret = ISIL_ERR;
    if(opened_logic_chan_ed != NULL){
        isil_h264_logic_encode_chan_t   *h264_logic_encode_chan = to_get_isil_h264_encode_chan_with_opened_logic_chan_ed(opened_logic_chan_ed);
        isil_kthread_msg_queue_t        *isil_msg_queue;
        isil_send_msg_controller_t      *send_msg_contr;
 
        if(atomic_read(&h264_logic_encode_chan->opened_flag) == 0){
            ISIL_DBG(ISIL_DBG_ERR, "%d, channel have been closed\n\n", h264_logic_encode_chan->logic_chan_id);
        } else {
            ISIL_DBG(ISIL_DBG_INFO, "%d, %d\n", h264_logic_encode_chan->logic_chan_id, h264_logic_encode_chan->master_slot->phy_slot_id);
            isil_h264_encode_chan_wait_robust_process_done(h264_logic_encode_chan);
            send_msg_contr = &h264_logic_encode_chan->send_msg_contr;
            send_msg_contr->op->delete_send_msg_controller(send_msg_contr);
            get_msg_queue_header(&isil_msg_queue);
            delete_all_msg_of_this_h264_encode_chan(isil_msg_queue, h264_logic_encode_chan);
            h264_logic_encode_chan_gen_close_event_in_thread_context(h264_logic_encode_chan);
            h264_logic_encode_chan->encode_control.i_statistic_minuter_counter = 0;
            unregister_opened_logic_encode_chan_into_master_slot(h264_logic_encode_chan, h264_logic_encode_chan->master_slot);
            {
                osd_chan_engine_t   *encode_osd_engine;
                encode_osd_engine = &h264_logic_encode_chan->encode_osd_engine;
                encode_osd_engine->op->release_rectangle(encode_osd_engine);
                encode_osd_engine->op->delete_chan_engine_rectangles(encode_osd_engine);
                close_encode_osd_chan_engine(encode_osd_engine);
            }
#ifdef  MV_MODULE
            {
                dpram_control_t *chip_dpram_controller;
                chip_dpram_controller = &h264_logic_encode_chan->chip->chip_dpram_controller;
                chip_dpram_controller->op->delete_read_mvflag_req(chip_dpram_controller, h264_logic_encode_chan);
                wait_for_completion(&h264_logic_encode_chan->mvflag_transfer_done);
                complete_all(&h264_logic_encode_chan->mvflag_transfer_done);
                remove_isil_video_mv_frame_tcb_queue(&h264_logic_encode_chan->encode_mv_frame_queue, &h264_logic_encode_chan->encode_chan_buf_pool);
            }
#endif
            remove_isil_h264_encode_control(&h264_logic_encode_chan->encode_control);
            remove_isil_h264_phy_video_slot(h264_logic_encode_chan->master_slot, h264_logic_encode_chan);
            remove_isil_video_frame_tcb_queue(&h264_logic_encode_chan->encode_frame_queue, &h264_logic_encode_chan->encode_chan_buf_pool);
            h264_logic_encode_chan->encode_control.op->reset(&h264_logic_encode_chan->encode_control);
            isil_h264_encode_chan_robust_process_done(h264_logic_encode_chan);
            ret = ISIL_OK;
            if(atomic_read(&h264_logic_encode_chan->opened_flag) == 0){
                isil_chip_t  *chip;
                chip = h264_logic_encode_chan->chip;
                chip->chip_close(chip);
            }
            ISIL_DBG(ISIL_DBG_INFO, "%d, %d\n\n", h264_logic_encode_chan->logic_chan_id, h264_logic_encode_chan->master_slot->phy_slot_id);        
        }
    }
    return ret;
}

static void h264_encode_hcd_interface_suspend(ed_tcb_t *opened_logic_chan_ed)
{
    if(opened_logic_chan_ed != NULL){
        isil_h264_logic_encode_chan_t   *h264_logic_encode_chan = to_get_isil_h264_encode_chan_with_opened_logic_chan_ed(opened_logic_chan_ed);
        isil_video_frame_tcb_queue_t    *video_frame_queue;
        isil_h264_encode_control_t      *encode_control;
        isil_send_msg_controller_t      *send_msg_contr;
        isil_register_node_t    *ed;
 
        ed = &opened_logic_chan_ed->ed;
        ed->op->init_wait_suspend_complete(ed);
        send_msg_contr = &h264_logic_encode_chan->send_msg_contr;
        send_msg_contr->op->delete_send_msg_controller(send_msg_contr);
        h264_logic_encode_chan_gen_suspend_event_in_thread_context(h264_logic_encode_chan);
        ed->op->wait_suspend_complete(ed);
        {
            osd_chan_engine_t   *encode_osd_engine;
            encode_osd_engine = &h264_logic_encode_chan->encode_osd_engine;
            encode_osd_engine->op->release_rectangle(encode_osd_engine);
            encode_osd_engine->op->delete_chan_engine_rectangles(encode_osd_engine);
            close_encode_osd_chan_engine(encode_osd_engine);
        }
#ifdef  MV_MODULE
        {
            dpram_control_t *chip_dpram_controller;
            chip_dpram_controller = &h264_logic_encode_chan->chip->chip_dpram_controller;
            chip_dpram_controller->op->delete_read_mvflag_req(chip_dpram_controller, h264_logic_encode_chan);
            wait_for_completion(&h264_logic_encode_chan->mvflag_transfer_done);
            complete_all(&h264_logic_encode_chan->mvflag_transfer_done);
        }
#endif
 
        encode_control = &h264_logic_encode_chan->encode_control;
        encode_control->op->clear_slice_head_ready(encode_control);
        video_frame_queue = &h264_logic_encode_chan->encode_frame_queue;
        video_frame_queue->op->release_curr_producer(video_frame_queue, &h264_logic_encode_chan->encode_chan_buf_pool);
        ed->op->suspend_complete_done(ed);
     }
}

static void h264_encode_hcd_interface_resume(ed_tcb_t *opened_logic_chan_ed)
{
    if(opened_logic_chan_ed != NULL){
        isil_h264_logic_encode_chan_t   *h264_logic_encode_chan = to_get_isil_h264_encode_chan_with_opened_logic_chan_ed(opened_logic_chan_ed);
        isil_register_node_t	*ed;

        ed = &opened_logic_chan_ed->ed;
        ed->op->init_wait_resume_complete(ed);
        h264_logic_encode_chan_gen_resume_event_in_thread_context(h264_logic_encode_chan);
        ed->op->wait_resume_complete(ed);
        ed->op->resume_complete_done(ed);
    }
}

static int  h264_encode_hcd_interface_ioctl(ed_tcb_t *opened_logic_chan_ed, unsigned int cmd, unsigned long arg)
{
    int ret = 0;
    isil_h264_logic_encode_chan_t   *h264_logic_encode_chan;
    isil_h264_encode_property_t     *encode_property;
    isil_h264_phy_video_slot_t      *phy_video_slot;
    isil_h264_logic_video_slot_t    *logic_video_slot;
    chip_driver_t                   *chip_driver;
    isil_video_bus_t                *video_bus;
    isil_h264_encode_control_t      *h264_encode_control;
    isil_h264_encode_configuration_t    *pconfig;
    struct isil_h264_encode_param       encode_param, *pencode_param;

    if(!opened_logic_chan_ed) {
        ISIL_DBG(ISIL_DBG_ERR, "invalid arg\n");
        return -EINVAL;
    }

    h264_logic_encode_chan = to_get_isil_h264_encode_chan_with_opened_logic_chan_ed(opened_logic_chan_ed);
    encode_property = &h264_logic_encode_chan->encode_property;
    phy_video_slot   = h264_logic_encode_chan->master_slot;
    logic_video_slot = phy_video_slot->cross_bus_logic_video_slot;
    chip_driver = h264_logic_encode_chan->chip->chip_driver;
    video_bus = &chip_driver->video_bus;
    h264_encode_control = &h264_logic_encode_chan->encode_control;
    pconfig = &encode_property->config_encode_property;
    pencode_param = &encode_param;
    ISIL_DBG(ISIL_DBG_INFO, "type: %c, cmd:%d, chip_id = %d, phy_slot_id = %d, logic_chan_id = %d\n", _IOC_TYPE(cmd), _IOC_NR(cmd), h264_logic_encode_chan->chip->chip_id, h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id);
    isil_h264_encode_chan_wait_robust_process_done(h264_logic_encode_chan);
    if(atomic_read(&h264_logic_encode_chan->opened_flag) == 0){
        ISIL_DBG(ISIL_DBG_ERR, "channel have not be opend\n");
        return -ENODEV;
    }
    switch(cmd) {
        case ISIL_CODEC_CHAN_FLUSH:
            {
                unsigned long   flags;
                spin_lock_irqsave(&h264_logic_encode_chan->fsm_matrix_call_lock, flags);
                h264_encode_control->op->flush_all_frame(h264_encode_control);
                spin_unlock_irqrestore(&h264_logic_encode_chan->fsm_matrix_call_lock, flags);
            }
            break;
        case DVM_CODEC_GET_VIDEO_ENCODER_PARAM:
            if(!arg) {
                ISIL_DBG(ISIL_DBG_ERR, "invalid arg\n");
                return -EINVAL;
            }
            pencode_param->i_bps = pconfig->bitrate;
            pencode_param->i_fps = pconfig->fps;
            pencode_param->i_I_P_stride = pconfig->keyFrameIntervals;
            pencode_param->i_P_B_stride = 0;
            pencode_param->i_gop_value = pconfig->gopIntervals;
            pencode_param->b_force_I_flag = 0;
            pencode_param->i_logic_video_width_mb_size = pconfig->width>>4;
            pencode_param->i_logic_video_height_mb_size = pconfig->height>>4;
            ISIL_DBG(ISIL_DBG_INFO, "[%d-%d]:bps=%d, fps=%d, I_P_stride=%d, gop=%d, width=%d, height=%d\n",
                        h264_logic_encode_chan->chip->chip_id,h264_logic_encode_chan->logic_chan_id,
                        pencode_param->i_bps, pencode_param->i_fps, pencode_param->i_I_P_stride,
                        pencode_param->i_gop_value, pencode_param->i_logic_video_width_mb_size,
                        pencode_param->i_logic_video_height_mb_size);
            if(copy_to_user((void *)arg, pencode_param, sizeof(struct isil_h264_encode_param)) != 0){
                printk("%s, %d: copy_to_user failed\n", __FUNCTION__, __LINE__);
            }
            break;
        case DVM_CODEC_SET_VIDEO_ENCODER_PARAM:
            if(!arg) {
                ISIL_DBG(ISIL_DBG_ERR, "invalid arg\n");
                return -EINVAL;
            }
            if(copy_from_user(pencode_param, (void *)arg, sizeof(struct isil_h264_encode_param)) != 0){
                printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
            } else {
                unsigned long   flags;
                spin_lock_irqsave(&encode_property->lock, flags);
                /*check parameter*/
                if(pencode_param->change_mask_flag & ISIL_h264_ENCODER_PARAM_BPS_MASK){
                    if(h264_logic_encode_chan->type == ISIL_MASTER_BITSTREAM){
                        if((pencode_param->i_bps < MASTER_MIN_BIT_RATE) || (pencode_param->i_bps > MASTER_MAX_BIT_RATE)){
                            ISIL_DBG(ISIL_DBG_ERR, "bitrate error %d [%d, %d]!\n", pencode_param->i_bps,
                                    MASTER_MIN_BIT_RATE, MASTER_MAX_BIT_RATE);
                            ret = -EINVAL;
                            spin_unlock_irqrestore(&encode_property->lock, flags);
                            break;
                        }
                        pconfig->bitrate = pencode_param->i_bps;
                        pconfig->changedMask |= ISIL_ENCODE_CONFIG_BITRATE_CHANGED;
                    }else{
                        if((pencode_param->i_bps < SUB_MIN_BIT_RATE) || (pencode_param->i_bps > SUB_MAX_BIT_RATE)){
                            ISIL_DBG(ISIL_DBG_ERR, "bitrate error %d [%d, %d]!\n", pencode_param->i_bps,
                                    SUB_MIN_BIT_RATE, SUB_MAX_BIT_RATE);
                            ret = -EINVAL;
                            spin_unlock_irqrestore(&encode_property->lock, flags);
                            break;
                        }
                        pconfig->bitrate = pencode_param->i_bps;
                        pconfig->changedMask |= ISIL_ENCODE_CONFIG_BITRATE_CHANGED;
                    }
                }

                if(pencode_param->change_mask_flag & ISIL_h264_ENCODER_PARAM_FPS_MASK){
                    int max_frame_rate, min_frame_rate;
                    if(video_bus->op->get_video_standard(video_bus) == ISIL_VIDEO_STANDARD_PAL) {
                        max_frame_rate = MAX_FRAME_RATE_PAL;
                        min_frame_rate = MIN_FRAME_RATE;
                    }else{
                        max_frame_rate = MAX_FRAME_RATE_NTSC;
                        min_frame_rate = MIN_FRAME_RATE;
                    }
                    if((pencode_param->i_fps < min_frame_rate) || (pencode_param->i_fps > max_frame_rate)) {
                        ISIL_DBG(ISIL_DBG_ERR, "fps error %d [%d, %d]!\n", pencode_param->i_fps,
                                min_frame_rate, max_frame_rate);
                        ret = -EINVAL;
                        spin_unlock_irqrestore(&encode_property->lock, flags);
                        break;
                    }else{
                        if(pencode_param->i_fps > logic_video_slot->op->get_fps(logic_video_slot)) {
                            ISIL_DBG(ISIL_DBG_ERR,"target fps %d should small than vi fps %d\n",pencode_param->i_fps,logic_video_slot->op->get_fps(logic_video_slot));
                            ret = -EINVAL;
                            spin_unlock_irqrestore(&encode_property->lock, flags);
                            break;
                        }
                        pconfig->fps = pencode_param->i_fps;
                        pconfig->changedMask |= ISIL_ENCODE_CONFIG_FPS_CHANGED;
                    }
                }

                if(pencode_param->change_mask_flag & ISIL_h264_ENCODER_PARAM_GOP_MASK){
                    if((pencode_param->i_gop_value < MIN_GOP) || (pencode_param->i_gop_value > MAX_GOP)) {
                        ISIL_DBG(ISIL_DBG_ERR, "gop error %d [%d, %d]!\n", pencode_param->i_gop_value,
                                MIN_GOP, MAX_GOP);
                        ret = -EINVAL;
                        spin_unlock_irqrestore(&encode_property->lock, flags);
                        break;
                    }else{
                        pconfig->gopIntervals = pencode_param->i_gop_value;
                        pconfig->changedMask |= ISIL_ENCODE_CONFIG_GOP_INTERVALS_CHANGED;
                    }
                }

                if(pencode_param->change_mask_flag & ISIL_h264_ENCODER_PARAM_FORCE_I_MASK){
                    h264_encode_control->op->force_I_frame(h264_encode_control);
                }

                if(pencode_param->change_mask_flag & ISIL_h264_ENCODER_PARAM_IP_STRIDE_MASK){
                    if((pencode_param->i_I_P_stride < MIN_I_P_STRIDE) || (pencode_param->i_I_P_stride > MAX_I_P_STRIDE)) {
                        ISIL_DBG(ISIL_DBG_ERR, "key frame interval error %d [%d, %d]!\n", pencode_param->i_I_P_stride,
                                MIN_I_P_STRIDE, MAX_I_P_STRIDE);
                        ret = -EINVAL;
                        spin_unlock_irqrestore(&encode_property->lock, flags);
                        break;
                    }else{
                        pconfig->keyFrameIntervals = pencode_param->i_I_P_stride;
                        pconfig->changedMask |= ISIL_ENCODE_CONFIG_KEYFRAME_INTERVALS_CHANGED;
                    }
                }

                if(pencode_param->change_mask_flag & ISIL_h264_ENCODER_PARAM_WIDTH_MASK) {
                    if(pencode_param->change_mask_flag & ISIL_h264_ENCODER_PARAM_HEIGHT_MASK) {
                        if((pencode_param->i_logic_video_height_mb_size == 0) 
                                || (pencode_param->i_logic_video_height_mb_size > VIDEO_SIZE_TO_HEIGHT(phy_video_slot->video_size, video_bus->op->get_video_standard(video_bus)))) {
                            ISIL_DBG(ISIL_DBG_ERR, "height (%d) error at norm %d\n", pencode_param->i_logic_video_height_mb_size, video_bus->op->get_video_standard(video_bus));
                            ret = -EINVAL;
                            spin_unlock_irqrestore(&encode_property->lock, flags);
                            break;
                        }
                        if((pencode_param->i_logic_video_width_mb_size == 0) 
                                || (pencode_param->i_logic_video_width_mb_size > VIDEO_SIZE_TO_WIDTH(phy_video_slot->video_size, video_bus->op->get_video_standard(video_bus)))) {
                            ISIL_DBG(ISIL_DBG_ERR, "width (%d) error at norm %d\n", pencode_param->i_logic_video_width_mb_size, video_bus->op->get_video_standard(video_bus));
                            ret = -EINVAL;
                            spin_unlock_irqrestore(&encode_property->lock, flags);
                            break;
                        }
                        if(h264_logic_encode_chan->type == ISIL_SUB_BITSTREAM) {
                            if((pencode_param->i_logic_video_width_mb_size<<4) >=VIDEO_SIZE_TO_WIDTH(phy_video_slot->video_size, video_bus->op->get_video_standard(video_bus))) {
                                ISIL_DBG(ISIL_DBG_ERR, "width should small than vi\n");
                                ret = -EINVAL;
                                spin_unlock_irqrestore(&encode_property->lock, flags);
                                break;
                            }
                            if((pencode_param->i_logic_video_height_mb_size<<4) >=VIDEO_SIZE_TO_HEIGHT(phy_video_slot->video_size, video_bus->op->get_video_standard(video_bus))) {
                                ISIL_DBG(ISIL_DBG_ERR, "heigh should small than vi\n");
                                ret = -EINVAL;
                                spin_unlock_irqrestore(&encode_property->lock, flags);
                                break;
                            }
                        }
                        ISIL_DBG(ISIL_DBG_INFO, "channel %d, update width and height to (%d, %d)\n", h264_logic_encode_chan->logic_chan_id, pencode_param->i_logic_video_width_mb_size, pencode_param->i_logic_video_height_mb_size);
                    }else{
                        ISIL_DBG(ISIL_DBG_ERR, "need set height\n");
                        ret = -EINVAL;
                        spin_unlock_irqrestore(&encode_property->lock, flags);
                        break;
                    }

                    pconfig->width = pencode_param->i_logic_video_width_mb_size<<4;
                    pconfig->height = pencode_param->i_logic_video_height_mb_size<<4;
                    pconfig->changedMask |= ISIL_ENCODE_CONFIG_ENCODE_SIZE_CHANGED;

                }
                ISIL_DBG(ISIL_DBG_INFO, "[%d-%d]:width = %3d, height = %3d, bps = %8d, fps = %2d, I_P_stride = %2d, P_B_stride = %2d, gop = %d, changed_mask = %08x\n",
                        h264_logic_encode_chan->chip->chip_id,h264_logic_encode_chan->logic_chan_id,
                        pencode_param->i_logic_video_width_mb_size, pencode_param->i_logic_video_height_mb_size,
                        pencode_param->i_bps, pencode_param->i_fps, pencode_param->i_I_P_stride, pencode_param->i_P_B_stride, 
                        pencode_param->i_gop_value, pencode_param->change_mask_flag);
                spin_unlock_irqrestore(&encode_property->lock, flags);
                driver_sync_config_to_running(opened_logic_chan_ed);
            }
            ret = ISIL_OK;
            break;
        case ISIL_LOGIC_CHAN_ENABLE_SET:
            opened_logic_chan_ed->op->resume(opened_logic_chan_ed);
            ret = ISIL_OK;
            break;
        case ISIL_LOGIC_CHAN_DISABLE_SET:
            opened_logic_chan_ed->op->suspend(opened_logic_chan_ed);
            ret = ISIL_OK;
            break;
        case ISIL_H264_ENCODE_FEATURE_SET:
            if(!arg) {
                ISIL_DBG(ISIL_DBG_ERR, "invalid arg\n");
                return -EINVAL;
            }
            if(h264_logic_encode_chan) {
                isil_h264_encode_feature_param_t h264_feature, *pconfig_feature, *pfeature = &h264_feature;

                pconfig_feature = &encode_property->encode_feature.config_feature_param;
                if(copy_from_user(pfeature, (void *)arg, sizeof(isil_h264_encode_feature_param_t)) != 0){
                    printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                } else {
                    unsigned long   flags;
                    spin_lock_irqsave(&encode_property->lock, flags);
                    if(pfeature->change_mask_flag & ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_DEINTERLACE_MASK) {
                        pconfig_feature->b_enable_deinterlace = pfeature->b_enable_deinterlace;
                        pconfig_feature->change_mask_flag |= ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_DEINTERLACE_MASK;
                    }
                    if(pfeature->change_mask_flag & ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_SKIP_MASK) {
                        pconfig_feature->b_enable_skip = pfeature->b_enable_skip;
                        pconfig_feature->change_mask_flag |= ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_SKIP_MASK;
                    }
                    if(pfeature->change_mask_flag & ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_I_4X4_MASK) {
                        pconfig_feature->b_enable_I_4X4 = pfeature->b_enable_I_4X4;
                        pconfig_feature->change_mask_flag |= ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_I_4X4_MASK;
                    }
                    if(pfeature->change_mask_flag & ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_QUARTER_PIXEL_MASK) {
                        pconfig_feature->b_enable_half_pixel = pfeature->b_enable_half_pixel;
                        pconfig_feature->change_mask_flag |= ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_QUARTER_PIXEL_MASK;
                    }
                    if(pfeature->change_mask_flag & ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_QUARTER_PIXEL_MASK) {
                        pconfig_feature->b_enable_quarter_pixel = pfeature->b_enable_quarter_pixel;
                        pconfig_feature->change_mask_flag |= ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_QUARTER_PIXEL_MASK;
                    }
                    if(pfeature->change_mask_flag & ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_MB_DELAY_MASK) {
                        if(pfeature->i_mb_delay_value) {
                            if((pfeature->i_mb_delay_value >= MAX_MB_DELAY) || (pfeature->i_mb_delay_value < MIN_MB_DELAY)){
                                ISIL_DBG(ISIL_DBG_ERR, "mb_delay %d error, [%d, %d)\n", pfeature->i_mb_delay_value, MIN_MB_DELAY, MAX_MB_DELAY);
                                ret = -EINVAL;
                                spin_unlock_irqrestore(&encode_property->lock, flags);
                                break;
                            }
                            pconfig_feature->i_mb_delay_value = pfeature->i_mb_delay_value;
                            pconfig_feature->change_mask_flag |= ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_MB_DELAY_MASK;
                        }else{
                            ISIL_DBG(ISIL_DBG_ERR, "mb_delay 0x%x error\n", pfeature->i_mb_delay_value);
                            ret = -EINVAL;
                            spin_unlock_irqrestore(&encode_property->lock, flags);
                            break;
                        }
                    }
                    spin_unlock_irqrestore(&encode_property->lock, flags);
                    ISIL_DBG(ISIL_DBG_INFO, "[%d-%d]:deint=%d, skip=%d, I_4X4=%d, 1/2pixel=%d, 1/4pixel=%d, mb_dealy=%d\n",
                            h264_logic_encode_chan->chip->chip_id,h264_logic_encode_chan->logic_chan_id,
                            pconfig_feature->b_enable_deinterlace, pconfig_feature->b_enable_skip,
                            pconfig_feature->b_enable_I_4X4, pconfig_feature->b_enable_half_pixel,
                            pconfig_feature->b_enable_quarter_pixel, pconfig_feature->i_mb_delay_value);
                    driver_sync_config_to_running(opened_logic_chan_ed);
                }
                ret = ISIL_OK;
            }
            break;
        case ISIL_H264_ENCODE_FEATURE_GET:
            if(!arg) {
                ISIL_DBG(ISIL_DBG_ERR, "invalid arg\n");
                return -EINVAL;
            }
            if(h264_logic_encode_chan) {
                isil_h264_encode_feature_param_t  h264_feature, *prunning_feature, *pfeature = &h264_feature;

                prunning_feature = &encode_property->encode_feature.running_feature_param;
                pfeature->b_enable_deinterlace = prunning_feature->b_enable_deinterlace;
                pfeature->b_enable_skip = prunning_feature->b_enable_skip;
                pfeature->b_enable_I_4X4 = prunning_feature->b_enable_I_4X4;
                pfeature->b_enable_half_pixel = prunning_feature->b_enable_half_pixel;
                pfeature->b_enable_quarter_pixel = prunning_feature->b_enable_quarter_pixel;
                pfeature->i_mb_delay_value = prunning_feature->i_mb_delay_value;
                ISIL_DBG(ISIL_DBG_INFO, "[%d-%d]:deint=%d, skip=%d, I_4X4=%d, 1/2pixel=%d, 1/4pixel=%d, mb_dealy=%d\n",
                        h264_logic_encode_chan->chip->chip_id,h264_logic_encode_chan->logic_chan_id,
                        prunning_feature->b_enable_deinterlace, prunning_feature->b_enable_skip,
                        prunning_feature->b_enable_I_4X4, prunning_feature->b_enable_half_pixel,
                        prunning_feature->b_enable_quarter_pixel, prunning_feature->i_mb_delay_value);
                if(copy_to_user((void *)arg, pfeature, sizeof(isil_h264_encode_feature_param_t)) != 0){
                    printk("%s, %d: copy_to_user failed\n", __FUNCTION__, __LINE__);
                }
                ret = ISIL_OK;
            }
            break;
        case ISIL_H264_ENCODE_RC_SET:
            if(!arg) {
                ISIL_DBG(ISIL_DBG_ERR, "invalid arg\n");
                return -EINVAL;
            }
            if(h264_logic_encode_chan) {
                isil_h264_encode_rc_t   *encode_rc;
                isil_h264_encode_rc_config_t    rc, *p_rc = &rc, *config_rc;
                unsigned long   flags;

                encode_rc = &encode_property->encode_rc;
                if(copy_from_user(p_rc, (void *)arg, sizeof(isil_h264_encode_rc_config_t)) != 0){
                    printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                } else {
                    spin_lock_irqsave(&encode_property->lock, flags);
                    config_rc = &encode_rc->config_rc;
                    if(p_rc->change_mask_flag & ISIL_H264_ENCODE_RC_ENABLE_CHANGE_IMAGE_PRIORITY_MASK  ) {
                        if((p_rc->e_image_priority >= ISIL_RC_IMAGE_QUALITY_FIRST) && (p_rc->e_image_priority <= ISIL_RC_IMAGE_SMOOTH_FIRST)) {
                            config_rc->e_image_priority = p_rc->e_image_priority;
                            config_rc->change_mask_flag |= ISIL_H264_ENCODE_RC_ENABLE_CHANGE_IMAGE_PRIORITY_MASK;
                            ISIL_DBG(ISIL_DBG_INFO, "image_priority %d\n", config_rc->e_image_priority);
                        }else{
                            ISIL_DBG(ISIL_DBG_ERR, "unknown image_priority %d\n", p_rc->e_image_priority);
                            ret = -EINVAL;
                            spin_unlock_irqrestore(&encode_property->lock, flags);
                            break;
                        }
                    }
                    if(p_rc->change_mask_flag & ISIL_H264_ENCODE_RC_ENABLE_CHANGE_RC_TYPE_MASK) {
                        if((p_rc->e_rc_type >= ISIL_H264_NO_RC) && (p_rc->e_rc_type <= ISIL_H264_VBR)) {
                            config_rc->e_rc_type = p_rc->e_rc_type;
                            config_rc->change_mask_flag |= ISIL_H264_ENCODE_RC_ENABLE_CHANGE_RC_TYPE_MASK;
                            ISIL_DBG(ISIL_DBG_INFO, "rc_type %d\n", config_rc->e_rc_type);
                        }else{
                            ISIL_DBG(ISIL_DBG_ERR, "unknown rc_type %d\n", p_rc->e_rc_type);
                            ret = -EINVAL;
                            spin_unlock_irqrestore(&encode_property->lock, flags);
                            break;
                        }
                    }
                    if(p_rc->change_mask_flag & ISIL_H264_ENCODE_RC_ENABLE_CHANGE_QPI_MASK) {
                        int qp_min = QUANT_SCALE_MIN, qp_max = QUANT_SCALE_MAX;
                        if(p_rc->change_mask_flag & ISIL_H264_ENCODE_RC_ENABLE_CHANGE_RC_TYPE_MASK) {
                            if(p_rc->e_rc_type == ISIL_H264_NO_RC) {
                                qp_min = 12;
                            }
                        }
                        if((p_rc->i_qpi >= qp_min) && (p_rc->i_qpi <= qp_max)) {
                            config_rc->i_qpi = p_rc->i_qpi;
                            config_rc->change_mask_flag |= ISIL_H264_ENCODE_RC_ENABLE_CHANGE_QPI_MASK;
                            ISIL_DBG(ISIL_DBG_INFO, "i_qpi %d\n", config_rc->i_qpi);
                        }else{
                            ISIL_DBG(ISIL_DBG_ERR, "qpi %d error, [%d, %d]\n", p_rc->i_qpi, qp_min, qp_max);
                            ret = -EINVAL;
                            spin_unlock_irqrestore(&encode_property->lock, flags);
                            break;
                        }
                    }
                    if(p_rc->change_mask_flag & ISIL_H264_ENCODE_RC_ENABLE_CHANGE_QPP_MASK) {
                        int qp_min = QUANT_SCALE_MIN, qp_max = QUANT_SCALE_MAX;
                        if((p_rc->i_qpp >= qp_min) && (p_rc->i_qpp <= qp_max)) {
                            config_rc->i_qpp = p_rc->i_qpp;
                            config_rc->change_mask_flag |= ISIL_H264_ENCODE_RC_ENABLE_CHANGE_QPP_MASK;
                            ISIL_DBG(ISIL_DBG_INFO, "i_qpp %d\n", config_rc->i_qpp);
                        }else{
                            ISIL_DBG(ISIL_DBG_ERR, "qpp %d error, [%d, %d]\n", p_rc->i_qpp, qp_min, qp_max);
                            ret = -EINVAL;
                            spin_unlock_irqrestore(&encode_property->lock, flags);
                            break;
                        }
                    }
                    if(p_rc->change_mask_flag & ISIL_H264_ENCODE_RC_ENABLE_CHANGE_QPB_MASK) {
                        int qp_min = QUANT_SCALE_MIN, qp_max = QUANT_SCALE_MAX;
                        if((p_rc->i_qpb >= qp_min) && (p_rc->i_qpb <= qp_max)) {
                            config_rc->i_qpb = p_rc->i_qpb;
                            config_rc->change_mask_flag |= ISIL_H264_ENCODE_RC_ENABLE_CHANGE_QPB_MASK;
                            ISIL_DBG(ISIL_DBG_INFO, "i_qpb %d\n", config_rc->i_qpb);
                        }else{
                            ISIL_DBG(ISIL_DBG_ERR, "qpb %d error, [%d, %d]\n", p_rc->i_qpb, qp_min, qp_max);
                            ret = -EINVAL;
                            spin_unlock_irqrestore(&encode_property->lock, flags);
                            break;
                        }
                    }
                    spin_unlock_irqrestore(&encode_property->lock, flags);
                    ISIL_DBG(ISIL_DBG_INFO, "[%d-%d]:image_pri=%d, rc_type=%d, qpi=%d, qpp=%d, qpb=%d\n",
                            h264_logic_encode_chan->chip->chip_id,h264_logic_encode_chan->logic_chan_id,
                            config_rc->e_image_priority, config_rc->e_rc_type, config_rc->i_qpi,
                            config_rc->i_qpp, config_rc->i_qpb);
                    driver_sync_config_to_running(opened_logic_chan_ed);
                    ret = ISIL_OK;
                }
            }
            break;
        case ISIL_H264_ENCODE_RC_GET:
            if(!arg) {
                ISIL_DBG(ISIL_DBG_ERR, "invalid arg\n");
                return -EINVAL;
            }
            if(h264_logic_encode_chan) {
                isil_h264_encode_rc_t *encode_rc;
                isil_h264_encode_rc_config_t rc, *p_rc = &rc, *running_rc;

                encode_rc = &encode_property->encode_rc;
                running_rc = &encode_rc->running_rc;
                p_rc->e_rc_type = running_rc->e_rc_type;
                p_rc->e_image_priority = running_rc->e_image_priority;
                p_rc->i_qpi = running_rc->i_qpi;
                p_rc->i_qpp = running_rc->i_qpp;
                p_rc->i_qpb = running_rc->i_qpb;
                ISIL_DBG(ISIL_DBG_INFO, "[%d-%d]:image_pri=%d, rc_type=%d, qpi=%d, qpp=%d, qpb=%d\n",
                        h264_logic_encode_chan->chip->chip_id,h264_logic_encode_chan->logic_chan_id,
                        running_rc->e_image_priority, running_rc->e_rc_type, running_rc->i_qpi,
                        running_rc->i_qpp, running_rc->i_qpb);
                if(copy_to_user((void *)arg, p_rc, sizeof(isil_h264_encode_rc_config_t)) != 0){
                    printk("%s, %d: copy_to_user failed\n", __FUNCTION__, __LINE__);                    
                }
                ret = ISIL_OK;
            }
            break;
        case ISIL_H264_ENCODE_RC_PARAM_SET:
        case ISIL_H264_ENCODE_RC_PARAM_GET:
            ISIL_DBG(ISIL_DBG_ERR, "not support\n");
            ret = -EPERM;
            break;
        case ISIL_H264_ENCODE_CHAN_MAP_GET:
            if(!arg) {
                ISIL_DBG(ISIL_DBG_ERR, "invalid arg\n");
                return -EINVAL;
            }
            if(h264_logic_encode_chan){
                struct isil_h264_chan_map_info map_info, *p_map = &map_info;

                p_map->i_phy_chan_id   = h264_logic_encode_chan->master_slot->phy_slot_id;
                p_map->i_logic_chan_id = h264_logic_encode_chan->master_slot->cross_bus_logic_video_slot->logicSlotId;

                if(copy_to_user((void *)arg, p_map, sizeof(struct isil_h264_chan_map_info)) != 0){
                    printk("%s, %d: copy_to_user failed\n", __FUNCTION__, __LINE__);
                }
            }
            ret = ISIL_OK;
            break;
        case ISIL_LOGIC_CHAN_OSD_GET_PARAM:
            if(arg){
                ISIL_OSD_PARAM    *osd_config_param;

                osd_config_param = &h264_logic_encode_chan->encode_osd_engine.osd_running_param;
                if(copy_to_user((void *)arg, osd_config_param, sizeof(ISIL_OSD_PARAM)) != 0){
                    printk("%s, %d: copy_to_user failed\n", __FUNCTION__, __LINE__);
                }
            }else{
                ISIL_DBG(ISIL_DBG_ERR, "invalid arg\n");
                return -EINVAL;
            }
            break;
        case ISIL_LOGIC_CHAN_OSD_SET_PARAM:
            if(arg){
                ISIL_OSD_PARAM    *osd_config_param, config_param;
                osd_chan_engine_t *engine;
                u32 width , height;

                osd_config_param = &config_param;
                if(copy_from_user(osd_config_param, (void *)arg, sizeof(ISIL_OSD_PARAM)) != 0){
                    printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                } else {
                    engine = &h264_logic_encode_chan->encode_osd_engine;
                    width = encode_property->op->get_encodeSize_width(encode_property);
                    height = encode_property->op->get_encodeSize_height(encode_property);
                    /*name*/
                    osd_config_param->name[NAME_LEN - 1] = '\0';
                    if((osd_config_param->name_attrib != OSD_ATTR_DISPLAY_ON) && (osd_config_param->name_attrib != OSD_ATTR_DISPLAY_OFF)){
                        ISIL_DBG(ISIL_DBG_ERR, "osd name_attrib %d error, only support OSD_ATTR_DISPLAY_ON, OSD_ATTR_DISPLAY_OFF\n", osd_config_param->name_attrib);
                        goto OSD_FAILED;
                    }
                    if(osd_config_param->name_pos_x >= width) {
                        ISIL_DBG(ISIL_DBG_ERR, "osd name_pos_x %d overflow, should small than %d\n", osd_config_param->name_pos_x, width);
                        goto OSD_FAILED;
                    }
                    if(osd_config_param->name_pos_y >= height) {
                        ISIL_DBG(ISIL_DBG_ERR, "osd name_pos_y %d overflow, should small than %d\n", osd_config_param->name_pos_y, height);
                        goto OSD_FAILED;
                    }
                    /*time*/
                    if((osd_config_param->time_attrib != OSD_ATTR_DISPLAY_ON) && (osd_config_param->time_attrib != OSD_ATTR_DISPLAY_OFF)){
                        ISIL_DBG(ISIL_DBG_ERR, "osd time_attrib %d error, only support OSD_ATTR_DISPLAY_ON, OSD_ATTR_DISPLAY_OFF\n", osd_config_param->time_attrib);
                        goto OSD_FAILED;
                    }
                    if(osd_config_param->time_pos_x >= width) {
                        ISIL_DBG(ISIL_DBG_ERR, "osd time_pos_x %d overflow, should small than %d\n", osd_config_param->time_pos_x, width);
                        goto OSD_FAILED;
                    }
                    if(osd_config_param->time_pos_y >= height) {
                        ISIL_DBG(ISIL_DBG_ERR, "osd time_pos_y %d overflow, should small than %d\n", osd_config_param->time_pos_y, height);
                        goto OSD_FAILED;
                    }
                    /*mask 0*/
                    if((osd_config_param->shelter1_attrib != OSD_ATTR_DISPLAY_ON) && (osd_config_param->shelter1_attrib != OSD_ATTR_DISPLAY_OFF)){
                        ISIL_DBG(ISIL_DBG_ERR, "osd shelter1_attrib %d error, only support OSD_ATTR_DISPLAY_ON, OSD_ATTR_DISPLAY_OFF\n", osd_config_param->shelter1_attrib);
                        goto OSD_FAILED;
                    }
                    if((osd_config_param->shelter1_pos_x >= width) || ((osd_config_param->shelter1_pos_x + osd_config_param->shelter1_width) >width)
                            ||(osd_config_param->shelter1_pos_y >= height) || ((osd_config_param->shelter1_pos_y + osd_config_param->shelter1_height) >height)) {
                        ISIL_DBG(ISIL_DBG_ERR, "osd shelter1 region (%d, %d, %d, %d) overflow\n", osd_config_param->shelter1_pos_x,
                                osd_config_param->shelter1_pos_y, osd_config_param->shelter1_width, osd_config_param->shelter1_height);
                        goto OSD_FAILED;
                    }
                    /*mask 1*/
                    if((osd_config_param->shelter2_attrib != OSD_ATTR_DISPLAY_ON) && (osd_config_param->shelter2_attrib != OSD_ATTR_DISPLAY_OFF)){
                        ISIL_DBG(ISIL_DBG_ERR, "osd shelter2_attrib %d error, only support OSD_ATTR_DISPLAY_ON, OSD_ATTR_DISPLAY_OFF\n", osd_config_param->shelter2_attrib);
                        goto OSD_FAILED;
                    }
                    if((osd_config_param->shelter2_pos_x >= width) || ((osd_config_param->shelter2_pos_x + osd_config_param->shelter2_width) >width)
                            ||(osd_config_param->shelter2_pos_y >= height) || ((osd_config_param->shelter2_pos_y + osd_config_param->shelter2_height) >height)) {
                        ISIL_DBG(ISIL_DBG_ERR, "osd shelter2 region (%d, %d, %d, %d) overflow\n", osd_config_param->shelter2_pos_x,
                                osd_config_param->shelter2_pos_y, osd_config_param->shelter2_width, osd_config_param->shelter2_height);
                        goto OSD_FAILED;
                    }
                    /*subtitle 0*/
                    osd_config_param->subtitle1[SUB_LEN - 1] = '\0';
                    if((osd_config_param->subtitle1_attrib != OSD_ATTR_DISPLAY_ON) && (osd_config_param->subtitle1_attrib != OSD_ATTR_DISPLAY_OFF)){
                        ISIL_DBG(ISIL_DBG_ERR, "osd subtitle1_attrib %d error, only support OSD_ATTR_DISPLAY_ON, OSD_ATTR_DISPLAY_OFF\n", osd_config_param->subtitle1_attrib);
                        goto OSD_FAILED;
                    }
                    if(osd_config_param->subtitle1_pos_x >= width) {
                        ISIL_DBG(ISIL_DBG_ERR, "osd subtitle1_pos_x %d overflow, should small than %d\n", osd_config_param->subtitle1_pos_x, width);
                        goto OSD_FAILED;
                    }
                    if(osd_config_param->subtitle1_pos_y >= height) {
                        ISIL_DBG(ISIL_DBG_ERR, "osd subtitle1_pos_y %d overflow, should small than %d\n", osd_config_param->subtitle1_pos_y, height);
                        goto OSD_FAILED;
                    }
                    /*subtitle 1*/
                    osd_config_param->subtitle2[SUB_LEN - 1] = '\0';
                    if((osd_config_param->subtitle2_attrib != OSD_ATTR_DISPLAY_ON) && (osd_config_param->subtitle2_attrib != OSD_ATTR_DISPLAY_OFF)){
                        ISIL_DBG(ISIL_DBG_ERR, "osd subtitle2_attrib %d error, only support OSD_ATTR_DISPLAY_ON, OSD_ATTR_DISPLAY_OFF\n", osd_config_param->subtitle2_attrib);
                        goto OSD_FAILED;
                    }
                    if(osd_config_param->subtitle2_pos_x >= width) {
                        ISIL_DBG(ISIL_DBG_ERR, "osd subtitle2_pos_x %d overflow, should small than %d\n", osd_config_param->subtitle2_pos_x, width);
                        goto OSD_FAILED;
                    }
                    if(osd_config_param->subtitle2_pos_y >= height) {
                        ISIL_DBG(ISIL_DBG_ERR, "osd subtitle2_pos_y %d overflow, should small than %d\n", osd_config_param->subtitle2_pos_y, height);
                        goto OSD_FAILED;
                    }
                    /*subtitle 2*/
                    osd_config_param->subtitle3[SUB_LEN - 1] = '\0';
                    if((osd_config_param->subtitle3_attrib != OSD_ATTR_DISPLAY_ON) && (osd_config_param->subtitle3_attrib != OSD_ATTR_DISPLAY_OFF)){
                        ISIL_DBG(ISIL_DBG_ERR, "osd subtitle3_attrib %d error, only support OSD_ATTR_DISPLAY_ON, OSD_ATTR_DISPLAY_OFF\n", osd_config_param->subtitle3_attrib);
                        goto OSD_FAILED;
                    }
                    if(osd_config_param->subtitle3_pos_x >= width) {
                        ISIL_DBG(ISIL_DBG_ERR, "osd subtitle3_pos_x %d overflow, should small than %d\n", osd_config_param->subtitle3_pos_x, width);
                        goto OSD_FAILED;
                    }
                    if(osd_config_param->subtitle3_pos_y >= height) {
                        ISIL_DBG(ISIL_DBG_ERR, "osd subtitle3_pos_y %d overflow, should small than %d\n", osd_config_param->subtitle3_pos_y, height);
                        goto OSD_FAILED;
                    }
                    /*subtitle 0*/
                    osd_config_param->subtitle4[SUB_LEN - 1] = '\0';
                    if((osd_config_param->subtitle4_attrib != OSD_ATTR_DISPLAY_ON) && (osd_config_param->subtitle4_attrib != OSD_ATTR_DISPLAY_OFF)){
                        ISIL_DBG(ISIL_DBG_ERR, "osd subtitle4_attrib %d error, only support OSD_ATTR_DISPLAY_ON, OSD_ATTR_DISPLAY_OFF\n", osd_config_param->subtitle4_attrib);
                        goto OSD_FAILED;
                    }
                    if(osd_config_param->subtitle4_pos_x >= width) {
                        ISIL_DBG(ISIL_DBG_ERR, "osd subtitle4_pos_x %d overflow, should small than %d\n", osd_config_param->subtitle4_pos_x, width);
                        goto OSD_FAILED;
                    }
                    if(osd_config_param->subtitle4_pos_y >= height) {
                        ISIL_DBG(ISIL_DBG_ERR, "osd subtitle4_pos_y %d overflow, should small than %d\n", osd_config_param->subtitle4_pos_y, height);
                        goto OSD_FAILED;
                    }
                    engine->op->update_encode_osd_property(engine, osd_config_param);
                }
                return ISIL_OK;
OSD_FAILED:
                return -EINVAL;
            }else{
                ISIL_DBG(ISIL_DBG_ERR, "invalid arg\n");
                return -EINVAL;
            }
            break;
        default:
            return -ENODEV;
    }

    return ret;
}

static int  h264_encode_hcd_interface_get_state(ed_tcb_t *opened_logic_chan_ed)
{
    int ret = ISIL_ED_UNREGISTER;
    if(opened_logic_chan_ed != NULL){
        isil_ed_fsm_t   *ed_fsm;
        ed_fsm = &opened_logic_chan_ed->ed_fsm;
        ret = ed_fsm->op->get_curr_state(ed_fsm);
    }
    return ret;
}

static void h264_encode_hcd_interface_suspend_done(ed_tcb_t *opened_logic_chan_ed)
{
    if(opened_logic_chan_ed != NULL){
        isil_register_node_t    *ed;
        ed = &opened_logic_chan_ed->ed;
        ed->op->suspend_complete_done(ed);   
    }
}

static void h264_encode_hcd_interface_resume_done(ed_tcb_t *opened_logic_chan_ed)
{
    if(opened_logic_chan_ed != NULL){
        isil_register_node_t    *ed;
        ed = &opened_logic_chan_ed->ed;
        ed->op->resume_complete_done(ed);   
    }
}

static void h264_encode_hcd_interface_close_done(ed_tcb_t *opened_logic_chan_ed)
{
    if(opened_logic_chan_ed != NULL){
        isil_register_node_t    *ed;
        ed = &opened_logic_chan_ed->ed;
        ed->op->all_complete_done(ed);   
    }
}

static isil_hcd_interface_operation    h264_encode_hcd_interface_op = {
    .open = h264_encode_hcd_interface_open,
    .close = h264_encode_hcd_interface_close,
    .suspend = h264_encode_hcd_interface_suspend,
    .resume = h264_encode_hcd_interface_resume,
    .ioctl = h264_encode_hcd_interface_ioctl,
    .get_state = h264_encode_hcd_interface_get_state,
    .suspend_done = h264_encode_hcd_interface_suspend_done,
    .resume_done = h264_encode_hcd_interface_resume_done,
    .close_done = h264_encode_hcd_interface_close_done,
};

static void h264_logic_encode_chan_driver_notify_myself(isil_register_node_t *node, void *priv, isil_notify_msg *msg)//chan recv async msg
{
    if((node!=NULL) && (priv!=NULL)){

    }
}

static int  h264_logic_encode_chan_driver_match_id(isil_register_node_t *node, void *priv, unsigned long logic_chan_id)
{
    int ret = ISIL_ERR;
    if((node!=NULL) && (priv!=NULL)){
        isil_h264_logic_encode_chan_t *h264_logic_encode_chan = (isil_h264_logic_encode_chan_t*)priv;
        if(h264_logic_encode_chan->logic_chan_id == logic_chan_id){
            ret = ISIL_OK;
        }
    }
    return ret;
}


static int h264_proc_read(struct seq_file *seq, void *data)
{
    int i = 0, issub = 0;
    isil_chip_t *chip;
    isil_h264_logic_encode_chan_t *h264_main, *base;
    isil_h264_logic_encode_chan_t *h264_sub;
    isil_h264_encode_configuration_t *h264_config;
    //isil_vd_cross_bus_t   *vd_bus;
    isil_h264_phy_video_slot_t *phy_slot;
    isil_h264_logic_video_slot_t	*logic_slot;
    isil_h264_encode_control_t    *encode_control;
    isil_video_frame_tcb_queue_t	*encode_frame_queue;
    isil_video_chan_buf_pool_t    *video_chan_buf_pool;
    isil_h264_encode_feature_t    *encode_feature;
    isil_h264_encode_feature_param_t *running_feature;
    isil_h264_encode_rc_t         *encode_rc;
    isil_h264_encode_rc_config_t  *running_rc;
    isil_vd_orig_buf_info_t *vd_orig_buf;
    isil_ed_fsm_t           *ed_fsm;
    osd_chan_engine_t       *encode_osd_engine;
    isil_register_table_t   *opened_logic_chan_table;

    chip = (isil_chip_t *)seq->private;
    base = chip->h264_master_encode_logic_chan;
    h264_main = chip->h264_master_encode_logic_chan;
    h264_sub  = chip->h264_sub_encode_logic_chan;
    seq_printf(seq, "vd map table:\n");
    seq_printf(seq, "%8s %8s %8s %8s %8s %8s %8s %8s %8s\n", "phy", "logic", "size", "fps", "discard", "stride", "enable", "hor", "ver");
    for(i = 0; i< ISIL_PHY_VJ_CHAN_NUMBER; i++){
        phy_slot = &chip->chip_driver->video_bus.vd_bus_driver->phy_video_slot[i];
        logic_slot = phy_slot->cross_bus_logic_video_slot;
        seq_printf(seq, "%8d %8d %8d %8d %08x %8d %8d %8d %8d\n",
                phy_slot->phy_slot_id, logic_slot->logicSlotId, logic_slot->video_size, logic_slot->slotFps, 
                logic_slot->discardTable, logic_slot->roundTableStride, logic_slot->enableSlot, logic_slot->hor_reverse, 
                logic_slot->ver_reverse);
    }
    {
retry:
        i = 0;
        seq_printf(seq, "%8s %8s %8s %8s %8s", "=====>ch", "phy", "logic", "type", "read_bitstream_mode\n");
        for(h264_main = base; (h264_main - base) < ISIL_PHY_VD_CHAN_NUMBER; h264_main++){
            if(atomic_read(&h264_main->opened_flag) == 0){
                continue;
            }
            seq_printf(seq, "%8d %8d %8d %8s %8s\n",i++, 
                    h264_main->phy_slot_id,
                    h264_main->logic_chan_id,
                    (h264_main->type == ISIL_MASTER_BITSTREAM)?"MAIN":"SUB",
                    (h264_main->read_bitstream_mode == READ_BITSTREAM_BY_DDR)?"DDR":
                    ((h264_main->read_bitstream_mode == READ_BITSTREAM_BY_RAM)?"RAM":"PCI"));
        }
        seq_printf(seq, "%8s %8s %8s %8s %8s %8s %8s\n", "Dterlace", "SMV", "MVA",
                "width", "height", "bitrate", "fps");
        for(h264_main = base; (h264_main - base) < ISIL_PHY_VD_CHAN_NUMBER; h264_main++){
            if(atomic_read(&h264_main->opened_flag) == 0){
                continue;
            }
            encode_feature = &h264_main->encode_property.encode_feature;
            running_feature = &encode_feature->running_feature_param;
            h264_config    = &h264_main->encode_property.running_encode_property;
            seq_printf(seq, "%8d %8d %8d %8d %8d %8d %8d\n", 
                    running_feature->b_enable_deinterlace,
                    h264_config->enableSubmitMotionVector,
                    h264_config->enableMotionVectorAnalyses,
                    h264_config->width,
                    h264_config->height,
                    h264_config->bitrate,
                    h264_config->fps);        
        }
        seq_printf(seq, "%8s %8s %8s %8s %8s %8s %8s %8s\n", "I", "gop",
                "pri", "rcType", "level", "qpi", "qpp", "qpb");
        for(h264_main = base; (h264_main - base) < ISIL_PHY_VD_CHAN_NUMBER; h264_main++){
            if(atomic_read(&h264_main->opened_flag) == 0){
                continue;
            }
            encode_rc      = &h264_main->encode_property.encode_rc;
            running_rc     = &encode_rc->running_rc;
            h264_config    = &h264_main->encode_property.running_encode_property;
            seq_printf(seq, "%8d %8d %8d %8d %8d %8d %8d %8d\n",
                    h264_config->keyFrameIntervals,
                    h264_config->gopIntervals,
                    h264_config->pri,
                    running_rc->e_rc_type,
                    h264_config->imageLevel,
                    running_rc->i_qpi,
                    running_rc->i_qpp,
                    running_rc->i_qpb);
        }
        seq_printf(seq, "encode feature:\n");
        seq_printf(seq, "%8s %8s %8s %8s %8s %8s\n", "deinterlace", "1/2", "1/4", "i_4x4", "skip", "MBDELAY");
        for(h264_main = base; (h264_main - base) < ISIL_PHY_VD_CHAN_NUMBER; h264_main++){
            if(atomic_read(&h264_main->opened_flag) == 0){
                continue;
            }
            encode_feature = &h264_main->encode_property.encode_feature;
            running_feature = &encode_feature->running_feature_param;
            seq_printf(seq, "%8s %8s %8s %8s %8s %8d\n",
                    running_feature->b_enable_deinterlace? "ON":"OFF",
                    running_feature->b_enable_half_pixel? "ON":"OFF",
                    running_feature->b_enable_quarter_pixel? "ON":"OFF",
                    running_feature->b_enable_I_4X4? "ON":"OFF",
                    running_feature->b_enable_skip? "ON":"OFF",
                    running_feature->i_mb_delay_value);
        }
        seq_printf(seq, "phy video slot info:\n");
        seq_printf(seq, "%8s %8s %8s %8s %8s %8s %8s", "slot_id", "bus_id", "logic_id", "size", "checkad", "discard_table", "soft_discard_table\n");
        for(h264_main = base; (h264_main - base) < ISIL_PHY_VD_CHAN_NUMBER; h264_main++){
            if(atomic_read(&h264_main->opened_flag) == 0){
                continue;
            }
            phy_slot = h264_main->master_slot;
            encode_control      = &h264_main->encode_control;
            if(!phy_slot || !phy_slot->cross_bus_logic_video_slot) {
                continue;
            }
            logic_slot = phy_slot->cross_bus_logic_video_slot;
            seq_printf(seq, "%8d%8d%8d%8d%8d  0x%08x       0x%08x\n",
                    phy_slot->phy_slot_id,
                    phy_slot->phy_bus_id,
                    phy_slot->map_logic_id,
                    phy_slot->video_size,
                    atomic_read(&phy_slot->vd_orig_buf.stop_check_ad),
                    logic_slot->discardTable,
                    encode_control->i_software_discard_frame_table);
        }
        seq_printf(seq, "encoder status:\n");
        seq_printf(seq, "%8s %8s %8s %8s %8s %8s %8s %8s %8s %8s %8s %8s %8s %8s %8s\n", "int", "AD_ready", "BlkCnt", "FrmCnt", "FrmQue", "fps", "slice", "cur_stat", "lst_stat", "max_int", "cur_int", "min_int", "enc_ptr", "ad_ptr", "org_cnt");
        for(h264_main = base; (h264_main - base) < ISIL_PHY_VD_CHAN_NUMBER; h264_main++){
            if(atomic_read(&h264_main->opened_flag) == 0){
                continue;
            }
            phy_slot = h264_main->master_slot;
            video_chan_buf_pool = &h264_main->encode_chan_buf_pool;
            encode_frame_queue  = &h264_main->encode_frame_queue;
            encode_control      = &h264_main->encode_control;
            vd_orig_buf         = &phy_slot->vd_orig_buf;
            ed_fsm              = &h264_main->opened_logic_chan_ed.ed_fsm;
            seq_printf(seq, "%8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d %8d\n", 
                    encode_control->i_fpm, encode_control->ad_ready,
                    video_chan_buf_pool->op->get_video_packet_tcb_pool_entry_number(video_chan_buf_pool),
                    video_chan_buf_pool->op->get_video_frame_tcb_pool_entry_number(video_chan_buf_pool),
                    encode_frame_queue->op->get_curr_queue_entry_number(encode_frame_queue),
                    encode_control->i_curr_fps,
                    atomic_read(&encode_control->slice_head_and_ad_status), 
                    ed_fsm->op->get_curr_state(ed_fsm),
                    ed_fsm->op->get_last_state(ed_fsm),
                    encode_control->encode_delta.max_delta,
                    encode_control->encode_delta.cur_delta,
                    encode_control->encode_delta.min_delta,
                    vd_orig_buf->vd_encode_ptr,
                    vd_orig_buf->vd_orig_ad_phy_ptr,
                    4);
        }
        seq_printf(seq, "encoder ptr status:\n");
        seq_printf(seq, "%8s %8s %8s %8s %8s %8s %8s %8s\n", "check_ad_flag", "phy_ad_ptr", "toggle_ad_ptr", "encode_ptr", "toggle_ptr", "ad_ready_map", "encode_over_map", "table_entry");
        for(h264_main = base; (h264_main - base) < ISIL_PHY_VD_CHAN_NUMBER; h264_main++){
            if(atomic_read(&h264_main->opened_flag) == 0){
                continue;
            }
            phy_slot = h264_main->master_slot;
            encode_control      = &h264_main->encode_control;
            vd_orig_buf         = &phy_slot->vd_orig_buf;
            ed_fsm              = &h264_main->opened_logic_chan_ed.ed_fsm;
            opened_logic_chan_table = &phy_slot->opened_logic_chan_table;
            seq_printf(seq, "%8d %8d %8d %8d %8d %8x %8x %d\n", 
                    atomic_read(&vd_orig_buf->stop_check_ad), 
                    vd_orig_buf->vd_orig_ad_phy_ptr,
                    vd_orig_buf->vd_toggle_orig_ad_phy_ptr,
                    vd_orig_buf->vd_encode_ptr,
                    vd_orig_buf->vd_toggle_encode_ptr,
                    vd_orig_buf->logic_chan_ad_ready_map_table,
                    vd_orig_buf->logic_chan_encode_over_map_table,
                    opened_logic_chan_table->op->get_curr_register_node_number(opened_logic_chan_table));
        }
        seq_printf(seq, "osd rectangle status:\n");
        seq_printf(seq, "%8s %8s %8s %8s %8s %8s\n", "phy", "logic", "close", "working", "sync", "status");
        for(h264_main = base; (h264_main - base) < ISIL_PHY_VD_CHAN_NUMBER; h264_main++){
            if(atomic_read(&h264_main->opened_flag) == 0){
                continue;
            }
            encode_osd_engine = &h264_main->encode_osd_engine;
            seq_printf(seq, "%8d %8d %8d %8d %8d %8x\n", h264_main->phy_slot_id, h264_main->logic_chan_id,
                    atomic_read(&encode_osd_engine->osd_encode_start_close),
                    atomic_read(&encode_osd_engine->osd_working),
                    atomic_read(&encode_osd_engine->sync_osd_param),
                    encode_osd_engine->osd_rectangle_need_transfer_data_flag);
        }
        if(issub == 0) {
            issub = 1;
            base = chip->h264_sub_encode_logic_chan;
            goto retry;
        }
    }

    return 0;
}

static ssize_t h264_proc_write(struct file *file, const char __user *buffer, size_t count,  loff_t *offset)
{
    isil_h264_logic_encode_chan_t *h264;
    char cmdbuf[128];
    char **argv;
    int argc, i, ch;
    isil_chip_t *chip;

    chip = (isil_chip_t *)isil_file_get_seq_private(file);
    h264 = chip->h264_master_encode_logic_chan;
    memset(cmdbuf, 0, 128);
    if(count > 128){
        if(copy_from_user(cmdbuf, buffer, 1024) != 0){
            printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);                    
        }
    }
    else{
        if(copy_from_user(cmdbuf, buffer, count) != 0){
            printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);                    
        }
    }
    argv = argv_split(GFP_KERNEL, cmdbuf, &argc);
    for(i = 0; i < (argc); i++)
    {
        ISIL_DBG(ISIL_DBG_INFO, "cmd %d: %s\n", i, argv[i]);
    }
    chip = h264->chip;

    if(strcmp(argv[0], "user_yuv") == 0)
    {
        char *input;
        char *output;

        if(argc < 3) {
            printk("need input output\n");
        }

        h264 += atoi(argv[1]);
        input = argv[2];
        output = argv[3];

        isil_h264_handle_user_q(h264, input, output, -1);
    }
    else if(strcmp(argv[0], "mbdelay") == 0){
        u32 mbdelay;
        isil_h264_encode_feature_t            *feature;
        isil_h264_encode_feature_param_t      *config_feature;

        mbdelay = atoi(argv[1]);

        for(ch = 0; ch < 16; ch++){
            h264 = &chip->h264_master_encode_logic_chan[ch];
            feature = &h264->encode_property.encode_feature;
            config_feature = &feature->config_feature_param;
            config_feature->i_mb_delay_value = mbdelay;
            config_feature->change_mask_flag |= ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_MB_DELAY_MASK;
            h264 = &chip->h264_sub_encode_logic_chan[ch];
            feature = &h264->encode_property.encode_feature;
            config_feature = &feature->config_feature_param;
            config_feature->i_mb_delay_value = mbdelay;
            config_feature->change_mask_flag |= ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_MB_DELAY_MASK;
        }
    }else if(strcmp(argv[0], "ch") == 0){
        ed_tcb_t    *opened_logic_chan_ed;
        ch = atoi(argv[2]);
        h264 += ch;

        //init_isil_h264_encode_chan(h264, 1, 0, ch, 0, chip);
        opened_logic_chan_ed = &h264->opened_logic_chan_ed;
        opened_logic_chan_ed->op->open(opened_logic_chan_ed);
        driver_gen_open_event(opened_logic_chan_ed, 1);
        driver_gen_resume_event(opened_logic_chan_ed, 1);
        isil_h264_save_chn(h264, argv[1]);
    }else if(strcmp(argv[0], "config") == 0){
        isil_h264_encode_feature_param_t encode_feature;
        isil_h264_encode_feature_param_t      *config_feature;
        isil_h264_encode_feature_t            *feature;
        isil_h264_logic_encode_chan_t *base, *logic_chan;
        int ison;

        if(strcmp(argv[1], "master") == 0){
            base = chip->h264_master_encode_logic_chan;
            memcpy(&encode_feature, &default_main_encode_feature, sizeof(isil_h264_encode_feature_param_t));
        }else{
            base = chip->h264_sub_encode_logic_chan;
            memcpy(&encode_feature, &default_sub_encode_feature, sizeof(isil_h264_encode_feature_param_t));
        }
        for(i = 2; i < argc; i++) {
            ison = (strcmp(argv[i], "ON") == 0) || (strcmp(argv[i], "on") == 0);
            switch(i) {
                case 2:encode_feature.b_enable_deinterlace = ison; printk("deinteralce=%s ", argv[2]); break;
                case 3:encode_feature.b_enable_skip = ison; printk("skip=%s ", argv[3]); break;
                case 4:encode_feature.b_enable_I_4X4 = ison; printk("I_4x4=%s ", argv[4]); break;
                case 5:encode_feature.b_enable_half_pixel = ison; printk("1/2pixel=%s ", argv[5]); break;
                case 6:encode_feature.b_enable_quarter_pixel = ison; printk("1/4pixel=%s ", argv[6]); break;
                default:
                       break;
            }
        }
        printk("\n");

        for(logic_chan = base; (logic_chan - base) < ISIL_PHY_VD_CHAN_NUMBER; logic_chan++){
            if(logic_chan && atomic_read(&logic_chan->opened_flag) == 0){
                continue;
            }
            feature = &logic_chan->encode_property.encode_feature;
            config_feature = &feature->config_feature_param;
            config_feature->b_enable_deinterlace = encode_feature.b_enable_deinterlace;
            config_feature->b_enable_skip = encode_feature.b_enable_skip;
            config_feature->b_enable_I_4X4 = encode_feature.b_enable_I_4X4;
            config_feature->b_enable_half_pixel = encode_feature.b_enable_half_pixel;
            config_feature->b_enable_quarter_pixel = encode_feature.b_enable_quarter_pixel;
            config_feature->change_mask_flag = ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_MASK_ALL;
        }
    }else{
        printk("no such cmd!\n");
    }
    argv_free(argv);

    return count;
}

void    reset_isil_h264_encode_chan_preprocessing(ed_tcb_t *opened_logic_chan_ed)
{
    if(opened_logic_chan_ed != NULL){
        isil_h264_logic_encode_chan_t   *h264_logic_encode_chan = to_get_isil_h264_encode_chan_with_opened_logic_chan_ed(opened_logic_chan_ed);
        isil_h264_encode_control_t  *encode_control;
        isil_ed_fsm_t   *ed_fsm;

        printk("%s.%d: %d, %d\n", __FUNCTION__, __LINE__, h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id);
        ed_fsm = &opened_logic_chan_ed->ed_fsm;
        ed_fsm->op->change_state_for_robust(ed_fsm);
        opened_logic_chan_ed->op->suspend(opened_logic_chan_ed);
        h264_logic_encode_chan_gen_close_event_in_thread_context(h264_logic_encode_chan);
        unregister_opened_logic_encode_chan_into_master_slot(h264_logic_encode_chan, h264_logic_encode_chan->master_slot);
        encode_control = &h264_logic_encode_chan->encode_control;
        remove_isil_h264_encode_control(encode_control);
        remove_isil_h264_phy_video_slot(h264_logic_encode_chan->master_slot, h264_logic_encode_chan);
        encode_control->op->reset(encode_control);
        if(atomic_read(&h264_logic_encode_chan->opened_flag) == 0){
            isil_chip_t  *chip;
            chip = h264_logic_encode_chan->chip;
            chip->chip_close(chip);
        }
    }
}

void    reset_isil_h264_encode_chan_postprocessing(ed_tcb_t *opened_logic_chan_ed)
{
    if(opened_logic_chan_ed != NULL){
        isil_h264_logic_encode_chan_t *h264_logic_encode_chan = to_get_isil_h264_encode_chan_with_opened_logic_chan_ed(opened_logic_chan_ed);
        isil_chip_t *chip;

        printk("%s.%d: %d, %d\n", __FUNCTION__, __LINE__, h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id);
        if(atomic_read(&h264_logic_encode_chan->opened_flag) == 0){
            chip =  h264_logic_encode_chan->chip;
            chip->chip_open(chip);
        }
        h264_logic_encode_chan_gen_open_event_in_thread_context(h264_logic_encode_chan);
        h264_logic_encode_chan_gen_resume_event_in_thread_context(h264_logic_encode_chan);
        isil_h264_encode_chan_robust_process_done(h264_logic_encode_chan);
        printk("%s.%d: %d, %d\n", __FUNCTION__, __LINE__, h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id);
    }
}

void    reset_isil_h264_encode_chan_processing(ed_tcb_t *opened_logic_chan_ed)
{
    if(opened_logic_chan_ed != NULL){
        isil_h264_logic_encode_chan_t *h264_logic_encode_chan = to_get_isil_h264_encode_chan_with_opened_logic_chan_ed(opened_logic_chan_ed);
        isil_h264_encode_control_t  *h264_encode_control;
        isil_chip_t *chip;

        printk("%s.%d: %d, %d\n", __FUNCTION__, __LINE__, h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id);
        h264_encode_control = &h264_logic_encode_chan->encode_control;
        h264_encode_control->op->reset(h264_encode_control);
        chip =  h264_logic_encode_chan->chip;
        if(atomic_read(&h264_logic_encode_chan->opened_flag) == 0){
            chip->chip_close(chip);
        }
    }
}

void    start_isil_h264_encode_chan_robust_process(isil_h264_logic_encode_chan_t *h264_logic_encode_chan)
{
    isil_send_msg_controller_t  *send_msg_contr;
    isil_kthread_msg_queue_t    *isil_msg_queue;
    robust_processing_control_t *h264_encode_chan_robust;

    start_chip_robust_process(h264_logic_encode_chan->chip);
    h264_encode_chan_robust = &h264_logic_encode_chan->h264_encode_chan_robust;
    h264_encode_chan_robust->op->start_robust_processing(h264_encode_chan_robust);
    send_msg_contr = &h264_logic_encode_chan->send_msg_contr;
    send_msg_contr->op->delete_send_msg_controller(send_msg_contr);
    get_msg_queue_header(&isil_msg_queue);
    delete_all_msg_of_this_h264_encode_chan(isil_msg_queue, h264_logic_encode_chan);
}

void    isil_h264_encode_chan_robust_process_done(isil_h264_logic_encode_chan_t *h264_logic_encode_chan)
{
    robust_processing_control_t *h264_encode_chan_robust;
    h264_encode_chan_robust = &h264_logic_encode_chan->h264_encode_chan_robust;
    h264_encode_chan_robust->op->robust_processing_done(h264_encode_chan_robust);
}

void    isil_h264_encode_chan_wait_robust_process_done(isil_h264_logic_encode_chan_t *h264_logic_encode_chan)
{
    robust_processing_control_t *h264_encode_chan_robust;

    h264_encode_chan_robust = &h264_logic_encode_chan->h264_encode_chan_robust;
    h264_encode_chan_robust->op->wait_robust_processing_done(h264_encode_chan_robust);
    chip_wait_robust_process_done(h264_logic_encode_chan->chip);
}

int isil_h264_encode_chan_is_in_robust_processing(isil_h264_logic_encode_chan_t *h264_logic_encode_chan)
{
    robust_processing_control_t *h264_encode_chan_robust;
    h264_encode_chan_robust = &h264_logic_encode_chan->h264_encode_chan_robust;
    return h264_encode_chan_robust->op->is_in_robust_processing(h264_encode_chan_robust);
}

int init_isil_h264_encode_chan(isil_h264_logic_encode_chan_t *h264_logic_encode_chan, int bus_id, int chip_id, int phy_slot_id, int type, isil_chip_t *chip)
{
    if((h264_logic_encode_chan!=NULL) && (chip!=NULL)){
        isil_vd_cross_bus_t *chip_vd_cross_bus;
        chip_driver_t       *chip_driver;
        isil_video_bus_t    *video_bus;
        isil_h264_encode_configuration_t    *encode_config, encode_param;
        ed_tcb_t            *opened_logic_chan_ed;
        isil_proc_register_s    *h264_proc;
        robust_processing_control_t *h264_encode_chan_robust;
        unsigned long   buf_size;

        chip_driver = chip->chip_driver;
        video_bus = &chip_driver->video_bus;
        h264_logic_encode_chan->phy_slot_id = phy_slot_id;
        h264_logic_encode_chan->read_bitstream_mode = READ_BITSTREAM_BY_PCI;
        h264_logic_encode_chan->chip = chip;
        h264_logic_encode_chan->isil_device_chan = NULL;
        chip_vd_cross_bus = &chip->chip_vd_cross_bus;

        if(chip_vd_cross_bus->op != NULL){
            chip_vd_cross_bus->op->get_phy_video_slot_by_phy_id(chip_vd_cross_bus, h264_logic_encode_chan->phy_slot_id, &h264_logic_encode_chan->master_slot);
        } else {
            isil_vd_cross_bus_get_phy_video_slot_by_phy_id(chip_vd_cross_bus, h264_logic_encode_chan->phy_slot_id, &h264_logic_encode_chan->master_slot);
        }
        if(type == ISIL_MASTER_BITSTREAM){
            buf_size = VIDEO_MASTER_CHAN_BUF_POOL_LEN;
            h264_logic_encode_chan->type = ISIL_MASTER_BITSTREAM;
            h264_logic_encode_chan->logic_chan_id = ((phy_slot_id<<VIDEO_MASTER_OR_SUB_FLAG_LEFT_SHIFT_NUMBER)|ISIL_MASTER_BITSTREAM);
            //encode_config = &default_h264_master_encode_property;
            encode_config = &encode_param;
            memcpy(encode_config, &default_h264_master_encode_property, sizeof(isil_h264_encode_configuration_t));
            encode_config->fps = (video_bus->op->get_video_standard(video_bus) == ISIL_VIDEO_STANDARD_PAL)? 25:30;
            init_isil_h264_master_encode_property(&h264_logic_encode_chan->encode_property, encode_config);
        } else {
            buf_size = VIDEO_SUB_CHAN_BUF_POOL_LEN;
            h264_logic_encode_chan->type = ISIL_SUB_BITSTREAM;
            h264_logic_encode_chan->logic_chan_id = ((phy_slot_id<<VIDEO_MASTER_OR_SUB_FLAG_LEFT_SHIFT_NUMBER)|ISIL_SUB_BITSTREAM);
            //encode_config = &default_h264_sub_encode_property;
            encode_config = &encode_param;
            memcpy(encode_config, &default_h264_sub_encode_property, sizeof(isil_h264_encode_configuration_t));
            encode_config->fps = (video_bus->op->get_video_standard(video_bus) == ISIL_VIDEO_STANDARD_PAL)? 25:30;
            init_isil_h264_sub_encode_property(&h264_logic_encode_chan->encode_property, encode_config);
        }
        init_endpoint_tcb(&h264_logic_encode_chan->logic_chan_ed, bus_id, chip_id, ISIL_ED_VIDEO_ENCODE_IN, h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id, h264_logic_encode_chan, h264_logic_encode_chan_driver_notify_myself, h264_logic_encode_chan_driver_match_id, &h264_encode_driver_fsm_state_transfer_matrix_table);
        init_endpoint_tcb(&h264_logic_encode_chan->opened_logic_chan_ed, bus_id, chip_id, ISIL_ED_VIDEO_ENCODE_IN, h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id, h264_logic_encode_chan, h264_logic_encode_chan_driver_notify_myself, h264_logic_encode_chan_driver_match_id, &h264_encode_driver_fsm_state_transfer_matrix_table);
        spin_lock_init(&h264_logic_encode_chan->fsm_matrix_call_lock);
        init_isil_send_msg_controller(&h264_logic_encode_chan->send_msg_contr, ISIL_H264_ENCODE_CHAN, h264_logic_encode_chan->logic_chan_id, h264_logic_encode_chan);
        atomic_set(&h264_logic_encode_chan->opened_flag, 0);
        atomic_set(&h264_logic_encode_chan->first_start_flag, 1);
        h264_encode_chan_robust = &h264_logic_encode_chan->h264_encode_chan_robust;
        init_robust_processing_control(h264_encode_chan_robust); 

        init_isil_h264_encode_control(&h264_logic_encode_chan->encode_control, h264_logic_encode_chan->read_bitstream_mode);
        init_rc_driver(h264_logic_encode_chan);
        init_encode_osd_chan_engine(&h264_logic_encode_chan->encode_osd_engine, h264_logic_encode_chan->type);
        init_encode_chan_service_tcb_with_null(&h264_logic_encode_chan->encode_request_tcb);
        init_dpram_request_service_tcb(&h264_logic_encode_chan->read_video_bitstream_req);
        h264_logic_encode_chan->video_bitstream_buf = NULL;

        init_isil_video_chan_buf_pool(&h264_logic_encode_chan->encode_chan_buf_pool, buf_size);
        init_isil_video_frame_tcb_queue(&h264_logic_encode_chan->encode_frame_queue);
#ifdef  MV_MODULE
        init_isil_video_mv_frame_tcb_queue(&h264_logic_encode_chan->encode_mv_frame_queue);
#endif

        register_logic_encode_chan_into_master_slot(h264_logic_encode_chan, h264_logic_encode_chan->master_slot);
        opened_logic_chan_ed = &h264_logic_encode_chan->opened_logic_chan_ed;
        opened_logic_chan_ed->op = &h264_encode_hcd_interface_op;

        h264_proc = &chip->h264_proc;
        strcpy(h264_proc->name, "h264");
        h264_proc->read  = h264_proc_read;
        h264_proc->write = h264_proc_write;
        h264_proc->private = chip;
        isil_module_register(chip, h264_proc);

        return ISIL_OK;
    }
    return ISIL_ERR;
}

void    remove_isil_h264_encode_chan(isil_h264_logic_encode_chan_t *h264_logic_encode_chan)
{
    if(h264_logic_encode_chan!=NULL){
        ed_tcb_t *opened_logic_chan_ed;

        opened_logic_chan_ed = &h264_logic_encode_chan->opened_logic_chan_ed;
        ISIL_DBG(ISIL_DBG_INFO, "%d, %d\n\n", h264_logic_encode_chan->logic_chan_id, h264_logic_encode_chan->master_slot->phy_slot_id);
        if(opened_logic_chan_ed->op != NULL){
            opened_logic_chan_ed->op->close(opened_logic_chan_ed);
        }
        ISIL_DBG(ISIL_DBG_INFO, "%d, %d\n\n", h264_logic_encode_chan->logic_chan_id, h264_logic_encode_chan->master_slot->phy_slot_id);
        remove_isil_video_chan_buf_pool(&h264_logic_encode_chan->encode_chan_buf_pool);
        ISIL_DBG(ISIL_DBG_INFO, "%d, %d\n\n", h264_logic_encode_chan->logic_chan_id, h264_logic_encode_chan->master_slot->phy_slot_id);
        remove_encode_osd_chan_engine(&h264_logic_encode_chan->encode_osd_engine);
        ISIL_DBG(ISIL_DBG_INFO, "%d, %d\n\n", h264_logic_encode_chan->logic_chan_id, h264_logic_encode_chan->master_slot->phy_slot_id);
        unregister_logic_encode_chan_into_master_slot(h264_logic_encode_chan, h264_logic_encode_chan->master_slot);
    }

    isil_module_unregister(h264_logic_encode_chan->chip, &h264_logic_encode_chan->chip->h264_proc);
    ISIL_DBG(ISIL_DBG_INFO, "%d, %d\n\n", h264_logic_encode_chan->logic_chan_id, h264_logic_encode_chan->master_slot->phy_slot_id);
}

void	notify_remove_encode_chan_service(encode_chan_service_tcb_t *curr_consumer)
{
    if(curr_consumer != NULL){
        isil_h264_logic_encode_chan_t *h264_logic_encode_chan = to_get_isil_h264_encode_chan_with_encode_request_tcb(curr_consumer);
        ISIL_DBG(ISIL_DBG_INFO, "%d, %d\n\n", h264_logic_encode_chan->logic_chan_id, h264_logic_encode_chan->master_slot->phy_slot_id);
        remove_isil_h264_encode_chan(h264_logic_encode_chan);
    }
}


void Write_One_Frame_To_DDR_D1_Frame(isil_h264_logic_encode_chan_t *encoder, u16 FrameIdx, u16 channel,u8 *buf, enum ISIL_VIDEO_SIZE video_size, enum ISIL_VIDEO_STANDARD standard);
int isil_h264_handle_user_q(isil_h264_logic_encode_chan_t *logic_chnl, char *name, char *save, int cnt)
{
    struct file *yuv_file = NULL;
    struct file *h264_file = NULL;
    struct inode *inode = NULL;
    struct inode *h264_inode = NULL;
    isil_chip_t *chip;
    mm_segment_t fs; 
    char *yuv_buf = NULL;
    isil_h264_encode_configuration_t *h264_config = NULL;
    ed_tcb_t    *opened_logic_chan_ed;
    isil_h264_encode_control_t    *encode_control;
    isil_h264_phy_video_slot_t    *master_slot;
    isil_vd_orig_buf_info_t   *vd_orig_buf;
    isil_video_frame_tcb_t  *frame;
    isil_video_frame_tcb_queue_t *frame_queue;
    isil_ed_fsm_t     *ed_fsm;
    int size;
    int len, orig_buf_id;
    u8 buf[12];

    if(!logic_chnl || !name|| !save)
    {
        printk("NULL Pointer!\n");
        return -EPERM;
    }

    master_slot = logic_chnl->master_slot;
    if(!master_slot) {
        printk("h264 encode channle not init!\n");
        return -ENODEV;
    }
    vd_orig_buf = &master_slot->vd_orig_buf;
    chip = logic_chnl->chip;
    h264_config = &logic_chnl->encode_property.running_encode_property;
    encode_control = &logic_chnl->encode_control;
    opened_logic_chan_ed = &logic_chnl->opened_logic_chan_ed;
    ed_fsm = &opened_logic_chan_ed->ed_fsm;

    chip->io_op->chip_write32(chip, 0x9004, 0x0);
    opened_logic_chan_ed->op->open(opened_logic_chan_ed);

    master_slot->delete_encode_chan_monitor_hook(master_slot, chip);
    driver_gen_resume_event(opened_logic_chan_ed, 1);

    chip->io_op->chip_write32(chip, 0x9004, 0x0);
    ssleep(1);
    memset(buf, 0, 12);

    yuv_file = filp_open(name, O_RDONLY, 0655);
    if(IS_ERR(yuv_file))
    {
        printk("open file \"%s\" faild, %d\n", name, (int )yuv_file);
        return IS_ERR(yuv_file);
    }
    inode=yuv_file->f_dentry->d_inode;
    h264_file = filp_open(save, O_CREAT | O_RDWR | O_SYNC, 0655);
    if(IS_ERR(h264_file))
    {
        printk("open file \"%s\" faild, %d\n", name, (int )h264_file);
        return IS_ERR(h264_file);
    }

    h264_inode = h264_file->f_dentry->d_inode;

    /*only support YV12, YUV420*/
    //size = h264_config->width * h264_config->height * 3 / 2;
    size = 720 * 480 * 3 / 2;

    yuv_buf = (char*)__get_free_pages(GFP_KERNEL, get_order(size));
    if(!yuv_buf)
    {
        goto q_release;
    }

    if(inode->i_size < size) {
        printk("only support YV12, YCbCr420!\n");
        goto q_release;
    }
    fs=get_fs();
    set_fs(KERNEL_DS);
    orig_buf_id = vd_orig_buf->vd_encode_ptr;
    printk("first sync orig_buf_id = %d, vd_orig_buf->vd_encode_ptr = %d\n", orig_buf_id, vd_orig_buf->vd_encode_ptr);
    while(1){
        if((atomic_read(&vd_orig_buf->stop_check_ad) != 0) || (ed_fsm->op->get_curr_state(ed_fsm) != ISIL_ED_IDLE)){
            schedule();
            if (signal_pending(current)) {
                goto q_release;
            }
            continue;
        }
        len = yuv_file->f_op->read(yuv_file, yuv_buf, size, &(yuv_file->f_pos));
        if(len != size)
        {
            len = yuv_file->f_op->llseek(yuv_file, 0, 0);
            printk("end of file\n");
            //continue;
            goto q_release;
        }
        Write_One_Frame_To_DDR_D1_Frame(logic_chnl, orig_buf_id, 0, yuv_buf, ISIL_VIDEO_SIZE_D1, ISIL_VIDEO_STANDARD_NTSC);
        encode_control->op->update_ad_ready(encode_control);
        if(encode_control->op->get_slice_head_and_ad_status(encode_control)) {
            atomic_set(&vd_orig_buf->stop_check_ad, 1);
            printk("start encode frame serial %08d\n", encode_control->i_frame_serial);
            driver_gen_deliver_event(opened_logic_chan_ed, 1);
        }
#if 1
        /*wait frame ready*/
        frame_queue = &logic_chnl->encode_frame_queue;
        while(!frame_queue->op->get_curr_queue_entry_number(frame_queue)){
            schedule();
            if (signal_pending(current)) {
                goto q_release;
            }
        }
        frame_queue->op->get_curr_consumer_from_queue(frame_queue);
        frame = frame_queue->curr_consumer;
        if(!frame->frame_is_err){
            isil_h264_to_file(h264_file, frame);
        }
        frame_queue->op->release_curr_consumer(frame_queue, &logic_chnl->encode_chan_buf_pool);
#endif
        orig_buf_id++;
        if(orig_buf_id > 3) {
            orig_buf_id = 0;
        }
        if(orig_buf_id != vd_orig_buf->vd_encode_ptr) {
            printk("BUG: orig_buf_id(%d) != vd_orig_buf->vd_encode_ptr(%d)\n", orig_buf_id, vd_orig_buf->vd_encode_ptr);
        }
    }

    set_fs(fs);
q_release:
    if(yuv_buf)
        free_pages((unsigned long)yuv_buf, get_order(size));
    if(!(IS_ERR(yuv_file)))
    {
        filp_close(yuv_file, NULL);
        filp_close(h264_file, NULL);
    } 
    return 0;
}

int isil_h264_save_chn(isil_h264_logic_encode_chan_t *logic_chnl, char *path)
{
    struct file *h264_file;
    //struct file *sub_file;
    isil_video_frame_tcb_t  *frame;
    //isil_video_frame_tcb_t *sub_frame;
    isil_video_frame_tcb_queue_t *frame_queue;
    //isil_video_frame_tcb_queue_t *sub_frame_queue;
    int chnl = logic_chnl->logic_chan_id;
    char name[128];
    //char sub_name[128];

#if 1
    memset(name, 0, 128);
    //memset(sub_name, 0, 128);
    sprintf(name, "%s/video_chn_%02d_%02d.h264", path, chnl, logic_chnl->phy_slot_id);
    //sprintf(sub_name, "%s/sub_video_chn_%02d_%02d.h264", path, chnl, logic_chnl->phy_slot_id);
    h264_file = isil_kernel_file_open(name);
    if(IS_ERR(h264_file)){
        return -EINVAL;
    }
    //sub_file = isil_kernel_file_open(sub_name);
    //if(IS_ERR(sub_file)){
    //    return -EINVAL;
    //}
    frame_queue = &logic_chnl->encode_frame_queue;
    //sub_frame_queue = &logic_chnl->chip->h264_sub_encode_logic_chan[logic_chnl->phy_slot_id].encode_frame_queue;

    while(1){

        //sub_frame_queue->op->try_get_curr_consumer_from_queue(sub_frame_queue);
        //sub_frame_queue->op->release_curr_consumer(sub_frame_queue, &logic_chnl->chip->h264_sub_encode_logic_chan[logic_chnl->phy_slot_id].encode_chan_buf_pool);
        while(frame_queue->op->get_curr_queue_entry_number(frame_queue)){
            frame_queue->op->get_curr_consumer_from_queue(frame_queue);
            frame = frame_queue->curr_consumer;
            if(!frame->frame_is_err){
                isil_h264_to_file(h264_file, frame);
            }
            frame_queue->op->release_curr_consumer(frame_queue, &logic_chnl->encode_chan_buf_pool);
        }

#if 0
        sub_frame_queue->op->get_curr_queue_entry_number(sub_frame_queue);
        while(sub_frame_queue->op->get_curr_queue_entry_number(sub_frame_queue)) {
            sub_frame = sub_frame_queue->curr_consumer;
            if(sub_frame){
                isil_h264_to_file(sub_file, sub_frame);
                sub_frame_queue->op->release_curr_consumer(sub_frame_queue, &logic_chnl->chip->h264_sub_encode_logic_chan[logic_chnl->phy_slot_id].encode_chan_buf_pool);
            }
        }
#endif
        schedule();
        if (signal_pending(current)) {
            break;
        }
    }
    isil_kernel_file_close(h264_file);
    //isil_kernel_file_close(sub_file);
#else
    {
        mm_segment_t fs; 
        struct file *h264_sync_file;
        int len = 0;
        char *buffer = NULL;

        h264_sync_file = isil_kernel_file_open("/dev/video0");
        h264_file      = isil_kernel_file_open("/software/h264_sync.h264");

        buffer = __get_free_pages(GFP_KERNEL, 9);

        fs=get_fs();
        set_fs(KERNEL_DS);
        while(1) {
            len = h264_sync_file->f_op->read(h264_sync_file, buffer, 0x80000, &(h264_sync_file->f_pos));
            if(len > 0) {
                h264_file->f_op->write(h264_file, buffer, len, &(h264_file->f_pos));
            }
            schedule();
            if (signal_pending(current)) {
                break;
            }
        }
        set_fs(fs);
        kfree(buffer);
        isil_kernel_file_close(h264_sync_file);
        isil_kernel_file_close(h264_file);

        return 0;
    }

#endif
    return 0;
}



int isil_h264_to_file(struct file *h264_file, isil_video_frame_tcb_t  *frame)
{
    mm_segment_t fs; 
    isil_video_packet_tcb_t *packet;
    isil_video_packet_tcb_queue_t *packet_queue;
    int len;
    h264_nal_t *nal;
    //int first = 1;

    if(!h264_file || !frame)
    {
        return -EINVAL;
    }

    fs=get_fs();
    nal = &frame->nal;
    set_fs(KERNEL_DS);
    packet_queue = &frame->video_packet_queue;
    while(packet_queue->op->get_curr_queue_entry_number(packet_queue)){
        packet_queue->op->get_curr_consumer_from_queue(packet_queue);
        packet = packet_queue->curr_consumer;
        //if(first == 1) {
        //    len = h264_file->f_op->write(h264_file, packet->data + sizeof(h264_nal_bitstream_t), packet->payload_len - sizeof(h264_nal_bitstream_t), &(h264_file->f_pos));
        //    first = 0;
        //}else{
        len = h264_file->f_op->write(h264_file, packet->data, packet->payload_len, &(h264_file->f_pos));
        //}
        packet_queue->op->release_curr_consumer(packet_queue, frame->video_chan_buf_pool);
    }
    set_fs(fs);

    return 0;
}

#ifdef  MV_MODULE
int isil_mv_to_file(struct file *mv_file, isil_video_mv_frame_tcb_t  *frame)
{
    mm_segment_t fs; 
    isil_video_mv_packet_tcb_t *packet;
    isil_video_mv_packet_tcb_queue_t *packet_queue;
    int len;

    if(!mv_file || !frame)
    {
        return -EINVAL;
    }

    fs=get_fs();
    set_fs(KERNEL_DS);

    packet_queue = &frame->mv_queue;
    while(packet_queue->op->get_curr_queue_entry_number(packet_queue)){
        packet_queue->op->get_curr_consumer_from_queue(packet_queue);
        packet = packet_queue->curr_consumer;
        if(packet){
            len = mv_file->f_op->write(mv_file, packet->data, packet->mvVector_len, &(mv_file->f_pos));
            packet_queue->op->release_curr_consumer(packet_queue, frame->video_chan_buf_pool);
        }
    }

    set_fs(fs);

    return 0;
}
#endif

