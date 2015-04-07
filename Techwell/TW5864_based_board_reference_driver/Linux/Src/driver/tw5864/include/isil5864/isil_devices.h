#ifndef DVM4000I_H
#define DVM4000I_H

#ifdef __cplusplus
extern "C"
{
#endif

#define	DEV_NAME_LEN		32
    struct isil_video_device{
	char	name[DEV_NAME_LEN];
	int		device_id;
	int		device_type;
	int		device_state;
	struct semaphore	device_lock;
	isil_chip_t			*chip;
	struct video_device	vfd;

	struct list_head	device_list;
	void	(*register_device)(chip_register_t	*, isil_video_device_t *);
	void	(*unregister_device)(chip_register_t *, isil_video_device_t *);
	struct list_head	opened_device_list;
	void	(*register_opened_device)(isil_chip_t *, isil_video_device_t *);
	void	(*unregister_opened_device)(isil_chip_t *, isil_video_device_t *);

	spinlock_t	device_map_lock;
	void	*service_chan_driver;
	int		(*attach_service_chan_driver)(isil_video_device_t *, void *);
	void	(*detach_service_chan_driver)(isil_video_device_t *);
	wait_queue_head_t	wait_poll;
	ssize_t	(*read)(isil_video_device_t *, char __user *, size_t , loff_t *);
	ssize_t	(*write)(isil_video_device_t *, const char __user *, size_t , loff_t *);
    };


    extern void isil_video_device_remove(int );
    extern int  isil_video_device_init(isil_chip_t *, int);
    extern void	isil_video_group_remove(isil_video_group_t *);
    extern int	isil_video_group_init(isil_chip_t *, int);
    extern void	remove_encode_chan(isil_h264_encode_t *);
    extern int	encode_chan_init(isil_video_group_t *, int);

    extern void process_slice_header_req(isil_h264_encode_t *);
    extern void process_pps_req(isil_h264_encode_t *);
    extern void process_sps_req(isil_h264_encode_t *);

#ifdef __cplusplus
}
#endif

#endif

