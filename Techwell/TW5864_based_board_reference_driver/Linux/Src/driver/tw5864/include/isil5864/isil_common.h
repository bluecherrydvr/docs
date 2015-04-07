#ifndef	DVM_COMMON_H
#define	DVM_COMMON_H

#ifdef __cplusplus
extern "C"
{
#endif

#include    <linux/types.h>
#include    <linux/module.h>
#include    <linux/moduleparam.h>
#include    <linux/ioport.h>
#include    <linux/kernel.h>
#include    <linux/list.h>
#include    <linux/delay.h>
#include    <linux/init.h>
#include    <linux/interrupt.h>
#include    <linux/kmod.h>
#include    <linux/i2c.h>
#include    <linux/stringify.h>
#include    <linux/mutex.h>
#include    <linux/slab.h>
#include    <linux/vmalloc.h>
#include    <linux/mm.h>
#include    <linux/wait.h>
#include    <linux/completion.h>
#include    <linux/time.h>
#include    <linux/dma-mapping.h>
#include    <linux/platform_device.h>
#include    <linux/videodev2.h>
#include    <linux/dma-mapping.h>
#include    <linux/kthread.h>
#include    <linux/highmem.h>
    //#include    <linux/freezer.h>
#include    <linux/poll.h>
#include    <linux/irq.h>
#include    <linux/ioctl.h>
#include    <linux/proc_fs.h>
#include    <linux/seq_file.h>
#include    <linux/rtc.h>
#include    <linux/version.h>

#include    <asm/io.h>
#include    <asm/page.h>
#include    <asm/irq.h>
#include    <asm/atomic.h>
#include    <asm/dma.h>
#include    <asm/uaccess.h>

#define TW5864_CHIP


#ifdef PCI_PLATFORM
#include    <linux/pci.h>
#define PCI_ISIL_VENDOR_ID   0x1797
#define PCI_DEVICE_ID_TW5864 0x5864
#define PCI_DEVICE_ID_TW5866 0x5866
#endif

#include    <media/v4l2-common.h>
#include    <media/v4l2-dev.h>

    //#define   USE_SYSTEM_TIMER
#define     MJPEG_MODULE
//#define     TESTING_TIMESTAMP
#define     HARDWARE_FILL_HEADER
    //#define     ADD_DVM_NAL_HEAD
    //#define     USER_FILE_IO_REQ
#define     USE_ON_CHIP_TIMER
    //#define     FPGA_330_5864_TESTING
#define     TW5864_ASIC_NEW

#define     USE_DISCRETE_BUF
#define     NEW_HEADER

//#define     USE_FOUR_CHAN_PREVIEW


#if	defined(X86_PLATFORM) || defined(HISILICON_PLATFORM) || defined(POWERPC_PLATFORM)
#define	MAX_CHIP_NUM_IN_BOARD           (4)
#define	INVALID_CHIP_ID                 (MAX_CHIP_NUM_IN_BOARD)
#define	MAX_GROUP_NUMBER_PER_CHIP       (4)
#define	DEFAULT_FRAME_WIDTH             (704)
#define	DEFAULT_FRAME_HEIGHT            (576)
#endif


#define	DATA_SECTION_ALIGN(x)	__attribute__ ((aligned(x)))
#define	STRUCT_PACKET_ALIGN(x)	__attribute__ ((packed))
    /****************************************************************************
     * Macros
     ****************************************************************************/
#define ISIL_CHIP_IOC_MAGIC      'H'  /*ai,ao,vi,vo,chip control use it*/
#define ISIL_CODEC_IOC_MAGIC     'S'  /*only codec use it*/

#define H264_MIN(a,b)               ( (a)<(b) ? (a) : (b) )
#define H264_MAX(a,b)               ( (a)>(b) ? (a) : (b) )
#define ABS(x)                      (((x)<0) ? -(x) : (x))
#define	H264_MEDIAN(min, max, x)    (H264_MIN(H264_MAX((min), (x)), (max)))
#define H264_MIN3(a,b,c)            H264_MIN((a),X264_MIN((b),(c)))
#define H264_MAX3(a,b,c)            H264_MAX((a),X264_MAX((b),(c)))
#define H264_MIN4(a,b,c,d)          H264_MIN((a),X264_MIN3((b),(c),(d)))
#define H264_MAX4(a,b,c,d)          H264_MAX((a),X264_MAX3((b),(c),(d)))
#define XCHG(type,a,b)              { type t = a; a = b; b = t; }

#define	WIDTH_FRAME_1080P           (1920)
#define	WIDTH_FRAME_720P            (1280)
#define WIDTH_FRAME_4CIF_PAL        (704)
#define WIDTH_FRAME_2CIF_PAL        (WIDTH_FRAME_4CIF_PAL)
#define WIDTH_FRAME_4CIF_NTSC       (704)
#define WIDTH_FRAME_2CIF_NTSC       (WIDTH_FRAME_4CIF_NTSC)
#define	WIDTH_FRAME_D1_PAL          (720)
#define	WIDTH_FRAME_D1_NTSC         (720)
#define	WIDTH_FRAME_HALF_D1_PAL     (WIDTH_FRAME_D1_PAL)
#define	WIDTH_FRAME_HALF_D1_NTSC    (WIDTH_FRAME_D1_NTSC)
#define	WIDTH_FRAME_VGA             (640)
#define	WIDTH_FRAME_CIF_PAL         (WIDTH_FRAME_4CIF_PAL>>1)
    //#define	WIDTH_FRAME_CIF_PAL         (WIDTH_FRAME_D1_PAL>>1)
#define	WIDTH_FRAME_HCIF_PAL        (WIDTH_FRAME_CIF_PAL)
#define	WIDTH_FRAME_CIF_NTSC        (WIDTH_FRAME_4CIF_NTSC>>1)
    //#define	WIDTH_FRAME_CIF_NTSC        (WIDTH_FRAME_D1_NTSC>>1)
#define	WIDTH_FRAME_HCIF_NTSC       (WIDTH_FRAME_CIF_NTSC)
#define	WIDTH_FRAME_QCIF_PAL        (WIDTH_FRAME_4CIF_PAL>>2)
#define	WIDTH_FRAME_QCIF_NTSC       (WIDTH_FRAME_4CIF_NTSC>>2)
#define	WIDTH_FRAME_QVGA            (WIDTH_FRAME_VGA>>1)
#define WIDTH_FRAME_SVGA            (800)
#define WIDTH_FRAME_XGA             (1024)

#define	HEIGHT_FRAME_1080P          (1080)
#define	HEIGHT_FRAME_720P           (720)
#define HEIGHT_FRAME_4CIF_PAL       (576)
#define HEIGHT_FRAME_2CIF_PAL       (HEIGHT_FRAME_4CIF_PAL>>1)
#define HEIGHT_FRAME_HCIF_PAL       (HEIGHT_FRAME_2CIF_PAL>>1)
#define HEIGHT_FRAME_4CIF_NTSC      (480)
#define HEIGHT_FRAME_2CIF_NTSC      (HEIGHT_FRAME_4CIF_NTSC>>1)
#define HEIGHT_FRAME_HCIF_NTSC      (HEIGHT_FRAME_2CIF_NTSC>>1)
#define	HEIGHT_FRAME_D1_PAL         (576)
#define	HEIGHT_FRAME_D1_NTSC        (480)
#define	HEIGHT_FRAME_HALF_D1_PAL    (HEIGHT_FRAME_D1_PAL>>1)
#define	HEIGHT_FRAME_HALF_D1_NTSC   (HEIGHT_FRAME_D1_NTSC>>1)
#define	HEIGHT_FRAME_VGA            (480)
#define	HEIGHT_FRAME_CIF_PAL        (HEIGHT_FRAME_4CIF_PAL>>1)
#define	HEIGHT_FRAME_CIF_NTSC       (HEIGHT_FRAME_4CIF_NTSC>>1)
#define	HEIGHT_FRAME_QCIF_PAL       (HEIGHT_FRAME_4CIF_PAL>>2)
#define	HEIGHT_FRAME_QCIF_NTSC      (((HEIGHT_FRAME_4CIF_NTSC>>2)>>4)<<4)
#define	HEIGHT_FRAME_QVGA           (HEIGHT_FRAME_VGA>>1)
#define HEIGHT_FRAME_SVGA           (600)
#define HEIGHT_FRAME_XGA            (768)

#define	MAX_FRAME_WIDTH             (WIDTH_FRAME_1080P)
#define	MIN_FRAME_WIDTH             (WIDTH_FRAME_QCIF_PAL)
#define	MAX_FRAME_HEIGHT            (HEIGHT_FRAME_1080P)
#define	MIN_FRAME_HEIGHT            (HEIGHT_FRAME_QCIF_NTSC)
#define	MAX_FRAME_RATE_PAL          (25)
#define	MAX_FRAME_RATE_NTSC         (30)
#define	MAX_FRAME_RATE_VGA          (60)
#define	MIN_FRAME_RATE              (0)
#define	MAX_GOP                     (65535)
#define	MIN_GOP                     (100)
#define	MAX_I_P_STRIDE              (MAX_GOP)
#define	MIN_I_P_STRIDE              (25)
#define	MASTER_MAX_BIT_RATE         (3000000)
#define	MASTER_MIN_BIT_RATE         (64000)
#define	SUB_MAX_BIT_RATE            (1000000)
#define	SUB_MIN_BIT_RATE            (32000)

#define DEFAULT_I_P_STRIDE          (MIN_I_P_STRIDE)
#define	DEFAULT_GOP_VALUE           (MIN_GOP)
#define	DEFAULT_BITRATE_D1SIZE      (2000000)
#define	DEFAULT_BITRATE_VGASIZE     (2000000)	//for 60 frame
#define	DEFAULT_BITRATE_QVGASIZE    (700000)	//for 60 frame
#define	DEFAULT_BITRATE_CIFSIZE     (1000000)
#define	DEFAULT_BITRATE_QCIFSIZE    (500000)
#define	DEFAULT_QP                  (28)
#define	DEFAULT_LEVEL_IDC           (30)
#define	DEFAULT_FRAMERATE_PAL       (MAX_FRAME_RATE_PAL)
#define	DEFAULT_FRAMERATE_NTSC      (MAX_FRAME_RATE_NTSC)
#define	DEFAULT_FRAMERATE_VGA       (MAX_FRAME_RATE_VGA)


#define	ISIL_LOGIC_VD_CHAN0       (0)
#define	ISIL_LOGIC_VD_CHAN1       (1)
#define	ISIL_LOGIC_VD_CHAN2       (2)
#define	ISIL_LOGIC_VD_CHAN3       (3)
#define	ISIL_LOGIC_VD_CHAN4       (4)
#define	ISIL_LOGIC_VD_CHAN5       (5)
#define	ISIL_LOGIC_VD_CHAN6       (6)
#define	ISIL_LOGIC_VD_CHAN7       (7)
#define	ISIL_LOGIC_VD_CHAN8       (8)
#define	ISIL_LOGIC_VD_CHAN9       (9)
#define	ISIL_LOGIC_VD_CHAN10      (10)
#define	ISIL_LOGIC_VD_CHAN11      (11)
#define	ISIL_LOGIC_VD_CHAN12      (12)
#define	ISIL_LOGIC_VD_CHAN13      (13)
#define	ISIL_LOGIC_VD_CHAN14      (14)
#define	ISIL_LOGIC_VD_CHAN15      (15)
#define	ISIL_CROSS_BUS_LOGIC_VD_CHAN_NUMBER   (16)
#define	ISIL_PHY_VD_CHAN_NUMBER   (16)
#define	ISIL_LOGIC_VD_INVALID     (-1)
#define ISIL_PHY_VJ_CHAN_NUMBER   (16)
#define ISIL_PHY_VP_CHAN_NUMBER   (16)

#define	ISIL_AUDIO_IN_CHAN0_ID          (0)
#define	ISIL_AUDIO_IN_CHAN1_ID          (1)
#define	ISIL_AUDIO_IN_CHAN2_ID          (2)
#define	ISIL_AUDIO_IN_CHAN3_ID          (3)
#define	ISIL_AUDIO_IN_CHAN4_ID          (4)
#define	ISIL_AUDIO_IN_CHAN5_ID          (5)
#define	ISIL_AUDIO_IN_CHAN6_ID          (6)
#define	ISIL_AUDIO_IN_CHAN7_ID          (7)
#define	ISIL_AUDIO_IN_CHAN8_ID          (8)
#define	ISIL_AUDIO_IN_CHAN9_ID          (9)
#define	ISIL_AUDIO_IN_CHAN10_ID         (10)
#define	ISIL_AUDIO_IN_CHAN11_ID         (11)
#define	ISIL_AUDIO_IN_CHAN12_ID         (12)
#define	ISIL_AUDIO_IN_CHAN13_ID         (13)
#define	ISIL_AUDIO_IN_CHAN14_ID         (14)
#define	ISIL_AUDIO_IN_CHAN15_ID         (15)
#define ISIL_AUDIO_IN_CHAN16_ID         (16)
#define	ISIL_AUDIO_OUT_SPEAKER_ID       (17)
#define	ISIL_AUDIO_OUT_PLAYBACK_ID      (18)

#define ISIL_TRUE  1
#define ISIL_FALSE 0

#define ISIL_DBG_DEBUG      4
#define ISIL_DBG_INFO       3
#define ISIL_DBG_WARN       2
#define ISIL_DBG_ERR        1
#define ISIL_DBG_FATAL      0

#define ISIL_DBG_CUR_LEVEL  ISIL_DBG_DEBUG

#define MESSAGE_LOG
#ifndef MESSAGE_LOG
#define ISIL_DBG(level, fmt...) \
    do{ \
	if(level <= ISIL_DBG_CUR_LEVEL){\
	    printk("<%8d><%d>%s, %d:", jiffies_to_msecs(jiffies), level, __FUNCTION__, __LINE__);\
	    printk(fmt);\
	}\
    }while(0)
#else

#define MESSAGE_LEN (0x40000)

#define isil_message_log(fmt...)          \
    do{                                 \
	if(message_log) {               \
	    message_log->end += sprintf(message_log->buf + message_log->end, fmt); \
	    if(message_log->end >= (MESSAGE_LEN - (MESSAGE_LEN>>8))) {\
		message_log->end = 0;                 \
	    }                               \
	    wake_up_interruptible(&message_log->wait);\
	} \
    }while (0)

#define ISIL_DBG(level, fmt...) \
    do{ \
	if(level <= message_level){\
	    isil_message_log("<%8d><%d>%s, %d:", jiffies_to_msecs(jiffies), level, __FUNCTION__, __LINE__);\
	    isil_message_log(fmt);\
	}\
    }while(0)

#endif

#define SWAP32(x) \
    ((u32)( \
	(((u32)(x) & (u32)0x000000ffUL) << 24) | \
	(((u32)(x) & (u32)0x0000ff00UL) <<  8) | \
	(((u32)(x) & (u32)0x00ff0000UL) >>  8) | \
	(((u32)(x) & (u32)0xff000000UL) >> 24) ))


#define SWAP16(x) \
    ((u16)( \
	(((u16)(x) & (u16)0x00ffUL) << 8) | \
	(((u16)(x) & (u16)0xff00UL) >> 8) ))

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 20)
#define __used __attribute__((used))
#endif
    //come from isil_new_buf.h
    typedef struct tcb_node         tcb_node_t;
    typedef	struct tcb_pool         tcb_node_pool_t;
    typedef struct tcb_node_queue   tcb_node_queue_t;

    //come from chip_bus_schedule.h
    typedef struct notify_msg           isil_notify_msg;
    typedef struct isil_register_node     isil_register_node_t;
    typedef struct register_table_node  isil_register_table_t;
    typedef struct isil_root_bus          isil_root_bus_t;
    typedef struct type_bus             type_bus_t;
    typedef struct isil_chip_bus          isil_chip_bus_t;
    typedef struct robust_processing_control    robust_processing_control_t;

    //come from isil_endpoint.h
    typedef struct endpoint_id          ed_id;
    typedef struct endpoint_tcb         ed_tcb_t;
    typedef struct fsm_state_transfer_matrix_table  fsm_state_transfer_matrix_table_t;
    typedef struct isil_ed_fsm            isil_ed_fsm_t;
    typedef struct hcd_interface_operation  isil_hcd_interface_operation;
    typedef struct	_ISIL_FRAME_TIME_PAD	ISIL_FRAME_TIME_PAD;
    //    typedef struct isil_timestamp         isil_timestamp_t;


    //come from algo_h264_driver.h
    typedef enum nal_priority           nal_priority_e;
    typedef enum nal_unit_type          nal_unit_type_e;
    typedef enum frame_type             frame_type_e;
    typedef enum slice_type             slice_type_e;
    typedef struct h264_nal             h264_nal_t;
