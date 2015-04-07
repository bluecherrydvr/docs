#ifndef	TW5864_CHIP_H
#define	TW5864_CHIP_H

#ifdef __cplusplus
extern "C"
{
#endif

    typedef int (*isil_irq_handler_t)(int irq, void *context);
#define	QUANTIZATION_TABLE_LEN	(96)
#define	VLC_LOOKUP_TABLE_LEN	(1024)
#define	ENCODE_VLC_LOOKUP_TABLE_BASE	(0x2000)
#define	ENCRYPT_FIRMWARE_BASE	(0x82000)
#define	ENCRYPT_FIRMWARE_LEN	(2048)
#define	START_ENCRYPT_FIRMWARE	(0x80044)

#define	DDRON                   (0x0001)
#define	INTERON                 (0x0002)
#define	CMOSON                  (0x0004)
#define	INTRAON                 (0x0008)
#define	DEBLOCKINGON            (0x0010)
#define	DAON                    (0x0020)
#define	CIF                     (0x0000)
#define	D1                      (0x0040)
#define	HD_720P_1080I           (0x0080)
#define	HALFD1                  (0x00c0)

#define ENABLE_VLC_BYTE_SWP     (0x00040)
#define ENABLE_INSERT_03        (0x00080)
#define ENABLE_CDC_VLCS_MAST_R  (0x02000)
#define	ENABLE_VLC_FLOWCONTROL  (0x04000)
#define	ENABLE_VLC_PCI_SEL      (1 << 15)
#define ENABLE_HARDWARE_GEN_HEADER (1 << 19)
#define MASK_HARDWARE_INSERT_03 (0x10000)
#define ENABLE_VLC_MVD          (1 << 23)

#define ENABLE_VLC_BITSTREAM_SEND_DDR   (0x0080)

#define	AD_DA_LOOP      (8)
#define	START_DAC       (2)
#define	START_COMS_EN   (4)
#define	START_ORIG_DAC  (1)

#define	DDR_A_CHIP      (0<<5)
#define	DDR_B_CHIP      (1<<5)
#define VLC_FLOW_CONTROL    (1 << 6)
#define MAX_MB_DELAY    (128 * 16)
#define MIN_MB_DELAY    (0)
    /*48 = 0x320, 128 = 0x820, 192 = 0xc20, 240 = 0xf20, 384 = 0x330, 512 = 0x430, 768 = 0x630, 1024 = 0x830, 1536 = 0xc30, 1920 = 0xf30*/
#define MB_DELAY_VALUE  (0xa<<4) /*0x3<<4 = 0x320, 0xf<<4 = 0xf20, 0x4<<7 = 0x430*/
#define SEARCH_WINDOW_SIZE_VALUE    (0x0109)

#define DEFAULT_SYS_CLK         185 /*default 185M*/
#define PCI_MASTER_OLD
//#define USE_CRC_CHECK

#define	MODE                    (0x0004)
#define	CHIPSCOPE               (0x000c)
#define	TOTAL_CHAN_CUR_PTR      (0x0010)
#define	WORD_SHORT_CNTL         (0x0014)
#define	QP                      (0x0018)
#define CIF_MAP_D1_MODE         (0x001c)
#define	ORIGINALSEQUENCE        (0x0020)
#define	REFERENCEFRAME          (0x0024)
#define	CHANNEL_SELECT          (0x0028)
#define	CHANNEL_ID              (0x002c)
#define	DDRPAGE                 (0x0030)
#define	DSP_DEBUG_CNTL          (0x0034)
#define	AD_ORIG_RAM_BASE        (0x0038)
#define	AD_CHAN_8_15_ORIG_BASE  (0x003c)
#define	NEW_SENSOR_CONTROL      (0x0040)
#define	FRAME_DELAY             (0x0050)
#define	TOTAL_CHAN_8_15_CUR_PTR (0x004c)
#define	INTERSTART              (0x0200)
#define	WINDOWSIZE              (0x0204)
#define	MBADDR                  (0x0208)
#define	INTRANDMECONTROL        (0x020c)
#define	REFFRAMEINDEX           (0x0210)
#define	LAMDA                   (0x0214)
#define	FRAMERESOLUTION         (0x0218)
#define	DSP_CUR_RAM_BASE        (0x021c)

#define	OSD_ATTRI_BASE_REG      (0x0220)
#define	OSD_ATTRI_UPDATE_REG    (0x0224)
#define	OSD_RECTANGLE_EN_REG    (0x0228)

#define	ENCRYPT_CLOCK           (0x0248)
#define	INTRASTART              (0x0400)
#define INTRA4X4_THRESHOLD_I    (0x040c)
#define	INTRAENABLE             (0x0410)
#define MB4X4LAMDA              (0x0414)
#define	MEREADY                 (0x0600)
#define INTRA4X4_THRESHOLD_IP   (0x604)
#define	MBMODE                  (0x060c)
#define	QUANTIZATION            (0x0800)
#define	ENCODER_VLC_PARAM       (0x1000)
#define	SLICE_TOTAL_BIT_NUMBER  (0x1004)
#define	RES_TOTAL_BIT_NUMBER    (0x1008)
#define	TOTAL_COEFF_NUMBER      (0x1010)
#define	ENCODER_VLC_INTR_STATUS (0x100c)
#define	VLC_ENCODER_INTERRUPT   (0x1014)
#define	ENABLE_VLC_LOOKUP_TABLE (0x101c)
#define ENCODE_FRAME_TYPE       (0x1028)

#define	MV_VECTOR_CONTROL       (0xfc04)


#define CHIP_TIEMR_INTERVAL     (0x1804c)
#define ONE_MS_INTERVAL         (0x0)
#define TWO_MS_INTERVAL         (0x1)
#define FOUR_MS_INTERVAL        (0x2)
#define EIGHT_MS_INTERVAL       (0x3)

    /*senif configure*/
#define SENIF_BUS01_MODE         (0x0d00)
#define SENIF_BUS23_MODE         (0x0d04)
#define SENIF_HOR_MIR            (0x0d08)
#define SENIF_VER_MIR            (0x0d0c)
#define SENIF_WIDTH_PARTA(bus)   (0x0d10 + (bus) * 0x10)
#define SENIF_WIDTH_PARTB(bus)   (0x0d14 + (bus) * 0x10)
#define SENIF_HEIGHT_PARTA(bus)  (0x0d18 + (bus) * 0x10)
#define SENIF_HEIGHT_PARTB(bus)  (0x0d1c + (bus) * 0x10)
#define SENIF_FULL_FLAGS         (0x0d50)
#define SENIF_PARTAB_SELECT      (0x0d54)



#define AUDIO_1MS_PLL_COUNTER_REG           (0x4000)
#define AUDIO_EN_CONTROL_REG                (0x4004)
#define AUDIO_IN_ORIG_EN_CONTROL_REG        (0x4008)
#define AUDIO_IN_ADPCM_EN_CONTROL_REG       (0x400c)
#define AUDIO_OUT_CONTROL_REG               (0x4014)

#define HOST_BLOCK_RD_ADPCM_FINISH_REG      (0x4018)
#define ADPCM_ENCODE_WR_PTR_CH0_CH9_REG     (0x401c)
#define ADPCM_ENCODE_WR_PTR_CH10_CH16_REG   (0x4020)    
#define ADPCM_ENCODE_RD_PTR_CH0_CH9_REG     (0x4024)
#define ADPCM_ENCODE_RD_PTR_CH10_CH16_REG   (0x4028)

#define ADPCM_DECODE_RD_WR_PTR_REG          (0x402c)

#define PCM_ENCODE_WR_PTR_CH0_CH7_REG       (0x4030)
#define PCM_ENCODE_WR_PTR_CH8_CH15_REG      (0x4034)
#define PCM_ENCODE_WR_PTR_CH16_REG          (0x4038)
#define PCM_ENCODE_RD_PTR_CH0_CH7_REG       (0x403c)
#define PCM_ENCODE_RD_PTR_CH8_CH15_REG      (0x4040)
#define PCM_ENCODE_RD_PTR_CH16_REG          (0x4044)
#define HOST_BLOCK_RD_PCM_FINISH_REG        (0x4048)

#define PCI_AUDIO_CHAN_SELECT_REG           (0x404c)
#define AUDIO_DEC_ERR_STATUS_REG            (0x4050)
#define MPI_CS_SETUP                        (0x8000)
#define MPI_RDY_HOLD                        (0x8004)
#define CDC_VLCS_MASTER_REG                     (0x8008)
#define CHIP_IRQS_NR                            (32)
#define	INTERRUPT_SOURCE_TRIGGER_REG_L	        (0x8800)
#define	INTERRUPT_SOURCE_TRIGGER_REG_H	        (0x8804)
#define	EDGE_TRIGGER				(1)
#define	LEVEL_TRIGGER				(0)
#define	INTERRUPT_SOURCE_ENABLE_REG_L		(0x8808)
#define	INTERRUPT_SOURCE_ENABLE_REG_H		(0x880c)
#define	INTERRUPT_ENABLE			(1)
#define	INTERRUPT_DISABLE			(0)
#define	INTERRUPT_CLEAR_REG_L			(0x8810)
#define	INTERRUPT_CLEAR_REG_H			(0x8814)
#define	INTERRUPT_ASSERT_REG_L			(0x8818)
#define	INTERRUPT_ASSERT_REG_H			(0x881c)
#define	INTERRUPT_OUTPUT_ASSERT_REG		(0x8820)
#define	INTERRUPT_STATUS_REG_L			(0x8838)
#define	INTERRUPT_STATUS_REG_H			(0x883c)

#define GPIO_REG                                (0x9800)

#define INTERRUPT_FLAGS_EXT             (0x18000)
#define PCI_MASTER_CONTROL              (0x18004)
#define PCI_MASTER_INTR_ENABLE_REG      (0x1800c)
#define IRQ_EXT_PCI_MASTER              (0)
#define IRQ_EXT_VLC_INTR                (1)
#define IRQ_EXT_VLC_MVD                 (2)
#define IRQ_EXT_ADSYNC_INTR             (3)
#define IRQ_EXT_PRVEOF_INTR             (4)/*preview eof intr*/
#define IRQ_EXT_PRVOVR_INTR             (5)/*preview overflow intr*/
#define IRQ_EXT_TIMER_INTR              (6)
#define IRQ_EXT_JPEG_INTR               (7)
#define IRQ_EXT_AUDIO_EOF_INTR          (8)
#define IRQ_EXT_IIC_INTR                (24)/*iic done*/
#define IRQ_EXT_AD_INTR                 (25) /*AD interrupt, video lost ...*/

#define PCI_I2C                         (0x18014)
#define MCU_IIC_CONF                    (0x18048)
#define PCI_TIMER_CONTROL               (0x1804c)
#define I2C_PHASE_SHIFT                 (0x800c)

#define PCI_INTR_CHAN_SOURCE            (0x18008)
#define PCI_PREVIEW_AUDIO_AB            (0x18010)
#define PCI_PREVIEW_CHAN_OFFSET_BASE    (0)
#define PCI_AUDIO_CHAN_OFFSET_BASE      (16)
#define PCI_VLC_MAX_LEN                 (0x18020)
#define PCI_JPEG_MAX_LEN                (0x18050)
#define PCI_JPEG_BUF_INTR               (0x18060)

#define PCI_VLC_CRC                     (0x1801c)

#if defined(PCI_MASTER_OLD)
#define PCI_VLC_LEN                     (0x18024)
#define PCI_VLC_INTRA_CRC_I             (0x18028)
#define PCI_VLC_INTRA_CRC_O             (0x1802c)
#define PCI_VLC_MV_LEN                  (0x18034)
#else
#define PCI_VLC_LEN                     (0x18200)
#define PCI_VLC_INTRA_CRC_I             (0x18204)
#define PCI_VLC_INTRA_CRC_O             (0x18208)
#define PCI_VLC_MV_LEN                  (0x18210)
#endif

#define PCI_MASTER_REG_SWITCH           (0x1807c)
#define PCI_VLC_BASE_ADDR               (0x18080)
#define PCI_MV_BASE_ADDR                (0x18084)
#define PCI_AUDIO_CHAN00_CHAN03_PING_BASE_ADDR_REG  (0x180a0)
#define PCI_AUDIO_CHAN00_CHAN03_PONG_BASE_ADDR_REG  (0x180a4)
#define PCI_AUDIO_CHAN04_CHAN07_PING_BASE_ADDR_REG  (0x180a8)
#define PCI_AUDIO_CHAN04_CHAN07_PONG_BASE_ADDR_REG  (0x180ac)
#define PCI_AUDIO_CHAN08_CHAN11_PING_BASE_ADDR_REG  (0x180b0)
#define PCI_AUDIO_CHAN08_CHAN11_PONG_BASE_ADDR_REG  (0x180b4)
#define PCI_AUDIO_CHAN12_CHAN15_PING_BASE_ADDR_REG  (0x180b8)
#define PCI_AUDIO_CHAN12_CHAN15_PONG_BASE_ADDR_REG  (0x180bc)
#define PCI_JPEG_BUFFER0_BASE_ADDR_REG  (0x180c0)
#define PCI_JPEG_BUFFER1_BASE_ADDR_REG  (0x180c4)
#define PCI_JPEG_BUFFER2_BASE_ADDR_REG  (0x180c8)
#define PCI_JPEG_BUFFER3_BASE_ADDR_REG  (0x180cc)
#define PCI_JPEG_BUFFER4_BASE_ADDR_REG  (0x180d0)
#define PCI_JPEG_BUFFER5_BASE_ADDR_REG  (0x180d4)
#define PCI_JPEG_BUFFER6_BASE_ADDR_REG  (0x180d8)
#define PCI_JPEG_BUFFER7_BASE_ADDR_REG  (0x180dc)

#define PCI_INTR_OUTPUT_REG             (0x18064)

#ifdef USE_ON_CHIP_TIMER
#define MS_PER_TICK                     (4)
#else
#define MS_PER_TICK                     (5)
#endif

#define	IRQ_VLC_TYPE_INTR			            (0)
#define IRQ_BURST_TYPE_INTR                     (1)
#define	IRQ_MV_TYPE_INTR			            (2)
#define IRQ_FRONT_END_TYPE_INTR                 (3)
#define	IRQ_JPEG_TYPE_INTR			            (12)
#define IRQ_TIMER_TYPE_INTR                     (22)
#define IRQ_VLC_DONE_TYPE_INTR                  (17)
#define	IRQ_AD_SYNC_TYPE_INTR			        (19)
#define IRQ_PREV_EOF_TYPE_INTR                  (20)
#define IRQ_PREV_OVERFLOW_TYPE_INTR             (21)
#define IRQ_AUDIO_EOF_TYPE_INTR                 (24)
#define IRQ_IIC_DONE_INTR                       (25)
#define IRQ_AD_EVENT_INTR                       (26)

#define	ASIC_FRAME_MODE         (0x9000)
#define	ASIC_FRAME_MODE_UPDATE  (0x900C)

#define	DDRBASE             (0x80000)
#define	DDR_BURST_REG       (0x80040)
#define DDR_MAP_COMPRESS_ENABLE     (0)
#define DDR_MAP_COMPRESS_DISABLE    (1)

#define	AUDIO_ONE_CHIP_FOUR_CHAN        (0x3)
#define	AUDIO_TWO_CHIP_FOUR_CHAN        (0xc)
#define	AUDIO_ONE_CHIP_TWO_CHAN_AND_TWO_CHIP_TWO_CHAN   (0xa)

#define OSD_ATTR_REG_BASE_ADDR          (0x047f)
#define OSD_ATTR_REG_SHIFT              (10)

#define CHAN_VIDEO_TIMESTAMP_BASE       (0xc00)

#define FPGA_LEN_OFFSET                 (4)
#define VLC_INTR_STATUS_MASK            (7)
#define VLC_INTR_MASK_ALL               (0xf)

#define	VLC_INTERRUPT_NULL              (0)
#define	VLC_A_BANK_FULL                 (1)
#define	VLC_B_BANK_FULL                 (2)
#define	VLC_A_B_BANK_FULL               (3)
#define	VLC_END_SLICE                   (4)
#define	VLC_A_FULL_END_SLICE            (5)
#define	VLC_B_FULL_END_SLICE            (6)
#define	VLC_BANK_END_OVERFLOW           (7)
#define	VLC_OVERFLOW                    (8)




#define FPGA_ASYNC_SECTION_LEN          (0x200000)

#define FPGA_RESET_VALID    0
#define FPGA_RESET_INVALID  1


#ifdef	POWERPC_PLATFORM
#define	TEST_REG                (0x00)
#define	TW2815_INT              (0x01)
#define	ALM_IN_HIGH             (0x02)
#define	ALM_IN_LOW              (0x03)
#define	FPGA_INT_ID             (0x04)
#define	FPGA_CHIP_ALL_MASK      (0x1f)
#define	FPGA_CHIP_1_INT_ID      (0x10)
#define FPGA_CHIP_2_INT_ID      (0x08)
#define FPGA_CHIP_3_INT_ID      (0x04)
#define FPGA_CHIP_4_INT_ID      (0x02)
#define FPGA_CHIP_5_INT_ID      (0x01)
#define	RESET_REG               (0x08)
#define	RESET_REG_TW2835        (0x80)
#define RESET_REG_VGA           (0x40)
#define	RESET_REG_FPGA_CHIP_5   (0x20)
#define RESET_REG_FPGA_CHIP_4   (0x10)
#define RESET_REG_FPGA_CHIP_3   (0x08)
#define RESET_REG_FPGA_CHIP_2   (0x04)
#define RESET_REG_FPGA_CHIP_1   (0x02)
#define	CPLD_BASE_ADDR          (0x50000000)
#define	CPLD_SECTION_LEN        (0x100000)

#define FPGA0_BASE_ADDR         (0x60000000)
#define FPGA1_BASE_ADDR         (0x60400000)
#define FPGA2_BASE_ADDR         (0x60800000)
#define FPGA3_BASE_ADDR         (0x60c00000)
#define FPGA4_BASE_ADDR         (0x61000000)

#define	FPGA0_SYNC_BASE_ADDR    (0x70000000)
#define	FPGA1_SYNC_BASE_ADDR    (0x70400000)
#define	FPGA2_SYNC_BASE_ADDR    (0x70800000)
#define	FPGA3_SYNC_BASE_ADDR    (0x70c00000)
#define	FPGA4_SYNC_BASE_ADDR    (0x71000000)
#define FPGA_SYNC_SECTION_LEN   (0x100000)

#ifdef PCI_PLATFORM
#define IRQ_FPGA                (0x13)
#else
#define IRQ_FPGA                (0x30)
#endif
#define IRQ_SYS_INTERNAL        (0x41)
#endif
#define	IRQT_LOW                (IRQ_TYPE_LEVEL_LOW)

#define	ENCODE_4_D1                     (0x00000000)
#define	ENCODE_16_CIF                   (0x00000001)
#define	ENCODE_WORK_MODE_MASK           (0x00000003)

    typedef struct
    {
	u8	first_device_number;
	u32	video_encode_work_mode;
	u32	encode_reg_4;
	u32	fpga_sync_base_addr;
	u32	fpga_sync_section_len;
	u32	fpga_async_base_addr;
	u32	fpga_async_section_len;
	u32 ddr_map_mode;
    }DVMNVS_CHIP_VIDEO_ENCODE_CAPABILITY;

    struct isil_interrupt_control_system_interface_operation{
	int     (*init)(isil_interrupt_control_t *, isil_chip_t *chip);
	void    (*enable_interrupt_source)(isil_interrupt_control_t *, u32 source);
	void    (*disable_interrupt_source)(isil_interrupt_control_t *, u32 source);
	void    (*set_interrupt_source_edge_trigger)(isil_interrupt_control_t *, u32 source);
	void    (*set_interrupt_source_level_trigger)(isil_interrupt_control_t *, u32 source);
    };

    struct isil_interrupt_control_hardware_interface_operation{
	void    (*get_interrupt_status)(isil_interrupt_control_t *);
	void    (*ack_interrupt)(isil_interrupt_control_t *, u32 source);
    };

    struct chip_irq_action{
	isil_irq_handler_t  handler;
	int             irq;
	unsigned long   flags;
	const char      *name;
	void            *context;
	struct chip_irq_action *next;
    };

    struct chip_irq_desc{
	u32                     irq;
	u32                     irq_count;
	u32                     irqs_unhandled;
	spinlock_t              lock;
	atomic_t		        depth;
	struct chip_irq_action  action;
	const char              *name;
	unsigned long           flags;
        
    u32                     irq_duration;
	u32                     base_tick;
	u32                     base_irq_count;
	u32                     delta_irq_count;
	u32                     irq_overflow_count;
    };



    struct isil_interrupt_control{
	spinlock_t  lock;
	__u32   interrupt_source;
	__u32   interrupt_source_trigger_mode;
	__u32   interrupt_source_assert;
	__u32   chip_interrupt_output_assert;
	__u32   chip_interrupt_status;

	struct chip_irq_desc chip_irq_desc[CHIP_IRQS_NR];

	int         chip_irq;
        int         chip_irq_register_flags;
	isil_chip_t  *chip;

	struct isil_interrupt_control_system_interface_operation      *system_op;
	struct isil_interrupt_control_hardware_interface_operation    *hardware_op;
    };

    struct chip_io_operation{
	u32     (*chip_read32)(isil_chip_t *, u32 offset);
	u16     (*chip_read16)(isil_chip_t *, u32 offset);
	u8      (*chip_read8)(isil_chip_t *, u32 offset);
	void    (*chip_read_block)(isil_chip_t *, u32 offset, int len, u32 *dest);
	void    (*chip_write32)(isil_chip_t *, u32 offset, u32 value);
	void    (*chip_write16)(isil_chip_t *, u32 offset, u16 value);
	void    (*chip_write8)(isil_chip_t *, u32 offset, u8 value);
	void    (*chip_write_block)(isil_chip_t *, u32 offset, int len, u32 *src);
    };

#define to_get_chip_with_chip_timer_cont(node)          container_of(node, isil_chip_t, chip_timer_cont)
#define to_get_chip_with_chip_dpram_controller(node)    container_of(node, isil_chip_t, chip_dpram_controller)
#define	to_get_chip_with_chip_cross_bus(node)           container_of(node, isil_chip_t, chip_vd_cross_bus)
#define	to_get_chip_with_chip_vj_bus(node)              container_of(node, isil_chip_t, chip_vj_bus)
#define	to_get_chip_with_chip_vp_bus(node)              container_of(node, isil_chip_t, chip_vp_bus)
#define	to_get_chip_with_chip_driver(node)              container_of(node, isil_chip_t, chip_driver)
#define	to_get_chip_with_chip_bus(node)                 container_of(node, isil_chip_t, isil_chip_bus)
#define	to_get_chip_with_chip_encode_service_queue(node)    container_of(node, isil_chip_t, chip_encode_service_queue)
#define	CHIP_NAME_LEN   (32)
    struct isil4000i_chip
    {
	char        name[CHIP_NAME_LEN];
	int         bus_id;
	int         chip_id;
	int         first_device_id;
	atomic_t    open_chan_number;
	robust_processing_control_t    chip_robust_processing;
	DVMNVS_CHIP_VIDEO_ENCODE_CAPABILITY *video_encode_cap;

	u32         sys_clock;

        struct  device  *dev;
	struct resource *resource;
	void __iomem    *regs;
	unsigned long   pa_regs;
	struct resource *sync_resource;
	void __iomem    *sync_regs;
	unsigned long   pa_sync_regs;
	isil_interrupt_control_t  isil_int_control;

	spinlock_t  lock;
	isil_h264_logic_encode_chan_t *curr_h264_encode_chan;
	int     vlc_ping_pong_index;
	void    (*register_curr_h264_encode_chan)(isil_chip_t *, isil_h264_logic_encode_chan_t *);
	void    (*unregister_curr_h264_encode_chan)(isil_chip_t *, isil_h264_logic_encode_chan_t *);
	void    (*register_curr_h264_encode_push_chan)(isil_chip_t *, isil_h264_logic_encode_chan_t *);
	void    (*unregister_curr_h264_encode_push_chan)(isil_chip_t *, isil_h264_logic_encode_chan_t *);
	void    (*register_timer)(isil_chip_t *, isil_irq_handler_t);
	void    (*enable_timer)(isil_chip_t *);
	void    (*disable_timer)(isil_chip_t *);
	void    (*unregister_timer)(isil_chip_t *);

	int     (*chip_open)(isil_chip_t *);
	void    (*chip_close)(isil_chip_t *);
	void    (*chip_set_invalid)(isil_chip_t *);
	void    (*chip_set_valid)(isil_chip_t *);
	void    (*chip_reset)(isil_chip_t *);
	struct chip_io_operation    *io_op;
	int     chip_polling_timeid;
	void    (*add_chip_polling_task)(isil_chip_t *);
	void    (*delete_chip_polling_task)(isil_chip_t *);

        isil_chip_timer_controller_t    chip_timer_cont;
	isil_chip_bus_t     isil_chip_bus;
	chip_audio_t        chip_audio;
	isil_vd_cross_bus_t chip_vd_cross_bus;
	isil_vj_bus_t       chip_vj_bus;
	isil_vp_bus_t       chip_vp_bus;
	chip_encode_chan_service_queue_t    chip_encode_service_queue;
	chip_ddr_burst_engine_t             chip_ddr_burst_interface;

	chip_driver_t               *chip_driver;
	isil_chip_ai_driver_t       *chip_ai_driver;
	isil_chip_vi_driver_t       *chip_vi_driver;
	isil_display_driver_t       *display_driver;

	isil_audio_driver_t             audio_chan_driver[ISIL_AUDIO_CHAN_NUMBER];
	isil_h264_logic_encode_chan_t   h264_master_encode_logic_chan[ISIL_PHY_VD_CHAN_NUMBER];
	isil_h264_logic_encode_chan_t   h264_sub_encode_logic_chan[ISIL_PHY_VD_CHAN_NUMBER];
#ifdef  MJPEG_MODULE
	isil_jpeg_logic_encode_chan_t   jpeg_encode_chan[ISIL_PHY_VJ_CHAN_NUMBER];
#endif
	isil_avSync_dev_t               av_avSync_device_chan[ISIL_PHY_VD_CHAN_NUMBER];
	//  isil_audio_speaker_device_t   audio_speaker_device;
	//  isil_audio_playback_t         audio_playback_device;
	struct proc_dir_entry   *proc_entry;
	isil_proc_register_s    chip_proc, h264_proc, jpeg_proc, audio_proc, h264d_proc, preview_proc, push_buf_mgt_proc;
	isil_prev_logic_chan_t prev_chan[ISIL_PHY_VP_CHAN_NUMBER];
	push_buf_pool_t push_buf_pool;
    };

    struct FPGA_DDR_BURST_REG{
#if defined(__LITTLE_ENDIAN_BITFIELD)
	u32	ddr_op_base_addr    :   20,
		reserved            :   4,
		ddr_len             :   4,
		ddr_read_status     :   1,
		ddr_write_status    :   1,
		ddr_read            :   1,
		ddr_write           :   1;
#elif defined (__BIG_ENDIAN_BITFIELD)
	u32	ddr_write           :   1,
		ddr_read            :   1,
		ddr_write_status    :   1,
		ddr_read_status     :   1,
		ddr_len             :   4,
		reserved            :   4,
		ddr_op_base_addr    :   20;
#endif
    };

    typedef union{
	struct FPGA_DDR_BURST_REG	reg;
	u32	value;
    }FPGA_DDR_BURST_CONTROLLER;

    static inline struct chip_irq_desc *chip_irq_to_desc(isil_chip_t *chip, unsigned int irq)
    {
	isil_interrupt_control_t *irq_ctrl;

	if(chip){
	    irq_ctrl = &chip->isil_int_control;
	    return (irq < CHIP_IRQS_NR) ? irq_ctrl->chip_irq_desc + irq : NULL;
	}

	return NULL;
    }


    extern void get_isil_avSync_device(isil_avSync_dev_t **ptr_device, int inode_id);
    extern void disable_timer_int(void);
    extern void enable_timer_int(void);
    extern void disable_chip_int(isil_chip_t *chip);
    extern void enable_chip_int(isil_chip_t *chip);

    extern int  chip_request_irq(isil_chip_t *chip, u32 irq, isil_irq_handler_t handler, const char *devname, void *context);
    extern void chip_free_irq(isil_chip_t *chip, u32 irq, void *context);
    extern void chip_enable_irq(isil_chip_t *chip, u32 irq);
    extern void chip_disable_irq(isil_chip_t *chip, u32 irq);
    extern void start_chip_robust_process(isil_chip_t *chip);
    extern void chip_robust_process_done(isil_chip_t *chip);
    extern void chip_wait_robust_process_done(isil_chip_t *chip);
    extern int  chip_is_in_robust_processing(isil_chip_t *chip);

    void mpb_write(isil_chip_t *chip, u32 addr,u32 data);
    u32 mpb_read(isil_chip_t *chip, u32 addr);
    int pci_i2c_read(isil_chip_t *chip, u8 devid, u8 devfn, u8 *buf);
    int pci_i2c_write(isil_chip_t *chip, u8 devid, u8 devfn, u8 buf);
#ifdef __cplusplus
}
#endif

#endif

