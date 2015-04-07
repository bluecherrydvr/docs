#ifndef	ISIL_H264_ENCODE_H
#define	ISIL_H264_ENCODE_H

#ifdef __cplusplus
extern "C"
{
#endif

#define	AD_IS_READY             (1)
#define	SLICEHEAD_IS_READY      (2)
#define HAVE_HARDWARE_BUF_CACHE (4)
#define	DEV_CAN_SERVICE         (3)
#define FIRST_START_DISCARD_FRAME_NUMBER    (4)
#define	ENCODE_CHAN_TIMEOUT     (100/MS_PER_TICK)
#define ENCODE_CHAN_MONITOR_COUNTER (1)
#define TIMER_1MS               (1/MS_PER_TICK)
#define TIMER_5MS               (5/MS_PER_TICK)
#define TIMER_40MS              (40/MS_PER_TICK)
#define TIMER_200MS             (200/MS_PER_TICK)
#define TIMER_500MS             (500/MS_PER_TICK)
#define TIMER_1_SECOND          (1000/MS_PER_TICK)
#define TIMER_1_MINUTE          (60000/MS_PER_TICK)

#define	VLC_STATUS_NULL         (0)
#define	VLC_STATUS_NEED_PROCESS (1)
#define	VLC_STATUS_OVER         (2)
#define	VLC_NO_PACKET_BUF       (3)

#define READ_BITSTREAM_BY_RAM   (0)
#define READ_BITSTREAM_BY_DDR   (1)
#define READ_BITSTREAM_BY_PCI   (12)
#define WRITE_BITSTREAM_BY_RAM  (2)
#define WRITE_BITSTREAM_BY_DDR  (3)

#define	VIDEO_MASTER_OR_SUB_FLAG_LEFT_SHIFT_NUMBER  (1)
#define VIDEO_LOGIC_MASTER_OR_SUB_MASK              (1)
#define VD_CHANNEL_VALID            (0x9000)
#define VD_CHANNEL_SWITCH           (0x9004)
#define VD_CHANNEL_DOWNSAMPLE       (0x9008)
#define VD_CHANNEL_INTERLACE        (0x900c)
#define VD_BUS_MAX_CHANNEL          (0x9010)
#define VD_BUS_MAX_LINE0            (0x9014)
#define VD_BUS_MAX_LINE1            (0x9018)
#define VD_CHANNEL_FORMAT0          (0x9020)
#define VD_CHANNEL_FORMAT1          (0x9024)
#define VD_CHANNEL_LMAP(pos)        (0x9100 + (pos) * sizeof(int) * 2)
#define VD_CHANNEL_HMAP(pos)        (0x9100 + (pos) * sizeof(int) * 2 + sizeof(int))
#define VD_BUS_MAP(bus)             (0x9200 + (bus) * sizeof(int))

#define FLUSH_CURR_PRODUCER_FRAME   (1)
#define FLUSH_CURR_CONSUMER_FRAME   (2)

    typedef struct isil_h264_encode_feature_param     isil_h264_encode_feature_param_t;
    typedef struct isil_h264_encode_feature           isil_h264_encode_feature_t;
    typedef struct isil_h264_encode_feature_operation isil_h264_encode_feature_operation_t;
    typedef struct isil_h264_encode_rc_config         isil_h264_encode_rc_config_t;
    typedef struct isil_h264_encode_rc_operation      isil_h264_encode_rc_operation_t;
    typedef struct isil_h264_encode_rc                isil_h264_encode_rc_t;
    typedef struct isil_h264_encode_rc_param          isil_h264_encode_rc_param_t;
    typedef struct isil_h264_encode_delta             isil_h264_encode_delta_t;
    enum ISIL_VIDEO_SIZE{
	ISIL_VIDEO_SIZE_QCIF = 0,
	ISIL_VIDEO_SIZE_QHALF_D1,
	ISIL_VIDEO_SIZE_CIF,
	ISIL_VIDEO_SIZE_HALF_D1,
	ISIL_VIDEO_SIZE_D1,
	ISIL_VIDEO_SIZE_HCIF,
	ISIL_VIDEO_SIZE_2CIF,
	ISIL_VIDEO_SIZE_4CIF,
	ISIL_VIDEO_SIZE_VGA,
	ISIL_VIDEO_SIZE_SVGA,
	ISIL_VIDEO_SIZE_XGA,
	ISIL_VIDEO_SIZE_H960,
	ISIL_VIDEO_SIZE_720P,
	ISIL_VIDEO_SIZE_1080P,
	ISIL_VIDEO_SIZE_USER,
    };

    enum ISIL_VIDEO_STANDARD{
	ISIL_VIDEO_STANDARD_PAL,
	ISIL_VIDEO_STANDARD_NTSC,
	ISIL_VIDEO_STANDARD_USER_DEFINE,
    };

    enum ISIL_H264_RC_TYPE{
	ISIL_H264_NO_RC,
	ISIL_H264_CBR,
	ISIL_H264_VBR,
    };

    enum ISIL_RC_IMAGE_PRIORITY{
	ISIL_RC_IMAGE_QUALITY_FIRST = 0,
	ISIL_RC_IMAGE_SMOOTH_FIRST,
    };

    enum ISIL_CROSS_BUS_WORK_MODE{
	ISIL_CROSS_BUS_4D1_REALTIME = 0,
	ISIL_CROSS_BUS_8HalfD1_REALTIME,
	ISIL_CROSS_BUS_16CIF_REALTIME,
	ISIL_CROSS_BUS_UNREALTIME,
    };

enum ISIL_VIDEO_FIELD{
    ISIL_VIDEO_FIELD_ODD = 0,
    ISIL_VIDEO_FIELD_EVEN,
    ISIL_VIDEO_FIELD_BOTH,
    ISIL_VIDEO_FIELD_RESERVED,
};

enum ISIL_FRAME_MODE{
    ISIL_FRAME_MODE_INTERLACE = 0,
    ISIL_FRAME_MODE_PROGRESSIVE,
    ISIL_FRAME_MODE_RESERVED,
};


    /*codec cmd*/
#define	DVM_CODEC_GET_VIDEO_ENCODER_PARAM       _IOR(ISIL_CODEC_IOC_MAGIC, 0, unsigned int)
#define	DVM_CODEC_SET_VIDEO_ENCODER_PARAM       _IOW(ISIL_CODEC_IOC_MAGIC, 1, unsigned int)

#define	DVM_CODEC_SET_VIDEO_TIMESTAMP_BASE      _IOW(ISIL_CODEC_IOC_MAGIC, 2, unsigned int)
#define	DVM_CODEC_GET_VIDEO_TIMESTAMP_BASE      _IOR(ISIL_CODEC_IOC_MAGIC, 3, unsigned int)

    /*motion detection cmd*/
#define	DVM_MOTION_DECTION_GET_PARAM            _IOR(ISIL_CODEC_IOC_MAGIC, 4, unsigned int)
#define	DVM_MOTION_DECTION_SET_PARAM            _IOW(ISIL_CODEC_IOC_MAGIC, 5, unsigned int)

    /*osd cmd*/
#define	ISIL_LOGIC_CHAN_OSD_GET_PARAM           _IOR(ISIL_CODEC_IOC_MAGIC, 6, unsigned int)
#define	ISIL_LOGIC_CHAN_OSD_SET_PARAM           _IOW(ISIL_CODEC_IOC_MAGIC, 7, unsigned int)
#define ISIL_H264_ENCODE_CHAN_MAP_GET           _IOR(ISIL_CODEC_IOC_MAGIC, 8, unsigned int)
#define ISIL_LOGIC_CHAN_ENABLE_SET              _IOW(ISIL_CODEC_IOC_MAGIC, 9, unsigned int)
#define ISIL_LOGIC_CHAN_DISABLE_SET             _IOW(ISIL_CODEC_IOC_MAGIC, 10, unsigned int)
#define ISIL_H264_ENCODE_FEATURE_SET            _IOW(ISIL_CODEC_IOC_MAGIC, 11, unsigned int)
#define ISIL_H264_ENCODE_FEATURE_GET            _IOR(ISIL_CODEC_IOC_MAGIC, 12, unsigned int)

#define ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_DEINTERLACE_MASK      0x00000001
#define ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_SKIP_MASK             0x00000002
#define ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_I_4X4_MASK            0x00000004
#define ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_HALF_PIXEL_MASK       0x00000008
#define ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_QUARTER_PIXEL_MASK    0x00000010
#define ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_MB_DELAY_MASK         0x00000020
#define ISIL_H264_ENCODE_FEATURE_ENABLE_CHANGE_MASK_ALL              0x0000003f

#define ISIL_H264_FEATURE_ON      1
#define ISIL_H264_FEATURE_OFF     0

#define ISIL_H264_ENCODE_RC_SET           _IOW(ISIL_CODEC_IOC_MAGIC, 13, unsigned int)
#define ISIL_H264_ENCODE_RC_GET           _IOR(ISIL_CODEC_IOC_MAGIC, 14, unsigned int)
#define ISIL_H264_ENCODE_RC_PARAM_SET     _IOW(ISIL_CODEC_IOC_MAGIC, 15, unsigned int)
#define ISIL_H264_ENCODE_RC_PARAM_GET     _IOR(ISIL_CODEC_IOC_MAGIC, 16, unsigned int)

#define ISIL_H264_ENCODE_RC_ENABLE_CHANGE_RC_TYPE_MASK              0x00000001
#define ISIL_H264_ENCODE_RC_ENABLE_CHANGE_IMAGE_PRIORITY_MASK       0x00000002
#define ISIL_H264_ENCODE_RC_ENABLE_CHANGE_QPI_MASK                  0x00000004
#define ISIL_H264_ENCODE_RC_ENABLE_CHANGE_QPP_MASK                  0x00000008
#define ISIL_H264_ENCODE_RC_ENABLE_CHANGE_QPB_MASK                  0x00000010
#define ISIL_H264_ENCODE_RC_ENABLE_CHANGE_ALL_MASK                  0x0000001f

    struct isil_h264_chan_map_info{
	u16 i_logic_chan_id;
	u16 i_phy_chan_id;
    };

    struct isil_h264_encode_rc_config{
	u32 change_mask_flag;
	u32 e_rc_type;
	u32 e_image_priority;
	u32 i_qpi;
	u32 i_qpp;
	u32 i_qpb;
    };

    struct isil_h264_encode_rc_operation{
	int (*init)(isil_h264_encode_rc_t *);
	int (*reset)(isil_h264_encode_rc_t *);

	u32 (*get_rc_type)(isil_h264_encode_rc_t *);
	u32 (*get_image_priority)(isil_h264_encode_rc_t *);
	u32 (*get_qpi)(isil_h264_encode_rc_t *);
	u32 (*get_qpp)(isil_h264_encode_rc_t *);
	u32 (*get_qpb)(isil_h264_encode_rc_t *);
	int (*is_qpi_changed)(isil_h264_encode_rc_t *);
	int (*is_qpp_changed)(isil_h264_encode_rc_t *);
	int (*is_qpb_changed)(isil_h264_encode_rc_t *);
	int (*is_rc_type_changed)(isil_h264_encode_rc_t *);
	int (*is_image_priority_changed)(isil_h264_encode_rc_t *);
    };

    struct isil_h264_encode_rc{
	isil_h264_encode_rc_config_t config_rc;
	isil_h264_encode_rc_config_t running_rc;

	isil_h264_encode_rc_operation_t *op;
    };

    struct isil_h264_encode_rc_param{
	u32 e_rc_type;
	u8  rc_param[0];
    };

    struct isil_h264_encode_feature_param{
	u32 change_mask_flag;
	u8  b_enable_deinterlace;
	u8  b_enable_skip;
	u8  b_enable_I_4X4;
	u8  b_enable_half_pixel;
	u8  b_enable_quarter_pixel;
	u32 i_mb_delay_value;
    };

    struct isil_h264_encode_feature_operation{
	int (*init)(isil_h264_encode_feature_t *);
	int (*reset)(isil_h264_encode_feature_t *);

	int (*get_deinterlace)(isil_h264_encode_feature_t *);
	int (*get_skip)(isil_h264_encode_feature_t *);
	int (*get_i_4x4)(isil_h264_encode_feature_t *);
	int (*get_half_pixel)(isil_h264_encode_feature_t *);
	int (*get_quarter_pixel)(isil_h264_encode_feature_t *);
	u32 (*get_mb_delay)(isil_h264_encode_feature_t *);
    };

    struct isil_h264_encode_feature{
	isil_h264_encode_feature_param_t config_feature_param;
	isil_h264_encode_feature_param_t running_feature_param;

	struct isil_h264_encode_feature_operation *op;
    };

    enum ISIL_ENCODE_CONFIG_CHANGED_MASK
    {
	ISIL_ENCODE_CONFIG_ENCODE_SIZE_CHANGED        = 0x00000004,
	ISIL_ENCODE_CONFIG_BITRATE_CHANGED            = 0x00000008,
	ISIL_ENCODE_CONFIG_FPS_CHANGED                = 0x00000010,
	ISIL_ENCODE_CONFIG_KEYFRAME_INTERVALS_CHANGED = 0x00000020,
	ISIL_ENCODE_CONFIG_GOP_INTERVALS_CHANGED      = 0x00000040,
	ISIL_ENCODE_CONFIG_IMAGE_LEVEL_CHANGED        = 0x00000100,
    };

#define ISIL_h264_ENCODER_PARAM_BPS_MASK          0x0001
#define ISIL_h264_ENCODER_PARAM_FPS_MASK          0x0002
#define ISIL_h264_ENCODER_PARAM_IP_STRIDE_MASK    0x0004
#define ISIL_h264_ENCODER_PARAM_PB_STRIDE_MASK    0x0008
#define ISIL_h264_ENCODER_PARAM_GOP_MASK          0x0010
#define ISIL_h264_ENCODER_PARAM_FORCE_I_MASK      0x0020
#define ISIL_h264_ENCODER_PARAM_WIDTH_MASK        0x0040
#define ISIL_h264_ENCODER_PARAM_HEIGHT_MASK       0x0080

    struct isil_h264_encode_param{
	u32 change_mask_flag;
	u32 i_bps;
	u32 i_fps;
	u32 i_I_P_stride;
	u32 i_P_B_stride;
	u32 i_gop_value;
	u32 b_force_I_flag;
	u16 i_logic_video_width_mb_size;
	u16 i_logic_video_height_mb_size;
    };

#define	ISIL_ENCODE_CONFIG_MASKALL	(0xfff)
#define	ISIL_ENCODE_PROPERTY_CHANGED		(1)
#define	ISIL_ENCODE_PROPERTY_NO_CHANGED	(0)

    struct isil_h264_encode_configuration
    {
	u32	changedMask;						/*标示域，用于说明那些需要更改*/
	int	enableSubmitMotionVector;			/*MV数据提交的控制开关*/
	int	enableMotionVectorAnalyses;			/*MV算法的控制开关*/
	int	width;								/*图像的宽，仅在使用ISIL_VIDEO_SIZE_USER有效*/
	int	height;								/*图像的高，仅在使用ISIL_VIDEO_SIZE_USER有效*/
	int	bitrate; 							/*编码的目标码率*/
	int	fps;								/*编码的目标帧率,可能小于i_crossbus_fps*/
	int	keyFrameIntervals;					/*I帧的间隔*/
	int	gopIntervals;						/*GOP帧的间隔*/
	int	pri;
	int	left_pixel_x, left_pixel_y;
	int	right_pixel_x, right_pixel_y;
	int	imageLevel;							/*图像质量等级*/
    };

    struct isil_h264_encode_property_operation{
	int	(*init)(isil_h264_encode_property_t *);
	int	(*reset)(isil_h264_encode_property_t *);

	int (*is_encodeSize_changed)(isil_h264_encode_property_t *);
	int	(*get_encodeSize_width)(isil_h264_encode_property_t *);
	int	(*get_encodeSize_height)(isil_h264_encode_property_t *);
	int	(*is_target_bitrate_changed)(isil_h264_encode_property_t *);
	int	(*get_target_bitrate)(isil_h264_encode_property_t *);
	int	(*is_target_fps_changed)(isil_h264_encode_property_t *);
	int	(*get_target_fps)(isil_h264_encode_property_t *);
	int	(*is_keyFrameIntervals_changed)(isil_h264_encode_property_t *);
	int	(*get_keyFrameIntervals)(isil_h264_encode_property_t *);
	int	(*is_gopIntervals_changed)(isil_h264_encode_property_t *);
	int	(*get_gopIntervals)(isil_h264_encode_property_t *);
	int	(*is_iamgeLevel_changed)(isil_h264_encode_property_t *);
	int	(*get_iamgeLevel)(isil_h264_encode_property_t *);
    };

    struct isil_h264_encode_property{
	isil_h264_encode_configuration_t      config_encode_property;
	isil_h264_encode_configuration_t      running_encode_property;
	isil_h264_encode_feature_t            encode_feature;
	isil_h264_encode_rc_t                 encode_rc;
	spinlock_t	lock;
	struct isil_h264_encode_property_operation	*op;
    };

    struct isil_vd_orig_buf_info_operation{
	void    (*init)(isil_vd_orig_buf_info_t *, int slot_id);
	void    (*first_sync_orig_buf_info)(isil_vd_orig_buf_info_t *, isil_chip_t *);
	int     (*vd_orig_buf_ready)(isil_vd_orig_buf_info_t *, isil_chip_t *, isil_h264_logic_encode_chan_t *);
	int     (*clear_vd_orig_buf_ready)(isil_vd_orig_buf_info_t *, isil_chip_t *, isil_h264_logic_encode_chan_t *);
	int     (*get_vd_ad_phy_ptr)(isil_vd_orig_buf_info_t *, isil_chip_t *chip);
	void    (*set_vd_update_orig_buf_reg)(isil_vd_orig_buf_info_t *, isil_chip_t *chip);
	void    (*set_vd_curr_encode_ptr)(isil_vd_orig_buf_info_t *, isil_chip_t *chip, isil_h264_logic_encode_chan_t *logic_encode_chan);
	void    (*update_vd_encode_ptr)(isil_vd_orig_buf_info_t *);
	void    (*update_timestamp)(isil_vd_orig_buf_info_t *, isil_chip_t *chip);
	u32     (*get_timestamp)(isil_vd_orig_buf_info_t *);
	void    (*software_discard_frame)(isil_vd_orig_buf_info_t *, isil_chip_t *chip, isil_h264_logic_encode_chan_t *);
	void    (*encode_endofslice_notify)(isil_vd_orig_buf_info_t *, isil_h264_logic_encode_chan_t *);
	void    (*encode_unregister_notify)(isil_vd_orig_buf_info_t *, isil_h264_logic_encode_chan_t *);
    };

    struct isil_vd_orig_buf_info{
	atomic_t    stop_check_ad;
	spinlock_t  lock;
	int	phy_slot_id;
	int	first_start_discard_frame_number;
	u32	vd_orig_ad_phy_ptr;
        u32     vd_orig_ad_last_phy_ptr;
        u32     vd_toggle_orig_ad_phy_ptr;
	u32	vd_update_orig_ad_phy_ptr;
	u32	vd_encode_ptr;
        u32     vd_toggle_encode_ptr;
	isil_timestamp_t  vd_encode_timestamp;
	int	vd_ptr_check_count;
	int ptr_no_running_count;
	u32	logic_chan_ad_ready_map_table;
	u32	logic_chan_encode_over_map_table;

	struct isil_vd_orig_buf_info_operation    *op;
    };

    struct isil_adaptive_deinterlace_control;
    struct isil_adaptive_deinterlace_control_operation{
	int (*init)(struct isil_adaptive_deinterlace_control *);
	int (*reset)(struct isil_adaptive_deinterlace_control *);

	int (*adjust_sensitive)(struct isil_adaptive_deinterlace_control *, u32);
	int (*update)(struct isil_adaptive_deinterlace_control *);
	int (*detect)(struct isil_adaptive_deinterlace_control *, isil_chip_t *chip, u32);
	int (*ischange)(struct isil_adaptive_deinterlace_control *);
    };

    struct isil_adaptive_deinterlace_control{
	u32 last_flags;
	u32 curr_flags;

	u32 sensitive;
	u32 enable;

	struct isil_adaptive_deinterlace_control_operation *op;
    };

    struct isil_h264_phy_video_slot_operation{
	void    (*init)(isil_h264_phy_video_slot_t *, int , enum ISIL_VIDEO_SIZE );
	void    (*slot_reset)(isil_h264_phy_video_slot_t *, isil_chip_t *);
	void    (*slot_software_discard_frame)(isil_h264_phy_video_slot_t *, isil_chip_t *, isil_h264_logic_encode_chan_t *);
	int     (*slot_ad_is_ready)(isil_h264_phy_video_slot_t *, isil_chip_t *, isil_h264_logic_encode_chan_t *);
	void    (*slot_clear_ad_ready)(isil_h264_phy_video_slot_t *, isil_chip_t *, isil_h264_logic_encode_chan_t *);
	void    (*slot_first_start)(isil_h264_phy_video_slot_t *, isil_chip_t *);
	void    (*slot_end_stop)(isil_h264_phy_video_slot_t *, isil_h264_logic_encode_chan_t *);
	void    (*set_slot_encode_ptr)(isil_h264_phy_video_slot_t *, isil_chip_t *, isil_h264_logic_encode_chan_t *);
	u32     (*get_timestamp)(isil_h264_phy_video_slot_t *);
	void    (*encode_endofslice_notify)(isil_h264_phy_video_slot_t *, isil_h264_logic_encode_chan_t *);
	void    (*encode_unregister_notify)(isil_h264_phy_video_slot_t *, isil_h264_logic_encode_chan_t *);
	void    (*update_map_logic_slot)(isil_h264_phy_video_slot_t *, int , isil_h264_logic_video_slot_t *);
	int     (*get_video_size)(isil_h264_phy_video_slot_t *);
	void    (*set_video_size)(isil_h264_phy_video_slot_t *, enum ISIL_VIDEO_SIZE video_size);
        int     (*get_interlace)(isil_h264_phy_video_slot_t *);
        int     (*set_interlace)(isil_h264_phy_video_slot_t *, enum ISIL_FRAME_MODE);
	void    (*get_phy_slot_by_phy_slot_id)(isil_chip_t *, int phy_slo_id, isil_h264_phy_video_slot_t **);
    };

#define to_get_h264_phy_video_slot_width_vd_orig_buf(node) container_of(node, isil_h264_phy_video_slot_t, vd_orig_buf)
    struct isil_h264_phy_video_slot{
	atomic_t    first_start_flag;
	int         phy_slot_id;
	int         phy_bus_id;
	int         map_logic_id;
	isil_h264_logic_video_slot_t  *cross_bus_logic_video_slot;
	isil_vd_orig_buf_info_t   vd_orig_buf;
	enum ISIL_VIDEO_SIZE      video_size;		    //该通道视频信号进入錭crossbus前端的图像尺寸
        enum ISIL_FRAME_MODE    interlace;
        enum ISIL_VIDEO_SIZE   capability;

	struct isil_h264_phy_video_slot_operation *op;

	isil_register_table_t logic_chan_table;
	isil_register_table_t opened_logic_chan_table;

	struct isil_adaptive_deinterlace_control deinterlace; /*detection interlace flags*/

	int     encode_chan_monitor_timeid;
	void    (*start_encode_chan_monitor_hook)(isil_h264_phy_video_slot_t *, isil_chip_t *);
	void    (*delete_encode_chan_monitor_hook)(isil_h264_phy_video_slot_t *, isil_chip_t *);
    };

    struct isil_h264_logic_video_slot_operation{
	void    (*init)(isil_h264_logic_video_slot_t *, int logic_slot_id, enum ISIL_VIDEO_SIZE video_size);
	void    (*set_mapPhySlotId)(isil_h264_logic_video_slot_t *, int phy_slot_id);
	int     (*get_mapPhySlotId)(isil_h264_logic_video_slot_t *);
	void    (*set_discardTable)(isil_h264_logic_video_slot_t *, u32 discardTable);
	u32     (*get_discardTable)(isil_h264_logic_video_slot_t *);
	void    (*set_roundTableStride)(isil_h264_logic_video_slot_t *, int roundTableStride);
	int     (*get_roundTableStride)(isil_h264_logic_video_slot_t *);
	void    (*clear_needUpdateFlag)(isil_h264_logic_video_slot_t *);
	int     (*get_needUpdateFlag)(isil_h264_logic_video_slot_t *);
	int     (*get_video_size)(isil_h264_logic_video_slot_t *);
	void    (*set_video_size)(isil_h264_logic_video_slot_t *, enum ISIL_VIDEO_SIZE video_size);
	u32     (*get_fps)(isil_h264_logic_video_slot_t *);
	void    (*set_fps)(isil_h264_logic_video_slot_t *, u32);
	u32     (*get_enableslot)(isil_h264_logic_video_slot_t *);
	void    (*set_enableslot)(isil_h264_logic_video_slot_t *, u32 );

	void    (*set_chip_end_logic_video_slot_param)(isil_h264_logic_video_slot_t *, isil_chip_t *);
	u32     (*get_chip_end_logic_video_slot_param)(isil_h264_logic_video_slot_t *, isil_chip_t *);
    };

    struct isil_h264_logic_video_slot{
	u32	logicSlotId;
	u32	mapPhySlotId;
	u32 enableSlot;
	u32 slotFps;
	u32	discardTable;
	u32	roundTableStride;
	u32 hor_reverse;    /*Horizontal reverse*/
	u32 ver_reverse;    /*Vertical   reverse*/
	enum ISIL_VIDEO_SIZE	video_size;				//来自映射后的物理通道的图像尺寸配置信息
	atomic_t	needUpdateFlag;

	struct isil_h264_logic_video_slot_operation	*op;
    };

    struct vd_chan_map_info{
	u32	phy_slot_id;
	u32	map_logic_slot_id;
	u32 enable;
	u32	fps;
	u32	roundTableStride;
	enum ISIL_VIDEO_SIZE	video_size;	
	u32	logic_slot_discard_table;
        enum ISIL_FRAME_MODE interlace;
    };

    struct isil_vd_cross_bus_operation{
	int	 (*init)(isil_vd_cross_bus_t *, enum ISIL_CROSS_BUS_WORK_MODE mode, enum ISIL_VIDEO_STANDARD video_standard);
	int  (*reset)(isil_vd_cross_bus_t *);
	void (*get_logic_video_slot_by_logic_id)(isil_vd_cross_bus_t *, int logic_slot_id, isil_h264_logic_video_slot_t **);
	void (*get_logic_video_slot_by_phy_id)(isil_vd_cross_bus_t *, int slot_id, isil_h264_logic_video_slot_t **);
	void (*get_phy_video_slot_by_logic_id)(isil_vd_cross_bus_t *, int logic_slot_id, isil_h264_phy_video_slot_t **);
	void (*get_phy_video_slot_by_phy_id)(isil_vd_cross_bus_t *, int slot_id, isil_h264_phy_video_slot_t **);

	int	 (*get_work_mode)(isil_vd_cross_bus_t *);
	int  (*update_work_mode)(isil_vd_cross_bus_t *, enum ISIL_CROSS_BUS_WORK_MODE mode);

	int	 (*calculate_cross_bus_map_table)(isil_vd_cross_bus_t *, vd_chan_map_info_t *);
	int	 (*set_chip_end_cross_bus)(isil_vd_cross_bus_t *, isil_chip_t *);
    };

    struct isil_vd_cross_bus{
	enum ISIL_CROSS_BUS_WORK_MODE	mode;
	enum GROUP_MODE             bus_mode[ISIL_MAX_GRP];
	int		discard_stride_limit;
	isil_h264_logic_video_slot_t	logic_video_slot[ISIL_CROSS_BUS_LOGIC_VD_CHAN_NUMBER];
	isil_h264_phy_video_slot_t	phy_video_slot[ISIL_PHY_VD_CHAN_NUMBER];

	struct isil_vd_cross_bus_operation	*op;
    };

    struct isil_h264_encode_control_operation{
	void    (*init)(isil_h264_encode_control_t *);
	void    (*reset)(isil_h264_encode_control_t *);

	void    (*set_sps)(isil_h264_encode_control_t*);
	void    (*set_pps)(isil_h264_encode_control_t *);
	void    (*set_slice_head)(isil_h264_encode_control_t *);
	void    (*generator_head)(isil_h264_encode_control_t *, isil_video_frame_tcb_t *frame);

	void    (*calculate_frame_type)(isil_h264_encode_control_t *);
	int     (*get_frame_type)(isil_h264_encode_control_t *);
	void    (*force_I_frame)(isil_h264_encode_control_t *);
	void    (*flush_all_frame)(isil_h264_encode_control_t *);
	void    (*flush_curr_producer_frame)(isil_h264_encode_control_t *);
	void    (*flush_curr_consumer_frame)(isil_h264_encode_control_t *);
    void    (*flush_frame_queue)(isil_h264_encode_control_t *);
	void    (*update_at_encode_frame_ok)(isil_h264_encode_control_t *);
	void    (*update_at_discard_frame)(isil_h264_encode_control_t *);
	void    (*update_at_encode_frame_err)(isil_h264_encode_control_t *);
	void    (*update_image_resolution)(isil_h264_encode_control_t *);
	void    (*update_fps_discard_table)(isil_h264_encode_control_t *, u32 , u32);

	int     (*get_slice_head_and_ad_status)(isil_h264_encode_control_t *);
	void    (*clear_slice_head_and_ad_status)(isil_h264_encode_control_t *);
	void    (*update_slice_head_ready)(isil_h264_encode_control_t *);
	int     (*get_slice_head_status)(isil_h264_encode_control_t *);
	void    (*clear_slice_head_ready)(isil_h264_encode_control_t *);
	void    (*update_ad_ready)(isil_h264_encode_control_t *);
	void    (*clear_ad_ready)(isil_h264_encode_control_t *);
	int     (*get_ad_status)(isil_h264_encode_control_t *);

	u32     (*get_timestamp)(isil_h264_encode_control_t *);
	void    (*first_start)(isil_h264_encode_control_t *);
	void    (*start_encode)(isil_h264_encode_control_t *);
	void    (*stop_encode)(isil_h264_encode_control_t *);

	int     (*irq_func)(int irq, void *);
	void    (*read_vlc_param)(isil_h264_encode_control_t *, isil_chip_t *chip);
	void    (*calculate_mov_param)(isil_h264_encode_control_t *, isil_chip_t *chip);
	void    (*mov_vlc_coding)(isil_h264_encode_control_t *h264_encode_control, isil_h264_logic_encode_chan_t *h264_logic_encode_chan, isil_chip_t *chip);
	int     (*chip_ping_pong_update)(isil_h264_encode_control_t *h264_encode_control, isil_h264_logic_encode_chan_t *h264_logic_encode_chan, isil_chip_t *chip);
	int     (*notify)(isil_h264_encode_control_t *h264_encode_control, isil_h264_logic_encode_chan_t *h264_logic_encode_chan, isil_chip_t *chip);
	int     (*notify_timeout)(isil_h264_encode_control_t *h264_encode_control, isil_h264_logic_encode_chan_t *h264_logic_encode_chan, isil_chip_t *chip);
    };

    struct isil_h264_encode_delta{
	struct timeval last_time;
	struct timeval curr_time;

	u32 max_delta;
	u32 cur_delta;
	u32 min_delta;
    };
    struct isil_h264_encode_control {
	spinlock_t  slice_head_and_ad_status_lock;
	atomic_t    slice_head_and_ad_status;
	int         frame_type;

	atomic_t    enable; /*channel enable*/

	int b_force_i_frame;
	int b_flush_all_frame;
        u32 i_frameNumber;
	u32 i_frame_serial;
	int i_slice_frame_num;
	int i_crossbus_fps;					//是实际硬件上能够通过的帧率
	int i_software_discard_frame_table;
	int i_discard_frame_number;
	int i_last_frame_per_second;
	int i_frame_per_second;
	int i_osd_submit_frame_position;
	int i_p_stride;
	int i_gop_stride;
	int i_statistic_minuter_counter;
	u64 last_jiffies;
	u32 i_curr_fps;
	u32 i_old_fps;
	int i_fpm;
	u32 i_fps;
	int ad_ready;

	int i_last_idr;
	int i_idr_pic_id;
	int i_mb_x;
	int i_mb_y;
	int i_first_mb;
	int i_first_mb_x_and_y;
	int i_curr_lamda;
	int i_curr_qp;
	int i_vd_reference_ptr;

	int master_or_sub;
	int sen_type;
	int need_large_mbdelay;
	int encodeSize;

	u32 total_frame_len;
	int frame_base_addr;
	int section_number;
	int have_recv_section_number;
	int section_len;
	int tailer_len;

	h264_sps_t  sps_array[1];
	h264_pps_t  pps_array[1];
	h264_slice_header_t slice_head_array[1];
	h264_sps_t  *sps;
	h264_pps_t  *pps;
	h264_slice_header_t *slice_head;

	int	overflow;
	u32	vlc_status;
	u32	vlcIntrLen;

	isil_h264_encode_delta_t    encode_delta;
	struct isil_h264_encode_control_operation   *op;
    };

    enum ISIL_REQ_TYPE{
	DVM_CHIP_REQ_FIRST_STRAT_CHAN,
	DVM_CHIP_REQ_START_CHAN,
    };

#define	to_get_encode_chan_service_tcb_with_service_tcb(node)	container_of(node, encode_chan_service_tcb_t, service_tcb)
    struct encode_chan_service_tcb{
	tcb_node_t	service_tcb;
	enum ISIL_REQ_TYPE    type;
	void    *context;
	void    (*req_callback)(void *context);
        void    (*delete_req_notify)(void *context);
    };

    struct chip_encode_chan_service_queue_operation{
	void    (*init)(chip_encode_chan_service_queue_t *);
	int     (*get_queue_curr_entry_number)(chip_encode_chan_service_queue_t *);
	void    (*put_service_request_into_queue)(chip_encode_chan_service_queue_t *, encode_chan_service_tcb_t *);
	void    (*put_service_request_into_queue_header)(chip_encode_chan_service_queue_t *, encode_chan_service_tcb_t *);
	int     (*delete_service_request_from_queue)(chip_encode_chan_service_queue_t *, encode_chan_service_tcb_t *);
	void    (*try_get_curr_consumer_from_queue)(chip_encode_chan_service_queue_t *);
	void    (*release_curr_consumer)(chip_encode_chan_service_queue_t *);
	void    (*trigger_chip_pending_service_request)(chip_encode_chan_service_queue_t *);
    };

    struct chip_encode_chan_service_queue{
	spinlock_t  lock;
	atomic_t    h264_master_can_recv_number;
	atomic_t    h264_ddr_cache_number;
	tcb_node_queue_t    service_queue_node;
	encode_chan_service_tcb_t   *curr_consumer;

	struct chip_encode_chan_service_queue_operation *op;
    };

#define	to_get_isil_h264_encode_chan_with_logic_chan_ed(node)         container_of(node, isil_h264_logic_encode_chan_t, logic_chan_ed)
#define	to_get_isil_h264_encode_chan_with_opened_logic_chan_ed(node)  container_of(node, isil_h264_logic_encode_chan_t, opened_logic_chan_ed)
#define	to_get_isil_h264_encode_chan_with_vd_ref(node)                container_of(node, isil_h264_logic_encode_chan_t, vd_ref)
#define	to_get_isil_h264_encode_chan_with_encode_property(node)       container_of(node, isil_h264_logic_encode_chan_t, encode_property)
#define	to_get_isil_h264_encode_chan_with_encode_control(node)        container_of(node, isil_h264_logic_encode_chan_t, encode_control)
#define to_get_isil_h264_encode_chan_with_osd_engine(node)            container_of(node, isil_h264_logic_encode_chan_t, encode_osd_engine)
#define	to_get_isil_h264_encode_chan_with_encode_request_tcb(node)    container_of(node, isil_h264_logic_encode_chan_t, encode_request_tcb)
    struct isil_h264_logic_encode_chan
    {
	int phy_slot_id;
	int logic_chan_id;
	int type;
	int read_bitstream_mode;
	atomic_t    opened_flag;
	atomic_t    first_start_flag;
	robust_processing_control_t h264_encode_chan_robust;
	ed_tcb_t    logic_chan_ed;
	ed_tcb_t    opened_logic_chan_ed;
        spinlock_t  fsm_matrix_call_lock;
	isil_send_msg_controller_t  send_msg_contr;

	isil_chip_t                 *chip;
	isil_avSync_dev_t           *isil_device_chan;
	isil_h264_phy_video_slot_t  *master_slot;

	isil_h264_encode_property_t encode_property;
	isil_h264_encode_control_t  encode_control;
	osd_chan_engine_t           encode_osd_engine;
	isil_rc_driver_t            rc_driver;
	encode_chan_service_tcb_t   encode_request_tcb;
	dpram_request_t             read_video_bitstream_req;
	dpram_page_node_t           *dpram_page;
	ddr_video_bitstream_buf_node_t  *video_bitstream_buf;

	isil_video_chan_buf_pool_t      encode_chan_buf_pool;
	isil_video_frame_tcb_queue_t    encode_frame_queue;
#ifdef  MV_MODULE
	isil_video_mv_frame_tcb_queue_t encode_mv_frame_queue;
	dpram_request_t                 read_mvflag_req;
	struct completion               mvflag_transfer_done;
#endif
	u8 *push_buf;
	u8 *mv_push_buf;
	dma_addr_t	 h264_push_dma_addr, mv_push_dma_addr;
    unsigned long   if_slience_discard;
    };

    struct ddr_bitstream_header{
	unsigned char   valid;
	unsigned char   masterOrSub;
	unsigned char   frameType;
	unsigned char   chan_id_and_orig_id;
	unsigned int    offset;
	unsigned int    len;
	unsigned int    timeStamp;
	unsigned int    reserver1;
	unsigned int    reserver2;
	unsigned int    reserver3;
	unsigned int    reserver4;
    };

    struct ddr_video_bitstream_header{
	ddr_bitstream_header_t  mvFlags_header;
	ddr_bitstream_header_t  mvVextor_header;
	ddr_bitstream_header_t  h264_bitstream_header;
    };

    static u32 inline VIDEO_SIZE_TO_WIDTH(enum ISIL_VIDEO_SIZE size, enum ISIL_VIDEO_STANDARD standard)
    {
	switch (size) {
	    case ISIL_VIDEO_SIZE_QCIF:
		return WIDTH_FRAME_QCIF_PAL;
	    case ISIL_VIDEO_SIZE_CIF:
		return WIDTH_FRAME_CIF_PAL;        
	    case ISIL_VIDEO_SIZE_2CIF:
		return WIDTH_FRAME_2CIF_PAL;
	    case ISIL_VIDEO_SIZE_4CIF:
		return WIDTH_FRAME_4CIF_PAL;
	    case ISIL_VIDEO_SIZE_HALF_D1:
		return WIDTH_FRAME_HALF_D1_PAL;
	    case ISIL_VIDEO_SIZE_D1:
		return WIDTH_FRAME_D1_PAL;
	    case ISIL_VIDEO_SIZE_VGA:
		return WIDTH_FRAME_VGA;
	    case ISIL_VIDEO_SIZE_SVGA:
		return WIDTH_FRAME_SVGA;
	    case ISIL_VIDEO_SIZE_XGA:
		return WIDTH_FRAME_XGA;
	    case ISIL_VIDEO_SIZE_720P:
		return WIDTH_FRAME_720P;
	    case ISIL_VIDEO_SIZE_1080P:
		return WIDTH_FRAME_1080P;
	    case ISIL_VIDEO_SIZE_QHALF_D1:
	    case ISIL_VIDEO_SIZE_USER:
	    default:
		return WIDTH_FRAME_CIF_PAL;
	}
    }

    static u32 inline VIDEO_SIZE_TO_HEIGHT(enum ISIL_VIDEO_SIZE size, enum ISIL_VIDEO_STANDARD standard)
    {
	if(standard == ISIL_VIDEO_STANDARD_PAL) {
	    switch (size) {
		case ISIL_VIDEO_SIZE_QCIF:
		    return HEIGHT_FRAME_QCIF_PAL;
		case ISIL_VIDEO_SIZE_CIF:
		    return HEIGHT_FRAME_CIF_PAL;        
		case ISIL_VIDEO_SIZE_2CIF:
		    return HEIGHT_FRAME_2CIF_PAL;
		case ISIL_VIDEO_SIZE_4CIF:
		    return HEIGHT_FRAME_4CIF_PAL;
		case ISIL_VIDEO_SIZE_HALF_D1:
		    return HEIGHT_FRAME_HALF_D1_PAL;
		case ISIL_VIDEO_SIZE_D1:
		    return HEIGHT_FRAME_D1_PAL;
		case ISIL_VIDEO_SIZE_VGA:
		    return HEIGHT_FRAME_VGA;
		case ISIL_VIDEO_SIZE_SVGA:
		    return HEIGHT_FRAME_SVGA;
		case ISIL_VIDEO_SIZE_XGA:
		    return HEIGHT_FRAME_XGA;
		case ISIL_VIDEO_SIZE_720P:
		    return HEIGHT_FRAME_720P;
		case ISIL_VIDEO_SIZE_1080P:
		    return HEIGHT_FRAME_1080P;
		case ISIL_VIDEO_SIZE_QHALF_D1:
		case ISIL_VIDEO_SIZE_USER:
		default:
		    return HEIGHT_FRAME_CIF_PAL;
	    }
	}else if(standard == ISIL_VIDEO_STANDARD_NTSC){
	    switch (size) {
		case ISIL_VIDEO_SIZE_QCIF:
		    return HEIGHT_FRAME_QCIF_NTSC;
		case ISIL_VIDEO_SIZE_CIF:
		    return HEIGHT_FRAME_CIF_NTSC;        
		case ISIL_VIDEO_SIZE_2CIF:
		    return HEIGHT_FRAME_2CIF_NTSC;
		case ISIL_VIDEO_SIZE_4CIF:
		    return HEIGHT_FRAME_4CIF_NTSC;
		case ISIL_VIDEO_SIZE_HALF_D1:
		    return HEIGHT_FRAME_HALF_D1_NTSC;
		case ISIL_VIDEO_SIZE_D1:
		    return HEIGHT_FRAME_D1_NTSC;
		case ISIL_VIDEO_SIZE_VGA:
		    return HEIGHT_FRAME_VGA;
		case ISIL_VIDEO_SIZE_SVGA:
		    return HEIGHT_FRAME_SVGA;
		case ISIL_VIDEO_SIZE_XGA:
		    return HEIGHT_FRAME_XGA;
		case ISIL_VIDEO_SIZE_720P:
		    return HEIGHT_FRAME_720P;
		case ISIL_VIDEO_SIZE_1080P:
		    return HEIGHT_FRAME_1080P;
		case ISIL_VIDEO_SIZE_QHALF_D1:
		case ISIL_VIDEO_SIZE_USER:
		default:
		    return HEIGHT_FRAME_CIF_NTSC;
	    }
	}else{
	    return 0;
	}
    }

    static u32 inline VIDEO_SIZE_FROM_WIDTH_HEIGHT(u32 width, u32 height, enum ISIL_VIDEO_STANDARD standard)
    {
	//if(standard == ISIL_VIDEO_STANDARD_PAL) {
	if(((width == WIDTH_FRAME_QCIF_PAL) || (width == WIDTH_FRAME_D1_PAL>>2)) && (height == HEIGHT_FRAME_QCIF_PAL)) {
	    return ISIL_VIDEO_SIZE_QCIF;
	}
	if((width == WIDTH_FRAME_QCIF_PAL) && (height == HEIGHT_FRAME_QCIF_PAL)) {
	    return ISIL_VIDEO_SIZE_QHALF_D1;
	}
	if((width == WIDTH_FRAME_CIF_PAL) && (height == HEIGHT_FRAME_CIF_PAL)) {
	    return ISIL_VIDEO_SIZE_CIF;
	}
	if((width == (WIDTH_FRAME_D1_PAL >> 1)) && (height == HEIGHT_FRAME_D1_PAL >> 1)) {
	    return ISIL_VIDEO_SIZE_CIF;
	}
	if((width == WIDTH_FRAME_HALF_D1_PAL) && (height == HEIGHT_FRAME_HALF_D1_PAL)) {
	    return ISIL_VIDEO_SIZE_HALF_D1;
	}
	if((width == WIDTH_FRAME_D1_PAL) && (height == HEIGHT_FRAME_D1_PAL)) {
	    return ISIL_VIDEO_SIZE_D1;
	}
	if((width == WIDTH_FRAME_HCIF_PAL) && (height == HEIGHT_FRAME_HCIF_PAL)) {
	    return ISIL_VIDEO_SIZE_HCIF;
	}
	if((width == (WIDTH_FRAME_D1_PAL >> 1)) && (height == HEIGHT_FRAME_D1_PAL >> 2)) {
	    return ISIL_VIDEO_SIZE_HCIF;
	}
	if((width == WIDTH_FRAME_2CIF_PAL) && (height == HEIGHT_FRAME_2CIF_PAL)) {
	    return ISIL_VIDEO_SIZE_2CIF;
	}
	if((width == WIDTH_FRAME_4CIF_PAL) && (height == HEIGHT_FRAME_4CIF_PAL)) {
	    return ISIL_VIDEO_SIZE_4CIF;
	}
	if((width == WIDTH_FRAME_VGA) && (height == HEIGHT_FRAME_VGA)) {
	    return ISIL_VIDEO_SIZE_VGA;
	}
	if((width == WIDTH_FRAME_SVGA) && (height == HEIGHT_FRAME_VGA)) {
	    return ISIL_VIDEO_SIZE_SVGA;
	}
	if((width == WIDTH_FRAME_XGA) && (height == HEIGHT_FRAME_XGA)) {
	    return ISIL_VIDEO_SIZE_XGA;
	}
	if((width == WIDTH_FRAME_720P) && (height == HEIGHT_FRAME_720P)) {
	    return ISIL_VIDEO_SIZE_720P;
	}
	if((width == WIDTH_FRAME_1080P) && ((height == HEIGHT_FRAME_1080P) || (height == ((HEIGHT_FRAME_1080P >> 4) << 4)))) {
	    return ISIL_VIDEO_SIZE_1080P;
	}
	//}else if(standard == ISIL_VIDEO_STANDARD_NTSC){
	if(((width == WIDTH_FRAME_QCIF_NTSC) || (width == WIDTH_FRAME_D1_PAL>>2)) && (height == HEIGHT_FRAME_QCIF_NTSC)) {
	    return ISIL_VIDEO_SIZE_QCIF;
	}
	if((width == WIDTH_FRAME_QCIF_NTSC) && (height == HEIGHT_FRAME_QCIF_NTSC)) {
	    return ISIL_VIDEO_SIZE_QHALF_D1;
	}
	if((width == WIDTH_FRAME_CIF_NTSC) && (height == HEIGHT_FRAME_CIF_NTSC)) {
	    return ISIL_VIDEO_SIZE_CIF;
	}
	if((width == (WIDTH_FRAME_D1_NTSC >> 1)) && (height == HEIGHT_FRAME_D1_NTSC >> 1)) {
	    return ISIL_VIDEO_SIZE_CIF;
	}
	if((width == WIDTH_FRAME_HALF_D1_NTSC) && (height == HEIGHT_FRAME_HALF_D1_NTSC)) {
	    return ISIL_VIDEO_SIZE_HALF_D1;
	}
	if((width == WIDTH_FRAME_D1_NTSC) && (height == HEIGHT_FRAME_D1_NTSC)) {
	    return ISIL_VIDEO_SIZE_D1;
	}
	if((width == WIDTH_FRAME_HCIF_NTSC) && (height == HEIGHT_FRAME_HCIF_NTSC)) {
	    return ISIL_VIDEO_SIZE_HCIF;
	}
	if((width == (WIDTH_FRAME_D1_NTSC >> 1)) && (height == HEIGHT_FRAME_D1_NTSC >> 2)) {
	    return ISIL_VIDEO_SIZE_HCIF;
	}
	if((width == WIDTH_FRAME_2CIF_NTSC) && (height == HEIGHT_FRAME_2CIF_NTSC)) {
	    return ISIL_VIDEO_SIZE_2CIF;
	}
	if((width == WIDTH_FRAME_4CIF_NTSC) && (height == HEIGHT_FRAME_4CIF_NTSC)) {
	    return ISIL_VIDEO_SIZE_4CIF;
	}
	if((width == WIDTH_FRAME_VGA) && (height == HEIGHT_FRAME_VGA)) {
	    return ISIL_VIDEO_SIZE_VGA;
	}
	if((width == WIDTH_FRAME_SVGA) && (height == HEIGHT_FRAME_VGA)) {
	    return ISIL_VIDEO_SIZE_SVGA;
	}
	if((width == WIDTH_FRAME_XGA) && (height == HEIGHT_FRAME_XGA)) {
	    return ISIL_VIDEO_SIZE_XGA;
	}
	if((width == WIDTH_FRAME_720P) && (height == HEIGHT_FRAME_720P)) {
	    return ISIL_VIDEO_SIZE_720P;
	}
	if((width == WIDTH_FRAME_1080P) && ((height == HEIGHT_FRAME_1080P) || (height == ((HEIGHT_FRAME_1080P >> 4) << 4)))) {
	    return ISIL_VIDEO_SIZE_1080P;
	}
	//}else{
	return ISIL_VIDEO_SIZE_USER;
	//}

	return ISIL_VIDEO_SIZE_USER;
    }

    extern void init_isil_encode_timestamp(isil_timestamp_t *isil_encode_timestamp);
    extern int  init_isil_vd_cross_bus(isil_vd_cross_bus_t *, enum ISIL_CROSS_BUS_WORK_MODE mode, enum ISIL_VIDEO_STANDARD	video_standard);
    extern void remove_isil_vd_cross_bus(isil_vd_cross_bus_t *vd_cross_bus);
    extern void reset_isil_h264_encode_chan_preprocessing(ed_tcb_t *opened_logic_chan_ed);
    extern void reset_isil_h264_encode_chan_postprocessing(ed_tcb_t *opened_logic_chan_ed);
    extern void reset_isil_h264_encode_chan_processing(ed_tcb_t *opened_logic_chan_ed);
    extern void start_isil_h264_encode_chan_robust_process(isil_h264_logic_encode_chan_t *h264_logic_encode_chan);
    extern void isil_h264_encode_chan_robust_process_done(isil_h264_logic_encode_chan_t *h264_logic_encode_chan);
    extern void isil_h264_encode_chan_wait_robust_process_done(isil_h264_logic_encode_chan_t *h264_logic_encode_chan);
    extern int  isil_h264_encode_chan_is_in_robust_processing(isil_h264_logic_encode_chan_t *h264_logic_encode_chan);
    extern int  init_isil_h264_encode_chan(isil_h264_logic_encode_chan_t *, int bus_id, int chip_id, int phy_slot_id, int type, isil_chip_t *chip);
    extern void remove_isil_h264_encode_chan(isil_h264_logic_encode_chan_t *h264_logic_encode_chan);
    extern void notify_remove_encode_chan_service(encode_chan_service_tcb_t *curr_consumer);
    extern void init_encode_chan_service_tcb_with_null(encode_chan_service_tcb_t *encode_request_tcb);
    extern void init_encode_chan_service_tcb_with_first_start(encode_chan_service_tcb_t *encode_request_tcb);
    extern void init_encode_chan_service_tcb_with_start_encode(encode_chan_service_tcb_t *encode_request_tcb);

#ifdef __cplusplus
}
#endif

#endif	//ISIL_H264_ENCODE_H

