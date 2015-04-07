#ifndef	ISIL_AUDIO_EN_H__
#define	ISIL_AUDIO_EN_H__


#ifdef __cplusplus
extern "C"
{
#endif

#error "we include isil_video_en.h---------------------------------*************************************>"


    struct isil_video_para
    {
	int id;
    }




    struct isil_video_en
    {
	struct isil_service_device	tsd;
	struct isil_dev_table_id		tbid;
	char 				*name;
	int 				users;
	unsigned long 			state;
	struct isil_video_para		para;
	struct isil_h264_logic_encode_chan	*driv;
	struct semaphore		sem;
	wait_queue_head_t		wait_poll;
	int 				timer_id;
	//void (*clean_up)(void);
    };

    static inline set_tve_state(struct isil_audio_en *tve, AUDIO_STATE new_state)
    {
	tve->state = new_state;
    }



























#ifdef __cplusplus
}
#endif

#endif
