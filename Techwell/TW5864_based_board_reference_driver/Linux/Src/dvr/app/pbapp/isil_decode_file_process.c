/*
 * isil_decode_file_process.c
 *
 *  Created on: 2011-6-30
 *      Author: zoucx
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#include "isil_decode_file_process.h"
#include "net_thread_pool.h"
#include "isil_msg_mgt.h"
#include "isil_pb_mgt.h"
#include "isil_decode_api.h"
#include "isil_decoder.h"
#include "isil_data_stream.h"
#include "isil_net_data_header.h"
#include "isil_io_utils.h"



#define PBSLEEPTIME (5000)

int thr_handle_dec_msg(single_pb_mgt *s_pb_mgt)
{

    int ret;
    struct ISIL_MSG_MGT *msg_mgt;
    struct ISIL_MSG_BLK  *msg_node;

    msg_mgt = s_pb_mgt->msg_list;

    if( !msg_mgt ) {
        return 0;
    }

    msg_node = msg_mgt->get_blk_from_head(msg_mgt);
    if(!msg_node) {
        return 0;
    }
    
    ret = parse_dec_msg(s_pb_mgt,msg_node);
    
#if 0
    if(msg_node->release) {
        msg_node->release(msg_node);
    }
#endif

    return ret;


}


int handle_dec_video_date(single_pb_mgt *s_pb_mgt)
{

    int ret;

    struct NET_DATA_NODE_T *data_node;
    ISIL_AV_PACKET *pck;

    if(!s_pb_mgt->video_list) {
        usleep(PBSLEEPTIME);
        return 0;
    }

    data_node = get_v_data_node_pb_mgt_vlist(s_pb_mgt);
    if( !data_node ) {
        usleep(PBSLEEPTIME);
        return 0;
    }

    if( !data_node->arg ) {
        usleep(PBSLEEPTIME);
        if(data_node->release) {
            data_node->release(data_node);
        }
        return -1;
    }

    pck = (ISIL_AV_PACKET *)data_node->arg;

    if(!pck->buff || !pck->date_len) {
        usleep(PBSLEEPTIME);
        if(data_node->release) {
            data_node->release(data_node);
        }
        return -1;
    }

    //fprintf(stderr, "-----write pck, buff[%d],data[%d]----\n", pck->buff_size, pck->date_len);
    ret = r_write(s_pb_mgt->video_fd ,(char *)pck->buff ,pck->date_len);
    if( ret < 0) {
        fprintf( stderr,"Write video  to drive err .\n");
        if(data_node->release) {
            data_node->release(data_node);
        }
        return -1;
    }

    fprintf(stderr, "-----write vts: %x, len: %d-------\n", pck->pts, pck->date_len);
    //fprintf(stderr, "-----write fd[%d] ok,[%d],[%d],[%d]----\n", s_pb_mgt->video_fd, pck->buff_size, pck->date_len, ret);

    if(data_node->release) {
        data_node->release(data_node);
    }

    return 0;
}


int handle_dec_audio_date(single_pb_mgt *s_pb_mgt)
{

    int ret;

    struct NET_DATA_NODE_T *data_node;
    ISIL_AV_PACKET *pck;

    if(!s_pb_mgt->audio_list) {
        usleep(PBSLEEPTIME);
        return 0;
    }

    data_node = get_a_data_node_pb_mgt_alist(s_pb_mgt);
    if( !data_node ) {
        usleep(PBSLEEPTIME);
        return 0;
    }

    if( !data_node->arg ) {
        usleep(PBSLEEPTIME);
        if(data_node->release) {
            data_node->release(data_node);
        }
        return -1;
    }

    pck = (ISIL_AV_PACKET *)data_node->arg;

    if(!pck->buff || !pck->date_len) {
        usleep(PBSLEEPTIME);
        if(data_node->release) {
            data_node->release(data_node);
        }
        return -1;
    }

    ret = r_write(s_pb_mgt->audio_fd ,(char *)pck->buff ,pck->date_len);
    if( ret < 0) {
        fprintf( stderr,"Write audio to drive err .\n");
        if(data_node->release) {
            data_node->release(data_node);
        }
        return -1;
    }

    fprintf(stderr, "-----write ats: %x, len: %d-------\n", pck->pts, pck->date_len);
    if(data_node->release) {
        data_node->release(data_node);
    }

    return 0;
}




#define DEC_SELECT_TIMEOUT (10000)


#include "isil_hndl_track_mode.h"
#include <time.h>
#include <sys/time.h>
static unsigned long ISIL_GetTimeStamp(void)
{
    //unsigned long tmp = 0;
    struct timeval cur;
    memset(&cur, 0x00, sizeof(struct timeval));
    gettimeofday(&cur, NULL);

	return((cur.tv_sec * 1000000 + cur.tv_usec)/1000);
}

void file_decode_thread(void * arg)
{
  int ret,maxFd ,ch;
  struct timeval tv;
  fd_set writeset;
  fd_set *pWriteset = NULL;

  single_pb_mgt *s_pb_mgt;
//  VDEC_CH_HEADER vdec_ch_header_e; 

  s_pb_mgt = (single_pb_mgt *)arg;
  assert(s_pb_mgt);

  /* Create audio or video chan*/

  ret = create_video_dec_chan(s_pb_mgt);
  if(ret < 0) {
      fprintf( stderr,"create_video_dec_chan err .\n");
      goto DEC_FILE_THR_EXIT;
  }

  ret = create_audio_dec_chan(s_pb_mgt);
  if(ret < 0) {
	  fprintf( stderr,"create_audio_dec_chan err .\n");
	  goto DEC_FILE_THR_EXIT;
  }
  s_pb_mgt->read_av_file = dec_normal_read_file;
  s_pb_mgt->need_read_file = 1;
  s_pb_mgt->is_pb_thr_run = 1;
  //DEBUG_FUNCTION();

  while(s_pb_mgt->is_pb_thr_run ){

      ret = thr_handle_dec_msg(s_pb_mgt);
      if(ret < 0) {
          fprintf(stderr,"thr_handle_dec_msg err .\n");
          break;
      }


      if(GET_S_PB_READ_FILE_VALUE(s_pb_mgt)) {
            if(s_pb_mgt->read_av_file && !s_pb_mgt->wait) {
                ret = s_pb_mgt->read_av_file(s_pb_mgt,s_pb_mgt->arg);
                if(ret < 0) {
                    fprintf(stderr,"dec_read_file err .\n");
                    break;
                }
            }else{
                //fprintf(stderr, "---ch[%x], wait driver,not read file--\n", s_pb_mgt->video_chan_num);
            }
      }

RETRY:
      //fprintf(stderr, " file_decode_thread 33333\n");
      maxFd = 0;
      pWriteset = NULL;
      FD_ZERO(&writeset);

      if(!s_pb_mgt->stop_dec) {
            
        if(s_pb_mgt->have_audio && s_pb_mgt->is_audio_chan_open) {

            FD_SET(s_pb_mgt->audio_fd, &writeset);
            maxFd = s_pb_mgt->audio_fd;
            pWriteset = &writeset;
        }
    
    
        if(s_pb_mgt->have_video && s_pb_mgt->is_video_chan_open) {
            
            FD_SET(s_pb_mgt->video_fd, &writeset);
            maxFd = ((maxFd > s_pb_mgt->video_fd) ?maxFd : s_pb_mgt->video_fd);
            pWriteset = &writeset;
            
        }
      }


	  //fprintf(stderr, "----s_pb_mgt->video_fd: %d, %d, %d, %d-----\n", s_pb_mgt->video_fd, maxFd,
	  //		  s_pb_mgt->stop_dec, s_pb_mgt->have_video);

      tv.tv_sec = 0;
      tv.tv_usec = DEC_SELECT_TIMEOUT;
      ret = select(maxFd+1,NULL,pWriteset,NULL,&tv);
      if( ret == 0 ) {
        fprintf(stderr, "---select timeout--\n");
        goto RETRY;
        //continue;
      }
      else if(ret < 0) {
        perror("Select:");
        break;
      }

      if(FD_ISSET(s_pb_mgt->video_fd, &writeset)) {
    	fprintf(stderr, "---before write v, %lu--\n", ISIL_GetTimeStamp());
        if(handle_dec_video_date(s_pb_mgt) < 0){
            fprintf(stderr,"handle_dec_video_date err .\n");
            break;
        }
      }

      if(FD_ISSET(s_pb_mgt->audio_fd, &writeset)) {
    	  	  fprintf(stderr, "---before write a, %lu--\n", ISIL_GetTimeStamp());

        if(handle_dec_audio_date(s_pb_mgt) < 0){
            fprintf(stderr,"handle_dec_video_date .\n");
            break;
        }
      }

      fprintf(stderr, "---after write, %lu--\n", ISIL_GetTimeStamp());

  }

DEC_FILE_THR_EXIT:
    
  DEBUG_FUNCTION();
  if(s_pb_mgt->release) {
      s_pb_mgt->release(s_pb_mgt);
  }

}




