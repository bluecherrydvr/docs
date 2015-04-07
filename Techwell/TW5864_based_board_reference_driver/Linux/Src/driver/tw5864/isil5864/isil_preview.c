#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/cdev.h>
#include <linux/ioport.h>
#include <linux/input.h>
#include <asm/system.h>	/* cli(), *_flags */
#include <asm/types.h>
#include <isil5864/isil_common.h>

static const isil_prev_norm_t prev_norms[] = {
    {
        .v4l2_id        = V4L2_STD_PAL,
        .name           = "PAL",
    },{
        .v4l2_id        = V4L2_STD_NTSC,
        .name           = "NTSC",
    }
};
static const unsigned int PREV_NORM_CNT = ARRAY_SIZE(prev_norms);

static isil_prev_param_t default_prev_param_pal = {
    .video_size = ISIL_VIDEO_SIZE_QCIF,
    .fps = 0,
    .discard = 0,
    .stride = 24,
    .cropcap = {
        .bounds = {
            .left = 0,
            .top = 0,
            .width = PAL_D1_WIDTH,
            .height = PAL_D1_HEIGHT,
        },
        .defrect = {
            .left = 0,
            .top = 0,
            .width = PAL_D1_WIDTH,
            .height = PAL_D1_HEIGHT,
        },
    },
    .pix_format = {
        .width = PAL_QCIF_WIDTH,
        .height = PAL_QCIF_HEIGHT,
        .pixelformat =V4L2_PIX_FMT_YUV420,
        .field = V4L2_FIELD_NONE,
        .bytesperline = (PAL_QCIF_WIDTH * PIX_DEPTH) / 8,
        .sizeimage = (PAL_QCIF_HEIGHT * PAL_QCIF_WIDTH * PIX_DEPTH) / 8,
        .colorspace = 0,
        .priv = PIX_DEPTH,
    },
    .rect = {
        .left = 0,
        .top = 0,
        .width = PAL_QCIF_WIDTH,
        .height = PAL_QCIF_HEIGHT,
    },
};

static isil_prev_param_t default_prev_param_ntsc = {
    .video_size = ISIL_VIDEO_SIZE_QCIF,
    .fps = 0,
    .discard = 0,
    .stride = 29,
    .cropcap = {
        .bounds = {
            .left = 0,
            .top = 0,
            .width = NTSC_D1_WIDTH,
            .height = NTSC_D1_HEIGHT,
        },
        .defrect = {
            .left = 0,
            .top = 0,
            .width = NTSC_D1_WIDTH,
            .height = NTSC_D1_HEIGHT,
        },
    },
    .pix_format = {
        .width = NTSC_QCIF_WIDTH,
        .height = NTSC_QCIF_HEIGHT,
        .pixelformat =V4L2_PIX_FMT_YUV420,
        .field = V4L2_FIELD_NONE,
        .bytesperline = (NTSC_QCIF_WIDTH * PIX_DEPTH) / 8,
        .sizeimage = (NTSC_QCIF_HEIGHT * NTSC_QCIF_WIDTH * PIX_DEPTH) / 8,
        .colorspace = 0,
        .priv = PIX_DEPTH,
    },
    .rect = {
        .left = 0,
        .top = 0,
        .width = NTSC_QCIF_WIDTH,
        .height = NTSC_QCIF_HEIGHT,
    },
};

static const u32 g_discard_table_pal[STRIDE_PAL + 2] = {
    0x00000000,0x01000000,0x00800800,0x00808080,0x00820820,
    0x01084210,0x00888888,0x00912244,0x00924924,0x00924a52,
    0x01294a52,0x0152a54a,0x00aaaaaa,0x01555555,0x00ad5ab5,
    0x00d6b5ad,0x016db5ad,0x016db6db,0x016eddbb,0x01777777,
    0x00f7bdef,0x017df7df,0x017f7f7f,0x017ff7ff,0x00ffffff,
    0x01ffffff
};

static const u32 g_discard_table_ntsc[STRIDE_NTSC + 2] = {
    0x00000000,0x20000000,0x20004000,0x20080200,0x20204040,
    0x20820820,0x21084210,0x11108888,0x28888888,0x22489224,
    0x24924924,0x12929292,0x25294a52,0x2952a54a,0x25554aaa,
    0x2aaaaaaa,0x1aaab555,0x16ad5ab5,0x1ad6b5ad,0x2d6d6d6d,
    0x1b6db6db,0x1db76ddb,0x17777777,0x2eef7777,0x1ef7bdef,
    0x1f7df7df,0x1fdfbfbf,0x1ff7fdff,0x1fffbfff,0x1fffffff,
    0x3fffffff
};

static void reset_isil_prev_chan(isil_prev_logic_chan_t *chan);
static void prev_chan_release_vbufs(isil_prev_logic_chan_t *chan);
static int prev_chan_req_vbufs(isil_prev_logic_chan_t *chan, u32 nbuffers);
static void prev_chan_enable(isil_prev_logic_chan_t *chan);
static void prev_chan_disable(isil_prev_logic_chan_t *chan);
static inline u32 fmt_to_img_size(int fmt, int video_std);
static inline u32 fmt_to_buf_size(int fmt);
static int prev_resource_on(isil_prev_logic_chan_t *chan);

static int  prev_overflow_irq_handler(int irq, void *context);
static int  prev_push_irq_handler(int irq, void *context);

static int prev_driver_in_unregister_state_recv_close_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if (ed_fsm != NULL && context != NULL) {
        robust_processing_control_t *robust_processing_control;
        isil_prev_logic_chan_t   *chan;
        isil_chip_t         *chip;
        ed_tcb_t    *ed_tcb;
        isil_vp_bus_t *vp;

        chan = (isil_prev_logic_chan_t *)context;
        chip = chan->chip;
        vp = &chip->chip_vp_bus;
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);

        if(atomic_read(&chan->opened_flag) == 0){
            ISIL_DBG(ISIL_DBG_ERR, "channel %d already closed\n", chan->phy_slot_id);
            return ISIL_ERR;
        }
        robust_processing_control = &chan->robust_processing_control;
        if(robust_processing_control->op->is_in_robust_processing(robust_processing_control)){
            ed_fsm->op->save_event_for_robust_processing(ed_fsm);
        }
        
        //prev_chan_disable(chan);
        //prev_chan_release_vbufs(chan);
        
        if(atomic_dec_and_test(&chan->opened_flag)){
            chan->enable = 0;
            chan->map_refer = 0;
            chan->stream_on = 0;
            prev_chan_disable(chan);
            ISIL_DBG(ISIL_DBG_INFO, "chan%d close!\n", chan->logic_chan_id);
            //chip->chip_close(chip);
            ed_fsm->op->reset(ed_fsm);
            ed_tcb->ed.op->all_complete_done(&ed_tcb->ed);
        }
        return ISIL_OK;
    }
    return ISIL_ERR;
}

static int prev_driver_in_unregister_state_recv_suspend_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if (ed_fsm != NULL && context != NULL) {
        ed_tcb_t    *ed_tcb;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        ed_tcb->ed.op->suspend_complete_done(&ed_tcb->ed);
        
        return ISIL_OK;
    }
    return ISIL_ERR;
}

static int prev_driver_in_unregister_state_recv_resume_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if (ed_fsm != NULL && context != NULL) {
        ed_tcb_t    *ed_tcb;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        ed_tcb->ed.op->resume_complete_done(&ed_tcb->ed);
    
        return ISIL_OK;
    }
    return ISIL_ERR;
}

static int prev_driver_in_unregister_state_recv_open_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if (ed_fsm != NULL && context != NULL) {
        isil_prev_logic_chan_t *chan;
        isil_chip_t         *chip;
        ed_tcb_t    *ed_tcb;
        isil_vp_bus_t *vp;
        robust_processing_control_t *robust_processing_control;

        chan = (isil_prev_logic_chan_t *)context;
        chip = chan->chip;
        vp = &chip->chip_vp_bus;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        if (atomic_read(&chan->opened_flag) == 0) {
            atomic_set(&chan->opened_flag, 1);
            chan->enable = 1;

            reset_isil_prev_chan(chan);
            ed_tcb->ed.op->init_complete(&ed_tcb->ed);
            ed_fsm->op->change_state(ed_fsm, ISIL_ED_SUSPEND);

            robust_processing_control = &chan->robust_processing_control;
            if(robust_processing_control->op->is_in_robust_processing(robust_processing_control)) {
                ed_fsm->op->restore_event_from_robust_processing(ed_fsm);
                if(have_any_pending_event(ed_tcb)) {
                    driver_trigger_pending_event(ed_tcb);
                }
            }
        } else {
            atomic_inc(&chan->opened_flag);
        }
        return ISIL_OK;
    }
    return ISIL_ERR;
}

static int prev_driver_in_standby_state_recv_close_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if (ed_fsm != NULL && context != NULL) {
        ed_tcb_t    *ed_tcb;

        //ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        //driver_gen_close_event(ed_tcb, 0);
        ed_fsm->op->change_state(ed_fsm, ISIL_ED_UNREGISTER);
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_close_event(ed_tcb, 1);

        return ISIL_OK;
    }
    return ISIL_ERR;
}

static int prev_driver_in_standby_state_recv_timeout_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if (ed_fsm != NULL && context != NULL) {
        isil_prev_logic_chan_t   *chan;
        isil_chip_t     *chip;
        chip_driver_t *chip_driver;
        isil_video_bus_t *video_bus;
        isil_vp_bus_t *vp_bus;
        robust_processing_control_t *robust;

        chan = (isil_prev_logic_chan_t *)context;
        chip = chan->chip;
        chip_driver = chip->chip_driver;
        video_bus = &chip_driver->video_bus;
        vp_bus = video_bus->vp_bus_driver;
        
        ISIL_DBG(ISIL_DBG_ERR, "chan%d standby state timeout!\n", chan->logic_chan_id);

        robust = &chip->chip_robust_processing;
        if(!robust->op->is_in_robust_processing(robust)){
            robust->op->start_robust_processing(robust);

            robust = &vp_bus->robust_processing_control;
            if(!robust->op->is_in_robust_processing(robust)){
                robust->op->start_robust_processing(robust);
                robust = &chan->robust_processing_control;
                robust->op->start_robust_processing(robust);
                prev_chan_gen_req_msg(chan, REQ_ALGO_CHIP_RESET, NONBLOCK_OP);
            }
        }
        return ISIL_OK;
    }
    return ISIL_ERR;
}

static int prev_driver_in_standby_state_recv_suspend_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if (ed_fsm != NULL && context != NULL) {
        ed_tcb_t    *ed_tcb;
        isil_prev_logic_chan_t *chan;

        chan = (isil_prev_logic_chan_t *)context;

        ISIL_DBG(ISIL_DBG_INFO, "standby recv suspend, to disable chan%d!\n", chan->logic_chan_id);
        //ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        //driver_gen_suspend_event(ed_tcb, 0);
        ed_fsm->op->change_state(ed_fsm, ISIL_ED_SUSPEND);
        prev_chan_disable(chan);
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        ed_tcb->ed.op->suspend_complete_done(&ed_tcb->ed);

        return ISIL_OK;
    }
    return ISIL_ERR;
}

static int prev_driver_in_standby_state_recv_resume_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if (ed_fsm != NULL && context != NULL) {
        ed_tcb_t    *ed_tcb;

        //ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        //driver_gen_resume_event(ed_tcb, 0);
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        ed_tcb->ed.op->resume_complete_done(&ed_tcb->ed);

        return ISIL_OK;
    }
    return ISIL_ERR;
}

static int prev_driver_in_standby_state_recv_deliver_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if (ed_fsm != NULL && context != NULL) {
        ed_tcb_t    *ed_tcb;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        ed_fsm->op->change_state(ed_fsm, ISIL_ED_TRANSFERING);
        driver_gen_deliver_event(ed_tcb, 1);

        return ISIL_OK;
    }
    return ISIL_ERR;
}

static int prev_driver_in_suspend_state_recv_close_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if (ed_fsm != NULL && context != NULL) {
        ed_tcb_t    *ed_tcb;

        ed_fsm->op->change_state(ed_fsm, ISIL_ED_UNREGISTER);
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_close_event(ed_tcb, 1);

        return ISIL_OK;
    }
    return ISIL_ERR;
}

static int prev_driver_in_suspend_state_recv_suspend_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if (ed_fsm != NULL && context != NULL) {
        ed_tcb_t    *ed_tcb;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        ed_tcb->ed.op->suspend_complete_done(&ed_tcb->ed);

        return ISIL_OK;
    }
    return ISIL_ERR;
}

static int prev_driver_in_suspend_state_recv_resume_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if (ed_fsm != NULL && context != NULL) {
        isil_prev_logic_chan_t *chan;
        ed_tcb_t    *ed_tcb;

        chan = (isil_prev_logic_chan_t *)context;

        if (prev_resource_on(chan)) {
            ISIL_DBG(ISIL_DBG_INFO, "suspend recv resume, to enable chan%d!\n", chan->logic_chan_id);
            ed_fsm->op->change_state(ed_fsm, ISIL_ED_STANDBY);
            prev_chan_enable(chan);
        }
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        ed_tcb->ed.op->resume_complete_done(&ed_tcb->ed);

        return ISIL_OK;
    }
    return ISIL_ERR;
}

static int prev_driver_in_transfering_state_recv_close_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if (ed_fsm != NULL && context != NULL) {
        ed_tcb_t    *ed_tcb;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_close_event(ed_tcb, 0);

        return ISIL_OK;
    }
    return ISIL_ERR;
}

static int prev_driver_in_transfering_state_recv_suspend_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if (ed_fsm != NULL && context != NULL) {
        ed_tcb_t    *ed_tcb;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_suspend_event(ed_tcb, 0);

        return ISIL_OK;
    }
    return ISIL_ERR;
}

static int prev_driver_in_transfering_state_recv_resume_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if (ed_fsm != NULL && context != NULL) {
        ed_tcb_t    *ed_tcb;

        //ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        //driver_gen_resume_event(ed_tcb, 0);
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        ed_tcb->ed.op->resume_complete_done(&ed_tcb->ed);

        return ISIL_OK;
    }
    return ISIL_ERR;
}

static int prev_driver_in_transfering_state_recv_deliver_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if (ed_fsm != NULL && context != NULL) {
        ed_tcb_t    *ed_tcb;

        ed_fsm->op->change_state(ed_fsm, ISIL_ED_DONE);
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        if(have_any_pending_event(ed_tcb))
            driver_trigger_pending_event(ed_tcb);
        else
            driver_gen_deliver_event(ed_tcb, 1);

        return ISIL_OK;
    }
    return ISIL_ERR;
}

static int prev_driver_in_done_state_recv_close_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if (ed_fsm != NULL && context != NULL) {
        ed_tcb_t    *ed_tcb;

        ed_fsm->op->change_state(ed_fsm, ISIL_ED_UNREGISTER);
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_close_event(ed_tcb, 1);

        return ISIL_OK;
    }
    return ISIL_ERR;
}

static int prev_driver_in_done_state_recv_suspend_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if (ed_fsm != NULL && context != NULL) {
        isil_prev_logic_chan_t *chan;
        ed_tcb_t    *ed_tcb;

        chan = (isil_prev_logic_chan_t *)context;

        ISIL_DBG(ISIL_DBG_INFO, "done recv suspend, to disable chan%d!\n", chan->logic_chan_id);

        ed_fsm->op->change_state(ed_fsm, ISIL_ED_SUSPEND);
        prev_chan_disable(chan);
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        ed_tcb->ed.op->suspend_complete_done(&ed_tcb->ed);

        return ISIL_OK;
    }
    return ISIL_ERR;
}

static int prev_driver_in_done_state_recv_resume_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if (ed_fsm != NULL && context != NULL) {
        ed_tcb_t    *ed_tcb;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        ed_tcb->ed.op->resume_complete_done(&ed_tcb->ed);
        return ISIL_OK;
    }
    return ISIL_ERR;
}

static int prev_driver_in_done_state_recv_deliver_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if (ed_fsm != NULL && context != NULL) {
        ed_fsm->op->change_state(ed_fsm, ISIL_ED_STANDBY);
        return ISIL_OK;
    }
    return ISIL_ERR;
}

