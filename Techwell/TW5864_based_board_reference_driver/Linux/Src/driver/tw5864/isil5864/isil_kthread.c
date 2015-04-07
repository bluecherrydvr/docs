#include	<isil5864/isil_common.h>

static struct task_struct   *algo_kthread=NULL;
static atomic_t algo_running_flag;

void process_slice_header_req(isil_h264_logic_encode_chan_t *encode_chan)
{
    isil_chip_t *chip;
    isil_h264_encode_property_t *encode_property;
    isil_h264_encode_feature_t  *encode_feature;
    isil_h264_encode_rc_t       *encode_rc;
    isil_h264_encode_control_t	*encode_control;
    isil_h264_phy_video_slot_t  *master_slot;
    isil_video_chan_buf_pool_t	*encode_chan_buf_pool;
    isil_video_frame_tcb_queue_t    *encode_frame_queue;
    isil_video_frame_tcb_t          *video_frame=NULL;
    isil_video_packet_tcb_queue_t   *video_packet_queue;
    isil_video_packet_tcb_t         *packet;
    ed_tcb_t    *opened_logic_chan_ed;
    isil_kthread_msg_queue_t    *isil_msg_queue;
    isil_send_msg_controller_t  *send_msg_contr;
    chip_driver_t               *chip_driver;
    isil_video_bus_t            *video_bus;
    int need_init_rc_flag = 0, need_discard_orig_frame = 0;
    unsigned long   flags;

    if(encode_chan == NULL){
        ISIL_DBG(ISIL_DBG_FATAL, "null pointer\n");
        return;
    }

    spin_lock_irqsave(&encode_chan->fsm_matrix_call_lock, flags);
    chip = encode_chan->chip;
    encode_control = &encode_chan->encode_control;
    encode_property = &encode_chan->encode_property;
    encode_feature  = &encode_property->encode_feature;
    encode_rc       = &encode_property->encode_rc;
    opened_logic_chan_ed = &encode_chan->opened_logic_chan_ed;
    encode_chan_buf_pool = &encode_chan->encode_chan_buf_pool;
    encode_frame_queue = &encode_chan->encode_frame_queue;
    send_msg_contr = &encode_chan->send_msg_contr;
    get_msg_queue_header(&isil_msg_queue);
    master_slot = encode_chan->master_slot;
    chip_driver = chip->chip_driver;
    video_bus = &chip_driver->video_bus;

    direct_driver_config_to_running(opened_logic_chan_ed);
    if(chip_is_in_robust_processing(chip) || isil_h264_encode_chan_is_in_robust_processing(encode_chan)){
        need_discard_orig_frame = 1;
        send_msg_contr->op->delete_send_msg_controller(send_msg_contr);
        delete_all_msg_of_this_h264_encode_chan(isil_msg_queue, encode_chan);
        printk("%s.%d\n", __FUNCTION__, __LINE__);
        goto escape;
    }
    if(encode_control->op->get_slice_head_status(encode_control)){
        need_discard_orig_frame = 1;
        printk("%s.%d, %d\n", __FUNCTION__, __LINE__, encode_control->op->get_slice_head_status(encode_control));
        goto escape;
    }
    if(opened_logic_chan_ed->op->get_state(opened_logic_chan_ed) != ISIL_ED_IDLE){
        need_discard_orig_frame = 1;
        printk("%s.%d, %d\n", __FUNCTION__, __LINE__, opened_logic_chan_ed->op->get_state(opened_logic_chan_ed));
        goto escape;
    }

    if(encode_property->op->is_encodeSize_changed(encode_property) == ISIL_ENCODE_PROPERTY_CHANGED){
        encode_control->op->update_image_resolution(encode_control);
        encode_control->i_gop_stride = 0;
        encode_control->encodeSize = VIDEO_SIZE_FROM_WIDTH_HEIGHT((encode_control->i_mb_x << 4), (encode_control->i_mb_y << 4), video_bus->op->get_video_standard(video_bus));
        encode_control->master_or_sub = ISIL_SUB_BITSTREAM;
        encode_control->sen_type = 4;
        encode_control->need_large_mbdelay = 0;
        switch(master_slot->op->get_video_size(master_slot)){
            case ISIL_VIDEO_SIZE_4CIF:
            case ISIL_VIDEO_SIZE_D1:
                switch(encode_control->encodeSize){
                    default:
                        ISIL_DBG(ISIL_DBG_ERR, "no surpport, phy_video_size=%d, encode_video_size=%d\n", master_slot->op->get_video_size(master_slot), encode_control->encodeSize);
                        need_discard_orig_frame = 1;
                        break;
                    case ISIL_VIDEO_SIZE_4CIF:
                    case ISIL_VIDEO_SIZE_D1:
                        if(encode_feature->op->get_deinterlace(encode_feature)){
                            encode_control->sen_type = 0x6;
                        } else {
                            encode_control->sen_type = 0x4;
                        }
                        encode_control->master_or_sub = ISIL_MASTER_BITSTREAM;
                        break;
                    case ISIL_VIDEO_SIZE_2CIF:
                    case ISIL_VIDEO_SIZE_HALF_D1:
                        encode_control->sen_type = 0x144;
                        break;
                    case ISIL_VIDEO_SIZE_CIF:
                        encode_control->sen_type = 0x154;
                        break;
                    case ISIL_VIDEO_SIZE_QCIF:
                        encode_control->sen_type = 0x1a4;
                        encode_control->need_large_mbdelay = 1;
                        break;
                }
                break;
            case ISIL_VIDEO_SIZE_2CIF:
            case ISIL_VIDEO_SIZE_HALF_D1:
                switch(encode_control->encodeSize){
                    default:
                        ISIL_DBG(ISIL_DBG_ERR, "no surpport, phy_video_size=%d, encode_video_size=%d\n", master_slot->op->get_video_size(master_slot), encode_control->encodeSize);
                        need_discard_orig_frame = 1;
                        break;
                    case ISIL_VIDEO_SIZE_2CIF:
                    case ISIL_VIDEO_SIZE_HALF_D1:
                        encode_control->sen_type = 0x4;
                        encode_control->master_or_sub = ISIL_MASTER_BITSTREAM;
                        break;
                    case ISIL_VIDEO_SIZE_CIF:
                        encode_control->sen_type = 0x114;
                        break;
                    case ISIL_VIDEO_SIZE_QCIF:
                        encode_control->sen_type = 0x164;
                        encode_control->need_large_mbdelay = 1;
                        break;
                }
                break;
            case ISIL_VIDEO_SIZE_CIF:
                switch(encode_control->encodeSize){
                    default:
                        ISIL_DBG(ISIL_DBG_ERR, "no surpport, phy_video_size=%d, encode_video_size=%d\n", master_slot->op->get_video_size(master_slot), encode_control->encodeSize);
                        need_discard_orig_frame = 1;
                        break;
                    case ISIL_VIDEO_SIZE_CIF:
                        encode_control->sen_type = 0x4;
                        encode_control->master_or_sub = ISIL_MASTER_BITSTREAM;
                        break;
                    case ISIL_VIDEO_SIZE_QCIF:
                        encode_control->sen_type = 0x154;
                        encode_control->need_large_mbdelay = 1;
                        break;
                }
                break;
            default:
                ISIL_DBG(ISIL_DBG_ERR, "no surpport, phy_video_size=%d, encode_video_size=%d\n", master_slot->op->get_video_size(master_slot), encode_control->encodeSize);
                need_discard_orig_frame = 1;
                break;
        }
    }

    if(need_discard_orig_frame){
        encode_chan_gen_req_msg(encode_chan, REQ_ALGO_SLICE_HEAD, DELAY_OP);
    } else {
        if(encode_property->op->is_gopIntervals_changed(encode_property) == ISIL_ENCODE_PROPERTY_CHANGED){
            encode_control->i_gop_stride = 0;
        }
        if(encode_rc->op->is_qpi_changed(encode_rc)){
            encode_control->i_gop_stride = 0;
        }
        if(encode_property->op->is_target_bitrate_changed(encode_property) == ISIL_ENCODE_PROPERTY_CHANGED){
            need_init_rc_flag = 1;
        }
        if(encode_property->op->is_target_fps_changed(encode_property) == ISIL_ENCODE_PROPERTY_CHANGED){
            need_init_rc_flag = 1;
        }
        if(encode_rc->op->is_rc_type_changed(encode_rc)){
            need_init_rc_flag = 1;
        }
        if(encode_property->op->is_iamgeLevel_changed(encode_property) == ISIL_ENCODE_PROPERTY_CHANGED){
            need_init_rc_flag = 1;
        }
        if(need_init_rc_flag){
            encode_control->i_gop_stride = 0;
            encode_chan->rc_driver.rc_op->init_rc(encode_chan);
        }
        if(atomic_read(&encode_chan->opened_flag)){
            encode_frame_queue->op->try_get_curr_producer_from_pool(encode_frame_queue, encode_chan_buf_pool);
            video_frame = (isil_video_frame_tcb_t*)encode_frame_queue->curr_producer;
            if(video_frame != NULL){
                video_packet_queue = &video_frame->video_packet_queue;
                packet = video_packet_queue->curr_producer;
                if (packet == NULL) {
                    video_packet_queue->op->try_get_curr_producer_from_pool(video_packet_queue, &encode_chan->encode_chan_buf_pool);
                    packet = video_packet_queue->curr_producer;
                }
                if(packet == NULL){
                    need_discard_orig_frame = 1;
                    encode_frame_queue->op->release_curr_producer(encode_frame_queue, encode_chan_buf_pool);
                    encode_chan_gen_req_msg(encode_chan, REQ_ALGO_SLICE_HEAD, DELAY_OP);
                    //ISIL_DBG(ISIL_DBG_ERR, "channel %d delay gen algo_slice_head req, can't get packet\n", encode_chan->logic_chan_id);
                } else {
                    encode_control->op->calculate_frame_type(encode_control);
                    video_frame->frame_type = encode_control->op->get_frame_type(encode_control);
                    if(video_frame->frame_type == H264_FRAME_TYPE_IDR){
                        encode_control->op->set_sps(encode_control);
                        encode_control->op->set_pps(encode_control);
                    }
                    encode_chan->rc_driver.rc_op->set_qp(encode_chan, video_frame);
                    video_frame->i_curr_qp = encode_control->i_curr_qp;
                    video_frame->i_mb_x = encode_control->i_mb_x;
                    video_frame->i_mb_y = encode_control->i_mb_y;
                    video_frame->frame_number = encode_control->i_frame_serial;
                    video_frame->i_log_gop_value = encode_control->sps->i_log2_max_frame_num;
                    video_frame->fps = encode_property->op->get_target_fps(encode_property);
                    video_frame->encodeSize = encode_control->encodeSize;
                    video_frame->master_or_sub = encode_control->master_or_sub;
                    video_frame->sen_type = encode_control->sen_type;
                    video_frame->need_large_mbdelay = encode_control->need_large_mbdelay;
                    encode_control->op->set_slice_head(encode_control);
                    encode_control->op->generator_head(encode_control, video_frame);
                    encode_control->op->update_slice_head_ready(encode_control);
                    if(encode_control->op->get_slice_head_and_ad_status(encode_control)){
                        driver_gen_deliver_event(opened_logic_chan_ed, 1);
                    }
                    {
                        isil_vd_orig_buf_info_t *vd_orig_buf = &master_slot->vd_orig_buf;
                        osd_chan_engine_t       *encode_osd_engine = &encode_chan->encode_osd_engine;
#ifdef	TESTING_TIMESTAMP
                        encode_osd_engine->op->create_rectangle(encode_osd_engine, &encode_osd_engine->osd_running_param, video_frame->master_or_sub, video_frame->encodeSize, vd_orig_buf->op->get_timestamp(vd_orig_buf), vd_orig_buf->vd_encode_ptr, encode_control->i_vd_reference_ptr, encode_control->i_fps, encode_control->i_fpm);
#else
                        encode_osd_engine->op->create_rectangle(encode_osd_engine, &encode_osd_engine->osd_running_param, video_frame->master_or_sub, video_frame->encodeSize);
#endif
                    }
                }
            } else {
                need_discard_orig_frame = 1;
                encode_chan_gen_req_msg(encode_chan, REQ_ALGO_SLICE_HEAD, DELAY_OP);
                //ISIL_DBG(ISIL_DBG_ERR, "channel %d delay gen algo_slice_head req, can't get frame\n", encode_chan->logic_chan_id);        
            }
        } else {
            need_discard_orig_frame = 1;
        }
    }

escape:
    if(need_discard_orig_frame){
        if(master_slot != NULL){
            if(master_slot->op->slot_ad_is_ready(master_slot, chip, encode_chan)
                    || encode_control->op->get_ad_status(encode_control)){
                master_slot->op->slot_clear_ad_ready(master_slot, chip, encode_chan);
                master_slot->op->slot_software_discard_frame(master_slot, chip, encode_chan);
            }                
        } 
    }
    spin_unlock_irqrestore(&encode_chan->fsm_matrix_call_lock, flags);
}