#ifdef  ADD_DVM_NAL_HEAD
    typedef struct _tag_isil_nal_head    DVM_NAL_HEAD;
#endif
    typedef struct nal_info             H264_NAL_INFO;
    typedef struct h264_nal_head        h264_nal_bitstream_t;
    typedef struct ext_h264_nal_head    ext_h264_nal_bitstream_t;
    typedef struct h264_sps             h264_sps_t;
    typedef struct h264_pps             h264_pps_t;
    typedef struct h264_slice_header    h264_slice_header_t;

    //come from isil_h264_video_buf.h
    typedef struct insert_03_state_machine      insert_03_state_machine_t;
    typedef struct isil_video_chan_buf_pool       isil_video_chan_buf_pool_t;
    typedef struct isil_video_packet_tcb          isil_video_packet_tcb_t;
    typedef struct isil_video_packet_tcb_queue    isil_video_packet_tcb_queue_t;
    typedef struct isil_video_frame_tcb           isil_video_frame_tcb_t;	
    typedef struct isil_video_frame_tcb_queue     isil_video_frame_tcb_queue_t;
#ifdef  MV_MODULE
    typedef struct isil_video_mv_packet_tcb       isil_video_mv_packet_tcb_t;
    typedef struct isil_video_mv_packet_tcb_queue isil_video_mv_packet_tcb_queue_t;
    typedef struct isil_video_mv_frame_tcb        isil_video_mv_frame_tcb_t;
    typedef struct isil_video_mv_frame_tcb_queue  isil_video_mv_frame_tcb_queue_t;