static fsm_state_transfer_matrix_table_t prev_driver_fsm_state_transfer_matrix_table = {
    .action = {
        {
            prev_driver_in_unregister_state_recv_close_event, 
            no_op,
            prev_driver_in_unregister_state_recv_suspend_event,
            prev_driver_in_unregister_state_recv_resume_event,
            prev_driver_in_unregister_state_recv_open_event,
            no_op
        },
        {
            no_op, 
            no_op,
            no_op,
            no_op,
            no_op,
            no_op 
        },
        {
            prev_driver_in_standby_state_recv_close_event, 
            prev_driver_in_standby_state_recv_timeout_event,
            prev_driver_in_standby_state_recv_suspend_event,
            prev_driver_in_standby_state_recv_resume_event,
            no_op,
            prev_driver_in_standby_state_recv_deliver_event
        },
        {
            prev_driver_in_suspend_state_recv_close_event,
            no_op,
            prev_driver_in_suspend_state_recv_suspend_event,
            prev_driver_in_suspend_state_recv_resume_event,
            no_op,
            no_op
        },
        {
            no_op,
            no_op,
            no_op,
            no_op,
            no_op,
            no_op,
        },
        {
            prev_driver_in_transfering_state_recv_close_event,
            no_op,
            prev_driver_in_transfering_state_recv_suspend_event,
            prev_driver_in_transfering_state_recv_resume_event,
            no_op,
            prev_driver_in_transfering_state_recv_deliver_event
        },
        {
            prev_driver_in_done_state_recv_close_event,
            no_op,
            prev_driver_in_done_state_recv_suspend_event,
            prev_driver_in_done_state_recv_resume_event,
            no_op,
            prev_driver_in_done_state_recv_deliver_event
        },
    },
    .sync_hook = NULL,
    .sync_config_to_running_hook = NULL,
    .sync_running_to_config_hook = NULL,
};

static void reset_isil_prev_chan(isil_prev_logic_chan_t *chan)
{
    isil_chip_t *chip;
    isil_vp_bus_t *vp;
    isil_prev_property_t *prev_property;
    int norm;
    u32 fps;

    if (chan == NULL)
        return;

    chan->timestamp.op->reset(&chan->timestamp);
    chan->serial = 0;
    chan->discard_number = 0;

    prev_property = &chan->prev_property;
    chip = chan->chip;
    vp = &chip->chip_vp_bus;
    norm = vp->op->get_norm(vp);
    if (norm == ISIL_VIDEO_STANDARD_PAL) {
        fps = FULL_FPS_PAL;
        prev_property->op->set_fps(prev_property, fps);
        prev_property->op->set_discard(prev_property, g_discard_table_pal[fps]);
    } else {
        fps = FULL_FPS_NTSC;
        prev_property->op->set_fps(prev_property, fps);
        prev_property->op->set_discard(prev_property, g_discard_table_ntsc[fps]);
    }
}

static inline u32 fmt_to_img_size(int fmt, int video_std)
{
    if (video_std == ISIL_VIDEO_STANDARD_PAL) {
        switch (fmt) 
        {
            case ISIL_VIDEO_SIZE_D1:
                return PAL_D1_SIZE;

            case ISIL_VIDEO_SIZE_CIF:
                return PAL_CIF_SIZE;

            case ISIL_VIDEO_SIZE_QCIF:
                return PAL_QCIF_SIZE;

            default:
                ISIL_DBG(ISIL_DBG_INFO, "invalid format type %d\n", fmt);
                return 0;
        }
    }
    else {
        switch (fmt) 
        {
            case ISIL_VIDEO_SIZE_D1:
                return NTSC_D1_SIZE;

            case ISIL_VIDEO_SIZE_CIF:
                return NTSC_CIF_SIZE;

            case ISIL_VIDEO_SIZE_QCIF:
                return NTSC_QCIF_SIZE;

            default:
                ISIL_DBG(ISIL_DBG_INFO, "prev: invalid format type %d\n", fmt);
                return 0;
        }
    }
}

static inline u32 fmt_to_buf_size(int fmt)
{
    switch (fmt) {
        case ISIL_VIDEO_SIZE_D1:
            return D1_BUF_SIZE;

        case ISIL_VIDEO_SIZE_CIF:
            return CIF_BUF_SIZE;

        case ISIL_VIDEO_SIZE_QCIF:
            return QCIF_BUF_SIZE;

        default:
            ISIL_DBG(ISIL_DBG_INFO, "invalid format type %d\n", fmt);
            return 0;
    }		
}

static inline u32 fmt_to_width(int fmt, int video_std)
{
    if (video_std == ISIL_VIDEO_STANDARD_PAL) {
        switch (fmt) 
        {
            case ISIL_VIDEO_SIZE_D1:
                return PAL_D1_WIDTH;

            case ISIL_VIDEO_SIZE_CIF:
                return PAL_CIF_WIDTH;

            case ISIL_VIDEO_SIZE_QCIF:
                return PAL_QCIF_WIDTH;

            default:
                ISIL_DBG(ISIL_DBG_INFO, "invalid format type %d\n", fmt);
                return 0;
        }
    }
    else {
        switch (fmt) 
        {
            case ISIL_VIDEO_SIZE_D1:
                return NTSC_D1_WIDTH;

            case ISIL_VIDEO_SIZE_CIF:
                return NTSC_CIF_WIDTH;

            case ISIL_VIDEO_SIZE_QCIF:
                return NTSC_QCIF_WIDTH;

            default:
                ISIL_DBG(ISIL_DBG_INFO, "invalid format type %d\n", fmt);
                return 0;
        }
    }	
}

static inline u32 fmt_to_height(int fmt, int video_std)
{
    if (video_std == ISIL_VIDEO_STANDARD_PAL) {
        switch (fmt) 
        {
            case ISIL_VIDEO_SIZE_D1:
                return PAL_D1_HEIGHT;

            case ISIL_VIDEO_SIZE_CIF:
                return PAL_CIF_HEIGHT;

            case ISIL_VIDEO_SIZE_QCIF:
                return PAL_QCIF_HEIGHT;

            default:
                ISIL_DBG(ISIL_DBG_INFO, "invalid format type %d\n", fmt);
                return 0;
        }
    }
    else {
        switch (fmt) 
        {
            case ISIL_VIDEO_SIZE_D1:
                return NTSC_D1_HEIGHT;

            case ISIL_VIDEO_SIZE_CIF:
                return NTSC_CIF_HEIGHT;

            case ISIL_VIDEO_SIZE_QCIF:
                return NTSC_QCIF_HEIGHT;

            default:
                ISIL_DBG(ISIL_DBG_INFO, "invalid format type %d\n", fmt);
                return 0;
        }
    }		
}

static int prev_resource_on(isil_prev_logic_chan_t *chan)
{
    if (!chan->nbuffers)
        return 0;

    if (!chan->stream_on)
        return 0;

    return 1;
}

static u32 prev_discard_to_fps(u32 discard)
{
    u32 fps = 0;

    while (discard) {
        discard &= discard - 1;
        fps++;
    }
    return fps;
}

static u32 prev_fps_to_discard(u32 fps)
{
    int i;
    u32 n = 0x1;
    u32 discard = 0;

    for (i = 0; i < fps; i++) {
        discard |= n;
        n <<= 1;
    } 
    return discard;
}

static void covert_to_yvu420(char *src, char *dst, int fmt, int norm, int app_norm)
{
    char *Y, *U, *V, *temp;
    int width, height;
    int app_width, app_height;
    int i;

    width = fmt_to_width(fmt, norm);
    height= fmt_to_height(fmt, norm);
    app_width = fmt_to_width(fmt, app_norm);
    app_height= fmt_to_height(fmt, app_norm);

    Y = dst;
    if(app_height > height){
        U = dst + (app_width * app_height);
        V = U + ((app_width * app_height) >> 2);
    }else{
        U = dst + (app_width * height);
        V = U + ((app_width * height) >> 2);
    }

    /*Y*/
    temp = src;
    memcpy(Y, temp, (app_width * MIN(app_height, height)));
    if(app_height > height){
        memset(Y + (app_width * height), 0, app_width * (app_height - height));
    }
    temp = src + (width * height);
    for(i = 0; i < ((app_width * MIN(app_height, height)) >> 2); i++){
        *V++ = *temp++;
        *U++ = *temp++;
    }
    if(app_height > height){
        for(i = 0; i < ((app_width * (app_height - height)) >> 2); i++){
            *U++ = 0x80;
            *V++ = 0x80;
        }
    }
}

static void covert_to_yuv420(char *src, char *dst, int fmt, int norm, int app_norm)
{
    char *Y, *U, *V, *temp;
    int width, height;
    int app_width, app_height;
    int i;

    width = fmt_to_width(fmt, norm);
    height= fmt_to_height(fmt, norm);
    app_width = fmt_to_width(fmt, app_norm);
    app_height= fmt_to_height(fmt, app_norm);

    Y = dst;
    if(app_height > height){
        U = dst + (app_width * app_height);
        V = U + ((app_width * app_height) >> 2);
    }else{
        U = dst + (app_width * height);
        V = U + ((app_width * height) >> 2);
    }

    /*Y*/
    temp = src;
    memcpy(Y, temp, (app_width * MIN(app_height, height)));
    if(app_height > height){
        memset(Y + (app_width * height), 0, app_width * (app_height - height));
    }
    temp = src + (width * height);
    for(i = 0; i < ((app_width * MIN(app_height, height)) >> 2); i++){
        *U++ = *temp++;
        *V++ = *temp++;
    }
    if(app_height > height){
        for(i = 0; i < ((app_width * (app_height - height)) >> 2); i++){
            *U++ = 0x80;
            *V++ = 0x80;
        }
    }
}

static void covert_to_raw(char *src, char *dst, int fmt, int norm, int app_norm)
{
    int app_width, app_height;

    app_width = fmt_to_width(fmt, app_norm);
    app_height= fmt_to_height(fmt, app_norm);

    memcpy(src, dst, ((app_width * app_height * 3) >> 1));
}

static void covert_to_nv12(char *src, char *dst, int fmt, int norm, int app_norm)
{
    char *Y, *U, *V, *temp;
    int width, height;
    int app_width, app_height;
    int i, j;

    width = fmt_to_width(fmt, norm);
    height= fmt_to_height(fmt, norm);
    app_width = fmt_to_width(fmt, app_norm);
    app_height= fmt_to_height(fmt, app_norm);

    Y = dst;
    if(app_height > height){
        U = dst + (app_width * app_height);
        V = U + ((app_width * app_height) >> 2);
    }else{
        U = dst + (app_width * height);
        V = U + ((app_width * height) >> 2);
    }

    /*Y*/
    temp = src;
    memcpy(Y, temp, (app_width * MIN(height, app_height)));
    if(app_height > height){
        memset(Y + (app_width * MIN(height, app_height)), 0x00, app_width * (app_height - height));
    }
    temp = src + (width * height);
    for(i = 0; i < (app_width >> 1); i++){
        for(j = 0; j < (app_height >> 1); j++){
            if(app_height <= height){
                *V++ = *temp++;
                *U++ = *temp++;
            }else{
                *V++ = 0x80;
                *U++ = 0x80;
            }
        }
    }
}

static void covert_to_nv21(char *src, char *dst, int fmt, int norm, int app_norm)
{
    char *Y, *U, *V, *temp;
    int width, height;
    int app_width, app_height;

    width = fmt_to_width(fmt, norm);
    height= fmt_to_height(fmt, norm);
    app_width = fmt_to_width(fmt, app_norm);
    app_height= fmt_to_height(fmt, app_norm);

    Y = dst;
    if(app_height > height){
        U = dst + (app_width * app_height);
        V = U + ((app_width * app_height) >> 2);
    }else{
        U = dst + (app_width * height);
        V = U + ((app_width * height) >> 2);
    }

    /*Y*/
    temp = src;
    memcpy(Y, temp, (app_width * MIN(height, app_height)));
    if(app_height > height){
        memset(Y + (app_width * MIN(height, app_height)), 0x00, app_width * (app_height - height));
    }
    temp = src + (width * height);
    memcpy(U, temp, ((app_width * MIN(height, app_height)) >> 1));
    if(app_height > height){
        memset(U + ((app_width * MIN(height, app_height)) >> 1), 0x80, ((app_width * (app_height - height)) >> 1));
    }
}

static int video_std_construct(struct v4l2_standard *vs,
        int id, char *name)
{
    u32 index = vs->index;

    memset(vs, 0, sizeof(struct v4l2_standard));
    vs->index = index;
    vs->id    = id;
    if (id & V4L2_STD_NTSC) {
        vs->frameperiod.numerator = 1;
        vs->frameperiod.denominator = 30;
        vs->framelines = 480;
    } else {
        vs->frameperiod.numerator = 1;
        vs->frameperiod.denominator = 25;
        vs->framelines = 576;
    }
    strlcpy(vs->name,name,sizeof(vs->name));
    return 0;
}

static int need_bus4ext(int d1_cnt, int cif_cnt, int qcif_cnt)
{
    cif_cnt = (cif_cnt & 0x1) ? (cif_cnt + 1) : cif_cnt;

    return (d1_cnt*4 + cif_cnt*2 + qcif_cnt) > 16 ? 1 : 0;
}

static int map_with_bus4ext(vp_map_info_t *map_order, vp_map_info_t *map_modify, 
        int cnt, int *bus4_chan)
{
    /* we start from bus3 means that we map bus by dec order,
     * it's convenient for us, because bus0 probably can't be
     * used for D1 already. */
    int bus_pos = 3;
    int chan_pos = 0;
    int cur_bus_type = ISIL_VIDEO_SIZE_USER;
    int i;

    /* occupy bus0[0+2] for bus4's use directly */
    *bus4_chan = 0x1;
    map_order->map_id = 2;
    map_modify[0 + 2] =  *map_order;
    map_order++;
    cnt--;

    /* if 2nd map_order is also CIF, bus0[0+3] is occupied for bus4 too.*/
    if (map_modify[0 + 2].video_size == ISIL_VIDEO_SIZE_CIF && 
            map_order->video_size == ISIL_VIDEO_SIZE_CIF) {
        *bus4_chan = 0x3;
        map_order->map_id = 3;
        map_modify[0 + 3] =  *map_order;
        map_order++;
        cnt--;		
    }

    for (i = 0; i < cnt; i++, map_order++) {
        if (bus_pos < 0 || chan_pos >= VP_BUS_CHAN_CNT) {
            ISIL_DBG(ISIL_DBG_ERR, "bus or chan pos overflow %d %d\n", bus_pos, chan_pos);
            return ISIL_ERR;
        }

        switch (map_order->video_size) {
            case ISIL_VIDEO_SIZE_D1:
                if (bus_pos == 0)
                    return ISIL_ERR;

                map_order->map_id = bus_pos*VP_BUS_CHAN_CNT;
                map_modify[bus_pos*VP_BUS_CHAN_CNT + 0] =  *map_order;
                bus_pos--;
                chan_pos = 0;
                cur_bus_type = ISIL_VIDEO_SIZE_USER;
                break;

            case ISIL_VIDEO_SIZE_CIF:
                if (cur_bus_type == ISIL_VIDEO_SIZE_USER) {
                    map_order->map_id = bus_pos*VP_BUS_CHAN_CNT;
                    map_modify[bus_pos*VP_BUS_CHAN_CNT + 0] =  *map_order;
                    chan_pos = 1;
                    cur_bus_type = ISIL_VIDEO_SIZE_CIF;
                } else {
                    map_order->map_id = bus_pos*VP_BUS_CHAN_CNT + 1;
                    map_modify[bus_pos*VP_BUS_CHAN_CNT + 1] =  *map_order;
                    bus_pos--;
                    chan_pos = 0;
                    cur_bus_type = ISIL_VIDEO_SIZE_USER;
                }
                break;

            case ISIL_VIDEO_SIZE_QCIF:
            default:
                if (map_order->video_size != ISIL_VIDEO_SIZE_QCIF)
                    ISIL_DBG(ISIL_DBG_ERR, "invalid format type %d\n", map_order->video_size);

                if (cur_bus_type == ISIL_VIDEO_SIZE_USER) {
                    map_order->map_id = bus_pos*VP_BUS_CHAN_CNT;
                    map_modify[bus_pos*VP_BUS_CHAN_CNT + 0] =  *map_order;
                    chan_pos = 1;
                    cur_bus_type = ISIL_VIDEO_SIZE_QCIF;
                } else if (cur_bus_type == ISIL_VIDEO_SIZE_CIF) {
                    bus_pos--;
                    chan_pos = 0;
                    if (bus_pos < 0) {
                        ISIL_DBG(ISIL_DBG_ERR, "bus pos overflow %d\n", bus_pos);
                        return ISIL_ERR;
                    }
                    map_order->map_id = bus_pos*VP_BUS_CHAN_CNT;
                    map_modify[bus_pos*VP_BUS_CHAN_CNT + 0] =  *map_order;
                    chan_pos = 1;
                    cur_bus_type = ISIL_VIDEO_SIZE_QCIF;
                } else {
                    if (bus_pos == 0) {
                        if (chan_pos == 1) {
                            map_order->map_id = bus_pos*VP_BUS_CHAN_CNT + 1;
                            map_modify[bus_pos*VP_BUS_CHAN_CNT + 1] =  *map_order;
                            chan_pos++;
                        } else if (chan_pos == 2) {
                            if (*bus4_chan == 0x1) {
                                map_order->map_id = bus_pos*VP_BUS_CHAN_CNT + 3;
                                map_modify[bus_pos*VP_BUS_CHAN_CNT + 3] =  *map_order;
                                bus_pos--;
                                chan_pos = 0;
                                cur_bus_type = ISIL_VIDEO_SIZE_USER;
                            } else
                                return ISIL_ERR;
                        } else
                            ISIL_DBG(ISIL_DBG_ERR, "impossible situation %d\n", chan_pos);
                    } else {
                        map_order->map_id = bus_pos*VP_BUS_CHAN_CNT + chan_pos;
                        map_modify[bus_pos*VP_BUS_CHAN_CNT + chan_pos] =  *map_order;
                        chan_pos++;
                        if (chan_pos >= VP_BUS_CHAN_CNT) {
                            bus_pos--;
                            chan_pos = 0;
                            cur_bus_type = ISIL_VIDEO_SIZE_USER;
                        }
                    }
                }
                break;
        }	
    }	
    return ISIL_OK;
}