static void h264_module_phy_video_slot_reset_preproessing(isil_h264_phy_video_slot_t *phy_video_slot)
{
    isil_register_table_t   *opened_logic_chan_table, *logic_chan_table;
    isil_register_node_t    *register_node;
    struct list_head        *list;
    ed_tcb_t                *opened_logic_chan_ed;
    isil_h264_logic_encode_chan_t   *h264_logic_encode_chan;
    unsigned long           flags1, flags2;
    int entry_number, entry_number_pre_close, total_entry_number, have_precessed_number;

    opened_logic_chan_table = &phy_video_slot->opened_logic_chan_table;
    logic_chan_table = &phy_video_slot->logic_chan_table;
again:
    spin_lock_irqsave(&opened_logic_chan_table->lock, flags1);
    spin_lock_irqsave(&logic_chan_table->lock, flags2);
    total_entry_number = entry_number = opened_logic_chan_table->op->get_curr_register_node_number(opened_logic_chan_table);
    have_precessed_number = entry_number_pre_close = 0;
    if(entry_number){
        list_for_each(list, &opened_logic_chan_table->register_header){
            if(entry_number == 0){
                break;
            }
            entry_number--;
            register_node = to_get_register_node(list);
            opened_logic_chan_ed = to_get_endpoint_tcb_with_register_node(register_node);
            h264_logic_encode_chan = to_get_isil_h264_encode_chan_with_opened_logic_chan_ed(opened_logic_chan_ed);
            if(opened_logic_chan_ed->op->get_state(opened_logic_chan_ed) == ISIL_ED_UNREGISTER){
                entry_number_pre_close++;
                printk("%s-%d: %d, %d, %d\n", __FUNCTION__, __LINE__, opened_logic_chan_table->op->get_curr_register_node_number(opened_logic_chan_table), h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id);
                continue;
            } else {
                printk("%s-%d: %d, %d, %d\n", __FUNCTION__, __LINE__, opened_logic_chan_table->op->get_curr_register_node_number(opened_logic_chan_table), h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id);
                start_isil_h264_encode_chan_robust_process(h264_logic_encode_chan);
            }
            spin_unlock_irqrestore(&logic_chan_table->lock, flags2);
            spin_unlock_irqrestore(&opened_logic_chan_table->lock, flags1);

            reset_isil_h264_encode_chan_preprocessing(opened_logic_chan_ed);
            have_precessed_number++;
            printk("%s-%d: %d, %d, %d\n", __FUNCTION__, __LINE__, opened_logic_chan_table->op->get_curr_register_node_number(opened_logic_chan_table), h264_logic_encode_chan->phy_slot_id, h264_logic_encode_chan->logic_chan_id);
            if((entry_number_pre_close+have_precessed_number) < total_entry_number){
                goto again;
            } else {
                break;
            }
        }
    }
    spin_unlock_irqrestore(&logic_chan_table->lock, flags2);
    spin_unlock_irqrestore(&opened_logic_chan_table->lock, flags1);
}

