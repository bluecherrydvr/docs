#include <errno.h>
#include "isil_codec_debug.h"
#include "isil_encoder.h"
#include "isil_private_func.h"

#ifndef MMAP_PAGE_SIZE
#define MMAP_PAGE_SIZE (4096)//page size 4k
#endif


static unsigned int calc_mmap_len(unsigned int val, unsigned int base)
{
    unsigned int fac = 0;
    if(val%base) {
        fac = 1;
    }else{
        fac = 0;
    }

    return((val/base)*base + fac*base);
}

//#define VERIFY_MAP_DATA

#ifdef VERIFY_MAP_DATA
int map_fd = 0;
int usr_fd = 0;
int send_fd = 0;
#endif

int ISIL_ENC_GetDataByRead(CODEC_HANDLE* pHdl, CODEC_STREAM_DATA *stpStream)
{
    int ret = 0;

    ret = read(*pHdl, (void *)stpStream->pUsrBuf, stpStream->u32UsrBufLen);
    if(ret < 0){
        return CODEC_ERR_FAIL;
    }

    stpStream->u32ActiveLen = ret;
    stpStream->u32FrameCnt = 1;

    return CODEC_ERR_OK;
}


int ISIL_ENC_GetDataBymmap(CODEC_HANDLE* pHdl, char** pMmapAddr, CODEC_STREAM_DATA *stpStream)
{

#if 0
    int ch_fd = 0;
    int ret = 0;
    struct tc_buffer tb;
    int len = 0;
    unsigned int framecnt = 0;
    unsigned int mmap_len = 0;
    char *mmap_addr = NULL;
    char *ret_addr = NULL;
    int retrycnt = 0;
    char *src = NULL;
    char *dst = NULL;
    unsigned int active_len = 0;

    //check paraments effectiveness
    if(!pHdl || !stpStream || !stpStream->pUsrBuf) {
         CODEC_DEBUG(" input is invalid ");
         return CODEC_ERR_INVAL;
    }

    ch_fd = *pHdl;

    stpStream->u32ActiveLen = 0;
    stpStream->u32FrameCnt = 0;

    memset(&tb, 0x00, sizeof(struct tc_buffer));
    memset(stpStream->pUsrBuf, 0x00, stpStream->u32UsrBufLen);


    #if 0
    struct timeval tv ;
    fd_set readset;

    //require data to driver
    FD_ZERO(&readset);
    FD_SET(ch_fd, &readset);

    tv.tv_sec = 0;
    tv.tv_usec = 40000;

    //check if have data
    ret = select(ch_fd+1, &readset, NULL, NULL, &tv);
    if(ret < 0) {//fail
        CODEC_DEBUG(" select fail ");
        return CODEC_ERR_INVAL;
    }else if(ret == 0){//time out
        //CODEC_DEBUG(" select time out");
        return CODEC_ERR_OK;
    }else{//ok
        //CODEC_DEBUG(" select ok ");
    }

    if(!FD_ISSET(ch_fd, &readset)) {
        //CODEC_DEBUG(" no data ");
        return CODEC_ERR_OK;
    }
    #endif

    //data is ready
    tb.__tc__size = stpStream->u32UsrBufLen;

RETRY:
    ret = ioctl(ch_fd, ISIL_CODEC_CHAN_REQUEST_MEM, &tb);
    if(ret < 0){
        if(errno == ENOMEM) {
            stpStream->u32ActiveLen = tb.__tc__size;
            return CODEC_ERR_FAIL;
        }else if(errno == EAGAIN){
            CODEC_DEBUG(" isil_CODEC_CHAN_REQUEST_MEM again ");
            usleep(1000);
            retrycnt++;
            if(retrycnt >= 3) {
                stpStream->u32ActiveLen = 0;
                return CODEC_ERR_FAIL;
            }
            goto RETRY;
        }else{
            CODEC_DEBUG(" ioctl fail ");
            perror("fail ");
            stpStream->u32ActiveLen = 0;
            return CODEC_ERR_FAIL;
        }
    }


    //CODEC_DEBUG(" request mem ok ");

    //parsing data
    mmap_len = calc_mmap_len(tb.__tc__size, MMAP_PAGE_SIZE);
    //fprintf(stderr, "--mmap_addr: %p, mmap_len: %d, tb->size: %d, offset: %d---\n", *pMmapAddr, mmap_len, tb.size, tb.off_to_page);

    mmap_addr = *pMmapAddr;
    //mmap
    ret_addr = (char *)mmap((void *)mmap_addr, mmap_len, PROT_READ|PROT_WRITE, MAP_SHARED, ch_fd, 0);
    if(ret_addr == MAP_FAILED){
        CODEC_DEBUG(" mmap fail ");
        perror("fail ");
        ioctl(ch_fd, ISIL_CODEC_CHAN_RELEASE_MEM, tb);
        return CODEC_ERR_FAIL;
    }

    //fprintf(stderr, " ret_addr: %p, nr: %d\n", ret_addr, tb.nr_frame);

    *pMmapAddr = ret_addr;
    stpStream->u32FrameCnt = tb.__tc__nr_frame;
    //stpStream->u32ActiveLen = tb.size;
    dst = stpStream->pUsrBuf;
    src = ret_addr;// + tb.off_to_page;

 #if 1
    while(framecnt < stpStream->u32FrameCnt) {
        framecnt++;
        //tc buffer
        struct tc_buffer *tb_tmp = (struct tc_buffer *)src;
        //fprintf(stderr, "----tb addr: %p,src: %p---\n", tb_tmp, src);

        src = tb_tmp->tc_frame_head;

        //fprintf(stderr, "----frame head addr: %p---\n", src);
        //copy frame header
        struct isil_frame_header* driver_frame_header = (struct isil_frame_header *)src;
        struct isil_frame_header* frame_header_tmp = (struct isil_frame_header *)dst;

      	//fprintf(stderr, "codec type: %d, frame type: %d, offset: %d, len: %d\n", driver_frame_header->codecType,
         //                                   driver_frame_header->frameType,
          //                                  driver_frame_header->payload_offset,
           //                                 driver_frame_header->payloadLen);

        //fprintf(stderr, "---FN[%d]---\n", driver_frame_header->frameSerial);

        len = sizeof(struct isil_frame_header);
        memcpy(dst, src, len);
        dst += len;
        src += len;
        active_len += len;

        frame_header_tmp->payload_offset = sizeof(struct isil_frame_header);

        #ifdef VERIFY_MAP_DATA
         if(!map_fd){
             map_fd = open("/dev/shm/mmap.data", O_RDWR|O_APPEND|O_CREAT);
         }

         if(!usr_fd){
             usr_fd = open("/dev/shm/usr.data", O_RDWR|O_APPEND|O_CREAT);
         }
        #endif

        switch(driver_frame_header->frameType){
        case H264_FRAME_TYPE_IDR:
            //fprintf(stderr, "-------idr, src: %p, dst: %p-------\n", src, dst);
            //idr_pad write(map_fd, idr_addr, len);
            len = sizeof(struct isil_h264_idr_frame_pad);
            struct isil_h264_idr_frame_pad * idr_pad_dst = (struct isil_h264_idr_frame_pad *)dst;
            struct isil_h264_idr_frame_pad * idr_pad_src = (struct isil_h264_idr_frame_pad *)src;

            //fprintf(stderr, "--src sps offset: %d, len: %d\n", idr_pad_src->sps_frame_offset, idr_pad_src->sps_frame_size);
            //fprintf(stderr, "--src pps offset: %d, len: %d\n", idr_pad_src->pps_frame_offset, idr_pad_src->pps_frame_size);
            //fprintf(stderr, "--src idr offset: %d, len: %d\n", idr_pad_src->idr_frame_offset, idr_pad_src->idr_frame_size);

            //fprintf(stderr, "-------copy before, src: %p, dst: %p, len: %d-------\n", src, dst, len);
            memcpy(idr_pad_dst, idr_pad_src, len);
            //fprintf(stderr, "-------copy after, src: %p, dst: %p,len: %d-------\n", src, dst, len);

            idr_pad_dst->sps_frame_offset = len;
            idr_pad_dst->pps_frame_offset = idr_pad_dst->sps_frame_offset + idr_pad_src->sps_frame_size;
            idr_pad_dst->idr_frame_offset = idr_pad_dst->pps_frame_offset + idr_pad_src->pps_frame_size;
            dst += len;
            src += len;
            active_len += len;

            //fprintf(stderr, "--dst sps offset: %d, len: %d\n", idr_pad_dst->sps_frame_offset, idr_pad_dst->sps_frame_size);
            //fprintf(stderr, "--dst pps offset: %d, len: %d\n", idr_pad_dst->pps_frame_offset, idr_pad_dst->pps_frame_size);
            //fprintf(stderr, "--dst idr offset: %d, len: %d\n", idr_pad_dst->idr_frame_offset, idr_pad_dst->idr_frame_size);

            //copy sps nal
            len = idr_pad_src->sps_frame_size;
            char *sps_addr = src - sizeof(struct isil_h264_idr_frame_pad) + idr_pad_src->sps_frame_offset;
            //fprintf(stderr, "----sps addr: %p, %d, %p---\n", sps_addr, len, dst);
            memcpy(dst, sps_addr, len);

            #ifdef VERIFY_MAP_DATA
            write(map_fd, sps_addr, len);
            write(usr_fd, dst, len);
            #endif

            dst += len;
            active_len += len;

            //copy pps nal
            len = idr_pad_src->pps_frame_size;
            char *pps_addr = src - sizeof(struct isil_h264_idr_frame_pad) + idr_pad_src->pps_frame_offset;
            //fprintf(stderr, "----pps addr: %p, %d, %p---\n", pps_addr, len, dst);
            memcpy(dst, pps_addr, len);


            #ifdef VERIFY_MAP_DATA
            write(map_fd, pps_addr, len);
            write(usr_fd, dst, len);
            #endif


            dst += len;
            active_len += len;

            //copy idr nal
            len = idr_pad_src->idr_frame_size;
            char *idr_addr = src - sizeof(struct isil_h264_idr_frame_pad) + idr_pad_src->idr_frame_offset;
            //fprintf(stderr, "----idr addr: %p, %d, %p---\n", idr_addr, len, dst);
            memcpy(dst, idr_addr, len);

            #ifdef VERIFY_MAP_DATA
            write(map_fd, idr_addr, len);
            write(usr_fd, dst, len);
            #endif


            dst += len;
            active_len += len;

            //fprintf(stderr, "----000 src: %p----\n", src);
            if((unsigned int)src%4){
            	src = src +4 - ((unsigned int)src)%4;//4 bytes align
          	}
            //fprintf(stderr, "---- 111 src: %p----\n", src);
            break;

        case H264_MV_FRAME_TYPE:
            fprintf(stderr, "----mv frame, not process now----\n");
            break;

        case AUDIO_FRAME_TYPE:
            //fprintf(stderr, "-------4444-------\n");
            //audio pad
            len = sizeof(struct isil_audio_frame_pad);
            struct isil_audio_frame_pad * aud_pad_dst = (struct isil_audio_frame_pad *)dst;
            struct isil_audio_frame_pad * aud_pad_src = (struct isil_audio_frame_pad *)src;
            memcpy(aud_pad_dst, aud_pad_src, len);

            aud_pad_dst->frame_offset = len;
            dst += len;
            src += len;
            active_len += len;
            //copy audio nal
            len = aud_pad_src->frame_size;
            char *aud_addr = src - sizeof(struct isil_audio_frame_pad) + aud_pad_src->frame_offset;
            memcpy(dst, aud_addr, len);

            #ifdef VERIFY_MAP_DATA
            write(map_fd, aud_addr+6, len-6);
            #endif

            dst += len;
            active_len += len;
            if((unsigned int)src%4){
            	src = src +4 - (unsigned int)src%4;//4 bytes align
          	}
            break;

        case H264_FRAME_TYPE_I:
        case H264_FRAME_TYPE_P:
        case H264_FRAME_TYPE_B:
        case H264_FRAME_TYPE_SI:
        case H264_FRAME_TYPE_SP:
        case MJPEG_FRAME_TYPE:
            //fprintf(stderr, "-----p, src: %p, dst: %p-------\n", src, dst);
            len = driver_frame_header->payloadLen;
            char *Nal = src - sizeof(struct isil_frame_header) + driver_frame_header->payload_offset;

            //fprintf(stderr, "--nal addr: %p, %d, %p---\n", Nal, len, dst);
            memcpy(dst, Nal, len);

            #ifdef VERIFY_MAP_DATA
            write(map_fd, Nal, len);
            write(usr_fd, dst, len);
            #endif


            dst += len;
            active_len += len;

            //fprintf(stderr, "-----len:%d,offset:%d,dst:%p------\n", driver_frame_header->payloadLen, driver_frame_header->payload_offset, dst);

            //fprintf(stderr, "---- 333 src: %p----\n", src);

            if((unsigned int)src%4){
            	src = src +4 - ((unsigned int)src)%4;//4 bytes align
          	}
            //fprintf(stderr, "---- 444 src: %p----\n", src);
            break;

        default:
            CODEC_DEBUG(" frame type error \n");
            break;
        }
    }
 #endif

    stpStream->u32ActiveLen = active_len;

    if(munmap(ret_addr, calc_mmap_len(tb.__tc__size, MMAP_PAGE_SIZE)) < 0){
        CODEC_DEBUG(" ioctl release mem fail ");
        return CODEC_ERR_FAIL;
    }


    if(ioctl(ch_fd, ISIL_CODEC_CHAN_RELEASE_MEM, tb) < 0){
        CODEC_DEBUG(" ioctl release mem fail ");
        return CODEC_ERR_FAIL;
    }


    #ifdef VERIFY_MAP_DATA
    if(!send_fd){
             send_fd = open("/dev/shm/send.data", O_RDWR|O_APPEND|O_CREAT);
    }

    write(send_fd, stpStream->pUsrBuf, stpStream->u32ActiveLen);
    #endif
#endif
    return CODEC_ERR_OK;
}




