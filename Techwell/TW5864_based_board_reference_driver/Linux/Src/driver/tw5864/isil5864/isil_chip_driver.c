#include <isil5864/isil_common.h>
#include <isil5864/tc_common.h>

static int chip_driver_hcd_interface_open(ed_tcb_t *opened_control_ed)
{
    int ret = ISIL_ERR;

    if(opened_control_ed) {
        chip_driver_t *driver;

        driver = to_get_chip_driver_with_opened_logic_chan_ed(opened_control_ed);
        switch(atomic_read(&opened_control_ed->state)){
            default:
            case ISIL_ED_UNREGISTER:
                opened_control_ed->ed.op->init_complete(&opened_control_ed->ed);
                atomic_set(&driver->opened_flag, 1);
                ret = 0;
                break;
            case ISIL_ED_IDLE:
            case ISIL_ED_STANDBY:
            case ISIL_ED_SUSPEND:
            case ISIL_ED_RUNNING:
                ret = -EBUSY;
                ISIL_DBG(ISIL_DBG_ERR, "chip control have been opened\n");
                break;
        }
    }

    return ret;
}

static int chip_driver_hcd_interface_close(ed_tcb_t *opened_control_ed)
{
    int ret = ISIL_ERR;

    if(opened_control_ed) {
        chip_driver_t *driver;

        driver = to_get_chip_driver_with_opened_logic_chan_ed(opened_control_ed);
        switch(atomic_read(&opened_control_ed->state)){
            default:
            case ISIL_ED_UNREGISTER:
                ret = 0;
                ISIL_DBG(ISIL_DBG_INFO, "chip control have been close\n");
                break;
            case ISIL_ED_IDLE:
            case ISIL_ED_STANDBY:
            case ISIL_ED_SUSPEND:
            case ISIL_ED_RUNNING:
                if(opened_control_ed->ed.op != NULL){
                    atomic_set(&opened_control_ed->state, ISIL_ED_UNREGISTER);
                    opened_control_ed->ed.op->complete_done(&opened_control_ed->ed);
                    ISIL_DBG(ISIL_DBG_INFO, "control complete doen\n");
                    ret = 0;
                }else{
                    ISIL_DBG(ISIL_DBG_FATAL, "control node have not inital!\n");
                    ret = -EACCES;
                }
                atomic_set(&driver->opened_flag, 0);
                break;
        }
    }

    return ret;
}

static void chip_driver_hcd_interface_suspend(ed_tcb_t *opened_control_ed)
{
    if(opened_control_ed) {
        chip_driver_t *driver;

        driver = to_get_chip_driver_with_opened_logic_chan_ed(opened_control_ed);
        if(atomic_read(&driver->opened_flag)) {
        }else{
            ISIL_DBG(ISIL_DBG_ERR, "device have not opened\n");
            return;
        }
    }

    return ;
}

static void chip_driver_hcd_interface_resume(ed_tcb_t *opened_control_ed)
{
    if(opened_control_ed) {
        chip_driver_t *driver;

        driver = to_get_chip_driver_with_opened_logic_chan_ed(opened_control_ed);
        if(atomic_read(&driver->opened_flag)) {
        }else{
            ISIL_DBG(ISIL_DBG_ERR, "device have not opened\n");
            return;
        }
    }

    return ;
}

