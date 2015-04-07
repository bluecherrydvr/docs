#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


#include "isil_hndl_track_mode.h"

#include "isil_fm_play.h"
#include "isil_av_packet_ex.h"
#include "isil_net_data_header.h"
#include "frame_parse_to_drv_msg.h"
#include "isil_interface.h"


#define DECFRAMEADDHEADERSIZE  (200)

static int send_av_packet_to_pb_mgt_v_list(single_pb_mgt * s_pb_mgt , ISIL_AV_PACKET *pck)
{

    struct NET_DATA_MGT_T *v_list;
    struct NET_DATA_NODE_T *data_node;
    if( !s_pb_mgt || !pck ) {
        return -1;
    }

    v_list = s_pb_mgt->video_list;
    if( !v_list ) {
        return -1;
    }

    data_node = put_av_packet_to_net_data_node(pck);
    if( !data_node ) {
        return -1;
    }

    v_list->put_net_data_node(v_list,data_node);
     
    return 0;
}


static int send_av_packet_to_pb_mgt_a_list(single_pb_mgt * s_pb_mgt , ISIL_AV_PACKET *pck)
{

    struct NET_DATA_MGT_T *a_list;
    struct NET_DATA_NODE_T *data_node;
    if( !s_pb_mgt || !pck ) {
        return -1;
    }

    a_list = s_pb_mgt->audio_list;
    if( !a_list ) {
        return -1;
    }

    data_node = put_av_packet_to_net_data_node(pck);
    if( !data_node ) {
        return -1;
    }

    a_list->put_net_data_node(a_list,data_node);
     
    return 0;
}


static int dec_back_get_frame_for_starttime(single_pb_mgt * s_pb_mgt ,void * arg)
{
    ISIL_AV_PACKET *pck = NULL;
    int ReadFileRet = 0 ,len = 0;
    int ret;
    PB_FRAME_INFO  pb_frame_inf_t;
    isil_h264_decode_bit_info_msg_t dspt_t;


    int fd = get_s_pb_mgt_cur_file_fd(s_pb_mgt);
RETRY:

    ReadFileRet = ISIL_FM_PLAY_ParsingFile(fd,&pb_frame_inf_t);
    if(ReadFileRet < 0) {
        fprintf(stderr,"ISIL_FM_PLAY_ParsingFile failed .\n");
        return -1;
    }

    if(!pb_frame_inf_t.nalcnt || !pb_frame_inf_t.len){
        fprintf(stderr,"Get idr frame err .\n");
        return -1;
    }

    /* todo */
    if((pb_frame_inf_t.nal[0].naltype > E_FRAME_TYPE_AUDIO)
            &&(pb_frame_inf_t.nal[0].naltype < E_FRAME_TYPE_AUDIO_END)){
        	fprintf(stderr, "now discard audio frame\n");
            goto RETRY;

    }

	if((pb_frame_inf_t.nal[0].naltype > E_FRAME_TYPE_MJPG)
			&&(pb_frame_inf_t.nal[0].naltype < E_FRAME_TYPE_MJPG_END)){
			fprintf(stderr, "now discard mjpg frame\n");
			goto RETRY;

	}

#if 0
    if( pb_frame_inf_t.ts < GET_S_PB_END_TS(s_pb_mgt) ){

        fprintf(stderr,"Get ts[%u] frame err .\n",GET_S_PB_END_TS(s_pb_mgt));
        return -1;
    }
#endif

    len = pb_frame_inf_t.len + DECFRAMEADDHEADERSIZE;

    pck = alloc_av_packet_ex(len);
    if( !pck ) {
        return -1;
    }

    dspt_t.msg_type = ISIL_H264_DECODE_BIT_INFO_MSG;
    dspt_t.msg_len  = sizeof(isil_h264_decode_bit_info_msg_t);

    if(GET_S_PB_CUR_MODE(s_pb_mgt) == FRAMEBACK_PB_MODE) {
        dspt_t.bit_info |= BI_IS_DISPLAY(1);
        
    }
    else{
        dspt_t.bit_info |= BI_IS_DISPLAY(0);
    }

    dspt_t.bit_info |= BI_IS_END(0);
    dspt_t.bit_info |= BI_IS_FILE_END(0);
    dspt_t.bit_info |= BI_IS_GOP_END(0);


    add_single_pb_mgt_v_count(s_pb_mgt);
    ret = decode_fill_h264_buff_header(pck->buff ,
                                       get_single_pb_mgt_v_count(s_pb_mgt),
                                       &pb_frame_inf_t,
                                       &dspt_t);
    if( ret < 0 ) {
        fprintf(stderr ,"decode_fill_h264_buff_header err .\n");
        release_av_packet_ex(pck);
        return -1;
    }
    if(ret > len) {
        fprintf( stderr,"Bug, len is small .\n");
        assert(0);
    }
    pck->date_len = ret;
    pck->pts = pb_frame_inf_t.ts;

    

    
    ret = send_av_packet_to_pb_mgt_v_list(s_pb_mgt,pck); 
    if(ret < 0 ) {
        release_av_packet_ex(pck);
        return -1;
    }

    s_pb_mgt->have_video = 1;

    
    s_pb_mgt->cur_pck = pck;

    set_single_pb_mgt_v_tsp(s_pb_mgt,pb_frame_inf_t.ts);
    return 0;
}


