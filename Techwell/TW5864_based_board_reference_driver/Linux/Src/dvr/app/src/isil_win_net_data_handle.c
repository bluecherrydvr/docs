#include <string.h>
#include <stdio.h>

#include "isil_win_net_header.h"
#include "isil_data_stream.h"
#include "io_utils.h"
//#include "isil_codec_common.h"


void net_swap_net_packet_head(sNet_packet_head *net_packet_head)
{
    unsigned int tmp32;
    unsigned short tmp16;

    tmp32 = net_packet_head->lFrameID;
    net_packet_head->lFrameID = bswap32(tmp32);
    tmp32 = net_packet_head->lFrameSize;
    net_packet_head->lFrameSize =  bswap32(tmp32);
    tmp16 = net_packet_head->nDataLen;
    net_packet_head->nDataLen = bswap16(tmp16);
    tmp16 = net_packet_head->nPacketCount;
    net_packet_head->nPacketCount = bswap16(tmp16);
    tmp16 = net_packet_head->nPacketNum;
    net_packet_head->nPacketNum = bswap16(tmp16);

}


void net_swap_isil_frame_head( struct _tagISIL_FRAME_HEAD  *isil_frame_head)
{
    unsigned int tmp32;
    unsigned short tmp16;
    tmp32 = isil_frame_head->lEncodeType;
    isil_frame_head->lEncodeType = bswap32(tmp32);
    tmp32 = isil_frame_head->lFrameFlag;
    isil_frame_head->lFrameFlag =  bswap32(tmp32);
    tmp32 = isil_frame_head->lFrameSize;
    isil_frame_head->lFrameSize = bswap32(tmp32);
    tmp32 = isil_frame_head->lTimeStamp ;
    isil_frame_head->lTimeStamp = bswap32(tmp32);
    tmp32 = isil_frame_head->lReserve;
    isil_frame_head->lReserve =  bswap32(tmp32);
    tmp16 = isil_frame_head->nFrameType;
    isil_frame_head->nFrameType = bswap16(tmp16);
    tmp16 = isil_frame_head->nHeadSize;
    isil_frame_head->nHeadSize = bswap16(tmp16);
}

void net_swap_isil_nal_head( comISIL_NAL_HEAD * isil_nal_head)
{
    int *tmp;
    tmp = (int *)isil_nal_head;
    *tmp = bswap32(*tmp);
}