static int chip_driver_hcd_interface_ioctl(ed_tcb_t *opened_control_ed, unsigned int cmd, unsigned long arg)
{
    int ret = ISIL_ERR;
    chip_driver_t     *driver;
    isil_video_bus_t    *video_bus;
    isil_audio_bus_t    *audio_bus;
    chip_audio_t      *chip_audio_driver;
    isil_audio_driver_t *audio_driver;
    char tmp[256] = {0};

    if(opened_control_ed) {
        driver = to_get_chip_driver_with_opened_logic_chan_ed(opened_control_ed);
        video_bus = &driver->video_bus;
        audio_bus = &driver->audio_bus;

        if(atomic_read(&driver->opened_flag)) {
        }else{
            ISIL_DBG(ISIL_DBG_ERR, "device have not opened\n");
            return -EPERM;
        }

        ISIL_DBG(ISIL_DBG_INFO, "%c, %d\n", _IOC_TYPE(cmd), _IOC_NR(cmd));
        switch(cmd) {
            case ISIL_CHIP_AUDIO_ENCODE_PARAM_SET:
            case ISIL_CHIP_AUDIO_ENCODE_PARAM_GET:
            case ISIL_CHIP_AUDIO_DECODE_PARAM_SET:
            case ISIL_CHIP_AUDIO_DECODE_PARAM_GET:
                chip_audio_driver = audio_bus->chip_audio_driver;
                if(chip_audio_driver) {
                    struct isil_chip_audio_param audio_param;
                    if(copy_from_user(&audio_param, (void *)arg, sizeof(struct isil_chip_audio_param)) != 0){
                        printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                    }
                    if((audio_param.channel >= ISIL_AUDIO_IN_CHAN0_ID) && (audio_param.channel <= ISIL_AUDIO_OUT_PLAYBACK_ID)) {
                        chip_audio_driver->op->get_audio_chan_driver(chip_audio_driver, audio_param.channel, &audio_driver);
                        if(audio_driver) {
                            ed_tcb_t *ed;
                            ed = &audio_driver->opened_logic_chan_ed;
                            ret = ed->op->ioctl(ed, cmd, arg);
                            if(ret) {
                                ISIL_DBG(ISIL_DBG_ERR, "audio ioctl failed %d\n", ret);
                                return ret;
                            }
                        }else{
                            ISIL_DBG(ISIL_DBG_FATAL, "audio driver not installed!\n");
                            ret = -ENOSYS;
                        }
                    }else{
                        ISIL_DBG(ISIL_DBG_ERR, "audio channel number %d overflow\n", audio_param.channel);
                        ret = -ECHRNG;
                    }

                }else{
                    ISIL_DBG(ISIL_DBG_FATAL, "audio driver not installed!\n");
                    ret = -ENOSYS;
                }
                break;
            case ISIL_VIDEO_BUS_PARAM_SET:
                if(video_bus && video_bus->op) {
                    struct isil_chip_video_param video_param;
                    if(copy_from_user(&video_param, (void *)arg, sizeof(struct isil_chip_video_param)) != 0){
                        printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                    }
                    if(video_param.change_mask_flag & ISIL_VIDEO_BUS_PARAM_ENABLE_CHANGE_VIDEO_STANDARD_MASK) {
                        ret = video_bus->op->set_video_standard(video_bus, video_param.e_video_standard);
                    }
                }
                break;
            case ISIL_VIDEO_BUS_PARAM_GET:
                if(video_bus && video_bus->op) {
                    struct isil_chip_video_param video_param;

                    if(copy_from_user(&video_param, (void *)arg, sizeof(struct isil_chip_video_param)) != 0){
                        printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                    }
                    if(video_param.change_mask_flag & ISIL_VIDEO_BUS_PARAM_ENABLE_CHANGE_VIDEO_STANDARD_MASK) {
                        video_param.e_video_standard = video_bus->op->get_video_standard(video_bus);

                        if(copy_to_user((void *)arg, &video_param, sizeof(struct isil_chip_video_param)) != 0){
                            printk("%s, %d: copy_to_user failed\n", __FUNCTION__, __LINE__);
                        }

                        ret = ISIL_OK;
                    }else{
                        ISIL_DBG(ISIL_DBG_INFO, "change_mask_flag not set\n");
                        ret = -EINVAL;
                    }

                }
                break;
            case ISIL_CHIP_VD_CONFIG_PARAM_SET:
                if(video_bus) {
                    u32 chnl;
                    isil_vd_cross_bus_t *vd_bus;
                    struct isil_chip_vd_param vd_param;
                    struct isil_vd_chan_param *vd_chan_param;
                    isil_h264_logic_video_slot_t *logic_video_slot;
                    isil_h264_phy_video_slot_t *phy_video_slot;
                    vd_chan_map_info_t chan_map_info[ISIL_PHY_VD_CHAN_NUMBER], *map_info;

                    if(copy_from_user(&vd_param, (void *)arg, sizeof(struct isil_chip_vd_param)) != 0){
                        printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                    }
                    map_info = chan_map_info;
                    if(vd_param.i_vd_chan_number > ISIL_PHY_VD_CHAN_NUMBER){
                        ISIL_DBG(ISIL_DBG_ERR, "channle number %d overflow\n", vd_param.i_vd_chan_number);
                        ret = -EINVAL;
                        return ret;
                    }

                    vd_bus = video_bus->vd_bus_driver;
                    if(vd_bus->op->get_work_mode(vd_bus) != ISIL_CROSS_BUS_UNREALTIME) {
                        ISIL_DBG(ISIL_DBG_ERR, "To change vd parameter, Only work on ISIL_CROSS_BUS_UNREALTIME mode!\n");
                        //ret = -EPERM;
                    }
                    ISIL_DBG(ISIL_DBG_DEBUG, "change %d channel\n", vd_param.i_vd_chan_number);
                    for(chnl = 0; chnl < vd_param.i_vd_chan_number; chnl++, map_info++) {
                        vd_chan_param = &vd_param.vd_chan_param[chnl];
                        ISIL_DBG(ISIL_DBG_DEBUG, "ch = %d, fps = %2d, mb_width = %2d, mb_height = %2d\n", vd_chan_param->i_phy_chan_id, vd_chan_param->i_phy_chan_fps, vd_chan_param->i_phy_video_width_mb_size, vd_chan_param->i_phy_video_height_mb_size);
                        if(vd_chan_param->i_phy_chan_id < ISIL_PHY_VD_CHAN_NUMBER) {
                            if(video_bus->op->get_video_standard(video_bus) == ISIL_VIDEO_STANDARD_PAL) {
                                if(vd_chan_param->i_phy_chan_fps <= MAX_FRAME_RATE_PAL) {
                                    map_info->fps         = vd_chan_param->i_phy_chan_fps;
                                }else{
                                    ISIL_DBG(ISIL_DBG_ERR, "fps %d overflow, [0, %d]\n", vd_chan_param->i_phy_chan_fps, MAX_FRAME_RATE_PAL);
                                    ret = -EINVAL;
                                    return ret;
                                }
                            }else{
                                if(vd_chan_param->i_phy_chan_fps <= MAX_FRAME_RATE_NTSC) {
                                    map_info->fps         = vd_chan_param->i_phy_chan_fps;
                                }else{
                                    ISIL_DBG(ISIL_DBG_ERR, "fps %d overflow, [0, %d]\n", vd_chan_param->i_phy_chan_fps, MAX_FRAME_RATE_NTSC);
                                    ret = -EINVAL;
                                    return ret;
                                }
                            }
                            map_info->enable      = vd_chan_param->i_phy_chan_enable;
                            if(map_info->enable) {
                                map_info->phy_slot_id = vd_chan_param->i_phy_chan_id;
                            }else{
                                map_info->phy_slot_id = ISIL_LOGIC_VD_INVALID;
                            }
                            map_info->video_size  = VIDEO_SIZE_FROM_WIDTH_HEIGHT(vd_chan_param->i_phy_video_width_mb_size<<4,
                                    vd_chan_param->i_phy_video_height_mb_size<<4, 
                                    video_bus->op->get_video_standard(video_bus));
                            if(map_info->video_size == ISIL_VIDEO_SIZE_USER) {
                                ISIL_DBG(ISIL_DBG_ERR, "channel width or height error\n");
                                ret = -EINVAL;
                                return ret;
                            }
                            map_info->interlace = vd_chan_param->i_interlace;
                            if(vd_chan_param->i_interlace >= ISIL_FRAME_MODE_RESERVED){
                                ISIL_DBG(ISIL_DBG_ERR, "only support interlace or progress\n");
                                ret = -EINVAL;
                                return ret;
                            }
                        }else{
                            ISIL_DBG(ISIL_DBG_ERR, "No such channel %d\n", vd_chan_param->i_phy_chan_id);
                            ret = -EINVAL;
                            return ret;
                        }
                    }
                    for(; chnl < ISIL_PHY_VD_CHAN_NUMBER; chnl++, map_info++) {
                        map_info->fps           = 0;
                        map_info->enable        = 0;
                        map_info->phy_slot_id   = ISIL_LOGIC_VD_INVALID;
                        map_info->video_size    = ISIL_VIDEO_SIZE_D1;
                        map_info->interlace     = ISIL_FRAME_MODE_INTERLACE;
                    }
                    ret = vd_bus->op->calculate_cross_bus_map_table(vd_bus, chan_map_info);
                    if(ret) {
                        ISIL_DBG(ISIL_DBG_ERR, "please check configuration\n");
                        break;
                    }
                    map_info = chan_map_info;
                    if(video_bus->op->notify_suspend_logic_chan(video_bus)){
                        ISIL_DBG(ISIL_DBG_FATAL, "suspend logic channel failed\n");
                        return -EBUSY;
                    }
                    for(chnl = 0; chnl < ISIL_CROSS_BUS_LOGIC_VD_CHAN_NUMBER; chnl++, map_info++) {
                        logic_video_slot = vd_bus->logic_video_slot + map_info->map_logic_slot_id;
                        logic_video_slot->op->set_mapPhySlotId(logic_video_slot, map_info->phy_slot_id);
                        logic_video_slot->op->set_discardTable(logic_video_slot, map_info->logic_slot_discard_table);
                        logic_video_slot->op->set_roundTableStride(logic_video_slot, map_info->roundTableStride);
                        logic_video_slot->op->set_video_size(logic_video_slot, map_info->video_size);
                        logic_video_slot->op->set_enableslot(logic_video_slot, map_info->enable);
                        logic_video_slot->op->set_fps(logic_video_slot, map_info->fps);
                        atomic_set(&logic_video_slot->needUpdateFlag, 1);
                        vd_bus->op->get_phy_video_slot_by_phy_id(vd_bus, map_info->phy_slot_id, &phy_video_slot);
                        phy_video_slot->op->set_video_size(phy_video_slot, logic_video_slot->op->get_video_size(logic_video_slot));
                        phy_video_slot->op->set_interlace(phy_video_slot, map_info->interlace);
                        phy_video_slot->op->update_map_logic_slot(phy_video_slot, logic_video_slot->logicSlotId, logic_video_slot);
                    }
                    if(vd_bus->op->set_chip_end_cross_bus(vd_bus, to_get_chip_with_chip_cross_bus(vd_bus))){
                        ISIL_DBG(ISIL_DBG_FATAL, "sync to hardware failed\n");
                    }
                    if(video_bus->op->notify_resume_logic_chan(video_bus)){
                        ISIL_DBG(ISIL_DBG_FATAL, "resume logic channel failed\n");
                        return -EBUSY;
                    }
                    /*notify logic video channel*/
                    if(video_bus->op->notify_logic_chan_change(video_bus)){
                        ISIL_DBG(ISIL_DBG_FATAL, "sync to logic channel failed\n");
                        return -EBUSY;
                    }
                    ret = ISIL_OK;
                }
                break;
            case ISIL_CHIP_VD_CONFIG_PARAM_GET:
                if(video_bus) {
                    u32 chnl;
                    isil_vd_cross_bus_t *vd_bus;
                    isil_h264_phy_video_slot_t *phy_video_slot;
                    isil_h264_logic_video_slot_t *logic_video_slot;
                    struct isil_chip_vd_param vd_param;
                    struct isil_vd_chan_param *vd_chan_param;

                    if(copy_from_user(&vd_param, (void *)arg, sizeof(struct isil_chip_vd_param)) != 0){
                        printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                    }
                    if(vd_param.i_vd_chan_number > ISIL_PHY_VD_CHAN_NUMBER){
                        ISIL_DBG(ISIL_DBG_ERR, "channle number %d overflow\n", vd_param.i_vd_chan_number);
                        ret = -EINVAL;
                        break;
                    }
                    vd_bus = video_bus->vd_bus_driver;
                    for(chnl = 0; chnl < vd_param.i_vd_chan_number; chnl++) {
                        vd_chan_param = &vd_param.vd_chan_param[chnl];
                        if(vd_chan_param->i_phy_chan_id < ISIL_PHY_VD_CHAN_NUMBER) {
                            vd_bus->op->get_phy_video_slot_by_phy_id(vd_bus, vd_chan_param->i_phy_chan_id, &phy_video_slot);
                            logic_video_slot = phy_video_slot->cross_bus_logic_video_slot;
                            vd_chan_param->i_phy_chan_enable = logic_video_slot->op->get_enableslot(logic_video_slot);
                            vd_chan_param->i_phy_chan_fps    = logic_video_slot->op->get_fps(logic_video_slot);
                            vd_chan_param->i_phy_video_width_mb_size = VIDEO_SIZE_TO_WIDTH(logic_video_slot->op->get_video_size(logic_video_slot), 
                                    video_bus->op->get_video_standard(video_bus))>>4;
                            vd_chan_param->i_phy_video_height_mb_size= VIDEO_SIZE_TO_HEIGHT(logic_video_slot->op->get_video_size(logic_video_slot), 
                                    video_bus->op->get_video_standard(video_bus))>>4;
                            vd_chan_param->i_interlace = phy_video_slot->op->get_interlace(phy_video_slot);
                        }else{
                            ISIL_DBG(ISIL_DBG_ERR, "No such channel %d\n", vd_chan_param->i_phy_chan_id);
                            ret = -EINVAL;
                            break;
                        }
                    }
                    if(copy_to_user((void *)arg, &vd_param, sizeof(struct isil_chip_vd_param)) != 0){
                        printk("%s, %d: copy_to_user failed\n", __FUNCTION__, __LINE__);
                    }
                }
                break;
            case ISIL_CHIP_VD_CHNL_CONFIG_SET:
                if(video_bus) {
                    isil_vd_cross_bus_t *vd_bus;
                    isil_h264_logic_video_slot_t *logic_video_slot;
                    isil_h264_phy_video_slot_t *phy_video_slot;
                    vd_chan_map_info_t chan_map_info[ISIL_PHY_VD_CHAN_NUMBER], *map_info;
                    struct isil_vd_chan_param vd_chan_param;
                    int i;

                    if(!arg) {
                        ISIL_DBG(ISIL_DBG_ERR, "arg null\n");
                        ret = -EINVAL;
                        break;
                    }

                    if(copy_from_user(&vd_chan_param, (void *)arg, sizeof(struct isil_vd_chan_param)) != 0){
                        printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                    }
                    if(vd_chan_param.i_phy_chan_id > ISIL_PHY_VD_CHAN_NUMBER){
                        ISIL_DBG(ISIL_DBG_ERR, "channle number %d overflow\n", vd_chan_param.i_phy_chan_id);
                        ret = -EINVAL;
                        break;
                    }

                    ISIL_DBG(ISIL_DBG_DEBUG, "ch = %d, fps = %2d, mb_width = %2d, mb_height = %2d\n", vd_chan_param.i_phy_chan_id, vd_chan_param.i_phy_chan_fps, vd_chan_param.i_phy_video_width_mb_size, vd_chan_param.i_phy_video_height_mb_size);
                    map_info = chan_map_info;
                    vd_bus = video_bus->vd_bus_driver;
                    map_info->phy_slot_id = vd_chan_param.i_phy_chan_id;
                    map_info->video_size = VIDEO_SIZE_FROM_WIDTH_HEIGHT(vd_chan_param.i_phy_video_width_mb_size<<4,
                            vd_chan_param.i_phy_video_height_mb_size<<4, 
                            video_bus->op->get_video_standard(video_bus));
                    if(map_info->video_size == ISIL_VIDEO_SIZE_USER) {
                        ISIL_DBG(ISIL_DBG_ERR, "channel width or height error\n");
                        ret = -EINVAL;
                        break;
                    }
                    if(video_bus->op->get_video_standard(video_bus) == ISIL_VIDEO_STANDARD_PAL) {
                        if(vd_chan_param.i_phy_chan_fps <= MAX_FRAME_RATE_PAL) {
                            map_info->fps         = vd_chan_param.i_phy_chan_fps;
                        }else{
                            ISIL_DBG(ISIL_DBG_ERR, "fps %d overflow, [0, %d]\n", vd_chan_param.i_phy_chan_fps, MAX_FRAME_RATE_PAL);
                            ret = -EINVAL;
                            break;
                        }
                    }else{
                        if(vd_chan_param.i_phy_chan_fps <= MAX_FRAME_RATE_NTSC) {
                            map_info->fps         = vd_chan_param.i_phy_chan_fps;
                        }else{
                            ISIL_DBG(ISIL_DBG_ERR, "fps %d overflow, [0, %d]\n", vd_chan_param.i_phy_chan_fps, MAX_FRAME_RATE_NTSC);
                            ret = -EINVAL;
                            break;
                        }
                    }
                    map_info->interlace = vd_chan_param.i_interlace;
                    if(vd_chan_param.i_interlace >= ISIL_FRAME_MODE_RESERVED){
                        ISIL_DBG(ISIL_DBG_ERR, "only support interlace or progress\n");
                        ret = -EINVAL;
                        return ret;
                    }
                    map_info->enable = 1;
                    map_info++;
                    for(i = 0; i < ISIL_PHY_VD_CHAN_NUMBER; i++) {
                        if(i == vd_chan_param.i_phy_chan_id) {
                            continue;
                        }
                        vd_bus->op->get_phy_video_slot_by_phy_id(vd_bus, i, &phy_video_slot);
                        if(!phy_video_slot) {
                            ISIL_DBG(ISIL_DBG_ERR, "unable to find phy_video_slot %d\n", i);
                            return -EFAULT;
                        }
                        logic_video_slot = phy_video_slot->cross_bus_logic_video_slot;
                        map_info->enable = logic_video_slot->op->get_enableslot(logic_video_slot);
                        map_info->fps    = logic_video_slot->op->get_fps(logic_video_slot);
                        map_info->video_size = logic_video_slot->op->get_video_size(logic_video_slot);
                        map_info->phy_slot_id = i;
                        map_info->interlace = phy_video_slot->op->get_interlace(phy_video_slot);
                        map_info++;
                    }

                    ret = vd_bus->op->calculate_cross_bus_map_table(vd_bus, chan_map_info);
                    if(ret) {
                        ISIL_DBG(ISIL_DBG_ERR, "please check configuration\n");
                        break;
                    }
                    map_info = chan_map_info;
                    if(video_bus->op->notify_suspend_logic_chan(video_bus)){
                        ISIL_DBG(ISIL_DBG_FATAL, "suspend logic channel failed\n");
                        return -EBUSY;
                    }
                    for(i = 0; i < ISIL_CROSS_BUS_LOGIC_VD_CHAN_NUMBER; i++, map_info++) {
                        logic_video_slot = vd_bus->logic_video_slot + map_info->map_logic_slot_id;
                        logic_video_slot->op->set_mapPhySlotId(logic_video_slot, map_info->phy_slot_id);
                        logic_video_slot->op->set_discardTable(logic_video_slot, map_info->logic_slot_discard_table);
                        logic_video_slot->op->set_roundTableStride(logic_video_slot, map_info->roundTableStride);
                        logic_video_slot->op->set_video_size(logic_video_slot, map_info->video_size);
                        logic_video_slot->op->set_enableslot(logic_video_slot, map_info->enable);
                        logic_video_slot->op->set_fps(logic_video_slot, map_info->fps);
                        atomic_set(&logic_video_slot->needUpdateFlag, 1);
                        vd_bus->op->get_phy_video_slot_by_phy_id(vd_bus, map_info->phy_slot_id, &phy_video_slot);
                        phy_video_slot->op->set_interlace(phy_video_slot, map_info->interlace);
                        phy_video_slot->op->set_video_size(phy_video_slot, logic_video_slot->op->get_video_size(logic_video_slot));
                        phy_video_slot->op->update_map_logic_slot(phy_video_slot, logic_video_slot->logicSlotId, logic_video_slot);
                    }
                    if(vd_bus->op->set_chip_end_cross_bus(vd_bus, to_get_chip_with_chip_cross_bus(vd_bus))){
                        ISIL_DBG(ISIL_DBG_FATAL, "sync to hardware failed\n");
                    }
                    if(video_bus->op->notify_resume_logic_chan(video_bus)){
                        ISIL_DBG(ISIL_DBG_FATAL, "resume logic channel failed\n");
                        return -EBUSY;
                    }
                    /*notify logic video channel*/
                    if(video_bus->op->notify_logic_chan_change(video_bus)){
                        ISIL_DBG(ISIL_DBG_FATAL, "sync to logic channel failed\n");
                        return -EBUSY;
                    }
                    ret = ISIL_OK;
                }
                break;
            case ISIL_CHIP_VD_CHNL_CONFIG_GET:
                if(video_bus) {
                    isil_vd_cross_bus_t *vd_bus;
                    isil_h264_logic_video_slot_t *logic_video_slot;
                    isil_h264_phy_video_slot_t *phy_video_slot;
                    struct isil_vd_chan_param vd_chan_param;

                    if(copy_from_user(&vd_chan_param, (void *)arg, sizeof(struct isil_vd_chan_param)) != 0){
                        printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                    }
                    if(vd_chan_param.i_phy_chan_id > ISIL_PHY_VD_CHAN_NUMBER){
                        ISIL_DBG(ISIL_DBG_ERR, "channle number %d overflow\n", vd_chan_param.i_phy_chan_id);
                        ret = -EINVAL;
                        break;
                    }

                    vd_bus = video_bus->vd_bus_driver;
                    vd_bus->op->get_phy_video_slot_by_phy_id(vd_bus, vd_chan_param.i_phy_chan_id, &phy_video_slot);
                    logic_video_slot = phy_video_slot->cross_bus_logic_video_slot;
                    vd_chan_param.i_phy_chan_enable = logic_video_slot->op->get_enableslot(logic_video_slot);
                    vd_chan_param.i_phy_chan_fps    = logic_video_slot->op->get_fps(logic_video_slot);
                    vd_chan_param.i_phy_video_width_mb_size = VIDEO_SIZE_TO_WIDTH(logic_video_slot->op->get_video_size(logic_video_slot), 
                            video_bus->op->get_video_standard(video_bus))>>4;
                    vd_chan_param.i_phy_video_height_mb_size= VIDEO_SIZE_TO_HEIGHT(logic_video_slot->op->get_video_size(logic_video_slot), 
                            video_bus->op->get_video_standard(video_bus))>>4;
                    vd_chan_param.hor_reverse = logic_video_slot->hor_reverse;
                    vd_chan_param.ver_reverse = logic_video_slot->ver_reverse;
                    vd_chan_param.i_interlace = phy_video_slot->op->get_interlace(phy_video_slot);

                    if(copy_to_user((void *)arg, &vd_chan_param, sizeof(struct isil_vd_chan_param)) != 0){
                        printk("%s, %d: copy_to_user failed\n", __FUNCTION__, __LINE__);
                    }

                    ret = ISIL_OK;
                }
                break;
            case ISIL_CHIP_VD_MAP_PARAM_SET:
                if(video_bus) {
                    isil_vd_cross_bus_t *vd_bus;
                    isil_h264_logic_video_slot_t *logic_video_slot;
                    isil_h264_phy_video_slot_t *phy_video_slot;
                    isil_vd_config_param_t vd_config_param;
                    vd_chan_map_info_t *map_info;
                    u32 logic_mask, phy_mask, last_video_size = 0, consistency = ISIL_TRUE, first_enable = 0;
                    isil_chip_t *chip;
                    int i;

                    if(!arg) {
                        ISIL_DBG(ISIL_DBG_ERR, "arg null\n");
                        ret = -EINVAL;
                        break;
                    }

                    if(copy_from_user(&vd_config_param, (void *)arg, sizeof(isil_vd_config_param_t)) != 0){
                        printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                    }
                    if(video_bus->op->notify_suspend_logic_chan(video_bus)){
                        ISIL_DBG(ISIL_DBG_FATAL, "suspend logic channel failed\n");
                        return -EBUSY;
                    }

                    chip = driver->chip;
                    vd_bus = video_bus->vd_bus_driver;
                    map_info = vd_config_param.map_info;
                    logic_mask = 0;
                    phy_mask = 0;
                    for(i = 0; i < ISIL_CROSS_BUS_LOGIC_VD_CHAN_NUMBER; i++, map_info++) {
                        if(map_info->map_logic_slot_id > ISIL_CROSS_BUS_LOGIC_VD_CHAN_NUMBER){
                            ISIL_DBG(ISIL_DBG_ERR, "map_logic_slot_id = %d overflow\n", map_info->map_logic_slot_id);
                            return -EINVAL;
                        }
                        if(map_info->phy_slot_id > ISIL_PHY_VD_CHAN_NUMBER){
                            ISIL_DBG(ISIL_DBG_ERR, "phy_slot_id = %d overflow\n", map_info->phy_slot_id);
                            return -EINVAL;
                        }
                        if((map_info->video_size > ISIL_VIDEO_SIZE_USER) || (map_info->video_size < 0)){
                            ISIL_DBG(ISIL_DBG_ERR, "video_size = %d unsupport\n", map_info->video_size);
                            return -EINVAL;
                        }
                        if(map_info->video_size == ISIL_VIDEO_SIZE_QCIF){
                            map_info->video_size = ISIL_VIDEO_SIZE_CIF;
                        }
                        if(logic_mask & (1 << map_info->map_logic_slot_id)){
                            ISIL_DBG(ISIL_DBG_ERR, "map_logic_slot_id %d repeat\n", map_info->map_logic_slot_id);
                            return -EINVAL;
                        }
                        logic_mask |= (1 << map_info->map_logic_slot_id);

                        if(phy_mask & (1 << map_info->phy_slot_id)){
                            ISIL_DBG(ISIL_DBG_ERR, "phy_slot_id %d repeat\n", map_info->phy_slot_id);
                            return -EINVAL;
                        }
                        phy_mask |= (1 << map_info->phy_slot_id);
                    }
                    map_info = vd_config_param.map_info;
                    logic_mask = 0;
                    phy_mask = 0;
                    for(i = 0; i < ISIL_CROSS_BUS_LOGIC_VD_CHAN_NUMBER; i++, map_info++) {
                        if(map_info->map_logic_slot_id > ISIL_CROSS_BUS_LOGIC_VD_CHAN_NUMBER){
                            ISIL_DBG(ISIL_DBG_ERR, "map_logic_slot_id = %d overflow\n", map_info->map_logic_slot_id);
                            return -EINVAL;
                        }
                        if(map_info->phy_slot_id > ISIL_PHY_VD_CHAN_NUMBER){
                            ISIL_DBG(ISIL_DBG_ERR, "phy_slot_id = %d overflow\n", map_info->phy_slot_id);
                            return -EINVAL;
                        }
                        if((map_info->video_size > ISIL_VIDEO_SIZE_USER) || (map_info->video_size < 0)){
                            ISIL_DBG(ISIL_DBG_ERR, "video_size = %d unsupport\n", map_info->video_size);
                            return -EINVAL;
                        }
                        if(logic_mask & (1 << map_info->map_logic_slot_id)){
                            ISIL_DBG(ISIL_DBG_ERR, "map_logic_slot_id %d repeat\n", map_info->map_logic_slot_id);
                            return -EINVAL;
                        }
                        logic_mask |= (1 << map_info->map_logic_slot_id);

                        if(phy_mask & (1 << map_info->phy_slot_id)){
                            ISIL_DBG(ISIL_DBG_ERR, "phy_slot_id %d repeat\n", map_info->phy_slot_id);
                            return -EINVAL;
                        }
                        phy_mask |= (1 << map_info->phy_slot_id);
                        if(map_info->enable && (map_info->fps > 0)) {
                            first_enable++;
                            if(consistency == ISIL_TRUE ) {
                                if(first_enable == 1) {
                                    last_video_size = map_info->video_size;
                                }
                                if(map_info->video_size != last_video_size) {
                                    consistency = ISIL_FALSE;
                                }
                            }
                        }

                        ISIL_DBG(ISIL_DBG_DEBUG, "phy_slot_id = %2d, map_logic_slot_id = %2d, enable = %d, fps = %2d, roundTableStride = %2d, video_size = %2d, discard_table = 0x%08x, interlace = %d\n",
                                map_info->phy_slot_id, map_info->map_logic_slot_id, map_info->enable, map_info->fps, map_info->roundTableStride, map_info->video_size,
                                map_info->logic_slot_discard_table, map_info->interlace);
                        logic_video_slot = vd_bus->logic_video_slot + map_info->map_logic_slot_id;
                        logic_video_slot->op->set_mapPhySlotId(logic_video_slot, map_info->phy_slot_id);
                        logic_video_slot->op->set_discardTable(logic_video_slot, map_info->logic_slot_discard_table);
                        logic_video_slot->op->set_roundTableStride(logic_video_slot, map_info->roundTableStride);
                        logic_video_slot->op->set_video_size(logic_video_slot, map_info->video_size);
                        logic_video_slot->op->set_enableslot(logic_video_slot, map_info->enable);
                        logic_video_slot->op->set_fps(logic_video_slot, map_info->fps);
                        atomic_set(&logic_video_slot->needUpdateFlag, 1);
                        vd_bus->op->get_phy_video_slot_by_phy_id(vd_bus, map_info->phy_slot_id, &phy_video_slot);
                        phy_video_slot->op->set_interlace(phy_video_slot, map_info->interlace);
                        phy_video_slot->op->set_video_size(phy_video_slot, logic_video_slot->op->get_video_size(logic_video_slot));
                        phy_video_slot->op->update_map_logic_slot(phy_video_slot, logic_video_slot->logicSlotId, logic_video_slot);
                    }

                    if(consistency == ISIL_FALSE) {
                        chip->video_encode_cap->ddr_map_mode = DDR_MAP_COMPRESS_DISABLE;
                        ISIL_DBG(ISIL_DBG_INFO, "change video_encode_work_mode to DDR_MAP_COMPRESS_DISABLE\n");
                    }else{
                        chip->video_encode_cap->ddr_map_mode = DDR_MAP_COMPRESS_ENABLE;
                        ISIL_DBG(ISIL_DBG_INFO, "change video_encode_work_mode to DDR_MAP_COMPRESS_ENABLE\n");
                    }

                    if(vd_bus->op->set_chip_end_cross_bus(vd_bus, to_get_chip_with_chip_cross_bus(vd_bus))){
                        ISIL_DBG(ISIL_DBG_FATAL, "sync to hardware failed\n");
                    }
                    if(video_bus->op->notify_resume_logic_chan(video_bus)){
                        ISIL_DBG(ISIL_DBG_FATAL, "resume logic channel failed\n");
                        return -EBUSY;
                    }
                    /*notify logic video channel*/
                    if(video_bus->op->notify_logic_chan_change(video_bus)){
                        ISIL_DBG(ISIL_DBG_FATAL, "sync to logic channel failed\n");
                        return -EBUSY;
                    }
                    ret = ISIL_OK;
                }
                break;
            case ISIL_CHIP_VD_MAP_PARAM_GET:
                return -ENOSYS;
                break;
            case ISIL_CHIP_VP_MAP_PARAM_SET:
                return -ENOSYS;
                break;
            case ISIL_CHIP_VP_MAP_PARAM_GET:
                return -ENOSYS;
                break;
            case ISIL_CHIP_VD_WORK_MODE_GET:
                if(video_bus) {
                    u32 mode;
                    isil_vd_cross_bus_t *vd_bus;

                    vd_bus = video_bus->vd_bus_driver;
                    mode = vd_bus->op->get_work_mode(vd_bus);
                    if(copy_to_user((void *)arg, &mode, sizeof(u32)) != 0){
                        printk("%s, %d: copy_to_user failed\n", __FUNCTION__, __LINE__);
                    }
                    ret = ISIL_OK;
                }else{
                    ret = -ENOSYS;
                }
                break;
            case ISIL_CHIP_VD_WORK_MODE_SET:
                if(video_bus) {
                    u32 mode;
                    isil_vd_cross_bus_t *vd_bus;

                    vd_bus = video_bus->vd_bus_driver;
                    if(copy_from_user(&mode, (void *)arg, sizeof(u32)) != 0){
                        printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                    }

                    ret = vd_bus->op->update_work_mode(vd_bus, mode);
                    if(ret) {
                        ISIL_DBG(ISIL_DBG_ERR, "update work mode failed\n");
                    }else{
                        isil_h264_logic_video_slot_t	*logic_video_slot;
                        isil_h264_phy_video_slot_t	*phy_video_slot;
                        vd_chan_map_info_t	chan_map_table[ISIL_PHY_VD_CHAN_NUMBER], *chan_map;
                        isil_chip_t	*chip = NULL;
                        u32  local_fps = 0;
                        int	i;

                        chip = driver->chip;
                        chan_map = chan_map_table;
                        if(video_bus->op->get_video_standard(video_bus) == ISIL_VIDEO_STANDARD_PAL) {
                            local_fps = 25;
                        }else{
                            local_fps = 30;
                        }
                        switch(vd_bus->op->get_work_mode(vd_bus)){
                            case ISIL_CROSS_BUS_4D1_REALTIME:
                                for(i=0; i<4; i++, chan_map++){
                                    chan_map->phy_slot_id = i;
                                    chan_map->fps = local_fps;
                                    chan_map->enable = 1;
                                    chan_map->video_size = ISIL_VIDEO_SIZE_D1;
                                }

                                for(;i<ISIL_PHY_VD_CHAN_NUMBER; i++, chan_map++){
                                    chan_map->phy_slot_id = ISIL_LOGIC_VD_INVALID;
                                    chan_map->enable = 0;
                                    chan_map->fps = 0;
                                }
                                break;
                            case ISIL_CROSS_BUS_8HalfD1_REALTIME:
                                for(i=0; i<8; i++, chan_map++){
                                    chan_map->phy_slot_id = i;
                                    chan_map->fps = local_fps;
                                    chan_map->enable = 1;
                                    chan_map->video_size = ISIL_VIDEO_SIZE_HALF_D1;
                                }
                                for(; i < ISIL_PHY_VD_CHAN_NUMBER; i++, chan_map++){
                                    chan_map->phy_slot_id = ISIL_LOGIC_VD_INVALID;
                                    chan_map->enable = 0;
                                    chan_map->fps = 0;
                                }
                                break;
                            case ISIL_CROSS_BUS_16CIF_REALTIME:
                                for(i=0; i<ISIL_PHY_VD_CHAN_NUMBER; i++, chan_map++){
                                    chan_map->phy_slot_id = i;
                                    chan_map->fps = local_fps;
                                    chan_map->enable = 1;
                                    chan_map->video_size = ISIL_VIDEO_SIZE_CIF;
                                }
                                break;
                            default:
                            case ISIL_CROSS_BUS_UNREALTIME:
                                for(i=0; i<ISIL_PHY_VD_CHAN_NUMBER; i++, chan_map++){
                                    logic_video_slot = &vd_bus->logic_video_slot[i];
                                    chan_map->phy_slot_id = logic_video_slot->op->get_mapPhySlotId(logic_video_slot);
                                    chan_map->fps = logic_video_slot->op->get_fps(logic_video_slot);
                                    chan_map->enable = logic_video_slot->op->get_enableslot(logic_video_slot);
                                    chan_map->video_size = logic_video_slot->op->get_video_size(logic_video_slot);
                                }
                        }

                        if(vd_bus->op->calculate_cross_bus_map_table(vd_bus, chan_map_table)){
                            ISIL_DBG(ISIL_DBG_ERR, "calculate map table failed, please check parameter!\n");
                            return -EINVAL;
                        }
                        chan_map = chan_map_table;
                        if(video_bus->op->notify_suspend_logic_chan(video_bus)){
                            ISIL_DBG(ISIL_DBG_FATAL, "suspend logic channel failed\n");
                            return -EBUSY;
                        }
                        for(i = 0; i < ISIL_CROSS_BUS_LOGIC_VD_CHAN_NUMBER; i++, chan_map++) {
                            if(chan_map->map_logic_slot_id >= ISIL_CROSS_BUS_LOGIC_VD_CHAN_NUMBER) {
                                ISIL_DBG(ISIL_DBG_ERR, "logic channel number %d overflow\n", chan_map->map_logic_slot_id);
                                return -EINVAL;
                            }
                            logic_video_slot = &vd_bus->logic_video_slot[chan_map->map_logic_slot_id];
                            logic_video_slot->op->set_mapPhySlotId(logic_video_slot, chan_map->phy_slot_id);
                            logic_video_slot->op->set_discardTable(logic_video_slot, chan_map->logic_slot_discard_table);
                            logic_video_slot->op->set_roundTableStride(logic_video_slot, chan_map->roundTableStride);
                            logic_video_slot->op->set_video_size(logic_video_slot, chan_map->video_size);
                            logic_video_slot->op->set_enableslot(logic_video_slot, chan_map->enable);
                            logic_video_slot->op->set_fps(logic_video_slot, chan_map->fps);
                            atomic_set(&logic_video_slot->needUpdateFlag, 1);
                            //vd_bus->op->get_phy_video_slot_by_phy_id(vd_bus, chan_map->phy_slot_id, &phy_video_slot);
                            phy_video_slot = &vd_bus->phy_video_slot[chan_map->phy_slot_id];
                            phy_video_slot->op->set_interlace(phy_video_slot, ISIL_FRAME_MODE_INTERLACE);
                            phy_video_slot->op->set_video_size(phy_video_slot, logic_video_slot->op->get_video_size(logic_video_slot));
                            phy_video_slot->op->update_map_logic_slot(phy_video_slot, logic_video_slot->logicSlotId, logic_video_slot);
                        }
                        if(vd_bus->op->set_chip_end_cross_bus(vd_bus, to_get_chip_with_chip_cross_bus(vd_bus))){
                            ISIL_DBG(ISIL_DBG_FATAL, "sync to hardware failed\n");
                        }
                        if(video_bus->op->notify_resume_logic_chan(video_bus)){
                            ISIL_DBG(ISIL_DBG_FATAL, "resume logic channel failed\n");
                            return -EBUSY;
                        }
                        /*notify logic video channel*/
                        if(video_bus->op->notify_logic_chan_change(video_bus)){
                            ISIL_DBG(ISIL_DBG_FATAL, "sync to logic channel failed\n");
                            return -EBUSY;
                        }
                        ret = ISIL_OK;
                    }
                }else{
                    ret = -ENOSYS;
                }
                break;
            case ISIL_CHIP_VIN_CHAN_NUMBER_GET:
                {
                    struct isil_chip_vi_info vi_info;

                    vi_info.i_vin_ad_chan_number = ISIL_VI_ON_CHIP_CHNL;
                    vi_info.i_vin_chan_nubmer    = ISIL_PHY_VD_CHAN_NUMBER;
                    vi_info.i_cascade_vin_ad_chan_number = ISIL_PHY_VD_CHAN_NUMBER - ISIL_VI_ON_CHIP_CHNL;
                    if(arg) {
                        if(copy_to_user((void *)arg, &vi_info, sizeof(struct isil_chip_vi_info)) != 0){
                            printk("%s, %d: copy_to_user failed\n", __FUNCTION__, __LINE__);
                        }
                        ret = ISIL_OK;
                    }else{
                        ISIL_DBG(ISIL_DBG_ERR, "arg is null\n");
                        ret = -EINVAL;
                    }
                    break;
                }
            case ISIL_CHIP_VOUT_CHAN_NUMBER_GET:
                {
                    struct isil_chip_vo_info vo_info;

                    vo_info.i_vout_ad_chan_number = 0;
                    vo_info.i_vout_chan_nubmer    = 0;
                    if(arg) {
                        if(copy_to_user((void *)arg, &vo_info, sizeof(struct isil_chip_vo_info)) != 0){
                            printk("%s, %d: copy_to_user failed\n", __FUNCTION__, __LINE__);
                        }
                        ret = ISIL_OK;
                    }else{
                        ISIL_DBG(ISIL_DBG_ERR, "arg is null\n");
                        ret = -EINVAL;
                    }
                    break;
                }
            case ISIL_CHIP_AIN_CHAN_NUMBER_GET:
                {
                    struct isil_chip_ai_info ai_info;

                    ai_info.i_ain_ad_chan_number = ISIL_VI_ON_CHIP_CHNL;
                    ai_info.i_ain_chan_nubmer    = ISIL_AUDIO_IN_CHAN16_ID + 1;
                    ai_info.i_cascade_ain_ad_chan_number = 0;
                    if(arg) {
                        if(copy_to_user((void *)arg, &ai_info, sizeof(struct isil_chip_ai_info)) != 0){
                            printk("%s, %d: copy_to_user failed\n", __FUNCTION__, __LINE__);
                        }
                        ret = ISIL_OK;
                    }else{
                        ISIL_DBG(ISIL_DBG_ERR, "arg is null\n");
                        ret = -EINVAL;
                    }
                    break;
                }
            case ISIL_CHIP_AOUT_CHAN_NUMBER_GET:
                {
                    struct isil_chip_ao_info ao_info;

                    ao_info.i_aout_ad_chan_number = ISIL_AUDIO_OUT_PLAYBACK_ID - ISIL_AUDIO_IN_CHAN16_ID;
                    ao_info.i_aout_chan_nubmer    = ISIL_AUDIO_OUT_PLAYBACK_ID - ISIL_AUDIO_IN_CHAN16_ID;
                    if(arg) {
                        if(copy_to_user((void *)arg, &ao_info, sizeof(struct isil_chip_ao_info)) != 0){
                            printk("%s, %d: copy_to_user failed\n", __FUNCTION__, __LINE__);
                        }
                        ret = ISIL_OK;
                    }else{
                        ISIL_DBG(ISIL_DBG_ERR, "arg is null\n");
                        ret = -EINVAL;
                    }
                    break;
                }
            case ISIL_CHIP_ID_GET:
                if(driver) {
                    isil_chip_t *chip = driver->chip;
                    struct isil_chip_id_info chip_info, *p_info = &chip_info;

#ifdef  PCI_PLATFORM
                    struct pci_dev *pci_dev;

                    pci_dev = to_pci_dev(chip->dev);
                    p_info->i_vendor_id  = pci_dev->vendor;
                    p_info->i_version_id = pci_dev->device;
#else
                    p_info->i_vendor_id  = 0xffffffff;/*unknown ID*/
                    p_info->i_version_id = 0xffffffff;
#endif
                    if(arg) {
                        if(copy_to_user((void *)arg, p_info, sizeof(struct isil_chip_id_info)) != 0){
                            printk("%s, %d: copy_to_user failed\n", __FUNCTION__, __LINE__);
                        }
                    }
                }
                ret = ISIL_OK;
                break;
            case ISIL_CHIP_CONFIG_WRITE_REG:
                if(driver) {
                    struct isil_reg_conf reg;
                    if(arg) {
                        void __iomem *vaddr;
                        u32 offset;

                        if(copy_from_user(&reg, (void *)arg, sizeof(struct isil_reg_conf)) != 0){
                            printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                        }
                        vaddr  = (void *)(reg.addr & (~(PAGE_SIZE - 1)));
                        offset = reg.addr & (PAGE_SIZE - 1);

                        vaddr = ioremap((u32 )vaddr, PAGE_SIZE);
                        if(!vaddr) {
                            ISIL_DBG(ISIL_DBG_ERR, "ioremap error\n");
                            ret = -EFAULT;
                        }else{
                            __raw_writel(reg.val, vaddr + offset);
                            iounmap(vaddr);
                            ret = ISIL_OK;
                        }
                    }
                }
                break;
            case ISIL_CHIP_CONFIG_READ_REG:
                if(driver) {
                    struct isil_reg_conf reg;
                    if(arg) {
                        void __iomem *vaddr;
                        u32 offset;

                        if(copy_from_user(&reg, (void *)arg, sizeof(struct isil_reg_conf)) != 0){
                            printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                        }
                        vaddr  = (void *)(reg.addr & (~(PAGE_SIZE - 1)));
                        offset = reg.addr & (PAGE_SIZE - 1);

                        vaddr = ioremap((u32 )vaddr, PAGE_SIZE);
                        if(!vaddr) {
                            ISIL_DBG(ISIL_DBG_ERR, "ioremap error\n");
                            ret = -EFAULT;
                        }else{
                            reg.val = __raw_readl(vaddr + offset);
                            if(copy_to_user((void *)arg, &reg, sizeof(struct isil_reg_conf)) != 0){
                                printk("%s, %d: copy_to_user failed\n", __FUNCTION__, __LINE__);
                            }
                            iounmap(vaddr);
                            ret = ISIL_OK;
                        }
                    }
                }
                break;
            case ISIL_CHIP_LOGIC_MAP_TABLE_SET:
                {
                    isil_logic_map_table_t *map_table;
                    int table_size = sizeof(isil_logic_map_table_t) + sizeof(*map_table->map_table) * (H264_MAX(ISIL_PHY_VD_CHAN_NUMBER, ISIL_AUDIO_OUT_PLAYBACK_ID) + 1);

                    map_table = (isil_logic_map_table_t *)kmalloc(table_size, GFP_KERNEL);
                    if(!map_table) {
                        ISIL_DBG(ISIL_DBG_ERR, "no memory for map table\n");
                        ret = -ENOMEM;
                        break;
                    }
                    if(arg) {
                        if(copy_from_user(map_table, (void *)arg, sizeof(isil_logic_map_table_t)) != 0){
                            printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                        }
                        if((map_table->e_bind_type >= LOGIC_MAP_TABLE_BIND_RESERVED) || (map_table->e_bind_type < 0)) {
                            ISIL_DBG(ISIL_DBG_ERR, "unknown e_bind_type %d\n", map_table->e_bind_type);
                            ret = -EINVAL;
                        }
                        if(map_table->i_chan_number > (H264_MAX(ISIL_PHY_VD_CHAN_NUMBER, ISIL_AUDIO_OUT_PLAYBACK_ID) + 1)) {
                            ISIL_DBG(ISIL_DBG_ERR, "mpa table channel number %d overflow\n", map_table->i_chan_number);
                            ret = -EINVAL;
                        }else{
                            isil_logic_phy_table_t *logic_map_phy;


                            if(copy_from_user(map_table, (void *)arg, sizeof(isil_logic_map_table_t))){
                                printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                            }
                            logic_map_phy = &driver->logic_map_phy;
                            logic_map_phy->op->update_map_table(logic_map_phy, map_table->e_bind_type, map_table);

                            ret = ISIL_OK;
                        }
                        if(map_table) {
                            kfree(map_table);
                        }
                    }else{
                        ISIL_DBG(ISIL_DBG_ERR, "arg is null\n");
                        ret = -EINVAL;
                    }
                }
                break;
            case ISIL_CHIP_LOGIC_MAP_TABLE_GET:
                {
                    isil_logic_map_table_t *map_table;
                    int table_size = sizeof(isil_logic_map_table_t) + sizeof(*map_table->map_table) * (H264_MAX(ISIL_PHY_VD_CHAN_NUMBER, ISIL_AUDIO_OUT_PLAYBACK_ID) + 1);

                    map_table = (isil_logic_map_table_t *)kmalloc(table_size, GFP_KERNEL);
                    if(!map_table) {
                        ISIL_DBG(ISIL_DBG_ERR, "no memory for map table\n");
                        ret = -ENOMEM;
                        break;
                    }
                    if(arg) {
                        if(copy_from_user(map_table, (void *)arg, sizeof(isil_logic_map_table_t)) != 0){
                            printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                        }
                        if((map_table->e_bind_type >= LOGIC_MAP_TABLE_BIND_RESERVED) || (map_table->e_bind_type < 0)) {
                            ISIL_DBG(ISIL_DBG_ERR, "unknown e_bind_type %d\n", map_table->e_bind_type);
                            ret = -EINVAL;
                        }
                        if(map_table->i_chan_number > (H264_MAX(ISIL_PHY_VD_CHAN_NUMBER, ISIL_AUDIO_OUT_PLAYBACK_ID) + 1)) {
                            ISIL_DBG(ISIL_DBG_ERR, "mpa table channel number %d overflow\n", map_table->i_chan_number);
                            ret = -EINVAL;
                        }else{
                            isil_logic_phy_table_t *logic_map_phy;

                            logic_map_phy = &driver->logic_map_phy;
                            logic_map_phy->op->get_map_table(logic_map_phy, map_table->e_bind_type, map_table);
                            if(copy_to_user((void *)arg, map_table, sizeof(isil_logic_map_table_t) + sizeof(*map_table->map_table) * (map_table->i_chan_number)) != (sizeof(isil_logic_map_table_t) + sizeof(*map_table->map_table) * (map_table->i_chan_number))){
                                printk("%s, %d: copy_to_user failed\n", __FUNCTION__, __LINE__);
                            }

                            ret = ISIL_OK;
                        }
                        if(map_table) {
                            kfree(map_table);
                        }
                    }else{
                        ISIL_DBG(ISIL_DBG_ERR, "arg is null\n");
                        ret = -EINVAL;
                    }
                }
                break;

            case ISIL_CHIP_GET_VERSION:
                snprintf(&tmp[0], 256, "Version: %d.%d.%d-%d,GCC Version: %s,ARCH: %s,Build at: %s:%s", 
                        get_version_major(__ISIL__CODE__VERSION__), get_version_submajor(__ISIL__CODE__VERSION__), get_version_minor(__ISIL__CODE__VERSION__), __ISIL_SVN_VERSION__,
                        __VERSION__, __ISIL_ARCH__ , __DATE__, __TIME__);

                if(copy_to_user((void *)arg, &tmp[0], strlen(tmp)) != 0){
                    printk("copy to user failed!\n");
                }
                break;

	    /* Testing I/O */
            case ISIL_CHIP_REG_READ:
	    {
                isil_chip_t *chip = driver->chip;
		struct reg_info *p = (struct reg_info *)arg;
		int *buf = vmalloc(p->count);
		int i;

                ISIL_DBG(ISIL_DBG_INFO, "REG_READ addr 0x%lx count %ld\n", p->startaddr, p->count);
		for (i = 0; i < p->count; i++) {
			buf[i] = chip->io_op->chip_read32(chip, p->startaddr + (i << 2));
		}
		ret = copy_to_user((void __user *)p->buffer, (char *)buf, p->count << 2);
                if(ret > 0){
                    ret = -EFAULT;
                }
		vfree(buf);
	    }
                break;
            case ISIL_CHIP_REG_WRITE:
	    {
                isil_chip_t *chip = driver->chip;
		struct reg_info *p = (struct reg_info *)arg;
		int *buf = vmalloc(p->count);
		int i;

		ret = copy_from_user((char *)buf, (void __user *)p->buffer, p->count << 2);
		for (i = 0; i < p->count; i++) {
			chip->io_op->chip_write32(chip, p->startaddr + (i << 2), buf[i]);
		}
                if(ret > 0){
                    ret = -EFAULT;
                }
		vfree(buf);
	    }
                break;
            case ISIL_CHIP_MPB_READ:
	    {
                isil_chip_t *chip = driver->chip;
		struct mpb_info *p = (struct mpb_info *)arg;
		char *buf = vmalloc(p->count);
		int i;
		for (i = 0; i < p->count; i++) {
			buf[i] = mpb_read(chip, p->startaddr + i);
		}
		ret = copy_to_user((void __user *)p->buffer, (char *)buf, p->count);
                if(ret > 0){
                    ret = -EFAULT;
                }
		vfree(buf);
	    }
                break;
            case ISIL_CHIP_MPB_WRITE:
	    {
                isil_chip_t *chip = driver->chip;
		struct mpb_info *p = (struct mpb_info *)arg;
		char *buf = vmalloc(p->count);
		int i;

		ret = copy_from_user((char *)buf, (void __user *)p->buffer, p->count);
		for (i = 0; i < p->count; i++) {
			mpb_write(chip, p->startaddr + i, buf[i]);
		}
                if(ret > 0){
                    ret = -EFAULT;
                }
		vfree(buf);
	    }
                break;
            case ISIL_CHIP_I2C_READ:
	    {
                isil_chip_t *chip = driver->chip;
		struct i2c_info *p = (struct i2c_info *)arg;
		char *buf = vmalloc(p->count);
		int i;
		for (i = 0; i < p->count; i++) {
			pci_i2c_read(chip, p->busaddr, p->offset + i, &buf[i]);
		}
		ret = copy_to_user((void __user *)p->buffer, buf, p->count);
                if(ret > 0){
                    ret = -EFAULT;
                }
		vfree(buf);
	    }
                break;
            case ISIL_CHIP_I2C_WRITE:
	    {
                isil_chip_t *chip = driver->chip;
		struct i2c_info *p = (struct i2c_info *)arg;
		char *buf = vmalloc(p->count);
		int i;
		ret = copy_from_user((char *)buf, (void __user *)p->buffer, p->count);
                if(ret > 0){
                    ret = -EFAULT;
                    break;
                }
		for (i = 0; i < p->count; i++) {
			pci_i2c_write(chip, p->busaddr, p->offset + i, buf[i]);
		}
		vfree(buf);
	    }
                break;
            case ISIL_CHIP_DDR_READ:
	    {
                isil_chip_t *chip = driver->chip;
		struct ddr_info *p = (struct ddr_info *)arg;
		dpram_control_t *chip_dpram_controller;
		chip_ddr_burst_engine_t *chip_ddr_burst_interface;
		ddr_burst_interface_t *burst_interface;
		dpram_page_node_t *dpram_page;
		unsigned int ddr_end_addr, page_id;
		unsigned int section_size = 0x800;
		unsigned int section_cnt;
		int i = 0;
		void *buf = (void *)__get_free_page(GFP_KERNEL | GFP_DMA);
		dma_addr_t buf_phy = dma_map_single(NULL, buf, PAGE_SIZE, DMA_FROM_DEVICE);
                
		chip->chip_open(chip);	// remove in the future
		get_isil_dpram_controller(&chip_dpram_controller);
		chip_ddr_burst_interface = &chip->chip_ddr_burst_interface;
		chip_ddr_burst_interface->op->get_ddr_burst_interface(chip_ddr_burst_interface, &burst_interface);
                section_cnt = ((p->count << 2) + (section_size - 1)) / section_size;
                
                while((i < section_cnt) && chip_dpram_controller->op->is_can_submit_move_data_from_ddr_to_dpram_service_req(chip_dpram_controller, &dpram_page, chip)){
                    ddr_end_addr = p->startaddr + section_size * i;
		    page_id = ddr_end_addr >> 19;

                    burst_interface->op->start_block_transfer_from_ddr_to_sram(burst_interface, dpram_page, 
                            ddr_end_addr, page_id, section_size, DDR_CHIP_A);
#if defined(PIO_INTERFACE)
                    burst_interface->op->pio_host_to_sram_read(burst_interface, dpram_page, buf, section_size, 0);
#elif defined(DMA_INTERFACE)
                    burst_interface->op->dma_host_to_sram_read(burst_interface, dpram_page, buf_phy, section_size, 0);
#endif
                    chip_dpram_controller->op->notify_end_move_data_from_dpram_to_host_service_req(chip_dpram_controller, &dpram_page, chip);
		    ret = copy_to_user((void __user *)(p->buffer + (section_size >> 2) * i), (char *)buf, p->count << 2);
                    if(ret > 0){
                        ret = -EFAULT;
                        break;
                    }
                    i++;
                }

		chip->chip_close(chip);	// remove in the future
                dma_unmap_single(NULL, buf_phy, PAGE_SIZE, DMA_FROM_DEVICE);
		free_page((unsigned long)buf);
	    }
                break;
            case ISIL_CHIP_DDR_WRITE:
	    {
                isil_chip_t *chip = driver->chip;
		struct ddr_info *p = (struct ddr_info *)arg;
		dpram_control_t *chip_dpram_controller;
		chip_ddr_burst_engine_t *chip_ddr_burst_interface;
		ddr_burst_interface_t *burst_interface;
		dpram_page_node_t *dpram_page;
		unsigned int ddr_end_addr, page_id;
		unsigned int section_size = 0x800;
		unsigned int section_cnt;
		int i = 0;
		void *buf = (void *)__get_free_page(GFP_KERNEL | GFP_DMA);
		dma_addr_t buf_phy = dma_map_single(NULL, buf, PAGE_SIZE, DMA_TO_DEVICE);

		chip->chip_open(chip);	// remove in the future
		get_isil_dpram_controller(&chip_dpram_controller);
		chip_ddr_burst_interface = &chip->chip_ddr_burst_interface;
		chip_ddr_burst_interface->op->get_ddr_burst_interface(chip_ddr_burst_interface, &burst_interface);
                section_cnt = ((p->count << 2) + (section_size - 1)) / section_size;
                
		while((i < section_cnt) && chip_dpram_controller->op->is_can_submit_move_data_from_host_to_dpram_service_req(chip_dpram_controller, &dpram_page,chip)){
                    ddr_end_addr = p->startaddr + section_size * i;
		    page_id = ddr_end_addr >> 19;
		    ret = copy_from_user((char *)buf, (void __user *)(p->buffer + (section_size >> 2) * i), p->count << 2);
                    if(ret > 0){
                        ret = -EFAULT;
                        break;
                    }
#if defined(PIO_INTERFACE)
                    burst_interface->op->pio_host_to_sram_write(burst_interface, dpram_page, buf, section_size, 0);
#elif defined(DMA_INTERFACE)
                    burst_interface->op->dma_host_to_sram_write(burst_interface, dpram_page, buf_phy, section_size, 0);
#endif
                    burst_interface->op->start_block_transfer_from_sram_to_ddr(burst_interface, dpram_page, 
                            ddr_end_addr, page_id, section_size, DDR_CHIP_A);
                    chip_dpram_controller->op->notify_end_move_data_from_dpram_to_ddr_service_req(chip_dpram_controller, &dpram_page, chip);

                    i++;
		}

		chip->chip_close(chip);	// remove in the future
                dma_unmap_single(NULL, buf_phy, PAGE_SIZE, DMA_TO_DEVICE);
		free_page((unsigned long)buf);
	    }
                break;
            case ISIL_CHIP_GPIO_INPUT:
	    {
                isil_chip_t *chip = driver->chip;
		struct gpio_info *p = (struct gpio_info *)arg;
		int mode, val;

		/* set mode: input */
		mode = chip->io_op->chip_read32(chip, GPIO_REG);
		mode &= ~(0x1 << p->num);
		chip->io_op->chip_write32(chip, GPIO_REG, mode);
		/* input */
		val = chip->io_op->chip_read32(chip, GPIO_REG);
		val = (val >> p->num) & 0x1;
		ret = put_user(val, &p->value);
	    }
                break;
            case ISIL_CHIP_GPIO_OUTPUT:
	    {
                isil_chip_t *chip = driver->chip;
		struct gpio_info *p = (struct gpio_info *)arg;
		int mode, val, tmp;

		/* set mode: output */
		mode = chip->io_op->chip_read32(chip, GPIO_REG);
		mode |= (0x1 << p->num);
		chip->io_op->chip_write32(chip, GPIO_REG, mode);

		/* output */
		ret = get_user(tmp, &p->value);
		val = chip->io_op->chip_read32(chip, GPIO_REG);
		if(tmp)
			val |= (0x1 << p->num);
		else
			val &= ~(0x1 << p->num);
		chip->io_op->chip_write32(chip, GPIO_REG, val);
	    }
                break;
	    /* Testing I/O */

            default:
                ret = -EBADRQC;
                ISIL_DBG(ISIL_DBG_ERR, "No such command!\n");
                break;
        }
    }

    return ret;
}