static void h264_module_reset_preprocessing(isil_chip_t *chip)
{
    isil_vd_cross_bus_t *chip_vd_cross_bus;
    isil_h264_phy_video_slot_t  *phy_video_slot;
    int     i;

    chip_vd_cross_bus = &chip->chip_vd_cross_bus;
    for(i=0; i<ISIL_PHY_VD_CHAN_NUMBER; i++){
        phy_video_slot = &chip_vd_cross_bus->phy_video_slot[i];
        h264_module_phy_video_slot_reset_preproessing(phy_video_slot);
    }
}

static void h264_module_phy_video_slot_reset_proessing(isil_h264_phy_video_slot_t *phy_video_slot)
{
    isil_register_table_t   *opened_logic_chan_table, *logic_chan_table;
    isil_register_node_t    *register_node;
    struct list_head        *list;
    ed_tcb_t                *opened_logic_chan_ed, *logic_chan_ed;
    isil_h264_logic_encode_chan_t   *encode_chan;
    unsigned long           flags1, flags2;

    opened_logic_chan_table = &phy_video_slot->opened_logic_chan_table;
    logic_chan_table = &phy_video_slot->logic_chan_table;
    spin_lock_irqsave(&opened_logic_chan_table->lock, flags1);
    spin_lock_irqsave(&logic_chan_table->lock, flags2);
    if(logic_chan_table->op->get_curr_register_node_number(logic_chan_table)){
        list_for_each(list, &logic_chan_table->register_header){
            register_node = to_get_register_node(list);
            spin_unlock_irqrestore(&logic_chan_table->lock, flags2);
            spin_unlock_irqrestore(&opened_logic_chan_table->lock, flags1);

            logic_chan_ed = to_get_endpoint_tcb_with_register_node(register_node);
            encode_chan = to_get_isil_h264_encode_chan_with_logic_chan_ed(logic_chan_ed);
            if(isil_h264_encode_chan_is_in_robust_processing(encode_chan) == 1){
                opened_logic_chan_ed = &encode_chan->opened_logic_chan_ed;
                reset_isil_h264_encode_chan_processing(opened_logic_chan_ed);
                printk("%s-%d\n", __FUNCTION__, __LINE__);
            }

            spin_lock_irqsave(&opened_logic_chan_table->lock, flags1);
            spin_lock_irqsave(&logic_chan_table->lock, flags2);
        }
    }
    spin_unlock_irqrestore(&logic_chan_table->lock, flags2);
    spin_unlock_irqrestore(&opened_logic_chan_table->lock, flags1);
}

