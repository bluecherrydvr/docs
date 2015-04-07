#include    <isil5864/isil_common.h>

static void register_opened_logic_encode_chan_into_chip_audio_slot(isil_audio_driver_t *audio_driver, chip_audio_t *chip_audio);
static void unregister_opened_logic_encode_chan_from_chip_audio_slot(isil_audio_driver_t *audio_driver, chip_audio_t *chip_audio);
static void register_opened_logic_decode_chan_into_chip_audio_slot(isil_audio_driver_t *audio_driver, chip_audio_t *chip_audio);
static void unregister_opened_logic_decode_chan_from_chip_audio_slot(isil_audio_driver_t *audio_driver, chip_audio_t *chip_audio);

static int  audio_encode_driver_in_unregister_state_recv_close_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t *audio_driver;
        chip_audio_t    *chip_audio;

        audio_driver = (isil_audio_driver_t*)context;
        chip_audio = audio_driver->chip_audio;
        if(atomic_read(&audio_driver->opened_flag) > 0){
            if(isil_audio_chan_is_in_robust_processing(audio_driver)){
                ed_fsm->op->save_event_for_robust_processing(ed_fsm);
            }

            if(audio_driver->read_bitstream_mode == READ_BITSTREAM_BY_PCI){
                chip_audio_push_isr_interface_t *audio_push_interface;
                audio_push_interface = &chip_audio->audio_push_interface;
                audio_push_interface->op->disable_chip_audio_chan(audio_push_interface, audio_driver->audio_logic_chan_id);
                audio_push_interface->op->close_chip_audio(audio_push_interface, audio_driver->audio_logic_chan_id);
            } else {
                chip_audio->op->set_enable_audio_chan(chip_audio, audio_driver->audio_logic_chan_id, AUDIO_DISABLE);
            }

            ed_fsm->op->reset(ed_fsm);
            atomic_dec(&audio_driver->opened_flag);
        }
        if(atomic_read(&audio_driver->opened_flag) == 0){
            ed_tcb_t    *ed_tcb;
            ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
            ed_tcb->op->close_done(ed_tcb);
        }
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
    }
    return ISIL_OK;
}

static int  audio_encode_driver_in_unregister_state_recv_suspend_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t   *audio_driver;
        ed_tcb_t    *ed_tcb;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        ed_tcb->op->suspend_done(ed_tcb);
        audio_driver = (isil_audio_driver_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
    }
    return ISIL_OK;
}

static int  audio_encode_driver_in_unregister_state_recv_resume_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t   *audio_driver;
        ed_tcb_t    *ed_tcb;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        ed_tcb->op->resume_done(ed_tcb);
        audio_driver = (isil_audio_driver_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
    }
    return ISIL_OK;
}

static int  audio_encode_driver_in_unregister_state_recv_open_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t *audio_driver;
        chip_audio_t        *chip_audio;

        audio_driver = (isil_audio_driver_t*)context;
        chip_audio = audio_driver->chip_audio;
        if(atomic_read(&audio_driver->opened_flag) == 0){
            register_opened_logic_encode_chan_into_chip_audio_slot(audio_driver, chip_audio);

            reset_isil_audio_encode_chan(audio_driver);
            ed_fsm->op->change_state(ed_fsm, ISIL_ED_SUSPEND);
            if(audio_driver->read_bitstream_mode == READ_BITSTREAM_BY_PCI){
                chip_audio_push_isr_interface_t *audio_push_interface;
                audio_push_interface = &chip_audio->audio_push_interface;
                audio_push_interface->op->open_chip_audio(audio_push_interface, audio_driver->audio_logic_chan_id);
            }

            if(isil_audio_chan_is_in_robust_processing(audio_driver)){
                ed_tcb_t    *ed_tcb;
                ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
                ed_fsm->op->restore_event_from_robust_processing(ed_fsm);
                if(have_any_pending_event(ed_tcb)){
                    driver_trigger_pending_event(ed_tcb);
                }
            }
        }
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
    }
    return ISIL_OK;
}

static int  audio_encode_driver_in_idle_state_recv_close_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t   *audio_driver;
        ed_tcb_t    *ed_tcb;

        ed_fsm->op->change_state(ed_fsm, ISIL_ED_UNREGISTER);
        audio_driver = (isil_audio_driver_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_close_event(ed_tcb, 1);
    }
    return ISIL_OK;
}

static int  audio_encode_driver_in_idle_state_recv_timeout_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t *audio_driver;

        audio_driver = (isil_audio_driver_t*)context;
        start_audio_chan_robust_process(audio_driver);
        audio_chan_gen_req_msg(audio_driver, REQ_ALGO_CHIP_RESET, NONBLOCK_OP);        
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
    }
    return ISIL_OK;
}

static int  audio_encode_driver_in_idle_state_recv_suspend_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t *audio_driver;
        chip_audio_t        *chip_audio;
        ed_tcb_t            *ed_tcb;
        chip_audio_push_isr_interface_t *audio_push_interface;

        ed_fsm->op->change_state(ed_fsm, ISIL_ED_SUSPEND);
        audio_driver = (isil_audio_driver_t*)context;
        chip_audio = audio_driver->chip_audio;
        if(audio_driver->read_bitstream_mode == READ_BITSTREAM_BY_PCI){
            audio_push_interface = &chip_audio->audio_push_interface;
            audio_push_interface->op->disable_chip_audio_chan(audio_push_interface, audio_driver->audio_logic_chan_id);
            ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
        } else {
            chip_audio->op->set_enable_audio_chan(chip_audio, audio_driver->audio_logic_chan_id, AUDIO_DISABLE);
            ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
        }
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        ed_tcb->op->suspend_done(ed_tcb);
    }
    return ISIL_OK;
}

static int  audio_encode_driver_in_idle_state_recv_resume_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t   *audio_driver;
        ed_tcb_t    *ed_tcb;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        ed_tcb->op->resume_done(ed_tcb);
        audio_driver = (isil_audio_driver_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
    }
    return ISIL_OK;
}

static int  audio_encode_driver_in_idle_state_recv_deliver_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t *audio_driver = (isil_audio_driver_t*)context;
        chip_audio_t        *chip_audio;
        isil_chip_t         *chip;

        if(audio_driver->read_bitstream_mode == READ_BITSTREAM_BY_DDR){
            chip_audio_encode_t *audio_encode;
            isil_audio_packet_queue_t   *audio_packet_queue;
            isil_audio_chan_buf_pool_t  *audio_buf_pool;
            audio_section_ptr_info_t    *audio_section_param;

            chip_audio = audio_driver->chip_audio;
            chip = chip_audio->chip;
            audio_encode = &chip_audio->audio_encode;
            audio_encode->op->get_audio_encode_param(audio_encode, chip);

            audio_packet_queue = &audio_driver->audio_packet_queue;
            audio_buf_pool = &audio_driver->audio_buf_pool;
            audio_section_param = &audio_driver->audio_section_param;
            audio_section_param->op->update_header_ptr(audio_section_param, chip_audio);
            audio_section_param->op->update_tailer_ptr(audio_section_param, chip_audio);
            if(audio_section_param->op->queue_capacity(audio_section_param)){
                if(audio_section_param->discard_number > 0){
                    audio_section_param->discard_number--;
                    audio_encode->op->set_finish_rd_ptr(audio_encode, audio_section_param->op->get_update_finish_id(audio_section_param), chip);
                    ed_fsm->op->feed_state_watchdog(ed_fsm);    
                } else {
                    dpram_control_t *chip_dpram_controller;
                    dpram_request_t *read_audio_bitstream_req;

                    ed_fsm->op->change_state(ed_fsm, ISIL_ED_STANDBY);
                    audio_section_param->op->update_curr_section_offset(audio_section_param, chip_audio);
                    read_audio_bitstream_req = &audio_driver->audio_bitstream_req;
                    init_submit_recv_audio_bitstream_service(read_audio_bitstream_req, audio_driver);
                    get_isil_dpram_controller(&chip_dpram_controller);
                    chip_dpram_controller->op->submit_read_audio_req(chip_dpram_controller, audio_driver);
                }
            }
        }
    }
    return ISIL_OK;
}

static int  audio_encode_driver_in_standby_state_recv_close_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t *audio_driver;
        ed_tcb_t    *ed_tcb;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        audio_driver = (isil_audio_driver_t*)context;
        if(audio_driver->read_bitstream_mode == READ_BITSTREAM_BY_PCI){
            driver_gen_close_event(ed_tcb, 0);
        } else {
            dpram_control_t *chip_dpram_controller;
            get_isil_dpram_controller(&chip_dpram_controller);
            if(chip_dpram_controller->op->delete_read_audio_req(chip_dpram_controller, audio_driver)){
                ed_fsm->op->change_state(ed_fsm, ISIL_ED_UNREGISTER);
                driver_gen_close_event(ed_tcb, 1);
            } else {
                driver_gen_close_event(ed_tcb, 0);
                ISIL_DBG(ISIL_DBG_INFO, "%d, prevent crit scene\n", audio_driver->audio_logic_chan_id);
            }
            ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
        }
    }
    return ISIL_OK;
}

static int  audio_encode_driver_in_standby_state_recv_timeout_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t *audio_driver;

        audio_driver = (isil_audio_driver_t*)context;
        start_audio_chan_robust_process(audio_driver);
        if(audio_driver->read_bitstream_mode == READ_BITSTREAM_BY_DDR){
            dpram_control_t *chip_dpram_controller;
            get_isil_dpram_controller(&chip_dpram_controller);
            chip_dpram_controller->op->delete_read_audio_req(chip_dpram_controller, audio_driver);
        }
        audio_chan_gen_req_msg(audio_driver, REQ_ALGO_CHIP_RESET, NONBLOCK_OP);
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
    }
    return ISIL_OK;
}

static int  audio_encode_driver_in_standby_state_recv_suspend_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t *audio_driver;
        chip_audio_t    *chip_audio;
        ed_tcb_t        *ed_tcb;

        audio_driver = (isil_audio_driver_t*)context;
        chip_audio = audio_driver->chip_audio;
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        if(audio_driver->read_bitstream_mode == READ_BITSTREAM_BY_PCI){
            driver_gen_suspend_event(ed_tcb, 0);
            ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
        } else {
            dpram_control_t *chip_dpram_controller;
            get_isil_dpram_controller(&chip_dpram_controller);
            if(chip_dpram_controller->op->delete_read_audio_req(chip_dpram_controller, audio_driver)){
                chip_audio->op->set_enable_audio_chan(chip_audio, audio_driver->audio_logic_chan_id, AUDIO_DISABLE);
                ed_fsm->op->change_state(ed_fsm, ISIL_ED_SUSPEND);
                ed_tcb->op->suspend_done(ed_tcb);
            } else {
                driver_gen_suspend_event(ed_tcb, 0);
                ISIL_DBG(ISIL_DBG_INFO, "%d, prevent crit scene\n", audio_driver->audio_logic_chan_id);
            }
            ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
        }
    }
    return ISIL_OK;
}

static int  audio_encode_driver_in_standby_state_recv_resume_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t   *audio_driver;
        ed_tcb_t    *ed_tcb;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        audio_driver = (isil_audio_driver_t*)context;
        if(audio_driver->read_bitstream_mode == READ_BITSTREAM_BY_PCI){
            driver_gen_resume_event(ed_tcb, 0);
        } else {
            ed_tcb->op->resume_done(ed_tcb);
        }
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
    }
    return ISIL_OK;
}

static int  audio_encode_driver_in_standby_state_recv_deliver_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t   *audio_driver;
        ed_tcb_t    *ed_tcb;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        audio_driver = (isil_audio_driver_t*)context;
        if(audio_driver->read_bitstream_mode == READ_BITSTREAM_BY_PCI){
            ed_fsm->op->change_state(ed_fsm, ISIL_ED_TRANSFERING);
            driver_gen_deliver_event(ed_tcb, 1);
        } else {
            ed_fsm->op->change_state(ed_fsm, ISIL_ED_RUNNING);
        }
    }
    return ISIL_OK;
}

static int  audio_encode_driver_in_suspend_state_recv_close_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t   *audio_driver;
        ed_tcb_t    *ed_tcb;

        ed_fsm->op->change_state(ed_fsm, ISIL_ED_UNREGISTER);
        audio_driver = (isil_audio_driver_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_close_event(ed_tcb, 1);
    }
    return ISIL_OK;
}

static int  audio_encode_driver_in_suspend_state_recv_suspend_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t   *audio_driver;
        ed_tcb_t    *ed_tcb;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        ed_tcb->op->suspend_done(ed_tcb);
        audio_driver = (isil_audio_driver_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
    }
    return ISIL_OK;
}

static int  audio_encode_driver_in_suspend_state_recv_resume_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t *audio_driver;
        chip_audio_t    *chip_audio;
        ed_tcb_t    *ed_tcb;

        audio_driver = (isil_audio_driver_t*)context;
        chip_audio = audio_driver->chip_audio;
        if(audio_driver->read_bitstream_mode == READ_BITSTREAM_BY_PCI){
            chip_audio_push_isr_interface_t *audio_push_interface;
            ed_fsm->op->change_state(ed_fsm, ISIL_ED_STANDBY);
            audio_push_interface = &chip_audio->audio_push_interface;
            audio_push_interface->op->enable_chip_audio_chan(audio_push_interface, audio_driver->audio_logic_chan_id);
        } else {
            ed_fsm->op->change_state(ed_fsm, ISIL_ED_IDLE);
            chip_audio->op->set_enable_audio_chan(chip_audio, audio_driver->audio_logic_chan_id, AUDIO_ENABLE);
        }
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        ed_tcb->op->resume_done(ed_tcb);
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
    }
    return ISIL_OK;
}

static int  audio_encode_driver_in_running_state_recv_close_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t   *audio_driver;
        ed_tcb_t    *ed_tcb;

        audio_driver = (isil_audio_driver_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_close_event(ed_tcb, 0);
    }
    return ISIL_OK;
}