static int chip_driver_hcd_interface_get_state(ed_tcb_t *opened_control_ed)
{
    int	ret = ISIL_ED_UNREGISTER;

    if(opened_control_ed != NULL){
        ret = atomic_read(&opened_control_ed->state);
    }

    return ret;
}

static isil_hcd_interface_operation chip_driver_hcd_interface_op = {
    .open = chip_driver_hcd_interface_open,
    .close = chip_driver_hcd_interface_close,
    .suspend = chip_driver_hcd_interface_suspend,
    .resume = chip_driver_hcd_interface_resume,
    .ioctl = chip_driver_hcd_interface_ioctl,
    .get_state = chip_driver_hcd_interface_get_state,
};

static int video_bus_init(isil_video_bus_t *video_bus, isil_chip_t *chip)
{
    int ret = ISIL_ERR;

    if(video_bus && chip) {
        video_bus->vd_bus_driver = &chip->chip_vd_cross_bus;
        video_bus->vj_bus_driver = &chip->chip_vj_bus;
        video_bus->vp_bus_driver = &chip->chip_vp_bus;
        video_bus->op->set_video_standard(video_bus, ISIL_VIDEO_STANDARD_NTSC);

        ret = init_isil_vd_cross_bus(&chip->chip_vd_cross_bus, ISIL_CROSS_BUS_4D1_REALTIME, video_bus->op->get_video_standard(video_bus));
        //ret = init_isil_vd_cross_bus(&chip->chip_vd_cross_bus, ISIL_CROSS_BUS_8HalfD1_REALTIME, video_bus->op->get_video_standard(video_bus));
        //ret = init_isil_vd_cross_bus(&chip->chip_vd_cross_bus, ISIL_CROSS_BUS_16CIF_REALTIME, video_bus->op->get_video_standard(video_bus));
        //ret = init_isil_vd_cross_bus(&chip->chip_vd_cross_bus, ISIL_CROSS_BUS_UNREALTIME, video_bus->op->get_video_standard(video_bus));
        if(ret != ISIL_OK) {
            ISIL_DBG(ISIL_DBG_ERR, "init vd bus error 0x%08x\n", ret);
            return ret;
        }

        ret = init_isil_vj_bus(&chip->chip_vj_bus);
        if(ret != ISIL_OK) {
            ISIL_DBG(ISIL_DBG_ERR, "init vj bus error 0x%08x\n", ret);
            return ret;
        }
        
        ret = init_isil_vp_bus(&chip->chip_vp_bus);
        if(ret != ISIL_OK) {
            ISIL_DBG(ISIL_DBG_ERR, "init vp bus error 0x%08x\n", ret);
            return ret;
        }

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static void video_bus_release(isil_video_bus_t *video_bus)
{
    if(video_bus) {
        remove_isil_vd_cross_bus(video_bus->vd_bus_driver);
        remove_isil_vj_bus(video_bus->vj_bus_driver);
        remove_isil_vp_bus(video_bus->vp_bus_driver);
    }
}

static int video_bus_reset(isil_video_bus_t *video_bus)
{
    if(video_bus) {
        isil_vd_cross_bus_t   *vd_bus_driver = video_bus->vd_bus_driver;
        isil_vj_bus_t         *vj_bus_driver = video_bus->vj_bus_driver;
        isil_vp_bus_t         *vp_bus_driver = video_bus->vp_bus_driver;

        if(vd_bus_driver->op) {
            vd_bus_driver->op->reset(vd_bus_driver);
        }

        if(vj_bus_driver->op) {
            vj_bus_driver->op->reset(vj_bus_driver);
        }

        if(vp_bus_driver->op) {
            vp_bus_driver->op->reset(vp_bus_driver);
        }
        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int video_bus_get_video_standard(isil_video_bus_t *video_bus)
{
    if(video_bus) {
        return video_bus->video_param.e_video_standard;
    }

    return ISIL_ERR;
}

static int video_bus_set_video_standard(isil_video_bus_t *video_bus, int video_standard)
{
    int ret = ISIL_ERR;

    if(video_bus) {
        switch(video_standard) {
            case ISIL_VIDEO_STANDARD_PAL:
            case ISIL_VIDEO_STANDARD_NTSC:
                ret = ISIL_OK;
                break;
            case ISIL_VIDEO_STANDARD_USER_DEFINE:
            default:
                ISIL_DBG(ISIL_DBG_ERR, "This video standard '%d' not support!\n", video_standard);
                ret = -EINVAL;
                break;
        }
        if(ret == ISIL_OK) {
            isil_vd_cross_bus_t   *vd_bus_driver;
            isil_vp_bus_t   *vp_bus_driver;

            ISIL_DBG(ISIL_DBG_DEBUG, "update video standard to %d\n", video_standard);
            video_bus->video_param.e_video_standard = video_standard;


            vd_bus_driver = video_bus->vd_bus_driver;
            if(vd_bus_driver && vd_bus_driver->op) {
                ret = vd_bus_driver->op->reset(vd_bus_driver);
                if(ret) {
                    ISIL_DBG(ISIL_DBG_ERR, "reset vd_bus failed %d\n", ret);
                }
            }

            vp_bus_driver = video_bus->vp_bus_driver;
            if(vp_bus_driver && vp_bus_driver->op) {
                ret = vp_bus_driver->op->reset(vp_bus_driver);
                if(ret) {
                    ISIL_DBG(ISIL_DBG_ERR, "reset vp_bus failed %d\n", ret);
                }
            }
        }
    }

    return ret;
}

static void isil_jpeg_logic_encode_notify_property_changed(void *context)
{
    if(context){
	isil_jpeg_encode_property_t   *encode_property;
	isil_mjpeg_encode_param_t     *encode_config_property;
        isil_jpeg_logic_encode_chan_t *jpeg_logic_encode_chan = (isil_jpeg_logic_encode_chan_t *)context;

        encode_property = &jpeg_logic_encode_chan->encode_property;
        encode_config_property = &encode_property->encode_property;
        encode_config_property->change_mask_flag |= ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_IMAGE_WIDTH_MASK | ISIL_MJPEG_ENCODE_PARAM_ENABLE_CHANGE_IMAGE_HEIGHT_MASK;
    }
}
static int video_bus_notify_logic_chan_change(isil_video_bus_t *video_bus)
{
    int i, logic_chan_id, phy_video_fps;
    isil_h264_phy_video_slot_t *phy_h264_slot;
    isil_h264_encode_control_t *encode_control;
    isil_jpeg_phy_video_slot_t *phy_jpeg_slot;
    isil_register_table_t *logic_chan_table;
    isil_register_node_t  *register_node;
    ed_tcb_t    *logic_chan_ed;

    isil_h264_logic_encode_chan_t *h264_logic_encode_chan;

    /*notify logic video channel*/
    phy_h264_slot = video_bus->vd_bus_driver->phy_video_slot;
    phy_jpeg_slot = video_bus->vj_bus_driver->phy_video_slot;
    for(i = 0; i < ISIL_PHY_VD_CHAN_NUMBER; i++, phy_h264_slot++, phy_jpeg_slot++) {
        logic_chan_table = &phy_h264_slot->opened_logic_chan_table;
        if(logic_chan_table->op) {
            phy_video_fps = phy_h264_slot->cross_bus_logic_video_slot->op->get_fps(phy_h264_slot->cross_bus_logic_video_slot);
            /*master stream*/
            logic_chan_id = phy_h264_slot->phy_slot_id;
            logic_chan_id <<= VIDEO_MASTER_OR_SUB_FLAG_LEFT_SHIFT_NUMBER;
            logic_chan_id |= ISIL_MASTER_BITSTREAM;
            logic_chan_table->op->find_register_node_in_table(logic_chan_table, &register_node, logic_chan_id);
            if(register_node) {
                isil_h264_encode_property_t      *encode_property;
                isil_h264_encode_configuration_t *h264_encode_config;
                unsigned long   flags;

                logic_chan_ed = to_get_endpoint_tcb_with_register_node(register_node);
                h264_logic_encode_chan = to_get_isil_h264_encode_chan_with_opened_logic_chan_ed(logic_chan_ed);
                encode_property = &h264_logic_encode_chan->encode_property;
                encode_control = &h264_logic_encode_chan->encode_control;

                spin_lock_irqsave(&encode_property->lock, flags);
                encode_control->i_crossbus_fps = phy_video_fps;
                h264_encode_config = &encode_property->config_encode_property;
                if(video_bus->vd_bus_driver->op->get_work_mode(video_bus->vd_bus_driver) == ISIL_CROSS_BUS_UNREALTIME) {
                    if(phy_video_fps < encode_property->op->get_target_fps(encode_property)) {
                        h264_encode_config->fps = phy_video_fps;
                    }else{
                        h264_encode_config->fps = encode_property->op->get_target_fps(encode_property);
                    }
                }else{
                    h264_encode_config->fps = phy_video_fps;
                }
                ISIL_DBG(ISIL_DBG_DEBUG, "adjust channel %2d fps to %d\n", h264_logic_encode_chan->logic_chan_id, h264_encode_config->fps);
                h264_encode_config->changedMask |= ISIL_ENCODE_CONFIG_ENCODE_SIZE_CHANGED | ISIL_ENCODE_CONFIG_FPS_CHANGED;
                spin_unlock_irqrestore(&encode_property->lock, flags);
            }

            /*sub stream*/
            logic_chan_id = phy_h264_slot->phy_slot_id;
            logic_chan_id <<= VIDEO_MASTER_OR_SUB_FLAG_LEFT_SHIFT_NUMBER;
            logic_chan_id |= ISIL_SUB_BITSTREAM;
            logic_chan_table = &phy_h264_slot->opened_logic_chan_table;
            logic_chan_table->op->find_register_node_in_table(logic_chan_table, &register_node, logic_chan_id);
            if(register_node) {
                isil_h264_encode_property_t      *encode_property;
                isil_h264_encode_configuration_t *h264_encode_config;
                unsigned long   flags;

                logic_chan_ed = to_get_endpoint_tcb_with_register_node(register_node);
                h264_logic_encode_chan = to_get_isil_h264_encode_chan_with_opened_logic_chan_ed(logic_chan_ed);
                encode_property = &h264_logic_encode_chan->encode_property;
                encode_control = &h264_logic_encode_chan->encode_control;

                spin_lock_irqsave(&encode_property->lock, flags);
                encode_control->i_crossbus_fps = phy_video_fps;
                h264_encode_config = &encode_property->config_encode_property;
                if(video_bus->vd_bus_driver->op->get_work_mode(video_bus->vd_bus_driver) == ISIL_CROSS_BUS_UNREALTIME) {
                    if(phy_video_fps < encode_property->op->get_target_fps(encode_property)) {
                        h264_encode_config->fps = phy_video_fps;
                    }else{
                        h264_encode_config->fps = encode_property->op->get_target_fps(encode_property);
                    }
                }else{
                    h264_encode_config->fps = phy_video_fps;
                }
                ISIL_DBG(ISIL_DBG_DEBUG, "adjust channel %2d fps to %d\n", h264_logic_encode_chan->logic_chan_id, h264_encode_config->fps);
                h264_encode_config->changedMask |= ISIL_ENCODE_CONFIG_ENCODE_SIZE_CHANGED | ISIL_ENCODE_CONFIG_FPS_CHANGED;
                spin_unlock_irqrestore(&encode_property->lock, flags);
            }
        }
        /*notify jpeg*/
        if(phy_jpeg_slot->op){
            phy_jpeg_slot->op->touch_each_register_logic_chan(phy_jpeg_slot, isil_jpeg_logic_encode_notify_property_changed);
        }
    }

    return ISIL_OK;
}

static int video_bus_notify_suspend_logic_chan(isil_video_bus_t *video_bus)
{
    if(video_bus) {
        int i, logic_chan_id;
        isil_h264_phy_video_slot_t *phy_h264_slot;
        isil_register_table_t *logic_chan_table;
        isil_register_node_t  *register_node;
        ed_tcb_t    *logic_chan_ed;

        /*notify logic video channel*/
        phy_h264_slot = video_bus->vd_bus_driver->phy_video_slot;
        for(i = 0; i < ISIL_PHY_VD_CHAN_NUMBER; i++, phy_h264_slot++) {
            logic_chan_table = &phy_h264_slot->opened_logic_chan_table;
            if(logic_chan_table->op) {
                /*master stream*/
                logic_chan_id = phy_h264_slot->phy_slot_id;
                logic_chan_id <<= VIDEO_MASTER_OR_SUB_FLAG_LEFT_SHIFT_NUMBER;
                logic_chan_id |= ISIL_MASTER_BITSTREAM;
                logic_chan_table->op->find_register_node_in_table(logic_chan_table, &register_node, logic_chan_id);
                if(register_node) {
                    logic_chan_ed = to_get_endpoint_tcb_with_register_node(register_node);
                    if(logic_chan_ed->op != NULL){
                        //logic_chan_ed->op->suspend(logic_chan_ed);
                    }
                }
                /*sub stream*/
                logic_chan_id = phy_h264_slot->phy_slot_id;
                logic_chan_id <<= VIDEO_MASTER_OR_SUB_FLAG_LEFT_SHIFT_NUMBER;
                logic_chan_id |= ISIL_SUB_BITSTREAM;
                logic_chan_table = &phy_h264_slot->opened_logic_chan_table;
                logic_chan_table->op->find_register_node_in_table(logic_chan_table, &register_node, logic_chan_id);
                if(register_node) {
                    logic_chan_ed = to_get_endpoint_tcb_with_register_node(register_node);
                    if(logic_chan_ed->op != NULL){
                        //logic_chan_ed->op->suspend(logic_chan_ed);
                    }
                }
            }
        }

        return ISIL_OK;
    }

    return -EINVAL;
}

static int video_bus_notify_resume_logic_chan(isil_video_bus_t *video_bus)
{
    if(video_bus) {
        int i, logic_chan_id;
        isil_h264_phy_video_slot_t *phy_h264_slot;
        isil_register_table_t *logic_chan_table;
        isil_register_node_t  *register_node;
        ed_tcb_t    *logic_chan_ed;

        /*notify logic video channel*/
        phy_h264_slot = video_bus->vd_bus_driver->phy_video_slot;
        for(i = 0; i < ISIL_PHY_VD_CHAN_NUMBER; i++, phy_h264_slot++) {
            /*master stream*/
            logic_chan_id = phy_h264_slot->phy_slot_id;
            logic_chan_id <<= VIDEO_MASTER_OR_SUB_FLAG_LEFT_SHIFT_NUMBER;
            logic_chan_id |= ISIL_MASTER_BITSTREAM;
            logic_chan_table = &phy_h264_slot->opened_logic_chan_table;
            if(logic_chan_table->op) {
                logic_chan_table->op->find_register_node_in_table(logic_chan_table, &register_node, logic_chan_id);
                if(register_node) {
                    logic_chan_ed = to_get_endpoint_tcb_with_register_node(register_node);
                    if(logic_chan_ed->op != NULL){
                        //logic_chan_ed->op->resume(logic_chan_ed);
                    }
                }
                /*sub stream*/
                logic_chan_id = phy_h264_slot->phy_slot_id;
                logic_chan_id <<= VIDEO_MASTER_OR_SUB_FLAG_LEFT_SHIFT_NUMBER;
                logic_chan_id |= ISIL_SUB_BITSTREAM;
                logic_chan_table = &phy_h264_slot->opened_logic_chan_table;
                logic_chan_table->op->find_register_node_in_table(logic_chan_table, &register_node, logic_chan_id);
                if(register_node) {
                    logic_chan_ed = to_get_endpoint_tcb_with_register_node(register_node);
                    if(logic_chan_ed->op != NULL){
                        //logic_chan_ed->op->resume(logic_chan_ed);
                    }
                }
            }
        }

        return ISIL_OK;
    }

    return -EINVAL;
}

struct video_bus_operation video_bus_op = {
    .init                       = video_bus_init,
    .release                    = video_bus_release,
    .reset                      = video_bus_reset,

    .get_video_standard         = video_bus_get_video_standard,
    .set_video_standard         = video_bus_set_video_standard,
    .notify_logic_chan_change   = video_bus_notify_logic_chan_change,
    .notify_suspend_logic_chan  = video_bus_notify_suspend_logic_chan,
    .notify_resume_logic_chan   = video_bus_notify_resume_logic_chan,
};

static int init_video_bus(isil_video_bus_t *video_bus, isil_chip_t *chip)
{
    if(video_bus && chip) {
        video_bus->op = &video_bus_op;
        return video_bus->op->init(video_bus, chip);
    }

    return ISIL_ERR;
}

static int audio_bus_init(isil_audio_bus_t *audio_bus, isil_chip_t *chip)
{
    int ret = ISIL_ERR;

    if(audio_bus && chip) {
        audio_bus->chip_audio_driver = &chip->chip_audio;

        ret = init_chip_audio(&chip->chip_audio, chip, chip->bus_id);
        if(ret != ISIL_OK) {
            ISIL_DBG(ISIL_DBG_ERR, "init audio error 0x%08x\n", ret);
            return ret;
        }

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static void audio_bus_release(isil_audio_bus_t *audio_bus)
{
    if(audio_bus) {
        remove_chip_audio(audio_bus->chip_audio_driver);
    }
}

static int audio_bus_reset(isil_audio_bus_t *audio_bus)
{
    if(audio_bus) {
        return ISIL_OK;
    }

    return ISIL_ERR;
}

struct audio_bus_operation audio_bus_op = {
    .init    = audio_bus_init,
    .release = audio_bus_release,
    .reset   = audio_bus_reset,
};

static int init_audio_bus(isil_audio_bus_t *audio_bus, isil_chip_t *chip)
{
    if(audio_bus && chip) {
        audio_bus->op = &audio_bus_op;
        return audio_bus->op->init(audio_bus, chip);
    }

    return ISIL_ERR;
}

static int isil_logic_phy_map_table_init(isil_logic_phy_table_t *table)
{
    if(table) {
        int type, i;
        isil_logic_map_table_t *logic_talbe;

        spin_lock_init(&table->lock);
        for(type = 0; type < LOGIC_MAP_TABLE_BIND_RESERVED; type++) {
            logic_talbe = (isil_logic_map_table_t *)kmalloc(sizeof(isil_logic_map_table_t) 
                    + sizeof(*logic_talbe->map_table) * (H264_MAX(ISIL_PHY_VD_CHAN_NUMBER, ISIL_AUDIO_OUT_PLAYBACK_ID) + 1),
                    GFP_KERNEL);
            if(!logic_talbe) {
                ISIL_DBG(ISIL_DBG_ERR, "no memory for logic map table\n");
                goto error_release;
            }
            logic_talbe->e_bind_type = type;
            logic_talbe->i_chan_number = H264_MAX(ISIL_PHY_VD_CHAN_NUMBER, ISIL_AUDIO_OUT_PLAYBACK_ID) + 1;
            for(i = 0; i < logic_talbe->i_chan_number; i++) {
                logic_talbe->map_table[i].logic_slot_id = i;
                logic_talbe->map_table[i].phy_slot_id   = i;
            }
            table->logic_talbe[type] = logic_talbe;
        }

        return ISIL_OK;
    }
error_release:
    if(table->op->release) {
        table->op->release(table);
    }

    return ISIL_ERR;
}

static int isil_logic_phy_map_table_reset(isil_logic_phy_table_t *table)
{
    if(table) {
        int type, i;
        unsigned long flags;
        isil_logic_map_table_t *logic_talbe;

        spin_lock_irqsave(&table->lock, flags);
        for(type = 0; type < LOGIC_MAP_TABLE_BIND_RESERVED; type++) {
            logic_talbe = table->logic_talbe[type];
            logic_talbe->e_bind_type = type;
            logic_talbe->i_chan_number = H264_MAX(ISIL_PHY_VD_CHAN_NUMBER, ISIL_AUDIO_OUT_PLAYBACK_ID) + 1;
            for(i = 0; i < logic_talbe->i_chan_number; i++) {
                logic_talbe->map_table[i].logic_slot_id = i;
                logic_talbe->map_table[i].phy_slot_id   = i;
            }
        }
        spin_unlock_irqrestore(&table->lock, flags);

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_logic_phy_map_table_release(isil_logic_phy_table_t *table)
{
    if(table) {
        int type;
        unsigned long flags;
        isil_logic_map_table_t *logic_table;

        spin_lock_irqsave(&table->lock, flags);
        for(type = 0; type < LOGIC_MAP_TABLE_BIND_RESERVED; type++) {
            logic_table = table->logic_talbe[type];
            if(logic_table) {
                kfree(logic_table);
            }
            table->logic_talbe[type] = NULL;
        }
        spin_unlock_irqrestore(&table->lock, flags);

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_logic_phy_map_table_find_logic_by_phy(isil_logic_phy_table_t *table, enum LOGIC_MAP_TABLE_TYPE type, u32 phy_slot_id, u32 *logic_slot_id)
{
    if(table && logic_slot_id) {
        int i;
        unsigned long flags;
        isil_logic_map_table_t *logic_table;

        if((type < 0) || (type >= LOGIC_MAP_TABLE_BIND_RESERVED)) {
            ISIL_DBG(ISIL_DBG_ERR, "map table type %d error\n", type);
            return -EINVAL;
        }

        logic_table = table->logic_talbe[type];
        if(phy_slot_id >= logic_table->i_chan_number) {
            ISIL_DBG(ISIL_DBG_ERR, "phy slot id %d overflow\n", phy_slot_id);
            return -EINVAL;
        }
        spin_lock_irqsave(&table->lock, flags);
        for(i = 0; i < logic_table->i_chan_number; i++) {
            if(logic_table->map_table[i].phy_slot_id == phy_slot_id) {
                *logic_slot_id = logic_table->map_table[i].logic_slot_id;
                spin_unlock_irqrestore(&table->lock, flags);
                return ISIL_OK;
            }
        }
        spin_unlock_irqrestore(&table->lock, flags);
        ISIL_DBG(ISIL_DBG_ERR, "no such phy_slot_id = %d, in table type %d\n", phy_slot_id, type);
        return -ENODATA;
    }

    ISIL_DBG(ISIL_DBG_ERR, "null pointer\n");

    return ISIL_ERR;
}

static int isil_logic_phy_map_table_find_phy_by_logic(isil_logic_phy_table_t *table, enum LOGIC_MAP_TABLE_TYPE type, u32 logic_slot_id, u32 *phy_slot_id)
{
    if(table && phy_slot_id) {
        int i;
        unsigned long flags;
        isil_logic_map_table_t *logic_table;

        if((type < 0) || (type >= LOGIC_MAP_TABLE_BIND_RESERVED)) {
            ISIL_DBG(ISIL_DBG_ERR, "map table type %d error\n", type);
            return -EINVAL;
        }

        logic_table = table->logic_talbe[type];
        if(logic_slot_id >= logic_table->i_chan_number) {
            ISIL_DBG(ISIL_DBG_ERR, "phy slot id %d overflow\n", logic_slot_id);
            return -EINVAL;
        }
        spin_lock_irqsave(&table->lock, flags);
        for(i = 0; i < logic_table->i_chan_number; i++) {
            if(logic_table->map_table[i].logic_slot_id == logic_slot_id) {
                *phy_slot_id = logic_table->map_table[i].phy_slot_id;
                spin_unlock_irqrestore(&table->lock, flags);
                return ISIL_OK;
            }
        }
        spin_unlock_irqrestore(&table->lock, flags);
        ISIL_DBG(ISIL_DBG_ERR, "no such logic_slot_id = %d, in table type %d\n", logic_slot_id, type);
        return -ENODATA;
    }

    ISIL_DBG(ISIL_DBG_ERR, "null pointer\n");

    return ISIL_ERR;
}

static int isil_logic_phy_map_table_update_map_table(isil_logic_phy_table_t *table, enum LOGIC_MAP_TABLE_TYPE type, isil_logic_map_table_t *new_logic_table)
{
    if(table && new_logic_table) {
        int i, j;
        unsigned long flags;
        isil_logic_map_table_t *logic_table;

        if((type < 0) || (type >= LOGIC_MAP_TABLE_BIND_RESERVED)) {
            ISIL_DBG(ISIL_DBG_ERR, "map table type %d error\n", type);
            return -EINVAL;
        }
        if(new_logic_table->i_chan_number > (H264_MAX(ISIL_PHY_VD_CHAN_NUMBER, ISIL_AUDIO_OUT_PLAYBACK_ID) + 1)) {
            ISIL_DBG(ISIL_DBG_ERR, "input logic channel number %d overflow\n", new_logic_table->i_chan_number);
            return -EINVAL;
        }
        /*check phy_slot_id and logic_slot_id unique*/
        for(i = 0; i < new_logic_table->i_chan_number; i++) {
            for(j = 0; j < new_logic_table->i_chan_number; j++) {
                if((i != j) && (new_logic_table->map_table[i].logic_slot_id == new_logic_table->map_table[j].logic_slot_id)) {
                    ISIL_DBG(ISIL_DBG_ERR, "repeat logic_slot_id %d\n", new_logic_table->map_table[i].logic_slot_id);
                    return -EINVAL;
                }
            }
        }
        for(i = 0; i < new_logic_table->i_chan_number; i++) {
            for(j = 0; j < new_logic_table->i_chan_number; j++) {
                if((i != j) && (new_logic_table->map_table[i].phy_slot_id == new_logic_table->map_table[j].phy_slot_id)) {
                    ISIL_DBG(ISIL_DBG_ERR, "repeat phy_slot_id %d\n", new_logic_table->map_table[i].phy_slot_id);
                    return -EINVAL;
                }
            }
        }

        spin_lock_irqsave(&table->lock, flags);
        logic_table = table->logic_talbe[type];
        logic_table->i_chan_number = new_logic_table->i_chan_number;
        for(i = 0; i < logic_table->i_chan_number; i++) {
            logic_table->map_table[i].logic_slot_id = new_logic_table->map_table[i].logic_slot_id;
            logic_table->map_table[i].phy_slot_id = new_logic_table->map_table[i].phy_slot_id;
        }
        spin_unlock_irqrestore(&table->lock, flags);

        return ISIL_OK;
    }

    ISIL_DBG(ISIL_DBG_ERR, "null pointer\n");

    return ISIL_ERR;
}


static int isil_logic_phy_map_table_get_map_table(isil_logic_phy_table_t *table, enum LOGIC_MAP_TABLE_TYPE type, isil_logic_map_table_t *new_logic_table)
{
    if(table && new_logic_table) {
        int i;
        unsigned long flags;
        isil_logic_map_table_t *logic_table;

        if((type < 0) || (type >= LOGIC_MAP_TABLE_BIND_RESERVED)) {
            ISIL_DBG(ISIL_DBG_ERR, "map table type %d error\n", type);
            return -EINVAL;
        }
        if(new_logic_table->i_chan_number > (H264_MAX(ISIL_PHY_VD_CHAN_NUMBER, ISIL_AUDIO_OUT_PLAYBACK_ID) + 1)) {
            ISIL_DBG(ISIL_DBG_ERR, "input logic channel number %d overflow\n", new_logic_table->i_chan_number);
            return -EINVAL;
        }

        spin_lock_irqsave(&table->lock, flags);
        logic_table = table->logic_talbe[type];
        logic_table->i_chan_number = new_logic_table->i_chan_number;
        for(i = 0; i < logic_table->i_chan_number; i++) {
            new_logic_table->map_table[i].logic_slot_id = logic_table->map_table[i].logic_slot_id;
            new_logic_table->map_table[i].phy_slot_id = logic_table->map_table[i].phy_slot_id;
        }
        spin_unlock_irqrestore(&table->lock, flags);

        return ISIL_OK;
    }

    ISIL_DBG(ISIL_DBG_ERR, "null pointer\n");

    return ISIL_ERR;
}

static isil_logic_phy_operation_t isil_logic_phy_operation_ops = {
    .init               = isil_logic_phy_map_table_init,
    .reset              = isil_logic_phy_map_table_reset,
    .release            = isil_logic_phy_map_table_release,

    .find_logic_by_phy  = isil_logic_phy_map_table_find_logic_by_phy,
    .find_phy_by_logic  = isil_logic_phy_map_table_find_phy_by_logic,
    .update_map_table   = isil_logic_phy_map_table_update_map_table,
    .get_map_table      = isil_logic_phy_map_table_get_map_table,
};

static void chip_driver_notify(isil_register_node_t *node, void *priv, isil_notify_msg *msg)
{
    if((node!=NULL) && (priv!=NULL)){

    }
}

static int chip_driver_match_regsiter_node_priv_id(isil_register_node_t *node, void *priv, unsigned long id)
{
    if(node && priv) {
        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_msg_tcb_init(isil_msg_tcb_t *msg_tcb)
{
    if(msg_tcb){
        tcb_node_t *node;

        node = &msg_tcb->node;
        node->op = &tcb_node_op;
        node->op->init(node);
        msg_tcb->msg_type = ISIL_INFO_EXT_MSG;
        msg_tcb->msg_len = 0;

        return ISIL_OK;
    }

    return ISIL_ERR;
} 

static int isil_msg_tcb_reset(isil_msg_tcb_t *msg_tcb)
{
    if(msg_tcb){
        return ISIL_OK;
    }

    return ISIL_ERR;
} 

static int isil_msg_tcb_release(isil_msg_tcb_t *msg_tcb)
{
    if(msg_tcb){
        return ISIL_OK;
    }

    return ISIL_ERR;
} 

static size_t isil_msg_tcb_submit(isil_msg_tcb_t *msg_tcb, char __user *data, size_t count, loff_t *ppos)
{
    loff_t  buf_offset = 0;
    if(msg_tcb && (count >= msg_tcb->msg_len)){
        if(copy_to_user(data, msg_tcb->msg_data, msg_tcb->msg_len) != 0){
            printk("%s, %d: copy_to_user failed\n", __FUNCTION__, __LINE__);
        }
        buf_offset = msg_tcb->msg_len;
    }else{
        ISIL_DBG(ISIL_DBG_ERR, "user buffer is too small to submit message\n");
    }
    *ppos += buf_offset;

    return (size_t)buf_offset;
}

static struct isil_msg_tcb_operation isil_msg_tcb_operation_op = {
    .init       = isil_msg_tcb_init,
    .reset      = isil_msg_tcb_reset,
    .release    = isil_msg_tcb_release,

    .submit     = isil_msg_tcb_submit,
};

static int isil_msg_pool_init(isil_msg_pool_t *msg_pool)
{
    if(msg_pool){
        tcb_node_pool_t *node_pool;
        isil_msg_tcb_t *msg_tcb;
        int i;

        node_pool = &msg_pool->pool;
        node_pool->op = &tcb_node_pool_op;
        node_pool->op->init(node_pool, MAX_MESSAGE_NUMBER);
        msg_pool->msg_entry_number = MAX_MESSAGE_NUMBER;
        msg_pool->cache_order = get_order((msg_pool->msg_entry_number * MAX_MESSAGE_SIZE) + (sizeof(isil_msg_tcb_t) * msg_pool->msg_entry_number));

        msg_pool->msg_tcb = (isil_msg_tcb_t *)__get_free_pages(GFP_KERNEL, msg_pool->cache_order);
        if(msg_pool->msg_tcb){
            for(i = 0; i < msg_pool->msg_entry_number; i++){
                msg_tcb = &msg_pool->msg_tcb[i];
                msg_tcb->msg_id = i;
                msg_tcb->msg_data = ((void *)msg_pool->msg_tcb + (msg_tcb->msg_id * MAX_MESSAGE_SIZE) + (sizeof(isil_msg_tcb_t) * msg_pool->msg_entry_number));
                msg_tcb->op = &isil_msg_tcb_operation_op;
                msg_tcb->op->init(msg_tcb);
                msg_pool->op->put_msg_tcb(msg_pool, msg_tcb);
            }
            return ISIL_OK;
        }

        ISIL_DBG(ISIL_DBG_ERR, "no memory for message pool\n");

        return -ENOMEM;
    }

    return ISIL_ERR;
}

static int isil_msg_pool_reset(isil_msg_pool_t *msg_pool)
{
    if(msg_pool){
        isil_msg_tcb_t *msg_tcb;
        int i;

        if(msg_pool->msg_tcb){
            for(i = 0; i < msg_pool->msg_entry_number; i++){
                msg_tcb = &msg_pool->msg_tcb[i];
                msg_tcb->op->reset(msg_tcb);
            }
        }

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_msg_pool_release(isil_msg_pool_t *msg_pool)
{
    if(msg_pool){
        isil_msg_tcb_t *msg_tcb;
        int i;

        if(msg_pool->msg_tcb){
            for(i = 0; i < msg_pool->msg_entry_number; i++){
                msg_tcb = &msg_pool->msg_tcb[i];
                msg_tcb->op->release(msg_tcb);
            }
            free_pages((unsigned long)msg_pool->msg_tcb, msg_pool->cache_order);
            msg_pool->msg_tcb = NULL;
        }

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_msg_pool_get_msg_tcb(isil_msg_pool_t *msg_pool, isil_msg_tcb_t **msg_tcb)
{
    if(msg_pool && msg_tcb){
        tcb_node_t *node;
        tcb_node_pool_t *pool;

        *msg_tcb = NULL;
        pool = &msg_pool->pool;
        pool->op->get(pool, &node);
        if(node){
            *msg_tcb = to_get_msg_tcb_with_node(node);
        }

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_msg_pool_try_get_msg_tcb(isil_msg_pool_t *msg_pool, isil_msg_tcb_t **msg_tcb)
{
    if(msg_pool && msg_tcb){
        tcb_node_t *node;
        tcb_node_pool_t *pool;

        *msg_tcb = NULL;
        pool = &msg_pool->pool;
        pool->op->try_get(pool, &node);
        if(node){
            *msg_tcb = to_get_msg_tcb_with_node(node);
        }

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_msg_pool_put_msg_tcb(isil_msg_pool_t *msg_pool, isil_msg_tcb_t *msg_tcb)
{
    if(msg_pool && msg_tcb){
        tcb_node_t *node;
        tcb_node_pool_t *pool;

        pool = &msg_pool->pool;
        node = &msg_tcb->node;
        pool->op->put(pool, node);

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_msg_pool_get_msg_tcb_pool_entry_number(isil_msg_pool_t *msg_pool)
{
    if(msg_pool){
        tcb_node_pool_t *pool;

        pool = &msg_pool->pool;
        return pool->op->get_curr_pool_entry_number(pool);
    }

    return ISIL_ERR;
}

static struct isil_msg_pool_operation isil_msg_pool_operation_op = {
    .init                           = isil_msg_pool_init,
    .reset                          = isil_msg_pool_reset,
    .release                        = isil_msg_pool_release,
    .get_msg_tcb                    = isil_msg_pool_get_msg_tcb,
    .try_get_msg_tcb                = isil_msg_pool_try_get_msg_tcb,
    .put_msg_tcb                    = isil_msg_pool_put_msg_tcb,
    .get_msg_tcb_pool_entry_number  = isil_msg_pool_get_msg_tcb_pool_entry_number,
};

static int isil_msg_queue_init(isil_msg_queue_t *msg_queue)
{
    if(msg_queue){
        tcb_node_queue_t *queue;
        
        queue = &msg_queue->queue;
        queue->op = &tcb_node_queue_op;
        queue->op->init(queue);

        spin_lock_init(&msg_queue->lock);
        msg_queue->curr_consumer = NULL;
        msg_queue->curr_producer = NULL;

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_msg_queue_reset(isil_msg_queue_t *msg_queue)
{
    if(msg_queue){
        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_msg_queue_release(isil_msg_queue_t *msg_queue)
{
    if(msg_queue){
        tcb_node_queue_t *queue;
        tcb_node_t *node;
        isil_msg_tcb_t *msg_tcb;
        
        queue = &msg_queue->queue;
        while(queue->op->get_queue_curr_entry_number(queue)){
            queue->op->try_get(queue, &node);
            if(node){
                msg_tcb = to_get_msg_tcb_with_node(node);
                msg_tcb->op->release(msg_tcb);
            }
        }

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_msg_queue_get(isil_msg_queue_t *msg_queue, isil_msg_tcb_t **msg_tcb)
{
    if(msg_queue && msg_tcb){
        tcb_node_queue_t *queue;
        tcb_node_t *node;

        *msg_tcb = NULL;
        queue = &msg_queue->queue;
        queue->op->get(queue, &node);
        if(node){
            *msg_tcb = to_get_msg_tcb_with_node(node);
        }

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_msg_queue_try_get(isil_msg_queue_t *msg_queue, isil_msg_tcb_t **msg_tcb)
{
    if(msg_queue && msg_tcb){
        tcb_node_queue_t *queue;
        tcb_node_t *node;

        *msg_tcb = NULL;
        queue = &msg_queue->queue;
        queue->op->try_get(queue, &node);
        if(node){
            *msg_tcb = to_get_msg_tcb_with_node(node);
        }

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_msg_queue_put(isil_msg_queue_t *msg_queue, isil_msg_tcb_t *msg_tcb)
{
    if(msg_queue && msg_tcb){
        tcb_node_queue_t *queue;
        tcb_node_t *node;

        node = &msg_tcb->node;
        queue = &msg_queue->queue;
        queue->op->put(queue, node);
        
        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_msg_queue_put_header(isil_msg_queue_t *msg_queue, isil_msg_tcb_t *msg_tcb)
{
    if(msg_queue && msg_tcb){
        tcb_node_queue_t *queue;
        tcb_node_t *node;

        node = &msg_tcb->node;
        queue = &msg_queue->queue;
        queue->op->put_header(queue, node);
        
        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_msg_queue_get_curr_producer_from_pool(isil_msg_queue_t *msg_queue, isil_msg_pool_t *msg_pool)
{
    if(msg_queue && msg_pool){
        if(!msg_queue->curr_producer){
            msg_pool->op->get_msg_tcb(msg_pool, &msg_queue->curr_producer);
        }

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_msg_queue_try_get_curr_producer_from_pool(isil_msg_queue_t *msg_queue, isil_msg_pool_t *msg_pool)
{
    if(msg_queue && msg_pool){
        if(!msg_queue->curr_producer){
            msg_pool->op->try_get_msg_tcb(msg_pool, &msg_queue->curr_producer);
        }

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_msg_queue_release_curr_producer(isil_msg_queue_t *msg_queue, isil_msg_pool_t *msg_pool)
{
    if(msg_queue && msg_pool && msg_queue->curr_producer){
        msg_pool->op->put_msg_tcb(msg_pool, msg_queue->curr_producer);
        msg_queue->curr_producer = NULL;

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_msg_queue_put_curr_producer_into_queue(isil_msg_queue_t *msg_queue)
{
    if(msg_queue && msg_queue->curr_producer){
        msg_queue->op->put(msg_queue, msg_queue->curr_producer);
        msg_queue->curr_producer = NULL;

        return ISIL_OK;
    }

    return ISIL_ERR;
}


static int isil_msg_queue_get_curr_consumer_from_queue(isil_msg_queue_t *msg_queue)
{
    if(msg_queue){
        if(!msg_queue->curr_consumer){
            msg_queue->op->get(msg_queue, &msg_queue->curr_consumer);
        }

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_msg_queue_try_get_curr_consumer_from_queue(isil_msg_queue_t *msg_queue)
{
    if(msg_queue){
        if(!msg_queue->curr_consumer){
            msg_queue->op->try_get(msg_queue, &msg_queue->curr_consumer);
        }

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_msg_queue_release_curr_consumer(isil_msg_queue_t *msg_queue, isil_msg_pool_t *msg_pool)
{
    if(msg_queue && msg_pool){
        msg_pool->op->put_msg_tcb(msg_pool, msg_queue->curr_consumer);
        msg_queue->curr_consumer = NULL;

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_msg_queue_get_curr_queue_entry_number(isil_msg_queue_t *msg_queue)
{
    if(msg_queue){
        tcb_node_queue_t *queue;

        queue = &msg_queue->queue;

        return queue->op->get_queue_curr_entry_number(queue);
    }

    return ISIL_ERR;
}

static struct isil_msg_queue_operation isil_msg_queue_operation_op = {
    .init                               = isil_msg_queue_init,
    .reset                              = isil_msg_queue_reset,
    .release                            = isil_msg_queue_release,
    .get                                = isil_msg_queue_get,
    .try_get                            = isil_msg_queue_try_get,
    .put                                = isil_msg_queue_put,
    .put_header                         = isil_msg_queue_put_header,
    .get_curr_producer_from_pool        = isil_msg_queue_get_curr_producer_from_pool,
    .try_get_curr_producer_from_pool    = isil_msg_queue_try_get_curr_producer_from_pool,
    .put_curr_producer_into_queue       = isil_msg_queue_put_curr_producer_into_queue,
    .release_curr_producer              = isil_msg_queue_release_curr_producer,
    .get_curr_consumer_from_queue       = isil_msg_queue_get_curr_consumer_from_queue,
    .try_get_curr_consumer_from_queue   = isil_msg_queue_try_get_curr_consumer_from_queue,
    .release_curr_consumer              = isil_msg_queue_release_curr_consumer,
    .get_curr_queue_entry_number        = isil_msg_queue_get_curr_queue_entry_number,
};

int init_isil_msg_queue(isil_msg_queue_t *msg_queue)
{
    if(msg_queue){
        msg_queue->op = &isil_msg_queue_operation_op;
        return msg_queue->op->init(msg_queue);
    }

    return ISIL_ERR;
}

int remove_isil_msg_queue(isil_msg_queue_t *msg_queue)
{
    if(msg_queue){
        return msg_queue->op->release(msg_queue);
    }

    return ISIL_ERR;
}

static int chip_driver_init(chip_driver_t *driver)
{
    int ret = ISIL_ERR;

    if(driver) {
        isil_chip_t *chip;
        isil_video_bus_t *video_bus;
        isil_audio_bus_t *audio_bus;
        isil_logic_phy_table_t *map_table;
        ed_tcb_t       *ed;
        isil_msg_pool_t *msg_pool;

        chip = driver->chip;

        video_bus = &driver->video_bus;
        audio_bus = &driver->audio_bus;
        ed        = &driver->opened_control_ed;

        atomic_set(&driver->opened_flag, 0);
        init_endpoint_tcb(ed, chip->bus_id, chip->chip_id, 
                ISIL_ED_CONTROL, 0, 0, 
                driver, chip_driver_notify, 
                chip_driver_match_regsiter_node_priv_id, NULL);

        ed->op = &chip_driver_hcd_interface_op;
        ret = init_audio_bus(audio_bus, chip);
        if(ret != ISIL_OK) {
            ISIL_DBG(ISIL_DBG_ERR, "init audio bus error 0x%08x\n", ret);
            return ret;
        }

        ret = init_video_bus(video_bus, chip);
        if(ret != ISIL_OK) {
            ISIL_DBG(ISIL_DBG_ERR, "init video bus error 0x%08x\n", ret);
            return ret;
        }
        map_table = &driver->logic_map_phy;
        map_table->op = &isil_logic_phy_operation_ops;
        map_table->op->init(map_table);

        msg_pool = &driver->msg_pool;
        msg_pool->op = &isil_msg_pool_operation_op;
        ret = msg_pool->op->init(msg_pool);
        if(ret != ISIL_OK){
            ISIL_DBG(ISIL_DBG_ERR, "init message pool failed %d\n", ret);
            return ret;
        }

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static void chip_driver_release(chip_driver_t *driver)
{
    if(driver) {
        isil_video_bus_t *video_bus;
        isil_audio_bus_t *audio_bus;
        isil_logic_phy_table_t *map_table;
        isil_msg_pool_t *msg_pool;

        video_bus = &driver->video_bus;
        audio_bus = &driver->audio_bus;
        map_table = &driver->logic_map_phy;
        msg_pool  = &driver->msg_pool;

        if(video_bus->op) {
            video_bus->op->release(video_bus);
        }
        if(audio_bus->op) {
            audio_bus->op->release(audio_bus);
        }
        if(map_table->op) {
            map_table->op->release(map_table);
        }
        if(msg_pool->op){
            msg_pool->op->release(msg_pool);
        }
    }
}

static int chip_driver_reset(chip_driver_t *driver)
{
    int ret = ISIL_ERR;
    if(driver) {
        isil_video_bus_t *video_bus;
        isil_audio_bus_t *audio_bus;
        isil_logic_phy_table_t *map_table;

        video_bus = &driver->video_bus;
        audio_bus = &driver->audio_bus;
        map_table = &driver->logic_map_phy;

        if(video_bus->op) {
            ret = video_bus->op->reset(video_bus);
            if(ret) {
                ISIL_DBG(ISIL_DBG_ERR, "reset video bus failed\n");
                return ret;
            }
        }

        if(audio_bus->op) {
            ret = audio_bus->op->reset(audio_bus);
            if(ret) {
                ISIL_DBG(ISIL_DBG_ERR, "reset audio bus failed\n");
                return ret;
            }
        }
        if(map_table->op) {
            map_table->op->reset(map_table);
        }
    }

    return ret;
}

static struct chip_driver_operation chip_driver_op = {
    .init = chip_driver_init,
    .release = chip_driver_release,
    .reset   = chip_driver_reset,
};

int init_chip_driver(struct isil_chip_device * tcd, void * drv)
{
    int ret = 0;
    isil_chip_t *chip = tcd_priv(tcd);
    chip_driver_t *driver;

    //tc_trace;
    driver = (chip_driver_t *)kmalloc(sizeof(chip_driver_t),GFP_KERNEL);
    if(driver) {
        if(chip) {
            driver->chip = chip;
            driver->op = &chip_driver_op;
            chip->chip_driver = driver;

            ret = driver->op->init(driver);
            if(ret){
                printk("error while init!\n");
                return -ENOMEM;
            }
            tcd->ped = &driver->opened_control_ed;
            return ISIL_OK; 
        }else{
            ISIL_DBG(ISIL_DBG_ERR, "null pointer\n");
            return -EINVAL;
        }
    }else{
        ISIL_DBG(ISIL_DBG_ERR, "no memory\n");
        return -ENOMEM;
    }

    return ISIL_ERR;
}

void remove_chip_driver(struct isil_chip_device * tcd, void * drv)
{
    chip_driver_t *driver;

    if(tcd && tcd->ped) {
        isil_chip_t *chip;

        driver = to_get_chip_driver_with_opened_logic_chan_ed(tcd->ped);
        chip = driver->chip;
        driver->op->release(driver);
        kfree(driver);
        if(chip){
            chip->chip_driver = NULL;
        }
        tcd->ped = NULL;
    }
}


/*
   int isil_5864_chip_driver_init(struct isil_chip_device * tcd, struct isil_chip_driver * drv)
   {
   tc_trace;
   return 0;	
   }

   void isil_5864_chip_driver_remove(struct isil_chip_device * tcd, struct isil_chip_driver * drv)
   {
   tc_trace;
   return ;	
   }
   */

int chip5864_attach(void *drv,struct isil_chip_device *ptcd)
{
    struct isil_chip_driver * driver = (struct isil_chip_driver *)drv;
    struct isil_dev_table_id *driv_id = &driver->device_id;
    struct tcd_driver_operations *to = (struct tcd_driver_operations *)driv_id->ops;

    //printk("--->function %s called\n",__FUNCTION__);
    return to->init(ptcd,drv);
}

int chip5864_detach(void *drv,struct isil_chip_device *ptcd)
{
    struct isil_chip_driver * driver = (struct isil_chip_driver *)drv;
    struct isil_dev_table_id *driv_id = &driver->device_id;
    struct tcd_driver_operations *top = (struct tcd_driver_operations *)driv_id->ops;

    //printk("--->function %s called\n",__FUNCTION__);
    if(!top->remove){
        printk("chip driv has no rmove\n");
        return -EINVAL;
    }
    top->remove(ptcd,drv);
    return 0;
}

int chip5864_command(struct isil_chip_device *ptcd,unsigned int cmd, void *cmd_arg)
{
    int ret = 0;


    return ret;
}





struct isil_dev_id  isil_chip_dev_id =
{
    .epobj = {
        .vendor_id = ISIL,
        .bus_id =  1,
        .chip_id = TW5864,
    },
    .version = 1,
};


struct tcd_driver_operations  isil_5864_chip_ops =
{
    .init = init_chip_driver,
    .remove = remove_chip_driver,
};

struct isil_chip_driver   isil_5864_chip_driv =
{
    .driver = {
        .name = "tw5864_chip_driv",
    },
    .device_id = {
        .tid = &isil_chip_dev_id,
        .ops = &isil_5864_chip_ops,
    },
    .attach_chip  = chip5864_attach,
    .detach_chip  = chip5864_detach,
    .command      = chip5864_command,
};



int isil_chip_driver_init(void)
{
    epkey_t key = {0};
    struct isil_dev_table_id *table_id = &isil_5864_chip_driv.device_id;
    struct isil_dev_id  *id = kzalloc(sizeof(struct isil_dev_id),GFP_KERNEL);     
    if(!id)
        return -1;

    make_key((epkey_t *)&key,0,0,0,0,0,0);
    epobj_init(&id->epobj, ISIL, 1,TW5864,0,0,&key); 
    table_id->tid = id;
    return register_chip_driver(&isil_5864_chip_driv);
}

void isil_chip_driver_remove(void)
{
    unregister_chip_driver(&isil_5864_chip_driv);
}