static int dec_back_start_get_idr_from_file(single_pb_mgt * s_pb_mgt )
{
    ISIL_AV_PACKET *pck = NULL;
    int ReadFileRet = 0 ,len = 0;
    int ret ;
    PB_FRAME_INFO  pb_frame_inf_t;
    isil_h264_decode_bit_info_msg_t dspt_t;

    int fd = get_s_pb_mgt_cur_file_fd(s_pb_mgt);

    ReadFileRet = ISIL_FM_PLAY_ParsingFile(fd,&pb_frame_inf_t);
    if(ReadFileRet < 0) {
        fprintf(stderr,"ISIL_FM_PLAY_ParsingFile failed .\n");
        return -1;
    }

    if(!pb_frame_inf_t.nalcnt || !pb_frame_inf_t.len){
        fprintf(stderr,"Get idr frame err .\n");
        return -1;
    }

    len = pb_frame_inf_t.len + DECFRAMEADDHEADERSIZE;
    pck = alloc_av_packet_ex(len);
    if( !pck ) {
        return -1;
    }

    if( pb_frame_inf_t.nalcnt != 3 ){
        fprintf(stderr,"Get idr frame err.\n");
        release_av_packet_ex(pck);
        return -1;
    }

    if(GET_S_PB_CUR_MODE(s_pb_mgt) == FRAMEBACK_PB_MODE){
        dspt_t.bit_info |= BI_IS_DISPLAY(0);
    }else{
        dspt_t.bit_info |= BI_IS_DISPLAY(1);
    }

    dspt_t.bit_info |= BI_IS_END(0);
    dspt_t.bit_info |= BI_IS_FILE_END(0);
    dspt_t.bit_info |= BI_IS_GOP_END(0);


    add_single_pb_mgt_v_count(s_pb_mgt);
    
    ret = decode_fill_h264_buff_header(pck->buff ,
                                       get_single_pb_mgt_v_count(s_pb_mgt),
                                       &pb_frame_inf_t,
                                       &dspt_t);
    if( ret < 0 ) {
        fprintf(stderr ,"decode_fill_h264_buff_header err .\n");
        release_av_packet_ex(pck);
        return -1;
    }
    if(ret > len) {
        fprintf( stderr,"Bug, len is small .\n");
        assert(0);
    }
    pck->date_len = ret;
    pck->pts = pb_frame_inf_t.ts;
    
    
    ret = send_av_packet_to_pb_mgt_v_list(s_pb_mgt,pck); 
    if(ret < 0 ) {
        release_av_packet_ex(pck);
        return -1;
    }    

    s_pb_mgt->cur_pck = pck;

    s_pb_mgt->have_video = 1;
   
    set_single_pb_mgt_v_tsp(s_pb_mgt,pb_frame_inf_t.ts);

    

    

    return 0;

}


/*find need frame*/

static int dec_find_after_this_idr(single_pb_mgt * s_pb_mgt)
{
    int ret ,fd;

    MSG_FM_REMOTE_OPERATOR fm_remote_operator;
    PB_FRAME_INFO  pb_frame_inf_t;


    if( !s_pb_mgt ) {
        return -1;
    }


    fd = get_s_pb_mgt_cur_file_fd(s_pb_mgt);

    fm_remote_operator.FM_PLAY_CTL.cDirection = 0;

    ISIL_FM_PLAY_SetDirection(fd,fm_remote_operator);
    fm_remote_operator.FM_PLAY_CTL.cKeyFrame = 1;
    ISIL_FM_PLAY_SetKeyFrame(fd,fm_remote_operator);

    ret = ISIL_FM_PLAY_ParsingFile(fd,&pb_frame_inf_t);
    if(ret < 0) {
        fprintf(stderr,"ISIL_FM_PLAY_ParsingFile failed .\n");
        return -1;
    }

    fm_remote_operator.FM_PLAY_CTL.cDirection = 1;
    ISIL_FM_PLAY_SetDirection(fd,fm_remote_operator);
    fm_remote_operator.FM_PLAY_CTL.cKeyFrame = 0;
    ISIL_FM_PLAY_SetKeyFrame(fd,fm_remote_operator);

    /*read p or i frame*/
    ret = dec_back_get_frame_for_starttime(s_pb_mgt ,s_pb_mgt->arg);
    if(ret < 0) {
        fprintf(stderr,"dec_back_get_frame_for_starttime failed .\n");
        return -1;
    }

    return 0;

}

