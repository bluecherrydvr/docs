#ifndef  _ISIL_AV_PACKET_EX_H
#define  _ISIL_AV_PACKET_EX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "isil_data_stream.h"
#include "isil_net_data_header.h"


extern ISIL_AV_PACKET *alloc_av_packet_ex( int len);

extern void release_av_packet_ex(ISIL_AV_PACKET *av_packet);

extern struct NET_DATA_NODE_T * put_av_packet_to_net_data_node(ISIL_AV_PACKET *av_packet);


#ifdef __cplusplus
}
#endif

#endif
