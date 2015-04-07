#ifndef __DVM_CORE_IOCTL_H__
#define __DVM_CORE_IOCTL_H__

#include <linux/ioctl.h>
#include <isil5864/tc_common.h>



#define	DVM_SYS_IOC_MAGIC				'S'
#define	DVM_CODEC_IOC_MAGIC				'C'
#define	DVM_MOTION_DECTION_IOC_MAGIC	'D'
#define	DVM_OSD_IOC_MAGIC				'O'
#define	DVM_IOC_MAXNR					(256)




/*system cmd*/
#define	DVM_SYS_GET_CHAN_WORK_MODE			_IOR(ISIL_CODEC_IOC_MAGIC, 12, unsigned int)
#define	DVM_SYS_SET_CHAN_WORK_MODE			_IOW(ISIL_CODEC_IOC_MAGIC, 13, unsigned int)
#define DVM_SYS_CREATE_CHAN				_IOW(ISIL_CODEC_IOC_MAGIC, 14, unsigned int)
#define DVM_SYS_RELEASE_CHAN				_IOW(ISIL_CODEC_IOC_MAGIC, 15, unsigned int)
#define DVM_SYS_QUERY_CAP				_IOW(ISIL_CODEC_IOC_MAGIC, 16, unsigned int)
#define DVM_SYS_GET_CTL					_IOW(ISIL_CODEC_IOC_MAGIC, 17, unsigned int)
#define DVM_SYS_SET_CTL					_IOW(ISIL_CODEC_IOC_MAGIC, 18, unsigned int)
/*encoder cmds, from the user point of view*/
#define DVM_SYS_REQUEST_MEM				_IOW(ISIL_CODEC_IOC_MAGIC, 19, unsigned int)
#define DVM_SYS_RELEASE_MEM				_IOW(ISIL_CODEC_IOC_MAGIC, 20, unsigned int)



/*eg;  iocmd little endian 
 *                                      | di|  |       size         ||   type   ||   nr     |
 * DVM_SYS_REQUEST_MEM : 0x40045313---- 01      00, 0000, 0000, 0100, 0101, 0011, 0001, 0011  
 * DVM_SYS_QUERY_MEM   : 0x40205214 	
 * DVM_SYS_QUEUQ_MEM   : 0x40205315     01      00, 0000, 0010, 0000, 0101, 0011, 0001, 0101 
 */


/*codec cmd*/
#define	DVM_CODEC_GET_AUDIO_ENCODER_PARAM	_IOR(ISIL_CODEC_IOC_MAGIC, 4, unsigned int)
#define	DVM_CODEC_SET_AUDIO_ENCODER_PARAM	_IOW(ISIL_CODEC_IOC_MAGIC, 5, unsigned int)
#define	DVM_CODEC_SET_VIDEO_TIMESTAMP_BASE	_IOW(ISIL_CODEC_IOC_MAGIC, 6, unsigned int)
#define	DVM_CODEC_GET_VIDEO_TIMESTAMP_BASE	_IOW(ISIL_CODEC_IOC_MAGIC, 7, unsigned int)

/*motion detection cmd*/
#define	DVM_MOTION_DECTION_GET_PARAM		_IOR(ISIL_CODEC_IOC_MAGIC, 0, unsigned int)
#define	DVM_MOTION_DECTION_SET_PARAM		_IOW(ISIL_CODEC_IOC_MAGIC, 1, unsigned int)

/*osd cmd*/
#define	DVM_OSD_GET_PARAM					_IOR(ISIL_CODEC_IOC_MAGIC, 0, unsigned int)
#define	DVM_OSD_SET_PARAM					_IOW(ISIL_CODEC_IOC_MAGIC, 1, unsigned int)


#endif