static int dec_find_this_idr(single_pb_mgt * s_pb_mgt)
{
    int ret;
    ISIL_AV_PACKET *pck = NULL;
    int len = 0;
    isil_h264_decode_bit_info_msg_t dspt_t;

    MSG_FM_REMOTE_OPERATOR fm_remote_operator;
    PB_FRAME_INFO  pb_frame_inf_t;




    int fd = get_s_pb_mgt_cur_file_fd(s_pb_mgt);


    fm_remote_operator.FM_PLAY_CTL.cDirection = 0;
    ISIL_FM_PLAY_SetDirection(fd,fm_remote_operator);

    fm_remote_operator.FM_PLAY_CTL.cKeyFrame = 1;
    ISIL_FM_PLAY_SetKeyFrame(fd,fm_remote_operator);

    ret = dec_back_start_get_idr_from_file(s_pb_mgt);
    if(ret < 0) {
        fprintf(stderr,"dec_back_start_get_idr_from_file failed .\n");
        return -1;
    }



    fm_remote_operator.FM_PLAY_CTL.cDirection = 1;
    ISIL_FM_PLAY_SetDirection(fd,fm_remote_operator);
    fm_remote_operator.FM_PLAY_CTL.cKeyFrame = 0;
    ISIL_FM_PLAY_SetKeyFrame(fd,fm_remote_operator);

    return 0;

}


static int dec_find_prev_idr(single_pb_mgt * s_pb_mgt)
{
    int ret;
    ISIL_AV_PACKET *pck = NULL;
    int len = 0;
    isil_h264_decode_bit_info_msg_t dspt_t;

    MSG_FM_REMOTE_OPERATOR fm_remote_operator;
    PB_FRAME_INFO  pb_frame_inf_t;




    int fd = get_s_pb_mgt_cur_file_fd(s_pb_mgt);
    

    fm_remote_operator.FM_PLAY_CTL.cDirection = 0;


    ISIL_FM_PLAY_SetDirection(fd,fm_remote_operator);
    fm_remote_operator.FM_PLAY_CTL.cKeyFrame = 1;
    ISIL_FM_PLAY_SetKeyFrame(fd,fm_remote_operator);

    ret = ISIL_FM_PLAY_ParsingFile(fd,&pb_frame_inf_t);
    if(ret < 0) {
        fprintf(stderr,"ISIL_FM_PLAY_ParsingFile failed .\n");
        return -1;
    }

    ret = dec_back_start_get_idr_from_file(s_pb_mgt);
    if(ret < 0) {
        fprintf(stderr,"dec_back_start_get_idr_from_file failed .\n");
        return -1;
    }

    

    fm_remote_operator.FM_PLAY_CTL.cDirection = 1;
    ISIL_FM_PLAY_SetDirection(fd,fm_remote_operator);
    fm_remote_operator.FM_PLAY_CTL.cKeyFrame = 0;
    ISIL_FM_PLAY_SetKeyFrame(fd,fm_remote_operator);

    return 0;

}


static int dec_find_next_idr(single_pb_mgt * s_pb_mgt)
{
    int ret;

    ISIL_AV_PACKET *pck = NULL;
    int len = 0;
    isil_h264_decode_bit_info_msg_t dspt_t;
    MSG_FM_REMOTE_OPERATOR fm_remote_operator;
    PB_FRAME_INFO  pb_frame_inf_t;



    int fd = get_s_pb_mgt_cur_file_fd(s_pb_mgt);
    fm_remote_operator.FM_PLAY_CTL.cDirection = 1;


    ISIL_FM_PLAY_SetDirection(fd,fm_remote_operator);
    fm_remote_operator.FM_PLAY_CTL.cKeyFrame = 1;
    ISIL_FM_PLAY_SetKeyFrame(fd,fm_remote_operator);



    ret = dec_back_start_get_idr_from_file(s_pb_mgt);
    if(ret < 0) {
        fprintf(stderr,"dec_back_start_get_idr_from_file failed .\n");
        return -1;
    }


    fm_remote_operator.FM_PLAY_CTL.cKeyFrame = 0;
    ISIL_FM_PLAY_SetKeyFrame(fd,fm_remote_operator);

    return 0;

}


