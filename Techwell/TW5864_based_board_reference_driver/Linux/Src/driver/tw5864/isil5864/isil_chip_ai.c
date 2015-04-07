#include	<isil5864/isil_common.h>
#include	<isil5864/tc_common.h>

/*only for 256fs mode*/
static const char chip_hardware_sample_256fs[ISIL_AUDIO_RESERVED][6] = {
    /*-----ACKI-------|------ACKN--------*/
    {0x09, 0xb5, 0x83, 0x00, 0xa0, 0x00},
    {0x13, 0x6b, 0x07, 0x01, 0x40, 0x00},
    {0x26, 0xd6, 0x0e, 0x02, 0x80, 0x00},
    {0x35, 0x85, 0x65, 0x03, 0x72, 0x00},
    {0x3a, 0x41, 0x15, 0x03, 0xc0, 0x00},
};

static int chip_ai_driver_hcd_interface_open(ed_tcb_t *opened_ed)
{
    int ret = ISIL_ERR;

    if(opened_ed) {
        isil_chip_ai_driver_t *driver;

        driver = to_get_chip_ai_driver_with_opened_logic_chan_ed(opened_ed);
        switch(atomic_read(&opened_ed->state)){
            default:
            case ISIL_ED_UNREGISTER:
                opened_ed->ed.op->init_complete(&opened_ed->ed);
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

static int chip_ai_driver_hcd_interface_close(ed_tcb_t *opened_ed)
{
    int ret = ISIL_ERR;

    if(opened_ed) {
        isil_chip_ai_driver_t *driver;

        driver = to_get_chip_ai_driver_with_opened_logic_chan_ed(opened_ed);
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

static void chip_ai_driver_hcd_interface_suspend(ed_tcb_t *opened_ed)
{
    if(opened_ed) {
        isil_chip_ai_driver_t *driver;

        driver = to_get_chip_ai_driver_with_opened_logic_chan_ed(opened_ed);
        if(atomic_read(&driver->opened_flag)) {
        }else{
            ISIL_DBG(ISIL_DBG_ERR, "device have not opened\n");
            return;
        }
    }

    return ;
}

static void chip_ai_driver_hcd_interface_resume(ed_tcb_t *opened_ed)
{
    if(opened_ed) {
        isil_chip_ai_driver_t *driver;

        driver = to_get_chip_ai_driver_with_opened_logic_chan_ed(opened_ed);
        if(atomic_read(&driver->opened_flag)) {
        }else{
            ISIL_DBG(ISIL_DBG_ERR, "device have not opened\n");
            return;
        }
    }

    return ;
}

static int chip_ai_driver_hcd_interface_ioctl(ed_tcb_t *opened_ed, unsigned int cmd, unsigned long arg)
{
    int ret = ISIL_ERR;

    if(opened_ed) {
        isil_chip_ai_driver_t *driver;
        struct isil_chip_ai_sys ai_sys, *sys_p;
        struct isil_chip_ai_property ai_chn_property, *chn_p;
        AI_CH_VOLUME audio_volume;

        sys_p = &ai_sys;
        chn_p = &ai_chn_property;
        driver = to_get_chip_ai_driver_with_opened_logic_chan_ed(opened_ed);

        if(atomic_read(&driver->opened_flag)) {
        }else{
            ISIL_DBG(ISIL_DBG_ERR, "device have not opened\n");
            return -EPERM;
        }

        ISIL_DBG(ISIL_DBG_INFO, "%c, %d\n", _IOC_TYPE(cmd), _IOC_NR(cmd));
        switch(cmd) {
            case ISIL_AI_SET_AUDIO_SYS_PARM:
                if(copy_from_user(sys_p, (void *)arg, sizeof(struct isil_chip_ai_sys)) != 0){
                    printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                }
                if(sys_p->changemask & ISIL_CHIP_AI_CHANGEMASK_CLOCK_MODE) {
                    ret = driver->op->set_clock_mode(driver, sys_p->clock_mode);
                    if(ret) {
                        break;
                    }
                    ret = ISIL_OK;
                }
                if(sys_p->changemask & ISIL_CHIP_AI_CHANGEMASK_SYNC_MODE) {
                    ret = driver->op->set_sync_mode(driver, sys_p->clock_mode);
                    if(ret) {
                        break;
                    }
                    ret = ISIL_OK;
                }
                if(sys_p->changemask & ISIL_CHIP_AI_CHANGEMASK_SAMPLE_RATE) {
                    ret = driver->op->set_sample_rate(driver, sys_p->clock_mode);
                    if(ret) {
                        break;
                    }
                    ret = ISIL_OK;
                }
                if(sys_p->changemask & ISIL_CHIP_AI_CHANGEMASK_BITS) {
                    ret = driver->op->set_bits(driver, sys_p->clock_mode);
                    if(ret) {
                        break;
                    }
                    ret = ISIL_OK;
                }
                break;
            case ISIL_AI_GET_AUDIO_SYS_PARM:
                if(sys_p->changemask & ISIL_CHIP_AI_CHANGEMASK_CLOCK_MODE) {
                    sys_p->clock_mode = driver->op->get_clock_mode(driver);
                }
                if(sys_p->changemask & ISIL_CHIP_AI_CHANGEMASK_SYNC_MODE) {
                    sys_p->clock_mode = driver->op->get_sync_mode(driver);
                }
                if(sys_p->changemask & ISIL_CHIP_AI_CHANGEMASK_SAMPLE_RATE) {
                    sys_p->clock_mode = driver->op->get_sample_rate(driver);
                }
                if(sys_p->changemask & ISIL_CHIP_AI_CHANGEMASK_BITS) {
                    sys_p->clock_mode = driver->op->get_bits(driver);
                }
                if(sys_p->changemask) {
                    if(copy_to_user((void *)arg, sys_p, sizeof(struct isil_chip_ai_sys)) != 0){
                        printk("%s, %d: copy_to_user failed\n", __FUNCTION__, __LINE__);
                    }
                }
                ret = ISIL_OK;
                break;
            case ISIL_AI_SET_AUDIO_GAIN:
                if(copy_from_user(&audio_volume, (void *)arg, sizeof(AI_CH_VOLUME)) != 0){
                    printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                }
                chn_p->changemask |= ISIL_CHIP_AI_CHANGEMASK_GAIN;
                chn_p->channel    = audio_volume.u32Channel;
                chn_p->gain       = audio_volume.u32Volume;
                if(chn_p->changemask & ISIL_CHIP_AI_CHANGEMASK_GAIN) {
                    isil_chip_ai_config_property_t *ai_config;
                    if(chn_p->channel < ISIL_AI_MAX_CHNL) {
                        ai_config = &driver->audio_property[chn_p->channel];
                        ret = ai_config->op->set_gain(&ai_config->audio_property, chn_p->gain);
                        if(ret) {
                            break;
                        }
                    }else{
                        ISIL_DBG(ISIL_DBG_ERR, "audio channle number %d overflow\n", chn_p->channel);
                        ret = -EINVAL;
                    }
                }
                break;
            case ISIL_AI_GET_AUDIO_CHN_PARM:
                if(copy_from_user(chn_p, (void *)arg, sizeof(struct isil_chip_ai_property)) != 0){
                    printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);
                }
                if(chn_p->changemask & ISIL_CHIP_AI_CHANGEMASK_GAIN) {
                    isil_chip_ai_config_property_t *ai_config;
                    if(chn_p->channel < ISIL_AI_MAX_CHNL) {
                        ai_config = &driver->audio_property[chn_p->channel];
                        chn_p->gain = ai_config->op->get_gain(&ai_config->audio_property);
                        if(copy_to_user((void *)arg, chn_p, sizeof(struct isil_chip_ai_property)) != 0){
                            printk("%s, %d: copy_to_user failed\n", __FUNCTION__, __LINE__);
                        }
                        ret = ISIL_OK;
                    }else{
                        ISIL_DBG(ISIL_DBG_ERR, "audio channle number %d overflow\n", chn_p->channel);
                        ret = -EINVAL;
                    }
                }
                break;
            default:
                ret = -EBADRQC;
                ISIL_DBG(ISIL_DBG_ERR, "No such command!\n");
                break;
        }
    }

    return ret;
}

static int chip_ai_driver_hcd_interface_get_state(ed_tcb_t *opened_ed)
{
    int	ret = ISIL_ED_UNREGISTER;

    if(opened_ed != NULL){
        ret = atomic_read(&opened_ed->state);
    }

    return ret;
}

static isil_hcd_interface_operation chip_ai_driver_hcd_interface_op = {
    .open = chip_ai_driver_hcd_interface_open,
    .close = chip_ai_driver_hcd_interface_close,
    .suspend = chip_ai_driver_hcd_interface_suspend,
    .resume = chip_ai_driver_hcd_interface_resume,
    .ioctl = chip_ai_driver_hcd_interface_ioctl,
    .get_state = chip_ai_driver_hcd_interface_get_state,
};

static int isil_chip_ai_property_init(isil_chip_ai_property_t *property, u32 chnl)
{
    int ret = ISIL_ERR;
    if(property) {
        if(chnl < ISIL_AI_MAX_CHNL) {
            property->channel = chnl;
            property->gain    = 0;
            ret = ISIL_OK;
        }else{
            ISIL_DBG(ISIL_DBG_ERR, "channel number %d overflow\n", chnl);
        }
    }

    return ret;
}

static int isil_chip_ai_property_reset(isil_chip_ai_property_t *property)
{
    int ret = ISIL_ERR;
    if(property) {
        property->gain    = 0;
        ret = ISIL_OK;
    }

    return ret;
}

static u32 isil_chip_ai_property_get_gain(isil_chip_ai_property_t *property)
{
    if(property) {
        return property->gain;
    }

    return ISIL_ERR;
}

static int isil_chip_ai_property_set_gain(isil_chip_ai_property_t *property, u32 gain)
{
    if(property) {
        if((gain >= MIN_AI_GAIN) && (gain <= MAX_AI_GAIN)) {
            property->gain = gain;
            return ISIL_OK;
        }
        ISIL_DBG(ISIL_DBG_INFO, "set gain failed, [%d, %d]\n", MIN_VI_CONTRAST, MAX_VI_CONTRAST);
        return -EINVAL;
    }
    return ISIL_ERR;
}

static int isil_chip_ai_property_update(isil_chip_ai_property_t *property, isil_chip_t *chip)
{
    if(property && chip) {
        if(property->changemask & ISIL_CHIP_AI_CHANGEMASK_GAIN) {
            u32 temp;
            if(property->channel < 2) {
                temp = mpb_read(chip, ISIL_AI_GAIN_21);
                temp &= ~(0xf << (4 * property->channel));
                temp |= ((property->gain & 0xf) << (4 * property->channel));
                mpb_write(chip, ISIL_AI_GAIN_21, temp);
            }else{
                temp = mpb_read(chip, ISIL_AI_GAIN_43);
                temp &= ~(0xf << (4 * (property->channel - 2)));
                temp |= ((property->gain & 0xf) << (4 * (property->channel - 2)));
                mpb_write(chip, ISIL_AI_GAIN_43, temp);
            }
        }

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static struct isil_chip_ai_property_operation isil_chip_ai_property_operation_op = {
    .init       = isil_chip_ai_property_init,
    .reset      = isil_chip_ai_property_reset,
    .get_gain   = isil_chip_ai_property_get_gain,
    .set_gain   = isil_chip_ai_property_set_gain,
    .update     = isil_chip_ai_property_update,
};

static int chip_ai_driver_init(isil_chip_ai_driver_t *driver)
{
    if(driver) {
        int i;
        isil_chip_ai_config_property_t *audio_property;
        ed_tcb_t       *ed;

        ed = &driver->opened_ai_ed;
        audio_property = driver->audio_property;
        ed->op = &chip_ai_driver_hcd_interface_op;
        for(i = 0; i < ISIL_AI_MAX_CHNL; i++, audio_property++) {
            audio_property->op = &isil_chip_ai_property_operation_op;
            audio_property->op->init(&audio_property->audio_property, i);
        }

        return ISIL_OK;
    }

    return ISIL_ERR;
}

static void chip_ai_driver_release(isil_chip_ai_driver_t *driver)
{
    if(driver) {
        int i;
        isil_chip_ai_config_property_t *audio_property;

        audio_property = driver->audio_property;
        for(i = 0; i < ISIL_AI_MAX_CHNL; i++, audio_property++) {
            audio_property->op->reset(&audio_property->audio_property);
        }
    }
}

static int chip_ai_driver_reset(isil_chip_ai_driver_t *driver)
{
    if(driver) {
        isil_chip_t *chip;

        chip = driver->chip;

        mpb_write(chip, 0x62, 0x03);/*I2S, 16audios*/
        mpb_write(chip, 0x6c, 0xe1);
        mpb_write(chip, 0x6d, 0x00);
        mpb_write(chip, 0x64, 0x31);
        mpb_write(chip, 0x65, 0x75);
        mpb_write(chip, 0x66, 0xb9);
        mpb_write(chip, 0x67, 0xfd);
        mpb_write(chip, 0x68, 0x20);
        mpb_write(chip, 0x69, 0x64);
        mpb_write(chip, 0x6a, 0xa8);
        mpb_write(chip, 0x6b, 0xec);

#if defined(TW5864_ASIC_NEW)
        mpb_write(chip, 0xdf, 0x04);
        mpb_write(chip, 0xdd, 0xc0);
        mpb_write(chip, 0x6c, 0xf1);
        mpb_write(chip, 0x71, 0x10);

        mpb_write(chip, 0x75, 0xe4);
        mpb_write(chip, 0x76, 0x22);
        mpb_write(chip, 0x77, 0x0c);
        mpb_write(chip, 0x78, 0x00);
        mpb_write(chip, 0x79, 0xc0);
        mpb_write(chip, 0x7a, 0x00);
#endif
        return ISIL_OK;
    }

    return ISIL_ERR;
}

static int chip_ai_driver_get_clock_mode(isil_chip_ai_driver_t *driver)
{
    if(driver) {
        return driver->clock_mode;
    }

    return ISIL_ERR;
}

static int chip_ai_driver_set_clock_mode(isil_chip_ai_driver_t *driver, enum AUDIO_CLOCK_MODE clock_mode)
{
    int ret = ISIL_ERR, temp;

    if(driver) {
        switch(clock_mode) {
            case AUDIO_CLOCK_SLAVE:
                driver->clock_mode = clock_mode;
                temp = mpb_read(driver->chip, ISIL_AI_CLOCK_MODE);
                temp &= ~(1 << 5);
                mpb_write(driver->chip, ISIL_AI_CLOCK_MODE, temp);
                ret = ISIL_OK;
                break;
            case AUDIO_CLOCK_MASTER:
                driver->clock_mode = clock_mode;
                temp = mpb_read(driver->chip, ISIL_AI_CLOCK_MODE);
                temp |= (1 << 5);
                mpb_write(driver->chip, ISIL_AI_CLOCK_MODE, temp);
                ret = ISIL_OK;
                break;
            default:
                ISIL_DBG(ISIL_DBG_ERR, "unsupport clock mode %d\n", clock_mode);
                ret = -EINVAL;
                break;
        }
    }

    return ret;
}

static int chip_ai_driver_get_sync_mode(isil_chip_ai_driver_t *driver)
{
    if(driver) {
        return driver->sync_mode;
    }

    return ISIL_ERR;
}

static int chip_ai_driver_set_sync_mode(isil_chip_ai_driver_t *driver, enum AUDIO_SYNC_MODE sync_mode)
{
    int ret = ISIL_ERR, temp;

    if(driver) {
        switch(sync_mode) {
            case AUDIO_SYNC_MODE_DSP:
                driver->sync_mode = sync_mode;
                temp = mpb_read(driver->chip, ISIL_AI_SYNC_MODE);
                temp |= (1 << 6);
                mpb_write(driver->chip, ISIL_AI_SYNC_MODE, temp);
                ret = ISIL_OK;
                break;
            case AUDIO_SYNC_MODE_I2S:
                driver->sync_mode = sync_mode;
                temp = mpb_read(driver->chip, ISIL_AI_SYNC_MODE);
                temp &= ~(1 << 6);
                mpb_write(driver->chip, ISIL_AI_SYNC_MODE, temp);
                ret = ISIL_OK;
                break;
            default:
                ISIL_DBG(ISIL_DBG_ERR, "unsupport sync mode %d\n", sync_mode);
                ret = -EINVAL;
                break;
        }
    }

    return ISIL_ERR;
}

static int chip_ai_driver_get_sample_rate(isil_chip_ai_driver_t *driver)
{
    if(driver) {
        return driver->sample_rate;
    }

    return ISIL_ERR;
}

static int chip_ai_driver_set_sample_rate(isil_chip_ai_driver_t *driver, enum ISIL_AUDIO_SAMPLE_RATE sample_rate)
{
    int ret = ISIL_ERR, i;

    if(driver) {
        switch(sample_rate) {
            case ISIL_AUDIO_8000:
                driver->sample_rate = sample_rate;
                for(i = 0; i < 6; i++) {
                    mpb_write(driver->chip, ISIL_AI_SAMPLE_RATE + i, chip_hardware_sample_256fs[sample_rate][i]);
                }
                ret = ISIL_OK;
                break;
            case ISIL_AUDIO_16000:
                driver->sample_rate = sample_rate;
                for(i = 0; i < 6; i++) {
                    mpb_write(driver->chip, ISIL_AI_SAMPLE_RATE + i, chip_hardware_sample_256fs[sample_rate][i]);
                }
                ret = ISIL_OK;
                break;
            case ISIL_AUDIO_32000:
                driver->sample_rate = sample_rate;
                for(i = 0; i < 6; i++) {
                    mpb_write(driver->chip, ISIL_AI_SAMPLE_RATE + i, chip_hardware_sample_256fs[sample_rate][i]);
                }
                ret = ISIL_OK;
                break;
            case ISIL_AUDIO_44100:
                driver->sample_rate = sample_rate;
                for(i = 0; i < 6; i++) {
                    mpb_write(driver->chip, ISIL_AI_SAMPLE_RATE + i, chip_hardware_sample_256fs[sample_rate][i]);
                }
                ret = ISIL_OK;
                break;
            case ISIL_AUDIO_48000:
                driver->sample_rate = sample_rate;
                for(i = 0; i < 6; i++) {
                    mpb_write(driver->chip, ISIL_AI_SAMPLE_RATE + i, chip_hardware_sample_256fs[sample_rate][i]);
                }
                ret = ISIL_OK;
                break;
            default:
                ISIL_DBG(ISIL_DBG_ERR, "unsupport sample rate %d\n", sample_rate);
                ret = -EINVAL;
                break;
        }
    }

    return ISIL_ERR;
}

static int chip_ai_driver_get_bits(isil_chip_ai_driver_t *driver)
{
    if(driver) {
        return driver->bits;
    }

    return ISIL_ERR;
}

static int chip_ai_driver_set_bits(isil_chip_ai_driver_t *driver, enum ISIL_AUDIO_BIT bits)
{
    int ret = ISIL_ERR, temp;

    if(driver) {
        switch(bits) {
            case ISIL_AUDIO_8BIT:
                driver->bits = bits;
                temp = mpb_read(driver->chip, ISIL_AI_BITS);
                temp |= (1 << 2);
                mpb_write(driver->chip, ISIL_AI_BITS, temp);
                ret = ISIL_OK;
                break;
            case ISIL_AUDIO_16BIT:
                driver->bits = bits;
                temp = mpb_read(driver->chip, ISIL_AI_BITS);
                temp &= ~(1 << 2);
                mpb_write(driver->chip, ISIL_AI_BITS, temp);
                ret = ISIL_OK;
                break;
            default:
                ISIL_DBG(ISIL_DBG_ERR, "unsupport bits %d\n", bits);
                ret = -EINVAL;
                break;
        }
    }

    return ISIL_ERR;
}

struct isil_chip_ai_operation isil_chip_ai_operation_op = {
    .init           = chip_ai_driver_init,
    .release        = chip_ai_driver_release,
    .reset          = chip_ai_driver_reset,

    .get_clock_mode = chip_ai_driver_get_clock_mode,
    .set_clock_mode = chip_ai_driver_set_clock_mode,
    .get_sync_mode  = chip_ai_driver_get_sync_mode,
    .set_sync_mode  = chip_ai_driver_set_sync_mode,
    .get_sample_rate= chip_ai_driver_get_sample_rate,
    .set_sample_rate= chip_ai_driver_set_sample_rate,
    .get_bits       = chip_ai_driver_get_bits,
    .set_bits       = chip_ai_driver_set_bits
};

int init_chip_ai_driver(struct isil_service_device *tsd, isil_chip_t *chip)
{
    int ret = 0;
    isil_chip_ai_driver_t *driver;

    driver = (isil_chip_ai_driver_t *)kmalloc(sizeof(isil_chip_ai_driver_t),GFP_KERNEL);
    if(driver) {
        if(chip) {
            driver->chip = chip;
            driver->op = &isil_chip_ai_operation_op;
            chip->chip_ai_driver = driver;
            ret = driver->op->init(driver);
            driver->op->reset(driver);
            if(ret){
                ISIL_DBG(ISIL_DBG_ERR, "failed to init ai driver %d!\n", ret);
                return -ENOMEM;
            }
            init_endpoint_tcb(&driver->opened_ai_ed, chip->bus_id, chip->chip_id, 
                    INVALID_ISIL_ED_TYPE_ID, 0, 
                    0, driver, NULL, 
                    NULL, NULL);
            tsd->ped = &driver->opened_ai_ed;
            return ISIL_OK;
        }else{
            ISIL_DBG(ISIL_DBG_ERR, "null pointer\n");
            return -EINVAL;
        }
    }else{
        ISIL_DBG(ISIL_DBG_ERR, "nomemory\n");
        return -ENOMEM;
    }

    return ISIL_ERR;
}

int remove_chip_ai_driver(struct isil_service_device *tsd)
{
    isil_chip_ai_driver_t *driver;

    if(tsd && tsd->ped) {
        isil_chip_t *chip;

        driver = to_get_chip_ai_driver_with_opened_logic_chan_ed(tsd->ped);
        chip = driver->chip;
        driver->op->release(driver);
        kfree(driver);
        if(chip){
            chip->chip_ai_driver = NULL;
        }
        return ISIL_OK;
    }

    return ISIL_ERR;
}





static int isil_chip_ai_register_driver_init(struct isil_chip_device *tcd,struct isil_service_device *tsd,void * para,struct cmd_arg *arg)
{
    isil_chip_t  *chip = (isil_chip_t  *)para;

    return init_chip_ai_driver(tsd,chip);
}


static void isil_chip_ai_register_driver_remove(struct isil_chip_device *tcd,struct isil_service_device *tsd,void * para,struct cmd_arg *arg)
{      
    printk("in %s\n",__FUNCTION__);
    remove_chip_ai_driver(tsd);
}


struct isil_dev_id  isil_chip_ai_dev_id =
{
    .epobj = {
        .vendor_id = ISIL,
        .bus_id =  1,
        .chip_id = TW5864,
        .func_id = (0 << 16) | 0,
        .type = ISIL_AUDIO_IN,
        .key = {0},                
    },
    .version = 1,
};


struct tsd_driver_operations  isil_chip_ai_ops =
{
    .init = isil_chip_ai_register_driver_init,
    .remove = isil_chip_ai_register_driver_remove,
};

struct isil_service_driver   isil_chip_ai_driv =
{
    .driver = {
        .name = "tw5864_audio_input_driv",
    },
    .device_id = {
        .tid = &isil_chip_ai_dev_id,
        .ops = &isil_chip_ai_ops,
        //.driver_data =  //parameters
    },
};


int isil_chip_ai_driver_init(void)
{
    //isil_chip_ai_driv.device_id.para = para;
    return register_tc_driver(&isil_chip_ai_driv);
}


void isil_chip_ai_driver_remove(void)
{
    unregister_tc_driver(&isil_chip_ai_driv);
}