#endif

    //come from isil_rc.h
    typedef struct cbr_rc_struct            isil_cbr_rc_param_t;
    typedef struct rate_control_driver_op   isil_rc_driver_op;
    typedef struct vbr_rc_struct            isil_vbr_rc_struct_t;
    typedef struct cbr_driver               isil_cbr_driver_t;
    typedef struct vbr_driver               isil_vbr_driver_t;
    typedef struct rc_top_driver            isil_rc_driver_t;

    //come from isil_osd.h
    typedef struct osd_attribute_regs       osd_attribute_regs_t;
    typedef struct osd_attribute_regs_group osd_attribute_regs_group_t;
    typedef struct encode_osd_timer_attr    encode_osd_timer_attr_t;
    typedef struct encode_osd_string_attr   encode_osd_string_attr_t;
    typedef struct encode_osd_mask_attr     encode_osd_mask_attr_t;
    typedef union rectangle_attribute       rectangle_attribute_t;
    typedef struct osd_mb_pool              osd_mb_pool_t;
    typedef struct osd_mb_char_entry        osd_mb_char_entry_t;
    typedef struct osd_rectangle_dpram_tcb  osd_rectangle_dpram_tcb_t;
    typedef struct osd_rectangle_entry      osd_rectangle_entry_t;
    typedef struct osd_chan_engine          osd_chan_engine_t;
    typedef struct _ISIL_OSD_PARAM          ISIL_OSD_PARAM;

    //come from isil_h264_encode.h
    typedef struct isil_timestamp                     isil_timestamp_t;

    typedef struct isil_h264_encode_configuration     isil_h264_encode_configuration_t;
    typedef struct isil_h264_encode_property          isil_h264_encode_property_t;
    typedef struct isil_h264_logic_video_slot         isil_h264_logic_video_slot_t;
    typedef struct vd_chan_map_info                 vd_chan_map_info_t;
    typedef struct isil_vd_cross_bus              isil_vd_cross_bus_t;
    typedef struct isil_vd_orig_buf_info              isil_vd_orig_buf_info_t;
    typedef struct isil_h264_phy_video_slot           isil_h264_phy_video_slot_t;
    typedef struct isil_h264_encode_control           isil_h264_encode_control_t;
    typedef struct encode_chan_service_tcb          encode_chan_service_tcb_t;
    typedef struct chip_encode_chan_service_queue   chip_encode_chan_service_queue_t;
    typedef struct isil_h264_logic_encode_chan        isil_h264_logic_encode_chan_t;
    typedef struct ddr_bitstream_header             ddr_bitstream_header_t;
    typedef struct ddr_video_bitstream_header       ddr_video_bitstream_header_t;

    //come from isil_jpeg_encode.h
    typedef struct isil_jpeg_logic_encode_chan    isil_jpeg_logic_encode_chan_t;

    //come from isil_preview.h
    typedef struct isil_prev_logic_chan         isil_prev_logic_chan_t;

    //come from isil_audio_iop.h
    typedef struct isil_audio_section_descriptor    isil_audio_descriptor_t;
    typedef struct isil_audio_param                 isil_audio_param_t;
    typedef struct audio_config_descriptor          isil_audio_config_descriptor_t;
    typedef struct audio_encode_control             audio_encode_control_t;
    typedef struct audio_pcm_encode_ptr_control     audio_pcm_encode_ptr_control_t;
    typedef struct audio_pcm_encode_data_addr       audio_pcm_encode_data_addr_t;
    typedef struct audio_adpcm_encode_ptr_control   audio_adpcm_encode_ptr_control_t;
    typedef struct audio_adpcm_encode_data_addr     audio_adpcm_encode_data_addr_t;
    typedef struct chip_audio_encode                chip_audio_encode_t;
    typedef struct audio_decode_control             audio_decode_control_t;
    typedef struct audio_decode_ptr_control         audio_decode_ptr_control_t;
    typedef struct audio_decode_data_addr           audio_decode_data_addr_t;
    typedef struct chip_audio_decode                chip_audio_decode_t;
    typedef struct audio_enable_control             audio_enable_control_t;
    typedef struct chip_audio_push_isr_interface	chip_audio_push_isr_interface_t;
    typedef struct chip_audio                       chip_audio_t;

    //come from isil_audio_buf.h
    typedef struct audio_section_descriptor audio_section_descriptor_t;
    typedef struct isil_audio_chan_buf_pool   isil_audio_chan_buf_pool_t;
    typedef struct isil_audio_packet_section  isil_audio_packet_section_t;
    typedef struct isil_audio_packet_queue    isil_audio_packet_queue_t;

    //come from isil_audio_chan_driver.h
    typedef struct isil_audio_chan_push_interface	isil_audio_chan_push_interface_t;
    typedef struct isil_audio_chan_driver     isil_audio_driver_t;
    typedef struct audio_section_ptr_info   audio_section_ptr_info_t;

    //come from ddr_burst_interface.h
    typedef struct ddr_burst_interface_reg      ddr_burst_interface_t;
    typedef struct dpram_page_node              dpram_page_node_t;
    typedef struct dpram_page_resource_manage   dpram_page_resource_manage_t;
    typedef struct ddr_video_bitstream_buf_node ddr_video_bitstream_buf_node_t;
    typedef struct ddr_video_bitstream_resource_manage  ddr_video_bitstream_resource_manage_t;
    typedef struct dpram_request                dpram_request_t;
    typedef struct dpram_request_queue          dpram_request_queue_t;
    typedef struct dpram_control                dpram_control_t;
    typedef struct chip_ddr_burst_engine        chip_ddr_burst_engine_t;


    //come from isil_chip.h
    typedef unsigned int    blockram_addr;
    typedef unsigned int    fpga_reg_addr;

    typedef struct isil4000i_chip   isil_chip_t;
    typedef struct isil_interrupt_control   isil_interrupt_control_t;



    //come from isil_machine.h


    //come from isil_avSync_device_buf.h
    typedef struct avSync_frame_buf_pool    avSync_frame_buf_pool_t;
    typedef struct avSync_frame_tcb         avSync_frame_tcb_t;
    typedef struct avSync_frame_queue       avSync_frame_queue_t;


    //come from isil_avSync_device.h
    typedef struct isil_avSync_dev      isil_avSync_dev_t;

    //come from isil_kthread_msg.h
    typedef struct fortmcb              FORTMCB;
    typedef struct tmcb                 TMCB;
    typedef struct isil_chip_timer_controller   isil_chip_timer_controller_t;
    typedef struct isil_kthread_msg         isil_kthread_msg_t;
    typedef struct isil_kthread_msg_queue   isil_kthread_msg_queue_t;
    typedef struct isil_kthread_msg_pool    isil_kthread_msg_pool_t;
    typedef struct isil_send_msg_controller isil_send_msg_controller_t;

    //come from isil_interface.h
    typedef struct isil_frame_header              isil_frame_header_t;
    typedef struct isil_h264_idr_frame_pad        isil_h264_idr_frame_pad_t;
    typedef struct isil_video_motion_vector_pad   isil_video_motion_vector_pad_t;
    typedef struct isil_audio_frame_pad           isil_audio_frame_pad_t;

    struct isil_service_device;
    struct isil_chip_device;
    struct cmd_arg;
    struct ptr_value;