static int dec_find_read_starttime_frame(single_pb_mgt * s_pb_mgt ,unsigned int ts)
{
	int ret;
	MSG_FM_REMOTE_OPERATOR fm_remote_operator;
	PB_FRAME_INFO  pb_frame_inf_t;
	//assert(s_pb_mgt->arg);
	int fd = get_s_pb_mgt_cur_file_fd(s_pb_mgt);

	fm_remote_operator.FM_PLAY_CTL.cDirection = 1;
	ISIL_FM_PLAY_SetDirection(fd,fm_remote_operator);
	fm_remote_operator.FM_PLAY_CTL.cKeyFrame = 0;
	ISIL_FM_PLAY_SetKeyFrame(fd,fm_remote_operator);

	ret = ISIL_FM_PLAY_SetTS(fd,ts);
	if( ret < 0) {
		fprintf(stderr,"ISIL_FM_PLAY_SetTS failed .\n");
		return -1;
	}

AGAIN:
	ret = ISIL_FM_PLAY_ParsingFile(fd,&pb_frame_inf_t);
	if(ret < 0) {
		fprintf(stderr,"ISIL_FM_PLAY_ParsingFile failed .\n");
		return -1;
	}

	if(pb_frame_inf_t.ts != ts){
		fprintf(stderr," ts[%x] != required ts[%x] failed .\n", pb_frame_inf_t.ts, ts);
		goto AGAIN;
	}

	/* read p or i frame*/
	ret = dec_back_get_frame_for_starttime(s_pb_mgt,s_pb_mgt->arg);
	if(ret < 0) {
		fprintf(stderr,"dec_back_get_frame_for_starttime failed .\n");
		return -1;
	}
	return 0;
}


/*handle f_invalid=0x10*/
static int dec_back_get_frame_until_idr(single_pb_mgt * s_pb_mgt ,void * arg)
{
    ISIL_AV_PACKET *pck = NULL;
    int ReadFileRet = 0 ,len = 0 ;
    int ret;
    PB_FRAME_INFO  pb_frame_inf_t;
    isil_h264_decode_bit_info_msg_t dspt_t; 



    int fd = get_s_pb_mgt_cur_file_fd(s_pb_mgt);

RETRY:
    ReadFileRet = ISIL_FM_PLAY_ParsingFile(fd,&pb_frame_inf_t);
    if(ReadFileRet < 0) {
        fprintf(stderr,"ISIL_FM_PLAY_ParsingFile failed .\n");
        return -1;
    }

    if(!pb_frame_inf_t.nalcnt || !pb_frame_inf_t.len){
        fprintf(stderr,"Get idr frame err .\n");
        return -1;
    }

    if((pb_frame_inf_t.nal[0].naltype > E_FRAME_TYPE_AUDIO)
            &&(pb_frame_inf_t.nal[0].naltype < E_FRAME_TYPE_AUDIO_END)){
        	fprintf(stderr, "now discard audio frame\n");
            goto RETRY;

    }

	if((pb_frame_inf_t.nal[0].naltype > E_FRAME_TYPE_MJPG)
			&&(pb_frame_inf_t.nal[0].naltype < E_FRAME_TYPE_MJPG_END)){
			fprintf(stderr, "now discard mjpg frame\n");
			goto RETRY;

	}

    /*until idr frame*/
    if( pb_frame_inf_t.nalcnt == 3 ){

        fprintf(stderr,"this frame must discard .\n");
        if( s_pb_mgt->cur_pck ) {
            dspt_t.bit_info |= BI_IS_DISPLAY(1);
            dspt_t.bit_info |= BI_IS_END(1);
            dspt_t.bit_info |= BI_IS_FILE_END(0);
            dspt_t.bit_info |= BI_IS_GOP_END(1);
            decode_only_fill_h264_bitinfo(s_pb_mgt->cur_pck->buff, dspt_t.bit_info);
            s_pb_mgt->cur_pck = NULL;

        }

        /*No read and send frame*/
		//SET_S_PB_NO_READ_FILE( s_pb_mgt );

		return 0;
    }

    len = pb_frame_inf_t.len + DECFRAMEADDHEADERSIZE;
    pck = alloc_av_packet_ex(len);
    if( !pck ) {
        return -1;
    }

    if(GET_S_PB_CUR_MODE(s_pb_mgt) == FRAMEBACK_PB_MODE){
        dspt_t.bit_info |= BI_IS_DISPLAY(0);
    }else{
        dspt_t.bit_info |= BI_IS_DISPLAY(1);
    }


    dspt_t.bit_info |= BI_IS_END(0);
    dspt_t.bit_info |= BI_IS_FILE_END(0);
    dspt_t.bit_info |= BI_IS_GOP_END(0);

    
    add_single_pb_mgt_v_count(s_pb_mgt);

    ret = decode_fill_h264_buff_header(pck->buff ,
                                       get_single_pb_mgt_v_count(s_pb_mgt),
                                       &pb_frame_inf_t,
                                       &dspt_t);

    if( ret < 0 ) {
        fprintf(stderr ,"decode_fill_h264_buff_header err .\n");
        release_av_packet_ex(pck);
        return -1;
    }

    if(ret > len) {
        fprintf( stderr,"Bug, len is small .\n");
        assert(0);
    }

    pck->pts = pb_frame_inf_t.ts;
    pck->date_len = ret;

    ret = send_av_packet_to_pb_mgt_v_list(s_pb_mgt,pck); 
    if(ret < 0 ) {
        release_av_packet_ex(pck);
        return -1;
    }
    
    s_pb_mgt->cur_pck = pck;    
    
    s_pb_mgt->have_video = 1;

    set_single_pb_mgt_v_tsp(s_pb_mgt,pb_frame_inf_t.ts);


    return 0;
}