static void h264_module_reset_processing(isil_chip_t *chip)
{
    isil_vd_cross_bus_t *chip_vd_cross_bus;
    isil_h264_phy_video_slot_t  *phy_video_slot;
    int     i;

    chip_vd_cross_bus = &chip->chip_vd_cross_bus;
    for(i=0; i<ISIL_PHY_VD_CHAN_NUMBER; i++){
        phy_video_slot = &chip_vd_cross_bus->phy_video_slot[i];
        h264_module_phy_video_slot_reset_proessing(phy_video_slot);
    }
}

static void h264_module_phy_video_slot_reset_postproessing(isil_h264_phy_video_slot_t *phy_video_slot)
{
    isil_register_table_t   *opened_logic_chan_table, *logic_chan_table;
    isil_register_node_t    *register_node;
    struct list_head        *list;
    ed_tcb_t                *opened_logic_chan_ed, *logic_chan_ed;
    isil_h264_logic_encode_chan_t   *encode_chan;
    unsigned long           flags1, flags2;

    opened_logic_chan_table = &phy_video_slot->opened_logic_chan_table;
    logic_chan_table = &phy_video_slot->logic_chan_table;
    spin_lock_irqsave(&opened_logic_chan_table->lock, flags1);
    spin_lock_irqsave(&logic_chan_table->lock, flags2);
    if(logic_chan_table->op->get_curr_register_node_number(logic_chan_table)){
        list_for_each(list, &logic_chan_table->register_header){
            register_node = to_get_register_node(list);
            spin_unlock_irqrestore(&logic_chan_table->lock, flags2);
            spin_unlock_irqrestore(&opened_logic_chan_table->lock, flags1);

            logic_chan_ed = to_get_endpoint_tcb_with_register_node(register_node);
            encode_chan = to_get_isil_h264_encode_chan_with_logic_chan_ed(logic_chan_ed);
            if(isil_h264_encode_chan_is_in_robust_processing(encode_chan) == 1){
                opened_logic_chan_ed = &encode_chan->opened_logic_chan_ed;
                reset_isil_h264_encode_chan_postprocessing(opened_logic_chan_ed);
                printk("%s-%d: %d\n", __FUNCTION__, __LINE__, opened_logic_chan_table->op->get_curr_register_node_number(opened_logic_chan_table));
            }

            spin_lock_irqsave(&opened_logic_chan_table->lock, flags1);
            spin_lock_irqsave(&logic_chan_table->lock, flags2);
        }
    }
    spin_unlock_irqrestore(&logic_chan_table->lock, flags2);
    spin_unlock_irqrestore(&opened_logic_chan_table->lock, flags1);
}

