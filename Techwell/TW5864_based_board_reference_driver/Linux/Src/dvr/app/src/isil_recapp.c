#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>

#include "isil_fm_rec.h"
#include "linux_list.h" 
#include "isil_recapp.h"
#include "isil_data_stream.h"
#include "isil_file_rec_mgt.h"
#include "net_debug.h"



typedef struct{
	unsigned int  chan;
	isil_rec_mgt* file;
	struct list_head list;
}isil_recapp_mgt;

LIST_HEAD(RECLIST);

static isil_recapp_mgt* isil_recapp_searchlist(unsigned int chan)
{
	isil_recapp_mgt *pos, *tmp;

	if(list_empty(&RECLIST)){
		printf("----reclist is empty---\n");
		return NULL;
	}

	list_for_each_entry_safe(pos, tmp, &RECLIST, list){
		if(pos->chan == chan){
			return pos;
		}
	}
	
	return NULL;
}

static isil_recapp_mgt* isil_recapp_accloc_init_list(unsigned int chan)
{
	isil_recapp_mgt* reclist = NULL;

	reclist = isil_recapp_searchlist(chan);

	if(!reclist){
		reclist = (isil_recapp_mgt*)malloc(sizeof(isil_recapp_mgt));
		if(!reclist){
			printf("----malloc reclist fail---\n");
			return NULL;
		}
	
		memset(reclist, 0x00, sizeof(isil_recapp_mgt));
		reclist->chan = chan;
		reclist->file = ISIL_FM_REC_Init(chan);
		if(!reclist->file){
			printf("----ISIL_FM_REC_Init fail---\n");
			return NULL;
		}

		list_add_tail(&(reclist->list), &RECLIST);
	}

	return reclist;
}

static int isil_recapp_releaselist(isil_recapp_mgt* reclist)
{
	if(!reclist){
		printf("----%s, %d, fail---\n", __FUNCTION__, __LINE__);
		return -1;
	}

	list_del(&(reclist->list));
	free(reclist);
	reclist = NULL;
	return 0;
}


/*start rec file callback*/
int isil_recapp_startrec(void* startrec)
{
	isil_recapp_mgt* reclist = NULL;

	RECAPP_START* fmstartrec = (RECAPP_START*)startrec;
	if(!fmstartrec){
		printf("----%s, %d, fail---\n", __FUNCTION__, __LINE__);
		return -1;
	}

	fprintf(stderr, "----%s---\n", __FUNCTION__);

	reclist = isil_recapp_accloc_init_list(fmstartrec->chan);
	if(!reclist){
		printf("----%s, %d, fail---\n", __FUNCTION__, __LINE__);
		return -1;
	}

	return ISIL_FM_REC_OpenFile(reclist->file, fmstartrec->maintype, fmstartrec->subtype);
}


int isil_recapp_sync(void* recsync)
{
	isil_recapp_mgt* reclist = NULL;

	RECAPP_COMM* reccomm = (RECAPP_COMM*)recsync;
	if(!reccomm){
		printf("----%s, %d, fail---\n", __FUNCTION__, __LINE__);
		return -1;
	}

	fprintf(stderr, "----%s---\n", __FUNCTION__);

	reclist = isil_recapp_searchlist(reccomm->chan);
	if(!reclist){
		printf("----%s, %d, fail---\n", __FUNCTION__, __LINE__);
		return -1;
	}

	return ISIL_FM_REC_SyncFile(reclist->file);
}