//get frame until ts, exclude end ts
static int dec_back_get_frame_until_ts_exclude_end_ts(single_pb_mgt * s_pb_mgt ,void * arg)
{
    ISIL_AV_PACKET *pck = NULL;
    int ReadFileRet = 0 ,len = 0;
    int ret ;
    PB_FRAME_INFO  pb_frame_inf_t;
    isil_h264_decode_bit_info_msg_t dspt_t;


    int fd = get_s_pb_mgt_cur_file_fd(s_pb_mgt);
RETRY:

    ReadFileRet = ISIL_FM_PLAY_ParsingFile(fd,&pb_frame_inf_t);
    if(ReadFileRet < 0) {
        fprintf(stderr,"ISIL_FM_PLAY_ParsingFile failed .\n");
        return -1;
    }

    if(!pb_frame_inf_t.nalcnt || !pb_frame_inf_t.len){
        fprintf(stderr,"Get idr frame err .\n");
        return -1;
    }

    /* todo */
    if((pb_frame_inf_t.nal[0].naltype > E_FRAME_TYPE_AUDIO)
            &&(pb_frame_inf_t.nal[0].naltype < E_FRAME_TYPE_AUDIO_END)){
        	fprintf(stderr, "now discard audio frame\n");
            goto RETRY;

    }

	if((pb_frame_inf_t.nal[0].naltype > E_FRAME_TYPE_MJPG)
			&&(pb_frame_inf_t.nal[0].naltype < E_FRAME_TYPE_MJPG_END)){
			fprintf(stderr, "now discard mjpg frame\n");
			goto RETRY;

	}

    if( pb_frame_inf_t.ts < GET_S_PB_END_TS(s_pb_mgt) ) {
		if(GET_S_PB_CUR_MODE(s_pb_mgt) == FRAMEBACK_PB_MODE){
			dspt_t.bit_info |= BI_IS_DISPLAY(0);
		}
		else{
			dspt_t.bit_info |= BI_IS_DISPLAY(1);
		}
		dspt_t.bit_info |= BI_IS_END(0);
		dspt_t.bit_info |= BI_IS_FILE_END(0);
		dspt_t.bit_info |= BI_IS_GOP_END(0);
	}else{
		 fprintf(stderr,"ok last frame discard.\n");
		 if( s_pb_mgt->cur_pck ) {
			dspt_t.bit_info |= BI_IS_DISPLAY(1);
			dspt_t.bit_info |= BI_IS_END(1);
			dspt_t.bit_info |= BI_IS_FILE_END(0);
			dspt_t.bit_info |= BI_IS_GOP_END(0);
			decode_only_fill_h264_bitinfo(s_pb_mgt->cur_pck->buff, dspt_t.bit_info);
			s_pb_mgt->cur_pck = NULL;
		}

		/*No read and send frame*/
		SET_S_PB_NO_READ_FILE( s_pb_mgt );
		return 0;
	}


    len = pb_frame_inf_t.len + DECFRAMEADDHEADERSIZE;

    pck = alloc_av_packet_ex(len);
    if( !pck ) {
        return -1;
    }

    add_single_pb_mgt_v_count(s_pb_mgt);
    ret = decode_fill_h264_buff_header(pck->buff ,
                                       get_single_pb_mgt_v_count(s_pb_mgt),
                                       &pb_frame_inf_t,
                                       &dspt_t);

    if( ret < 0 ) {
        fprintf(stderr ,"decode_fill_h264_buff_header err .\n");
        release_av_packet_ex(pck);
        return -1;
    }




    ret = send_av_packet_to_pb_mgt_v_list(s_pb_mgt,pck);
    if(ret < 0 ) {
        release_av_packet_ex(pck);
        return -1;
    }

    s_pb_mgt->cur_pck = pck;
    s_pb_mgt->have_video = 1;
    set_single_pb_mgt_v_tsp(s_pb_mgt,pb_frame_inf_t.ts);

    return 0;
}
/*handle f_invalid=0x10*/

