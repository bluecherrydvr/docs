#include <isil5864/tc_common.h>





int isil_audio_de_register_driver_init(struct isil_chip_device *tcd,struct isil_service_device * tsd,void * para,struct cmd_arg *arg)
{
    int ret;
    unsigned long bus_id,chip_id,lg_id;
    isil_chip_t  *chip = (isil_chip_t  *)para;
    isil_audio_driver_t  *ad;

    bus_id = get_busid_from_epobj(&tcd->epobj);
    chip_id = get_chipid_from_epobj(&tcd->epobj);
    lg_id = ISIL_AUDIO_OUT_SPEAKER_ID;

    ad = chip->audio_chan_driver;
    ad += ISIL_AUDIO_OUT_SPEAKER_ID;

    printk("ad[0x%8p] in %s---> \n",ad, __FUNCTION__);
    ret = init_isil_audio_decode_chan(ad,&chip->chip_audio,bus_id,chip_id,lg_id);
    if(ret){
        dbg("audio decoder driv init failed!\n");
        return ret;
    }
    tsd->ped = &ad->opened_logic_chan_ed;
    return ret;
}


void isil_audio_de_register_driver_remove(struct isil_chip_device *tcd,struct isil_service_device * tsd,void * para,struct cmd_arg *arg)
{
    isil_audio_driver_t  *ad;

    //tc_trace;
    if(!tsd->ped){
        dbg("tsd has no ed!\n");
        return;
    }
    ad = container_of(tsd->ped,isil_audio_driver_t,opened_logic_chan_ed);
    remove_isil_audio_decode_chan(ad);
}



struct isil_dev_id  isil_audio_de_dev_id =
{
    .epobj = {
        .vendor_id = ISIL,
        .bus_id =  1,
        .chip_id = TW5864,
        .func_id = (STREAM_TYPE_AUDIO << 16) | CODEC_AUDIO_ADPCM,
        .type = ISIL_DECODER,                
    },
    .version = 1,
};


struct tsd_driver_operations  isil_audio_de_ops =
{
    .init = isil_audio_de_register_driver_init,
    .remove = isil_audio_de_register_driver_remove,
};

struct isil_service_driver   isil_audio_decoder_driv =
{
    .driver = {
        .name = "tw5864_audio_dedriv",
    },
    .device_id = {
        .tid = &isil_audio_de_dev_id,
        .ops = &isil_audio_de_ops,
        //.driver_data =  //parameters
    },
};


int isil_audio_de_driver_init(void)
{
    //isil_audio_decoder_driv.device_id.para = para;
    return register_tc_driver(&isil_audio_decoder_driv);
}


void isil_audio_de_driver_remove(void)
{
    unregister_tc_driver(&isil_audio_decoder_driv);
}