static void h264_module_reset_postprocessing(isil_chip_t *chip)
{
    isil_vd_cross_bus_t *chip_vd_cross_bus;
    isil_h264_phy_video_slot_t  *phy_video_slot;
    int     i;

    chip_vd_cross_bus = &chip->chip_vd_cross_bus;
    for(i=0; i<ISIL_PHY_VD_CHAN_NUMBER; i++){
        phy_video_slot = &chip_vd_cross_bus->phy_video_slot[i];
        h264_module_phy_video_slot_reset_postproessing(phy_video_slot);
    }
}

static void audio_module_reset_preprocessing(isil_chip_t *chip)
{
    isil_register_table_t   *opened_logic_chan_table, *logic_chan_table;
    isil_register_node_t    *register_node;
    struct list_head        *list;
    ed_tcb_t                *opened_logic_chan_ed;
    isil_audio_driver_t     *audio_chan_driver;
    chip_audio_t            *chip_audio;    
    unsigned long           flags1, flags2;
    int entry_number;

    chip_audio = &chip->chip_audio;
    opened_logic_chan_table = &chip_audio->opened_logic_chan_table;
    logic_chan_table = &chip_audio->logic_chan_table;

    spin_lock_irqsave(&opened_logic_chan_table->lock, flags1);
    spin_lock_irqsave(&logic_chan_table->lock, flags2);
    entry_number = opened_logic_chan_table->op->get_curr_register_node_number(opened_logic_chan_table);
    if(entry_number){
        list_for_each(list, &opened_logic_chan_table->register_header){
            if(entry_number == 0){
                break;
            }
            entry_number--;
            register_node = to_get_register_node(list);
            opened_logic_chan_ed = to_get_endpoint_tcb_with_register_node(register_node);
            audio_chan_driver = to_isil_audio_chan_driver_with_opened_logic_chan_ed(opened_logic_chan_ed);
            if(opened_logic_chan_ed->op->get_state(opened_logic_chan_ed) == ISIL_ED_UNREGISTER){
                continue;
            } else {
                start_audio_chan_robust_process(audio_chan_driver);
            }
            spin_unlock_irqrestore(&logic_chan_table->lock, flags2);
            spin_unlock_irqrestore(&opened_logic_chan_table->lock, flags1);

            if(audio_chan_driver->audio_logic_chan_id < ISIL_AUDIO_IN_CHAN_NUMBER){
                reset_isil_audio_encode_chan_preprocessing(opened_logic_chan_ed);
            } else {
                reset_isil_audio_decode_chan_preprocessing(opened_logic_chan_ed);
            }

            printk("%s-%d: %d\n", __FUNCTION__, __LINE__, opened_logic_chan_table->op->get_curr_register_node_number(opened_logic_chan_table));
            spin_lock_irqsave(&opened_logic_chan_table->lock, flags1);
            spin_lock_irqsave(&logic_chan_table->lock, flags2);
        }
    }
    spin_unlock_irqrestore(&logic_chan_table->lock, flags2);
    spin_unlock_irqrestore(&opened_logic_chan_table->lock, flags1);
}

