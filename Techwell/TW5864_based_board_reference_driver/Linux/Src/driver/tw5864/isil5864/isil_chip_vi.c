#include	<isil5864/isil_common.h>

#define ISIL_VI_EVENT_IRQ 0 /*0:timer, 1:irq*/

static const struct isil_ddr_dll_param{
    u32 clk;
    struct ddr_dqs{
        u32 dqs0_delay;
        u32 dqs1_delay;
        u32 clk90;
    } dqs[2];
} ddr_clk[] = {
#ifdef TW5864_ASIC_NEW
    //{185, {{0x1, 0x1, 0x0}, {0x1, 0x1, 0x1}}, },
    {185, {{0x2, 0x2, 0x2}, {0x2, 0x2, 0x2}}, },
#else
    {185, {{0x0, 0x0, 0xf}, {0x0, 0x0, 0x1}}, },
#endif
    {175, {{0x2, 0x2, 0x1}, {0x2, 0x2, 0x1}}, },
    {172, {{0x1, 0x1, 0x0}, {0x1, 0x1, 0x1}}, },
    {166, {{0x0, 0x0, 0x1}, {0x1, 0x1, 0x0}}, },
    {150, {{0xf, 0xf, 0x0}, {0x0, 0x0, 0x1}}, },
    {120, {{0x3, 0x3, 0xf}, {0x3, 0x3, 0xf}}, },
    {110, {{0x4, 0x4, 0x2}, {0x4, 0x4, 0x2}}, },
    {108, {{0x4, 0x4, 0x2}, {0x4, 0x4, 0x2}}, },
    {},
};

static isil_chip_vi_feature_t default_vi_feature = {
    .e_motion_mode = MOTION_TRIGGER_AUTIO,
    .i_motion_sensitive  = CHIP_VI_EVENT_MIN_SENSITIVE,
    .i_night_sensitive   = CHIP_VI_EVENT_MIN_SENSITIVE,
    .i_blind_sensitive  = CHIP_VI_EVENT_MIN_SENSITIVE,

    .b_novideo_enable = ISIL_VI_FEATURE_ON,
    .b_underflow_enable = ISIL_VI_FEATURE_OFF,
    .b_overflow_enable  = ISIL_VI_FEATURE_OFF,
    .change_mask      = ISIL_VI_FEATURE_ENABLE_CHANGE_MASK_ALL,
};

static isil_chip_vi_feature_ext_t default_vi_feature_ext = {
    .b_video_connect = ISIL_VI_FEATURE_OFF,
    .b_video_disconnect = ISIL_VI_FEATURE_OFF,
    .b_video_night_add = ISIL_VI_FEATURE_OFF,
    .b_video_night_del = ISIL_VI_FEATURE_OFF,
    .b_video_blind_add = ISIL_VI_FEATURE_OFF,
    .b_video_blind_del = ISIL_VI_FEATURE_OFF,
    .b_video_standard_change = ISIL_VI_FEATURE_OFF,
    .b_video_overflow = ISIL_VI_FEATURE_OFF,
    .b_video_underflow = ISIL_VI_FEATURE_OFF,
};

static isil_chip_vi_property_t default_video_property = {
    .u32Brightness = DEFAULT_VI_BRIGHTNESS,
    .u32Contrast   = DEFAULT_VI_CONTRAST,
    .u32Hue        = DEFAULT_VI_HUE,
    .u32Saturation = DEFAULT_VI_SATURATION,
    .u32Sharpness  = DEFAULT_VI_SHARPNESS,
    .u32Mask       = 0xff,
};

static int isil_chip_event_mask(isil_chip_vi_driver_t *, enum CHIP_VI_EVENT, u32 , int);

static int chip_vi_driver_hcd_interface_open(ed_tcb_t *opened_ed)
{
    int ret = ISIL_ERR;

    if(opened_ed) {
        isil_chip_vi_driver_t *driver;

        driver = to_get_chip_vi_driver_with_opened_logic_chan_ed(opened_ed);
        switch(atomic_read(&opened_ed->state)){
            default:
            case ISIL_ED_UNREGISTER:
                if(opened_ed->ed.op) {
                    opened_ed->ed.op->init_complete(&opened_ed->ed);
                }else{
                    ISIL_DBG(ISIL_DBG_FATAL, "driver not init\n");
                    return -EPERM;
                }
                atomic_set(&driver->opened_flag, 1);
                ret = 0;
                break;
            case ISIL_ED_IDLE:
            case ISIL_ED_STANDBY:
            case ISIL_ED_SUSPEND:
            case ISIL_ED_RUNNING:
                ret = -EBUSY;
                ISIL_DBG(ISIL_DBG_ERR, "chip vi driver have been opened\n");
                break;
        }
    }

    return ret;
}