static int  audio_encode_driver_in_running_state_recv_timeout_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t *audio_driver;

        audio_driver = (isil_audio_driver_t*)context;
        start_audio_chan_robust_process(audio_driver);
        if(audio_driver->read_bitstream_mode == READ_BITSTREAM_BY_DDR){
            dpram_control_t *chip_dpram_controller;
            isil_chip_t     *chip;

            chip = audio_driver->chip_audio->chip;
            get_isil_dpram_controller(&chip_dpram_controller);
            chip_free_irq(chip, IRQ_BURST_TYPE_INTR, audio_driver);
            chip_dpram_controller->op->release_end_move_data_from_dpram_to_host_service_req(chip_dpram_controller, &audio_driver->dpram_page, chip);
        }
        audio_chan_gen_req_msg(audio_driver, REQ_ALGO_CHIP_RESET, NONBLOCK_OP);
        ed_fsm->op->change_state(ed_fsm, ISIL_ED_DONE);
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
    }
    return ISIL_OK;
}

static int  audio_encode_driver_in_running_state_recv_suspend_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t   *audio_driver;
        ed_tcb_t    *ed_tcb;

        audio_driver = (isil_audio_driver_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_suspend_event(ed_tcb, 0);
    }
    return ISIL_OK;
}

static int  audio_encode_driver_in_running_state_recv_resume_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t   *audio_driver;
        ed_tcb_t    *ed_tcb;

        audio_driver = (isil_audio_driver_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_resume_event(ed_tcb, 0);
    }
    return ISIL_OK;
}

static int  audio_encode_driver_in_running_state_recv_deliver_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        ed_fsm->op->change_state(ed_fsm, ISIL_ED_TRANSFERING);
    }
    return ISIL_OK;
}

static int  audio_encode_driver_in_transfering_state_recv_close_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t   *audio_driver;
        ed_tcb_t    *ed_tcb;

        audio_driver = (isil_audio_driver_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_close_event(ed_tcb, 0);
    }
    return ISIL_OK;
}

static int  audio_encode_driver_in_transfering_state_recv_timeout_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t *audio_driver;

        audio_driver = (isil_audio_driver_t*)context;
        start_audio_chan_robust_process(audio_driver);
        if(audio_driver->read_bitstream_mode == READ_BITSTREAM_BY_DDR){
            dpram_control_t *chip_dpram_controller;
            isil_chip_t     *chip;

            chip = audio_driver->chip_audio->chip;
            get_isil_dpram_controller(&chip_dpram_controller);
            chip_free_irq(chip, IRQ_BURST_TYPE_INTR, audio_driver);
            chip_dpram_controller->op->release_end_move_data_from_dpram_to_host_service_req(chip_dpram_controller, &audio_driver->dpram_page, chip);            
        }
        audio_chan_gen_req_msg(audio_driver, REQ_ALGO_CHIP_RESET, NONBLOCK_OP);
        ed_fsm->op->change_state(ed_fsm, ISIL_ED_DONE);
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
    }
    return ISIL_OK;
}

static int  audio_encode_driver_in_transfering_state_recv_suspend_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t   *audio_driver;
        ed_tcb_t    *ed_tcb;

        audio_driver = (isil_audio_driver_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_suspend_event(ed_tcb, 0);
    }
    return ISIL_OK;
}

static int  audio_encode_driver_in_transfering_state_recv_resume_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t   *audio_driver;
        ed_tcb_t    *ed_tcb;

        audio_driver = (isil_audio_driver_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_resume_event(ed_tcb, 0);
    }
    return ISIL_OK;
}

static int  audio_encode_driver_in_transfering_state_recv_deliver_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        ed_tcb_t    *ed_tcb;
        isil_audio_driver_t   *audio_driver;

        audio_driver = (isil_audio_driver_t*)context;
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

static int  audio_encode_driver_in_done_state_recv_close_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t   *audio_driver;
        ed_tcb_t    *ed_tcb;

        ed_fsm->op->change_state(ed_fsm, ISIL_ED_UNREGISTER);
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_close_event(ed_tcb, 1);
        audio_driver = (isil_audio_driver_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
    }
    return ISIL_OK;
}

static int  audio_encode_driver_in_done_state_recv_timeout_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t *audio_driver;

        audio_driver = (isil_audio_driver_t*)context;
        start_audio_chan_robust_process(audio_driver);
        audio_chan_gen_req_msg(audio_driver, REQ_ALGO_CHIP_RESET, NONBLOCK_OP);
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
    }
    return ISIL_OK;
}

static int  audio_encode_driver_in_done_state_recv_suspend_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t *audio_driver;
        chip_audio_t    *chip_audio;
        ed_tcb_t        *ed_tcb;

        audio_driver = (isil_audio_driver_t*)context;
        chip_audio = audio_driver->chip_audio;
        if(audio_driver->read_bitstream_mode == READ_BITSTREAM_BY_PCI){
            chip_audio_push_isr_interface_t *audio_push_interface;
            audio_push_interface = &chip_audio->audio_push_interface;
            audio_push_interface->op->disable_chip_audio_chan(audio_push_interface, audio_driver->audio_logic_chan_id);
            ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
        } else {
            chip_audio->op->set_enable_audio_chan(chip_audio, audio_driver->audio_logic_chan_id, AUDIO_DISABLE);
            ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
        }
        ed_fsm->op->change_state(ed_fsm, ISIL_ED_SUSPEND);
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        ed_tcb->op->suspend_done(ed_tcb);
    }
    return ISIL_OK;
}

static int  audio_encode_driver_in_done_state_recv_resume_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t *audio_driver;
        ed_tcb_t    *ed_tcb;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_deliver_event(ed_tcb, 1);
        ed_tcb->op->resume_done(ed_tcb);
        audio_driver = (isil_audio_driver_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
    }
    return ISIL_OK;
}

static int  audio_encode_driver_in_done_state_recv_deliver_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t *audio_driver;

        audio_driver = (isil_audio_driver_t*)context;
        if(audio_driver->read_bitstream_mode == READ_BITSTREAM_BY_PCI){
            ed_fsm->op->change_state(ed_fsm, ISIL_ED_STANDBY);
        } else {
            ed_fsm->op->change_state(ed_fsm, ISIL_ED_IDLE);
        }
    }
    return ISIL_OK;
}

static int  audio_encode_driver_sync_config_to_running_hook(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t *audio_driver;
        chip_audio_t        *chip_audio;
        chip_audio_encode_t *audio_encode;
        audio_encode_control_t  *encode_control;
        isil_audio_config_descriptor_t  *config_param;
        isil_audio_param_t  *running_param;

        audio_driver = (isil_audio_driver_t*)context;
        chip_audio = audio_driver->chip_audio;
        audio_encode = &chip_audio->audio_encode;
        encode_control = &audio_encode->audio_control;
        config_param = &audio_encode->config_param;
        running_param = &audio_encode->running_param;

        if(atomic_read(&audio_encode->need_sync_param)){
            running_param->op->update_bit_wide(running_param, config_param->bit_wide);
            running_param->op->update_sample_rate(running_param, config_param->sample_rate);
            running_param->op->update_type(running_param, config_param->type);        
            if(audio_driver->read_bitstream_mode == READ_BITSTREAM_BY_PCI){
                encode_control->op->set_enable_pci_sel_pcm_or_adpcm(encode_control, audio_driver->audio_logic_chan_id, running_param->op->get_type(running_param), chip_audio->chip);
            }
            atomic_set(&audio_encode->need_sync_param, 0);
            ISIL_DBG(ISIL_DBG_INFO, "%d, param=%d, %d, %d\n", audio_driver->audio_logic_chan_id,
                    running_param->op->get_bit_wide(running_param),
                    running_param->op->get_sample_rate(running_param),
                    running_param->op->get_type(running_param));
        }
    }
    return ISIL_OK;
}

static int  audio_encode_driver_sync_running_to_config_hook(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t *audio_driver;
        chip_audio_t        *chip_audio;
        chip_audio_encode_t *audio_encode;
        isil_audio_config_descriptor_t  *config_param;
        isil_audio_param_t  *running_param;

        audio_driver = (isil_audio_driver_t*)context;
        chip_audio = audio_driver->chip_audio;
        audio_encode = &chip_audio->audio_encode;
        config_param = &audio_encode->config_param;
        running_param = &audio_encode->running_param;

        config_param->bit_wide =  running_param->op->get_bit_wide(running_param);
        config_param->sample_rate =  running_param->op->get_sample_rate(running_param);
        config_param->type =  running_param->op->get_type(running_param);
        atomic_set(&audio_encode->need_sync_param, 0);
    }
    return ISIL_OK;
}

static fsm_state_transfer_matrix_table_t    audio_encode_driver_fsm_state_transfer_matrix_table = {
    .action = {
        {
            audio_encode_driver_in_unregister_state_recv_close_event, 
            no_op,
            audio_encode_driver_in_unregister_state_recv_suspend_event,
            audio_encode_driver_in_unregister_state_recv_resume_event,
            audio_encode_driver_in_unregister_state_recv_open_event,
            no_op
        },
        {
            audio_encode_driver_in_idle_state_recv_close_event, 
            audio_encode_driver_in_idle_state_recv_timeout_event,
            audio_encode_driver_in_idle_state_recv_suspend_event,
            audio_encode_driver_in_idle_state_recv_resume_event,
            no_op,
            audio_encode_driver_in_idle_state_recv_deliver_event
        },
        {
            audio_encode_driver_in_standby_state_recv_close_event, 
            audio_encode_driver_in_standby_state_recv_timeout_event,
            audio_encode_driver_in_standby_state_recv_suspend_event,
            audio_encode_driver_in_standby_state_recv_resume_event,
            no_op,
            audio_encode_driver_in_standby_state_recv_deliver_event
        },
        {
            audio_encode_driver_in_suspend_state_recv_close_event,
            no_op,
            audio_encode_driver_in_suspend_state_recv_suspend_event,
            audio_encode_driver_in_suspend_state_recv_resume_event,
            no_op,
            no_op
        },
        {
            audio_encode_driver_in_running_state_recv_close_event,
            audio_encode_driver_in_running_state_recv_timeout_event,
            audio_encode_driver_in_running_state_recv_suspend_event,
            audio_encode_driver_in_running_state_recv_resume_event,
            no_op,
            audio_encode_driver_in_running_state_recv_deliver_event
        },
        {
            audio_encode_driver_in_transfering_state_recv_close_event,
            audio_encode_driver_in_transfering_state_recv_timeout_event,
            audio_encode_driver_in_transfering_state_recv_suspend_event,
            audio_encode_driver_in_transfering_state_recv_resume_event,
            no_op,
            audio_encode_driver_in_transfering_state_recv_deliver_event
        },
        {
            audio_encode_driver_in_done_state_recv_close_event,
            audio_encode_driver_in_done_state_recv_timeout_event,
            audio_encode_driver_in_done_state_recv_suspend_event,
            audio_encode_driver_in_done_state_recv_resume_event,
            no_op,
            audio_encode_driver_in_done_state_recv_deliver_event
        },
    },
    .sync_hook = NULL,
    .sync_config_to_running_hook = audio_encode_driver_sync_config_to_running_hook,
    .sync_running_to_config_hook = audio_encode_driver_sync_running_to_config_hook,
};

static void audio_chan_gen_open_event_in_thread_context(isil_audio_driver_t *audio_driver)
{
    if(audio_driver != NULL){
        ed_tcb_t        *opened_logic_chan_ed;
        unsigned long   flags;

        spin_lock_irqsave(&audio_driver->fsm_matrix_call_lock, flags);
        opened_logic_chan_ed = &audio_driver->opened_logic_chan_ed;
        driver_gen_open_event(opened_logic_chan_ed, 1);
        spin_unlock_irqrestore(&audio_driver->fsm_matrix_call_lock, flags);        
   }
}

static void audio_chan_gen_close_event_in_thread_context(isil_audio_driver_t *audio_driver)
{
    if(audio_driver != NULL){
        ed_tcb_t        *opened_logic_chan_ed;
        isil_ed_fsm_t   *ed_fsm;
        unsigned long   flags;

        spin_lock_irqsave(&audio_driver->fsm_matrix_call_lock, flags);
        opened_logic_chan_ed = &audio_driver->opened_logic_chan_ed;
        ed_fsm = &opened_logic_chan_ed->ed_fsm;
        ed_fsm->op->feed_state_watchdog(ed_fsm);
        driver_gen_close_event(opened_logic_chan_ed, 1);
        spin_unlock_irqrestore(&audio_driver->fsm_matrix_call_lock, flags);        
   }
}

static void audio_chan_gen_suspend_event_in_thread_context(isil_audio_driver_t *audio_driver)
{
    if(audio_driver != NULL){
        ed_tcb_t        *opened_logic_chan_ed;
        unsigned long   flags;

        spin_lock_irqsave(&audio_driver->fsm_matrix_call_lock, flags);
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
        opened_logic_chan_ed = &audio_driver->opened_logic_chan_ed;
        driver_gen_suspend_event(opened_logic_chan_ed, 1);
        spin_unlock_irqrestore(&audio_driver->fsm_matrix_call_lock, flags);        
   }
}

static void audio_chan_gen_resume_event_in_thread_context(isil_audio_driver_t *audio_driver)
{
    if(audio_driver != NULL){
        ed_tcb_t        *opened_logic_chan_ed;
        unsigned long   flags;

        spin_lock_irqsave(&audio_driver->fsm_matrix_call_lock, flags);
        opened_logic_chan_ed = &audio_driver->opened_logic_chan_ed;
        driver_gen_resume_event(opened_logic_chan_ed, 1);
        spin_unlock_irqrestore(&audio_driver->fsm_matrix_call_lock, flags);        
   }
}

static int  isil_audio_encode_hcd_interface_open(ed_tcb_t *ed_tcb)
{
    int ret = ISIL_ERR;
    if(ed_tcb != NULL){
        isil_audio_driver_t *audio_driver;

        audio_driver = to_isil_audio_chan_driver_with_opened_logic_chan_ed(ed_tcb);
        if(atomic_read(&audio_driver->opened_flag) == 0){
            isil_chip_t *chip;

            isil_audio_chan_wait_robust_process_done(audio_driver);
            chip = audio_driver->chip_audio->chip;
            chip->chip_open(chip);
            audio_chan_gen_open_event_in_thread_context(audio_driver);
            ret = ISIL_OK;
        } else {
            ISIL_DBG(ISIL_DBG_ERR, "%d, channel have been opened\n\n", audio_driver->audio_logic_chan_id);
        }
    }
    return ret;
}

