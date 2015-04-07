#include <isil5864/tc_common.h>





int isil_jpeg_driver_init(struct isil_chip_device *tcd, struct isil_service_device *tsd, void * para,struct cmd_arg *arg)
{
    int ret;
    unsigned long bus_id,chip_id,lg_id;
    isil_chip_t  *chip = (isil_chip_t  *)para;
    isil_jpeg_logic_encode_chan_t	*jed;

    bus_id = get_busid_from_epobj(&tcd->epobj);
    chip_id = get_chipid_from_epobj(&tcd->epobj);
    lg_id = arg->channel_idx;

    jed = chip->jpeg_encode_chan;
    jed += lg_id;

    printk("jed[0x%8p] in %s---> \n",jed, __FUNCTION__);
    ret = init_isil_jpeg_encode_chan(jed, chip->bus_id, chip->chip_id, lg_id, chip);
    if(ret){
        dbg("jpeg driv init failed!\n");
        return ret;
    }
    tsd->ped = &jed->opened_logic_chan_ed;
    return ret;
}


void isil_jpeg_driver_remove(struct isil_chip_device *tcd, struct isil_service_device *tsd, void * para,struct cmd_arg *arg)
{
    isil_jpeg_logic_encode_chan_t *vj;

    //tc_trace;
    if(!tsd->ped){
        dbg("tsd has no ed!\n");
        return;
    }
    vj = container_of(tsd->ped,isil_jpeg_logic_encode_chan_t,opened_logic_chan_ed);
    remove_isil_jpeg_encode_chan(vj);        
}


struct isil_dev_id  isil_jpeg_en_dev_id =
{
    .epobj = {
		.vendor_id = ISIL,
        .bus_id =  1,
        .chip_id = TW5864,
        .func_id = (STREAM_TYPE_MAJOR << 16) | CODEC_VIDEO_MJPG,
        .type = ISIL_ENCODER,                
    },
    .version = 1,
};


struct tsd_driver_operations  isil_jpeg_en_ops =
{
    .init = isil_jpeg_driver_init,
    .remove = isil_jpeg_driver_remove,
};

struct isil_service_driver   isil_jpeg_encoder_driv =
{
    .driver = {
        .name = "tw5864_jpeg_driv",
    },
    .device_id = {
        .tid = &isil_jpeg_en_dev_id,
        .ops = &isil_jpeg_en_ops,
        //.driver_data =  //parameters
    },
};


int isil_jpeg_en_driver_init(void)
{
    //isil_jpeg_encoder_driv.device_id.para = para;
    return register_tc_driver(&isil_jpeg_encoder_driv);
}


void isil_jpeg_en_driver_remove()
{
    unregister_tc_driver(&isil_jpeg_encoder_driv);
}