static void audio_module_reset_processing(isil_chip_t *chip)
{
    isil_register_table_t   *opened_logic_chan_table, *logic_chan_table;
    isil_register_node_t    *register_node;
    struct list_head        *list;
    ed_tcb_t                *opened_logic_chan_ed, *logic_chan_ed;
    isil_audio_driver_t     *audio_chan_driver;
    chip_audio_t            *chip_audio;    
    unsigned long           flags1, flags2;

    chip_audio = &chip->chip_audio;
    opened_logic_chan_table = &chip_audio->opened_logic_chan_table;
    logic_chan_table = &chip_audio->logic_chan_table;
    spin_lock_irqsave(&opened_logic_chan_table->lock, flags1);
    spin_lock_irqsave(&logic_chan_table->lock, flags2);
    if(logic_chan_table->op->get_curr_register_node_number(logic_chan_table)){
        list_for_each(list, &logic_chan_table->register_header){
            register_node = to_get_register_node(list);
            spin_unlock_irqrestore(&logic_chan_table->lock, flags2);
            spin_unlock_irqrestore(&opened_logic_chan_table->lock, flags1);

            logic_chan_ed = to_get_endpoint_tcb_with_register_node(register_node);
            audio_chan_driver = to_isil_audio_chan_driver_with_logic_chan_ed(logic_chan_ed);
            if(isil_audio_chan_is_in_robust_processing(audio_chan_driver) == 1){
                opened_logic_chan_ed = &audio_chan_driver->opened_logic_chan_ed;
                if(audio_chan_driver->audio_logic_chan_id < ISIL_AUDIO_IN_CHAN_NUMBER){
                    reset_isil_audio_encode_chan_processing(opened_logic_chan_ed);
                } else {
                    reset_isil_audio_decode_chan_processing(opened_logic_chan_ed);
                }
                printk("%s-%d\n", __FUNCTION__, __LINE__);
            }

            spin_lock_irqsave(&opened_logic_chan_table->lock, flags1);
            spin_lock_irqsave(&logic_chan_table->lock, flags2);
        }
    }
    spin_unlock_irqrestore(&logic_chan_table->lock, flags2);
    spin_unlock_irqrestore(&opened_logic_chan_table->lock, flags1);
}

static void audio_module_reset_postprocessing(isil_chip_t *chip)
{
    isil_register_table_t   *opened_logic_chan_table, *logic_chan_table;
    isil_register_node_t    *register_node;
    struct list_head        *list;
    ed_tcb_t                *opened_logic_chan_ed, *logic_chan_ed;
    isil_audio_driver_t     *audio_chan_driver;
    chip_audio_t            *chip_audio;    
    unsigned long           flags1, flags2;

    chip_audio = &chip->chip_audio;
    opened_logic_chan_table = &chip_audio->opened_logic_chan_table;
    logic_chan_table = &chip_audio->logic_chan_table;
    spin_lock_irqsave(&opened_logic_chan_table->lock, flags1);
    spin_lock_irqsave(&logic_chan_table->lock, flags2);
    if(logic_chan_table->op->get_curr_register_node_number(logic_chan_table)){
        list_for_each(list, &logic_chan_table->register_header){
            register_node = to_get_register_node(list);
            spin_unlock_irqrestore(&logic_chan_table->lock, flags2);
            spin_unlock_irqrestore(&opened_logic_chan_table->lock, flags1);

            logic_chan_ed = to_get_endpoint_tcb_with_register_node(register_node);
            audio_chan_driver = to_isil_audio_chan_driver_with_logic_chan_ed(logic_chan_ed);
            if(isil_audio_chan_is_in_robust_processing(audio_chan_driver) == 1){
                opened_logic_chan_ed = &audio_chan_driver->opened_logic_chan_ed;
                if(audio_chan_driver->audio_logic_chan_id < ISIL_AUDIO_IN_CHAN_NUMBER){
                    reset_isil_audio_encode_chan_postprocessing(opened_logic_chan_ed);
                } else {
                    reset_isil_audio_decode_chan_postprocessing(opened_logic_chan_ed);
                }
                printk("%s-%d\n", __FUNCTION__, __LINE__);
            }

            spin_lock_irqsave(&opened_logic_chan_table->lock, flags1);
            spin_lock_irqsave(&logic_chan_table->lock, flags2);
        }
    }
    spin_unlock_irqrestore(&logic_chan_table->lock, flags2);
    spin_unlock_irqrestore(&opened_logic_chan_table->lock, flags1);
}

static void jpeg_module_reset_preprocessing(isil_chip_t *chip)
{
    if(chip){
        isil_vj_bus_t *vj_bus;
        isil_jpeg_phy_video_slot_t *phy_video_slot;

        vj_bus = &chip->chip_vj_bus;
        for(phy_video_slot = vj_bus->phy_video_slot; (phy_video_slot - vj_bus->phy_video_slot) < ISIL_PHY_VJ_CHAN_NUMBER; phy_video_slot++){
            phy_video_slot->op->touch_each_register_logic_chan(phy_video_slot, (void (*)(void *))reset_isil_jpeg_encode_chan_preprocessing);
        }
    }
}