static int  isil_audio_encode_hcd_interface_close(ed_tcb_t *ed_tcb)
{
    int	ret = ISIL_ERR;

    if(ed_tcb != NULL){
        isil_audio_driver_t *audio_driver;

        audio_driver = to_isil_audio_chan_driver_with_opened_logic_chan_ed(ed_tcb);
        isil_audio_chan_wait_robust_process_done(audio_driver);
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
        if(atomic_read(&audio_driver->opened_flag)){
            isil_chip_t     *chip;
            chip_audio_t    *chip_audio;
            isil_send_msg_controller_t  *send_msg_contr;

            chip_audio = audio_driver->chip_audio;
            chip = chip_audio->chip;
            audio_chan_gen_close_event_in_thread_context(audio_driver);
            ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
            unregister_opened_logic_encode_chan_from_chip_audio_slot(audio_driver, chip_audio);
            send_msg_contr = &audio_driver->send_msg_contr;
            send_msg_contr->op->delete_send_msg_controller(send_msg_contr);
            ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
            isil_audio_chan_robust_process_done(audio_driver);
            if(atomic_read(&audio_driver->opened_flag) == 0){
                chip->chip_close(chip);
            }
        }
        remove_isil_audio_packet_queue(&audio_driver->audio_packet_queue, &audio_driver->audio_buf_pool);
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
        ret = ISIL_OK;
    }
    return ret;
}

static void isil_audio_encode_hcd_interface_suspend(ed_tcb_t *ed_tcb)
{
    if(ed_tcb != NULL){
        isil_register_node_t    *ed;
        isil_audio_driver_t     *audio_driver;

        ed = &ed_tcb->ed;
        ed->op->init_wait_suspend_complete(ed);
        audio_driver = to_isil_audio_chan_driver_with_opened_logic_chan_ed(ed_tcb);
        audio_chan_gen_suspend_event_in_thread_context(audio_driver);
        ed->op->wait_suspend_complete(ed);
        ed->op->suspend_complete_done(ed);
    }
}

static void isil_audio_encode_hcd_interface_resume(ed_tcb_t *ed_tcb)
{
    if(ed_tcb != NULL){
        isil_register_node_t	*ed;
        isil_audio_driver_t     *audio_driver;

        ed = &ed_tcb->ed;
        ed->op->init_wait_resume_complete(ed);
        audio_driver = to_isil_audio_chan_driver_with_opened_logic_chan_ed(ed_tcb);
        audio_chan_gen_resume_event_in_thread_context(audio_driver);
        ed->op->wait_resume_complete(ed);
        ed->op->resume_complete_done(ed);
    }
}

static int  isil_audio_encode_hcd_interface_ioctl(ed_tcb_t *ed_tcb, unsigned int cmd, unsigned long arg)
{
    isil_audio_driver_t *audio_driver;
    chip_audio_encode_t *audio_encode;
    isil_audio_param_t  *running_param;
    isil_audio_config_descriptor_t  *config_param;
    struct isil_chip_audio_param    audio_param;
    int ret = ISIL_OK, need_sync=0;

    ISIL_DBG(ISIL_DBG_INFO, "%c, %d\n", _IOC_TYPE(cmd), _IOC_NR(cmd));

    if(ed_tcb) {
        audio_driver = to_isil_audio_chan_driver_with_opened_logic_chan_ed(ed_tcb);
        isil_audio_chan_wait_robust_process_done(audio_driver);
        audio_encode = &audio_driver->chip_audio->audio_encode;
        config_param = &audio_encode->config_param;
        running_param = &audio_encode->running_param;
        switch(cmd) {
            case ISIL_CHIP_AUDIO_ENCODE_PARAM_SET:
                if(copy_from_user(&audio_param, (void *)arg, sizeof(struct isil_chip_audio_param)) != 0){
                    printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);                    
                } else {
                    unsigned long   flags;
                    spin_lock_irqsave(&audio_driver->fsm_matrix_call_lock, flags);
                    if(audio_param.change_mask_flag & ISIL_CHIP_AUDIO_ENCODE_PARAM_ENABLE_CHANGE_BIT_WIDE_MASK) {
                        config_param->bit_wide =  audio_param.i_bit_wide;
                        need_sync = 1;
                    }
                    if(audio_param.change_mask_flag & ISIL_CHIP_AUDIO_ENCODE_PARAM_ENABLE_CHANGE_SAMPLE_RATE_MASK) {
                        config_param->sample_rate =  audio_param.i_sample_rate;
                        need_sync = 1;
                    }
                    if(audio_param.change_mask_flag & ISIL_CHIP_AUDIO_ENCODE_PARAM_ENABLE_CHANGE_ENCODE_TYPE_MASK) {
                        config_param->type =  audio_param.e_audio_type;
                        need_sync = 1;
                    }
                    if(need_sync){
                        atomic_set(&audio_encode->need_sync_param, 1);
                    }
                    direct_driver_config_to_running(ed_tcb);
                    spin_unlock_irqrestore(&audio_driver->fsm_matrix_call_lock, flags);
                    ISIL_DBG(ISIL_DBG_INFO, "bit_wide = %d, sample_reate = %d, type = %d\n", config_param->bit_wide, config_param->sample_rate, config_param->type);
                }
                break;
            case ISIL_CHIP_AUDIO_ENCODE_PARAM_GET:
                audio_param.i_bit_wide = running_param->op->get_bit_wide(running_param);
                audio_param.i_sample_rate = running_param->op->get_sample_rate(running_param);
                audio_param.e_audio_type = running_param->op->get_type(running_param);
                if(copy_to_user((void *)arg, &audio_param, sizeof(struct isil_chip_audio_param)) != 0){
                    printk("%s, %d: copy_to_user failed\n", __FUNCTION__, __LINE__);                    
                }
                ret = ISIL_OK;
                break;
            case ISIL_LOGIC_CHAN_ENABLE_SET:
                ed_tcb->op->resume(ed_tcb);
                ret = ISIL_OK;
                break;
            case ISIL_LOGIC_CHAN_DISABLE_SET:
                ed_tcb->op->suspend(ed_tcb);
                ret = ISIL_OK;
                break;
            case ISIL_CODEC_CHAN_FLUSH:
                {
                    unsigned long   flags;
                    spin_lock_irqsave(&audio_driver->fsm_matrix_call_lock, flags);
                    flush_all_audio_encode_frame(audio_driver);
                    spin_unlock_irqrestore(&audio_driver->fsm_matrix_call_lock, flags);
                }
                break; 
            default:
                ISIL_DBG(ISIL_DBG_ERR, "bad command\n");
                ret = -EBADRQC;
                break;
        }
    }

    return ret;
}

static int  isil_audio_encode_hcd_interface_get_state(ed_tcb_t *ed_tcb)
{
    int	ret = ISIL_ED_UNREGISTER;
    if(ed_tcb != NULL){
        ret = ed_tcb->ed_fsm.op->get_curr_state(&ed_tcb->ed_fsm);
    }
    return ret;
}

static void isil_audio_hcd_interface_suspend_done(ed_tcb_t *opened_logic_chan_ed)
{
    if(opened_logic_chan_ed != NULL){
        isil_register_node_t    *ed;
        ed = &opened_logic_chan_ed->ed;
        ed->op->suspend_complete_done(ed);
    }
}

static void isil_audio_hcd_interface_resume_done(ed_tcb_t *opened_logic_chan_ed)
{
    if(opened_logic_chan_ed != NULL){
        isil_register_node_t    *ed;
        ed = &opened_logic_chan_ed->ed;
        ed->op->resume_complete_done(ed);
    }
}

static void isil_audio_hcd_interface_close_done(ed_tcb_t *opened_logic_chan_ed)
{
    if(opened_logic_chan_ed != NULL){
        isil_register_node_t    *ed;
        ed = &opened_logic_chan_ed->ed;
        ed->op->all_complete_done(ed);
    }
}

static isil_hcd_interface_operation   isil_audio_encode_hcd_interface_op = {
    .open = isil_audio_encode_hcd_interface_open,
    .close = isil_audio_encode_hcd_interface_close,
    .suspend = isil_audio_encode_hcd_interface_suspend,
    .resume = isil_audio_encode_hcd_interface_resume,
    .ioctl = isil_audio_encode_hcd_interface_ioctl,
    .get_state = isil_audio_encode_hcd_interface_get_state,
    .suspend_done = isil_audio_hcd_interface_suspend_done,
    .resume_done = isil_audio_hcd_interface_resume_done,
    .close_done = isil_audio_hcd_interface_close_done,
};

static void audio_encode_section_param_init(audio_section_ptr_info_t *audio_section_param)
{
    if(audio_section_param != NULL) {
        audio_section_param->audio_section_queue_header_ptr = INVALID_ISIL_AUDIO_SECTION_ID;
        audio_section_param->audio_section_queue_header_ptr_ext_flag = 0;
        audio_section_param->last_audio_section_queue_header_ptr_ext_flag = 0;
        audio_section_param->audio_section_queue_tailer_ptr = INVALID_ISIL_AUDIO_SECTION_ID;
        audio_section_param->audio_section_queue_tailer_ptr_ext_flag = 0;
        audio_section_param->section_queue_size = 0;
        audio_section_param->discard_number = AUDIO_IN_ADPCM_CHAN_SECTION_NUMBER;
    }
}

static void audio_encode_section_param_update_header_ptr(audio_section_ptr_info_t *audio_section_param, chip_audio_t *chip_audio)
{
    if((audio_section_param!=NULL) && (chip_audio!=NULL)){
        chip_audio_encode_t *audio_encode;
        isil_audio_driver_t   *audio_driver;
        int chan_id;
        audio_driver = to_get_audio_chan_driver_with_audio_section(audio_section_param);
        chan_id = audio_driver->audio_logic_chan_id;
        audio_encode = &chip_audio->audio_encode;
        if(audio_encode->op != NULL){
            audio_section_param->audio_section_queue_header_ptr = audio_encode->op->get_audio_encode_chan_wr_ptr(audio_encode, chan_id, &audio_section_param->audio_section_queue_header_ptr_ext_flag, &audio_section_param->section_queue_size);
        } else {
            audio_section_param->audio_section_queue_header_ptr = INVALID_ISIL_AUDIO_SECTION_ID;
            audio_section_param->audio_section_queue_header_ptr_ext_flag = 0;
            audio_section_param->section_queue_size = 0;
        }
    }
}

static void audio_encode_section_param_update_tailer_ptr(audio_section_ptr_info_t *audio_section_param, chip_audio_t *chip_audio)
{
    if((audio_section_param!=NULL) && (chip_audio!=NULL)){
        chip_audio_encode_t *audio_encode;
        isil_audio_driver_t   *audio_driver;
        int chan_id;

        audio_driver = to_get_audio_chan_driver_with_audio_section(audio_section_param);
        chan_id = audio_driver->audio_logic_chan_id;
        audio_encode = &chip_audio->audio_encode;
        if(audio_encode->op != NULL) {
            audio_section_param->audio_section_queue_tailer_ptr = audio_encode->op->get_audio_encode_chan_rd_ptr(audio_encode, chan_id, &audio_section_param->audio_section_queue_tailer_ptr_ext_flag, &audio_section_param->section_queue_size);
        } else {
            audio_section_param->audio_section_queue_tailer_ptr = INVALID_ISIL_AUDIO_SECTION_ID;
            audio_section_param->audio_section_queue_tailer_ptr_ext_flag = 0;
            audio_section_param->section_queue_size = 0;
        }
    }
}

static int  audio_encode_section_param_queue_capacity(audio_section_ptr_info_t *audio_section_param)
{
    int ret = 0;
    if(audio_section_param != NULL) {
        if(audio_section_param->section_queue_size > 0) {
            isil_audio_driver_t   *audio_driver;
            audio_driver = to_get_audio_chan_driver_with_audio_section(audio_section_param);
            if(atomic_read(&audio_driver->audio_sync_discard_number) > 0){
                atomic_dec(&audio_driver->audio_sync_discard_number);
            } else {
                ret = audio_section_param->audio_section_queue_header_ptr + audio_section_param->section_queue_size - audio_section_param->audio_section_queue_tailer_ptr;
                ret %= audio_section_param->section_queue_size;
                if(ret == 0){
                    if(audio_section_param->audio_section_queue_tailer_ptr_ext_flag != audio_section_param->audio_section_queue_header_ptr_ext_flag){
                        ret = 1;
                    }
                } else if (ret < 2){
                    ret = 0;
                } else {
                    ret = 1;
                }            
            }
            /*ISIL_DBG(ISIL_DBG_FATAL, "%d-%d: %d, %d, %d, %d, %d\n", audio_driver->audio_logic_chan_id,
              atomic_read(&audio_driver->audio_sync_discard_number),
              audio_section_param->audio_section_queue_header_ptr, 
              audio_section_param->audio_section_queue_header_ptr_ext_flag,
              audio_section_param->audio_section_queue_tailer_ptr,
              audio_section_param->audio_section_queue_tailer_ptr_ext_flag,
              ret);*/
        } else {
            printk("%s.%d: queue size is 0\n", __FILE__, __LINE__);
        }
    }
    return ret;
}