int net_handle_5864_IDR_frame(int fd ,
                             void* net_addr,
                             ISIL_AV_PACKET *av_header,
                             int realorpb,
                             short nErrFlag,
                             void *sps_buff,
                             int sps_len,
                             void *pps_buff,
                             int pps_len)
{

     int i  = 1,frame_size = 0 ,nal_num = 0 ,left_len = 0;
     unsigned short tmpShort;

     unsigned int packetCout ,packetNum = 0;

     char *buff;

     struct iovec  iovSend[IOVMAX];

     sISIL_FRAME_HEAD isil_frame_head;
     comISIL_NAL_HEAD isil_nal_head[NALMAXUNIT];
     sNet_packet_head Net_packet_head;

     memset(&isil_frame_head, 0x00, sizeof(sISIL_FRAME_HEAD));
     memset(&Net_packet_head, 0x00, sizeof(sNet_packet_head));

     if(sps_buff == NULL || sps_len == 0) {
         return -1;
     }

     if(pps_buff == NULL || pps_len == 0) {
         return -1;
     }

     if( !av_header->date_len || !av_header->buff) {
         return -1;
     }


     iovSend[0].iov_base = (char*)&Net_packet_head;
     iovSend[0].iov_len  = sizeof(sNet_packet_head);


     H264_INF_T *h264_inf_p = &(av_header->data_type_u.h264_inf);

     isil_frame_head.lFrameFlag =ISIL_FRAME_HEAD_FLAG; //htonl(ISIL_FRAME_HEAD_FLAG);
     isil_frame_head.nHeadSize  =  sizeof(sISIL_FRAME_HEAD);//htons(sizeof(sISIL_FRAME_HEAD));

     if(h264_inf_p->stream_type == ISIL_MAIN_STREAM_E) {
        isil_frame_head.nFrameType = ISIL_FRAME_TYPE_MAIN_I;//htons(ISIL_FRAME_TYPE_MAIN_I);
    }
    else{
        isil_frame_head.nFrameType = ISIL_FRAME_TYPE_SUB_I;
    }

    iovSend[i].iov_base = (char*)&isil_frame_head;
    iovSend[i].iov_len =  sizeof(sISIL_FRAME_HEAD);

    frame_size += iovSend[i].iov_len;
    ++i;

    isil_nal_head[nal_num].cNalType =ISIL_NAL_SPS ;
    isil_nal_head[nal_num].lNalSize  = sps_len;

    net_swap_isil_nal_head(&isil_nal_head[nal_num]);

    iovSend[i].iov_base = (char*)&isil_nal_head[nal_num];
    iovSend[i].iov_len = sizeof(comISIL_NAL_HEAD);

    frame_size += iovSend[i].iov_len;

    ++i;

    iovSend[i].iov_base = sps_buff;
    iovSend[i].iov_len  =  sps_len;

    frame_size += iovSend[i].iov_len;

    ++i;
    ++nal_num;

    isil_nal_head[nal_num].cNalType = ISIL_NAL_PPS;

    isil_nal_head[nal_num].lNalSize  = pps_len;

    net_swap_isil_nal_head(&isil_nal_head[nal_num]);

    iovSend[i].iov_base = (char*)&isil_nal_head[nal_num];
    iovSend[i].iov_len = sizeof(comISIL_NAL_HEAD);

    frame_size += iovSend[i].iov_len;

    ++i;

    iovSend[i].iov_base = pps_buff;
    iovSend[i].iov_len  = pps_len;

    frame_size += iovSend[i].iov_len;

    ++i;
    ++nal_num;

    isil_nal_head[nal_num].cNalType = ISIL_NAL_SLICE_IDR;//htonl(ISIL_NAL_SLICE_IDR);
    isil_nal_head[nal_num].lNalSize   = av_header->date_len;

    net_swap_isil_nal_head(&isil_nal_head[nal_num]);

    iovSend[i].iov_base = (char*)&isil_nal_head[nal_num];
    iovSend[i].iov_len = sizeof(comISIL_NAL_HEAD);

    frame_size += iovSend[i].iov_len;

    ++i;

    frame_size += av_header->date_len;

    buff  = (char *)av_header->buff;

    if( frame_size <= NETVALIADSIZE ) {
        iovSend[i].iov_base = buff;
        iovSend[i].iov_len  = av_header->date_len;
        Net_packet_head.nDataLen =   frame_size;
        left_len = 0;
    }
    else{

        iovSend[i].iov_base = buff ;

        iovSend[i].iov_len   = NETVALIADSIZE + av_header->date_len - frame_size;
        Net_packet_head.nDataLen =   NETVALIADSIZE;
        left_len = frame_size - NETVALIADSIZE;

    }

    packetCout = ( frame_size / NETVALIADSIZE) +1;

    isil_frame_head.lFrameSize =  frame_size - sizeof(sISIL_FRAME_HEAD);
    isil_frame_head.lTimeStamp = av_header->pts;
    isil_frame_head.lEncodeType = ISIL_VENCODE_H264;
    Net_packet_head.lFrameID  = av_header->frm_seq;
    Net_packet_head.lFrameSize = frame_size;

    Net_packet_head.nPacketNum = packetNum++;
    Net_packet_head.nPacketCount = packetCout;

    if(realorpb == REALSTREAM){
        Net_packet_head.xInfo.cFlag    = sizeof(sNet_packet_head);
        Net_packet_head.xInfo.cFrameType = isil_frame_head.nFrameType;
    }
    else if (realorpb == FILESTREAM ){

        Net_packet_head.xFlag.nType  = bswap16(FRAMEDATA);
        Net_packet_head.xFlag.nErrFlag = bswap16(nErrFlag);
        if( nErrFlag ==  -1188)
            isil_frame_head.lReserve = 0;
        else
            isil_frame_head.lReserve  = bswap32(1);
    }
    else
            return -1;

    net_swap_net_packet_head(&Net_packet_head);
    net_swap_isil_frame_head(&isil_frame_head);

    if( isil_writeiovall(fd,iovSend,8, net_addr) < 0 ){
            perror("Net Send Data failed :");
            return -1;
    }

    buff += iovSend[i].iov_len;

    while(left_len > 0){

        iovSend[1].iov_base =  (char*)buff;
        iovSend[1].iov_len =  ((left_len <= NETVALIADSIZE)? left_len:NETVALIADSIZE);
        tmpShort = packetNum++;
        Net_packet_head.nPacketNum = bswap16(tmpShort);
        tmpShort = iovSend[1].iov_len;
        Net_packet_head.nDataLen =     bswap16(tmpShort);

        if(isil_writeiovall(fd,iovSend,2, net_addr) < 0){
            perror("Net Send Data failed :");
            return -1;
        }

        left_len-= iovSend[1].iov_len;
        buff += iovSend[1].iov_len;

    }

    return 0;

}


