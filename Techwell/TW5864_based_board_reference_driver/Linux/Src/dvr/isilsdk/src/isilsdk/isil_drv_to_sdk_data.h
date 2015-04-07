#ifndef   _ISIL_DRV_TO_SDK_DATA_H
#define  _ISIL_DRV_TO_SDK_DATA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "isil_data_stream.h"
#include "isil_interface.h"


extern int parse_drv_stream_to_av_pck(unsigned int chip_id,
                               unsigned int chan_id,
                               STREAM_TYPE_E stream_type,
                               unsigned char * buff ,
                               ISIL_AV_PACKET *av_packet);






#ifdef __cplusplus
}
#endif

#endif