static void audio_encode_section_param_update_curr_section_offset(audio_section_ptr_info_t *audio_section_param, chip_audio_t *chip_audio)
{
    if((audio_section_param!=NULL) && (chip_audio!=NULL)){
        isil_audio_driver_t *audio_driver;
        chip_audio_encode_t *audio_encode;
        isil_audio_param_t  *running_param;
        audio_pcm_encode_data_addr_t    *pcm_audio_data_base;
        audio_adpcm_encode_data_addr_t  *adpcm_audio_data_base;
        int chan_id;

        if(audio_section_param->section_queue_size > 0) {
            audio_driver = to_get_audio_chan_driver_with_audio_section(audio_section_param);
            chan_id = audio_driver->audio_logic_chan_id;
            audio_encode = &chip_audio->audio_encode;
            running_param = &audio_encode->running_param;
            if(running_param->op!= NULL){
                switch(running_param->op->get_type(running_param)){
                    case ISIL_AUDIO_PCM:
                        pcm_audio_data_base = &audio_encode->pcm_audio_data_base;
                        if(pcm_audio_data_base->op != NULL) {
                            pcm_audio_data_base->op->set_reg_bit_pcm_type(pcm_audio_data_base);
                            pcm_audio_data_base->op->set_reg_bit_chan_id(pcm_audio_data_base, chan_id);
                            pcm_audio_data_base->op->set_reg_bit_section_id(pcm_audio_data_base, audio_section_param->audio_section_queue_tailer_ptr);
                            pcm_audio_data_base->op->set_reg_bit_section_offset(pcm_audio_data_base, 0);
                            audio_section_param->page_id = pcm_audio_data_base->op->get_buf_page_id(pcm_audio_data_base);
                            audio_section_param->chip_a_or_b = pcm_audio_data_base->op->get_buf_in_chip_a_or_b(pcm_audio_data_base);
                            audio_section_param->ddr_end_addr = pcm_audio_data_base->op->get_pcm_encode_data_addr_in_ddr_end_offset(pcm_audio_data_base);
                        }
                        break;
                    case ISIL_AUDIO_ADPCM_32K:
                        adpcm_audio_data_base = &audio_encode->adpcm_audio_data_base;
                        if(adpcm_audio_data_base->op != NULL) {
                            adpcm_audio_data_base->op->set_reg_bit_chan_id(adpcm_audio_data_base, chan_id);
                            adpcm_audio_data_base->op->set_reg_bit_adpcm_type(adpcm_audio_data_base);
                            adpcm_audio_data_base->op->set_reg_bit_section_id(adpcm_audio_data_base, audio_section_param->audio_section_queue_tailer_ptr);
                            adpcm_audio_data_base->op->set_reg_bit_section_offset(adpcm_audio_data_base, 0);
                            audio_section_param->page_id = adpcm_audio_data_base->op->get_buf_page_id(adpcm_audio_data_base);
                            audio_section_param->chip_a_or_b = adpcm_audio_data_base->op->get_buf_in_chip_a_or_b(adpcm_audio_data_base);
                            audio_section_param->ddr_end_addr = adpcm_audio_data_base->op->get_adpcm_encode_data_addr_in_ddr_end_offset(adpcm_audio_data_base);
                        }
                        break;
                    default:
                        printk("%s.%d: no surpport %d type\n", __FUNCTION__, __LINE__, running_param->op->get_type(running_param));
                        break;
                }
            }
        }
    }
}

static int  audio_encode_section_param_get_update_finish_id(audio_section_ptr_info_t *audio_section_param)
{
    isil_audio_driver_t *audio_driver = to_get_audio_chan_driver_with_audio_section(audio_section_param);
    return audio_driver->audio_logic_chan_id;
}

static struct audio_section_ptr_info_operation	audio_encode_section_param_op = {
    .init = audio_encode_section_param_init,
    .update_header_ptr = audio_encode_section_param_update_header_ptr,
    .update_tailer_ptr = audio_encode_section_param_update_tailer_ptr,
    .queue_capacity = audio_encode_section_param_queue_capacity,
    .update_curr_section_offset = audio_encode_section_param_update_curr_section_offset,
    .get_update_finish_id = audio_encode_section_param_get_update_finish_id,
};

static void audio_encode_chan_driver_section_init(audio_section_ptr_info_t *audio_section_param)
{
    if(audio_section_param != NULL){
        audio_section_param->op = &audio_encode_section_param_op;
        audio_section_param->op->init(audio_section_param);
    }
}

void    flush_audio_encode_curr_consumer_frame(isil_audio_driver_t *audio_driver)
{
    isil_audio_packet_queue_t   *audio_packet_queue;
    isil_audio_chan_buf_pool_t  *audio_chan_buf_pool;
    unsigned long   flags;

    spin_lock_irqsave(&audio_driver->fsm_matrix_call_lock, flags);
    audio_packet_queue = &audio_driver->audio_packet_queue;
    audio_chan_buf_pool = &audio_driver->audio_buf_pool;
    if(audio_driver->b_flush_all_frame & (1<<FLUSH_CURR_CONSUMER_FRAME)){
        audio_packet_queue->op->release_curr_consumer(audio_packet_queue, audio_chan_buf_pool);
        flush_audio_encode_frame_queue(audio_driver);
        audio_driver->b_flush_all_frame &= ~(1<<FLUSH_CURR_CONSUMER_FRAME);
    }
    spin_unlock_irqrestore(&audio_driver->fsm_matrix_call_lock, flags);
}

void    flush_audio_encode_curr_producer_frame(isil_audio_driver_t *audio_driver)
{
    isil_audio_packet_queue_t   *audio_packet_queue;
    isil_audio_chan_buf_pool_t  *audio_chan_buf_pool;

    audio_packet_queue = &audio_driver->audio_packet_queue;
    audio_chan_buf_pool = &audio_driver->audio_buf_pool;
    if(audio_driver->b_flush_all_frame & (1<<FLUSH_CURR_PRODUCER_FRAME)){
        audio_packet_queue->op->release_curr_producer(audio_packet_queue, audio_chan_buf_pool);
        audio_driver->b_flush_all_frame &= ~(1<<FLUSH_CURR_PRODUCER_FRAME);
    }
}

void    flush_audio_encode_frame_queue(isil_audio_driver_t *audio_driver)
{
    isil_audio_packet_queue_t   *audio_packet_queue;
    isil_audio_chan_buf_pool_t  *audio_chan_buf_pool;
    isil_audio_packet_section_t	*temp_frame;
    unsigned long   flags;

    audio_packet_queue = &audio_driver->audio_packet_queue;
    audio_chan_buf_pool = &audio_driver->audio_buf_pool;
    spin_lock_irqsave(&audio_packet_queue->lock, flags);
    while(audio_packet_queue->op->get_curr_queue_entry_number(audio_packet_queue)){
        audio_packet_queue->op->try_get(audio_packet_queue, &temp_frame);
        if(temp_frame != NULL){
            temp_frame->op->release(&temp_frame, audio_chan_buf_pool);
        } else {
            break;
        }
    }
    spin_unlock_irqrestore(&audio_packet_queue->lock, flags);
}

void    flush_all_audio_encode_frame(isil_audio_driver_t *audio_driver)
{
    audio_driver->b_flush_all_frame = ((1<<FLUSH_CURR_PRODUCER_FRAME)|(1<<FLUSH_CURR_CONSUMER_FRAME));
    flush_audio_encode_frame_queue(audio_driver);
}

void    reset_isil_audio_encode_chan(isil_audio_driver_t *audio_driver)
{
    if(audio_driver != NULL){
        audio_encode_chan_driver_section_init(&audio_driver->audio_section_param);
        audio_driver->timestamp.op->reset(&audio_driver->timestamp);
        audio_driver->i_frame_serial = 0;
        audio_driver->discard_number = 0;
        audio_driver->b_flush_all_frame = 0;
        atomic_set(&audio_driver->audio_sync_discard_number, 8);
    } 
}

static int  isil_audio_chan_push_interface_chan_is_open(isil_audio_chan_push_interface_t *audio_chan_push_interface, isil_audio_driver_t *audio_chan_driver)
{
    ed_tcb_t *ed;
    int	ret = ISIL_ERR;

    ed = &audio_chan_driver->opened_logic_chan_ed;
    if(atomic_read(&audio_chan_driver->opened_flag) == 0){
        return ret;
    }
    if(ed->op->get_state(ed) == ISIL_ED_STANDBY){
        ret = ISIL_OK;
    }
    return ret;
}

static void isil_audio_chan_push_interface_copy_data(isil_audio_chan_push_interface_t *audio_chan_push_interface, isil_audio_driver_t *audio_chan_driver, char *data, int len)
{
    isil_audio_packet_queue_t   *audio_packet_queue = &audio_chan_driver->audio_packet_queue;
    isil_timestamp_t            *timestamp = &audio_chan_driver->timestamp;
    audio_section_descriptor_t  *descriptor;
 
    descriptor = (audio_section_descriptor_t *)(data+AUDIO_SECTION_SIZE-sizeof(audio_section_descriptor_t));
#if	defined(PLATFORM_ENDIAN_DIFFERENT)
    //need swp byte order swap,because pci is little endian and processor is big endian
    *((u32*)descriptor) = in_le32((u32*)descriptor);
#endif

    timestamp->op->set_timestamp(timestamp, descriptor->timestamp, __FILE__);
    audio_packet_queue->op->try_get_curr_producer_from_pool(audio_packet_queue, &audio_chan_driver->audio_buf_pool);
    if(audio_packet_queue->curr_producer != NULL){
        isil_audio_packet_section_t   *audio_section = audio_packet_queue->curr_producer;

        audio_section->payload_len  = descriptor->valid_len;
        audio_section->sample_rate = descriptor->sample_rate;
        audio_section->type = descriptor->type;
        audio_section->bit_wide = descriptor->bit_wide;
        audio_section->timestamp = timestamp->op->get_timestamp(timestamp);
        audio_section->frameSerial = audio_chan_driver->i_frame_serial++;
        audio_section->payload_len = H264_MIN((audio_section->section_size-sizeof(ext_h264_nal_bitstream_t)), audio_section->payload_len);
        memcpy((audio_section->data + sizeof(ext_h264_nal_bitstream_t)), data, audio_section->payload_len);
        audio_section->payload_len = gen_ext_nal_audio_header(audio_section->data, audio_section->type, audio_section->payload_len);
        audio_packet_queue->op->put_curr_producer_into_queue(audio_packet_queue);
    } else {
        audio_chan_driver->discard_number++;
    }
}

static struct isil_audio_chan_push_interface_operation	isil_audio_chan_push_interface_op = {
    .chan_is_open = isil_audio_chan_push_interface_chan_is_open,
    .copy_data = isil_audio_chan_push_interface_copy_data,
};

static void	init_isil_audio_chan_push_interface(isil_audio_chan_push_interface_t *audio_chan_push_interface)
{
    if(audio_chan_push_interface != NULL){
        audio_chan_push_interface->op = &isil_audio_chan_push_interface_op;
    }
}

void    reset_isil_audio_encode_chan_preprocessing(ed_tcb_t *opened_logic_chan_ed)
{
    printk("%s.%d\n", __FUNCTION__, __LINE__);
    if(opened_logic_chan_ed != NULL){
        isil_audio_driver_t         *audio_driver = to_isil_audio_chan_driver_with_opened_logic_chan_ed(opened_logic_chan_ed);
        isil_audio_packet_queue_t   *audio_frame_queue = &audio_driver->audio_packet_queue;
        isil_send_msg_controller_t  *send_msg_contr = &audio_driver->send_msg_contr;
        isil_ed_fsm_t           *ed_fsm;

        printk("%s.%d\n", __FUNCTION__, __LINE__);
        if(atomic_read(&audio_driver->opened_flag)){
            ed_fsm = &opened_logic_chan_ed->ed_fsm;
            ed_fsm->op->change_state_for_robust(ed_fsm);
            opened_logic_chan_ed->op->suspend(opened_logic_chan_ed);
            printk("%s.%d\n", __FUNCTION__, __LINE__);
            audio_chan_gen_close_event_in_thread_context(audio_driver);
            printk("%s.%d\n", __FUNCTION__, __LINE__);
            unregister_opened_logic_encode_chan_from_chip_audio_slot(audio_driver, audio_driver->chip_audio);

            send_msg_contr->op->delete_send_msg_controller(send_msg_contr);
            audio_frame_queue->op->release_curr_producer(audio_frame_queue, &audio_driver->audio_buf_pool);
            ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
        }
        printk("%s.%d\n", __FUNCTION__, __LINE__);
    }
}

void    reset_isil_audio_encode_chan_postprocessing(ed_tcb_t *opened_logic_chan_ed)
{
    if(opened_logic_chan_ed != NULL){
        isil_audio_driver_t *audio_driver = to_isil_audio_chan_driver_with_opened_logic_chan_ed(opened_logic_chan_ed);
        isil_chip_t *chip;

        printk("%s.%d\n", __FUNCTION__, __LINE__);
        if(atomic_read(&audio_driver->opened_flag) == 0){
            chip = audio_driver->chip_audio->chip;
            chip->chip_open(chip);
        }
        audio_chan_gen_open_event_in_thread_context(audio_driver);
        audio_chan_gen_resume_event_in_thread_context(audio_driver);
        isil_audio_chan_robust_process_done(audio_driver);
    }
}

void    reset_isil_audio_encode_chan_processing(ed_tcb_t *opened_logic_chan_ed)
{
    if(opened_logic_chan_ed != NULL){
        isil_audio_driver_t *audio_driver = to_isil_audio_chan_driver_with_opened_logic_chan_ed(opened_logic_chan_ed);
        isil_chip_t *chip;

        printk("%s.%d\n", __FUNCTION__, __LINE__);
        chip =  audio_driver->chip_audio->chip;
        if(atomic_read(&audio_driver->opened_flag) == 0){
            chip->chip_close(chip);
        }
    }
}

void    start_audio_chan_robust_process(isil_audio_driver_t *audio_driver)
{
    robust_processing_control_t *audio_chan_robust;

    start_chip_audio_robust_process(audio_driver->chip_audio);
    audio_chan_robust = &audio_driver->audio_chan_robust;
    audio_chan_robust->op->start_robust_processing(audio_chan_robust);
}

void    isil_audio_chan_robust_process_done(isil_audio_driver_t *audio_driver)
{
    robust_processing_control_t *audio_chan_robust;

    audio_chan_robust = &audio_driver->audio_chan_robust;
    audio_chan_robust->op->robust_processing_done(audio_chan_robust);
}

void    isil_audio_chan_wait_robust_process_done(isil_audio_driver_t *audio_driver)
{
    robust_processing_control_t *audio_chan_robust;

    audio_chan_robust = &audio_driver->audio_chan_robust;
    audio_chan_robust->op->wait_robust_processing_done(audio_chan_robust);
    isil_chip_audio_wait_robust_process_done(audio_driver->chip_audio);
    chip_wait_robust_process_done(audio_driver->chip_audio->chip);
}

int isil_audio_chan_is_in_robust_processing(isil_audio_driver_t *audio_driver)
{
    robust_processing_control_t *audio_chan_robust;
    audio_chan_robust = &audio_driver->audio_chan_robust;
    return audio_chan_robust->op->is_in_robust_processing(audio_chan_robust);
}

static void audio_logic_encode_chan_driver_notify_myself(isil_register_node_t *node, void *priv, isil_notify_msg *msg)//chan recv async msg
{
    if((node!=NULL) && (priv!=NULL)){

    }
}

static int  audio_logic_encode_chan_driver_match_id(isil_register_node_t *node, void *priv, unsigned long logic_chan_id)
{
    int ret = ISIL_ERR;
    if((node!=NULL) && (priv!=NULL)){
        isil_audio_driver_t *audio_driver = (isil_audio_driver_t*)priv;
        if(audio_driver->audio_logic_chan_id == logic_chan_id){
            ret = ISIL_OK;
        }
    }
    return ret;
}