int net_handle_5864_audio_frame(int fd ,
                                                        void* net_addr,
                                                        ISIL_AV_PACKET *av_header,
                                                        int realorpb,
                                                        short nErrFlag )
{

    int frame_size = 0;

    unsigned int audioType,encodeType;
//    char *buff_ptr = NULL;

    AUDIO_INF_T *audio_inf_p;

    struct iovec  iovSend[IOVMAX];

    sISIL_FRAME_HEAD isil_frame_head;

    comISIL_NAL_HEAD isil_nal_head;
    sNet_packet_head Net_packet_head;

    if( !av_header ) {
        return -1;
    }

    if( !av_header->buff || !av_header->date_len) {
        return -1;
    }

    memset(&isil_frame_head, 0x00, sizeof(sISIL_FRAME_HEAD));
    memset(&Net_packet_head, 0x00, sizeof(sNet_packet_head));

    frame_size += av_header->date_len;

    audio_inf_p = &(av_header->data_type_u.audio_inf);

    switch (audio_inf_p->audio_type_e) {

    case ISIL_AUDIO_PCM_E:
        audioType = ISIL_NAL_AUDIO_PCM;
        encodeType = ISIL_AENCODE_PCM;

        break;

    case ISIL_AUDIO_ALAW_E:
        audioType = ISIL_NAL_AUDIO_ALAW;
        encodeType = ISIL_AENCODE_ALAW;
        break;

    case ISIL_AUDIO_ULAW_E:
        audioType = ISIL_NAL_AUDIO_ULAW;
        encodeType = ISIL_AENCODE_ULAW;
        break;

    case ISIL_AUDIO_ADPCM_E:

        
        audioType = ISIL_NAL_AUDIO_ADPCM;
        encodeType = ISIL_AENCODE_ADPCM;
        break;

    default:
        return -1;
    }

    isil_nal_head.cNalType = audioType;
    isil_nal_head.lNalSize = frame_size;

    net_swap_isil_nal_head(&isil_nal_head);


    frame_size += sizeof(comISIL_NAL_HEAD);
    isil_frame_head.lEncodeType = encodeType;
    isil_frame_head.lFrameFlag = ISIL_FRAME_HEAD_FLAG;
    isil_frame_head.nHeadSize  = sizeof(sISIL_FRAME_HEAD);

    isil_frame_head.lTimeStamp = av_header->pts;
    isil_frame_head.nFrameType =ISIL_FRAME_TYPE_A;//Ö¡ÀàÐÍ
    isil_frame_head.lFrameSize = frame_size ;

    frame_size += sizeof( sISIL_FRAME_HEAD );

    Net_packet_head.nDataLen = frame_size;
    Net_packet_head.lFrameSize = frame_size;
    Net_packet_head.nPacketNum = 0;
    Net_packet_head.nPacketCount = 1;

    Net_packet_head.lFrameID  = av_header->frm_seq;

    if(realorpb == REALSTREAM){
        Net_packet_head.xInfo.cFlag    =  sizeof(sNet_packet_head);
        Net_packet_head.xInfo.cFrameType = ISIL_FRAME_TYPE_A;
    }
    else if ( realorpb == FILESTREAM ){
        Net_packet_head.xFlag.nType  = bswap16(FRAMEDATA);
        Net_packet_head.xFlag.nErrFlag = bswap16(nErrFlag);
         if( nErrFlag ==  -1188)
                isil_frame_head.lReserve = 0;
        else
                isil_frame_head.lReserve  = bswap32(1);
    }
    else
        return -1;

    iovSend[1].iov_base = (char *)&isil_frame_head;
    iovSend[1].iov_len  = sizeof(sISIL_FRAME_HEAD);
    iovSend[2].iov_base = (char *)&isil_nal_head;
    iovSend[2].iov_len =  sizeof(comISIL_NAL_HEAD);
    iovSend[3].iov_base = (char *)av_header->buff;
    iovSend[3].iov_len  = av_header->date_len;

    iovSend[0].iov_len =  sizeof(sNet_packet_head);
    iovSend[0].iov_base = (char *)&Net_packet_head;

    net_swap_isil_frame_head(&isil_frame_head);
    net_swap_net_packet_head(&Net_packet_head);


    if(isil_writeiovall(fd,iovSend,4, net_addr) < 0){
            perror("NET Send data failed :");
            return -1;
    }

    return 0;

}