static void jpeg_module_reset_processing(isil_chip_t *chip)
{
    if(chip){
        isil_vj_bus_t *vj_bus;
        isil_jpeg_phy_video_slot_t *phy_video_slot;

        vj_bus = &chip->chip_vj_bus;
        for(phy_video_slot = vj_bus->phy_video_slot; (phy_video_slot - vj_bus->phy_video_slot) < ISIL_PHY_VJ_CHAN_NUMBER; phy_video_slot++){
            phy_video_slot->op->touch_each_register_logic_chan(phy_video_slot, (void (*)(void *))reset_isil_jpeg_encode_chan_processing);
        }
    }
}

static void jpeg_module_reset_postprocessing(isil_chip_t *chip)
{
    if(chip){
        isil_vj_bus_t *vj_bus;
        isil_jpeg_phy_video_slot_t *phy_video_slot;

        vj_bus = &chip->chip_vj_bus;
        for(phy_video_slot = vj_bus->phy_video_slot; (phy_video_slot - vj_bus->phy_video_slot) < ISIL_PHY_VJ_CHAN_NUMBER; phy_video_slot++){
            phy_video_slot->op->touch_each_register_logic_chan(phy_video_slot, (void (*)(void *))reset_isil_jpeg_encode_chan_postprocessing);
        }
    }
}

static void prev_module_reset_preprocessing(isil_chip_t *chip)
{
    int i;
    ed_tcb_t    *ed_tcb;
    isil_prev_logic_chan_t *chan;

    if (chip == NULL)
        return;

    for (i = 0; i < ISIL_PHY_VP_CHAN_NUMBER; i++) {
        chan = &chip->prev_chan[i];
        if (chan->enable) {
            ed_tcb = &chan->opened_logic_chan_ed;
            reset_isil_prev_chan_preprocessing(ed_tcb);
        }
    }
}

static void prev_module_reset_processing(isil_chip_t *chip)
{
    int i;
    ed_tcb_t    *ed_tcb;
    isil_prev_logic_chan_t *chan;

    if (chip == NULL)
        return;

    for (i = 0; i < ISIL_PHY_VP_CHAN_NUMBER; i++) {
        chan = &chip->prev_chan[i];
        if (chan->enable) {
            ed_tcb = &chan->opened_logic_chan_ed;
            reset_isil_prev_chan_processing(ed_tcb);
        }
    }
}

static void prev_module_reset_postprocessing(isil_chip_t *chip)
{
    int i;
    ed_tcb_t    *ed_tcb;
    isil_prev_logic_chan_t *chan;

    if (chip == NULL)
        return;

    for (i = 0; i < ISIL_PHY_VP_CHAN_NUMBER; i++) {
        chan = &chip->prev_chan[i];
        if (chan->enable) {
            ed_tcb = &chan->opened_logic_chan_ed;
            reset_isil_prev_chan_postprocessing(ed_tcb);
        }
    }
}

static void process_chip_reset_req(isil_chip_t *chip)
{
    if(chip != NULL){
        printk("\n\n\n\n%s-%d, recv chip reset msg\n", __FUNCTION__, __LINE__);

        //chip's reset pre-processing: include h264_encode, audio_encode, audio_decode, mjpeg_encode, preview
        h264_module_reset_preprocessing(chip);
        audio_module_reset_preprocessing(chip);
        jpeg_module_reset_preprocessing(chip);
        prev_module_reset_preprocessing(chip);

        //chip's reset precossing: include h264_encode, audio_encode, audio_decode, mjpeg_encode, preview
        h264_module_reset_processing(chip);
        audio_module_reset_processing(chip);
        jpeg_module_reset_processing(chip);
        prev_module_reset_processing(chip);

        if(chip->chip_set_valid != NULL){
            chip->chip_set_valid(chip);
        }

        //chip's reset post-precossing: include h264_encode, audio_encode, audio_decode, mjpeg_encode, preview
        h264_module_reset_postprocessing(chip);
        audio_module_reset_postprocessing(chip);
        jpeg_module_reset_postprocessing(chip);
        prev_module_reset_postprocessing(chip);

        isil_chip_prev_robust_process_done(&chip->chip_vp_bus);
        isil_chip_jpeg_robust_process_done(&chip->chip_vj_bus);
        isil_chip_audio_robust_process_done(&chip->chip_audio);
        chip_robust_process_done(chip);
        printk("%s-%d process end chip reset msg\n\n\n\n", __FUNCTION__, __LINE__);
    }
}


static void process_chip_video_standard_changed(isil_chip_t *chip)
{
    if(chip){
        isil_chip_vi_driver_t *vi_driver;
        isil_video_bus_t *video_bus;
        enum CHIP_VI_STANDARD video_standard;

        vi_driver = chip->chip_vi_driver;
        video_bus = &chip->chip_driver->video_bus;
        video_standard = vi_driver->op->get_hw_video_standard(vi_driver);

        vi_driver->op->set_video_standard(vi_driver, video_standard);
        if((video_standard == CHIP_VI_STANDARD_NTSC_M) || (video_standard == CHIP_VI_STANDARD_NTSC443)){
            if(video_bus->op->get_video_standard(video_bus) != ISIL_VIDEO_STANDARD_NTSC){
                video_bus->op->set_video_standard(video_bus, ISIL_VIDEO_STANDARD_NTSC);
            }
        }else{
            if(video_bus->op->get_video_standard(video_bus) != ISIL_VIDEO_STANDARD_PAL){
                video_bus->op->set_video_standard(video_bus, ISIL_VIDEO_STANDARD_PAL);
            }
        }
    }
}