static void register_logic_encode_chan_into_chip_audio_slot(isil_audio_driver_t *audio_driver, chip_audio_t *chip_audio)
{
    if((audio_driver!=NULL) && (chip_audio!=NULL)){
        isil_register_table_t   *logic_chan_table = &chip_audio->logic_chan_table;
        ed_tcb_t                *logic_chan_ed = &audio_driver->logic_chan_ed;
        isil_register_node_t    *register_node = &logic_chan_ed->ed;

        logic_chan_table->op->register_node_into_table(logic_chan_table, register_node);
    }
}

static __used void find_register_logic_encode_chan_in_chip_audio_slot(isil_audio_driver_t **ptr_audio_driver, chip_audio_t *chip_audio, int logic_chan_id) 
{
    if((ptr_audio_driver!=NULL) && (chip_audio!=NULL)){
        isil_register_table_t   *logic_chan_table = &chip_audio->logic_chan_table;
        isil_register_node_t    *register_node;
        ed_tcb_t                *logic_chan_ed;

        *ptr_audio_driver = NULL;
        logic_chan_table->op->find_register_node_in_table(logic_chan_table, &register_node, logic_chan_id);
        if(register_node != NULL){
            logic_chan_ed = to_get_endpoint_tcb_with_register_node(register_node);
            *ptr_audio_driver = to_isil_audio_chan_driver_with_logic_chan_ed(logic_chan_ed);
        }
    }
}

static __used void  unregister_logic_encode_chan_from_chip_audio_slot(isil_audio_driver_t *audio_driver, chip_audio_t *chip_audio)
{
    if((audio_driver!=NULL) && (chip_audio!=NULL)){
        isil_register_table_t   *logic_chan_table = &chip_audio->logic_chan_table;
        ed_tcb_t                *logic_chan_ed = &audio_driver->logic_chan_ed;
        isil_register_node_t    *register_node = &logic_chan_ed->ed;

        register_node->op->all_complete_done(register_node);
        logic_chan_table->op->unregister_node_from_table(logic_chan_table, register_node);
    }
}

static void register_opened_logic_encode_chan_into_chip_audio_slot(isil_audio_driver_t *audio_driver, chip_audio_t *chip_audio)
{
    if((audio_driver!=NULL) && (chip_audio!=NULL)){
        isil_register_table_t   *opened_logic_chan_table = &chip_audio->opened_logic_chan_table;
        ed_tcb_t                *opened_logic_chan_ed = &audio_driver->opened_logic_chan_ed;
        isil_register_node_t    *register_node = &opened_logic_chan_ed->ed;

        atomic_inc(&audio_driver->opened_flag);
        opened_logic_chan_table->op->register_node_into_table(opened_logic_chan_table, register_node);
    }
}

void    find_register_opened_logic_encode_chan_in_chip_audio_slot(isil_audio_driver_t **ptr_audio_driver, chip_audio_t *chip_audio, int logic_chan_id)
{
    if((ptr_audio_driver!=NULL) && (chip_audio!=NULL)){
        isil_register_table_t   *opened_logic_chan_table = &chip_audio->opened_logic_chan_table;
        ed_tcb_t                *opened_logic_chan_ed;
        isil_register_node_t    *register_node;

        *ptr_audio_driver = NULL;
        opened_logic_chan_table->op->find_register_node_in_table(opened_logic_chan_table, &register_node, logic_chan_id);
        if(register_node != NULL){
            opened_logic_chan_ed = to_get_endpoint_tcb_with_register_node(register_node);
            *ptr_audio_driver = to_isil_audio_chan_driver_with_opened_logic_chan_ed(opened_logic_chan_ed);
        }
    }
}

static void unregister_opened_logic_encode_chan_from_chip_audio_slot(isil_audio_driver_t *audio_driver, chip_audio_t *chip_audio)
{
    if((audio_driver!=NULL) && (chip_audio!=NULL)){
        isil_register_table_t   *opened_logic_chan_table = &chip_audio->opened_logic_chan_table;
        ed_tcb_t                *opened_logic_chan_ed = &audio_driver->opened_logic_chan_ed;
        isil_register_node_t    *register_node = &opened_logic_chan_ed->ed;

        opened_logic_chan_table->op->unregister_node_from_table(opened_logic_chan_table, register_node);
    }
}

int init_isil_audio_encode_chan(isil_audio_driver_t *audio_driver, chip_audio_t *chip_audio, int bus_id, int chip_id, int group_chan_id)
{
    ed_tcb_t *opened_logic_chan_ed;
    int	ret = ISIL_ERR;
    if(audio_driver != NULL){
        init_isil_encode_timestamp(&audio_driver->timestamp);
        ret = init_audio_chan_buf_pool(&audio_driver->audio_buf_pool);
        init_isil_audio_packet_queue(&audio_driver->audio_packet_queue);
        audio_driver->type = ISIL_AUDIO_ENCODE;
        audio_driver->audio_logic_chan_id = group_chan_id;
        audio_driver->audio_phy_chan_id = group_chan_id;
        if(audio_driver->audio_logic_chan_id < ISIL_AUDIO_IN_CHAN16_ID){
            audio_driver->read_bitstream_mode = READ_BITSTREAM_BY_PCI;
        } else {
            audio_driver->read_bitstream_mode = READ_BITSTREAM_BY_DDR;        
        }
        audio_driver->read_bitstream_mode = READ_BITSTREAM_BY_DDR;

        audio_driver->chip_audio = chip_audio;
        audio_driver->isil_device_chan = NULL;

        init_endpoint_tcb(&audio_driver->logic_chan_ed, bus_id, chip_id, ISIL_ED_AUDIO_ENCODE_IN, group_chan_id, 0, audio_driver, audio_logic_encode_chan_driver_notify_myself, audio_logic_encode_chan_driver_match_id, &audio_encode_driver_fsm_state_transfer_matrix_table);
        opened_logic_chan_ed = &audio_driver->opened_logic_chan_ed;
        opened_logic_chan_ed->op = &isil_audio_encode_hcd_interface_op;
        init_endpoint_tcb(opened_logic_chan_ed, bus_id, chip_id, ISIL_ED_AUDIO_ENCODE_IN, group_chan_id, 0, audio_driver, audio_logic_encode_chan_driver_notify_myself, audio_logic_encode_chan_driver_match_id, &audio_encode_driver_fsm_state_transfer_matrix_table);

        init_dpram_request_service_tcb(&audio_driver->audio_bitstream_req);
        reset_isil_audio_encode_chan(audio_driver);
        spin_lock_init(&audio_driver->fsm_matrix_call_lock);
        atomic_set(&audio_driver->opened_flag, 0);
        init_isil_send_msg_controller(&audio_driver->send_msg_contr, ISIL_AUDIO_ENCODE_CHAN, audio_driver->audio_logic_chan_id, audio_driver);
        init_robust_processing_control(&audio_driver->audio_chan_robust);
        init_isil_audio_chan_push_interface(&audio_driver->audio_chan_push_interface);
        register_logic_encode_chan_into_chip_audio_slot(audio_driver, chip_audio);
    }
    return ret;
}

void    remove_isil_audio_encode_chan(isil_audio_driver_t *audio_driver)
{
    if(audio_driver != NULL){
        ed_tcb_t *opened_logic_chan_ed;

        opened_logic_chan_ed = &audio_driver->opened_logic_chan_ed;
        if(opened_logic_chan_ed->op != NULL){
            opened_logic_chan_ed->op->close(opened_logic_chan_ed);
        }
        remove_audio_chan_buf_pool(&audio_driver->audio_buf_pool);
        unregister_logic_encode_chan_from_chip_audio_slot(audio_driver, audio_driver->chip_audio);
    }
}

static int  audio_decode_driver_in_unregister_state_recv_close_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t *audio_driver;
        chip_audio_t    *chip_audio;

        audio_driver = (isil_audio_driver_t*)context;
        chip_audio = audio_driver->chip_audio;
        if(atomic_read(&audio_driver->opened_flag) > 0){
            if(isil_audio_chan_is_in_robust_processing(audio_driver)){
                ed_fsm->op->save_event_for_robust_processing(ed_fsm);
            }

            chip_audio->op->set_enable_audio_chan(chip_audio, audio_driver->audio_logic_chan_id, AUDIO_DISABLE);
            ed_fsm->op->reset(ed_fsm);
            atomic_dec(&audio_driver->opened_flag);
        }
        if(atomic_read(&audio_driver->opened_flag) == 0){
            ed_tcb_t    *ed_tcb;
            ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
            ed_tcb->op->close_done(ed_tcb);
        }
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
    }
    return ISIL_OK;
}

static int  audio_decode_driver_in_unregister_state_recv_suspend_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t   *audio_driver;
        ed_tcb_t    *ed_tcb;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        ed_tcb->op->suspend_done(ed_tcb);
        audio_driver = (isil_audio_driver_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
    }
    return ISIL_OK;
}

static int  audio_decode_driver_in_unregister_state_recv_resume_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t   *audio_driver;
        ed_tcb_t    *ed_tcb;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        ed_tcb->op->resume_done(ed_tcb);
        audio_driver = (isil_audio_driver_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
    }
    return ISIL_OK;
}

static int  audio_decode_driver_in_unregister_state_recv_open_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t *audio_driver;
        chip_audio_t        *chip_audio;

        audio_driver = (isil_audio_driver_t*)context;
        chip_audio = audio_driver->chip_audio;
        if(atomic_read(&audio_driver->opened_flag) == 0){
            register_opened_logic_decode_chan_into_chip_audio_slot(audio_driver, chip_audio);

            reset_isil_audio_decode_chan(audio_driver);
            ed_fsm->op->reset(ed_fsm);
            ed_fsm->op->change_state(ed_fsm, ISIL_ED_SUSPEND);

            if(isil_audio_chan_is_in_robust_processing(audio_driver)){
                ed_tcb_t    *ed_tcb;

                ed_fsm->op->restore_event_from_robust_processing(ed_fsm);
                ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
                if(have_any_pending_event(ed_tcb)){
                    driver_trigger_pending_event(ed_tcb);
                }
            }
        }
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
    }
    return ISIL_OK;
}

static int  audio_decode_driver_in_idle_state_recv_close_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t   *audio_driver;
        ed_tcb_t    *ed_tcb;

        ed_fsm->op->change_state(ed_fsm, ISIL_ED_UNREGISTER);
        audio_driver = (isil_audio_driver_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_close_event(ed_tcb, 1);
    }
    return ISIL_OK;
}

static int  audio_decode_driver_in_idle_state_recv_timeout_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t *audio_driver;

        audio_driver = (isil_audio_driver_t*)context;
        start_audio_chan_robust_process(audio_driver);
        audio_chan_gen_req_msg(audio_driver, REQ_ALGO_CHIP_RESET, NONBLOCK_OP);
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
    }
    return ISIL_OK;
}

static int  audio_decode_driver_in_idle_state_recv_suspend_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t *audio_driver;
        chip_audio_t    *chip_audio;
        ed_tcb_t        *ed_tcb;

        audio_driver = (isil_audio_driver_t*)context;
        chip_audio = audio_driver->chip_audio;
        ed_fsm->op->change_state(ed_fsm, ISIL_ED_SUSPEND);
        chip_audio->op->set_enable_audio_chan(chip_audio, audio_driver->audio_logic_chan_id, AUDIO_DISABLE);
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        ed_tcb->op->suspend_done(ed_tcb);
    }
    return ISIL_OK;
}

static int  audio_decode_driver_in_idle_state_recv_resume_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t   *audio_driver;
        ed_tcb_t    *ed_tcb;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        ed_tcb->op->resume_done(ed_tcb);
        audio_driver = (isil_audio_driver_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
    }
    return ISIL_OK;
}

static int  audio_decode_driver_in_idle_state_recv_deliver_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        chip_audio_t                *chip_audio;
        isil_chip_t                 *chip;
        chip_audio_decode_t         *audio_decode;
        isil_audio_driver_t         *audio_chan_driver;
        isil_audio_packet_queue_t   *audio_packet_queue;
        audio_section_ptr_info_t    *audio_section_param;
        dpram_control_t             *chip_dpram_controller;
        dpram_request_t             *send_audio_bitstream_req;

        audio_chan_driver = (isil_audio_driver_t*)context;
        chip_audio = audio_chan_driver->chip_audio;
        if(isil_chip_audio_is_in_robust_processing(chip_audio)){
            return ISIL_OK;
        }

        chip = chip_audio->chip;
        audio_decode = &chip_audio->audio_decode;
        audio_decode->op->get_audio_decode_param(audio_decode, chip);

        audio_packet_queue = &audio_chan_driver->audio_packet_queue;
        audio_section_param = &audio_chan_driver->audio_section_param;
        audio_section_param->op->update_header_ptr(audio_section_param, chip_audio);
        audio_section_param->op->update_tailer_ptr(audio_section_param, chip_audio);
        if(audio_section_param->op->queue_capacity(audio_section_param)){
            audio_section_param->op->update_curr_section_offset(audio_section_param, chip_audio);
            ed_fsm->op->change_state(ed_fsm, ISIL_ED_STANDBY);
            send_audio_bitstream_req = &audio_chan_driver->audio_bitstream_req;
            init_submit_send_audio_bitstream_service(send_audio_bitstream_req, audio_chan_driver);
            get_isil_dpram_controller(&chip_dpram_controller);
            chip_dpram_controller->op->submit_write_audio_req(chip_dpram_controller, audio_chan_driver);
        }
    }
    return ISIL_OK;
}

static int  audio_decode_driver_in_standby_state_recv_close_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t *audio_driver;
        chip_audio_t        *chip_audio;
        isil_chip_t         *chip;
        dpram_control_t     *chip_dpram_controller;
        ed_tcb_t            *ed_tcb;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        audio_driver = (isil_audio_driver_t*)context;
        chip_audio = audio_driver->chip_audio;
        chip = chip_audio->chip;
        get_isil_dpram_controller(&chip_dpram_controller);
        if(chip_dpram_controller->op->delete_write_audio_req(chip_dpram_controller, audio_driver)){
            ed_fsm->op->change_state(ed_fsm, ISIL_ED_UNREGISTER);
            driver_gen_close_event(ed_tcb, 1);         
        } else {
            driver_gen_close_event(ed_tcb, 0);
            ISIL_DBG(ISIL_DBG_INFO, "%d, prevent crit scene\n", audio_driver->audio_logic_chan_id);
        }
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
    }
    return ISIL_OK;
}