static int dec_back_get_frame_until_ts(single_pb_mgt * s_pb_mgt ,void * arg)
{
    ISIL_AV_PACKET *pck = NULL;
    int ReadFileRet = 0 ,len = 0;
    int ret ;
    PB_FRAME_INFO  pb_frame_inf_t;
    isil_h264_decode_bit_info_msg_t dspt_t;


    int fd = get_s_pb_mgt_cur_file_fd(s_pb_mgt);
RETRY:

    ReadFileRet = ISIL_FM_PLAY_ParsingFile(fd,&pb_frame_inf_t);
    if(ReadFileRet < 0) {
        fprintf(stderr,"ISIL_FM_PLAY_ParsingFile failed .\n");
        return -1;
    }

    if(!pb_frame_inf_t.nalcnt || !pb_frame_inf_t.len){
        fprintf(stderr,"Get idr frame err .\n");
        return -1;
    }

    /* todo */
    if((pb_frame_inf_t.nal[0].naltype > E_FRAME_TYPE_AUDIO)
            &&(pb_frame_inf_t.nal[0].naltype < E_FRAME_TYPE_AUDIO_END)){
        	fprintf(stderr, "now discard audio frame\n");
            goto RETRY;

    }

	if((pb_frame_inf_t.nal[0].naltype > E_FRAME_TYPE_MJPG)
			&&(pb_frame_inf_t.nal[0].naltype < E_FRAME_TYPE_MJPG_END)){
			fprintf(stderr, "now discard mjpg frame\n");
			goto RETRY;

	}

    if( pb_frame_inf_t.ts < GET_S_PB_END_TS(s_pb_mgt) ) {
		if(GET_S_PB_CUR_MODE(s_pb_mgt) == FRAMEBACK_PB_MODE){
			dspt_t.bit_info |= BI_IS_DISPLAY(0);
		}
		else{
			dspt_t.bit_info |= BI_IS_DISPLAY(1);
		}
		dspt_t.bit_info |= BI_IS_END(0);
		dspt_t.bit_info |= BI_IS_FILE_END(0);
		dspt_t.bit_info |= BI_IS_GOP_END(0);
	}else{
		fprintf(stderr,"ok last frame .\n");
		dspt_t.bit_info |= BI_IS_DISPLAY(1);
		dspt_t.bit_info |= BI_IS_END(1);
		dspt_t.bit_info |= BI_IS_FILE_END(0);
		dspt_t.bit_info |= BI_IS_GOP_END(0);
	}


    len = pb_frame_inf_t.len + DECFRAMEADDHEADERSIZE;

    pck = alloc_av_packet_ex(len);
    if( !pck ) {
        return -1;
    }

    add_single_pb_mgt_v_count(s_pb_mgt);
    ret = decode_fill_h264_buff_header(pck->buff ,
                                       get_single_pb_mgt_v_count(s_pb_mgt),
                                       &pb_frame_inf_t,
                                       &dspt_t);

    if( ret < 0 ) {
        fprintf(stderr ,"decode_fill_h264_buff_header err .\n");
        release_av_packet_ex(pck);
        return -1;
    }




    ret = send_av_packet_to_pb_mgt_v_list(s_pb_mgt,pck);
    if(ret < 0 ) {
        release_av_packet_ex(pck);
        return -1;
    }

    s_pb_mgt->cur_pck = pck;
    s_pb_mgt->have_video = 1;
    set_single_pb_mgt_v_tsp(s_pb_mgt,pb_frame_inf_t.ts);

    return 0;
}



int dec_frame_forward_read_file(single_pb_mgt * s_pb_mgt,void *arg)
{
    ISIL_AV_PACKET *pck = NULL;
    int ReadFileRet = 0 ,len = 0,getFrametype = 0;
    int ret = 0;
    PB_FRAME_INFO  pb_frame_inf_t;
    isil_h264_decode_bit_info_msg_t dspt_t;


    int fd = get_s_pb_mgt_cur_file_fd(s_pb_mgt);

    dspt_t.bit_info |= BI_IS_DISPLAY(1);
    dspt_t.bit_info |= BI_IS_END(0);
    dspt_t.bit_info |= BI_IS_FILE_END(0);
    dspt_t.bit_info |= BI_IS_GOP_END(0);

    
RETRY:
    ReadFileRet = ISIL_FM_PLAY_ParsingFile(fd,&pb_frame_inf_t);
    if(ReadFileRet < 0) {
        fprintf(stderr,"ISIL_FM_PLAY_ParsingFile failed .\n");
        return -1;
    }

    if(!pb_frame_inf_t.nalcnt || !pb_frame_inf_t.len){
        return 0;
    }

    if((pb_frame_inf_t.nal[0].naltype > E_FRAME_TYPE_AUDIO)
            &&(pb_frame_inf_t.nal[0].naltype < E_FRAME_TYPE_AUDIO_END)){
        	fprintf(stderr, "now discard audio frame\n");
            goto RETRY;

    }

	if((pb_frame_inf_t.nal[0].naltype > E_FRAME_TYPE_MJPG)
			&&(pb_frame_inf_t.nal[0].naltype < E_FRAME_TYPE_MJPG_END)){
			fprintf(stderr, "now discard mjpg frame\n");
			goto RETRY;

	}


    len = pb_frame_inf_t.len + DECFRAMEADDHEADERSIZE;



    pck = alloc_av_packet_ex(len);
    if( !pck ) {
        return -1;
    }

    add_single_pb_mgt_v_count(s_pb_mgt);

    ret = decode_fill_h264_buff_header(pck->buff ,
                                       get_single_pb_mgt_v_count(s_pb_mgt),
                                       &pb_frame_inf_t,
                                       &dspt_t);

    if( ret < 0 ) {
        fprintf(stderr ,"decode_fill_h264_buff_header err .\n");
        release_av_packet_ex(pck);
        return -1;
    }

    if(ret > len) {
        fprintf( stderr,"Bug, len is small .\n");
        assert(0);
    }
    pck->date_len = ret;
    pck->pts = pb_frame_inf_t.ts;
    ret = send_av_packet_to_pb_mgt_v_list(s_pb_mgt,pck); 
    if(ret < 0 ) {
        release_av_packet_ex(pck);
        return -1;
    }
    
    s_pb_mgt->cur_pck = pck;

    s_pb_mgt->have_video = 1;
    set_single_pb_mgt_v_tsp(s_pb_mgt,pb_frame_inf_t.ts);

    
    

    SET_S_PB_NO_READ_FILE(s_pb_mgt);

    return 0;
}


