#ifndef	ISIL_AUDIO_DE_H__
#define	ISIL_AUDIO_DE_H__

#ifdef __cplusplus
extern "C"
{
#endif







#define	ISIL_AUDIO_8K         (0)
#define	ISIL_AUDIO_16K        (1)

#define	ISIL_AUDIO_16BIT      (0)
#define	ISIL_AUDIO_8BIT       (1)

#define	ISIL_AUDIO_PCM        (0)
#define	ISIL_AUDIO_ALAW       (1)
#define	ISIL_AUDIO_ULAW       (2)
#define	ISIL_AUDIO_ADPCM_32K  (3)
#define	ISIL_AUDIO_ADPCM_16K  (4)
#define	ISIL_AUDIO_ADPCM_48K  (5)

#define	AUDIO_ENABLE        (0x0001)
#define	AUDIO_DISABLE       (0x0000)
#define	AUDIO_MUTE_ENABLE   (0x0001)
#define	AUDIO_MUTE_DISABLE  (0x0000)



    typedef enum audio_state
    {
	NEED_INIT = 0,
	INITED,
	OPENED,
	CLOSED,
    }AUDIO_STATE;


    struct audio_en_operation
    {
	int (*init)(struct isil_audio_en *ai);
	//...
    };

    struct isil_audio_config
    {
	int type;
	int sample_rate;
	int bit_wide;
    };

    struct isil_audio_paramts
    {
	struct isil_audio_config    property;
	struct audio_en_operation  *op;
    };

    /*the top struct of audio in*/
    struct isil_audio_de
    {
	struct isil_service_device	tsd;
	char 				*name;
	int 				users;
	AUDIO_STATE			state;
	struct isil_audio_paramts		para;
	isil_audio_driver_t		*ae_driv;
	struct tc_queue			*queue;
	struct semaphore		sem;

    };










#ifdef __cplusplus
}
#endif

#endif