static int map_without_bus4ext(vp_map_info_t *map_order, vp_map_info_t *map_modify, int cnt)
{
    int bus_pos = 0;
    int chan_pos = 0;
    int cur_bus_type = ISIL_VIDEO_SIZE_USER;
    int i;

    for (i = 0; i < cnt; i++, map_order++) {
        if (bus_pos >= VP_BUS_CNT || chan_pos >= VP_BUS_CHAN_CNT) {
            ISIL_DBG(ISIL_DBG_ERR, "bus or chan pos overflow %d %d\n", bus_pos, chan_pos);
            return ISIL_ERR;
        }

        switch (map_order->video_size) {
            case ISIL_VIDEO_SIZE_D1:
                map_order->map_id = bus_pos*VP_BUS_CHAN_CNT;
                map_modify[bus_pos*VP_BUS_CHAN_CNT + 0] =  *map_order;
                bus_pos++;
                chan_pos = 0;
                cur_bus_type = ISIL_VIDEO_SIZE_USER;
                break;

            case ISIL_VIDEO_SIZE_CIF:
                if (cur_bus_type == ISIL_VIDEO_SIZE_USER) {
                    map_order->map_id = bus_pos*VP_BUS_CHAN_CNT;
                    map_modify[bus_pos*VP_BUS_CHAN_CNT + 0] =  *map_order;
                    chan_pos = 1;
                    cur_bus_type = ISIL_VIDEO_SIZE_CIF;
                } else {
                    map_order->map_id = bus_pos*VP_BUS_CHAN_CNT + 1;
                    map_modify[bus_pos*VP_BUS_CHAN_CNT + 1] =  *map_order;
                    bus_pos++;
                    chan_pos = 0;
                    cur_bus_type = ISIL_VIDEO_SIZE_USER;
                }
                break;

            case ISIL_VIDEO_SIZE_QCIF:
            default:
                if (map_order->video_size != ISIL_VIDEO_SIZE_QCIF)
                    ISIL_DBG(ISIL_DBG_ERR, "invalid format type %d\n", map_order->video_size);

                if (cur_bus_type == ISIL_VIDEO_SIZE_USER) {
                    map_order->map_id = bus_pos*VP_BUS_CHAN_CNT;
                    map_modify[bus_pos*VP_BUS_CHAN_CNT + 0] =  *map_order;
                    chan_pos = 1;
                    cur_bus_type = ISIL_VIDEO_SIZE_QCIF;
                } else if (cur_bus_type == ISIL_VIDEO_SIZE_CIF) {
                    bus_pos++;
                    chan_pos = 0;
                    if (bus_pos >= VP_BUS_CNT) {
                        ISIL_DBG(ISIL_DBG_ERR, "bus pos overflow %d\n", bus_pos);
                        return ISIL_ERR;
                    }
                    map_order->map_id = bus_pos*VP_BUS_CHAN_CNT;
                    map_modify[bus_pos*VP_BUS_CHAN_CNT + 0] =  *map_order;
                    chan_pos = 1;
                    cur_bus_type = ISIL_VIDEO_SIZE_QCIF;
                } else {
                    map_order->map_id = bus_pos*VP_BUS_CHAN_CNT + chan_pos;
                    map_modify[bus_pos*VP_BUS_CHAN_CNT + chan_pos] =  *map_order;
                    chan_pos++;
                    if (chan_pos >= VP_BUS_CHAN_CNT) {
                        bus_pos++;
                        chan_pos = 0;
                        cur_bus_type = ISIL_VIDEO_SIZE_USER;
                    }
                }
                break;
        }	
    }
    return ISIL_OK;
}

static int chip_get_norm(isil_chip_t *chip)
{
    chip_driver_t *driver;
    isil_video_bus_t *video_bus;

    //mdelay(100);
    driver = chip->chip_driver;
    video_bus = &driver->video_bus;
    return video_bus->op->get_video_standard(video_bus);
}

static void vp_enable_pci_push(isil_chip_t *chip)
{
    u32 flags;

    /* clear all preview irq flags */
    chip->io_op->chip_write32(chip, PCI_INTR_CHAN_SOURCE, 0xffff);
    /* enable pci side interrupts */
    flags = chip->io_op->chip_read32(chip, PCI_MASTER_INTR_ENABLE_REG);
    chip->io_op->chip_write32(chip, PCI_MASTER_INTR_ENABLE_REG, flags | (BIT_PREV | BIT_PREVOF));

    /* enable preview pci master */
    flags = chip->io_op->chip_read32(chip, PCI_MASTER_CONTROL);
    chip->io_op->chip_write32(chip, PCI_MASTER_CONTROL, flags | (BIT_MASTER | BIT_PREV | BIT_PREVOF));
}

static void vp_disable_pci_push(isil_chip_t *chip)
{
    u32 flags;

    /* disable pci side interrupts */
    flags = chip->io_op->chip_read32(chip, PCI_MASTER_INTR_ENABLE_REG);
    chip->io_op->chip_write32(chip, PCI_MASTER_INTR_ENABLE_REG, flags & ~(BIT_PREV | BIT_PREVOF));
    /* clear all preview irq flags */
    chip->io_op->chip_write32(chip, PCI_INTR_CHAN_SOURCE, 0xffff);

    /* disable preview pci master */
    flags = chip->io_op->chip_read32(chip, PCI_MASTER_CONTROL);
    chip->io_op->chip_write32(chip, PCI_MASTER_CONTROL, flags & ~(BIT_MASTER | BIT_PREV | BIT_PREVOF));
}

static void vp_regs_init(isil_chip_t *chip)
{
    chip->io_op->chip_write32(chip, VP_CHANNEL_EN, 0x0);
    vp_enable_pci_push(chip);
}

static void vp_regs_close(isil_chip_t *chip)
{
    chip->io_op->chip_write32(chip, VP_CHANNEL_EN, 0x0);
    vp_disable_pci_push(chip);

}

static void prev_tasklet_handler(unsigned long data)
{
    int i;
    u32 chan_flag;
    u32 ab_flag;
    isil_chip_t *chip;
    isil_vp_bus_t *vp;
    ed_tcb_t *ed_tcb;
    isil_prev_logic_chan_t *chan;
    isil_prev_frame_t *f;	
    tasklet_priv_t *priv = (tasklet_priv_t *)data;
    void *push_buf[ISIL_PHY_VP_CHAN_NUMBER];
    isil_prev_property_t *prev_property;
    int video_size;
    int norm;
    u32 chan_copy_en;

    if (priv == NULL)
        return;

    chip = priv->chip;
    vp = &chip->chip_vp_bus;
    norm = vp->op->get_norm(vp);

again:
    chan_flag = priv->chan_flag;
    ab_flag = chip->io_op->chip_read32(chip, PCI_PREVIEW_AUDIO_AB);

    priv->chan_flag &= ~chan_flag;

    chan_copy_en = 0;
    for (i = 0; i < ISIL_PHY_VP_CHAN_NUMBER; i++) {
        if (chan_flag & BIT(i)) {
            chan = &chip->prev_chan[i];
            ed_tcb = &chan->opened_logic_chan_ed;
            chan->serial++;
            if (!list_empty(&chan->inqueue)) {
                //ISIL_DBG(ISIL_DBG_INFO, "chan%d get frame%d\n", i, chan->serial);
                chan_copy_en |= BIT(i);
                if (ab_flag & BIT(i))
                    push_buf[i] = chan->pushA_buf;
                else
                    push_buf[i] = chan->pushB_buf;
            } else {
                //ISIL_DBG(ISIL_DBG_INFO, "chan%d lost frame%d\n", i, chan->serial);
                chan->discard_number++;
                driver_gen_deliver_event(ed_tcb, 1);
                wake_up(&chan->wait_frame);				
            }
        }
    }		

    for (i = 0; i < ISIL_PHY_VP_CHAN_NUMBER; i++) {
        if (chan_copy_en & BIT(i)) {
            isil_prev_param_t *config_param;
            struct v4l2_pix_format *pix_format;

            chan = &chip->prev_chan[i];
            ed_tcb = &chan->opened_logic_chan_ed;
            prev_property = &chan->prev_property;
            config_param = &prev_property->config_param;
            pix_format = &config_param->pix_format;
            video_size = prev_property->op->get_video_size(prev_property);

            spin_lock(&chan->queue_lock);
            f = list_entry(chan->inqueue.next, struct isil_prev_frame, frame);
            list_del(chan->inqueue.next);
            chan->inq_cnt--;
            spin_unlock(&chan->queue_lock);

            f->state = F_GRABBING;
            switch(pix_format->pixelformat){
                default:
                    covert_to_raw((char *)push_buf[i], (char *)f->data_virt, video_size, norm, chan->app_norm);
                    break;
                case V4L2_PIX_FMT_YVU420:
                    covert_to_yvu420((char *)push_buf[i], (char *)f->data_virt, video_size, norm, chan->app_norm);
                    break;
                case V4L2_PIX_FMT_YUV420:
                    covert_to_yuv420((char *)push_buf[i], (char *)f->data_virt, video_size, norm, chan->app_norm);
                    break;
                case V4L2_PIX_FMT_NV12:
                    covert_to_nv12((char *)push_buf[i], (char *)f->data_virt, video_size, norm, chan->app_norm);
                    break;
                case V4L2_PIX_FMT_NV21:
                    covert_to_nv21((char *)push_buf[i], (char *)f->data_virt, video_size, norm, chan->app_norm);
                    break;
            }

            do_gettimeofday(&f->buf.timestamp);
            f->buf.bytesused = fmt_to_img_size(video_size, chan->app_norm);
            f->buf.sequence = chan->serial;
            f->state = F_DONE;

            spin_lock(&chan->queue_lock);
            list_add_tail(&f->frame, &chan->outqueue);
            chan->outq_cnt++;
            spin_unlock(&chan->queue_lock);

            /* drive fsm forward */
            driver_gen_deliver_event(ed_tcb, 1);
            wake_up(&chan->wait_frame);
        }
    }

    /* interrupt come again when tasklet's processing, we need to handle with it. */
    priv->chan_flag &= 0xffff;
    if (priv->chan_flag)
        goto again;
}

static int  prev_push_irq_handler(int irq, void *context)
{
    u32 chan_flag;
    isil_chip_t *chip;
    isil_vp_bus_t *vp;

    if ((chip = (isil_chip_t *)context) == NULL)
        return IRQ_HANDLED;

    chan_flag = chip->io_op->chip_read32(chip, PCI_INTR_CHAN_SOURCE);
    chan_flag &= 0xffff;

    //ISIL_DBG(ISIL_DBG_INFO, "preview interrupt.\n");

    /* clear preview eof */
    chip->io_op->chip_write32(chip, PCI_INTR_CHAN_SOURCE, chan_flag);
    chip->io_op->chip_write32(chip, INTERRUPT_FLAGS_EXT, BIT_PREV);

    vp = &chip->chip_vp_bus;
    vp->tasklet_priv.chan_flag |= chan_flag;
    vp->tasklet_priv.chip = chip;
    tasklet_schedule(&vp->prev_tasklet);

    return IRQ_HANDLED;
}

static int  prev_overflow_irq_handler(int irq, void *context)
{
    isil_chip_t *chip;

    if ((chip = (isil_chip_t *)context) == NULL)
        return IRQ_HANDLED;

    //ISIL_DBG(ISIL_DBG_ERR, "preview overflow interrupt.\n");

    /* clear preview overflow eof */
    chip->io_op->chip_write32(chip, INTERRUPT_FLAGS_EXT, BIT_PREVOF);
    /* make hardware re-order frames */
    chip->io_op->chip_write32(chip, VP_CMD_START, 0x1);

    return IRQ_HANDLED;
}

static void prev_chan_empty_framequeues(isil_prev_logic_chan_t *chan)
{
    int i;
    unsigned long flags;

    spin_lock_irqsave(&chan->queue_lock, flags);
    INIT_LIST_HEAD(&chan->inqueue);
    INIT_LIST_HEAD(&chan->outqueue);
    chan->inq_cnt = 0;
    chan->outq_cnt = 0;
    spin_unlock_irqrestore(&chan->queue_lock, flags);

    for (i = 0; i < chan->nbuffers; i++) {
        chan->frame[i].state = F_UNUSED;
        chan->frame[i].buf.bytesused = 0;
    }
}

/*static void prev_chan_requeue_outqueue(isil_prev_logic_chan_t *chan)
{
    isil_prev_frame_t *i;

    list_for_each_entry(i, &chan->outqueue, frame) {
        i->state = F_QUEUED;
        list_add(&i->frame, &chan->inqueue);
    }
    INIT_LIST_HEAD(&chan->outqueue);
}*/

static void prev_chan_queue_unusedframes(isil_prev_logic_chan_t *chan)
{
    u32 i;
    unsigned long flags;

    for (i = 0; i < chan->nbuffers; i++)
        if (chan->frame[i].state == F_UNUSED) {
            chan->frame[i].state = F_QUEUED;
            spin_lock_irqsave(&chan->queue_lock, flags);
            list_add_tail(&chan->frame[i].frame, &chan->inqueue);
            chan->inq_cnt++;
            spin_unlock_irqrestore(&chan->queue_lock, flags);
        }
}

static void prev_chan_release_vbufs(isil_prev_logic_chan_t *chan)
{
    int i;
    isil_chip_t *chip;
    isil_prev_property_t *prev_property;
    int video_size;

    chip = chan->chip;
    prev_property = &chan->prev_property;
    video_size = prev_property->op->get_video_size(prev_property);
    if (chan->pushA_buf) {
        release_push_buf(BUF_TYPE_PREVIEW, video_size, chan->pushA_buf, chip);
        ISIL_DBG(ISIL_DBG_INFO, "chan%d pushA addr=%p releasing.\n", 
                chan->logic_chan_id, chan->pushA_buf);
        chan->pushA_buf = NULL;
    }
    if (chan->pushB_buf) {
        release_push_buf(BUF_TYPE_PREVIEW, video_size, chan->pushB_buf, chip);
        ISIL_DBG(ISIL_DBG_INFO, "chan%d pushB addr=%p releasing.\n", 
                chan->logic_chan_id, chan->pushB_buf);
        chan->pushB_buf = NULL;
    }

    for (i = 0; i < chan->nbuffers; i++) {
        if (chan->frame[i].data_virt) {
            vfree(chan->frame[i].data_virt);
            ISIL_DBG(ISIL_DBG_INFO, "chan%d buf%d, addr=%p releasing.\n", 
                    chan->logic_chan_id, i, chan->frame[i].data_virt);
        }
        chan->frame[i].data_virt = NULL;
    }
    chan->nbuffers = 0;
}

static void prev_chan_release_vbufs_fmt(isil_prev_logic_chan_t *chan, int video_size)
{
    int i;
    isil_chip_t *chip;

    chip = chan->chip;
    if (chan->pushA_buf) {
        release_push_buf(BUF_TYPE_PREVIEW, video_size, chan->pushA_buf, chip);
        ISIL_DBG(ISIL_DBG_INFO, "chan%d pushA addr=%p releasing.\n", 
                chan->logic_chan_id, chan->pushA_buf);
        chan->pushA_buf = NULL;
    }
    if (chan->pushB_buf) {
        release_push_buf(BUF_TYPE_PREVIEW, video_size, chan->pushB_buf, chip);
        ISIL_DBG(ISIL_DBG_INFO, "chan%d pushB addr=%p releasing.\n", 
                chan->logic_chan_id, chan->pushB_buf);
        chan->pushB_buf = NULL;
    }

    for (i = 0; i < chan->nbuffers; i++) {
        if (chan->frame[i].data_virt) {
            vfree(chan->frame[i].data_virt);
            ISIL_DBG(ISIL_DBG_INFO, "chan%d buf%d, addr=%p releasing.\n", 
                    chan->logic_chan_id, i, chan->frame[i].data_virt);
        }
        chan->frame[i].data_virt = NULL;
    }
    chan->nbuffers = 0;
}

