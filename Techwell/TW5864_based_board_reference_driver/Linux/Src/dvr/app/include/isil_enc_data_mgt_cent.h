#ifndef   _ISIL_ENC_DATA_MGT_CENT_H
#define  _ISIL_ENC_DATA_MGT_CENT_H



#ifdef __cplusplus
extern "C" {
#endif



#include "isil_net_nty.h"

#include "isil_media_config.h"




enum DATA_REG_TYPE_E{
    ENC_DATA_NONE_REG = 0 ,
    ENC_DATA_HAVE_REG = 1 ,
};


struct ENC_DATA_REG_T
{
    unsigned int chip_id;
    unsigned int chan_id;
    enum ECHANFUNCPRO chanpro;

    volatile int reg_stat; 

    struct NET_NOTIFY_REG_T nty_reg;
    
    int (*reg)(struct ENC_DATA_REG_T *enc_data_reg);
    int (*unreg)(struct ENC_DATA_REG_T *enc_data_reg);
    
};


extern void init_enc_data_reg_array( void );

extern struct ENC_DATA_REG_T *get_enc_data_reg_by_inf(unsigned int chip_num,
                                                                                    unsigned int chan_num ,
                                                                                    enum ECHANFUNCPRO chanpro );




#ifdef __cplusplus
}
#endif //__cplusplus

#endif


