#include <isil5864/tc_common.h>

int isil_video_en_register_driver_init(struct isil_chip_device *tcd,struct isil_service_device * tsd,void * para,struct cmd_arg *arg)
{
    int ret;
    unsigned long bus_id,chip_id,lg_id;
    isil_chip_t  *chip = (isil_chip_t  *)para;
    isil_h264_logic_encode_chan_t  *ved;

    bus_id = get_busid_from_epobj(&tcd->epobj);
    chip_id = get_chipid_from_epobj(&tcd->epobj);
    lg_id = arg->channel_idx;

    ved = chip->h264_master_encode_logic_chan;
    ved += lg_id;

    printk("ved[0x%8p] logic_id =%lu in %s---> \n",ved,lg_id, __FUNCTION__);
    ret = init_isil_h264_encode_chan(ved, chip->bus_id, chip->chip_id, lg_id, ISIL_MASTER_BITSTREAM, chip);
    if(ret){
        dbg("h264 master driv init failed!\n");
        return ret;
    }
    tsd->ped = &ved->opened_logic_chan_ed;
    return ret;
}


void isil_video_en_register_driver_remove(struct isil_chip_device *tcd,struct isil_service_device * tsd,void * para,struct cmd_arg *arg)
{
    isil_h264_logic_encode_chan_t  *vd;

    tc_trace;
    if(!tsd->ped){
        dbg("tsd has no ed!\n");
        return;
    }
    vd = container_of(tsd->ped,isil_h264_logic_encode_chan_t,opened_logic_chan_ed);
    remove_isil_h264_encode_chan(vd);
}


struct isil_dev_id  isil_video_en_dev_id =
{
    .epobj = {
		.vendor_id = ISIL,
        .bus_id =  1,
        .chip_id = TW5864,
        .func_id = STREAM_TYPE_MAJOR | CODEC_VIDEO_H264,
        .type = ISIL_ENCODER,                
    },
    .version = 1,
};


struct tsd_driver_operations  isil_video_en_ops =
{
    .init = isil_video_en_register_driver_init,
    .remove = isil_video_en_register_driver_remove,
};

struct isil_service_driver   isil_video_en_driv =
{
    .driver = {
        .name = "tw5864_video_driv",
    },
    .device_id = {
        .tid = &isil_video_en_dev_id,
        .ops = &isil_video_en_ops,
        //.driver_data =  //parameters
    },
};


int isil_video_en_driver_init(void)
{
    tc_trace;
    //isil_video_en_driv.device_id.para = para;
    return register_tc_driver(&isil_video_en_driv);
}


void isil_video_en_driver_remove()
{
    unregister_tc_driver(&isil_video_en_driv);
}










int isil_videosub_en_register_driver_init(struct isil_chip_device *tcd,struct isil_service_device * tsd,void * para,struct cmd_arg *arg)
{
    int ret;
    unsigned long bus_id,chip_id,lg_id;
    isil_chip_t  *chip = (isil_chip_t  *)para;
    isil_h264_logic_encode_chan_t  *ved;

    bus_id = get_busid_from_epobj(&tcd->epobj);
    chip_id = get_chipid_from_epobj(&tcd->epobj);
    lg_id = arg->channel_idx;

    ved = chip->h264_sub_encode_logic_chan;
    ved += lg_id;

    printk("ved[0x%8p] in %s---> \n",ved, __FUNCTION__);
    ret = init_isil_h264_encode_chan(ved, chip->bus_id, chip->chip_id, lg_id, ISIL_SUB_BITSTREAM, chip);
    if(ret){
        dbg("sub h264 driv init failed!\n");
        return ret;
    }
    tsd->ped = &ved->opened_logic_chan_ed;
    return ret;
}



struct isil_dev_id  isil_videosub_en_dev_id =
{
    .epobj = {
		.vendor_id = ISIL,
        .bus_id =  1,
        .chip_id = TW5864,
        .func_id = (STREAM_TYPE_MINOR << 16) | CODEC_VIDEO_H264,
        .type = ISIL_ENCODER,                
    },
    .version = 1,
};


struct tsd_driver_operations  isil_videosub_en_ops =
{
    .init = isil_videosub_en_register_driver_init,
};

struct isil_service_driver   isil_videosub_en_driv =
{
    .driver = {
        .name = "tw5864_videosub_driv",
    },
    .device_id = {
        .tid = &isil_videosub_en_dev_id,
        .ops = &isil_videosub_en_ops,
        //.driver_data =  //parameters
    },
};


int isil_videosub_en_driver_init(void)
{
    tc_trace;
    //isil_videosub_en_driv.device_id.para = para;
    return register_tc_driver(&isil_videosub_en_driv);
}


void isil_videosub_en_driver_remove()
{
    unregister_tc_driver(&isil_videosub_en_driv);
}