static int prev_chan_req_vbufs(isil_prev_logic_chan_t *chan, u32 nbuffers)
{
    isil_prev_property_t *prev_property;
    isil_chip_t *chip;
    isil_vp_bus_t *vp;
    int video_size;
    int norm;
    u32 image_size;
    u32 buf_size;
    u32 i;
    
    prev_property = &chan->prev_property;
    video_size = prev_property->op->get_video_size(prev_property);

    chip = chan->chip;
    vp = &chip->chip_vp_bus;
    norm = chan->app_norm;

    image_size = fmt_to_img_size(video_size, norm);
    buf_size = fmt_to_buf_size(video_size);

    if (!nbuffers)
        return 0;

    if (chan->pushA_buf == NULL)
        if (!(chan->pushA_buf = get_push_buf(BUF_TYPE_PREVIEW, video_size, chip))) {
            ISIL_DBG(ISIL_DBG_ERR, "Error: chan%d can't alloc pushA\n", chan->logic_chan_id);
            goto out;
        }
    if (chan->pushB_buf == NULL)
        if (!(chan->pushB_buf = get_push_buf(BUF_TYPE_PREVIEW, video_size, chip))) {
            ISIL_DBG(ISIL_DBG_ERR, "Error: chan%d can't alloc pushB\n", chan->logic_chan_id);
            goto out;
        }

    chip->io_op->chip_write32(chip, PCI_PREVIEW_BASE_ADDRA + (chan->logic_chan_id << 3), 
            virt_to_phys(chan->pushA_buf));
    chip->io_op->chip_write32(chip, PCI_PREVIEW_BASE_ADDRB + (chan->logic_chan_id << 3), 
            virt_to_phys(chan->pushB_buf));

    for (i = 0; i < nbuffers; i++) {
        if ((chan->frame[i].data_virt = vmalloc_32(buf_size)) == NULL) {
            ISIL_DBG(ISIL_DBG_ERR, "chan%d can't alloc buf%d.\n", chan->logic_chan_id, i);
            goto out;
        }
        chan->frame[i].buf.index = i;
        chan->frame[i].buf.m.offset = i * buf_size;
        chan->frame[i].buf.length = image_size;
        chan->frame[i].buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        chan->frame[i].buf.sequence = 0;
        chan->frame[i].buf.field = V4L2_FIELD_NONE;
        chan->frame[i].buf.memory = V4L2_MEMORY_MMAP;
        chan->frame[i].buf.flags = 0;
        ISIL_DBG(ISIL_DBG_INFO, "chan%d buf%d, addr=%p requested.\n", 
                chan->logic_chan_id, i, chan->frame[i].data_virt);
    }
    chan->nbuffers = nbuffers;
    return 0;
out:
    prev_chan_release_vbufs(chan);
    return -ENOMEM;
}

static void prev_chan_enable(isil_prev_logic_chan_t *chan)
{
    u32 chan_en;
    isil_chip_t *chip;

    chip = chan->chip;

    chan_en = chip->io_op->chip_read32(chip, VP_CHANNEL_EN);
    chip->io_op->chip_write32(chip, VP_CHANNEL_EN, chan_en | BIT(chan->logic_chan_id));
}

static void prev_chan_disable(isil_prev_logic_chan_t *chan)
{
    u32 chan_en;
    isil_chip_t *chip;

    chip = chan->chip;

    chan_en = chip->io_op->chip_read32(chip, VP_CHANNEL_EN);
    chip->io_op->chip_write32(chip, VP_CHANNEL_EN, chan_en & ~ BIT(chan->logic_chan_id));
}

static void prev_chan_vma_open(struct vm_area_struct *vma)
{
    isil_prev_frame_t *f = vma->vm_private_data;
    f->vma_use_count++;
    //ISIL_DBG(ISIL_DBG_INFO, "buf mapped.\n");
}

static void prev_chan_vma_close(struct vm_area_struct *vma)
{
    isil_prev_frame_t *f = vma->vm_private_data;
    f->vma_use_count--;
    //ISIL_DBG(ISIL_DBG_INFO, "buf unmapped.\n");
}

static struct vm_operations_struct prev_chan_vm_ops = {
    .open =  prev_chan_vma_open,
    .close = prev_chan_vma_close,
};

static int prev_chan_mmap(struct file *filp, struct vm_area_struct *vma)
{
    isil_prev_logic_chan_t *chan = video_get_drvdata(video_devdata(filp));
    unsigned long size = vma->vm_end - vma->vm_start;
    unsigned long start = vma->vm_start;
    void *pos;
    u32 i;

    if (down_interruptible(&chan->sem))
        return -ERESTARTSYS;

    if (size != PAGE_ALIGN(chan->frame[0].buf.length)) {
        up(&chan->sem);
        return -EINVAL;
    }

    for (i = 0; i < chan->nbuffers; i++) {
        if ((chan->frame[i].buf.m.offset>>PAGE_SHIFT) == vma->vm_pgoff)
            break;
    }
    if (i == chan->nbuffers) {
        up(&chan->sem);
        return -EINVAL;
    }

    vma->vm_flags |= VM_IO;
    vma->vm_flags |= VM_RESERVED;

    pos = chan->frame[i].data_virt;
    while (size > 0) {
        if (vm_insert_page(vma, start, vmalloc_to_page(pos))) {
            up(&chan->sem);
            return -EAGAIN;
        }
        start += PAGE_SIZE;
        pos += PAGE_SIZE;
        size -= PAGE_SIZE;
    }

    vma->vm_ops = &prev_chan_vm_ops;
    vma->vm_private_data = &chan->frame[i];

    vma->vm_ops->open(vma);

    ISIL_DBG(ISIL_DBG_INFO, "chan%d mmap success!\n", chan->logic_chan_id);
    up(&chan->sem);

    return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29)
static int prev_chan_open(struct file *filp)
#else
static int prev_chan_open(struct inode *inode, struct file *filp)
#endif
{
    isil_prev_logic_chan_t *chan = video_get_drvdata(video_devdata(filp));
    ed_tcb_t *ed_tcb;
    int ret;

    if (down_interruptible(&chan->sem))
        return -ERESTARTSYS;
    
    if (atomic_read(&chan->opened_flag) > 0)
        return -EBUSY;

    ed_tcb = &chan->opened_logic_chan_ed;
    if (ed_tcb->op != NULL) {
        ret = ed_tcb->op->open(ed_tcb);
        up(&chan->sem);
        return ret;
    }
    up(&chan->sem);
    return -EFAULT;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29)
static int prev_chan_release(struct file *filp)
#else
static int prev_chan_release(struct inode *inode, struct file *filp)
#endif
{
    isil_prev_logic_chan_t *chan = video_get_drvdata(video_devdata(filp));
    ed_tcb_t *ed_tcb;
    int ret;

    if (down_interruptible(&chan->sem))
        return -ERESTARTSYS;
    
    ed_tcb = &chan->opened_logic_chan_ed;
    if (ed_tcb->op != NULL) {
        ret = ed_tcb->op->close(ed_tcb);
        up(&chan->sem);
        return ret;
    }
    up(&chan->sem);
    return -EFAULT;
}

static ssize_t prev_chan_read(struct file *filp, char __user *buf, size_t count,
        loff_t *f_pos)
{
    return -EFAULT;
}

static unsigned int prev_chan_poll(struct file *filp, poll_table *wait)
{
    isil_prev_logic_chan_t *chan = video_get_drvdata(video_devdata(filp));
    unsigned int mask = 0;

    if (down_interruptible(&chan->sem))
        return POLLERR;

    if (!list_empty(&chan->outqueue)) {
        mask |= POLLIN | POLLRDNORM;
        goto out;
    }

    if (chan->stream_on == 0) {
        mask |= POLLHUP;
        goto out;
    }

    poll_wait(filp, &chan->wait_frame, wait);

    if (!list_empty(&chan->outqueue))
        mask |= POLLIN | POLLRDNORM;

out:
    up(&chan->sem);
    return mask;
}

static ssize_t prev_chan_write(struct file *filp, const char __user *buf, size_t count,
        loff_t *f_pos)
{
    return -EFAULT;
}

static int prev_chan_vidioc_querycap(isil_prev_logic_chan_t *chan, void __user *arg)
{
    struct v4l2_capability cap = {
        .driver = "prev_chan",
        .version = KERNEL_VERSION(1, 0, 0),
        .capabilities = V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_STREAMING,
    };

    strlcpy(cap.card, chan->v4ldev->name, sizeof(cap.card));

    if (copy_to_user(arg, &cap, sizeof(cap)))
        return -EFAULT;

    return 0;
}

static int prev_chan_vidioc_enuminput(isil_prev_logic_chan_t *chan, void __user *arg)
{
    struct v4l2_input in;
    isil_chip_t *chip;
    isil_vp_bus_t *vp;
    int norm;

    if (copy_from_user(&in, arg, sizeof(in)))
        return -EFAULT;

    if (in.index)
        return -EINVAL;

    memset(&in, 0, sizeof(in));
    strcpy(in.name, "PCI push preview");
    in.type = V4L2_INPUT_TYPE_CAMERA;

    chip = chan->chip;
    vp = &chip->chip_vp_bus;
    norm = vp->op->get_norm(vp);
    in.std = (norm == ISIL_VIDEO_STANDARD_PAL) ? V4L2_STD_PAL : V4L2_STD_NTSC;

    if (copy_to_user(arg, &in, sizeof(in)))
        return -EFAULT;

    return 0;
}

static int prev_chan_vidioc_g_input(isil_prev_logic_chan_t *chan, void __user *arg)
{
    int index = 0;

    if (copy_to_user(arg, &index, sizeof(index)))
        return -EFAULT;

    return 0;
}

static int prev_chan_vidioc_s_input(isil_prev_logic_chan_t *chan, void __user *arg)
{
    int index;

    if (copy_from_user(&index, arg, sizeof(index)))
        return -EFAULT;

    if (index != 0)
        return -EINVAL;

    return 0;
}

static int prev_chan_vidioc_query_ctrl(isil_prev_logic_chan_t *chan, void __user *arg)
{
    return -EINVAL;
}

static int prev_chan_vidioc_g_ctrl(isil_prev_logic_chan_t *chan, void __user *arg)
{
    return -EINVAL;
}

static int prev_chan_vidioc_s_ctrl(isil_prev_logic_chan_t *chan, void __user *arg)
{
    return -EINVAL;
}

static int prev_chan_vidioc_cropcap(isil_prev_logic_chan_t *chan, void __user *arg)
{
    struct v4l2_cropcap cc;
    isil_prev_property_t *prev_property;

    prev_property = &chan->prev_property;
    prev_property->op->get_cropcap(prev_property, &cc);

    cc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    cc.pixelaspect.numerator = 1;
    cc.pixelaspect.denominator = 1;

    if (copy_to_user(arg, &cc, sizeof(cc)))
        return -EFAULT;

    return 0;
}

static int prev_chan_vidioc_g_crop(isil_prev_logic_chan_t *chan, void __user *arg)
{
    struct v4l2_crop crop = {
        .type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
    };
    isil_prev_property_t *prev_property;

    prev_property = &chan->prev_property;
    prev_property->op->get_rect(prev_property, &crop.c);

    if (copy_to_user(arg, &crop, sizeof(crop)))
        return -EFAULT;

    return 0;
}