static int chip_vi_driver_hcd_interface_close(ed_tcb_t *opened_ed)
{
    int ret = ISIL_ERR;

    if(opened_ed) {
        isil_chip_vi_driver_t *driver;

        driver = to_get_chip_vi_driver_with_opened_logic_chan_ed(opened_ed);
        switch(atomic_read(&opened_ed->state)){
            default:
            case ISIL_ED_UNREGISTER:
                ret = 0;
                ISIL_DBG(ISIL_DBG_INFO, "chip control have been close\n");
                break;
            case ISIL_ED_IDLE:
            case ISIL_ED_STANDBY:
            case ISIL_ED_SUSPEND:
            case ISIL_ED_RUNNING:
                if(opened_ed->ed.op != NULL){
                    atomic_set(&opened_ed->state, ISIL_ED_UNREGISTER);
                    opened_ed->ed.op->complete_done(&opened_ed->ed);
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

static void chip_vi_driver_hcd_interface_suspend(ed_tcb_t *opened_ed)
{
    if(opened_ed) {
        isil_chip_vi_driver_t *driver;

        driver = to_get_chip_vi_driver_with_opened_logic_chan_ed(opened_ed);
        if(atomic_read(&driver->opened_flag)) {
        }else{
            ISIL_DBG(ISIL_DBG_ERR, "device have not opened\n");
            return;
        }
    }

    return ;
}

static void chip_vi_driver_hcd_interface_resume(ed_tcb_t *opened_ed)
{
    if(opened_ed) {
        isil_chip_vi_driver_t *driver;

        driver = to_get_chip_vi_driver_with_opened_logic_chan_ed(opened_ed);
        if(atomic_read(&driver->opened_flag)) {
        }else{
            ISIL_DBG(ISIL_DBG_ERR, "device have not opened\n");
            return;
        }
    }

    return ;
}

static int chip_vi_driver_hcd_interface_ioctl(ed_tcb_t *opened_ed, unsigned int cmd, unsigned long arg)
{
    int ret = ISIL_ERR;

    if(opened_ed) {
        isil_chip_vi_driver_t *driver;
        isil_chip_vi_config_property_t    *video_property;
        isil_chip_vi_property_t property, *p = &property;
        isil_chip_vi_feature_t  feature, *rfeature, *pfeature = &feature;

        driver = to_get_chip_vi_driver_with_opened_logic_chan_ed(opened_ed);

        if(!atomic_read(&driver->opened_flag)) {
            ISIL_DBG(ISIL_DBG_ERR, "device have not opened\n");
            return -EPERM;
        }

        ISIL_DBG(ISIL_DBG_INFO, "%c, %d\n", _IOC_TYPE(cmd), _IOC_NR(cmd));
        switch(cmd) {
            case ISIL_VI_SET_VIDEO_AD_PARM:
                if(copy_from_user(p, (void *)arg, sizeof(isil_chip_vi_property_t)) != 0){
                    printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                }
                if(p->u32Channel >= ISIL_VI_ON_CHIP_CHNL) {
                    ISIL_DBG(ISIL_DBG_ERR, "vi channel overflow, only support ISIL_VI_MAX_CHNL = %d\n", ISIL_VI_ON_CHIP_CHNL);
                    ret = -EINVAL;
                    break;
                }
                video_property = &driver->video_property[p->u32Channel];
                if(p->u32Mask & MASK_VI_BRIGHTNESS) {
                    ret = video_property->op->set_brightness(&video_property->video_property, p->u32Brightness);
                    if(ret) {
                        break;
                    }
                }
                if(p->u32Mask & MASK_VI_CONTRAST) {
                    ret = video_property->op->set_contrast(&video_property->video_property, p->u32Contrast);
                    if(ret) {
                        break;
                    }
                }
                if(p->u32Mask & MASK_VI_SATURATION) {
                    ret = video_property->op->set_saturation(&video_property->video_property, p->u32Saturation);
                    if(ret) {
                        break;
                    }
                }
                if(p->u32Mask & MASK_VI_HUE) {
                    ret = video_property->op->set_hue(&video_property->video_property, p->u32Hue);
                    if(ret) {
                        break;
                    }
                }
                if(p->u32Mask & MASK_VI_SHARPNESS) {
                    ret = video_property->op->set_sharpness(&video_property->video_property, p->u32Sharpness);
                    if(ret) {
                        break;
                    }
                }

                video_property->video_property.u32Mask = p->u32Mask;
                if(p->u32Mask) {
                    ret = video_property->op->update(driver, video_property);
                }else{
                    ret = ISIL_OK;
                }
                break;
            case ISIL_VI_GET_VIDEO_AD_PARM:
                if(copy_from_user(p, (void *)arg, sizeof(isil_chip_vi_property_t)) != 0){
                    printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                }
                if(p->u32Channel >= ISIL_VI_ON_CHIP_CHNL) {
                    ISIL_DBG(ISIL_DBG_ERR, "vi channel overflow, only support ISIL_VI_MAX_CHNL = %d\n", ISIL_VI_ON_CHIP_CHNL);
                    ret = -EINVAL;
                    break;
                }
                video_property = &driver->video_property[p->u32Channel];
                if(p->u32Mask & MASK_VI_BRIGHTNESS) {
                    p->u32Brightness = video_property->op->get_brightness(&video_property->video_property);
                }
                if(p->u32Mask & MASK_VI_CONTRAST) {
                    p->u32Contrast = video_property->op->get_contrast(&video_property->video_property);
                }
                if(p->u32Mask & MASK_VI_SATURATION) {
                    p->u32Saturation = video_property->op->get_saturation(&video_property->video_property);
                }
                if(p->u32Mask & MASK_VI_HUE) {
                    p->u32Hue = video_property->op->get_hue(&video_property->video_property);
                }
                if(p->u32Mask & MASK_VI_SHARPNESS) {
                    p->u32Sharpness = video_property->op->get_sharpness(&video_property->video_property);
                }
                if(p->u32Mask) {
                    if(copy_to_user((void *)arg, p, sizeof(isil_chip_vi_property_t)) != 0){
                        printk("%s, %d: copy_to_user failed\n", __FUNCTION__, __LINE__);
                    }
                }
                ret = ISIL_OK;
                break;
            case ISIL_VI_GET_FEATURE:
                if(copy_from_user(pfeature, (void *)arg, sizeof(isil_chip_vi_feature_t)) != 0){
                    printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                }
                if(pfeature->channel < ISIL_VI_MAX_CHNL) {
                    video_property = &driver->video_property[p->u32Channel];
                    rfeature = &video_property->video_feature;
                    if(pfeature->change_mask & ISIL_VI_FEATURE_ENABLE_CHANGE_MODE_MASK) {
                        pfeature->e_motion_mode = rfeature->e_motion_mode;
                    }
                    if(pfeature->change_mask & ISIL_VI_FEATURE_ENABLE_CHANGE_MOTION_MASK) {
                        pfeature->i_motion_sensitive = rfeature->i_motion_sensitive;
                    }
                    if(pfeature->change_mask & ISIL_VI_FEATURE_ENABLE_CHANGE_NIGHT_MASK) {
                        pfeature->i_night_sensitive = rfeature->i_night_sensitive;
                    }
                    if(pfeature->change_mask & ISIL_VI_FEATURE_ENABLE_CHANGE_BLIND_MASK) {
                        pfeature->i_blind_sensitive = rfeature->i_blind_sensitive;
                    }
                    if(pfeature->change_mask & ISIL_VI_FEATURE_ENABLE_CHANGE_NOVIDEO_MASK) {
                        pfeature->b_novideo_enable = !!rfeature->b_novideo_enable;
                    }
                    if(pfeature->change_mask & ISIL_VI_FEATURE_ENABLE_CHANGE_UNDERFLOW_MASK) {
                        pfeature->b_underflow_enable = !!rfeature->b_underflow_enable;
                    }
                    if(pfeature->change_mask & ISIL_VI_FEATURE_ENABLE_CHANGE_OVERFLOW_MASK) {
                        pfeature->b_overflow_enable = !!rfeature->b_overflow_enable;
                    }
                }else{
                    ISIL_DBG(ISIL_DBG_ERR, "channel number error\n");
                    ret = -EINVAL;
                }
                break;
            case ISIL_VI_SET_FEATURE:
                if(copy_from_user(pfeature, (void *)arg, sizeof(isil_chip_vi_feature_t)) != 0){
                    printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                }
                if(pfeature->channel < ISIL_VI_MAX_CHNL) {
                    isil_chip_vi_config_property_t property;

                    memset(&property, 0, sizeof(isil_chip_vi_config_property_t));
                    video_property = &driver->video_property[p->u32Channel];
                    rfeature = &property.video_feature;
                    if(pfeature->change_mask & ISIL_VI_FEATURE_ENABLE_CHANGE_MODE_MASK) {
                        if((pfeature->e_motion_mode >= MOTION_TRIGGER_AUTIO) || (pfeature->e_motion_mode < MOTION_TRIGGER_RESERVED)) {
                            rfeature->e_motion_mode = pfeature->e_motion_mode;
                        }else{
                            ret = -EINVAL;
                            ISIL_DBG(ISIL_DBG_ERR, "bad motion mode %d\n", pfeature->e_motion_mode);
                            break;
                        }
                    }
                    if(pfeature->change_mask & ISIL_VI_FEATURE_ENABLE_CHANGE_MOTION_MASK) {
                        if((pfeature->i_motion_sensitive >= CHIP_VI_EVENT_MIN_SENSITIVE) && ((pfeature->i_motion_sensitive <= CHIP_VI_EVENT_MAX_SENSITIVE))) {
                            rfeature->i_motion_sensitive = pfeature->i_motion_sensitive;
                        }else{
                            ret = -EINVAL;
                            ISIL_DBG(ISIL_DBG_ERR, "bad motion sensitive %d\n", pfeature->i_motion_sensitive);
                            break;
                        }
                    }
                    if(pfeature->change_mask & ISIL_VI_FEATURE_ENABLE_CHANGE_NIGHT_MASK) {
                        if((pfeature->i_night_sensitive >= CHIP_VI_EVENT_MIN_SENSITIVE) && ((pfeature->i_night_sensitive <= CHIP_VI_EVENT_MAX_SENSITIVE))) {
                            rfeature->i_night_sensitive = pfeature->i_night_sensitive;
                        }else{
                            ret = -EINVAL;
                            ISIL_DBG(ISIL_DBG_ERR, "bad night sensitive %d\n", pfeature->i_night_sensitive);
                            break;
                        }
                    }
                    if(pfeature->change_mask & ISIL_VI_FEATURE_ENABLE_CHANGE_BLIND_MASK) {
                        if((pfeature->i_blind_sensitive >= CHIP_VI_EVENT_MIN_SENSITIVE) && ((pfeature->i_blind_sensitive <= CHIP_VI_EVENT_MAX_SENSITIVE))) {
                            rfeature->i_blind_sensitive = pfeature->i_blind_sensitive;
                        }else{
                            ret = -EINVAL;
                            ISIL_DBG(ISIL_DBG_ERR, "bad blind sensitive %d\n", pfeature->i_blind_sensitive);
                            break;
                        }
                    }
                    if(pfeature->change_mask & ISIL_VI_FEATURE_ENABLE_CHANGE_NOVIDEO_MASK) {
                        rfeature->b_novideo_enable = !!pfeature->b_novideo_enable;
                    }
                    if(pfeature->change_mask & ISIL_VI_FEATURE_ENABLE_CHANGE_UNDERFLOW_MASK) {
                        rfeature->b_underflow_enable = !!pfeature->b_underflow_enable;
                    }
                    if(pfeature->change_mask & ISIL_VI_FEATURE_ENABLE_CHANGE_OVERFLOW_MASK) {
                        rfeature->b_overflow_enable = !!pfeature->b_overflow_enable;
                    }
                    rfeature->change_mask = pfeature->change_mask;
                    if(rfeature->change_mask) {
                        ret = video_property->op->update(driver, &property);
                    }else{
                        ret = ISIL_OK;
                    }
                }else{
                    ISIL_DBG(ISIL_DBG_ERR, "channel number error\n");
                    ret = -EINVAL;
                }
                break;
            case ISIL_VI_GET_VIDEO_STANDARD:
                ret = ISIL_OK;
                break;
            case ISIL_VI_SET_VIDEO_TYPE:
                {
                    VI_CH_VIDEOTYPE video_type;
                    if(copy_from_user(&video_type, (void *)arg, sizeof(VI_CH_VIDEOTYPE)) != 0){
                        printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                    }
                    ret = driver->op->set_video_standard_ext(driver, video_type.eVideoType);
                    break;
                }
            case ISIL_VI_VIDEO_LOST_CMD:
                {
                    isil_chip_vi_feature_ext_t *vi_feature_ext;
                    isil_transparent_msg_header_t transparent_msg_header;
                    u32 consumer_len, current_msg_cnt;
                    u32 msg_type;

                    consumer_len = 0;
                    if(copy_from_user(&transparent_msg_header, (void *)(arg + consumer_len), sizeof(isil_transparent_msg_header_t))){
                        printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                        ret = -EFAULT;
                        break;
                    }
                    /*check arg*/
                    if(transparent_msg_header.isil_transparent_msg_number > 512){
                        ISIL_DBG(ISIL_DBG_ERR, "too many messages (%d)", transparent_msg_header.isil_transparent_msg_number);
                        ret = -EINVAL;
                        break;
                    }
                    current_msg_cnt =  0;
                    consumer_len += sizeof(isil_transparent_msg_header_t);
                    ret = ISIL_OK;
                    while((current_msg_cnt < transparent_msg_header.isil_transparent_msg_number) 
                            && (consumer_len < transparent_msg_header.isil_transparent_msg_total_len)){
                        isil_video_lost_detection_msg_t video_lost_msg;
                        isil_video_night_detection_msg_t video_night_msg;
                        isil_video_blind_detection_msg_t video_blind_msg;
                        isil_video_standard_change_detection_msg_t video_standard_change_msg;

                        if(copy_from_user(&msg_type, (void *)(arg + consumer_len), sizeof(int))){
                            printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                            ret = -EFAULT;
                            break;
                        }
                        switch(msg_type){
                            case ISIL_VIDEO_LOST_DETECTION_MSG:
                                if(copy_from_user(&video_lost_msg, (void *)(arg + consumer_len), sizeof(isil_video_lost_detection_msg_t))){
                                    printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                                    ret = -EFAULT;
                                    break;
                                }
                                if(video_lost_msg.chan_id > ISIL_VI_MAX_CHNL){
                                    ISIL_DBG(ISIL_DBG_ERR, "channel number %d error\n", video_lost_msg.chan_id);
                                    ret = -EINVAL;
                                    break;
                                }
                                consumer_len += sizeof(isil_video_lost_detection_msg_t);
                                break;
                            case ISIL_VIDEO_NIGHT_DETECTION_MSG:
                                if(copy_from_user(&video_night_msg, (void *)(arg + consumer_len), sizeof(isil_video_night_detection_msg_t))){
                                    printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                                    ret = -EFAULT;
                                    break;
                                }
                                if(video_night_msg.chan_id > ISIL_VI_MAX_CHNL){
                                    ISIL_DBG(ISIL_DBG_ERR, "channel number %d error\n", video_night_msg.chan_id);
                                    ret = -EINVAL;
                                    break;
                                }
                                consumer_len += sizeof(isil_video_night_detection_msg_t);
                                break;
                            case ISIL_VIDEO_BLIND_DETECTION_MSG:
                                if(copy_from_user(&video_blind_msg, (void *)(arg + consumer_len), sizeof(isil_video_blind_detection_msg_t))){
                                    printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                                    ret = -EFAULT;
                                    break;
                                }
                                if(video_blind_msg.chan_id > ISIL_VI_MAX_CHNL){
                                    ISIL_DBG(ISIL_DBG_ERR, "channel number %d error\n", video_blind_msg.chan_id);
                                    ret = -EINVAL;
                                    break;
                                }
                                consumer_len += sizeof(isil_video_blind_detection_msg_t);
                                break;
                            case ISIL_VIDEO_STANDARD_CHANGE_DETECTION_MSG:
                                if(copy_from_user(&video_standard_change_msg, (void *)(arg + consumer_len), sizeof(isil_video_standard_change_detection_msg_t))){
                                    printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                                    ret = -EFAULT;
                                    break;
                                }
                                if(video_standard_change_msg.chan_id > ISIL_VI_ON_CHIP_CHNL){
                                    ISIL_DBG(ISIL_DBG_ERR, "channel number %d error, only support analog interface\n", video_standard_change_msg.chan_id);
                                    ret = -EINVAL;
                                    break;
                                }
                                consumer_len += sizeof(isil_video_standard_change_detection_msg_t);
                                break;
                            default:
                                ISIL_DBG(ISIL_DBG_ERR, "unknown MSG_TYPE 0x%08x\n", msg_type);
                                ret = -EINVAL;
                                break;
                        }
                        current_msg_cnt++;
                        if(ret){
                            break;
                        }
                    }
                    if(ret){
                        break;
                    }
                    if((current_msg_cnt < transparent_msg_header.isil_transparent_msg_number) 
                            || (consumer_len < transparent_msg_header.isil_transparent_msg_total_len)){
                        ISIL_DBG(ISIL_DBG_ERR, "message payload data error(total %d messages, current_msg_cnt %d, total messages length %d, current_consumer_len %d)\n", transparent_msg_header.isil_transparent_msg_number, current_msg_cnt, transparent_msg_header.isil_transparent_msg_total_len, consumer_len);
                        ret = -EINVAL;
                    }
                    current_msg_cnt =  0;
                    consumer_len = sizeof(isil_transparent_msg_header_t);
                    while((current_msg_cnt < transparent_msg_header.isil_transparent_msg_number) 
                            && (consumer_len < transparent_msg_header.isil_transparent_msg_total_len)){
                        isil_video_lost_detection_msg_t video_lost_msg;
                        isil_video_night_detection_msg_t video_night_msg;
                        isil_video_blind_detection_msg_t video_blind_msg;
                        isil_video_standard_change_detection_msg_t video_standard_change_msg;

                        if(copy_from_user(&msg_type, (void *)(arg + consumer_len), sizeof(int)) != 0){
                            printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);                        
                        }
                        switch(msg_type){
                            case ISIL_VIDEO_LOST_DETECTION_MSG:
                                if(copy_from_user(&video_lost_msg, (void *)(arg + consumer_len), sizeof(isil_video_lost_detection_msg_t)) != 0){
                                    printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);                                
                                }
                                vi_feature_ext = &driver->video_property[video_lost_msg.chan_id].video_feature_ext;
                                vi_feature_ext->b_video_connect = !!video_lost_msg.video_connect_valid;
                                vi_feature_ext->b_video_disconnect = !!video_lost_msg.video_lost_valid;
                                consumer_len += sizeof(isil_video_lost_detection_msg_t);
                                break;
                            case ISIL_VIDEO_NIGHT_DETECTION_MSG:
                                if(copy_from_user(&video_night_msg, (void *)(arg + consumer_len), sizeof(isil_video_night_detection_msg_t)) != 0){
                                    printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);                                
                                }
                                vi_feature_ext = &driver->video_property[video_night_msg.chan_id].video_feature_ext;
                                vi_feature_ext->b_video_night_add = !!video_night_msg.video_night_valid_from_day_to_night;
                                vi_feature_ext->b_video_night_del = !!video_night_msg.video_night_valid_from_night_to_day;
                                consumer_len += sizeof(isil_video_night_detection_msg_t);
                                break;
                            case ISIL_VIDEO_BLIND_DETECTION_MSG:
                                if(copy_from_user(&video_blind_msg, (void *)(arg + consumer_len), sizeof(isil_video_blind_detection_msg_t)) != 0){
                                    printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                                }
                                vi_feature_ext = &driver->video_property[video_blind_msg.chan_id].video_feature_ext;
                                vi_feature_ext->b_video_blind_add = !!video_blind_msg.video_blind_add_valid;
                                vi_feature_ext->b_video_blind_del = !!video_blind_msg.video_blind_remove_valid;
                                consumer_len += sizeof(isil_video_blind_detection_msg_t);
                                break;
                            case ISIL_VIDEO_STANDARD_CHANGE_DETECTION_MSG:
                                if(copy_from_user(&video_standard_change_msg, (void *)(arg + consumer_len), sizeof(isil_video_standard_change_detection_msg_t)) != 0){
                                    printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                                }
                                vi_feature_ext = &driver->video_property[video_standard_change_msg.chan_id].video_feature_ext;
                                vi_feature_ext->b_video_standard_change = !!video_standard_change_msg.curr_video_standard;
                                consumer_len += sizeof(isil_video_standard_change_detection_msg_t);
                                break;
                            default:
                                ret = -EINVAL;
                                break;
                        }
                        current_msg_cnt++;
                        if(ret){
                            break;
                        }
                    }
                    
                    break;
                }
            default:
                ret = -EBADRQC;
                ISIL_DBG(ISIL_DBG_ERR, "No such command!\n");
                break;
        }
    }else{
        ISIL_DBG(ISIL_DBG_FATAL, "vi device not init\n");
    }

    return ret;
}

static int chip_vi_driver_hcd_interface_get_state(ed_tcb_t *opened_ed)
{
    int	ret = ISIL_ED_UNREGISTER;

    if(opened_ed != NULL){
        ret = atomic_read(&opened_ed->state);
    }

    return ret;
}

static void chip_vi_driver_hcd_interface_suspend_done(ed_tcb_t *opened_ed)
{
    if(opened_ed != NULL){
        isil_register_node_t    *ed;
        ed = &opened_ed->ed;
        ed->op->suspend_complete_done(ed);
    }
}

static void chip_vi_driver_hcd_interface_resume_done(ed_tcb_t *opened_ed)
{
    if(opened_ed != NULL){
        isil_register_node_t    *ed;
        ed = &opened_ed->ed;
        ed->op->resume_complete_done(ed);
    }
}

static void chip_vi_driver_hcd_interface_close_done(ed_tcb_t *opened_ed)
{
    if(opened_ed != NULL){
        isil_register_node_t    *ed;
        ed = &opened_ed->ed;
        ed->op->all_complete_done(ed);
    }
}

static isil_hcd_interface_operation chip_vi_driver_hcd_interface_op = {
    .open = chip_vi_driver_hcd_interface_open,
    .close = chip_vi_driver_hcd_interface_close,
    .suspend = chip_vi_driver_hcd_interface_suspend,
    .resume = chip_vi_driver_hcd_interface_resume,
    .ioctl = chip_vi_driver_hcd_interface_ioctl,
    .get_state = chip_vi_driver_hcd_interface_get_state,
    .suspend_done = chip_vi_driver_hcd_interface_suspend_done,
    .resume_done = chip_vi_driver_hcd_interface_resume_done,
    .close_done = chip_vi_driver_hcd_interface_close_done,
};

static int isil_chip_vi_property_init(isil_chip_vi_property_t *property, u32 chn)
{
    if(property) {
        property->chip_num      = 0;/*not used*/
        property->u32Channel    = chn;
        property->u32Brightness = DEFAULT_VI_BRIGHTNESS;
        property->u32Contrast   = DEFAULT_VI_CONTRAST;
        property->u32Saturation = DEFAULT_VI_SATURATION;
        property->u32Hue        = DEFAULT_VI_HUE;
        property->u32Mask       = 0;
        property->u32Sharpness  = DEFAULT_VI_SHARPNESS;

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int isil_chip_vi_property_reset(isil_chip_vi_property_t *property)
{
    if(property) {
        property->u32Brightness = DEFAULT_VI_BRIGHTNESS;
        property->u32Contrast   = DEFAULT_VI_CONTRAST;
        property->u32Saturation = DEFAULT_VI_SATURATION;
        property->u32Hue        = DEFAULT_VI_HUE;
        property->u32Mask       = 0;
        property->u32Sharpness  = DEFAULT_VI_SHARPNESS;

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static u32 isil_chip_vi_property_get_brightness(isil_chip_vi_property_t *property)
{
    if(property) {
        return property->u32Brightness;
    }

    return ISIL_ERR;
}

static int isil_chip_vi_property_set_brightness(isil_chip_vi_property_t *property, u32 brightness)
{
    if(property) {
        if((brightness >= MIN_VI_BRIGHNESS) && (brightness <= MAX_VI_BRIGHNESS)) {
            property->u32Brightness = brightness;
            return ISIL_OK;
        }
        ISIL_DBG(ISIL_DBG_INFO, "set brightness failed %d, [%d, %d]\n", brightness, MIN_VI_BRIGHNESS, MAX_VI_BRIGHNESS);
        return -EINVAL;
    }

    return ISIL_ERR;
}

static u32 isil_chip_vi_property_get_contrast(isil_chip_vi_property_t *property)
{
    if(property) {
        return property->u32Contrast;
    }

    return ISIL_ERR;
}

static int isil_chip_vi_property_set_contrast(isil_chip_vi_property_t *property, u32 contrast)
{
    if(property) {
        if((contrast >= MIN_VI_CONTRAST) && (contrast <= MAX_VI_CONTRAST)) {
            property->u32Contrast = contrast;
            return ISIL_OK;
        }
        ISIL_DBG(ISIL_DBG_INFO, "set contrast failed %d, [%d, %d]\n", contrast, MIN_VI_CONTRAST, MAX_VI_CONTRAST);
        return -EINVAL;
    }

    return ISIL_ERR;
}

static u32 isil_chip_vi_property_get_saturation(isil_chip_vi_property_t *property)
{
    if(property) {
        return property->u32Saturation;
    }

    return ISIL_ERR;
}

static int isil_chip_vi_property_set_saturation(isil_chip_vi_property_t *property, u32 saturation)
{
    if(property) {
        if((saturation >= MIN_VI_SATURATION) && (saturation <= MAX_VI_SATURATION)) {
            property->u32Saturation = saturation;
            return ISIL_OK;
        }
        ISIL_DBG(ISIL_DBG_INFO, "set saturation failed %d, [%d, %d]\n", saturation, MIN_VI_SATURATION, MAX_VI_SATURATION);
        return -EINVAL;
    }

    return ISIL_ERR;
}

static u32 isil_chip_vi_property_get_hue(isil_chip_vi_property_t *property)
{
    if(property) {
        return property->u32Hue;
    }

    return ISIL_ERR;
}

static int isil_chip_vi_property_set_hue(isil_chip_vi_property_t *property, u32 hue)
{
    if(property) {
        if((hue >= MIN_VI_HUE) && (hue <= MAX_VI_HUE)) {
            property->u32Hue = hue;
            return ISIL_OK;
        }
        ISIL_DBG(ISIL_DBG_INFO, "set hue failed %d, [%d, %d]\n", hue, MIN_VI_HUE, MAX_VI_HUE);
        return -EINVAL;
    }

    return ISIL_ERR;
}

static u32 isil_chip_vi_property_get_sharpness(isil_chip_vi_property_t *property)
{
    if(property) {
        return property->u32Sharpness;
    }

    return ISIL_ERR;
}

static int isil_chip_vi_property_set_sharpness(isil_chip_vi_property_t *property, u32 sharpness)
{
    if(property) {
        if((sharpness >= MIN_VI_SHARPNESS) && (sharpness <= MAX_VI_SHARPNESS)) {
            property->u32Sharpness = sharpness;
            return ISIL_OK;
        }
        ISIL_DBG(ISIL_DBG_INFO, "set sharpness failed %d, [%d, %d]\n", sharpness, MIN_VI_SHARPNESS, MAX_VI_SHARPNESS);
        return -EINVAL;
    }

    return ISIL_ERR;
}

static int isil_chip_vi_property_update(isil_chip_vi_driver_t *driver, isil_chip_vi_config_property_t *config)
{
    if(driver && config) {
        isil_chip_t *chip;
        u32 value;
        isil_chip_vi_feature_t  *rfeature, *pfeature = &config->video_feature;
        isil_chip_vi_property_t *property = &config->video_property;

        chip = driver->chip;
        //ISIL_DBG(ISIL_DBG_INFO, "update vi %d parameter\n", property->u32Channel);
        if(property->u32Mask & MASK_VI_BRIGHTNESS) {
            //ISIL_DBG(ISIL_DBG_INFO, "brightness = %d\n", property->u32Brightness);
            mpb_write(chip, ((property->u32Channel & 0x3) * 0x10) + ISIL_VI_BRIGHNESS, (property->u32Brightness - 0x80));
            //pci_i2c_write(chip, 0x50, ((property->u32Channel & 0x3) * 0x10) + 0x01, (property->u32Brightness - 0x80));
        }
        if(property->u32Mask & MASK_VI_CONTRAST) {
            //ISIL_DBG(ISIL_DBG_INFO, "contrast = %d\n", property->u32Contrast);
            mpb_write(chip, ((property->u32Channel & 0x3) * 0x10) + ISIL_VI_CONTRAST, property->u32Contrast);
            //pci_i2c_write(chip, 0x50, ((property->u32Channel & 0x3) * 0x10) + 0x02, property->u32Contrast);
        }
        if(property->u32Mask & MASK_VI_SATURATION) {
            //ISIL_DBG(ISIL_DBG_INFO, "saturation = %d\n", property->u32Saturation);
            mpb_write(chip, ((property->u32Channel & 0x3) * 0x10) + ISIL_VI_SATURATION_U, property->u32Saturation);
            mpb_write(chip, ((property->u32Channel & 0x3) * 0x10) + ISIL_VI_SATURATION_V, property->u32Saturation);


            //pci_i2c_write(chip, 0x50, ((property->u32Channel & 0x3) * 0x10) + 0x04, property->u32Saturation);
            //pci_i2c_write(chip, 0x50, ((property->u32Channel & 0x3) * 0x10) + 0x05, property->u32Saturation);
        }
        if(property->u32Mask & MASK_VI_HUE) {
            //ISIL_DBG(ISIL_DBG_INFO, "hue = %d\n", property->u32Hue);
            mpb_write(chip, ((property->u32Channel & 0x3) * 0x10) + ISIL_VI_HUE, (property->u32Hue - 0x80));
            //pci_i2c_write(chip, 0x50, ((property->u32Channel & 0x3) * 0x10) + 0x06, (property->u32Hue - 0x80));
        }
        if(property->u32Mask & MASK_VI_SHARPNESS) {
            //ISIL_DBG(ISIL_DBG_INFO, "sharpness = %d\n", property->u32Sharpness);
            mpb_write(chip, ((property->u32Channel & 0x3) * 0x10) + ISIL_VI_SHARPNESS, property->u32Sharpness);
            //pci_i2c_write(chip, 0x50, ((property->u32Channel & 0x3) * 0x10) + 0x03, property->u32Sharpness);
        }
        property->u32Mask = 0x0;

        rfeature = &driver->video_property[pfeature->channel].video_feature;
        if(pfeature->change_mask & ISIL_VI_FEATURE_ENABLE_CHANGE_MODE_MASK) {

            rfeature->e_motion_mode = pfeature->e_motion_mode;
            value = mpb_read(chip, ISIL_VI_EVENT_CONTROL(property->u32Channel));
            if(rfeature->e_motion_mode == MOTION_TRIGGER_AUTIO) {
                value &= ~(0x3<<2);
            }else{
                value |= 0x3<<2;
            }
            mpb_write(chip, ISIL_VI_EVENT_CONTROL(property->u32Channel), value);
        }
        if(pfeature->change_mask & ISIL_VI_FEATURE_ENABLE_CHANGE_MOTION_MASK) {
            rfeature->i_motion_sensitive = pfeature->i_motion_sensitive;
            isil_chip_event_mask(driver, CHIP_VI_EVENT_MOTION, pfeature->channel, 
                    (rfeature->i_motion_sensitive < CHIP_VI_EVENT_MAX_SENSITIVE));
            /*update to hardware*/
            mpb_write(chip, ISIL_VI_EVENT_MD_TMPSENS(rfeature->channel), rfeature->i_motion_sensitive << 4);//temporal sensitivity
            mpb_write(chip, ISIL_VI_EVENT_MD_FIELD(rfeature->channel), rfeature->i_motion_sensitive & 0xf);//level sensitivity
            value = mpb_read(chip, ISIL_VI_EVENT_MD_SPSENS(rfeature->channel));
            value &= 0x0f;
            value |= (rfeature->i_motion_sensitive & 0xf)<<4;
            mpb_write(chip, ISIL_VI_EVENT_MD_SPSENS(rfeature->channel), value);//spatial sensitivity
        }
        if(pfeature->change_mask & ISIL_VI_FEATURE_ENABLE_CHANGE_NIGHT_MASK) {
            rfeature->i_night_sensitive = pfeature->i_night_sensitive;
            isil_chip_event_mask(driver, CHIP_VI_EVENT_NIGHT, pfeature->channel, 
                    (rfeature->i_night_sensitive < CHIP_VI_EVENT_MAX_SENSITIVE));
            /*update to hardware*/
            mpb_write(chip, ISIL_VI_EVENT_ND_TMPSENS(rfeature->channel), 
                    (rfeature->i_night_sensitive << 4) | (rfeature->i_night_sensitive / 4));
        }
        if(pfeature->change_mask & ISIL_VI_FEATURE_ENABLE_CHANGE_BLIND_MASK) {
            rfeature->i_blind_sensitive = pfeature->i_blind_sensitive;
            isil_chip_event_mask(driver, CHIP_VI_EVENT_BLIND, pfeature->channel, 
                    (rfeature->i_blind_sensitive < CHIP_VI_EVENT_MAX_SENSITIVE));
            /*update to hardware*/
            value = mpb_read(chip, ISIL_VI_EVENT_MD_SPSENS(rfeature->channel));
            value &= 0xf0;
            value |= rfeature->i_blind_sensitive & 0xf;
            mpb_write(chip, ISIL_VI_EVENT_MD_SPSENS(rfeature->channel), value);
        }
        if(pfeature->change_mask & ISIL_VI_FEATURE_ENABLE_CHANGE_NOVIDEO_MASK) {
            rfeature->b_novideo_enable = pfeature->b_novideo_enable;
            isil_chip_event_mask(driver, CHIP_VI_EVENT_LOST, pfeature->channel, rfeature->b_novideo_enable);
        }
        if(pfeature->change_mask & ISIL_VI_FEATURE_ENABLE_CHANGE_UNDERFLOW_MASK) {
            rfeature->b_underflow_enable = pfeature->b_underflow_enable;
            isil_chip_event_mask(driver, CHIP_VI_EVENT_UNDERFLOW, pfeature->channel, rfeature->b_underflow_enable);
        }
        if(pfeature->change_mask & ISIL_VI_FEATURE_ENABLE_CHANGE_OVERFLOW_MASK) {
            rfeature->b_overflow_enable = pfeature->b_overflow_enable;
            isil_chip_event_mask(driver, CHIP_VI_EVENT_OVERFLOW, pfeature->channel, rfeature->b_overflow_enable);
        }
        rfeature->change_mask = 0x0;

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static struct isil_chip_vi_property_operation video_property_op = {
    .init           = isil_chip_vi_property_init,
    .reset          = isil_chip_vi_property_reset,
    .get_brightness = isil_chip_vi_property_get_brightness,
    .set_brightness = isil_chip_vi_property_set_brightness,
    .get_contrast   = isil_chip_vi_property_get_contrast,
    .set_contrast   = isil_chip_vi_property_set_contrast,
    .get_saturation = isil_chip_vi_property_get_saturation,
    .set_saturation = isil_chip_vi_property_set_saturation,
    .get_hue        = isil_chip_vi_property_get_hue,
    .set_hue        = isil_chip_vi_property_set_hue,
    .get_sharpness  = isil_chip_vi_property_get_sharpness,
    .set_sharpness  = isil_chip_vi_property_set_sharpness,
    .update         = isil_chip_vi_property_update,
};

static int isil_chip_vi_motion_handler(u32 nevent, void *context)
{
    int ret = ISIL_ERR, channel;
    isil_chip_vi_event_t  *event;
    isil_chip_vi_feature_t *feature;
    isil_chip_vi_driver_t *driver = (isil_chip_vi_driver_t *)context;

    if(driver) {
        event = &driver->driver_event[nevent];
        for(channel = 0; channel < ISIL_VI_MAX_CHNL; channel++) {
            if(event->event_flags & (1 << channel)) {
                feature = &driver->video_property[channel].video_feature;
                if(feature->i_motion_sensitive != CHIP_VI_EVENT_MAX_SENSITIVE) {
                    /*add event handler*/
                    ret = ISIL_OK;
                }
            }
        }
    }else{
        ret = -EINVAL;
    }

    /*clear all event irq*/
    mpb_write(driver->chip, ISIL_VI_EVENT_MOTION_L, 0xff);
    mpb_write(driver->chip, ISIL_VI_EVENT_MOTION_H, 0xff);

    return ret;
}

static int isil_chip_vi_night_handler(u32 nevent, void *context)
{
    int ret = ISIL_ERR, channel;
    u32 flags;
    isil_chip_vi_event_t  *event;
    isil_chip_vi_feature_ext_t *feature;
    isil_chip_vi_driver_t *driver = (isil_chip_vi_driver_t *)context;

    if(driver) {
        event = &driver->driver_event[nevent];
        flags = (mpb_read(driver->chip, ND_DETL) & 0xff);
        flags |= ((mpb_read(driver->chip, ND_DETH) & 0xff) << 8);
        for(channel = 0; channel < ISIL_VI_MAX_CHNL; channel++) {
            if(event->event_flags & (1 << channel)) {
                feature = &driver->video_property[channel].video_feature_ext;
                if(feature->b_video_night_add || feature->b_video_night_del) {
                    /*add event handler*/
                    isil_msg_pool_t *msg_pool;
                    isil_msg_queue_t *msg_queue;
                    isil_msg_tcb_t *msg_tcb;

                    msg_pool = &driver->chip->chip_driver->msg_pool;
                    msg_queue = &driver->msg_queue;

                    msg_queue->op->try_get_curr_producer_from_pool(msg_queue, msg_pool);
                    if(msg_queue->curr_producer){
                        isil_video_night_detection_msg_t *msg_night;
                        msg_tcb = msg_queue->curr_producer;
                        msg_tcb->msg_type = ISIL_VIDEO_NIGHT_DETECTION_MSG;
                        msg_tcb->msg_len = sizeof(isil_video_night_detection_msg_t);
                        msg_night = (isil_video_night_detection_msg_t *)msg_tcb->msg_data;
                        msg_night->msg_type = ISIL_VIDEO_NIGHT_DETECTION_MSG;
                        msg_night->msg_len = sizeof(isil_video_night_detection_msg_t);
                        msg_night->chan_id = channel;
                        if(feature->b_video_night_add && (flags & (1 << channel))){
                            msg_night->video_night_valid_from_night_to_day = ISIL_FALSE;
                            msg_night->video_night_valid_from_day_to_night = ISIL_TRUE;
                        }else{
                            msg_night->video_night_valid_from_night_to_day = ISIL_TRUE;
                            msg_night->video_night_valid_from_day_to_night = ISIL_FALSE;
                        }

                        msg_queue->op->put_curr_producer_into_queue(msg_queue);
                        ret = ISIL_OK;
                    }else{
                        //ISIL_DBG(ISIL_DBG_ERR, "can't get msg_tcb from msg_pool, ignore channel %d night event\n", channel);
                        ret = -ENOMEM;
                    }
                }
            }
        }
    }else{
        ret = -EINVAL;
    }

    /*clear all event irq*/
    mpb_write(driver->chip, ISIL_VI_EVENT_NIGHT_L, 0xff);
    mpb_write(driver->chip, ISIL_VI_EVENT_NIGHT_H, 0xff);

    return ret;
}

static int isil_chip_vi_blind_handler(u32 nevent, void *context)
{
    int ret = ISIL_ERR, channel;
    u32 flags;
    isil_chip_vi_event_t  *event;
    isil_chip_vi_feature_ext_t *feature;
    isil_chip_vi_driver_t *driver = (isil_chip_vi_driver_t *)context;

    if(driver) {
        event = &driver->driver_event[nevent];
        flags = (mpb_read(driver->chip, ND_DETL) & 0xff);
        flags |= ((mpb_read(driver->chip, ND_DETH) & 0xff) << 8);
        for(channel = 0; channel < ISIL_VI_MAX_CHNL; channel++) {
            if(event->event_flags & (1 << channel)) {
                feature = &driver->video_property[channel].video_feature_ext;
                if(feature->b_video_blind_add || feature->b_video_blind_del) {
                    /*add event handler*/
                    isil_msg_pool_t *msg_pool;
                    isil_msg_queue_t *msg_queue;
                    isil_msg_tcb_t *msg_tcb;

                    msg_pool = &driver->chip->chip_driver->msg_pool;
                    msg_queue = &driver->msg_queue;

                    msg_queue->op->try_get_curr_producer_from_pool(msg_queue, msg_pool);
                    if(msg_queue->curr_producer){
                        isil_video_blind_detection_msg_t *msg_blind;
                        msg_tcb = msg_queue->curr_producer;
                        msg_tcb->msg_type = ISIL_VIDEO_BLIND_DETECTION_MSG;
                        msg_tcb->msg_len = sizeof(isil_video_blind_detection_msg_t);
                        msg_blind = (isil_video_blind_detection_msg_t *)msg_tcb->msg_data;
                        msg_blind->msg_type = ISIL_VIDEO_BLIND_DETECTION_MSG;
                        msg_blind->msg_len = sizeof(isil_video_blind_detection_msg_t);
                        msg_blind->chan_id = channel;
                        if(feature->b_video_blind_add && (flags & (1 << channel))){
                            msg_blind->video_blind_remove_valid = ISIL_FALSE;
                            msg_blind->video_blind_add_valid = ISIL_TRUE;
                        }else{
                            msg_blind->video_blind_remove_valid = ISIL_TRUE;
                            msg_blind->video_blind_add_valid = ISIL_FALSE;
                        }

                        msg_queue->op->put_curr_producer_into_queue(msg_queue);
                        ret = ISIL_OK;
                    }else{
                        //ISIL_DBG(ISIL_DBG_ERR, "can't get msg_tcb from msg_pool, ignore channel %d blind event\n", channel);
                        ret = -ENOMEM;
                    }
                    ret = ISIL_OK;
                }
            }
        }
    }else{
        ret = -EINVAL;
    }

    /*clear all event irq*/
    mpb_write(driver->chip, ISIL_VI_EVENT_BLIND_L, 0xff);
    mpb_write(driver->chip, ISIL_VI_EVENT_BLIND_H, 0xff);

    return ret;
}

static int isil_chip_vi_novideo_handler(u32 nevent, void *context)
{
    int ret = ISIL_ERR, channel, flags;
    isil_chip_vi_event_t  *event;
    isil_chip_vi_feature_ext_t *feature;
    isil_chip_vi_driver_t *driver = (isil_chip_vi_driver_t *)context;

    if(driver) {
        flags = (mpb_read(driver->chip, NOVID_DETL) & 0xff);
        flags |= ((mpb_read(driver->chip, NOVID_DETH) & 0xff) << 8);
        event = &driver->driver_event[nevent];
        for(channel = 0; channel < ISIL_VI_MAX_CHNL; channel++) {
            if(event->event_flags & (1 << channel)) {
                feature = &driver->video_property[channel].video_feature_ext;
                //ISIL_DBG(ISIL_DBG_INFO, "channel %d video lost\n", channel);
                if(feature->b_video_connect || feature->b_video_disconnect) {
                    /*add event handler*/
                    isil_msg_pool_t *msg_pool;
                    isil_msg_queue_t *msg_queue;
                    isil_msg_tcb_t *msg_tcb;

                    msg_pool = &driver->chip->chip_driver->msg_pool;
                    msg_queue = &driver->msg_queue;

                    msg_queue->op->try_get_curr_producer_from_pool(msg_queue, msg_pool);
                    if(msg_queue->curr_producer){
                        isil_video_lost_detection_msg_t *msg_lost;
                        msg_tcb = msg_queue->curr_producer;
                        msg_tcb->msg_type = ISIL_VIDEO_LOST_DETECTION_MSG;
                        msg_tcb->msg_len = sizeof(isil_video_lost_detection_msg_t);
                        msg_lost = (isil_video_lost_detection_msg_t *)msg_tcb->msg_data;
                        msg_lost->msg_type = ISIL_VIDEO_LOST_DETECTION_MSG;
                        msg_lost->msg_len = sizeof(isil_video_lost_detection_msg_t);
                        msg_lost->chan_id = channel;
                        if((feature->b_video_disconnect) && (flags & (1 << channel))) {
                            msg_lost->video_lost_valid = ISIL_TRUE;
                            msg_lost->video_connect_valid = ISIL_FALSE;
                            //ISIL_DBG(ISIL_DBG_INFO, "video channel %d offline\n", channel);
                        }else{
                            msg_lost->video_lost_valid = ISIL_FALSE;
                            msg_lost->video_connect_valid = ISIL_TRUE;
                            //ISIL_DBG(ISIL_DBG_INFO, "video channel %d online\n", channel);
                        }

                        msg_queue->op->put_curr_producer_into_queue(msg_queue);
                        ret = ISIL_OK;
                    }else{
                        //ISIL_DBG(ISIL_DBG_ERR, "can't get msg_tcb from msg_pool, ignore channel %d video lost event\n", channel);
                        ret = -ENOMEM;
                    }

                    ret = ISIL_OK;
                }
            }
        }
    }else{
        ret = -EINVAL;
    }

    /*clear all event irq*/
    mpb_write(driver->chip, ISIL_VI_EVENT_NOVIDEO_L, 0xff);
    mpb_write(driver->chip, ISIL_VI_EVENT_NOVIDEO_H, 0xff);

    return ret;}

static int isil_chip_vi_underflow_handler(u32 nevent, void *context)
{
    int ret = ISIL_ERR, channel;
    isil_chip_vi_event_t  *event;
    isil_chip_vi_feature_t *feature;
    isil_chip_vi_driver_t *driver = (isil_chip_vi_driver_t *)context;

    if(driver) {
        event = &driver->driver_event[nevent];
        for(channel = 0; channel < ISIL_VI_MAX_CHNL; channel++) {
            if(event->event_flags & (1 << channel)) {
                feature = &driver->video_property[channel].video_feature;
                if(feature->b_underflow_enable) {
                    /*add event handler*/
                    ret = ISIL_OK;
                }
            }
        }
    }else{
        ret = -EINVAL;
    }

    /*clear all event irq*/
    mpb_write(driver->chip, ISIL_VI_EVENT_UNDERFLOW_L, 0xff);
    mpb_write(driver->chip, ISIL_VI_EVENT_UNDERFLOW_H, 0xff);

    return ret;
}

static int isil_chip_vi_overflow_handler(u32 nevent, void *context)
{
    int ret = ISIL_ERR, channel;
    isil_chip_vi_event_t  *event;
    isil_chip_vi_feature_t *feature;
    isil_chip_vi_driver_t *driver = (isil_chip_vi_driver_t *)context;

    if(driver) {
        event = &driver->driver_event[nevent];
        for(channel = 0; channel < ISIL_VI_MAX_CHNL; channel++) {
            if(event->event_flags & (1 << channel)) {
                feature = &driver->video_property[channel].video_feature;
                if(feature->b_overflow_enable) {
                    /*add event handler*/
                    ret = ISIL_OK;
                }
            }
        }
    }else{
        ret = -EINVAL;
    }

    /*clear all event irq*/
    mpb_write(driver->chip, ISIL_VI_EVENT_OVERFLOW_L, 0xff);
    mpb_write(driver->chip, ISIL_VI_EVENT_OVERFLOW_H, 0xff);

    return ret;
}

static int isil_chip_event_mask(isil_chip_vi_driver_t *driver, enum CHIP_VI_EVENT nevent, u32 channel, int enable)
{
    int ret = ISIL_ERR;
    u32 addh, addl, value;

    if(!driver || ((nevent < CHIP_VI_EVENT_MOTION) || (nevent >= CHIP_VI_EVENT_RESERVED)) || (channel >= ISIL_VI_MAX_CHNL)) {
        ISIL_DBG(ISIL_DBG_ERR, "inavalid parameter\n");
        return -EINVAL;
    }

    switch(nevent) {
        default:
        case CHIP_VI_EVENT_MOTION:
            addh = ISIL_VI_EVENT_MASK_MOTION_H;
            addl = ISIL_VI_EVENT_MASK_MOTION_L;
            break;
        case CHIP_VI_EVENT_BLIND:
            addh = ISIL_VI_EVENT_MASK_BLIND_H;
            addl = ISIL_VI_EVENT_MASK_BLIND_L;
            break;
        case CHIP_VI_EVENT_NIGHT:
            addh = ISIL_VI_EVENT_MASK_NIGHT_H;
            addl = ISIL_VI_EVENT_MASK_NIGHT_L;
            break;
        case CHIP_VI_EVENT_LOST:
            addh = ISIL_VI_EVENT_MASK_NOVIDEO_H;
            addl = ISIL_VI_EVENT_MASK_NOVIDEO_L;
            break;
        case CHIP_VI_EVENT_UNDERFLOW:
            addh = ISIL_VI_EVENT_MASK_UNDERFLOW_H;
            addl = ISIL_VI_EVENT_MASK_UNDERFLOW_L;
            break;
        case CHIP_VI_EVENT_OVERFLOW:
            addh = ISIL_VI_EVENT_MASK_OVERFLOW_H;
            addl = ISIL_VI_EVENT_MASK_OVERFLOW_L;
            break;
    }

    if(channel < (ISIL_VI_MAX_CHNL>>1)) {
        value = mpb_read(driver->chip, addl);
        if(enable) {
            value |= (1 << channel);
        }else{
            value &= ~(1 << channel);
        }
        mpb_write(driver->chip, addl, value);
    }else{
        value = mpb_read(driver->chip, addh);
        if(enable) {
            value |= (1 <<(channel - (ISIL_VI_MAX_CHNL>>1)));
        }else{
            value &= ~(1 <<(channel - (ISIL_VI_MAX_CHNL>>1)));
        }
        mpb_write(driver->chip, addh, value);
    }

    //ISIL_DBG(ISIL_DBG_DEBUG, "%s channel %d event %d\n", enable?"enable":"disable", channel, nevent);

    return ret;
}

static int isil_chip_event_register(isil_chip_vi_driver_t *driver, enum CHIP_VI_EVENT nevent, event_handler handler, void *context)
{
    int ret = ISIL_ERR;
    unsigned long flags;
    isil_vi_event_action_t  *action;
    isil_chip_vi_event_t    *event;

    if(driver) {
        if((nevent < CHIP_VI_EVENT_MOTION) || (nevent > CHIP_VI_EVENT_OVERFLOW)) {
            ISIL_DBG(ISIL_DBG_ERR, "event number overflow\n");
            return -EINVAL;
        }
        event   = &driver->driver_event[nevent];
        action  = &event->action;
        spin_lock_irqsave(&event->lock, flags);
        if(action->context == NULL) {
            action->event   = nevent;
            action->context = context;
            action->handler = handler;
            action->next    = NULL;

            ret = ISIL_OK;
        }else{
            ISIL_DBG(ISIL_DBG_ERR, "event %d already register\n", nevent);

            ret = -ENODEV;
        }

        spin_unlock_irqrestore(&event->lock, flags);
    }

    return ret;
}

static int isil_chip_event_unregister(isil_chip_vi_driver_t *driver, enum CHIP_VI_EVENT nevent, void *context)
{
    int ret = ISIL_ERR;
    unsigned long flags;
    isil_vi_event_action_t  *action;
    isil_chip_vi_event_t    *event;

    if(driver) {
        if((nevent < CHIP_VI_EVENT_MOTION) || (nevent > CHIP_VI_EVENT_OVERFLOW)) {
            ISIL_DBG(ISIL_DBG_ERR, "event number overflow\n");
            return -EINVAL;
        }
        event   = &driver->driver_event[nevent];
        action  = &event->action;
        spin_lock_irqsave(&event->lock, flags);
        if(action->context == context) {
            action->context = NULL;
            action->handler = NULL;
            action->next    = NULL;

            ret = ISIL_OK;
        }else{
            ISIL_DBG(ISIL_DBG_ERR, "event %d already unregister\n", nevent);

            ret = -ENODEV;
        }

        spin_unlock_irqrestore(&event->lock, flags);
    }

    return ret;
}

static int isil_chip_vi_irq(int irq, void *context)
{
    u32                          event_flags;
    unsigned long flags;
    enum CHIP_VI_EVENT           nevent;
    isil_chip_vi_event_t           *event;
    isil_chip_vi_driver_t *driver = (isil_chip_vi_driver_t *)context;

    if(driver) {
        /*get event*/
        event_flags = driver->op->get_video_event(driver);
        for(nevent = CHIP_VI_EVENT_MOTION; nevent < CHIP_VI_EVENT_RESERVED; nevent++) {
            if(event_flags & (1 << nevent)) {
                isil_vi_event_action_t  *action;

                event = &driver->driver_event[nevent];
                event->count++;
                action = &event->action;
                spin_lock_irqsave(&event->lock, flags);
                while(action && action->handler) {
                    if(action->handler(event->event, action->context)){
                        event->unhandled++;
                    }

                    action = action->next;
                }
                spin_unlock_irqrestore(&event->lock, flags);
            }
        }
    }

    return 0;
}

static int isil_chip_vi_event_timer_check(void *context)
{
    if(context) {
        return isil_chip_vi_irq(IRQ_FRONT_END_TYPE_INTR, context);
    }

    return ISIL_ERR;
}

static inline char *vi_video_standard_to_string(enum CHIP_VI_STANDARD standard)
{
    char *id;

    switch(standard) {
        case CHIP_VI_STANDARD_NTSC_M:
            id = "NTSC(M)";
            break;
        case CHIP_VI_STANDARD_PAL_BDGHI:
            id = "PAL(B,D,G,H,I)";
            break;
        case CHIP_VI_STANDARD_SECAM:
            id = "SECAM";
            break;
        case CHIP_VI_STANDARD_NTSC443:
            id = "NTSC4.43";
            break;
        case CHIP_VI_STANDARD_PAL_M:
            id = "PAL(M)";
            break;
        case CHIP_VI_STANDARD_PAL_CN:
            id = "PAL(CN)";
            break;
        case CHIP_VI_STANDARD_PAL_60:
            id = "PAL 60";
            break;
        case CHIP_VI_STANDARD_AUTO:
            id = "Auto";
            break;
        default:
        case CHIP_VI_STANDARD_INVALID:
            id = "Invalid";
            break;
    }

    return id;
}

static int vi_proc_read(struct seq_file *seq, void *data)
{
    int i;
    enum CHIP_VI_EVENT nevent;
    isil_msg_queue_t *msg_queue;
    isil_chip_vi_event_t           *event;
    isil_chip_vi_config_property_t *video_property;
    isil_chip_vi_property_t *property;
    isil_chip_vi_feature_ext_t *feature_ext;
    isil_chip_vi_driver_t *driver = (isil_chip_vi_driver_t *)seq->private;

    msg_queue = &driver->msg_queue;
    seq_printf(seq, "video standard: %s, real video standard: %s\n", vi_video_standard_to_string(driver->op->get_video_standard(driver)), vi_video_standard_to_string(driver->op->get_hw_video_standard(driver)));
    seq_printf(seq, "vi configure\n");
    seq_printf(seq, "%8s %12s %12s %12s %12s %12s\n", "ch", "brightness", "contrast", "saturation", "hue", "sharpness");
    for(i = 0; i < ISIL_VI_ON_CHIP_CHNL; i++) {
        property = &driver->video_property[i].video_property;
        seq_printf(seq, "%8d %12d %12d %12d %12d %12d\n", property->u32Channel,
                property->u32Brightness, property->u32Contrast, property->u32Saturation,
                property->u32Hue, property->u32Sharpness);
    }
    seq_printf(seq, "vi feature configure:\n");
    seq_printf(seq, "%8s %8s %8s %8s %8s %8s %8s\n", "ch", "motion", "night",
            "blind", "lost", "under", "over");
    for(i = 0; i < ISIL_VI_MAX_CHNL; i++) {
        video_property = &driver->video_property[i];
        seq_printf(seq, "%8d %8d %8d %8d %8d %8d %8d\n", i,
                video_property->video_feature.i_motion_sensitive,
                video_property->video_feature.i_night_sensitive,
                video_property->video_feature.i_blind_sensitive,
                video_property->video_feature.b_novideo_enable,
                video_property->video_feature.b_underflow_enable,
                video_property->video_feature.b_overflow_enable);
    }
    seq_printf(seq, "%8s %8s %8s %8s %8s %8s %8s %8s %8s\n", "connect", "discon",
            "night_a", "night_d", "blind_a", "blind_d", "norm", "under", "over");
    for(i = 0; i < ISIL_VI_MAX_CHNL; i++) {
        video_property = &driver->video_property[i];
        feature_ext = &video_property->video_feature_ext;
        seq_printf(seq, "%8d %8d %8d %8d %8d %8d %8d %8d %8d\n",
                feature_ext->b_video_connect,
                feature_ext->b_video_disconnect,
                feature_ext->b_video_night_add,
                feature_ext->b_video_night_del,
                feature_ext->b_video_blind_add,
                feature_ext->b_video_blind_del,
                feature_ext->b_video_standard_change,
                feature_ext->b_video_underflow,
                feature_ext->b_video_overflow);
    }

    seq_printf(seq, "event status:\n%10s  :%8s %8s\n", "event", "happened", "unhandled");
    for(nevent = CHIP_VI_EVENT_MOTION; nevent < CHIP_VI_EVENT_RESERVED; nevent++) {
        event = &driver->driver_event[nevent];
        seq_printf(seq, "%10s  :%8d %8d\n", event->name, event->count, event->unhandled);
    }
    seq_printf(seq, "pending event: %d\n", msg_queue->op->get_curr_queue_entry_number(msg_queue));

    return 0;
}

static ssize_t vi_proc_write(struct file *file, const char __user *buffer,
        size_t count, loff_t *offset)
{
    isil_chip_vi_driver_t *driver = (isil_chip_vi_driver_t *)isil_file_get_seq_private(file);

    if(driver) {
        ISIL_DBG(ISIL_DBG_ERR, "vi driver uninitial\n");
    }

    return count;
}

static int chip_vi_driver_init(isil_chip_vi_driver_t *driver)
{
    int ret = ISIL_ERR;

    if(driver) {
        int i;
        isil_chip_vi_config_property_t *video_property;
        isil_chip_vi_feature_t         *video_feature;
        isil_chip_vi_feature_ext_t     *video_feature_ext;
        isil_chip_vi_event_t           *event;
        enum CHIP_VI_EVENT           nevent;
        ed_tcb_t *ed;
        isil_proc_register_s           *vi_proc;
        isil_chip_t *chip = driver->chip;
        isil_chip_timer_controller_t    *chip_timer_cont = &chip->chip_timer_cont;

        atomic_set(&driver->opened_flag, 0);
        ed = &driver->opened_vi_ed;
        vi_proc        = &driver->vi_proc;
        ed->op = &chip_vi_driver_hcd_interface_op;
        init_isil_msg_queue(&driver->msg_queue);
        init_isil_send_msg_controller(&driver->send_msg_contr, ISIL_OWNER_VI, 0, driver);
        for(i = 0; i < ISIL_VI_MAX_CHNL; i++, video_property++) {
            video_property = &driver->video_property[i];
            video_feature  = &video_property->video_feature;
            video_feature_ext  = &video_property->video_feature_ext;
            video_property->op = &video_property_op;
            video_property->op->init(&video_property->video_property, i);
            memcpy(video_feature, &default_vi_feature, sizeof(isil_chip_vi_feature_t));
            memcpy(video_feature_ext, &default_vi_feature_ext, sizeof(isil_chip_vi_feature_ext_t));
            memcpy(&video_property->video_property, &default_video_property, sizeof(isil_chip_vi_property_t));
            video_property->video_property.u32Channel = i;
            video_feature->channel = i;
            video_property->op->update(driver, video_property);
        }
        if(driver->op) {
            driver->op->set_video_standard(driver, CHIP_VI_STANDARD_PAL_BDGHI);
            driver->op->set_video_standard_ext(driver, CHIP_VI_STANDARD_AUTO);
        }else{
            ISIL_DBG(ISIL_DBG_INFO, "not support this operation, set video standard failed\n");
        }

        for(nevent = CHIP_VI_EVENT_MOTION; nevent < CHIP_VI_EVENT_RESERVED; nevent++) {
            event = &driver->driver_event[nevent];
            event->count     = 0;
            event->unhandled = 0;
            event->event     = nevent;
            memset(&event->action, 0, sizeof(isil_vi_event_action_t));
            spin_lock_init(&event->lock);
            switch(nevent) {
                case CHIP_VI_EVENT_MOTION:
                    event->name = "motion";
                    ret = isil_chip_event_register(driver, CHIP_VI_EVENT_MOTION, isil_chip_vi_motion_handler, driver);
                    break;
                case CHIP_VI_EVENT_BLIND:
                    event->name = "blind";
                    ret = isil_chip_event_register(driver, CHIP_VI_EVENT_BLIND, isil_chip_vi_blind_handler, driver);
                    break;
                case CHIP_VI_EVENT_NIGHT:
                    event->name = "night";
                    ret = isil_chip_event_register(driver, CHIP_VI_EVENT_NIGHT, isil_chip_vi_night_handler, driver);
                    break;
                case CHIP_VI_EVENT_LOST:
                    event->name = "vlost";
                    ret = isil_chip_event_register(driver, CHIP_VI_EVENT_LOST, isil_chip_vi_novideo_handler, driver);
                    break;
                case CHIP_VI_EVENT_UNDERFLOW:
                    event->name = "underflow";
                    ret = isil_chip_event_register(driver, CHIP_VI_EVENT_UNDERFLOW, isil_chip_vi_underflow_handler, driver);
                    break;
                case CHIP_VI_EVENT_OVERFLOW:
                    event->name = "overflow";
                    ret = isil_chip_event_register(driver, CHIP_VI_EVENT_OVERFLOW, isil_chip_vi_overflow_handler, driver);
                    break;
                default:
                    ISIL_DBG(ISIL_DBG_ERR, "unknown event %d\n", nevent);
                    ret = -EPERM;
            }

            if(ret) {
                ISIL_DBG(ISIL_DBG_ERR, "register event handler failed %d\n", ret);
                return ret;
            }
        }

        strcpy(vi_proc->name, "vi");
        vi_proc->read  = vi_proc_read;
        vi_proc->write = vi_proc_write;
        vi_proc->private = driver;
        vi_proc->entry   = NULL;
        isil_module_register(driver->chip, vi_proc);

#if ISIL_VI_EVENT_IRQ
        ret = chip_request_irq(driver->chip, IRQ_FRONT_END_TYPE_INTR, isil_chip_vi_irq, "vi_event", (void*)driver);
        if(ret) {
            ISIL_DBG(ISIL_DBG_ERR, "register irq failed, %d\n", ret);
        }
        driver->event_timer = INVALIDTIMERID;
#else
        driver->event_timer = chip_timer_cont->op->AddForFireTimerJob(chip_timer_cont, TIMER_40MS, isil_chip_vi_event_timer_check, driver);
        if(driver->event_timer == ADDJOBERROR) {
            ISIL_DBG(ISIL_DBG_ERR, "add timer error!\n");
            driver->event_timer = INVALIDTIMERID;
        }
#endif
    }else{
        ret = -EINVAL;
    }

    return ret;
}

static int chip_vi_driver_release(isil_chip_vi_driver_t *driver)
{
    if(driver) {
        enum CHIP_VI_EVENT  nevent;
        isil_chip_t *chip = driver->chip;
        isil_chip_timer_controller_t    *chip_timer_cont = &chip->chip_timer_cont;

        for(nevent = CHIP_VI_EVENT_MOTION; nevent < CHIP_VI_EVENT_RESERVED; nevent++) {
            isil_chip_event_unregister(driver, nevent, driver);
        }
#if ISIL_VI_EVENT_IRQ
        chip_free_irq(driver->chip, IRQ_FRONT_END_TYPE_INTR, driver);
#else
        if(driver->event_timer != INVALIDTIMERID) {
            chip_timer_cont->op->DeleteForFireTimerJob(chip_timer_cont, driver->event_timer);
            driver->event_timer = INVALIDTIMERID;
        }
#endif
        isil_module_unregister(driver->chip, &driver->vi_proc);

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int chip_vi_driver_reset(isil_chip_vi_driver_t *driver)
{
    if(driver) {
        const struct isil_ddr_dll_param *ddr_param;
        isil_chip_t *chip = driver->chip;

        mpb_write(chip, ISIL_VI_VD_EDGE, 0x03);/*use falling edge to sample ,54M to 108M*/
        mpb_write(chip, ISIL_VI_AGCGGAIN, 0x00);/*AGC loop function disable*/

        mpb_write(chip, ISIL_VI_BGNDEN, 0x04);
#ifndef FPGA_330_5864_TESTING
        ddr_param = &ddr_clk[0];
        while(ddr_param->clk){
            if(ddr_param->clk == chip->sys_clock){
                ISIL_DBG(ISIL_DBG_INFO, "configure system clock to %dMhz\n", chip->sys_clock);
                mpb_write(chip, DDRA_DLL_DQS_SEL0, ddr_param->dqs[0].dqs0_delay);
                mpb_write(chip, DDRA_DLL_DQS_SEL1, ddr_param->dqs[0].dqs1_delay);
                mpb_write(chip, DDRA_DLL_CLK90_SEL, ddr_param->dqs[0].clk90);
                mpb_write(chip, DDRB_DLL_DQS_SEL0, ddr_param->dqs[1].dqs0_delay);
                mpb_write(chip, DDRB_DLL_DQS_SEL1, ddr_param->dqs[1].dqs1_delay);
                mpb_write(chip, DDRB_DLL_CLK90_SEL, ddr_param->dqs[1].clk90);
                break;
            }
            ddr_param++;
        }
        if(ddr_param->clk == 0){
            ddr_param = &ddr_clk[0];
            mpb_write(chip, DDRA_DLL_DQS_SEL0, ddr_param->dqs[0].dqs0_delay);
            mpb_write(chip, DDRA_DLL_DQS_SEL1, ddr_param->dqs[0].dqs1_delay);
            mpb_write(chip, DDRA_DLL_CLK90_SEL, ddr_param->dqs[0].clk90);
            mpb_write(chip, DDRB_DLL_DQS_SEL0, ddr_param->dqs[1].dqs0_delay);
            mpb_write(chip, DDRB_DLL_DQS_SEL1, ddr_param->dqs[1].dqs1_delay);
            mpb_write(chip, DDRB_DLL_CLK90_SEL, ddr_param->dqs[1].clk90);
            printk("SYSTEM CLOCK RATE %dMhz NOT SUPPORT, reset  to %dMhz!!!\n", chip->sys_clock, ddr_param->clk);
            chip->sys_clock = ddr_param->clk;
        }
#endif
        mpb_write(chip, MD_DI_CELLSENS, 0x3);//adaptive sensitive
        mpb_write(chip, MD_DI_LVSENS, 0x0);

        /*send message to agent*/
        isil_vi_video_standard_gen_req(chip, REQ_VIDEO_STANDARD_CHANGED, NONBLOCK_OP);

        /*configure PV0 and PV1*/
        mpb_write(chip, PV0_LM_CHID0, (1 << 4) | (0));
        mpb_write(chip, PV1_LM_CHID0, (1 << 4) | (0));
        /*PV clock delay*/
        mpb_write(chip, PV_CLK_DLY10, 0x0);
        mpb_write(chip, PV_CLK_DLY32, 0x0);

        mpb_write(chip, PV_SEL_10, 0x44);
        mpb_write(chip, PV_SEL_32, 0x44);

        /*all PV port 108M*/
        mpb_write(chip, ISIL_VI_SYSTEM_CLOCK, 0x0);
        mpb_write(chip, PV_CLK_OUT, 0x0);

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int chip_vi_driver_get_video_standard(isil_chip_vi_driver_t *driver)
{
    if(driver) {
        return driver->video_standard;
    }

    return -EINVAL;
}

static int chip_vi_driver_set_video_standard(isil_chip_vi_driver_t *driver, enum CHIP_VI_STANDARD norm)
{
    int ret = ISIL_OK;

    if(driver) {
        isil_chip_t *chip;
        chip_driver_t    *chip_driver = NULL;
        isil_video_bus_t   *video_bus = NULL;
        int i = 0;

        chip = driver->chip;
        chip_driver = chip->chip_driver;
        if(chip_driver) {
            video_bus = &chip_driver->video_bus;
        }

#if 0 //disable cropping
        for(i = 0; i < ISIL_VI_ON_CHIP_CHNL; i++) {
            if((norm != CHIP_VI_STANDARD_NTSC443) && (norm != CHIP_VI_STANDARD_NTSC_M) && (norm != CHIP_VI_STANDARD_SECAM)) {
                ISIL_DBG(ISIL_DBG_INFO, "set vi channel %d cropping to 720x576\n", i);
                /*cropping pal 720x576*/
                mpb_write(chip, (i<<4) + ISIL_VI_HDELAY_XY, 0x0a);/*pal*/
                mpb_write(chip, (i<<4) + ISIL_VI_HACTIVE_XY, 0xd0);
                mpb_write(chip, (i<<4) + ISIL_VI_VDELAY_XY, 0x06);
                mpb_write(chip, (i<<4) + ISIL_VI_VACTIVE_XY, 0x20);/*288*/
                mpb_write(chip, (i<<4) + ISIL_VI_HV_XY, 0x28);

                pci_i2c_write(chip, 0x52, (i<<4) + 0x07, (0x0<<6) | (0x1<<4) | (0x0<<2) | (0x2<<0));
                pci_i2c_write(chip, 0x52, (i<<4) + 0x08, 0x12);
                pci_i2c_write(chip, 0x52, (i<<4) + 0x09, 0x20);
                pci_i2c_write(chip, 0x52, (i<<4) + 0x0a, 0x0a);
                pci_i2c_write(chip, 0x52, (i<<4) + 0x0b, 0xd0);

                pci_i2c_write(chip, 0x54, (i<<4) + 0x07, (0x0<<6) | (0x1<<4) | (0x0<<2) | (0x2<<0));
                pci_i2c_write(chip, 0x54, (i<<4) + 0x08, 0x12);
                pci_i2c_write(chip, 0x54, (i<<4) + 0x09, 0x20);
                pci_i2c_write(chip, 0x54, (i<<4) + 0x0a, 0x0a);
                pci_i2c_write(chip, 0x54, (i<<4) + 0x0b, 0xd0);

                pci_i2c_write(chip, 0x56, (i<<4) + 0x07, (0x0<<6) | (0x1<<4) | (0x0<<2) | (0x2<<0));
                pci_i2c_write(chip, 0x56, (i<<4) + 0x08, 0x12);
                pci_i2c_write(chip, 0x56, (i<<4) + 0x09, 0x20);
                pci_i2c_write(chip, 0x56, (i<<4) + 0x0a, 0x0a);
                pci_i2c_write(chip, 0x56, (i<<4) + 0x0b, 0xd0);
            }else{
                ISIL_DBG(ISIL_DBG_INFO, "set vi channel %d cropping to 720x480\n", i);
                /*cropping ntsc 720x480*/
                mpb_write(chip, (i<<4) + ISIL_VI_HDELAY_XY, 0x0f);/*ntsc*/
                mpb_write(chip, (i<<4) + ISIL_VI_HACTIVE_XY, 0xd0);
                mpb_write(chip, (i<<4) + ISIL_VI_VDELAY_XY, 0x06);
                mpb_write(chip, (i<<4) + ISIL_VI_VACTIVE_XY, 0xf0);/*240*/
                mpb_write(chip, (i<<4) + ISIL_VI_HV_XY, 0x08);

                pci_i2c_write(chip, 0x52, (i<<4) + 0x07, (0x0<<6) | (0x0<<4) | (0x0<<2) | (0x2<<0));
                pci_i2c_write(chip, 0x52, (i<<4) + 0x08, 0x12);
                pci_i2c_write(chip, 0x52, (i<<4) + 0x09, 0xf0);
                pci_i2c_write(chip, 0x52, (i<<4) + 0x0a, 0x0a);
                pci_i2c_write(chip, 0x52, (i<<4) + 0x0b, 0xd0);

                pci_i2c_write(chip, 0x54, (i<<4) + 0x07, (0x0<<6) | (0x0<<4) | (0x0<<2) | (0x2<<0));
                pci_i2c_write(chip, 0x54, (i<<4) + 0x08, 0x12);
                pci_i2c_write(chip, 0x54, (i<<4) + 0x09, 0xf0);
                pci_i2c_write(chip, 0x54, (i<<4) + 0x0a, 0x0a);
                pci_i2c_write(chip, 0x54, (i<<4) + 0x0b, 0xd0);

                pci_i2c_write(chip, 0x56, (i<<4) + 0x07, (0x0<<6) | (0x0<<4) | (0x0<<2) | (0x2<<0));
                pci_i2c_write(chip, 0x56, (i<<4) + 0x08, 0x12);
                pci_i2c_write(chip, 0x56, (i<<4) + 0x09, 0xf0);
                pci_i2c_write(chip, 0x56, (i<<4) + 0x0a, 0x0a);
                pci_i2c_write(chip, 0x56, (i<<4) + 0x0b, 0xd0);
            }
        }
#endif
        for(i = 0; i < ISIL_VI_MAX_CHNL; i++) {
            if((norm != CHIP_VI_STANDARD_NTSC443) && (norm != CHIP_VI_STANDARD_NTSC_M) && (norm != CHIP_VI_STANDARD_SECAM)) {
                mpb_write(chip, ISIL_VI_SYSTEM_IN_WIDTH(i),   (720>>2));
                mpb_write(chip, ISIL_VI_SYSTEM_IN_HEIGHT(i),  (576>>3));
                mpb_write(chip, ISIL_VI_SYSTEM_OUT_WIDTH(i),  (720>>3));
                mpb_write(chip, ISIL_VI_SYSTEM_OUT_HEIGHT(i), (576>>4));
                mpb_write(chip, ISIL_VI_SYSTEM_VIDEO_STANDARD, 0x1);/*pal*/
            }else{
                mpb_write(chip, ISIL_VI_SYSTEM_IN_WIDTH(i),   (720>>2));
                mpb_write(chip, ISIL_VI_SYSTEM_IN_HEIGHT(i),  (480>>3));
                mpb_write(chip, ISIL_VI_SYSTEM_OUT_WIDTH(i),  (720>>3));
                mpb_write(chip, ISIL_VI_SYSTEM_OUT_HEIGHT(i), (480>>4));
                mpb_write(chip, ISIL_VI_SYSTEM_VIDEO_STANDARD, 0x0);/*ntsc*/
            }
        }

    }else{
        ret = -EINVAL;
    }

    return ret;
}


static int chip_vi_driver_set_video_standard_ext(isil_chip_vi_driver_t *driver, enum CHIP_VI_STANDARD norm)
{
    if(driver) {
        int i, ret;
        u32 value = 0;
        isil_chip_t *chip;
        chip_driver_t    *chip_driver = NULL;

        chip = driver->chip;
        chip_driver = chip->chip_driver;

        ISIL_DBG(ISIL_DBG_INFO, "set vi video standard %d\n", norm);
        for(i = 0; i < ISIL_VI_ON_CHIP_CHNL; i++) {
            switch(norm) {
                case CHIP_VI_STANDARD_AUTO:
                    value = 0x07;
                    break;
                case CHIP_VI_STANDARD_NTSC443:
                    value = 0x03;
                    break;
                case CHIP_VI_STANDARD_NTSC_M:
                    value = 0x00;
                    break;
                case CHIP_VI_STANDARD_PAL_60:
                    value = 0x06;
                    break;
                case CHIP_VI_STANDARD_PAL_BDGHI:
                    value = 0x01;
                    break;
                case CHIP_VI_STANDARD_PAL_CN:
                    value = 0x05;
                    break;
                case CHIP_VI_STANDARD_PAL_M:
                    value = 0x04;
                    break;
                case CHIP_VI_STANDARD_SECAM:
                    value = 0x02;
                    break;
                default:
                    ret = -EINVAL;
                    ISIL_DBG(ISIL_DBG_ERR, "not support video standard %d\n", norm);
                    return ret;
            }
            mpb_write(chip, ISIL_VI_STANDARD + (i<<4), value);

            pci_i2c_write(chip, 0x52, ISIL_VI_STANDARD + (i<<4), value);
            pci_i2c_write(chip, 0x54, ISIL_VI_STANDARD + (i<<4), value);
            pci_i2c_write(chip, 0x56, ISIL_VI_STANDARD + (i<<4), value);
        }

        driver->video_standard = norm;

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static u32 chip_vi_driver_get_video_event(isil_chip_vi_driver_t *driver)
{
    enum CHIP_VI_EVENT nevent;
    u32 event_flags = 0, temp;
    isil_chip_t *chip;
    isil_chip_vi_event_t *event;

    if(driver) {
        chip = driver->chip;
        /*get event summary*/
        temp = (mpb_read(chip, ISIL_VI_EVENT_SUMMARY_L) & 0xff) |
            ((mpb_read(chip, ISIL_VI_EVENT_SUMMARY_H) & 0xf) << 8);
        for(nevent = CHIP_VI_EVENT_MOTION; nevent < CHIP_VI_EVENT_RESERVED; nevent++) {
            if(temp & 0x3) {
                /*get event state*/
                event_flags |= (1 << nevent);
                event = &driver->driver_event[nevent];
                switch(nevent) {
                    case CHIP_VI_EVENT_MOTION:
                        event->event_flags = (mpb_read(chip, ISIL_VI_EVENT_MOTION_L) & 0xff) | 
                            ((mpb_read(chip, ISIL_VI_EVENT_MOTION_H) & 0xff)<<8);
                        break;
                    case CHIP_VI_EVENT_NIGHT:
                        event->event_flags = (mpb_read(chip, ISIL_VI_EVENT_NIGHT_L) & 0xff) | 
                            ((mpb_read(chip, ISIL_VI_EVENT_NIGHT_H) & 0xff)<<8);
                        break;
                    case CHIP_VI_EVENT_BLIND:
                        event->event_flags = (mpb_read(chip, ISIL_VI_EVENT_BLIND_L) & 0xff) | 
                            ((mpb_read(chip, ISIL_VI_EVENT_BLIND_H) & 0xff)<<8);
                        break;
                    case CHIP_VI_EVENT_LOST:
                        event->event_flags = (mpb_read(chip, ISIL_VI_EVENT_NOVIDEO_L) & 0xff) | 
                            ((mpb_read(chip, ISIL_VI_EVENT_NOVIDEO_H) & 0xff)<<8);
                        break;
                    case CHIP_VI_EVENT_UNDERFLOW:
                        event->event_flags = (mpb_read(chip, ISIL_VI_EVENT_UNDERFLOW_L) & 0xff) | 
                            ((mpb_read(chip, ISIL_VI_EVENT_UNDERFLOW_H) & 0xff)<<8);
                        break;
                    case CHIP_VI_EVENT_OVERFLOW:
                        event->event_flags = (mpb_read(chip, ISIL_VI_EVENT_OVERFLOW_L) & 0xff) | 
                            ((mpb_read(chip, ISIL_VI_EVENT_OVERFLOW_H) & 0xff)<<8);
                        break;
                    default:
                        ISIL_DBG(ISIL_DBG_ERR, "unknown event %d\n", nevent);
                }
            }
            temp >>= 2;
        }
    }

    return event_flags;
}

static int chip_vi_driver_get_hw_video_standard(isil_chip_vi_driver_t *driver)
{
    if(driver){
        return driver->hw_video_standard;
    }

    return CHIP_VI_STANDARD_PAL_BDGHI;
}

static int chip_vi_driver_polling_hw_video_standard(isil_chip_vi_driver_t *driver)
{
    if(driver){
        isil_chip_t *chip;
        isil_chip_vi_feature_ext_t *feature;
        u32 data;

        chip = driver->chip;

        //adaptive video norm
#if defined(TW5864_ASIC_NEW)
        data = mpb_read(chip, ISIL_VI_STANDARD);
#else
        pci_i2c_read(chip, 0x50, 0x0e, &data);
#endif
        if(data & (1 << 7)){
            return ISIL_OK;
        }
        data >>= 4;
        data &= 0x7;

        driver->hw_video_standard = data;
        feature = &driver->video_property[0].video_feature_ext;

        if(driver->hw_video_standard != driver->pre_hw_video_standard){
            ISIL_DBG(ISIL_DBG_INFO, "hardware video standard from %s to %s\n", vi_video_standard_to_string(driver->pre_hw_video_standard), 
                    vi_video_standard_to_string(driver->hw_video_standard));
            if(feature->b_video_standard_change){
                isil_msg_pool_t *msg_pool;
                isil_msg_queue_t *msg_queue;
                isil_msg_tcb_t *msg_tcb;

                msg_pool = &chip->chip_driver->msg_pool;
                msg_queue = &driver->msg_queue;
                msg_queue->op->try_get_curr_producer_from_pool(msg_queue, msg_pool);
                if(msg_queue->curr_producer){
                    isil_video_standard_change_detection_msg_t *msg_norm;

                    msg_tcb = msg_queue->curr_producer;
                    msg_tcb->msg_type = ISIL_VIDEO_STANDARD_CHANGE_DETECTION_MSG;
                    msg_tcb->msg_len = sizeof(isil_video_standard_change_detection_msg_t);
                    msg_norm = (isil_video_standard_change_detection_msg_t *)msg_tcb->msg_data;
                    msg_norm->msg_type = ISIL_VIDEO_STANDARD_CHANGE_DETECTION_MSG;
                    msg_norm->msg_len = sizeof(isil_video_standard_change_detection_msg_t);
                    msg_norm->chan_id = 0;
                    msg_norm->latest_video_standard = driver->pre_hw_video_standard;
                    msg_norm->curr_video_standard = driver->hw_video_standard;

                    msg_queue->op->put_curr_producer_into_queue(msg_queue);
                }else{
                    //ISIL_DBG(ISIL_DBG_ERR, "can't get msg_tcb from msg_pool, ignore video_standard changed event\n");
                }
            }
            if(driver->op->get_video_standard(driver) == CHIP_VI_STANDARD_AUTO){
                /*send message to agent*/
                isil_vi_video_standard_gen_req(chip, REQ_VIDEO_STANDARD_CHANGED, NONBLOCK_OP);
            }
            driver->pre_hw_video_standard = driver->hw_video_standard;
        }

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static struct isil_chip_vi_operation isil_chip_vi_operation_op = {
    .init = chip_vi_driver_init,
    .release = chip_vi_driver_release,
    .reset   = chip_vi_driver_reset,

    .get_hw_video_standard = chip_vi_driver_get_hw_video_standard,
    .polling_hw_video_standard = chip_vi_driver_polling_hw_video_standard,
    .get_video_standard = chip_vi_driver_get_video_standard,
    .set_video_standard = chip_vi_driver_set_video_standard,
    .set_video_standard_ext = chip_vi_driver_set_video_standard_ext,
    .get_video_event    = chip_vi_driver_get_video_event,
};

int init_chip_vi_driver(struct isil_service_device *tsd, isil_chip_t *chip)
{
    int ret = 0;
    isil_chip_vi_driver_t *driver;

    driver = (isil_chip_vi_driver_t *)kzalloc(sizeof(isil_chip_vi_driver_t), GFP_KERNEL);
    if(driver) {
        if(chip) {
            driver->chip = chip;
            driver->op  = &isil_chip_vi_operation_op;
            chip->chip_vi_driver = driver;
            ret = driver->op->init(driver);
            driver->op->reset(driver);/*reset before all operation*/
            if(ret){
                ISIL_DBG(ISIL_DBG_ERR, "failed to init vi driver %d\n", ret);
                return -ENOMEM;
            }
            init_endpoint_tcb(&driver->opened_vi_ed, chip->bus_id, chip->chip_id, 
                    INVALID_ISIL_ED_TYPE_ID, 0, 
                    0, driver, NULL, 
                    NULL, NULL);
            tsd->ped = &driver->opened_vi_ed;
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

int remove_chip_vi_driver(struct isil_service_device *tsd)
{
    isil_chip_vi_driver_t *driver;
    int ret = ISIL_ERR;

    if(tsd && tsd->ped) {
        isil_chip_t *chip;

        driver = to_get_chip_vi_driver_with_opened_logic_chan_ed(tsd->ped);
        chip = driver->chip;
        ret = driver->op->release(driver);
        kfree(driver);
        if(chip){
            chip->chip_vi_driver = NULL;
        }
    }

    return ret;
}

static int isil_chip_vi_register_driver_init(struct isil_chip_device *tcd,struct isil_service_device *tsd,void * para,struct cmd_arg *arg)
{
    isil_chip_t  *chip = (isil_chip_t  *)para;

    return init_chip_vi_driver(tsd,chip);
}

static void isil_chip_vi_register_driver_remove(struct isil_chip_device *tcd,struct isil_service_device *tsd,void * para,struct cmd_arg *arg)
{
    /*other para may used furture... so we reserved them*/
    remove_chip_vi_driver(tsd);
}


struct isil_dev_id  isil_chip_vi_dev_id =
{
    .epobj = {
        .vendor_id = ISIL,
        .bus_id =  1,
        .chip_id = TW5864,
        .func_id = (0 << 16) | 0,
        .type = ISIL_VIDEO_IN,
        .key = {0},                
    },
    .version = 1,
};

struct tsd_driver_operations  isil_chip_vi_ops =
{
    .init = isil_chip_vi_register_driver_init,
    .remove = isil_chip_vi_register_driver_remove,
};

struct isil_service_driver   isil_chip_vi_driv =
{
    .driver = {
        .name = "tw5864_video_input_driv",
    },
    .device_id = {
        .tid = &isil_chip_vi_dev_id,
        .ops = &isil_chip_vi_ops,
        //.driver_data =  //parameters
    },
};


int isil_chip_vi_driver_init()
{
    //isil_chip_vi_driv.device_id.para = para;
    return register_tc_driver(&isil_chip_vi_driv);
}


void isil_chip_vi_driver_remove(void)
{
    unregister_tc_driver(&isil_chip_vi_driv);
}
