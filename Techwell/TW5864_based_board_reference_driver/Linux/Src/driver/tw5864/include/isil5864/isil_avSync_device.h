#ifndef ISIL_AVSYNC_DEVICE_H
#define	ISIL_AVSYNC_DEVICE_H

#ifdef __cplusplus
extern "C"
{
#endif

#define	DEV_NAME_LEN		32
    struct isil_avSync_dev
    {
	char    name[DEV_NAME_LEN];
	int     bus_id;
	int     chip_id;
	int     dev_inode_id;
	int     id;
	struct video_device	vfd;
	struct semaphore	sem;
	atomic_t	opened_flags;
	wait_queue_head_t	wait_poll;

	isil_h264_logic_encode_chan_t *h264_master_encode_driver;
	isil_h264_logic_encode_chan_t *h264_sub_encode_driver;
	isil_jpeg_logic_encode_chan_t *jpeg_encode_driver;
	isil_audio_driver_t   *audio_encode_driver;
	isil_audio_driver_t   *audio_decode_driver;

	avSync_frame_queue_t	avSync_frame_queue;
	avSync_frame_buf_pool_t	device_buf_pool;
    };

    extern int	init_isil_avSync_dev(isil_avSync_dev_t *dev, int bus_id, int chip_id, int inode_id, int id);
    extern void	remove_isil_avSync_dev(isil_avSync_dev_t *dev);
    extern void	bind_av_device_and_driver(isil_avSync_dev_t *isil_device_chan, isil_h264_logic_encode_chan_t *h264_master_encode_logic_chan, isil_h264_logic_encode_chan_t *h264_sub_encode_logic_chan, isil_jpeg_logic_encode_chan_t *jpeg_logic_chan, isil_audio_driver_t *audio_encode_driver);
    extern void	unbind_av_device_and_driver(isil_avSync_dev_t *isil_device_chan, isil_h264_logic_encode_chan_t *h264_master_encode_logic_chan, isil_h264_logic_encode_chan_t *h264_sub_encode_logic_chan, isil_jpeg_logic_encode_chan_t * jpeg_logic_chan, isil_audio_driver_t *audio_encode_driver);


#ifdef __cplusplus
}
#endif

#endif

