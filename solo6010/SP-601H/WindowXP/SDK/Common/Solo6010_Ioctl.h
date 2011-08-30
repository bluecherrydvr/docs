#ifndef __S6010_IOCTL_H
#define __S6010_IOCTL_H

/****************************************************************
 * The DeviceType field of the CTL_CODE is kept to the old
 * definition for compatibility reasons.  The next SDK release
 * may change this to a Microsoft defined type.
 ***************************************************************/
#define IOCTL_MSG( code )                           CTL_CODE(             \
                                                    FILE_DEVICE_UNKNOWN,  \
                                                    code,             \
                                                    METHOD_BUFFERED,  \
                                                    FILE_ANY_ACCESS   \
                                                    )

#define IOCTL_MSG_DIRECT( code )                    CTL_CODE(             \
                                                    FILE_DEVICE_UNKNOWN,  \
                                                    code,             \
                                                    METHOD_OUT_DIRECT,  \
                                                    FILE_ANY_ACCESS   \
                                                    )

 /*
 *  Macro definition for IOCTL control codes.
 *
 *  Note: Codes 0-2047 (0-7ffh) are reserved by Microsoft
 *        Code 2048-4095 are reserved for OEMs
 */
#define S6010_IOCTL_REG_READ					IOCTL_MSG_DIRECT(0x800)
#define S6010_IOCTL_REG_WRITE					IOCTL_MSG_DIRECT(0x801)

#define S6010_IOCTL_OPEN_S6010					IOCTL_MSG_DIRECT(0x802)

#define S6010_IOCTL_SET_REC_PROP				IOCTL_MSG_DIRECT(0x810)
#define S6010_IOCTL_START_REC					IOCTL_MSG_DIRECT(0x811)
#define S6010_IOCTL_END_REC						IOCTL_MSG_DIRECT(0x812)
#define S6010_IOCTL_CHANGE_REC_PROP				IOCTL_MSG_DIRECT(0x813)

#define S6010_IOCTL_SET_LIVE_PROP				IOCTL_MSG_DIRECT(0x820)
#define S6010_IOCTL_START_LIVE					IOCTL_MSG_DIRECT(0x821)
#define S6010_IOCTL_END_LIVE					IOCTL_MSG_DIRECT(0x822)

#define S6010_IOCTL_GET_COLOR					IOCTL_MSG_DIRECT(0x830)
#define S6010_IOCTL_SET_COLOR					IOCTL_MSG_DIRECT(0x831)
#define S6010_IOCTL_GET_VLOSS_STATE				IOCTL_MSG_DIRECT(0x832)
#define S6010_IOCTL_SET_MOSAIC_PROP				IOCTL_MSG_DIRECT(0x833)
#define S6010_IOCTL_SET_V_MOTION_PROP			IOCTL_MSG_DIRECT(0x834)
#define S6010_IOCTL_SET_CAM_TO_WND_PROP			IOCTL_MSG_DIRECT(0x835)
#define S6010_IOCTL_GET_CAM_BLK_STATE			IOCTL_MSG_DIRECT(0x836)

#define S6010_IOCTL_OPEN_HW_MP4_DEC				IOCTL_MSG_DIRECT(0x840)
#define S6010_IOCTL_CLOSE_HW_MP4_DEC			IOCTL_MSG_DIRECT(0x841)

#define S6010_IOCTL_OPEN_HW_G723_DEC			IOCTL_MSG_DIRECT(0x848)
#define S6010_IOCTL_CLOSE_HW_G723_DEC			IOCTL_MSG_DIRECT(0x849)
#define S6010_IOCTL_PLAY_HW_G723_DEC			IOCTL_MSG_DIRECT(0x84a)
#define S6010_IOCTL_STOP_HW_G723_DEC			IOCTL_MSG_DIRECT(0x84b)

#define S6010_IOCTL_GPIO_CTRL					IOCTL_MSG_DIRECT(0x850)

#define S6010_IOCTL_SET_ENC_OSD_PROP			IOCTL_MSG_DIRECT(0x860)
#define S6010_IOCTL_GET_ENC_OSD					IOCTL_MSG_DIRECT(0x861)
#define S6010_IOCTL_SET_ENC_OSD					IOCTL_MSG_DIRECT(0x862)

#define S6010_IOCTL_GET_VID_OSG					IOCTL_MSG_DIRECT(0x870)
#define S6010_IOCTL_SET_VID_OSG					IOCTL_MSG_DIRECT(0x871)

#define S6010_IOCTL_SETUP_UART					IOCTL_MSG_DIRECT(0x880)
#define S6010_IOCTL_READ_UART					IOCTL_MSG_DIRECT(0x881)
#define S6010_IOCTL_WRITE_UART					IOCTL_MSG_DIRECT(0x882)

#define S6010_IOCTL_START_ADV_MOTION_DATA_GET	IOCTL_MSG_DIRECT(0x890)
#define S6010_IOCTL_END_ADV_MOTION_DATA_GET		IOCTL_MSG_DIRECT(0x891)

#define S6010_IOCTL_I2C_READ					IOCTL_MSG_DIRECT(0x8a0)
#define S6010_IOCTL_I2C_WRITE					IOCTL_MSG_DIRECT(0x8a1)

#endif