static int  audio_decode_driver_in_standby_state_recv_timeout_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t *audio_driver;
        dpram_control_t *chip_dpram_controller;

        audio_driver = (isil_audio_driver_t*)context;
        start_audio_chan_robust_process(audio_driver);
        get_isil_dpram_controller(&chip_dpram_controller);
        chip_dpram_controller->op->delete_write_audio_req(chip_dpram_controller, audio_driver);
        audio_chan_gen_req_msg(audio_driver, REQ_ALGO_CHIP_RESET, NONBLOCK_OP);
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
    }
    return ISIL_OK;
}

static int  audio_decode_driver_in_standby_state_recv_suspend_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t *audio_driver;
        dpram_control_t *chip_dpram_controller;
        chip_audio_t    *chip_audio;
        isil_chip_t     *chip;
        ed_tcb_t        *ed_tcb;

        audio_driver = (isil_audio_driver_t*)context;
        chip_audio = audio_driver->chip_audio;
        chip = chip_audio->chip;
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        get_isil_dpram_controller(&chip_dpram_controller);
        if(chip_dpram_controller->op->delete_write_audio_req(chip_dpram_controller, audio_driver)){
            chip_audio->op->set_enable_audio_chan(chip_audio, audio_driver->audio_logic_chan_id, AUDIO_DISABLE);
            ed_fsm->op->change_state(ed_fsm, ISIL_ED_SUSPEND);
            ed_tcb->op->suspend_done(ed_tcb);
        } else {
            driver_gen_suspend_event(ed_tcb, 0);
            ISIL_DBG(ISIL_DBG_INFO, "%d, prevent crit scene\n", audio_driver->audio_logic_chan_id);
        }
    }
    return ISIL_OK;
}

static int  audio_decode_driver_in_standby_state_recv_deliver_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        ed_fsm->op->change_state(ed_fsm, ISIL_ED_RUNNING);
    }
    return ISIL_OK;
}

static int  audio_decode_driver_in_suspend_state_recv_close_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t   *audio_driver;
        ed_tcb_t    *ed_tcb;

        ed_fsm->op->change_state(ed_fsm, ISIL_ED_UNREGISTER);
        audio_driver = (isil_audio_driver_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_close_event(ed_tcb, 1);
    }
    return ISIL_OK;
}

static int  audio_decode_driver_in_suspend_state_recv_suspend_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t   *audio_driver;
        ed_tcb_t    *ed_tcb;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        ed_tcb->op->suspend_done(ed_tcb);
        audio_driver = (isil_audio_driver_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
    }
    return ISIL_OK;
}

static int  audio_decode_driver_in_suspend_state_recv_resume_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t *audio_driver;
        chip_audio_t    *chip_audio;
        ed_tcb_t    *ed_tcb;

        audio_driver = (isil_audio_driver_t*)context;
        chip_audio = audio_driver->chip_audio;
        ed_fsm->op->change_state(ed_fsm, ISIL_ED_IDLE);
        chip_audio->op->set_enable_audio_chan(chip_audio, audio_driver->audio_logic_chan_id, AUDIO_ENABLE);
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        ed_tcb->op->resume_done(ed_tcb);
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
    }
    return ISIL_OK;
}

static int  audio_decode_driver_in_running_state_recv_close_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        ed_tcb_t    *ed_tcb;
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_close_event(ed_tcb, 0);
    }
    return ISIL_OK;
}

static int  audio_decode_driver_in_running_state_recv_timeout_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t *audio_driver;

        audio_driver = (isil_audio_driver_t*)context;
        start_audio_chan_robust_process(audio_driver);
        if(audio_driver->read_bitstream_mode == READ_BITSTREAM_BY_DDR){
            dpram_control_t *chip_dpram_controller;
            isil_chip_t     *chip;

            chip = audio_driver->chip_audio->chip;
            get_isil_dpram_controller(&chip_dpram_controller);
            chip_free_irq(chip, IRQ_BURST_TYPE_INTR, audio_driver);
            chip_dpram_controller->op->release_end_move_data_from_dpram_to_ddr_service_req(chip_dpram_controller, &audio_driver->dpram_page, chip);
        }
        audio_chan_gen_req_msg(audio_driver, REQ_ALGO_CHIP_RESET, NONBLOCK_OP);
        ed_fsm->op->change_state(ed_fsm, ISIL_ED_DONE);
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
    }
    return ISIL_OK;
}

static int  audio_decode_driver_in_running_state_recv_suspend_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        ed_tcb_t    *ed_tcb;
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_suspend_event(ed_tcb, 0);
    }
    return ISIL_OK;
}

static int  audio_decode_driver_in_running_state_recv_resume_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        ed_tcb_t    *ed_tcb;
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_resume_event(ed_tcb, 0);
    }
    return ISIL_OK;
}

static int  audio_decode_driver_in_running_state_recv_deliver_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        ed_fsm->op->change_state(ed_fsm, ISIL_ED_TRANSFERING);
    }
    return ISIL_OK;
}

static int  audio_decode_driver_in_transfering_state_recv_close_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        ed_tcb_t    *ed_tcb;
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_close_event(ed_tcb, 0);
    }
    return ISIL_OK;
}

static int  audio_decode_driver_in_transfering_state_recv_timeout_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t *audio_driver;

        audio_driver = (isil_audio_driver_t*)context;
        start_audio_chan_robust_process(audio_driver);
        if(audio_driver->read_bitstream_mode == READ_BITSTREAM_BY_DDR){
            dpram_control_t *chip_dpram_controller;
            isil_chip_t     *chip;

            chip = audio_driver->chip_audio->chip;
            get_isil_dpram_controller(&chip_dpram_controller);
            chip_free_irq(chip, IRQ_BURST_TYPE_INTR, audio_driver);
            chip_dpram_controller->op->release_end_move_data_from_dpram_to_ddr_service_req(chip_dpram_controller, &audio_driver->dpram_page, chip);
        }
        audio_chan_gen_req_msg(audio_driver, REQ_ALGO_CHIP_RESET, NONBLOCK_OP);
        ed_fsm->op->change_state(ed_fsm, ISIL_ED_DONE);
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
    }
    return ISIL_OK;
}

static int  audio_decode_driver_in_transfering_state_recv_suspend_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        ed_tcb_t    *ed_tcb;
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_suspend_event(ed_tcb, 0);
    }
    return ISIL_OK;
}

static int  audio_decode_driver_in_transfering_state_recv_resume_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        ed_tcb_t    *ed_tcb;
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_resume_event(ed_tcb, 0);
    }
    return ISIL_OK;
}

static int  audio_decode_driver_in_transfering_state_recv_deliver_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        ed_tcb_t    *ed_tcb;
        isil_audio_driver_t   *audio_driver;

        audio_driver = (isil_audio_driver_t*)context;
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

static int  audio_decode_driver_in_done_state_recv_close_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t   *audio_driver;
        ed_tcb_t    *ed_tcb;

        ed_fsm->op->change_state(ed_fsm, ISIL_ED_UNREGISTER);
        audio_driver = (isil_audio_driver_t*)context;
        printk("%s.%d, %d\n", __FUNCTION__, __LINE__, audio_driver->audio_logic_chan_id);
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_close_event(ed_tcb, 1);
    }
    return ISIL_OK;
}

static int  audio_decode_driver_in_done_state_recv_timeout_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t *audio_driver;

        audio_driver = (isil_audio_driver_t*)context;
        start_audio_chan_robust_process(audio_driver);
        audio_chan_gen_req_msg(audio_driver, REQ_ALGO_CHIP_RESET, NONBLOCK_OP);
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
    }
    return ISIL_OK;
}

static int  audio_decode_driver_in_done_state_recv_suspend_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t *audio_driver;
        chip_audio_t    *chip_audio;
        ed_tcb_t        *ed_tcb;

        audio_driver = (isil_audio_driver_t*)context;
        chip_audio = audio_driver->chip_audio;
        chip_audio->op->set_enable_audio_chan(chip_audio, audio_driver->audio_logic_chan_id, AUDIO_DISABLE);
        ed_fsm->op->change_state(ed_fsm, ISIL_ED_SUSPEND);
        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        ed_tcb->op->suspend_done(ed_tcb);
    }
    return ISIL_OK;
}

static int  audio_decode_driver_in_done_state_recv_resume_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t *audio_driver;
        ed_tcb_t    *ed_tcb;

        ed_tcb = to_get_endpoint_tcb_with_isil_ed_fsm(ed_fsm);
        driver_gen_deliver_event(ed_tcb, 1);
        ed_tcb->op->resume_done(ed_tcb);
        audio_driver = (isil_audio_driver_t*)context;
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
    }
    return ISIL_OK;
}

static int  audio_decode_driver_in_done_state_recv_deliver_event(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        ed_fsm->op->change_state(ed_fsm, ISIL_ED_IDLE);
    }
    return ISIL_OK;
}

static int  audio_decode_driver_sync_config_to_running_hook(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t *audio_driver;
        chip_audio_t        *chip_audio;
        chip_audio_decode_t *audio_decode;
        isil_audio_config_descriptor_t  *config_param;
        isil_audio_param_t  *running_param;

        audio_driver = (isil_audio_driver_t*)context;
        chip_audio = audio_driver->chip_audio;
        audio_decode = &chip_audio->audio_decode;
        config_param = &audio_decode->config_param;
        running_param = &audio_decode->running_param;

        if(atomic_read(&audio_decode->need_sync_param)){
            running_param->op->update_bit_wide(running_param, config_param->bit_wide);
            running_param->op->update_sample_rate(running_param, config_param->sample_rate);
            running_param->op->update_type(running_param, config_param->type);
            atomic_set(&audio_decode->need_sync_param, 0);
        }
    }
    return ISIL_OK;
}

static int  audio_decode_driver_sync_running_to_config_hook(isil_ed_fsm_t *ed_fsm, void *context)
{
    if((ed_fsm!=NULL) && (context!=NULL)){
        isil_audio_driver_t *audio_driver;
        chip_audio_t        *chip_audio;
        chip_audio_decode_t *audio_decode;
        isil_audio_config_descriptor_t  *config_param;
        isil_audio_param_t  *running_param;

        audio_driver = (isil_audio_driver_t*)context;
        chip_audio = audio_driver->chip_audio;
        audio_decode = &chip_audio->audio_decode;
        config_param = &audio_decode->config_param;
        running_param = &audio_decode->running_param;

        config_param->bit_wide =  running_param->op->get_bit_wide(running_param);
        config_param->sample_rate =  running_param->op->get_sample_rate(running_param);
        config_param->type =  running_param->op->get_type(running_param);
        atomic_set(&audio_decode->need_sync_param, 0);
    }
    return ISIL_OK;
}

static fsm_state_transfer_matrix_table_t    audio_decode_driver_fsm_state_transfer_matrix_table = {
    .action = {
        {
            audio_decode_driver_in_unregister_state_recv_close_event, 
            no_op,
            audio_decode_driver_in_unregister_state_recv_suspend_event,
            audio_decode_driver_in_unregister_state_recv_resume_event,
            audio_decode_driver_in_unregister_state_recv_open_event,
            no_op
        },
        {
            audio_decode_driver_in_idle_state_recv_close_event, 
            audio_decode_driver_in_idle_state_recv_timeout_event,
            audio_decode_driver_in_idle_state_recv_suspend_event,
            audio_decode_driver_in_idle_state_recv_resume_event,
            no_op,
            audio_decode_driver_in_idle_state_recv_deliver_event
        },
        {
            audio_decode_driver_in_standby_state_recv_close_event, 
            audio_decode_driver_in_standby_state_recv_timeout_event,
            audio_decode_driver_in_standby_state_recv_suspend_event,
            no_op,
            no_op,
            audio_decode_driver_in_standby_state_recv_deliver_event
        },
        {
            audio_decode_driver_in_suspend_state_recv_close_event,
            no_op,
            audio_decode_driver_in_suspend_state_recv_suspend_event,
            audio_decode_driver_in_suspend_state_recv_resume_event,
            no_op,
            no_op
        },
        {
            audio_decode_driver_in_running_state_recv_close_event,
            audio_decode_driver_in_running_state_recv_timeout_event,
            audio_decode_driver_in_running_state_recv_suspend_event,
            audio_decode_driver_in_running_state_recv_resume_event,
            no_op,
            audio_decode_driver_in_running_state_recv_deliver_event
        },
        {
            audio_decode_driver_in_transfering_state_recv_close_event,
            audio_decode_driver_in_transfering_state_recv_timeout_event,
            audio_decode_driver_in_transfering_state_recv_suspend_event,
            audio_decode_driver_in_transfering_state_recv_resume_event,
            no_op,
            audio_decode_driver_in_transfering_state_recv_deliver_event
        },
        {
            audio_decode_driver_in_done_state_recv_close_event,
            audio_decode_driver_in_done_state_recv_timeout_event,
            audio_decode_driver_in_done_state_recv_suspend_event,
            audio_decode_driver_in_done_state_recv_resume_event,
            no_op,
            audio_decode_driver_in_done_state_recv_deliver_event
        },
    },
    .sync_hook = NULL,
    .sync_config_to_running_hook = audio_decode_driver_sync_config_to_running_hook,
    .sync_running_to_config_hook = audio_decode_driver_sync_running_to_config_hook,
};

static int  isil_audio_decode_hcd_interface_open(ed_tcb_t *ed_tcb)
{
    int ret = ISIL_ERR;
    if(ed_tcb != NULL){
        isil_audio_driver_t *audio_driver;

        audio_driver = to_isil_audio_chan_driver_with_opened_logic_chan_ed(ed_tcb);
        if(atomic_read(&audio_driver->opened_flag) == 0){
            isil_chip_t *chip;

            isil_audio_chan_wait_robust_process_done(audio_driver);
            chip = audio_driver->chip_audio->chip;
            chip->chip_open(chip);
            audio_chan_gen_open_event_in_thread_context(audio_driver);
            ret = ISIL_OK;
        } else {
            ISIL_DBG(ISIL_DBG_ERR, "%d, channel have been opened\n\n", audio_driver->audio_logic_chan_id);
        }
        ret = ISIL_OK;
    }
    return ret;
}