int net_handle_5864_MJPEG_frame(int fd ,
                                void* net_addr,
                                ISIL_AV_PACKET *av_header)
{

    int frame_size = 0;
    int packetNum = 0;
    int left_len = 0;
    int first_header_len = 0;
    int packetCout;
    char *buff_ptr = NULL;

    unsigned short tmpShort;

    struct iovec  iovSend[IOVMAX];

    sISIL_FRAME_HEAD isil_frame_head;

    comISIL_NAL_HEAD isil_nal_head;
    sNet_packet_head Net_packet_head;


    if( !av_header || !av_header->buff || !av_header->date_len) {
        return -1;
    }

    memset(&isil_frame_head, 0x00, sizeof(sISIL_FRAME_HEAD));
    memset(&Net_packet_head, 0x00, sizeof(sNet_packet_head));
    buff_ptr = (char  *)av_header->buff;


    frame_size += av_header->date_len;


    isil_nal_head.cNalType = (ISIL_NAL_MJPEG);//htonl(ISIL_NAL_SLICE_P);
    isil_nal_head.lNalSize = (frame_size) ; //htonl(frame_size);
    net_swap_isil_nal_head(&isil_nal_head);
    frame_size += sizeof(comISIL_NAL_HEAD);
    first_header_len += sizeof(comISIL_NAL_HEAD);


    isil_frame_head.nFrameType = ISIL_FRAME_TYPE_PIC;//htons(FrameType);
    isil_frame_head.lFrameFlag = ISIL_FRAME_HEAD_FLAG ;//htonl(ISIL_FRAME_HEAD_FLAG);
    isil_frame_head.nHeadSize  = sizeof(sISIL_FRAME_HEAD) ;//htons(sizeof(sISIL_FRAME_HEAD));
    isil_frame_head.lFrameSize = frame_size; //htonl(frame_size);
    isil_frame_head.lTimeStamp = av_header->pts ; //htonl(frame_header->timeStamp);
    isil_frame_head.lEncodeType = ISIL_PENCODE_MJPEG;

    frame_size += sizeof(sISIL_FRAME_HEAD);
    first_header_len += sizeof(sISIL_FRAME_HEAD);


    packetCout  = (frame_size /NETVALIADSIZE)+1;
    Net_packet_head.lFrameSize = frame_size;//htonl(frame_size);
    tmpShort = packetNum++;
    Net_packet_head.nPacketNum = tmpShort;//htons(packetNum++);

    Net_packet_head.lFrameID   = av_header->frm_seq;//htonl(frame_header->frameSerial);

    Net_packet_head.nPacketCount = packetCout; //htons(packetCout);


    Net_packet_head.xInfo.cFlag    = sizeof(sNet_packet_head);
    Net_packet_head.xInfo.cFrameType = ISIL_FRAME_TYPE_PIC;



    iovSend[0].iov_base = (char*)&Net_packet_head;
    iovSend[0].iov_len  =  sizeof(sNet_packet_head);
    iovSend[1].iov_base = (char*)&isil_frame_head;
    iovSend[1].iov_len  = sizeof(sISIL_FRAME_HEAD);


    iovSend[2].iov_base = (char*)&isil_nal_head;
    iovSend[2].iov_len  =  sizeof(comISIL_NAL_HEAD);

    iovSend[3].iov_base = (char *)buff_ptr;
    if(frame_size > NETVALIADSIZE) {
        iovSend[3].iov_len = NETVALIADSIZE - first_header_len;

        Net_packet_head.nDataLen = NETVALIADSIZE;
        left_len = frame_size - NETVALIADSIZE;

    }
    else{

        iovSend[3].iov_len = frame_size - first_header_len;

        Net_packet_head.nDataLen = frame_size;
        left_len           = 0;

    }

    net_swap_net_packet_head(&Net_packet_head);
    net_swap_isil_frame_head(&isil_frame_head);

    if(isil_writeiovall(fd,iovSend,4, net_addr) < 0){
        perror("Net Send Data failed :");
        return -1;
    }

    buff_ptr += iovSend[3].iov_len;

    while(left_len > 0) {
        iovSend[1].iov_base =  (char*)buff_ptr;
        iovSend[1].iov_len =  ( (left_len <= NETVALIADSIZE)? left_len:NETVALIADSIZE);

        tmpShort = packetNum++;
        Net_packet_head.nPacketNum = bswap16(tmpShort);
        tmpShort = iovSend[1].iov_len;
        Net_packet_head.nDataLen = bswap16(tmpShort);

        if(isil_writeiovall(fd,iovSend,2, net_addr) < 0){
            perror("Net Send Data failed :");
            return -1;
        }
        left_len -= iovSend[1].iov_len ;
        buff_ptr += iovSend[1].iov_len ;

    }

    return 0;

}