static int prev_chan_vidioc_s_crop(isil_prev_logic_chan_t *chan, void __user *arg)
{
    struct v4l2_crop crop;
    isil_prev_property_t *prev_property;

    if (copy_from_user(&crop, arg, sizeof(crop)))
        return -EFAULT;

    if (crop.type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
        return -EINVAL;

    /* not support this, just do so */
    prev_property = &chan->prev_property;
    prev_property->op->get_rect(prev_property, &crop.c);

    if (copy_to_user(arg, &crop, sizeof(crop))) 
        return -EFAULT;

    return 0;
}

static int prev_chan_vidioc_enum_fmt(isil_prev_logic_chan_t *chan, void __user *arg)
{
    struct v4l2_fmtdesc fmtd;

    if (copy_from_user(&fmtd, arg, sizeof(fmtd)))
        return -EFAULT;

    switch(fmtd.index) {
        case 0:
            strcpy(fmtd.description, "YUV420");
            fmtd.pixelformat =V4L2_PIX_FMT_YUV420;
            break;
        case 1:
            strcpy(fmtd.description, "YVU420");
            fmtd.pixelformat =V4L2_PIX_FMT_YVU420;
            break;
        case 2:
            strcpy(fmtd.description, "NV12");
            fmtd.pixelformat =V4L2_PIX_FMT_NV12;
            break;
        case 3:
            strcpy(fmtd.description, "NV21");
            fmtd.pixelformat =V4L2_PIX_FMT_NV21;
            break;
        default:
            return -EINVAL;
    } 

    fmtd.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    memset(&fmtd.reserved, 0, sizeof(fmtd.reserved));

    if (copy_to_user(arg, &fmtd, sizeof(fmtd)))
        return -EFAULT;

    return 0;
}

static int prev_chan_vidioc_g_fmt(isil_prev_logic_chan_t *chan, void __user *arg)
{
    struct v4l2_format format;
    isil_prev_property_t *prev_property;

    if (copy_from_user(&format, arg, sizeof(format)))
        return -EFAULT;

    if (format.type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
        return -EINVAL;

    prev_property = &chan->prev_property;
    prev_property->op->get_pix_format(prev_property, &format.fmt.pix);

    if (copy_to_user(arg, &format, sizeof(format)))
        return -EFAULT;

    return 0;
}

static int prev_chan_vidioc_try_s_fmt(isil_prev_logic_chan_t *chan, unsigned int cmd,
        void __user *arg)
{
    struct v4l2_format format;
    struct v4l2_pix_format* pix;
    int video_size, video_size_bak;
    struct v4l2_rect rect;
    isil_prev_property_t *prev_property;
    isil_chip_t *chip;
    isil_vp_bus_t *vp;
    ed_tcb_t *ed_tcb;
    u32 nbuffers;
    int norm;
    int i;

    if (copy_from_user(&format, arg, sizeof(format)))
        return -EFAULT;

    if (format.type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
        return -EINVAL;

    prev_property = &chan->prev_property;
    prev_property->op->get_rect(prev_property, &rect);
    pix = &(format.fmt.pix);

    if (pix->width >= CROP_D1_WIDTH)
        video_size = ISIL_VIDEO_SIZE_D1;
    else if (pix->width >= PAL_CIF_WIDTH)
        video_size = ISIL_VIDEO_SIZE_CIF;
    else
        video_size = ISIL_VIDEO_SIZE_QCIF;

    chip = chan->chip;
    vp = &chip->chip_vp_bus;
    norm = vp->op->get_norm(vp);

    switch(pix->pixelformat){
        case V4L2_PIX_FMT_YUV420:
        case V4L2_PIX_FMT_YVU420:
        case V4L2_PIX_FMT_NV12:
        case V4L2_PIX_FMT_NV21:
            break;
        default:
            ISIL_DBG(ISIL_DBG_ERR, "unsupport video format 0x%08x\n", pix->pixelformat);
            return -EINVAL;
    }
    pix->width = fmt_to_width(video_size, norm);
    pix->height = fmt_to_height(video_size, norm);
    pix->priv = PIX_DEPTH;
    pix->colorspace = 0;
    pix->bytesperline = (pix->width * pix->priv) / 8;
    pix->sizeimage = pix->height * ((pix->width * pix->priv) / 8);
    pix->field = V4L2_FIELD_NONE;
    rect.width = pix->width;
    rect.height = pix->height;

    if (cmd == VIDIOC_TRY_FMT) {
        if (copy_to_user(arg, &format, sizeof(format)))
            return -EFAULT;
        return 0;
    }

    for (i = 0; i < chan->nbuffers; i++)
        if (chan->frame[i].vma_use_count) {
            ISIL_DBG(ISIL_DBG_INFO, "VIDIOC_S_FMT failed. Unmap the buffers first.\n");
            return -EINVAL;
        }

    /* vp is global res, so we must lock it before changing it */
    if (mutex_lock_interruptible(&vp->vp_mutex))
        return -ERESTARTSYS;

    ISIL_DBG(ISIL_DBG_INFO, "chan%d get mutex and ready to bus map.\n", chan->logic_chan_id);
    chan->map_refer = 1;
    
    vp->op->notify_suspend_logic_chan(vp);

    video_size_bak = prev_property->op->get_video_size(prev_property);
    prev_property->op->set_video_size(prev_property, video_size);

    if (vp->op->bus_map(vp, R_FMT_CHANGE)) {
        prev_property->op->set_video_size(prev_property, video_size_bak);
        vp->op->notify_resume_logic_chan(vp);
        mutex_unlock(&vp->vp_mutex);
        return -EINVAL;
    }

    prev_property->op->set_pix_format(prev_property, pix);
    prev_property->op->set_rect(prev_property, &rect);

    ed_tcb = &chan->opened_logic_chan_ed;
    nbuffers = chan->nbuffers;
    prev_chan_release_vbufs_fmt(chan, video_size_bak);
    if (prev_chan_req_vbufs(chan, nbuffers)) {
        vp->op->notify_resume_logic_chan(vp);
        mutex_unlock(&vp->vp_mutex);
        return -EINVAL;
    }
    prev_chan_empty_framequeues(chan);
    prev_chan_queue_unusedframes(chan);

    /* resume all preview chans */
    vp->op->notify_resume_logic_chan(vp);
    mutex_unlock(&vp->vp_mutex);

    if (copy_to_user(arg, &format, sizeof(format))) {
        return -EFAULT;
    }
    return 0;
}

static int prev_chan_vidioc_g_jpegcomp(isil_prev_logic_chan_t *chan, void __user *arg)
{
    return -EINVAL;
}

static int prev_chan_vidioc_s_jpegcomp(isil_prev_logic_chan_t *chan, void __user *arg)
{
    return -EINVAL;
}

static int prev_chan_vidioc_reqbufs(isil_prev_logic_chan_t *chan, void __user *arg)
{
    struct v4l2_requestbuffers rb;
    ed_tcb_t *ed_tcb;
    u32 nbuffers;
    u32 i;

    if (copy_from_user(&rb, arg, sizeof(rb)))
        return -EFAULT;

    if (rb.type != V4L2_BUF_TYPE_VIDEO_CAPTURE ||
            rb.memory != V4L2_MEMORY_MMAP)
        return -EINVAL;

    for (i = 0; i < chan->nbuffers; i++)
        if (chan->frame[i].vma_use_count) {
            ISIL_DBG(ISIL_DBG_ERR, "REQBUFS failed. Previous buffers are still mapped.");
            return -EINVAL;
        }

    ed_tcb = &chan->opened_logic_chan_ed;
    ed_tcb->op->suspend(ed_tcb);

    prev_chan_empty_framequeues(chan);

    prev_chan_release_vbufs(chan);
    if (rb.count) {
        nbuffers = MIN(MAX_BUF_CNT, rb.count);
        if (prev_chan_req_vbufs(chan, nbuffers))
            return -ENOMEM;
        rb.count = nbuffers;
    }

    if (copy_to_user(arg, &rb, sizeof(rb))) {
        prev_chan_release_vbufs(chan);
        return -EFAULT;
    }

    return 0;
}

static int prev_chan_vidioc_querybuf(isil_prev_logic_chan_t *chan, void __user *arg)
{
    struct v4l2_buffer b;

    if (copy_from_user(&b, arg, sizeof(b)))
        return -EFAULT;

    if (b.type != V4L2_BUF_TYPE_VIDEO_CAPTURE ||
            b.index >= chan->nbuffers)
        return -EINVAL;

    memcpy(&b, &chan->frame[b.index].buf, sizeof(b));

    if (chan->frame[b.index].vma_use_count)
        b.flags |= V4L2_BUF_FLAG_MAPPED;

    if (chan->frame[b.index].state == F_DONE)
        b.flags |= V4L2_BUF_FLAG_DONE;
    else if (chan->frame[b.index].state != F_UNUSED)
        b.flags |= V4L2_BUF_FLAG_QUEUED;

    if (copy_to_user(arg, &b, sizeof(b)))
        return -EFAULT;

    return 0;
}

static int prev_chan_vidioc_qbuf(isil_prev_logic_chan_t *chan, void __user *arg)
{
    struct v4l2_buffer b;
    unsigned long flags;

    if (copy_from_user(&b, arg, sizeof(b)))
        return -EFAULT;

    if (b.type != V4L2_BUF_TYPE_VIDEO_CAPTURE ||
            b.index >= chan->nbuffers)
        return -EINVAL;

    if (chan->frame[b.index].state != F_UNUSED) {
        ISIL_DBG(ISIL_DBG_INFO, "frame state is not right!\n");
        return -EINVAL;
    }

    chan->frame[b.index].state = F_QUEUED;

    spin_lock_irqsave(&chan->queue_lock, flags);
    list_add_tail(&chan->frame[b.index].frame, &chan->inqueue);
    chan->inq_cnt++;
    spin_unlock_irqrestore(&chan->queue_lock, flags);

    return 0;
}

static int prev_chan_vidioc_dqbuf(isil_prev_logic_chan_t *chan, void __user *arg)
{
    struct v4l2_buffer b;
    isil_prev_frame_t *f;
    unsigned long flags;
    long timeout;
    //ed_tcb_t *ed_tcb;
    //isil_ed_fsm_t *ed_fsm;
    //int state;

    if (copy_from_user(&b, arg, sizeof(b)))
        return -EFAULT;

    if (b.type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
        return -EINVAL;

    if (list_empty(&chan->outqueue)) {
        /*ed_tcb = &chan->opened_logic_chan_ed;
        ed_fsm = &ed_tcb->ed_fsm;
        state = ed_fsm->op->get_curr_state(ed_fsm);
        if (state == ISIL_ED_UNREGISTER || state == ISIL_ED_SUSPEND)
            return -EINVAL;*/
        if (chan->stream_on == 0) {
            ISIL_DBG(ISIL_DBG_INFO, "chan%d need stream on, then dqbuf\n", chan->logic_chan_id);
            return -EINVAL;
        }

        timeout = wait_event_interruptible_timeout
            (chan->wait_frame, !list_empty(&chan->outqueue), msecs_to_jiffies(100));
        if (timeout < 0) {
            ISIL_DBG(ISIL_DBG_INFO, "DQBUF timeout=%ld.\n", timeout);
            return timeout;
        }
        if (!timeout) {
            ISIL_DBG(ISIL_DBG_INFO, "DQBUF timeout=%ld.\n", timeout);
            return -EIO;
        }
    }

    spin_lock_irqsave(&chan->queue_lock, flags);
    f = list_entry(chan->outqueue.next, struct isil_prev_frame, frame);
    list_del(chan->outqueue.next);
    chan->outq_cnt--;
    spin_unlock_irqrestore(&chan->queue_lock, flags);

    f->state = F_UNUSED;

    memcpy(&b, &f->buf, sizeof(b));
    if (f->vma_use_count)
        b.flags |= V4L2_BUF_FLAG_MAPPED;
    b.flags &= ~V4L2_BUF_FLAG_QUEUED;
    b.flags |= V4L2_BUF_FLAG_DONE;

    if (copy_to_user(arg, &b, sizeof(b)))
        return -EFAULT;

    return 0;
}

static int prev_chan_vidioc_streamon(isil_prev_logic_chan_t *chan, void __user *arg)
{
    int type;
    ed_tcb_t *ed_tcb;
    isil_chip_t *chip;
    isil_vp_bus_t *vp;

    if (copy_from_user(&type, arg, sizeof(type)))
        return -EFAULT;

    if (type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
        return -EINVAL;

    if (list_empty(&chan->inqueue) || chan->inq_cnt < 2) {
        ISIL_DBG(ISIL_DBG_ERR, "inqueue has not enough buffers\n");
        return -EINVAL;
    }

    chip = chan->chip;
    vp = &chip->chip_vp_bus;
    if (mutex_lock_interruptible(&vp->vp_mutex))
        return -ERESTARTSYS;
    chan->stream_on = 1;
    ISIL_DBG(ISIL_DBG_DEBUG, "chan%d get mutex and really to stream on!\n", chan->logic_chan_id);
    ed_tcb = &chan->opened_logic_chan_ed;
    ed_tcb->op->resume(ed_tcb);
    mutex_unlock(&vp->vp_mutex);

    return 0;
}

static int prev_chan_vidioc_streamoff(isil_prev_logic_chan_t *chan, void __user *arg)
{
    int type;
    ed_tcb_t *ed_tcb;
    isil_chip_t *chip;
    isil_vp_bus_t *vp;

    if (copy_from_user(&type, arg, sizeof(type)))
        return -EFAULT;

    if (type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
        return -EINVAL;

    chip = chan->chip;
    vp = &chip->chip_vp_bus;
    if (mutex_lock_interruptible(&vp->vp_mutex))
        return -ERESTARTSYS;
    chan->stream_on = 0;
    ISIL_DBG(ISIL_DBG_DEBUG, "chan%d get mutex and really to stream off!\n", chan->logic_chan_id);
    ed_tcb = &chan->opened_logic_chan_ed;
    ed_tcb->op->suspend(ed_tcb);
    mutex_unlock(&vp->vp_mutex);

    wake_up(&chan->wait_frame);				

    return 0;
}

static int prev_chan_vidioc_g_parm(isil_prev_logic_chan_t *chan, void __user *arg)
{
    struct v4l2_streamparm sp;
    struct v4l2_captureparm *cp;
    isil_prev_property_t *prev_property;
    u32 fps;

    if (copy_from_user(&sp, arg, sizeof(sp)))
        return -EFAULT;
    if (sp.type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
        return -EINVAL;

    prev_property = &chan->prev_property;
    fps = prev_property->op->get_fps(prev_property);

    cp = &sp.parm.capture;
    memset(cp, 0, sizeof(*cp));
    cp->capability = V4L2_CAP_TIMEPERFRAME;
    cp->timeperframe.numerator = 1;
    cp->timeperframe.denominator = fps;

    if (copy_to_user(arg, &sp, sizeof(sp)))
        return -EFAULT;

    return 0;
}

static int prev_chan_vidioc_s_parm(isil_prev_logic_chan_t *chan, void __user *arg)
{
    struct v4l2_streamparm sp;
    struct v4l2_fract *tpf;
    isil_prev_property_t *prev_property;
    isil_chip_t *chip;
    isil_vp_bus_t *vp;
    int norm;
    u32 fps;
    u32 fps_bak;
    u32 max_fps;

    if (copy_from_user(&sp, arg, sizeof(sp)))
        return -EFAULT;
    if (sp.type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
        return -EINVAL;
    if (sp.parm.capture.extendedmode != 0)
        return -EINVAL;

    chip = chan->chip;
    vp = &chip->chip_vp_bus;
    norm = vp->op->get_norm(vp);
    if (norm == ISIL_VIDEO_STANDARD_PAL)
        max_fps = FULL_FPS_PAL;
    else
        max_fps = FULL_FPS_NTSC;

    tpf = &sp.parm.capture.timeperframe;
    /*if (tpf->numerator == 0 || tpf->denominator == 0)
        div = 1;
    else
        div = (tpf->numerator*max_fps)/tpf->denominator;
    if (div == 0)
        div = 1;

    fps = max_fps/div;
    if (fps == 0)
        fps = 1;*/

    if (tpf->numerator == 0 || tpf->denominator == 0)
        fps = max_fps;
    else
        fps = tpf->denominator/tpf->numerator;

    if (fps == 0)
        fps = 1;
    else if (fps > max_fps)
        fps = max_fps;

    if (!chan->map_refer) {
        prev_property = &chan->prev_property;
        prev_property->op->set_fps(prev_property, fps);
        goto done;
    }

    if (mutex_lock_interruptible(&vp->vp_mutex))
        return -ERESTARTSYS;

    ISIL_DBG(ISIL_DBG_INFO, "chan%d get mutex and ready to bus map.\n", chan->logic_chan_id);

    vp->op->notify_suspend_logic_chan(vp);

    prev_property = &chan->prev_property;
    fps_bak = prev_property->op->get_fps(prev_property);
    prev_property->op->set_fps(prev_property, fps);

    if (vp->op->bus_map(vp, R_FPS_CHANGE)) {
        prev_property->op->set_fps(prev_property, fps_bak);
        vp->op->notify_resume_logic_chan(vp);
        mutex_unlock(&vp->vp_mutex);
        return -EINVAL;
    }

    vp->op->notify_resume_logic_chan(vp);
    mutex_unlock(&vp->vp_mutex);

done:
    tpf->numerator = 1;
    tpf->denominator = prev_property->op->get_fps(prev_property);
    if (copy_to_user(arg, &sp, sizeof(sp)))
        return -EFAULT;
    return 0;
}

static int prev_chan_vidioc_g_std(isil_prev_logic_chan_t *chan, void __user *arg)
{
    v4l2_std_id id;
    isil_chip_t *chip;
    isil_vp_bus_t *vp;
    int norm;

    chip = chan->chip;
    vp = &chip->chip_vp_bus;

    norm = vp->op->get_norm(vp);

    id = (norm == ISIL_VIDEO_STANDARD_PAL) 
        ? V4L2_STD_PAL : V4L2_STD_NTSC;

    if (copy_to_user(arg, &id, sizeof(id)))
        return -EFAULT;

    return 0;	
}

static int prev_chan_vidioc_s_std(isil_prev_logic_chan_t *chan, void __user *arg)
{
    v4l2_std_id id;
    isil_chip_t *chip;
    isil_vp_bus_t *vp;

    if (copy_from_user(&id, arg, sizeof(id)))
        return -EFAULT;

    chip = chan->chip;
    vp = &chip->chip_vp_bus;

    if (id == V4L2_STD_PAL)
        chan->app_norm = ISIL_VIDEO_STANDARD_PAL;
    else if (id == V4L2_STD_NTSC)
        chan->app_norm = ISIL_VIDEO_STANDARD_NTSC;
    else
        return -EINVAL;

    return 0;
}

static int prev_chan_vidioc_querystd(isil_prev_logic_chan_t *chan, void __user *arg)
{
    v4l2_std_id id;

    id = V4L2_STD_PAL | V4L2_STD_NTSC;

    if (copy_to_user(arg, &id, sizeof(id)))
        return -EFAULT;

    return 0;	
}

static int prev_chan_vidioc_enumstd(isil_prev_logic_chan_t *chan, void __user *arg)
{
    struct v4l2_standard vs;
    unsigned int index;

    if (copy_from_user(&vs, arg, sizeof(vs)))
        return -EFAULT;

    index = vs.index;

    if (index >= PREV_NORM_CNT)
        return -EINVAL;

    video_std_construct(&vs, prev_norms[index].v4l2_id,
            prev_norms[index].name);

    vs.index = index;

    if (copy_to_user(arg, &vs, sizeof(vs)))
        return -EFAULT;

    return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29)
static long prev_chan_ioctl(struct file *filp,
        unsigned int cmd, unsigned long arg)
#else
static int prev_chan_ioctl(struct inode *inode, struct file *filp,
        unsigned int cmd, unsigned long arg)
#endif
{
    isil_prev_logic_chan_t *chan = video_get_drvdata(video_devdata(filp));
    ed_tcb_t *ed_tcb;
    int ret;

    if (down_interruptible(&chan->sem))
        return -ERESTARTSYS;
    
    ed_tcb = &chan->opened_logic_chan_ed;
    if (ed_tcb->op != NULL) {
        ret = ed_tcb->op->ioctl(ed_tcb, cmd, arg);
        up(&chan->sem);
        return ret;
    }
    up(&chan->sem);
    return -EFAULT;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29)
static const struct v4l2_file_operations prev_chan_fops = {
#else
static const struct file_operations prev_chan_fops = {
#endif
    .owner = THIS_MODULE,
    .read = prev_chan_read,
    .write = prev_chan_write,
    .ioctl = prev_chan_ioctl,
    .open = prev_chan_open,
    .poll = prev_chan_poll,
    .release = prev_chan_release,
    .mmap = prev_chan_mmap,
};

static int isil_prev_property_init(isil_prev_property_t *prev_property, isil_prev_param_t *config_param)
{
    if (prev_property && config_param) {
        memcpy(&prev_property->config_param, config_param, sizeof(*config_param));
        memcpy(&prev_property->running_param, config_param, sizeof(*config_param));
    }
    return 0;
}

static int isil_prev_property_reset(isil_prev_property_t *prev_property, isil_prev_param_t *config_param)
{
    if(prev_property) {
        memcpy(&prev_property->config_param, config_param, sizeof(*config_param));
        memcpy(&prev_property->running_param, config_param, sizeof(*config_param));
    }
    return 0;
}

static int isil_prev_property_get_video_size(isil_prev_property_t *prev_property)
{
    isil_prev_param_t *prev_param;

    if (prev_property == NULL)
        return ISIL_ERR;
        
    prev_param = &prev_property->config_param;

    return prev_param->video_size;
}

static u32 isil_prev_property_get_fps(isil_prev_property_t *prev_property)
{
    isil_prev_param_t *prev_param;

    if (prev_property == NULL)
        return 0;
        
    prev_param = &prev_property->config_param;

    return prev_param->fps;
}

static u32 isil_prev_property_get_discard(isil_prev_property_t *prev_property)
{
    isil_prev_param_t *prev_param;

    if (prev_property == NULL)
        return 0;
        
    prev_param = &prev_property->config_param;

    return prev_param->discard;
}

static u32 isil_prev_property_get_stride(isil_prev_property_t *prev_property)
{
    isil_prev_param_t *prev_param;

    if (prev_property == NULL)
        return 0;
        
    prev_param = &prev_property->config_param;

    return prev_param->stride;
}

static int isil_prev_property_get_cropcap(isil_prev_property_t *prev_property, struct v4l2_cropcap *cropcap)
{
    if(prev_property) {
        isil_prev_param_t *prev_param = &prev_property->config_param;

        memcpy(cropcap, &prev_param->cropcap, sizeof(struct v4l2_cropcap));
        return ISIL_OK;
    }
    return ISIL_ERR;
}

static int isil_prev_property_get_pix_format(isil_prev_property_t *prev_property, struct v4l2_pix_format *pix_format)
{
    if(prev_property) {
        isil_prev_param_t *prev_param = &prev_property->config_param;

        memcpy(pix_format, &prev_param->pix_format, sizeof(struct v4l2_pix_format));
        return ISIL_OK;
    }
    return ISIL_ERR;
}

static int isil_prev_property_get_rect(isil_prev_property_t *prev_property, struct v4l2_rect *rect)
{
    if(prev_property) {
        isil_prev_param_t *prev_param = &prev_property->config_param;

        memcpy(rect, &prev_param->rect, sizeof(struct v4l2_rect));
        return ISIL_OK;
    }
    return ISIL_ERR;
}

static int isil_prev_property_set_video_size(isil_prev_property_t *prev_property, int video_size)
{
    isil_prev_param_t *prev_param;

    if (prev_property == NULL)
        return ISIL_ERR;
        
    prev_param = &prev_property->config_param;
    prev_param->video_size = video_size;

    return ISIL_OK;
}

static int isil_prev_property_set_fps(isil_prev_property_t *prev_property, u32 fps)
{
    isil_prev_param_t *prev_param;

    if (prev_property == NULL)
        return ISIL_ERR;
        
    prev_param = &prev_property->config_param;
    prev_param->fps = fps;
    prev_param->discard = prev_fps_to_discard(fps);

    return ISIL_OK;
}

static int isil_prev_property_set_discard(isil_prev_property_t *prev_property, u32 discard)
{
    isil_prev_param_t *prev_param;

    if (prev_property == NULL)
        return ISIL_ERR;
        
    prev_param = &prev_property->config_param;
    prev_param->discard = discard;
    prev_param->fps = prev_discard_to_fps(discard);

    return ISIL_OK;
}

static int isil_prev_property_set_stride(isil_prev_property_t *prev_property, u32 stride)
{
    isil_prev_param_t *prev_param;

    if (prev_property == NULL)
        return ISIL_ERR;
        
    prev_param = &prev_property->config_param;
    prev_param->stride = stride;

    return ISIL_OK;
}

static int isil_prev_property_set_cropcap(isil_prev_property_t *prev_property, struct v4l2_cropcap *cropcap)
{
    if(prev_property) {
        isil_prev_param_t *prev_param = &prev_property->config_param;
        
        memcpy(&prev_param->cropcap, cropcap, sizeof(struct v4l2_cropcap));
        return ISIL_OK;
    }
    return ISIL_ERR;
}

static int isil_prev_property_set_pix_format(isil_prev_property_t *prev_property, struct v4l2_pix_format *pix_format)
{
    if(prev_property) {
        isil_prev_param_t *prev_param = &prev_property->config_param;
        
        memcpy(&prev_param->pix_format, pix_format, sizeof(struct v4l2_pix_format));
        return ISIL_OK;
    }
    return ISIL_ERR;
}

static int isil_prev_property_set_rect(isil_prev_property_t *prev_property, struct v4l2_rect *rect)
{
    if(prev_property) {
        isil_prev_param_t *prev_param = &prev_property->config_param;
        
        memcpy(&prev_param->rect, rect, sizeof(struct v4l2_rect));
        return ISIL_OK;
    }
    return ISIL_ERR;
}

static struct isil_prev_property_operation prev_property_op = {
    .init   = isil_prev_property_init,
    .reset  = isil_prev_property_reset,

    .get_video_size = isil_prev_property_get_video_size,
    .get_fps = isil_prev_property_get_fps,
    .get_discard = isil_prev_property_get_discard,
    .get_stride = isil_prev_property_get_stride,
    .get_cropcap = isil_prev_property_get_cropcap,
    .get_pix_format = isil_prev_property_get_pix_format,
    .get_rect = isil_prev_property_get_rect,

    .set_video_size = isil_prev_property_set_video_size,
    .set_fps = isil_prev_property_set_fps,
    .set_discard = isil_prev_property_set_discard,
    .set_stride = isil_prev_property_set_stride,
    .set_cropcap = isil_prev_property_set_cropcap,
    .set_pix_format = isil_prev_property_set_pix_format,
    .set_rect = isil_prev_property_set_rect,
};

static void init_prev_property(isil_prev_property_t *prev_property, isil_prev_param_t *config_param)
{
    if (prev_property && config_param) {
        spin_lock_init(&prev_property->lock);
        prev_property->op = &prev_property_op;
        prev_property->op->init(prev_property, config_param);
    }
    return ;
}

static int prev_hcd_interface_open(ed_tcb_t *ed_tcb)
{
    isil_prev_logic_chan_t *chan;
    robust_processing_control_t *robust;
    isil_chip_t *chip;
    
    if (ed_tcb == NULL)
        return ISIL_ERR;

    chan = to_get_prev_channel_with_opened_logic_chan_ed(ed_tcb);
    robust = &chan->robust_processing_control;
    robust->op->wait_robust_processing_done(robust);

    if (atomic_read(&chan->opened_flag) == 0){
        chip = chan->chip;
        chip->chip_open(chip);
    }

    driver_gen_open_event(ed_tcb, 1);

    return ISIL_OK;
}

static int prev_hcd_interface_close(ed_tcb_t *ed_tcb)
{
    isil_prev_logic_chan_t *chan;
    isil_chip_t *chip;
    isil_vp_bus_t *vp;
    robust_processing_control_t *robust;
    isil_prev_property_t *prev_property;
    int norm;

    if (ed_tcb == NULL)
        return ISIL_ERR;

    chan = to_get_prev_channel_with_opened_logic_chan_ed(ed_tcb);
    prev_property = &chan->prev_property;
    chip = chan->chip;
    vp = &chip->chip_vp_bus;
    norm = vp->op->get_norm(vp);

    robust = &chan->robust_processing_control;
    robust->op->wait_robust_processing_done(robust);

    driver_gen_close_event(ed_tcb, 1);
    ed_tcb->ed.op->wait_complete(&ed_tcb->ed);

    prev_chan_empty_framequeues(chan);
    prev_chan_release_vbufs(chan);
    if (norm == ISIL_VIDEO_STANDARD_PAL)
        prev_property->op->reset(prev_property, &default_prev_param_pal);
    else
        prev_property->op->reset(prev_property, &default_prev_param_ntsc);

    if (atomic_read(&chan->opened_flag) == 0) {
        chip = chan->chip;
        chip->chip_close(chip);
    }

    return ISIL_OK;
}

static void prev_hcd_interface_suspend(ed_tcb_t *ed_tcb)
{
    if (ed_tcb == NULL)
        return;

    ed_tcb->ed.op->init_wait_suspend_complete(&ed_tcb->ed);
    driver_gen_suspend_event(ed_tcb, 1);
    ed_tcb->ed.op->wait_suspend_complete(&ed_tcb->ed);
    ed_tcb->ed.op->suspend_complete_done(&ed_tcb->ed);
}
    
static void prev_hcd_interface_resume(ed_tcb_t *ed_tcb)
{
    if (ed_tcb == NULL)
        return;

    ed_tcb->ed.op->init_wait_resume_complete(&ed_tcb->ed);
    driver_gen_resume_event(ed_tcb, 1);
    ed_tcb->ed.op->wait_resume_complete(&ed_tcb->ed);
    ed_tcb->ed.op->resume_complete_done(&ed_tcb->ed);
}

static int prev_hcd_interface_ioctl(ed_tcb_t *ed_tcb, unsigned int cmd, unsigned long arg)
{
    isil_prev_logic_chan_t *chan;
    robust_processing_control_t *robust;

    chan = to_get_prev_channel_with_opened_logic_chan_ed(ed_tcb);
    robust = &chan->robust_processing_control;
    robust->op->wait_robust_processing_done(robust);

    switch (cmd) {
        /* V4L2 ioctl command */
        case VIDIOC_QUERYCAP:
            ISIL_DBG(ISIL_DBG_INFO, "chan%d querycap.\n", chan->logic_chan_id);
            return prev_chan_vidioc_querycap(chan, (void __user *)arg);

        case VIDIOC_ENUMINPUT:
            return prev_chan_vidioc_enuminput(chan, (void __user *)arg);

        case VIDIOC_G_INPUT:
            return prev_chan_vidioc_g_input(chan, (void __user *)arg);

        case VIDIOC_S_INPUT:
            return prev_chan_vidioc_s_input(chan, (void __user *)arg);

        case VIDIOC_QUERYCTRL:
            return prev_chan_vidioc_query_ctrl(chan, (void __user *)arg);

        case VIDIOC_G_CTRL:
            return prev_chan_vidioc_g_ctrl(chan, (void __user *)arg);

        case VIDIOC_S_CTRL:
            return prev_chan_vidioc_s_ctrl(chan, (void __user *)arg);

        case VIDIOC_CROPCAP:
            return prev_chan_vidioc_cropcap(chan, (void __user *)arg);

        case VIDIOC_G_CROP:
            return prev_chan_vidioc_g_crop(chan, (void __user *)arg);

        case VIDIOC_S_CROP:
            return prev_chan_vidioc_s_crop(chan, (void __user *)arg);

        case VIDIOC_ENUM_FMT:
            return prev_chan_vidioc_enum_fmt(chan, (void __user *)arg);

        case VIDIOC_G_FMT:
            return prev_chan_vidioc_g_fmt(chan, (void __user *)arg);

        case VIDIOC_TRY_FMT:
        case VIDIOC_S_FMT:
            ISIL_DBG(ISIL_DBG_INFO, "chan%d set fmt.\n", chan->logic_chan_id);
            return prev_chan_vidioc_try_s_fmt(chan, cmd, (void __user *)arg);

        case VIDIOC_G_JPEGCOMP:
            return prev_chan_vidioc_g_jpegcomp(chan, (void __user *)arg);

        case VIDIOC_S_JPEGCOMP:
            return prev_chan_vidioc_s_jpegcomp(chan, (void __user *)arg);

        case VIDIOC_REQBUFS:
            ISIL_DBG(ISIL_DBG_INFO, "chan%d request buf.\n", chan->logic_chan_id);
            return prev_chan_vidioc_reqbufs(chan, (void __user *)arg);

        case VIDIOC_QUERYBUF:
            return prev_chan_vidioc_querybuf(chan, (void __user *)arg);

        case VIDIOC_QBUF:
            return prev_chan_vidioc_qbuf(chan, (void __user *)arg);

        case VIDIOC_DQBUF:
            return prev_chan_vidioc_dqbuf(chan, (void __user *)arg);

        case VIDIOC_STREAMON:
            ISIL_DBG(ISIL_DBG_INFO, "chan%d stream on.\n", chan->logic_chan_id);
            return prev_chan_vidioc_streamon(chan, (void __user *)arg);

        case VIDIOC_STREAMOFF:
            return prev_chan_vidioc_streamoff(chan, (void __user *)arg);

        case VIDIOC_G_PARM:
            return prev_chan_vidioc_g_parm(chan, (void __user *)arg);

        case VIDIOC_S_PARM:
            ISIL_DBG(ISIL_DBG_INFO, "chan%d set parm.\n", chan->logic_chan_id);
            return prev_chan_vidioc_s_parm(chan, (void __user *)arg);

        case VIDIOC_G_STD:
            return prev_chan_vidioc_g_std(chan, (void __user *)arg);

        case VIDIOC_S_STD:
            return prev_chan_vidioc_s_std(chan, (void __user *)arg);

        case VIDIOC_QUERYSTD:
            return prev_chan_vidioc_querystd(chan, (void __user *)arg);

        case VIDIOC_ENUMSTD:
            return prev_chan_vidioc_enumstd(chan, (void __user *)arg);

        case VIDIOC_QUERYMENU:
            return -EINVAL;

        default:
            return -EINVAL;
    }
}

static int prev_hcd_interface_get_state(ed_tcb_t *ed_tcb)
{
    int	ret = ISIL_ED_UNREGISTER;

    if(ed_tcb != NULL){
        ret = atomic_read(&ed_tcb->state);
    }
    return ret;
}

struct hcd_interface_operation	prev_hcd_interface_op = {
    .open = prev_hcd_interface_open,
    .close = prev_hcd_interface_close,
    .suspend = prev_hcd_interface_suspend,
    .resume = prev_hcd_interface_resume,
    .ioctl = prev_hcd_interface_ioctl,
    .get_state = prev_hcd_interface_get_state,
};

static void prev_chan_driver_notify(isil_register_node_t *node, void *priv, isil_notify_msg *msg)
{
    if((node!=NULL) && (priv!=NULL)){

    }
}

static int prev_chan_driver_match_id(isil_register_node_t *node, void *priv, unsigned long logic_chan_id)
{
    int	ret = ISIL_ERR;
    if((node!=NULL) && (priv!=NULL)){
        isil_prev_logic_chan_t *prev_logic = (isil_prev_logic_chan_t*)priv;
        if(prev_logic->logic_chan_id == logic_chan_id){
            ret = ISIL_OK;
        }
    }
    return ret;
}

static void debug_show_serial(isil_chip_t *chip)
{   
    int i;
    isil_prev_logic_chan_t *chan;

    ISIL_DBG(ISIL_DBG_INFO, "%8s %8s %8s\n", "chan", "serial", "discard");
    for (i = 0; i < ISIL_PHY_VP_CHAN_NUMBER; i++) {
        chan = &chip->prev_chan[i];
        ISIL_DBG(ISIL_DBG_INFO, "%8d %8d %8d\n", chan->logic_chan_id, chan->serial, chan->discard_number);
    }
}

static int preview_proc_read(struct seq_file *seq, void *data)
{
    int i;
    isil_prev_logic_chan_t *chan;
    isil_prev_property_t *prev_property;
    isil_prev_param_t *prev_param;
    struct v4l2_pix_format *pix_format;
    isil_chip_t *chip = (isil_chip_t *)seq->private;

    seq_printf(seq, "preview channel info:\n");
    seq_printf(seq, "%8s %8s %8s %8s %8s %8s %8s\n", "chan", "serial", "discard", "norm", "nbuffer", "inq_cnt", "outq_cnt");
    for (i = 0; i < ISIL_PHY_VP_CHAN_NUMBER; i++) {
        chan = &chip->prev_chan[i];
        if(atomic_read(&chan->opened_flag)){
            seq_printf(seq, "%8d %8d %8d %8d %8d %8d %8d\n", chan->logic_chan_id, chan->serial, chan->discard_number, chan->app_norm,
                    chan->nbuffers, chan->inq_cnt, chan->outq_cnt);
            prev_property = &chan->prev_property;
            prev_param = &prev_property->running_param;
        }
    }

    seq_printf(seq, "preview channel property:\n");
    seq_printf(seq, "%8s %8s %8s %8s %8s %8s %8s %8s\n", "chan", "vsize", "fps", "discard", "stride", "width", "height", "pixfmt");
    for (i = 0; i < ISIL_PHY_VP_CHAN_NUMBER; i++) {
        chan = &chip->prev_chan[i];
        if(atomic_read(&chan->opened_flag)){
            u8 pix_id[5];

            prev_property = &chan->prev_property;
            prev_param = &prev_property->config_param;
            pix_format = &prev_param->pix_format;
            pix_id[0] = ((pix_format->pixelformat >> 0) & 0xff);
            pix_id[1] = ((pix_format->pixelformat >> 8) & 0xff);
            pix_id[2] = ((pix_format->pixelformat >> 16) & 0xff);
            pix_id[3] = ((pix_format->pixelformat >> 24) & 0xff);
            pix_id[4] = 0;
            seq_printf(seq, "%8d %8d %8d %08x %8d %8d %8d %8s\n", chan->logic_chan_id, prev_param->video_size, prev_param->fps,
                    prev_param->discard, prev_param->stride, pix_format->width, pix_format->height, pix_id);
        }
    }

    return 0;
}

static ssize_t preview_proc_write(struct file *file, const char __user *buffer,
        size_t count, loff_t *offset)
{
    char cmdbuf[256];
    char **argv;
    int argc, i;
    isil_chip_t *chip;
    u32 ret;
    isil_vp_bus_t *vp;

    chip = (isil_chip_t *)isil_file_get_seq_private(file);
    vp = &chip->chip_vp_bus;

    memset(cmdbuf, 0, 256);
    if (count > 256){
        ret = copy_from_user(cmdbuf, buffer, 256);
    }
    else{
        ret = copy_from_user(cmdbuf, buffer, count);
    }

    argv = argv_split(GFP_KERNEL, cmdbuf, &argc);
    if (argc < 2)
        goto out;

    for (i = 0; i < argc; i++) {
        ISIL_DBG(ISIL_DBG_INFO, "cmd %d: %s\n", i, argv[i]);
    }

    if (strstr(argv[0], "show") != NULL) {
        if (strstr(argv[1], "map") != NULL) {
            //debug_show_map(vp);
        }
        else if (strstr(argv[1], "parm") != NULL) {
            //debug_show_parm(chip, atoi(argv[2]));
        }
        else if (strstr(argv[1], "serial") != NULL) {
            debug_show_serial(chip);
        }
        else {
            ISIL_DBG(ISIL_DBG_INFO, "no such parameter!\n");
        }
    }
    else {
        ISIL_DBG(ISIL_DBG_INFO, "no such cmd!\n");
    }

out:
    argv_free(argv);

    return count;
}

static void init_prev_frame_queue(isil_prev_logic_chan_t *chan)
{
    chan->nbuffers = 0;
    
    INIT_LIST_HEAD(&chan->inqueue);
    INIT_LIST_HEAD(&chan->outqueue);
    chan->inq_cnt = 0;
    chan->outq_cnt = 0;

    memset(&chan->frame[0], 0, sizeof(isil_prev_frame_t) * MAX_BUF_CNT);
    
    chan->pushA_buf = NULL;
    chan->pushB_buf = NULL;
    
    spin_lock_init(&chan->queue_lock);
    init_waitqueue_head(&chan->wait_frame);
}

static int init_isil_prev_chan(isil_prev_logic_chan_t *chan, int bus_id, int chip_id, int phy_slot_id, isil_chip_t *chip)
{
    ed_tcb_t *ed_tcb;
    isil_proc_register_s *preview_proc;
    isil_vp_bus_t *vp;
    
    if (chan && chip && phy_slot_id < ISIL_PHY_VP_CHAN_NUMBER) {
        vp = &chip->chip_vp_bus;
        chan->phy_slot_id = phy_slot_id;
        chan->logic_chan_id = phy_slot_id;
        chan->chip = chip;
        atomic_set(&chan->opened_flag, 0);
        sema_init(&chan->sem, 1);
        chan->serial = 0;
        chan->discard_number = 0;
        chan->enable = 0;
        chan->map_refer = 0;
        chan->stream_on = 0;
        chan->app_norm = vp->op->get_norm(vp);
        /* init frame queue */
        init_prev_frame_queue(chan);

        ed_tcb = &chan->opened_logic_chan_ed;
        init_robust_processing_control(&chan->robust_processing_control);
        init_isil_encode_timestamp(&chan->timestamp);

        init_isil_send_msg_controller(&chan->send_msg_contr, ISIL_VIDEO_PREVIEW_CHAN, 
                chan->logic_chan_id, chan);
        /*init parameter*/
        init_prev_property(&chan->prev_property, &default_prev_param_pal);
        /*node TCB*/
        init_endpoint_tcb(&chan->opened_logic_chan_ed, bus_id, chip_id, 
                ISIL_ED_VIDEO_PREVIEW_IN, chan->phy_slot_id, 
                chan->logic_chan_id, chan, prev_chan_driver_notify, 
                prev_chan_driver_match_id, &prev_driver_fsm_state_transfer_matrix_table);
        /*regist HCD operation*/
        ed_tcb->op = &prev_hcd_interface_op;

        preview_proc = &chip->preview_proc;
        strcpy(preview_proc->name, "preview");
        preview_proc->read  = preview_proc_read;
        preview_proc->write = preview_proc_write;
        preview_proc->private = chip;
        isil_module_register(chip, preview_proc);

        return 0;
    }
    return -EINVAL;
}

void reset_isil_prev_chan_preprocessing(ed_tcb_t *ed_tcb)
{
    if (ed_tcb && ed_tcb->op) {
        isil_ed_fsm_t   *ed_fsm;
        isil_prev_logic_chan_t *chan;
        robust_processing_control_t *robust;

        chan = to_get_prev_channel_with_opened_logic_chan_ed(ed_tcb);
        robust = &chan->robust_processing_control;
        if (!robust->op->is_in_robust_processing(robust))
            robust->op->start_robust_processing(robust);

        ed_fsm = &ed_tcb->ed_fsm;
        ed_fsm->op->change_state_for_robust(ed_fsm);

        /* for preview, no need to suspend chan for robust, 
         * as vp_bus_reset will do this implicitly. */
        //ed_tcb->op->suspend(ed_tcb);
        
        /* we can't close chan as other modules, cause app won't request 
         * buffer or mmap again */
        //driver_gen_close_event(ed_tcb, 1);
    }
}

void reset_isil_prev_chan_processing(ed_tcb_t *ed_tcb)
{
    isil_chip_t *chip;
    isil_prev_logic_chan_t *chan;

    chan = to_get_prev_channel_with_opened_logic_chan_ed(ed_tcb);
    chip = chan->chip;

    /* we didn't really close the preview chan,
     * but we need to trigger fake chip_close,
     * as chip reset is triggerd only when 
     * open_chan_number = 0.  
     * ps: I don't know what else we need to do. */
    chip->chip_close(chip);
}

void reset_isil_prev_chan_postprocessing(ed_tcb_t *ed_tcb)
{
    if (ed_tcb && ed_tcb->op) {
        isil_chip_t *chip;
        isil_prev_logic_chan_t *chan;
        robust_processing_control_t *robust;

        chan = to_get_prev_channel_with_opened_logic_chan_ed(ed_tcb);
        robust = &chan->robust_processing_control;
        robust->op->robust_processing_done(robust);

        /* we need to recover the fake chip_close. */
        chip = chan->chip;
        chip->chip_open(chip);

        //driver_gen_open_event(ed_tcb, 1);
        //ed_tcb->op->resume(ed_tcb);
    }
}

void isil_chip_prev_robust_process_done(isil_vp_bus_t *vp)
{
    if (vp){
        robust_processing_control_t *robust;

        robust = &vp->robust_processing_control;
        robust->op->robust_processing_done(robust);
    }
}

static void remove_isil_prev_chan(isil_prev_logic_chan_t *chan)
{
    isil_chip_t	*chip;
    ed_tcb_t *ed_tcb;

    if (chan) {
        ed_tcb = &chan->opened_logic_chan_ed;
        chip = chan->chip;
        prev_chan_disable(chan);
        prev_chan_release_vbufs(chan);
        isil_module_unregister(chip, &chip->preview_proc);
        ISIL_DBG(ISIL_DBG_INFO, "remove channel %d complete\n", chan->phy_slot_id);
    }
}

static void prev_video_device_release(struct video_device *vfd)
{
    kfree(vfd);
}

static int vp_bus_init(isil_vp_bus_t *vp)
{
    isil_chip_t *chip;
    isil_prev_logic_chan_t *chan;
    char name[48];
    int err;
    int i;
    
    if (vp == NULL)
        return ISIL_ERR;

    /* init bus-level resources and do reset */
    chip = to_get_chip_with_chip_vp_bus(vp);
    vp->norm = chip_get_norm(chip);
    mutex_init(&vp->vp_mutex);
    memset(&vp->tasklet_priv, 0, sizeof(tasklet_priv_t));
    init_robust_processing_control(&vp->robust_processing_control);

    /* alloc v4l2 device and init related logic channel res */
    for (i = 0; i < ISIL_PHY_VP_CHAN_NUMBER; i++) {
        chan = &chip->prev_chan[i];
        if (!(chan->v4ldev = video_device_alloc())) {
            ISIL_DBG(ISIL_DBG_INFO, "video_device_alloc() failed");
            goto fail;
        }
        snprintf(name, 48, "prev_chan%d", i);
        strcpy(chan->v4ldev->name, name);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 28)
        chan->v4ldev->parent = NULL;
#endif
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 16)
        chan->v4ldev->owner = THIS_MODULE;
        chan->v4ldev->type = VID_TYPE_CAPTURE;
        chan->v4ldev->hardware = 0;
#endif
        chan->v4ldev->fops = &prev_chan_fops;
        chan->v4ldev->minor = -1;
        chan->v4ldev->release = prev_video_device_release;
        video_set_drvdata(chan->v4ldev, chan);

#ifdef  USE_FOUR_CHAN_PREVIEW
        if(i < ISIL_LOGIC_VD_CHAN4){
#endif
            err = video_register_device(chan->v4ldev, VFL_TYPE_GRABBER, -1);
            if (err) {
                ISIL_DBG(ISIL_DBG_INFO, "V4L2 device registration failed");
                prev_video_device_release(chan->v4ldev);
                chan->v4ldev = NULL;
                goto fail;
            }
#ifdef  USE_FOUR_CHAN_PREVIEW
        }
#endif
        /* init preview logic channel res */
        init_isil_prev_chan(chan, chip->bus_id, chip->chip_id, i, chip);
    }     
    /* reset preview hw module if any */
    vp->op->reset(vp);
    vp_regs_init(chip);
    /* init preview's isr and tasklet */
    chip_request_irq(chip, IRQ_PREV_EOF_TYPE_INTR, prev_push_irq_handler, "prev", (void *)chip);
    chip_request_irq(chip, IRQ_PREV_OVERFLOW_TYPE_INTR, prev_overflow_irq_handler, "prevof", (void *)chip);	
    tasklet_init(&vp->prev_tasklet, prev_tasklet_handler, (unsigned long)&vp->tasklet_priv);

    return ISIL_OK;

fail:
    for (i = 0; i < ISIL_PHY_VP_CHAN_NUMBER; i++) {
        chan = &chip->prev_chan[i];
        if (chan->v4ldev) {
#ifdef  USE_FOUR_CHAN_PREVIEW
            if(i < ISIL_LOGIC_VD_CHAN4){
#endif
                video_unregister_device(chan->v4ldev);
#ifdef  USE_FOUR_CHAN_PREVIEW
            }
#endif
        }
    }
    return ISIL_ERR;
}

static int vp_bus_reset(isil_vp_bus_t *vp)
{
    if (vp == NULL)
        return ISIL_ERR;

    /* there should be some hw-level module reset code, 
     * but now, preview doesn't support hw-level module
     * reset yet. */

    ISIL_DBG(ISIL_DBG_INFO, "vp bus reset!\n");

    if (mutex_lock_interruptible(&vp->vp_mutex))
        return -ERESTARTSYS;
    vp->op->notify_suspend_logic_chan(vp);

    if (vp->op->bus_map(vp, R_RESET)) {
        vp->op->notify_resume_logic_chan(vp);
        mutex_unlock(&vp->vp_mutex);
        return ISIL_ERR;
    }
    vp->op->notify_resume_logic_chan(vp);
    mutex_unlock(&vp->vp_mutex);

    return ISIL_OK;
}

static int vp_bus_release(isil_vp_bus_t *vp)
{
    isil_chip_t *chip;
    isil_prev_logic_chan_t *chan;
    int i;
    
    if (vp == NULL)
        return ISIL_ERR;

    chip = to_get_chip_with_chip_vp_bus(vp);
    vp_regs_close(chip);

    chip_free_irq(chip, IRQ_PREV_EOF_TYPE_INTR, (void *)chip);
    chip_free_irq(chip, IRQ_PREV_OVERFLOW_TYPE_INTR, (void *)chip);	

    for (i = 0; i < ISIL_PHY_VP_CHAN_NUMBER; i++) {
        chan = &chip->prev_chan[i];
        remove_isil_prev_chan(chan);
        if (chan->v4ldev) {
            video_set_drvdata(chan->v4ldev, NULL);
#ifdef  USE_FOUR_CHAN_PREVIEW
            if(i < ISIL_LOGIC_VD_CHAN4){
#endif
                video_unregister_device(chan->v4ldev);
#ifdef  USE_FOUR_CHAN_PREVIEW
            }
#endif
        }
    }     
    return ISIL_OK;
}

static int vp_bus_notify_logic_chan_change(isil_vp_bus_t *vp)
{
    /*notify logic video channel*/

    return ISIL_OK;
}

static int vp_bus_notify_suspend_logic_chan(isil_vp_bus_t *vp)
{
    int i;
    ed_tcb_t *ed_tcb;
    isil_chip_t *chip;
    isil_prev_logic_chan_t *chan;

    if (vp == NULL)
        return ISIL_ERR;

    ISIL_DBG(ISIL_DBG_DEBUG, "vp bus all ready to suspend.\n");
    chip = to_get_chip_with_chip_vp_bus(vp);
    for (i = 0; i < ISIL_PHY_VP_CHAN_NUMBER; i++) {
        chan = &chip->prev_chan[i];
        ed_tcb = &chan->opened_logic_chan_ed;
        ed_tcb->op->suspend(ed_tcb);
    }
    vp_disable_pci_push(chip);
    ISIL_DBG(ISIL_DBG_DEBUG, "vp bus all suspend done.\n");
    return ISIL_OK;
}

static int vp_bus_notify_resume_logic_chan(isil_vp_bus_t *vp)
{
    int i;
    ed_tcb_t *ed_tcb;
    isil_chip_t *chip;
    isil_prev_logic_chan_t *chan;

    if (vp == NULL)
        return ISIL_ERR;

    ISIL_DBG(ISIL_DBG_INFO, "vp bus all ready to resume.\n");
    chip = to_get_chip_with_chip_vp_bus(vp);
    vp_enable_pci_push(chip);
    for (i = 0; i < ISIL_PHY_VP_CHAN_NUMBER; i++) {
        chan = &chip->prev_chan[i];
        ed_tcb = &chan->opened_logic_chan_ed;
        ed_tcb->op->resume(ed_tcb);
    }
    ISIL_DBG(ISIL_DBG_INFO, "vp bus all resume done.\n");
    return ISIL_OK;
}

static int vp_bus_map(isil_vp_bus_t *vp, int reason)
{
    int i, j;
    int cnt = 0, d1_cnt = 0, cif_cnt = 0, qcif_cnt = 0;
    vp_map_info_t map_order[VP_BUS_CNT*VP_BUS_CHAN_CNT];
    vp_map_info_t map_modify[VP_BUS_CNT*VP_BUS_CHAN_CNT];
    vp_map_info_t swap;
    vp_map_info_t *map_info;
    isil_prev_property_t *prev_property;
    isil_prev_logic_chan_t *chan;
    isil_chip_t *chip;
    //int enable;
    int refer;
    u32 fps;
    int norm;
    int bus4_chan; 
    int ret;

    memset(map_order, 0, sizeof(map_order));
    memset(map_modify, 0, sizeof(map_modify));

    chip = to_get_chip_with_chip_vp_bus(vp);
    for (i = 0; i < ISIL_PHY_VP_CHAN_NUMBER; i++) {
        chan = &chip->prev_chan[i];
        prev_property = &chan->prev_property;
        refer = chan->map_refer;
        fps = prev_property->op->get_fps(prev_property);
        if (refer && fps != 0) {
            map_order[cnt].phy_slot_id = chan->phy_slot_id;
            // no need to init map_id
            map_order[cnt].enable = 1;
            map_order[cnt].fps = fps;
            map_order[cnt].stride = prev_property->op->get_stride(prev_property);
            map_order[cnt].video_size = prev_property->op->get_video_size(prev_property);
            map_order[cnt].discard = prev_property->op->get_discard(prev_property);
            switch (map_order[cnt].video_size) {
                case ISIL_VIDEO_SIZE_D1:
                    d1_cnt++;
                    break;
                case ISIL_VIDEO_SIZE_CIF:
                    cif_cnt++;
                    break;
                case ISIL_VIDEO_SIZE_QCIF:
                default:
                    qcif_cnt++;
                    break;
            }
            cnt++;
        }
    }
    if (cnt == 0) {
        for (i = 0; i < ISIL_PHY_VP_CHAN_NUMBER; i++) {
            chan = &chip->prev_chan[i];
            prev_property = &chan->prev_property;
            map_modify[i].phy_slot_id = chan->phy_slot_id;
            map_modify[i].map_id = chan->phy_slot_id;
            map_modify[i].enable = 0;
            map_modify[i].fps = prev_property->op->get_fps(prev_property);
            map_modify[i].stride = prev_property->op->get_stride(prev_property);
            map_modify[i].video_size = prev_property->op->get_video_size(prev_property);
            map_modify[i].discard = prev_property->op->get_discard(prev_property);
        }       
        goto modify_done;
    }

    for (i = 0; i < cnt -1; i++)
        for (j = 0; j < cnt - i - 1; j++)
            if (map_order[j].video_size < map_order[j+1].video_size) {
                swap = map_order[j];
                map_order[j] = map_order[j+1];
                map_order[j+1] = swap;
            }

    if (need_bus4ext(d1_cnt, cif_cnt, qcif_cnt)) {
        ret = map_with_bus4ext(map_order, map_modify, cnt, &bus4_chan);
    } else {
        ret = map_without_bus4ext(map_order, map_modify, cnt);
        bus4_chan = 0;
    }
    if (ret == ISIL_ERR) {
        ISIL_DBG(ISIL_DBG_ERR, "vp bus fail!\n");
        return ISIL_ERR;
    }

    /* fullfill the disabled chan to map_modify to make sure channel numbers are complete */
    for (i = 0, j = 0; i < ISIL_PHY_VP_CHAN_NUMBER; i++) {
        chan = &chip->prev_chan[i];
        prev_property = &chan->prev_property;
        refer = chan->map_refer;
        fps = prev_property->op->get_fps(prev_property);
        if (!refer || fps == 0) {
            while (map_modify[j].enable)
                j++;
            map_modify[j].phy_slot_id = chan->phy_slot_id;
            map_modify[j].map_id = j;
            map_modify[j].enable = 0;
            map_modify[j].fps = prev_property->op->get_fps(prev_property);
            map_modify[j].stride = prev_property->op->get_stride(prev_property);
            map_modify[j].video_size = prev_property->op->get_video_size(prev_property);
            map_modify[j].discard = prev_property->op->get_discard(prev_property);
            j++;
        }
    }

modify_done:
    if (reason == R_RESET) {
        norm = chip_get_norm(chip);
        /* chech if RESET is triggered by P/N switch */
        if (norm != vp->op->get_norm(vp)) {
            vp->op->set_norm(vp, norm);
            map_info = map_modify;
            if (norm == ISIL_VIDEO_STANDARD_PAL)
                for (i = 0; i < ISIL_VP_MAP_CHAN_NUM; i++, map_info++) {
                    map_info->stride = STRIDE_PAL; 
                    if (map_info->fps > 2)
                        map_info->fps = (map_info->fps * FULL_FPS_PAL) / FULL_FPS_NTSC; 
                    if (map_info->fps > FULL_FPS_PAL)
                        map_info->fps = FULL_FPS_PAL;
                    /*map_info->discard = prev_fps_to_discard(map_info->fps); 

                    chan = &chip->prev_chan[map_info->phy_slot_id];
                    prev_property = &chan->prev_property;
                    prev_property->op->set_stride(prev_property, map_info->stride);
                    prev_property->op->set_fps(prev_property, map_info->fps);
                    prev_property->op->set_discard(prev_property, map_info->discard);*/
                }
            else
                for (i = 0; i < ISIL_VP_MAP_CHAN_NUM; i++, map_info++) {
                    map_info->stride = STRIDE_NTSC; 
                    if (map_info->fps > 2)
                        map_info->fps = (map_info->fps * FULL_FPS_NTSC) / FULL_FPS_PAL;
                    if (map_info->fps > FULL_FPS_NTSC)
                        map_info->fps = FULL_FPS_NTSC;
                    /*map_info->discard = prev_fps_to_discard(map_info->fps); 

                    chan = &chip->prev_chan[map_info->phy_slot_id];
                    prev_property = &chan->prev_property;
                    prev_property->op->set_stride(prev_property, map_info->stride);
                    prev_property->op->set_fps(prev_property, map_info->fps);
                    prev_property->op->set_discard(prev_property, map_info->discard);*/
                } 
        }
    }
    /* we should add some recalc fps code here, as the bus has limit.
     * Maybe, we will furtherly remap the map table to achieve optimization */
    norm = vp->op->get_norm(vp);
    for (i = 0, map_info = map_modify; i < ISIL_VP_MAP_CHAN_NUM; i++, map_info++) {
        if (norm == ISIL_VIDEO_STANDARD_PAL)
            map_info->discard = g_discard_table_pal[map_info->fps];
        else
            map_info->discard = g_discard_table_ntsc[map_info->fps];
    } 

    /* refresh parameter back to chan */
    for (i = 0, map_info = map_modify; i < ISIL_VP_MAP_CHAN_NUM; i++, map_info++) {
        chan = &chip->prev_chan[map_info->phy_slot_id];
        prev_property = &chan->prev_property;
        prev_property->op->set_stride(prev_property, map_info->stride);
        prev_property->op->set_fps(prev_property, map_info->fps);
        prev_property->op->set_discard(prev_property, map_info->discard);
    } 

    vp->op->bus_map_apply(vp, map_modify); 

    return ISIL_OK;
}

static int vp_get_norm(isil_vp_bus_t *vp)
{
    return vp->norm;
}

static int vp_set_norm(isil_vp_bus_t *vp, int norm)
{
    vp->norm = norm;

    return norm;
}

static int vp_bus_map_apply(isil_vp_bus_t *vp, vp_map_info_t *map_modify)
{
    int i, j;
    vp_map_info_t *map_info;
    isil_chip_t *chip;
    int bus_map[VP_BUS_CNT];
    int bus4_chan;
    int video_size;
    u32 phy_id; 
    u32 val;

    map_info = map_modify;
    chip = to_get_chip_with_chip_vp_bus(vp);

    ISIL_DBG(ISIL_DBG_INFO, "%8s %8s %8s %8s %8s %8s %8s\n", 
            "map", "phy", "size", "fps", "discard", "stride", "enable");
    for(i = 0; i < VP_BUS_CNT; i++)
        for(j = 0; j < VP_BUS_CHAN_CNT; j++)
            ISIL_DBG(ISIL_DBG_INFO, "%8d %8d %8d %8d %8x %8d %8d\n", 
                    map_info[i*VP_BUS_CHAN_CNT+j].map_id, 
                    map_info[i*VP_BUS_CHAN_CNT+j].phy_slot_id, 
                    map_info[i*VP_BUS_CHAN_CNT+j].video_size, 
                    map_info[i*VP_BUS_CHAN_CNT+j].fps, 
                    map_info[i*VP_BUS_CHAN_CNT+j].discard, 
                    map_info[i*VP_BUS_CHAN_CNT+j].stride,
                    map_info[i*VP_BUS_CHAN_CNT+j].enable);

    /* disable all the channels */
    //chip->io_op->chip_write32(chip, VP_CHANNEL_EN, 0x0);
    chip->io_op->chip_write32(chip, VP_CHANNEL_INTERLACE, 0x0);
    chip->io_op->chip_write32(chip, VP_BUS_MAX_CHANNEL, 0x0);
    chip->io_op->chip_write32(chip, VP_FIELD_SELECT, 0x0);

    if (vp->op->get_norm(vp) == ISIL_VIDEO_STANDARD_PAL) {
        val = chip->io_op->chip_read32(chip, PCI_PREVIEW_FORMAT);
        val &= ~(0x1 << 12);
        chip->io_op->chip_write32(chip, PCI_PREVIEW_FORMAT, val);

        chip->io_op->chip_write32(chip, VP_BUS_MAX_LINE0, 0x318);
        chip->io_op->chip_write32(chip, VP_BUS_MAX_LINE1, 0x318);
    } else {
        val = chip->io_op->chip_read32(chip, PCI_PREVIEW_FORMAT);
        val |= 0x1 << 12;
        chip->io_op->chip_write32(chip, PCI_PREVIEW_FORMAT, val);

        chip->io_op->chip_write32(chip, VP_BUS_MAX_LINE0, 0x3bd);
        chip->io_op->chip_write32(chip, VP_BUS_MAX_LINE1, 0x3bd);
    }

    map_info = map_modify;
    for (i = 0; i < ISIL_VP_MAP_CHAN_NUM; i++, map_info++) {
        video_size = map_info->video_size;
        phy_id = map_info->phy_slot_id;
        switch (video_size) {
            case ISIL_VIDEO_SIZE_D1:
                val = chip->io_op->chip_read32(chip, VP_CHANNEL_DOWNSAMPLE);
                val &= ~BIT(phy_id);
                chip->io_op->chip_write32(chip, VP_CHANNEL_DOWNSAMPLE, val);
                if (phy_id < 8) {
                    val = chip->io_op->chip_read32(chip, VP_CHANNEL_FORMAT0);
                    val &= ~(0x3 << phy_id * 2);
                    chip->io_op->chip_write32(chip, VP_CHANNEL_FORMAT0, val);
                }
                else {
                    val = chip->io_op->chip_read32(chip, VP_CHANNEL_FORMAT1);
                    val &= ~(0x3 << (phy_id - 8) * 2);
                    chip->io_op->chip_write32(chip, VP_CHANNEL_FORMAT1, val);
                }
                break;

            case ISIL_VIDEO_SIZE_CIF:
                val = chip->io_op->chip_read32(chip, VP_CHANNEL_DOWNSAMPLE);
                val |= BIT(phy_id);
                chip->io_op->chip_write32(chip, VP_CHANNEL_DOWNSAMPLE, val);
                if (phy_id < 8) {
                    val = chip->io_op->chip_read32(chip, VP_CHANNEL_FORMAT0);
                    val |= (0x3 << phy_id * 2);
                    chip->io_op->chip_write32(chip, VP_CHANNEL_FORMAT0, val);
                }
                else {
                    val = chip->io_op->chip_read32(chip, VP_CHANNEL_FORMAT1);
                    val |= (0x3 << (phy_id - 8) * 2);
                    chip->io_op->chip_write32(chip, VP_CHANNEL_FORMAT1, val);
                }
                break;

            case ISIL_VIDEO_SIZE_QCIF:
                val = chip->io_op->chip_read32(chip, VP_CHANNEL_DOWNSAMPLE);
                val |= BIT(phy_id);
                chip->io_op->chip_write32(chip, VP_CHANNEL_DOWNSAMPLE, val);
                if (phy_id < 8) {
                    val = chip->io_op->chip_read32(chip, VP_CHANNEL_FORMAT0);
                    val &= ~(0x3 << phy_id * 2);
                    val |= (0x1 << phy_id * 2);
                    chip->io_op->chip_write32(chip, VP_CHANNEL_FORMAT0, val);
                }
                else {
                    val = chip->io_op->chip_read32(chip, VP_CHANNEL_FORMAT1);
                    val &= ~(0x3 << (phy_id - 8) * 2);
                    val |= (0x1 << (phy_id - 8) * 2);
                    chip->io_op->chip_write32(chip, VP_CHANNEL_FORMAT1, val);
                }
                break;

            default:
                ISIL_DBG(ISIL_DBG_ERR, "invalid format type %d\n", video_size);
                return ISIL_ERR;
        }
    }

    /* sync discard table to hardware regs */
    map_info = map_modify;
    for (i = 0; i < VP_BUS_CNT; i++) {
        for(j = 0; j < VP_BUS_CHAN_CNT; j++) {
            chip->io_op->chip_write32(chip, VP_CHANNEL_LMAP(i*VP_BUS_CHAN_CNT + j), 
                    map_info[i*VP_BUS_CHAN_CNT+j].discard & 0xffff);
            chip->io_op->chip_write32(chip, VP_CHANNEL_HMAP(i*VP_BUS_CHAN_CNT + j), 
                    map_info[i*VP_BUS_CHAN_CNT+j].discard >> 16);
        }
    }

    /* sync vp bus map to hardware regs */
    for (i = 0; i < VP_BUS_CNT; i++) {
        bus_map[i] = 0;
        for(j = 0; j < VP_BUS_CHAN_CNT; j++)
            bus_map[i] |= (map_info[i*VP_BUS_CHAN_CNT+j].phy_slot_id & 0xf) << 4*j;
    }
    for (i = 0; i < VP_BUS_CNT; i++)
        chip->io_op->chip_write32(chip, VP_BUS_MAP(i), bus_map[i]);

    /* judge weather preivew bus4 needs to use. if using, how many slots to need */
    if (map_info[0 + 2].video_size == ISIL_VIDEO_SIZE_D1 || 
            map_info[0 + 2].video_size == ISIL_VIDEO_SIZE_CIF) {
        if (map_info[0 + 3].video_size == map_info[0 + 2].video_size)
            bus4_chan = 0x3;
        else
            bus4_chan = 0x1;
    }
    else {
        bus4_chan = 0x0;
    }
    /* sync bus4 info to hardware regs */
    chip->io_op->chip_write32(chip, PCI_PREVIEW_BUS4_ENB, bus4_chan | 0x10000);
    if (bus4_chan) {
        if (map_info[0 + 2].video_size == ISIL_VIDEO_SIZE_D1) {
            val = chip->io_op->chip_read32(chip, PCI_PREVIEW_FORMAT);
            val |= 0x3 << 13;
            val &= ~(0x1 << 15);
            chip->io_op->chip_write32(chip, PCI_PREVIEW_FORMAT, val);
        }
        else {
            val = chip->io_op->chip_read32(chip, PCI_PREVIEW_FORMAT);
            val &= ~(0x3 << 13);
            val |=(0x1 << 15);
            chip->io_op->chip_write32(chip, PCI_PREVIEW_FORMAT, val);
        }
    }

    /* snyc bus0~3 info to hardware regs */
    for (i = 0; i < VP_BUS_CNT; i++) {
        switch (map_info[i*VP_BUS_CHAN_CNT+ 0].video_size) {
            case ISIL_VIDEO_SIZE_D1:
                val = chip->io_op->chip_read32(chip, PCI_PREVIEW_FORMAT);
                val |= (0x3 << 2*i);
                val &= ~(0x1 << (8 + i));
                chip->io_op->chip_write32(chip, PCI_PREVIEW_FORMAT, val);
                break;

            case ISIL_VIDEO_SIZE_CIF:
                val = chip->io_op->chip_read32(chip, PCI_PREVIEW_FORMAT);
                val &= ~(0x3 << 2*i);
                val |= (0x1 << (8 + i));
                chip->io_op->chip_write32(chip, PCI_PREVIEW_FORMAT, val);
                break;

            case ISIL_VIDEO_SIZE_QCIF:
            default:
                val = chip->io_op->chip_read32(chip, PCI_PREVIEW_FORMAT);
                val &= ~(0x3 << 2*i);
                val |= (0x1 << 2*i);
                val |= (0x1 << (8 + i));
                chip->io_op->chip_write32(chip, PCI_PREVIEW_FORMAT, val);
                break;
        }
    }

    /* PCI related init */
    /*chip->io_op->chip_write32(chip, PCI_INTR_CHAN_SOURCE, 0xffff);
    val = chip->io_op->chip_read32(chip, PCI_MASTER_INTR_ENABLE_REG);
    chip->io_op->chip_write32(chip, PCI_MASTER_INTR_ENABLE_REG, (val | BIT_PREV | BIT_PREVOF));

    val = chip->io_op->chip_read32(chip, PCI_MASTER_CONTROL);
    chip->io_op->chip_write32(chip, PCI_MASTER_CONTROL, val | (BIT_MASTER | BIT_PREV | BIT_PREVOF));*/

    return ISIL_OK;
}

static void vp_bus_polling_task(isil_vp_bus_t *vp)
{
    int i;
    isil_chip_t *chip;
    ed_tcb_t    *ed_tcb;
    isil_ed_fsm_t *ed_fsm;
    isil_prev_logic_chan_t *chan;

    if (vp == NULL)
        return;

    chip = to_get_chip_with_chip_vp_bus(vp);
    for (i = 0; i < ISIL_PHY_VP_CHAN_NUMBER; i++) {
        chan = &chip->prev_chan[i];
        if (chan->enable) {
            ed_tcb = &chan->opened_logic_chan_ed;
            ed_fsm = &ed_tcb->ed_fsm;
            if(ed_fsm->op)
                ed_fsm->op->update_state_timeout_counter(ed_fsm);
        }
    }
}

static struct isil_vp_bus_operation	isil_vp_bus_op = {
    .init               = vp_bus_init,
    .reset              = vp_bus_reset,
    .release            = vp_bus_release,
    
    .notify_logic_chan_change   = vp_bus_notify_logic_chan_change,
    .notify_suspend_logic_chan  = vp_bus_notify_suspend_logic_chan,
    .notify_resume_logic_chan   = vp_bus_notify_resume_logic_chan,
    
    .get_norm = vp_get_norm,
    .set_norm = vp_set_norm,
    .bus_map = vp_bus_map,

    .calculate_vp_bus_map_table = NULL,
    .bus_map_apply = vp_bus_map_apply,

    .polling_task       = vp_bus_polling_task,
};

int init_isil_vp_bus(isil_vp_bus_t *vp_bus)
{
    if (vp_bus != NULL) {
        vp_bus->op = &isil_vp_bus_op;
        return vp_bus->op->init(vp_bus);
    }
    return ISIL_ERR;
}

void remove_isil_vp_bus(isil_vp_bus_t *vp_bus)
{
    if (vp_bus != NULL) {
        vp_bus->op->release(vp_bus);
    }
}