#include	<isil5864/isil_interface.h>
#include	<isil5864/version.h>

#include	<isil5864/HalSoftTimerLib.h>
#include	<isil5864/dma.h>

#include	<isil5864/isil_new_buf.h>
#include	<isil5864/chip_bus_schedule.h>
#include	<isil5864/isil_endpoint.h>
#include	<isil5864/isil_kthread_msg.h>
#include	<isil5864/ddr_burst_interface.h>
#include	<isil5864/bar_ctl.h>
#include	<isil5864/bs.h>
#include	<isil5864/bitstream.h>
#include	<isil5864/golomb.h>
#include	<isil5864/algo_h264_driver.h>
#include	<isil5864/isil_h264_video_buf.h>
#include	<isil5864/isil_rc.h>
#include        <isil5864/isil_osd.h>
#include	<isil5864/isil_h264_encode.h>

#include	<isil5864/isil_audio_iop.h>
#include	<isil5864/isil_audio_buf.h>
#include	<isil5864/isil_audio_chan_driver.h>

#include        <isil5864/isil_vb_common.h>
#include	<isil5864/isil_jpeg_encode.h>
#include	<isil5864/isil_preview.h>

#include	<isil5864/isil_avSync_device_buf.h>
#include	<isil5864/isil_avSync_device.h>


#include	<isil5864/proc_entry.h>
#include	<isil5864/isil_chip_driver.h>
#include    <isil5864/isil_chip_ai.h>
#include    <isil5864/isil_chip_ao.h>
#include    <isil5864/isil_chip_vi.h>
#include    <isil5864/isil_chip_vo.h>
#include    <isil5864/isil_video_display.h>
#include	<isil5864/tc_common.h>
#include    <isil5864/isil_push_buf_mgt.h>
#include	<isil5864/isil_chip.h>

#ifdef __cplusplus
}
#endif

#endif	//DVM_COMMON_H