int net_handle_5864_I_P_frame(int fd ,
                                                void* net_addr,
                                                ISIL_AV_PACKET *av_header,
                                                 int realorpb,
                                                short nErrFlag )
{

    int frame_size = 0;
    int packetNum = 0;
    int left_len = 0;
    int first_header_len = 0;
    int packetCout;
    char *buff_ptr = NULL;

    unsigned short tmpShort;

    struct iovec  iovSend[IOVMAX];

    sISIL_FRAME_HEAD isil_frame_head;

    comISIL_NAL_HEAD isil_nal_head;
    sNet_packet_head Net_packet_head;

    H264_INF_T *h264_inf_ptr;

    memset(&isil_frame_head, 0x00, sizeof(sISIL_FRAME_HEAD));
    memset(&Net_packet_head, 0x00, sizeof(sNet_packet_head));

    buff_ptr = (char *)av_header->buff;

    frame_size += av_header->date_len;

    isil_nal_head.cNalType = ISIL_NAL_SLICE_P;//htonl(ISIL_NAL_SLICE_P);
    isil_nal_head.lNalSize = frame_size ; //htonl(frame_size);

    net_swap_isil_nal_head(&isil_nal_head);

    frame_size += sizeof(comISIL_NAL_HEAD);
    first_header_len += sizeof(comISIL_NAL_HEAD);

    h264_inf_ptr = &(av_header->data_type_u.h264_inf);

    if(h264_inf_ptr->stream_type == ISIL_MAIN_STREAM_E) {
            isil_frame_head.nFrameType = ISIL_FRAME_TYPE_MAIN_P;//htons(ISIL_FRAME_TYPE_MAIN_I);
    }
    else{
            isil_frame_head.nFrameType = ISIL_FRAME_TYPE_SUB_P;
    }


    isil_frame_head.lFrameFlag = ISIL_FRAME_HEAD_FLAG ;//htonl(ISIL_FRAME_HEAD_FLAG);
    isil_frame_head.nHeadSize  = sizeof(sISIL_FRAME_HEAD) ;//htons(sizeof(sISIL_FRAME_HEAD));
    isil_frame_head.lFrameSize = frame_size; //htonl(frame_size);
    isil_frame_head.lTimeStamp = av_header->pts ; //htonl(frame_header->timeStamp);

    isil_frame_head.lEncodeType = ISIL_VENCODE_H264;

    frame_size += sizeof(sISIL_FRAME_HEAD);
    first_header_len += sizeof(sISIL_FRAME_HEAD);

    packetCout  = (frame_size /NETVALIADSIZE)+1;
    Net_packet_head.lFrameSize = frame_size;//htonl(frame_size);
    tmpShort = packetNum++;
    Net_packet_head.nPacketNum = tmpShort;//htons(packetNum++);

    Net_packet_head.lFrameID   = av_header->frm_seq;//htonl(frame_header->frameSerial);

    Net_packet_head.nPacketCount = packetCout; //htons(packetCout);

    if(realorpb == REALSTREAM){

         Net_packet_head.xInfo.cFlag    = sizeof(sNet_packet_head);

        Net_packet_head.xInfo.cFrameType = isil_frame_head.nFrameType;//htons(ISIL_FRAME_TYPE_MAIN_I);


     }
     else if ( realorpb == FILESTREAM) {

            Net_packet_head.xFlag.nType  =      bswap16(FRAMEDATA);
             Net_packet_head.xFlag.nErrFlag = bswap16(nErrFlag);
             if( nErrFlag ==  -1188)
                 isil_frame_head.lReserve = 0;
             else
                 isil_frame_head.lReserve  =bswap32(1);
     }
     else
         return -1;

     iovSend[0].iov_base = (char*)&Net_packet_head;
     iovSend[0].iov_len  =  sizeof(sNet_packet_head);
     iovSend[1].iov_base = (char*)&isil_frame_head;
     iovSend[1].iov_len  = sizeof(sISIL_FRAME_HEAD);
     iovSend[2].iov_base = (char*)&isil_nal_head;
     iovSend[2].iov_len  =  sizeof(comISIL_NAL_HEAD);
     iovSend[3].iov_base = (char *)buff_ptr;
     if(frame_size > NETVALIADSIZE) {
         iovSend[3].iov_len = NETVALIADSIZE - first_header_len;

         Net_packet_head.nDataLen = NETVALIADSIZE;
         left_len = frame_size - NETVALIADSIZE;

     }
    else{

        iovSend[3].iov_len = frame_size - first_header_len;

        Net_packet_head.nDataLen = frame_size;
        left_len           = 0;

    }

    net_swap_net_packet_head(&Net_packet_head);
    net_swap_isil_frame_head(&isil_frame_head);

    if(isil_writeiovall(fd,iovSend,4, net_addr) < 0){
        perror("Net Send Data failed :");
        return -1;
    }


    buff_ptr += iovSend[3].iov_len;


    while(left_len > 0) {

        iovSend[1].iov_base =  (char*)buff_ptr;
        iovSend[1].iov_len =  ( (left_len <= NETVALIADSIZE)? left_len:NETVALIADSIZE);

        tmpShort = packetNum++;
        Net_packet_head.nPacketNum = bswap16(tmpShort);
        tmpShort = iovSend[1].iov_len;
        Net_packet_head.nDataLen = bswap16(tmpShort);

        if(isil_writeiovall(fd,iovSend,2, net_addr) < 0){
            perror("Net Send Data failed :");
            return -1;
        }

        left_len -= iovSend[1].iov_len ;
        buff_ptr += iovSend[1].iov_len ;

    }

    return 0;

}