int dec_normal_read_file(single_pb_mgt * s_pb_mgt,void *arg)
{
    ISIL_AV_PACKET *pck = NULL;
    int ReadFileRet = 0 ,len = 0,getFrametype = 0;
    
    PB_FRAME_INFO  pb_frame_inf_t;
    isil_h264_decode_bit_info_msg_t dspt_t;

    int fd ,ret;



    fd =get_s_pb_mgt_cur_file_fd(s_pb_mgt);

    dspt_t.bit_info |= BI_IS_DISPLAY(1);
    dspt_t.bit_info |= BI_IS_END(0);
    dspt_t.bit_info |= BI_IS_FILE_END(0);
    dspt_t.bit_info |= BI_IS_GOP_END(0);


RETRY:
    ReadFileRet = ISIL_FM_PLAY_ParsingFile(fd,&pb_frame_inf_t);
    if(ReadFileRet < 0) {
        fprintf(stderr,"ISIL_FM_PLAY_ParsingFile failed .\n");
        return -1;
    }


    if(!pb_frame_inf_t.nalcnt || !pb_frame_inf_t.len){
        return 0;
    }

   // DEBUG_FUNCTION();

    if((pb_frame_inf_t.nal[0].naltype > E_FRAME_TYPE_AUDIO)
            &&(pb_frame_inf_t.nal[0].naltype < E_FRAME_TYPE_AUDIO_END)){
        	fprintf(stderr, "now discard audio frame\n");
            goto RETRY;

    }

	if((pb_frame_inf_t.nal[0].naltype > E_FRAME_TYPE_MJPG)
			&&(pb_frame_inf_t.nal[0].naltype < E_FRAME_TYPE_MJPG_END)){
			fprintf(stderr, "now discard mjpg frame\n");
			goto RETRY;

	}

    //fprintf(stderr, "------valid data len: %d-----\n", pb_frame_inf_t.len);

    len = pb_frame_inf_t.len + DECFRAMEADDHEADERSIZE;

    //DEBUG_FUNCTION();
    //fprintf(stderr, "------malloc len: %d-----\n", len);
    pck = alloc_av_packet_ex(len);
    if( !pck ) {
        return -1;
    }
    //DEBUG_FUNCTION();
    if((pb_frame_inf_t.nal[0].naltype > E_FRAME_TYPE_AUDIO)
       && (pb_frame_inf_t.nal[0].naltype < E_FRAME_TYPE_AUDIO_END)){
    	//DEBUG_FUNCTION();
        add_single_pb_mgt_a_count(s_pb_mgt);
        ret = decode_fill_audio_buff_header(pck->buff,
                                         get_single_pb_mgt_a_count(s_pb_mgt),
                                         &pb_frame_inf_t);

        if( ret < 0 ) {
            fprintf(stderr ,"decode_fill_audio_buff_header err .\n");
            release_av_packet_ex(pck);
            return -1;
        }

        getFrametype = FILE_GET_FRAME_IS_A;
        s_pb_mgt->have_audio = 1;
    }
    else{
    	//DEBUG_FUNCTION();
        getFrametype = FILE_GET_FRAME_IS_V;
        add_single_pb_mgt_v_count(s_pb_mgt);

        ret = decode_fill_h264_buff_header(pck->buff ,
                                       get_single_pb_mgt_v_count(s_pb_mgt),
                                       &pb_frame_inf_t,
                                       &dspt_t);

        if( ret < 0 ) {
            fprintf(stderr ,"decode_fill_h264_buff_header err .\n");
            release_av_packet_ex(pck);
            return -1;
        }

        s_pb_mgt->have_video = 1;

        //fprintf(stderr, "------pck->buff_size: %d, pck->date_len: %d-----\n", pck->buff_size, pck->date_len);
        if(ret > len) {
            fprintf( stderr,"Bug, len is small .\n");
            assert(0);
        }

    }


    pck->date_len = ret;
    pck->pts = pb_frame_inf_t.ts;

    if(getFrametype == FILE_GET_FRAME_IS_A) {
        ret = send_av_packet_to_pb_mgt_a_list(s_pb_mgt,pck); 
        if(ret < 0 ) {
            release_av_packet_ex(pck);
            return -1;
        }

        set_single_pb_mgt_a_tsp(s_pb_mgt,pb_frame_inf_t.ts);
    }
    else{
    	//DEBUG_FUNCTION();
        ret = send_av_packet_to_pb_mgt_v_list(s_pb_mgt,pck); 
        if(ret < 0 ) {
            release_av_packet_ex(pck);
            return -1;
        }
        //DEBUG_FUNCTION();
        set_single_pb_mgt_v_tsp(s_pb_mgt,pb_frame_inf_t.ts);
        if( !s_pb_mgt->cur_pck ) {
        	//DEBUG_FUNCTION();
            s_pb_mgt->cur_pck = pck;
            goto RETRY;
        }
        else{
        	//DEBUG_FUNCTION();
            s_pb_mgt->cur_pck = pck;
        }
        //DEBUG_FUNCTION();
    }

    //DEBUG_FUNCTION();
    return 0;
}