int isil_recapp_close(void* recclose)
{
	isil_recapp_mgt* reclist = NULL;

	RECAPP_COMM* reccomm = (RECAPP_COMM*)recclose;
	if(!reccomm){
		printf("----%s, %d, fail---\n", __FUNCTION__, __LINE__);
		return -1;
	}

	fprintf(stderr, "----%s---\n", __FUNCTION__);

	reclist = isil_recapp_searchlist(reccomm->chan);
	if(!reclist){
		printf("----%s, %d, fail---\n", __FUNCTION__, __LINE__);
		return -1;
	}

	if(ISIL_FM_REC_CloseFile(reclist->file) < 0){
		printf("----%s, %d, fail---\n", __FUNCTION__, __LINE__);
		return -1;
	}

	if(isil_recapp_releaselist(reclist) < 0){
		printf("----%s, %d, fail---\n", __FUNCTION__, __LINE__);
		return -1;
	}
	
	fprintf(stderr, "----%s ok---\n", __FUNCTION__);

	return 0;
}

static int isil_rec_trans_write(ISIL_AV_PACKET* av_packet, unsigned int chan)
{
    //int ret;
    //int len;
    int frametype;
    stDriverFrame localframe;
	isil_recapp_mgt* reclist = NULL;

	//fprintf(stderr, "----%s, ch[%d]---\n", __FUNCTION__, chan);

    if(!av_packet)
    {
        printf("----%s, %d, fail---\n", __FUNCTION__, __LINE__);
        return -1;
    }

	reclist = isil_recapp_searchlist(chan);
	if(!reclist){
		printf("----%s, %d, fail---\n", __FUNCTION__, __LINE__);
		return -1;
	}

	memset(&localframe, 0x00, sizeof(stDriverFrame));

	switch(av_packet->frm_type)
	{
		case ISIL_H264_DATA:
		{
			frametype = av_packet->data_type_u.h264_inf.nalu_type;
			
			if(av_packet->data_type_u.h264_inf.stream_type == ISIL_MAIN_STREAM_E){
				
				//printf("----%s, %d, main stream---\n", __FUNCTION__, __LINE__);

				if(frametype == H264_NALU_SPS){
					localframe.type = E_H264_FRAME_TYPE_MAIN_SPS; 
				}else if(frametype == H264_NALU_PPS){
					localframe.type = E_H264_FRAME_TYPE_MAIN_PPS;
				}else if(frametype == H264_NALU_IDRSLICE){
					localframe.type = E_H264_FRAME_TYPE_MAIN_IDR;
				}else if(frametype == H264_NALU_ISLICE){
					localframe.type = E_H264_FRAME_TYPE_MAIN_I;
				}else if(frametype == H264_NALU_PSLICE){
					localframe.type = E_H264_FRAME_TYPE_MAIN_P;
				}else{
					printf("----%s, %d, fail---\n", __FUNCTION__, __LINE__);
					return 0;
				}
			}else if(av_packet->data_type_u.h264_inf.stream_type == ISIL_SUB_STREAM_E){
				
				//printf("----%s, %d, sub stream---\n", __FUNCTION__, __LINE__);

				if(frametype == H264_NALU_SPS){
					localframe.type = E_H264_FRAME_TYPE_SUB_SPS; 
				}else if(frametype == H264_NALU_PPS){
					localframe.type = E_H264_FRAME_TYPE_SUB_PPS;
				}else if(frametype == H264_NALU_IDRSLICE){
					localframe.type = E_H264_FRAME_TYPE_SUB_IDR;
				}else if(frametype == H264_NALU_ISLICE){
					localframe.type = E_H264_FRAME_TYPE_SUB_I;
				}else if(frametype == H264_NALU_PSLICE){
					localframe.type = E_H264_FRAME_TYPE_SUB_P;
				}else{
					printf("----%s, %d, fail---\n", __FUNCTION__, __LINE__);
					return 0;
				}
			}
		}
		break;

		case ISIL_AUDIO_DATA:
		{
			//printf("----%s, %d, audio--\n", __FUNCTION__, __LINE__);

			frametype = av_packet->data_type_u.audio_inf.audio_type_e;

			if(frametype == ISIL_AUDIO_PCM_E){
				localframe.type = E_AUDIO_FRAME_TYPE_PCM;
			}else if(frametype == ISIL_AUDIO_ALAW_E){
				localframe.type = E_AUDIO_FRAME_TYPE_ALAW;
			}else if(frametype == ISIL_AUDIO_ULAW_E){
				localframe.type = E_AUDIO_FRAME_TYPE_ULAW;
			}else if(frametype == ISIL_AUDIO_ADPCM_E){
				localframe.type = E_AUDIO_FRAME_TYPE_ADPCM;
			}else{
				printf("----%s, %d, fail---\n", __FUNCTION__, __LINE__);
				return 0;
			}
		}
		break;

		case ISIL_MJPEG_DATA:
		{	
			//printf("----%s, %d, mjpg--\n", __FUNCTION__, __LINE__);
			frametype = E_H264_MJPEG_FRAME_TYPE;
		}
		break;
			
		default:
		   printf("----%s, %d, unkown frametype--\n", __FUNCTION__, __LINE__);
		   return 0;
	}

	localframe.timestamp = av_packet->pts;
	localframe.len = av_packet->date_len;
	localframe.payload = av_packet->buff;
	
	if(ISIL_FM_REC_WriteFile(&localframe, reclist->file) < 0){
		printf("----%s, %d, fail--\n", __FUNCTION__, __LINE__);
		return -1;
	}

	//fprintf(stderr, "----%s, ch[%d] ok---\n", __FUNCTION__, chan);

    return 0;
}