static int  isil_audio_decode_hcd_interface_close(ed_tcb_t *ed_tcb)
{
    int	ret = ISIL_ERR;

    if(ed_tcb != NULL){
        isil_audio_driver_t         *audio_driver;
        isil_send_msg_controller_t  *send_msg_contr;

        audio_driver = to_isil_audio_chan_driver_with_opened_logic_chan_ed(ed_tcb);
        isil_audio_chan_wait_robust_process_done(audio_driver);
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
        if(atomic_read(&audio_driver->opened_flag)){
            isil_chip_t     *chip;
            chip_audio_t    *chip_audio;

            chip_audio = audio_driver->chip_audio;
            chip = chip_audio->chip;
            audio_chan_gen_close_event_in_thread_context(audio_driver);
            unregister_opened_logic_decode_chan_from_chip_audio_slot(audio_driver, chip_audio);

            send_msg_contr = &audio_driver->send_msg_contr;
            send_msg_contr->op->delete_send_msg_controller(send_msg_contr);
            if(isil_audio_chan_is_in_robust_processing(audio_driver) == 0){
                isil_audio_chan_robust_process_done(audio_driver);
            }
            ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
            if(atomic_read(&audio_driver->opened_flag) == 0){
                chip->chip_close(chip);
            }
        }
        remove_isil_audio_packet_queue(&audio_driver->audio_packet_queue, &audio_driver->audio_buf_pool);
        ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
        ret = ISIL_OK;
    }
    return ret;
}

static void isil_audio_decode_hcd_interface_suspend(ed_tcb_t *ed_tcb)
{
    if(ed_tcb != NULL){
        isil_register_node_t    *ed;
        isil_audio_driver_t     *audio_driver;

        ed = &ed_tcb->ed;
        ed->op->init_wait_suspend_complete(ed);
        audio_driver = to_isil_audio_chan_driver_with_opened_logic_chan_ed(ed_tcb);
        audio_chan_gen_suspend_event_in_thread_context(audio_driver);
        ed->op->wait_suspend_complete(ed);
        ed->op->suspend_complete_done(ed);
    }
}

static void isil_audio_decode_hcd_interface_resume(ed_tcb_t *ed_tcb)
{
    if(ed_tcb != NULL){
        isil_register_node_t	*ed;
        isil_audio_driver_t     *audio_driver;

        ed = &ed_tcb->ed;
        ed->op->init_wait_resume_complete(ed);
        audio_driver = to_isil_audio_chan_driver_with_opened_logic_chan_ed(ed_tcb);
        audio_chan_gen_resume_event_in_thread_context(audio_driver);
        ed->op->wait_resume_complete(ed);
        ed->op->resume_complete_done(ed);
    }
}

static int  isil_audio_decode_hcd_interface_ioctl(ed_tcb_t *ed_tcb, unsigned int cmd, unsigned long arg)
{
    isil_audio_driver_t *audio_driver;
    chip_audio_decode_t *audio_decode;
    isil_audio_param_t  *running_param;
    isil_audio_config_descriptor_t  *config_param;
    struct isil_chip_audio_param    audio_param;
    int ret = 0, need_sync=0;

    ISIL_DBG(ISIL_DBG_INFO, "%c, %d\n", _IOC_TYPE(cmd), _IOC_NR(cmd));
    audio_driver = to_isil_audio_chan_driver_with_opened_logic_chan_ed(ed_tcb);
    audio_decode = &audio_driver->chip_audio->audio_decode;
    config_param = &audio_decode->config_param;
    running_param = &audio_decode->running_param;
    if(ed_tcb/* && arg*/) {
        isil_audio_chan_wait_robust_process_done(audio_driver);
        switch(cmd) {
            case ISIL_CHIP_AUDIO_DECODE_PARAM_SET:
                if(copy_from_user(&audio_param, (void *)arg, sizeof(struct isil_chip_audio_param)) != 0){
                    printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);                    
                } else {
                    unsigned long   flags;
                    spin_lock_irqsave(&audio_driver->fsm_matrix_call_lock, flags);
                    if(audio_param.change_mask_flag & ISIL_CHIP_AUDIO_DECODE_PARAM_ENABLE_CHANGE_BIT_WIDE_MASK) {
                        config_param->bit_wide =  audio_param.i_bit_wide;
                        need_sync = 1;
                    }
                    if(audio_param.change_mask_flag & ISIL_CHIP_AUDIO_DECODE_PARAM_ENABLE_CHANGE_SAMPLE_RATE_MASK) {
                        config_param->sample_rate =  audio_param.i_sample_rate;
                        need_sync = 1;
                    }
                    if(audio_param.change_mask_flag & ISIL_CHIP_AUDIO_DECODE_PARAM_ENABLE_CHANGE_ENCODE_TYPE_MASK) {
                        config_param->type =  audio_param.e_audio_type;
                        need_sync = 1;
                    }
                    if(need_sync) {
                        atomic_set(&audio_decode->need_sync_param, 1);
                    }
                    direct_driver_config_to_running(ed_tcb);
                    spin_unlock_irqrestore(&audio_driver->fsm_matrix_call_lock, flags);
                }
                break;
            case ISIL_CHIP_AUDIO_DECODE_PARAM_GET:
                audio_param.i_bit_wide = running_param->op->get_bit_wide(running_param);
                audio_param.i_sample_rate = running_param->op->get_sample_rate(running_param);
                audio_param.e_audio_type = running_param->op->get_type(running_param);
                if(copy_to_user((void *)arg, &audio_param, sizeof(struct isil_chip_audio_param)) != 0){
                    printk("%s, %d: copy_to_user failed\n", __FUNCTION__, __LINE__);                
                }
                ret = ISIL_OK;
                break;
            case ISIL_LOGIC_CHAN_ENABLE_SET:
                ed_tcb->op->resume(ed_tcb);
                ret = ISIL_OK;
                break;
            case ISIL_LOGIC_CHAN_DISABLE_SET:
                ed_tcb->op->suspend(ed_tcb);
                ret = ISIL_OK;
                break;
            case ISIL_CODEC_CHAN_FLUSH:
                {
                    unsigned long   flags;
                    spin_lock_irqsave(&audio_driver->fsm_matrix_call_lock, flags);
                    flush_all_audio_decode_frame(audio_driver);
                    spin_unlock_irqrestore(&audio_driver->fsm_matrix_call_lock, flags);
                }
                break; 
            default:
                ret = -EBADRQC;
                break;
        }
    }

    return ret;
}

static int  isil_audio_decode_hcd_interface_get_state(ed_tcb_t *ed_tcb)
{
    int	ret = ISIL_ED_UNREGISTER;
    if(ed_tcb != NULL){
        ret = atomic_read(&ed_tcb->state);
    }
    return ret;
}

static isil_hcd_interface_operation	isil_audio_decode_hcd_interface_op = {
    .open = isil_audio_decode_hcd_interface_open,
    .close = isil_audio_decode_hcd_interface_close,
    .suspend = isil_audio_decode_hcd_interface_suspend,
    .resume = isil_audio_decode_hcd_interface_resume,
    .ioctl = isil_audio_decode_hcd_interface_ioctl,
    .get_state = isil_audio_decode_hcd_interface_get_state,
    .suspend_done = isil_audio_hcd_interface_suspend_done,
    .resume_done = isil_audio_hcd_interface_resume_done,
    .close_done = isil_audio_hcd_interface_close_done,};

static void audio_decode_section_param_init(audio_section_ptr_info_t *audio_section_param)
{
    if(audio_section_param != NULL) {
        audio_section_param->audio_section_queue_header_ptr = INVALID_ISIL_AUDIO_SECTION_ID;
        audio_section_param->audio_section_queue_header_ptr_ext_flag = 0;
        audio_section_param->last_audio_section_queue_header_ptr_ext_flag = 0;
        audio_section_param->audio_section_queue_tailer_ptr = INVALID_ISIL_AUDIO_SECTION_ID;
        audio_section_param->audio_section_queue_tailer_ptr_ext_flag = 0;
        audio_section_param->section_queue_size = 0;
        audio_section_param->discard_number = 0;
    }
}

static void audio_decode_section_param_update_header_ptr(audio_section_ptr_info_t *audio_section_param, chip_audio_t *chip_audio)
{
    if((audio_section_param!=NULL) && (chip_audio!=NULL)){
        chip_audio_decode_t *audio_decode;
        isil_audio_driver_t   *audio_driver;

        audio_driver = to_get_audio_chan_driver_with_audio_section(audio_section_param);
        audio_decode = &chip_audio->audio_decode;
        if(audio_decode->op != NULL){
            audio_section_param->audio_section_queue_header_ptr = audio_decode->op->get_audio_decode_chan_wr_ptr(audio_decode, audio_driver->audio_logic_chan_id, &audio_section_param->audio_section_queue_header_ptr_ext_flag, &audio_section_param->section_queue_size);
        } else {
            audio_section_param->audio_section_queue_header_ptr = INVALID_ISIL_AUDIO_SECTION_ID;
            audio_section_param->audio_section_queue_header_ptr_ext_flag = 0;
            audio_section_param->section_queue_size = 0;
        }
    }
}

static void audio_decode_section_param_update_tailer_ptr(audio_section_ptr_info_t *audio_section_param, chip_audio_t *chip_audio)
{
    if((audio_section_param!=NULL) && (chip_audio!=NULL)){
        chip_audio_decode_t *audio_decode;
        isil_audio_driver_t   *audio_driver;

        audio_driver = to_get_audio_chan_driver_with_audio_section(audio_section_param);
        audio_decode = &chip_audio->audio_decode;
        if(audio_decode->op != NULL){
            audio_section_param->audio_section_queue_tailer_ptr = audio_decode->op->get_audio_decode_chan_rd_ptr(audio_decode, audio_driver->audio_logic_chan_id, &audio_section_param->audio_section_queue_tailer_ptr_ext_flag, &audio_section_param->section_queue_size);
        } else {
            audio_section_param->audio_section_queue_tailer_ptr = INVALID_ISIL_AUDIO_SECTION_ID;
            audio_section_param->audio_section_queue_tailer_ptr_ext_flag = 0;
            audio_section_param->section_queue_size = 0;
        }
    }
}

static int  audio_decode_section_param_queue_capacity(audio_section_ptr_info_t *audio_section_param)
{
    int	ret = 0;
    if(audio_section_param->section_queue_size > 0){
        isil_audio_driver_t   *audio_driver;
        audio_driver = to_get_audio_chan_driver_with_audio_section(audio_section_param);
        if(atomic_read(&audio_driver->audio_sync_discard_number) > 0){
            atomic_dec(&audio_driver->audio_sync_discard_number);
        } else {
            ret = audio_section_param->audio_section_queue_tailer_ptr + audio_section_param->section_queue_size - audio_section_param->audio_section_queue_header_ptr - 1;
            ret %= audio_section_param->section_queue_size;
            if(ret == 0){
                if(audio_section_param->audio_section_queue_tailer_ptr_ext_flag == audio_section_param->audio_section_queue_header_ptr_ext_flag){
                    ret = 1;
                }
            }
        }
        /*ISIL_DBG(ISIL_DBG_FATAL, "%d-%d: %d, %d, %d, %d, %d\n", audio_driver->audio_logic_chan_id,
          atomic_read(&audio_driver->audio_sync_discard_number),
          audio_section_param->audio_section_queue_header_ptr, 
          audio_section_param->audio_section_queue_header_ptr_ext_flag,
          audio_section_param->audio_section_queue_tailer_ptr,
          audio_section_param->audio_section_queue_tailer_ptr_ext_flag,
          ret);*/
    }
    return ret;
}

static void audio_decode_section_param_update_curr_section_offset(audio_section_ptr_info_t *audio_section_param, chip_audio_t *chip_audio)
{
    if((audio_section_param!=NULL) && (chip_audio!=NULL)){
        isil_audio_driver_t *audio_driver;
        chip_audio_decode_t *audio_decode;
        isil_audio_param_t  *running_param;
        audio_decode_data_addr_t    *adpcm_audio_data_base;
        int     chan_id;

        if(audio_section_param->audio_section_queue_header_ptr != INVALID_ISIL_AUDIO_SECTION_ID){
            audio_driver = to_get_audio_chan_driver_with_audio_section(audio_section_param);
            chan_id = audio_driver->audio_logic_chan_id-ISIL_AUDIO_OUT_SPEAKER_ID;
            audio_decode = &chip_audio->audio_decode;
            running_param = &audio_decode->running_param;
            if(running_param->op != NULL){
                switch(running_param->op->get_type(running_param)){
                    case ISIL_AUDIO_PCM:
                    case ISIL_AUDIO_ADPCM_32K:
                        adpcm_audio_data_base = &audio_decode->audio_data_base;
                        if(adpcm_audio_data_base->op != NULL){
                            adpcm_audio_data_base->op->set_reg_bit_chan_id(adpcm_audio_data_base, chan_id);
                            adpcm_audio_data_base->op->set_reg_bit_decode_type(adpcm_audio_data_base);
                            adpcm_audio_data_base->op->set_reg_bit_section_id(adpcm_audio_data_base, audio_section_param->audio_section_queue_header_ptr);
                            adpcm_audio_data_base->op->set_reg_bit_section_offset(adpcm_audio_data_base, 0);
                            audio_section_param->page_id = adpcm_audio_data_base->op->get_buf_page_id(adpcm_audio_data_base);
                            audio_section_param->chip_a_or_b = adpcm_audio_data_base->op->get_buf_in_chip_a_or_b(adpcm_audio_data_base);
                            audio_section_param->ddr_end_addr = adpcm_audio_data_base->op->get_adpcm_decode_data_addr_in_ddr_end_offset(adpcm_audio_data_base);
                        }
                        break;
                    default:
                        printk("%s.%d: no surpport %d type\n", __FUNCTION__, __LINE__, running_param->op->get_type(running_param));
                        break;
                }
            }
        }
    }
}

static int  audio_decode_section_param_get_update_finish_id(audio_section_ptr_info_t *audio_section_param)
{
    return ((audio_section_param->audio_section_queue_header_ptr_ext_flag<<(get_power_base((u32)AUDIO_OUT_CHAN_SECTION_NUMBER)))|audio_section_param->audio_section_queue_header_ptr);
}

static struct audio_section_ptr_info_operation	audio_decode_section_param_op = {
    .init = audio_decode_section_param_init,
    .update_header_ptr = audio_decode_section_param_update_header_ptr,
    .update_tailer_ptr = audio_decode_section_param_update_tailer_ptr,
    .queue_capacity = audio_decode_section_param_queue_capacity,
    .update_curr_section_offset = audio_decode_section_param_update_curr_section_offset,
    .get_update_finish_id = audio_decode_section_param_get_update_finish_id,
};