/************************************************************************************/
/************************************************************************************/
static int	kthread_algo_agent(void *data)
{
    isil_kthread_msg_pool_t   *isil_msg_pool;
    isil_kthread_msg_queue_t  *isil_msg_queue;
    int	escape = 0;

    get_msg_pool_header(&isil_msg_pool);
    if(init_isil_kthread_msg_pool(isil_msg_pool) != 0){
        printk("create algo msg pool err\n");
        return 0;
    }
    get_msg_queue_header(&isil_msg_queue);
    init_isil_kthread_msg_queue(isil_msg_queue);
    atomic_set(&algo_running_flag, 1);
    while(1){
        isil_msg_queue->op->get_curr_consumer_from_queue(isil_msg_queue);
        if(isil_msg_queue->curr_consumer != NULL) {
            switch(isil_msg_queue->curr_consumer->msg_type){
                case REQ_ALGO_SLICE_HEAD:
                    process_slice_header_req((isil_h264_logic_encode_chan_t*)isil_msg_queue->curr_consumer->msg_context);
                    break;
                case REQ_ALGO_CHIP_RESET:
                    process_chip_reset_req((isil_chip_t*)isil_msg_queue->curr_consumer->msg_context);
                    break;
                case REQ_VIDEO_STANDARD_CHANGED:
                    process_chip_video_standard_changed((isil_chip_t*)isil_msg_queue->curr_consumer->msg_context);
                    break;
                case SYSTEM_ESCAPE_MSG:
                    escape = 1;
                    break;
                default:
                    printk("I don't know's msg type = %d\n", isil_msg_queue->curr_consumer->msg_type);
                    break;
            }
        }
        isil_msg_queue->op->release_curr_consumer(isil_msg_queue);
        if(escape){
            break;
        }
    }
    remove_isil_kthread_msg_queue(isil_msg_queue);
    remove_isil_kthread_msg_pool(isil_msg_pool);
    //printk("success escape %s\n", __FUNCTION__);
    atomic_set(&algo_running_flag, 0);
    return 0;
}


/////////////////////////////////////////////////////////////////////////////////////
// kthread manage function
/////////////////////////////////////////////////////////////////////////////////////
static void	init_isil_kthread_manage(void)
{
    algo_kthread=NULL;
    atomic_set(&algo_running_flag, 0);
}

static void	notify_algo_kthread_kill_myself(void)
{
    isil_kthread_msg_pool_t   *isil_msg_pool;
    isil_kthread_msg_queue_t  *isil_msg_queue;
    isil_kthread_msg_t        *isil_msg;

    //printk("%s\n", __FUNCTION__);
    get_msg_pool_header(&isil_msg_pool);
    get_msg_queue_header(&isil_msg_queue);
    isil_msg_pool->op->get(isil_msg_pool, &isil_msg);
    if(isil_msg != NULL){
        isil_msg->msg_type = SYSTEM_ESCAPE_MSG;
        isil_msg_queue->op->put(isil_msg_queue, isil_msg);
    }
}

static void	wait_10_second_for_algo_kthread(void)
{
    int count=0;
    while(atomic_read(&algo_running_flag)){
        //printk("%s, %d, %d\n", __FUNCTION__, __LINE__, count);
        msleep(100);
        count++;
        if(count > 100){
            break;
        }
    }
}

static void force_close_kthread(struct task_struct **task)
{
    kthread_stop(*task);
    *task = NULL;
}

void    start_isil_kthread(void)
{
    char    name[64];
    init_isil_kthread_manage();
    memset(name, 0, 64);
    sprintf(name, "isil_agend");
    algo_kthread = kthread_run(kthread_algo_agent, NULL, name);
    if(IS_ERR(algo_kthread)) {
        printk("start isil_algo_kthread err\n");
        atomic_set(&algo_running_flag, 0);
        algo_kthread = NULL;
    }
}

void    stop_isil_kthread(void)
{
    if(algo_kthread != NULL) {
        notify_algo_kthread_kill_myself();
        wait_10_second_for_algo_kthread();
        //printk("%s,%d,ok\n", __FUNCTION__, __LINE__);
        if(atomic_read(&algo_running_flag)) {
            printk("algo_kthread myself kill err, so force kill it\n");
            force_close_kthread(&algo_kthread);
            atomic_set(&algo_running_flag, 0);
        }
        algo_kthread = NULL;
    }
    //printk("%s,%d,ok\n", __FUNCTION__, __LINE__);
}