int dev_drv_need_frame_handle(single_pb_mgt * s_pb_mgt,struct  query_info *query)
{
    int ret;
    struct frame_query *p_frame_query;

    if( !s_pb_mgt || !query) {
        return -1;
    }

    
    if(GET_S_PB_CUR_MODE(s_pb_mgt) != FRAMEBACK_PB_MODE && GET_S_PB_CUR_MODE(s_pb_mgt) != BACKWARD_PB_MODE) {
        fprintf(stderr,"s_pb cur mode err .\n");
        return 0;
    }

    p_frame_query = &query->f_query;

    fprintf(stderr, "---chan_id[%x],f_invalid[%x],start[%x],end[%x]-----\n",
            query->chan_id, query->f_query.f_invalid,
            query->f_query.start, query->f_query.end);

    if(p_frame_query->f_invalid == 0x00) {

        /*todo ,find idr, jump idr*/

        ret = dec_find_after_this_idr(s_pb_mgt);
        if( ret < 0) {
            fprintf(stderr,"dec_find_after_this_idr err .\n");
            return -1;
        }

        SET_S_PB_END_TS(s_pb_mgt ,p_frame_query->end);
        only_reg_read_cb_func(s_pb_mgt,dec_back_get_frame_until_ts_exclude_end_ts);

    }
    else if(p_frame_query->f_invalid == 0x01) {
         /*todo ,dj set file back get x frame*/

        SET_S_PB_NO_READ_FILE(s_pb_mgt);
        SET_S_PB_START_TS(s_pb_mgt,p_frame_query->start);
        if(dec_find_read_starttime_frame(s_pb_mgt,p_frame_query->start) < 0 ) {
            fprintf(stderr,"dec_back_get_frame_for_starttime err .\n");
            return -1;
        }

        /*reg read callback ,get other frame*/
        only_reg_read_cb_func(s_pb_mgt,dec_back_get_frame_until_idr);
        SET_S_PB_READ_FILE(s_pb_mgt);

    }
    else if( p_frame_query->f_invalid == 0x10 ) {

        /*todo ,dj set file back get prev IDR frame*/

        ret = dec_find_prev_idr(s_pb_mgt);
        if(ret < 0) {
            fprintf(stderr,"dec_find_prev_idr failed .\n");
            return -1;
        }

        /*reg read callback ,get other frame*/
        only_reg_read_cb_func(s_pb_mgt,dec_back_get_frame_until_idr);
        SET_S_PB_READ_FILE(s_pb_mgt);

    }
    else if( p_frame_query->f_invalid == 0x11 ){
          /*todo ,dj set file back get  frame from idr_next to idr next_to_next*/
          /*search next idr*/

        ret = dec_find_next_idr(s_pb_mgt);
        if(ret < 0) {
            fprintf(stderr,"dec_find_next_idr failed .\n");
            return -1;
        }

        only_reg_read_cb_func(s_pb_mgt,dec_back_get_frame_until_idr);
        SET_S_PB_READ_FILE(s_pb_mgt);

    }
    else if( p_frame_query->f_invalid == -1) {

        if(p_frame_query->end < p_frame_query->start ) {
            fprintf(stderr,"Set timestamp err .\n");
            return -1;
        }

        /*todo ,dj set file back get  frame from x to y*/
        /*search x ,read x*/
        SET_S_PB_START_TS(s_pb_mgt,p_frame_query->start);

        if(dec_find_read_starttime_frame(s_pb_mgt,p_frame_query->start) < 0 ) {
            fprintf(stderr,"dec_back_get_frame_for_starttime err .\n");
            return -1;
        }

        /*set read to y*/

        SET_S_PB_END_TS(s_pb_mgt ,p_frame_query->end);
        only_reg_read_cb_func(s_pb_mgt,dec_back_get_frame_until_ts);
        SET_S_PB_READ_FILE(s_pb_mgt);
    }
    else{
        assert(0);
    }


    return 0;
}