int isil_recapp_write(void* recwrite)
{
	struct NET_TASK* nettask = NULL;
	struct NET_DATA_MGT_T* net_data_mgt = NULL;;
	struct FILE_REC_TASK_T *rec_task = NULL;
	struct NET_DATA_NODE_T *data_node = NULL;
	ISIL_AV_PACKET *av_packet = NULL;

	rec_task = (struct FILE_REC_TASK_T *)recwrite;
	if(!rec_task){
		printf("----%s, %d, fail---\n", __FUNCTION__, __LINE__);
		return -1;
	}
	
	nettask = rec_task->task;
	if( !nettask ) {
		fprintf(stderr,"----%s, %d, fail---\n", __FUNCTION__, __LINE__);
		return -1;
	}
	net_data_mgt = nettask->net_data_mgt;
	if( !net_data_mgt ) {
		fprintf(stderr,"----%s, %d, fail---\n", __FUNCTION__, __LINE__);
		return -1;
	}

	do{
		
		
		
		av_packet = (ISIL_AV_PACKET*)data_node->arg;
		if(!av_packet){
			printf("----%s, %d, fail---\n", __FUNCTION__, __LINE__);
			data_node->release(data_node);
			
			return -1;
		}

		//isil_av_packet_debug(av_packet);

#if 1
		
		if(isil_rec_trans_write(av_packet, rec_task->logic_chan_id) < 0){
			printf("----%s, %d, fail---\n", __FUNCTION__, __LINE__);
			data_node->release(data_node);
			
			return -1;	
		}

#endif
		data_node->release(data_node);   	

	}while(1);

	
	
	return 0;
}



int new_isil_recapp_write(void* recwrite ,void *src)
{
	
	struct FILE_REC_TASK_T *rec_task = NULL;

	struct NET_DATA_NODE_T *data_node = (struct NET_DATA_NODE_T *)src;

	ISIL_AV_PACKET *av_packet = NULL;

	//fprintf(stderr, "----%s---\n", __FUNCTION__);

	rec_task = (struct FILE_REC_TASK_T *)recwrite;
	if(!rec_task){
		printf("----%s, %d, fail---\n", __FUNCTION__, __LINE__);
		return -1;
	}
	
	
	av_packet = (ISIL_AV_PACKET*)data_node->arg;
	if(!av_packet){
		printf("----%s, %d, fail---\n", __FUNCTION__, __LINE__);
		

		return -1;
	}

	if(isil_rec_trans_write(av_packet, rec_task->logic_chan_id) < 0){
			printf("----%s, %d, fail---\n", __FUNCTION__, __LINE__);
			
			return -1;	
	}


	
	return 0;
}