static void audio_decode_chan_driver_section_init(audio_section_ptr_info_t *audio_section_param)
{
    if(audio_section_param != NULL){
        audio_section_param->op = &audio_decode_section_param_op;
        audio_section_param->op->init(audio_section_param);
    }
}

void    flush_audio_decode_curr_consumer_frame(isil_audio_driver_t *audio_driver)
{
    isil_audio_packet_queue_t   *audio_packet_queue;
    isil_audio_chan_buf_pool_t  *audio_chan_buf_pool;

    audio_packet_queue = &audio_driver->audio_packet_queue;
    audio_chan_buf_pool = &audio_driver->audio_buf_pool;
    if(audio_driver->b_flush_all_frame & (1<<FLUSH_CURR_CONSUMER_FRAME)){
        audio_packet_queue->op->release_curr_consumer(audio_packet_queue, audio_chan_buf_pool);
        flush_audio_decode_frame_queue(audio_driver);
        audio_driver->b_flush_all_frame &= ~(1<<FLUSH_CURR_CONSUMER_FRAME);
    }
}

void    flush_audio_decode_curr_producer_frame(isil_audio_driver_t *audio_driver)
{
    isil_audio_packet_queue_t   *audio_packet_queue;
    isil_audio_chan_buf_pool_t  *audio_chan_buf_pool;

    audio_packet_queue = &audio_driver->audio_packet_queue;
    audio_chan_buf_pool = &audio_driver->audio_buf_pool;
    if(audio_driver->b_flush_all_frame & (1<<FLUSH_CURR_PRODUCER_FRAME)){
        audio_packet_queue->op->release_curr_producer(audio_packet_queue, audio_chan_buf_pool);
        audio_driver->b_flush_all_frame &= ~(1<<FLUSH_CURR_PRODUCER_FRAME);
    }
}

void    flush_audio_decode_frame_queue(isil_audio_driver_t *audio_driver)
{
    isil_audio_packet_queue_t   *audio_packet_queue;
    isil_audio_chan_buf_pool_t  *audio_chan_buf_pool;
    isil_audio_packet_section_t	*temp_frame;
    unsigned long   flags;

    audio_packet_queue = &audio_driver->audio_packet_queue;
    audio_chan_buf_pool = &audio_driver->audio_buf_pool;
    spin_lock_irqsave(&audio_packet_queue->lock, flags);
    while(audio_packet_queue->op->get_curr_queue_entry_number(audio_packet_queue)){
        audio_packet_queue->op->try_get(audio_packet_queue, &temp_frame);
        if(temp_frame != NULL){
            temp_frame->op->release(&temp_frame, audio_chan_buf_pool);
        } else {
            break;
        }
    }
    spin_unlock_irqrestore(&audio_packet_queue->lock, flags);
}

void    flush_all_audio_decode_frame(isil_audio_driver_t *audio_driver)
{
    audio_driver->b_flush_all_frame = ((1<<FLUSH_CURR_PRODUCER_FRAME)|(1<<FLUSH_CURR_CONSUMER_FRAME));
    flush_audio_decode_curr_producer_frame(audio_driver);
    flush_audio_decode_curr_consumer_frame(audio_driver);
}

void    reset_isil_audio_decode_chan_preprocessing(ed_tcb_t *opened_logic_chan_ed)
{
    if(opened_logic_chan_ed != NULL){
        isil_audio_driver_t *audio_driver = to_isil_audio_chan_driver_with_opened_logic_chan_ed(opened_logic_chan_ed);
        isil_audio_packet_queue_t   *audio_frame_queue = &audio_driver->audio_packet_queue;
        isil_send_msg_controller_t  *send_msg_contr = &audio_driver->send_msg_contr;
        isil_ed_fsm_t       *ed_fsm;

        printk("%s.%d\n", __FUNCTION__, __LINE__);
        if(atomic_read(&audio_driver->opened_flag)){
            ed_fsm = &opened_logic_chan_ed->ed_fsm;
            ed_fsm->op->change_state_for_robust(ed_fsm);
            opened_logic_chan_ed->op->suspend(opened_logic_chan_ed);
            audio_chan_gen_close_event_in_thread_context(audio_driver);
            unregister_opened_logic_decode_chan_from_chip_audio_slot(audio_driver, audio_driver->chip_audio);

            send_msg_contr->op->delete_send_msg_controller(send_msg_contr);
            audio_frame_queue->op->release_curr_consumer(audio_frame_queue, &audio_driver->audio_buf_pool);
            ISIL_DBG(ISIL_DBG_INFO, "%d\n", audio_driver->audio_logic_chan_id);
        }
        printk("%s.%d\n", __FUNCTION__, __LINE__);
    }
}

void    reset_isil_audio_decode_chan_postprocessing(ed_tcb_t *opened_logic_chan_ed)
{
    if(opened_logic_chan_ed != NULL){
        isil_audio_driver_t *audio_driver = to_isil_audio_chan_driver_with_opened_logic_chan_ed(opened_logic_chan_ed);
        isil_chip_t     *chip;

        if(atomic_read(&audio_driver->opened_flag) == 0){
            chip = audio_driver->chip_audio->chip;
            chip->chip_open(chip);
        }
        audio_chan_gen_open_event_in_thread_context(audio_driver);
        audio_chan_gen_resume_event_in_thread_context(audio_driver);
        isil_audio_chan_robust_process_done(audio_driver);
    }
}

void    reset_isil_audio_decode_chan_processing(ed_tcb_t *opened_logic_chan_ed)
{
    if(opened_logic_chan_ed != NULL){
        isil_audio_driver_t *audio_driver = to_isil_audio_chan_driver_with_opened_logic_chan_ed(opened_logic_chan_ed);
        isil_chip_t *chip;

        printk("%s.%d\n", __FUNCTION__, __LINE__);
        chip =  audio_driver->chip_audio->chip;
        if(atomic_read(&audio_driver->opened_flag) == 0){
            chip->chip_close(chip);
        }
    }
}

void    reset_isil_audio_decode_chan(isil_audio_driver_t *audio_driver)
{
    if(audio_driver != NULL){
        audio_driver->timestamp.op->reset(&audio_driver->timestamp);
        audio_driver->i_frame_serial = 0;
        audio_driver->discard_number = 0;
        audio_decode_chan_driver_section_init(&audio_driver->audio_section_param);
        atomic_set(&audio_driver->audio_sync_discard_number, 0);
    } 
}

static void audio_logic_decode_chan_driver_notify_myself(isil_register_node_t *node, void *priv, isil_notify_msg *msg)//chan recv async msg
{
    if((node!=NULL) && (priv!=NULL)){

    }
}

static int  audio_logic_decode_chan_driver_match_id(isil_register_node_t *node, void *priv, unsigned long logic_chan_id)
{
    int ret = ISIL_ERR;
    if((node!=NULL) && (priv!=NULL)){
        isil_audio_driver_t *audio_driver = (isil_audio_driver_t*)priv;
        if(audio_driver->audio_logic_chan_id == logic_chan_id){
            ret = ISIL_OK;
        }
    }
    return ret;
}

static void register_logic_decode_chan_into_chip_audio_slot(isil_audio_driver_t *audio_driver, chip_audio_t *chip_audio)
{
    if((audio_driver!=NULL) && (chip_audio!=NULL)){
        isil_register_table_t   *logic_chan_table = &chip_audio->logic_chan_table;
        ed_tcb_t                *logic_chan_ed = &audio_driver->logic_chan_ed;
        isil_register_node_t    *register_node = &logic_chan_ed->ed;

        logic_chan_table->op->register_node_into_table(logic_chan_table, register_node);
    }
}

static __used void find_register_logic_decode_chan_in_chip_audio_slot(isil_audio_driver_t **ptr_audio_driver, chip_audio_t *chip_audio, int logic_chan_id) 
{
    if((ptr_audio_driver!=NULL) && (chip_audio!=NULL)){
        isil_register_table_t   *logic_chan_table = &chip_audio->logic_chan_table;
        isil_register_node_t    *register_node;
        ed_tcb_t                *logic_chan_ed;

        *ptr_audio_driver = NULL;
        logic_chan_table->op->find_register_node_in_table(logic_chan_table, &register_node, logic_chan_id);
        if(register_node != NULL){
            logic_chan_ed = to_get_endpoint_tcb_with_register_node(register_node);
            *ptr_audio_driver = to_isil_audio_chan_driver_with_logic_chan_ed(logic_chan_ed);
        }
    }
}

static void  unregister_logic_decode_chan_from_chip_audio_slot(isil_audio_driver_t *audio_driver, chip_audio_t *chip_audio)
{
    if((audio_driver!=NULL) && (chip_audio!=NULL)){
        isil_register_table_t   *logic_chan_table = &chip_audio->logic_chan_table;
        ed_tcb_t                *logic_chan_ed = &audio_driver->logic_chan_ed;
        isil_register_node_t    *register_node = &logic_chan_ed->ed;

        register_node->op->all_complete_done(register_node);
        logic_chan_table->op->unregister_node_from_table(logic_chan_table, register_node);
    }
}

static void register_opened_logic_decode_chan_into_chip_audio_slot(isil_audio_driver_t *audio_driver, chip_audio_t *chip_audio)
{
    if((audio_driver!=NULL) && (chip_audio!=NULL)){
        isil_register_table_t   *opened_logic_chan_table = &chip_audio->opened_logic_chan_table;
        ed_tcb_t                *opened_logic_chan_ed = &audio_driver->opened_logic_chan_ed;
        isil_register_node_t    *register_node = &opened_logic_chan_ed->ed;

        atomic_inc(&audio_driver->opened_flag);
        opened_logic_chan_table->op->register_node_into_table(opened_logic_chan_table, register_node);
    }
}

void    find_register_opened_logic_decode_chan_in_chip_audio_slot(isil_audio_driver_t **ptr_audio_driver, chip_audio_t *chip_audio, int logic_chan_id)
{
    if((ptr_audio_driver!=NULL) && (chip_audio!=NULL)){
        isil_register_table_t   *opened_logic_chan_table = &chip_audio->opened_logic_chan_table;
        ed_tcb_t                *opened_logic_chan_ed;
        isil_register_node_t    *register_node;

        *ptr_audio_driver = NULL;
        opened_logic_chan_table->op->find_register_node_in_table(opened_logic_chan_table, &register_node, logic_chan_id);
        if(register_node != NULL){
            opened_logic_chan_ed = to_get_endpoint_tcb_with_register_node(register_node);
            *ptr_audio_driver = to_isil_audio_chan_driver_with_opened_logic_chan_ed(opened_logic_chan_ed);
        }
    }
}

static void unregister_opened_logic_decode_chan_from_chip_audio_slot(isil_audio_driver_t *audio_driver, chip_audio_t *chip_audio)
{
    if((audio_driver!=NULL) && (chip_audio!=NULL)){
        isil_register_table_t   *opened_logic_chan_table = &chip_audio->opened_logic_chan_table;
        ed_tcb_t                *opened_logic_chan_ed = &audio_driver->opened_logic_chan_ed;
        isil_register_node_t    *register_node = &opened_logic_chan_ed->ed;

        opened_logic_chan_table->op->unregister_node_from_table(opened_logic_chan_table, register_node);
    }
}

int init_isil_audio_decode_chan(isil_audio_driver_t *audio_driver, chip_audio_t *chip_audio, int bus_id, int chip_id, int group_chan_id)
{
    ed_tcb_t *opened_logic_chan_ed;
    int	ret = ISIL_ERR;
    if(audio_driver != NULL){
        init_isil_encode_timestamp(&audio_driver->timestamp);
        atomic_set(&audio_driver->opened_flag, 0);
        init_robust_processing_control(&audio_driver->audio_chan_robust);
        ret = init_audio_chan_buf_pool(&audio_driver->audio_buf_pool);
        if(ret == ISIL_OK){
            init_isil_audio_packet_queue(&audio_driver->audio_packet_queue);
            audio_decode_chan_driver_section_init(&audio_driver->audio_section_param);
            init_endpoint_tcb(&audio_driver->logic_chan_ed, bus_id, chip_id, ISIL_ED_AUDIO_DECODE_OUT, group_chan_id, 0, audio_driver, audio_logic_decode_chan_driver_notify_myself, audio_logic_decode_chan_driver_match_id, &audio_decode_driver_fsm_state_transfer_matrix_table);
            opened_logic_chan_ed = &audio_driver->opened_logic_chan_ed;
            opened_logic_chan_ed->op = &isil_audio_decode_hcd_interface_op;
            init_endpoint_tcb(opened_logic_chan_ed, bus_id, chip_id, ISIL_ED_AUDIO_DECODE_OUT, group_chan_id, 0, audio_driver, audio_logic_decode_chan_driver_notify_myself, audio_logic_decode_chan_driver_match_id, &audio_decode_driver_fsm_state_transfer_matrix_table);
            audio_driver->type = ISIL_AUDIO_DECODE;
            audio_driver->audio_logic_chan_id = group_chan_id;
            audio_driver->read_bitstream_mode = READ_BITSTREAM_BY_DDR;        
            audio_driver->chip_audio = chip_audio;
            audio_driver->isil_device_chan = NULL;
        }
        init_dpram_request_service_tcb(&audio_driver->audio_bitstream_req);
        reset_isil_audio_decode_chan(audio_driver);
        audio_driver->i_frame_serial = 0;
        audio_driver->discard_number = 0;
        spin_lock_init(&audio_driver->fsm_matrix_call_lock);
        atomic_set(&audio_driver->opened_flag, 0);
        init_isil_send_msg_controller(&audio_driver->send_msg_contr, ISIL_AUDIO_DECODE_CHAN, audio_driver->audio_logic_chan_id, audio_driver);
        register_logic_decode_chan_into_chip_audio_slot(audio_driver, chip_audio);
    }
    return ret;
}

void    remove_isil_audio_decode_chan(isil_audio_driver_t *audio_driver)
{
    if(audio_driver != NULL){
        ed_tcb_t *opened_logic_chan_ed;

        opened_logic_chan_ed = &audio_driver->opened_logic_chan_ed;
        if(opened_logic_chan_ed->op != NULL){
            opened_logic_chan_ed->op->close(opened_logic_chan_ed);
        }
        remove_audio_chan_buf_pool(&audio_driver->audio_buf_pool);
        unregister_logic_decode_chan_from_chip_audio_slot(audio_driver, audio_driver->chip_audio);
    }
}

