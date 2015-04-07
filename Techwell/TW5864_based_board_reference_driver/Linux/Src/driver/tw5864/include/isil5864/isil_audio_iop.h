#ifndef	ISIL_AUDIO_IOP
#define	ISIL_AUDIO_IOP

#ifdef __cplusplus
extern "C"
{
#endif

#define ISIL_AUDIO_BUF_PAGE_ID        (14)
#define ISIL_AUDIO_BUF_OF_DDR_ID      (DDR_CHIP_B)
#define ISIL_PCM_AUDIO_LEN            (498)
#define ISIL_ADPCM_AUDIO_LEN          (384)

#define ISIL_PHY_AUDIO_IN_CHAN_NUMBER (20)
#define	ISIL_AUDIO_IN_CHAN_NUMBER     (17)
#define	ISIL_AUDIO_OUT_CHAN_NUMBER    (2)
#define	ISIL_AUDIO_CHAN_NUMBER        (ISIL_AUDIO_IN_CHAN_NUMBER+ISIL_AUDIO_OUT_CHAN_NUMBER)

#define	AUDIO_SECTION_SIZE              (0x200)
#define	AUDIO_OUT_SECTION_VALID_SIZE	(0x180)

#define	INVALID_ISIL_AUDIO_SECTION_ID       (-1)
#define	AUDIO_IN_PCM_CHAN_SECTION_NUMBER    (6)
#define	AUDIO_IN_ADPCM_CHAN_SECTION_NUMBER  (4)
#define	AUDIO_OUT_CHAN_SECTION_NUMBER       (8)

#define	AUDIO_ENABLE        (0x0001)
#define	AUDIO_DISABLE       (0x0000)
#define	AUDIO_MUTE_ENABLE   (0x0001)
#define	AUDIO_MUTE_DISABLE  (0x0000)

    struct audio_property_update_operation {
	void	(*init)(isil_audio_param_t *, isil_audio_config_descriptor_t *);
	int     (*update_sample_rate)(isil_audio_param_t *, int);
	int     (*get_sample_rate)(isil_audio_param_t *);
	int     (*update_bit_wide)(isil_audio_param_t *, int);
	int     (*get_bit_wide)(isil_audio_param_t *);
	int     (*update_type)(isil_audio_param_t *, int);
	int     (*get_type)(isil_audio_param_t *);
    };

    struct audio_config_descriptor{
	short   type;
	char    sample_rate;
	char    bit_wide;
    }STRUCT_PACKET_ALIGN(1);

    struct isil_audio_param {
	isil_audio_config_descriptor_t    property;
	struct audio_property_update_operation  *op;
    };

    struct isil_audio_section_descriptor_operation {
	void    (*init_audio_descriptor)(isil_audio_descriptor_t *, void *);
	u32     (*get_audio_descriptor_timestamp)(isil_audio_descriptor_t *);
	void    (*set_audio_descriptor_timestamp)(isil_audio_descriptor_t *, u32);
	int     (*get_audio_descriptor_sample_rate)(isil_audio_descriptor_t *);
	void    (*set_audio_descriptor_sample_rate)(isil_audio_descriptor_t *, int);
	int     (*get_audio_descriptor_type)(isil_audio_descriptor_t *);
	void    (*set_audio_descriptor_type)(isil_audio_descriptor_t *, int);
	int     (*get_audio_descriptor_bit_wide)(isil_audio_descriptor_t *);
	void    (*set_audio_descriptor_bit_wide)(isil_audio_descriptor_t *, int);
	u32     (*get_audio_descriptor_valid_len)(isil_audio_descriptor_t *);
	void    (*set_audio_descriptor_valid_len)(isil_audio_descriptor_t *, u32);
    };

    struct audio_section_descriptor {
#if defined(__LITTLE_ENDIAN_BITFIELD)
	u32 timestamp:16;
	u32 sample_rate:2;
	u32 type:4;
	u32 bit_wide:1;
	u32 valid_len:9;
#elif defined (__BIG_ENDIAN_BITFIELD)
	u32 valid_len:9;
	u32 bit_wide:1;
	u32 type:4;
	u32 sample_rate:2;
	u32 timestamp:16;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
    }STRUCT_PACKET_ALIGN(1);

    struct isil_audio_section_descriptor {
	audio_section_descriptor_t  *descriptor;
	struct isil_audio_section_descriptor_operation    *op;
    };

    extern void	init_isil_audio_descriptor(isil_audio_descriptor_t *, void *);
    extern void	init_audio_encode_property(isil_audio_param_t *, isil_audio_config_descriptor_t *);
    extern void	init_audio_decode_property(isil_audio_param_t *, isil_audio_config_descriptor_t *);

    struct	audio_enable_control_reg {  //reg 0x4004
#if defined(__LITTLE_ENDIAN_BITFIELD)
	u32	adpcm_decode_enable : 1;
	u32	pcm_encode_enable : 1;
	u32	adpcm_encode_enable : 1;
	u32	reserve : 29;
#elif defined (__BIG_ENDIAN_BITFIELD)
	u32 reserve : 29;
	u32 adpcm_encode_enable : 1;
	u32	pcm_encode_enable : 1;
	u32	adpcm_decode_enable : 1;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
    }STRUCT_PACKET_ALIGN(1);

    typedef union audio_enable_control_union{
	struct audio_enable_control_reg bit_value;
	u32 value;
    }STRUCT_PACKET_ALIGN(1) audio_enable_control_reg_value;

    struct audio_pcm_encode_control_reg {   //reg 0x4008
#if defined(__LITTLE_ENDIAN_BITFIELD)
	u32 audio_chan0_en : 1;
	u32 audio_chan1_en : 1;
	u32 audio_chan2_en : 1;
	u32 audio_chan3_en : 1;
	u32 audio_chan4_en : 1;
	u32 audio_chan5_en : 1;
	u32 audio_chan6_en : 1;
	u32 audio_chan7_en : 1;
	u32 audio_chan8_en : 1;
	u32 audio_chan9_en : 1;
	u32 audio_chan10_en : 1;
	u32 audio_chan11_en : 1;
	u32 audio_chan12_en : 1;
	u32 audio_chan13_en : 1;
	u32 audio_chan14_en : 1;
	u32 audio_chan15_en : 1;
	u32 audio_chan16_en : 1;
	u32 AD_BIT_WIDE : 1;
	u32 audio_type : 4;
	u32 audio_sample_rate : 2;
	u32 audio_loopback_chan_id : 5;
	u32 audio_loopback_enable : 1;
	u32 audio_ad_loop:1;
	u32 arm_mode_or_pci_mod : 1;
#elif defined (__BIG_ENDIAN_BITFIELD)
	u32 arm_mode_or_pci_mod : 1;
	u32 audio_ad_loop : 1;
	u32 audio_loopback_enable : 1;
	u32 audio_loopback_chan_id : 5;
	u32 audio_sample_rate : 2;
	u32 audio_type : 4;
	u32 AD_BIT_WIDE : 1;
	u32 audio_chan16_en : 1;
	u32 audio_chan15_en : 1;
	u32 audio_chan14_en : 1;
	u32 audio_chan13_en : 1;
	u32 audio_chan12_en : 1;
	u32 audio_chan11_en : 1;
	u32 audio_chan10_en : 1;
	u32 audio_chan9_en : 1;
	u32 audio_chan8_en : 1;
	u32 audio_chan7_en : 1;
	u32 audio_chan6_en : 1;
	u32 audio_chan5_en : 1;
	u32 audio_chan4_en : 1;
	u32 audio_chan3_en : 1;
	u32 audio_chan2_en : 1;
	u32 audio_chan1_en : 1;
	u32 audio_chan0_en : 1;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
    }STRUCT_PACKET_ALIGN(1);

    typedef union {
	struct audio_pcm_encode_control_reg bit_value;
	u32 value;
    }STRUCT_PACKET_ALIGN(1) audio_pcm_encode_control_reg_value;

    struct	audio_adpcm_encode_control_reg {    //reg 0x400c
#if defined(__LITTLE_ENDIAN_BITFIELD)
	u32 audio_chan0_en : 1;
	u32 audio_chan1_en : 1;
	u32 audio_chan2_en : 1;
	u32 audio_chan3_en : 1;
	u32 audio_chan4_en : 1;
	u32 audio_chan5_en : 1;
	u32 audio_chan6_en : 1;
	u32 audio_chan7_en : 1;
	u32 audio_chan8_en : 1;
	u32 audio_chan9_en : 1;
	u32 audio_chan10_en : 1;
	u32 audio_chan11_en : 1;
	u32 audio_chan12_en : 1;
	u32 audio_chan13_en : 1;
	u32 audio_chan14_en : 1;
	u32 audio_chan15_en : 1;
	u32 audio_chan16_en : 1;
	u32 reserve : 15;
#elif defined (__BIG_ENDIAN_BITFIELD)
	u32 reserve : 15;
	u32 audio_chan16_en : 1;
	u32 audio_chan15_en : 1;
	u32 audio_chan14_en : 1;
	u32 audio_chan13_en : 1;
	u32 audio_chan12_en : 1;
	u32 audio_chan11_en : 1;
	u32 audio_chan10_en : 1;
	u32 audio_chan9_en : 1;
	u32 audio_chan8_en : 1;
	u32 audio_chan7_en : 1;
	u32 audio_chan6_en : 1;
	u32 audio_chan5_en : 1;
	u32 audio_chan4_en : 1;
	u32 audio_chan3_en : 1;
	u32 audio_chan2_en : 1;
	u32 audio_chan1_en : 1;
	u32 audio_chan0_en : 1;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
    }STRUCT_PACKET_ALIGN(1);

    typedef union {
	struct audio_adpcm_encode_control_reg   bit_value;
	u32     value;
    }STRUCT_PACKET_ALIGN(1) audio_adpcm_encode_control_reg_value;

    struct aduio_decode_control_reg {   //reg 0x4014
#if defined(__LITTLE_ENDIAN_BITFIELD)
	u32 audio_out_chan0_mute : 1;
	u32 audio_out_chan0_DA_BIT_WIDE : 1;
	u32 audio_out_chan1_mute : 1;
	u32 audio_out_chan1_DA_BIT_WIDE : 1;
	u32 audio_out_chan0_enable : 1;
	u32 audio_out_chan1_enable : 1;
	u32 reserve : 26;
#elif defined (__BIG_ENDIAN_BITFIELD)
	u32 reserve : 26;
	u32 audio_out_chan1_enable : 1;
	u32 audio_out_chan0_enable : 1;
	u32 audio_out_chan1_DA_BIT_WIDE : 1;
	u32 audio_out_chan1_mute : 1;
	u32 audio_out_chan0_DA_BIT_WIDE : 1;
	u32 audio_out_chan0_mute : 1;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
    }STRUCT_PACKET_ALIGN(1);

    typedef union {
	struct aduio_decode_control_reg bit_value;
	u32     value;
    }STRUCT_PACKET_ALIGN(1) aduio_decode_control_reg_value;

    struct audio_adpcm_encode_ptr_ch0_ch9_reg { //reg 0x401c, 0x4024
#if defined(__LITTLE_ENDIAN_BITFIELD)
	u32 audio_chan0_ptr : 3;
	u32 audio_chan1_ptr : 3;
	u32 audio_chan2_ptr : 3;
	u32 audio_chan3_ptr : 3;
	u32 audio_chan4_ptr : 3;
	u32 audio_chan5_ptr : 3;
	u32 audio_chan6_ptr : 3;
	u32 audio_chan7_ptr : 3;
	u32 audio_chan8_ptr : 3;
	u32 audio_chan9_ptr : 3;
	u32 reserve : 2;
#elif defined (__BIG_ENDIAN_BITFIELD)
	u32 reserve : 2;
	u32 audio_chan9_ptr : 3;
	u32 audio_chan8_ptr : 3;
	u32 audio_chan7_ptr : 3;
	u32 audio_chan6_ptr : 3;
	u32 audio_chan5_ptr : 3;
	u32 audio_chan4_ptr : 3;
	u32 audio_chan3_ptr : 3;
	u32 audio_chan2_ptr : 3;
	u32 audio_chan1_ptr : 3;
	u32 audio_chan0_ptr : 3;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
    }STRUCT_PACKET_ALIGN(1);

    typedef union {
	struct audio_adpcm_encode_ptr_ch0_ch9_reg   bit_value;
	u32     value;
    }STRUCT_PACKET_ALIGN(1) audio_adpcm_encode_ptr_ch0_ch9_reg_value;

    struct audio_adpcm_encode_ptr_ch10_ch16_reg {   //reg 0x4020, 0x4028
#if defined(__LITTLE_ENDIAN_BITFIELD)
	u32 audio_chan10_ptr : 3;
	u32 audio_chan11_ptr : 3;
	u32 audio_chan12_ptr : 3;
	u32 audio_chan13_ptr : 3;
	u32 audio_chan14_ptr : 3;
	u32 audio_chan15_ptr : 3;
	u32 audio_chan16_ptr : 3;
	u32 reserve : 11;
#elif defined (__BIG_ENDIAN_BITFIELD)
	u32 reserve : 11;
	u32 audio_chan16_ptr : 3;
	u32 audio_chan15_ptr : 3;
	u32 audio_chan14_ptr : 3;
	u32 audio_chan13_ptr : 3;
	u32 audio_chan12_ptr : 3;
	u32 audio_chan11_ptr : 3;
	u32 audio_chan10_ptr : 3;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
    }STRUCT_PACKET_ALIGN(1);

    typedef union {
	struct audio_adpcm_encode_ptr_ch10_ch16_reg bit_value;
	u32 value;
    }STRUCT_PACKET_ALIGN(1) audio_adpcm_encode_ptr_ch10_ch16_reg_value;

    struct audio_decode_wr_rd_ptr_reg { //reg 0x402c
#if defined(__LITTLE_ENDIAN_BITFIELD)
	u32 chan0_rd_ptr : 4;
	u32 chan1_rd_ptr : 4;
	u32 chan0_wr_ptr : 4;
	u32 chan1_wr_ptr : 4;
	u32 reserve : 16;
#elif defined (__BIG_ENDIAN_BITFIELD)
	u32 reserve : 16;
	u32 chan1_wr_ptr : 4;
	u32 chan0_wr_ptr : 4;
	u32 chan1_rd_ptr : 4;
	u32 chan0_rd_ptr : 4;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
    }STRUCT_PACKET_ALIGN(1);

    typedef union {
	struct audio_decode_wr_rd_ptr_reg   bit_value;
	u32     value;
    }STRUCT_PACKET_ALIGN(1) audio_decode_wr_rd_ptr_reg_value;

    struct audio_pcm_encode_ptr_ch0_ch7_reg {   //reg 0x4030, 0x403c
#if defined(__LITTLE_ENDIAN_BITFIELD)
	u32 audio_chan0_ptr : 4;
	u32 audio_chan1_ptr : 4;
	u32 audio_chan2_ptr : 4;
	u32 audio_chan3_ptr : 4;
	u32 audio_chan4_ptr : 4;
	u32 audio_chan5_ptr : 4;
	u32 audio_chan6_ptr : 4;
	u32 audio_chan7_ptr : 4;
#elif defined (__BIG_ENDIAN_BITFIELD)
	u32 audio_chan7_ptr : 4;
	u32 audio_chan6_ptr : 4;
	u32 audio_chan5_ptr : 4;
	u32 audio_chan4_ptr : 4;
	u32 audio_chan3_ptr : 4;
	u32 audio_chan2_ptr : 4;
	u32 audio_chan1_ptr : 4;
	u32 audio_chan0_ptr : 4;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
    }STRUCT_PACKET_ALIGN(1);

    typedef union{
	struct audio_pcm_encode_ptr_ch0_ch7_reg bit_value;
	u32     value;
    }STRUCT_PACKET_ALIGN(1) audio_pcm_encode_ptr_ch0_ch7_reg_value;

    struct audio_pcm_encode_ptr_ch8_ch15_reg {  //reg 0x4034, 0x4040
#if defined(__LITTLE_ENDIAN_BITFIELD)
	u32 audio_chan08_ptr : 4;
	u32 audio_chan09_ptr : 4;
	u32 audio_chan10_ptr : 4;
	u32 audio_chan11_ptr : 4;
	u32 audio_chan12_ptr : 4;
	u32 audio_chan13_ptr : 4;
	u32 audio_chan14_ptr : 4;
	u32 audio_chan15_ptr : 4;
#elif defined (__BIG_ENDIAN_BITFIELD)
	u32 audio_chan15_ptr : 4;
	u32 audio_chan14_ptr : 4;
	u32 audio_chan13_ptr : 4;
	u32 audio_chan12_ptr : 4;
	u32 audio_chan11_ptr : 4;
	u32 audio_chan10_ptr : 4;
	u32 audio_chan09_ptr : 4;
	u32 audio_chan08_ptr : 4;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
    }STRUCT_PACKET_ALIGN(1);

    typedef union {
	struct audio_pcm_encode_ptr_ch8_ch15_reg    bit_value;
	u32     value;
    }STRUCT_PACKET_ALIGN(1) audio_pcm_encode_ptr_ch8_ch15_reg_value;

    struct audio_pcm_encode_ptr_ch16_reg {  //reg 0x4038, 0x4044
#if defined(__LITTLE_ENDIAN_BITFIELD)
	u32 audio_chan16_ptr : 4;
	u32 reserve : 28;
#elif defined (__BIG_ENDIAN_BITFIELD)
	u32 reserve : 28;
	u32 audio_chan16_ptr : 4;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
    }STRUCT_PACKET_ALIGN(1);

    typedef union {
	struct audio_pcm_encode_ptr_ch16_reg    bit_value;
	u32     value;
    }STRUCT_PACKET_ALIGN(1) audio_pcm_encode_ptr_ch16_reg_value;

    struct audio_enable_control_operation {
	int     (*get_adpcm_decode_state)(audio_enable_control_t *);
	void    (*update_enable_adpcm_decode)(audio_enable_control_t *, int en);
	void    (*set_enable_adpcm_decode)(audio_enable_control_t *, int en, isil_chip_t *);

	int     (*get_pcm_encode_state)(audio_enable_control_t *);
	void    (*update_enable_pcm_encode)(audio_enable_control_t *, int en);
	void    (*set_enable_pcm_encode)(audio_enable_control_t *, int en, isil_chip_t *);

	int     (*get_adpcm_encode_state)(audio_enable_control_t *);
	void    (*update_enable_adpcm_encode)(audio_enable_control_t *, int en);
	void    (*set_enable_adpcm_encode)(audio_enable_control_t *, int en, isil_chip_t *);

	void    (*init)(audio_enable_control_t *);
	void    (*get_enable_control_param)(audio_enable_control_t *, isil_chip_t *);
	void    (*set_enable_control_param)(audio_enable_control_t *, isil_chip_t *);
    };

    struct audio_enable_control {
	audio_enable_control_reg_value  value;
	u32         reg_offset;         //reg 0x4004
	struct audio_enable_control_operation   *op;
    };

    struct audio_pcm_encode_wr_rd_ptr_operation {
	void    (*init)(audio_pcm_encode_ptr_control_t *);
	void    (*get_pcm_wr_rd_ptr)(audio_pcm_encode_ptr_control_t *, isil_chip_t *);

	int     (*get_chan_wr_ptr)(audio_pcm_encode_ptr_control_t *, int , int *);
	int     (*get_chan_rd_ptr)(audio_pcm_encode_ptr_control_t *, int , int *);
	int     (*get_chan_section_number)(audio_pcm_encode_ptr_control_t *);

	void    (*set_rd_finish_number)(audio_pcm_encode_ptr_control_t *, int , isil_chip_t *);
    };

    struct audio_pcm_encode_ptr_control {
	audio_pcm_encode_ptr_ch0_ch7_reg_value  ch0_ch7_wr_ptr_value;
	u32 ch0_ch7_wr_ptr_reg_offset;          //reg 0x4030
	audio_pcm_encode_ptr_ch8_ch15_reg_value ch8_ch15_wr_ptr_value;
	u32 ch8_ch15_wr_ptr_reg_offset;         //reg 0x4034
	audio_pcm_encode_ptr_ch16_reg_value     ch16_wr_ptr_value;
	u32 ch16_wr_ptr_reg_offset;             //reg 0x4038

	audio_pcm_encode_ptr_ch0_ch7_reg_value  ch0_ch7_rd_ptr_value;
	u32 ch0_ch7_rd_ptr_reg_offset;          //reg 0x403c
	audio_pcm_encode_ptr_ch8_ch15_reg_value ch8_ch15_rd_ptr_value;
	u32 ch8_ch15_rd_ptr_reg_offset;         //reg 0x4040
	audio_pcm_encode_ptr_ch16_reg_value     ch16_rd_ptr_value;
	u32 ch16_rd_ptr_reg_offset;             //reg 0x4044

	u32 rd_finish_number_reg_offset;        //reg 0x4048

	int section_number;

	struct audio_pcm_encode_wr_rd_ptr_operation *op;
    };

    struct pci_audio_chan_sel_reg{
#if defined(__LITTLE_ENDIAN_BITFIELD)
	u32 pci_audio_chan_sel_0 : 1;
	u32 pci_audio_chan_sel_1 : 1;
	u32 pci_audio_chan_sel_2 : 1;
	u32 pci_audio_chan_sel_3 : 1;
	u32 pci_audio_chan_sel_4 : 1;
	u32 pci_audio_chan_sel_5 : 1;
	u32 pci_audio_chan_sel_6 : 1;
	u32 pci_audio_chan_sel_7 : 1;
	u32 pci_audio_chan_sel_8 : 1;
	u32 pci_audio_chan_sel_9 : 1;
	u32 pci_audio_chan_sel_10 : 1;
	u32 pci_audio_chan_sel_11 : 1;
	u32 pci_audio_chan_sel_12 : 1;
	u32 pci_audio_chan_sel_13 : 1;
	u32 pci_audio_chan_sel_14 : 1;
	u32 pci_audio_chan_sel_15 : 1;
	u32 pci_audio_flow_en : 1;
	u32 pci_audio_frame_toggle : 1;
	u32 reserve : 14;
#elif defined (__BIG_ENDIAN_BITFIELD)
	u32 reserve : 14;
	u32 pci_audio_frame_toggle : 1;
	u32 pci_audio_flow_en : 1;
	u32 pci_audio_chan_sel_15 : 1;
	u32 pci_audio_chan_sel_14 : 1;
	u32 pci_audio_chan_sel_13 : 1;
	u32 pci_audio_chan_sel_12 : 1;
	u32 pci_audio_chan_sel_11 : 1;
	u32 pci_audio_chan_sel_9 : 1;
	u32 pci_audio_chan_sel_8 : 1;
	u32 pci_audio_chan_sel_7 : 1;
	u32 pci_audio_chan_sel_6 : 1;
	u32 pci_audio_chan_sel_5 : 1;
	u32 pci_audio_chan_sel_4 : 1;
	u32 pci_audio_chan_sel_3 : 1;
	u32 pci_audio_chan_sel_2 : 1;
	u32 pci_audio_chan_sel_1 : 1;
	u32 pci_audio_chan_sel_0 : 1;
#else
#error	"Please fix <asm/byteorder.h>"
#endif    
    }STRUCT_PACKET_ALIGN(1);

    typedef union { //0x404c
	struct pci_audio_chan_sel_reg   bit_value;
	u32     value;
    }STRUCT_PACKET_ALIGN(1) pci_audio_chan_sel_reg_value;

#define	ADUIO_DATA_ADDR_PCM_ENCODE_TYPE     (0x00)
    struct audio_pcm_encode_data_addr_reg {
#if defined(__LITTLE_ENDIAN_BITFIELD)
	u32 section_offset : 7;
	u32 section_id : 3;
	u32 chan_id : 5;
	u32 pcm_type : 1;
	u32 reserve : 16;
#elif defined (__BIG_ENDIAN_BITFIELD)
	u32 reserve : 16;
	u32 pcm_type : 1;
	u32 chan_id : 5;
	u32 section_id : 3;
	u32 section_offset : 7;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
    }STRUCT_PACKET_ALIGN(1);

    typedef union {
	struct audio_pcm_encode_data_addr_reg   bit_value;
	u32     value;
    }STRUCT_PACKET_ALIGN(1) audio_pcm_encode_data_addr_reg_value;

    struct audio_pcm_encode_data_addr_operation{
	void    (*set_reg_bit_section_offset)(audio_pcm_encode_data_addr_t *, u32 );
	u32     (*get_reg_bit_section_offset)(audio_pcm_encode_data_addr_t *);

	void    (*set_reg_bit_section_id)(audio_pcm_encode_data_addr_t *, u32 );
	u32     (*get_reg_bit_section_id)(audio_pcm_encode_data_addr_t *);

	void    (*set_reg_bit_chan_id)(audio_pcm_encode_data_addr_t *, u32 );
	u32     (*get_reg_bit_chan_id)(audio_pcm_encode_data_addr_t *);

	void    (*set_reg_bit_pcm_type)(audio_pcm_encode_data_addr_t *);
	u32     (*get_reg_bit_pcm_type)(audio_pcm_encode_data_addr_t *);

	u32     (*get_pcm_encode_data_addr)(audio_pcm_encode_data_addr_t *);
	int     (*get_buf_page_id)(audio_pcm_encode_data_addr_t *);
	int     (*get_buf_in_chip_a_or_b)(audio_pcm_encode_data_addr_t *);
	u32     (*get_pcm_encode_data_addr_in_host_end_offset)(audio_pcm_encode_data_addr_t *);
	u32     (*get_pcm_encode_data_addr_in_ddr_end_offset)(audio_pcm_encode_data_addr_t *);
	void    (*init)(audio_pcm_encode_data_addr_t *, int , int);
    };

    struct audio_pcm_encode_data_addr {
	int buf_page_id, chip_a_or_b;
	audio_pcm_encode_data_addr_reg_value	value;
	struct audio_pcm_encode_data_addr_operation	*op;
    };

    struct audio_adpcm_encode_wr_rd_ptr_operation {
	void    (*init)(audio_adpcm_encode_ptr_control_t *);
	void    (*get_adpcm_wr_rd_ptr)(audio_adpcm_encode_ptr_control_t *, isil_chip_t *);

	int     (*get_chan_wr_ptr)(audio_adpcm_encode_ptr_control_t *, int , int *);
	int     (*get_chan_rd_ptr)(audio_adpcm_encode_ptr_control_t *, int , int *);
	int     (*get_chan_section_number)(audio_adpcm_encode_ptr_control_t *);

	void    (*set_rd_finish_number)(audio_adpcm_encode_ptr_control_t *, int , isil_chip_t *);
    };

    struct audio_adpcm_encode_ptr_control {
	audio_adpcm_encode_ptr_ch0_ch9_reg_value    ch0_ch9_wr_ptr_value;
	u32     ch0_ch9_wr_ptr_reg_offset;          //reg 0x401c
	audio_adpcm_encode_ptr_ch10_ch16_reg_value  ch10_ch16_wr_ptr_value;
	u32     ch10_ch16_wr_ptr_reg_offset;        //reg 0x4020

	audio_adpcm_encode_ptr_ch0_ch9_reg_value    ch0_ch9_rd_ptr_value;
	u32     ch0_ch9_rd_ptr_reg_offset;          //reg 0x4024
	audio_adpcm_encode_ptr_ch10_ch16_reg_value  ch10_ch16_rd_ptr_value;
	u32     ch10_ch16_rd_ptr_reg_offset;        //reg 0x4028

	u32     rd_finish_number_reg_offset;        //reg 0x4018
	int     section_number;

	struct audio_adpcm_encode_wr_rd_ptr_operation	*op;
    };

#define	ADUIO_DATA_ADDR_ADPCM_ENCODE_TYPE   (0x02)
    struct audio_adpcm_encode_data_addr_reg {
#if defined(__LITTLE_ENDIAN_BITFIELD)
	u32 section_offset : 7;
	u32 section_id : 2;
	u32 chan_id : 5;
	u32 adpcm_encode_type : 2;
	u32 reserve : 16;
#elif defined (__BIG_ENDIAN_BITFIELD)
	u32 reserve : 16;
	u32 adpcm_encode_type : 2;
	u32 chan_id : 5;
	u32 section_id : 2;
	u32 section_offset : 7;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
    }STRUCT_PACKET_ALIGN(1);

    typedef union {
	struct audio_adpcm_encode_data_addr_reg bit_value;
	u32     value;
    }audio_adpcm_encode_data_addr_reg_value;

    struct audio_adpcm_encode_data_addr_operation {
	void    (*set_reg_bit_section_offset)(audio_adpcm_encode_data_addr_t *, u32 );
	u32     (*get_reg_bit_section_offset)(audio_adpcm_encode_data_addr_t *);

	void    (*set_reg_bit_section_id)(audio_adpcm_encode_data_addr_t *, u32 );
	u32     (*get_reg_bit_section_id)(audio_adpcm_encode_data_addr_t *);

	void    (*set_reg_bit_chan_id)(audio_adpcm_encode_data_addr_t *, u32 );
	u32     (*get_reg_bit_chan_id)(audio_adpcm_encode_data_addr_t *);

	void    (*set_reg_bit_adpcm_type)(audio_adpcm_encode_data_addr_t *);
	u32     (*get_reg_bit_adpcm_type)(audio_adpcm_encode_data_addr_t *);

	u32     (*get_adpcm_encode_data_addr)(audio_adpcm_encode_data_addr_t *);
	int     (*get_buf_page_id)(audio_adpcm_encode_data_addr_t *);
	int     (*get_buf_in_chip_a_or_b)(audio_adpcm_encode_data_addr_t *);
	u32     (*get_adpcm_encode_data_addr_in_host_end_offset)(audio_adpcm_encode_data_addr_t *);
	u32     (*get_adpcm_encode_data_addr_in_ddr_end_offset)(audio_adpcm_encode_data_addr_t *);
	void    (*init)(audio_adpcm_encode_data_addr_t *, int , int);
    };

    struct audio_adpcm_encode_data_addr {
	int buf_page_id, chip_a_or_b;
	audio_adpcm_encode_data_addr_reg_value  value;
	struct audio_adpcm_encode_data_addr_operation   *op;
    };

    struct audio_encode_control_operation {
	void    (*enable_pcm_chan)(audio_encode_control_t *, int );
	void    (*disable_pcm_chan)(audio_encode_control_t *, int );
	void    (*set_enable_pcm_chan)(audio_encode_control_t *, int , int , int , isil_chip_t *);
	int     (*get_pcm_chan_state)(audio_encode_control_t *, int );
	void    (*init_audio_pcm)(audio_encode_control_t *);

	void    (*enable_adpcm_chan)(audio_encode_control_t *, int );
	void    (*disable_adpcm_chan)(audio_encode_control_t *, int );
	void    (*set_enable_adpcm_chan)(audio_encode_control_t *, int , int , isil_chip_t *);
	int     (*get_adpcm_chan_state)(audio_encode_control_t *, int );
	void    (*init_audio_adpcm)(audio_encode_control_t *);

	void    (*update_audio_sample_rate)(audio_encode_control_t *, int );
	int     (*get_audio_sample_rate)(audio_encode_control_t *);
	void    (*set_audio_sample_rate)(audio_encode_control_t *, int , isil_chip_t *);

	void    (*update_audio_type)(audio_encode_control_t *, int );
	int     (*get_audio_type)(audio_encode_control_t *);
	void    (*set_audio_type)(audio_encode_control_t *, int , isil_chip_t *);

	void    (*update_bit_wide)(audio_encode_control_t *, int );
	int     (*get_bit_wide)(audio_encode_control_t *);
	void    (*set_bit_wide)(audio_encode_control_t *, int , isil_chip_t *);

	void	(*pci_sel_pcm_or_adpcm)(audio_encode_control_t *, int , int );
	void	(*set_enable_pci_sel_pcm_or_adpcm)(audio_encode_control_t *, int , int , isil_chip_t *);
	void (*toggle_pci_flowcn)(audio_encode_control_t *, isil_chip_t *);
	void	(*init_audio_pci_sel)(audio_encode_control_t *);
	void    (*set_audio_param)(audio_encode_control_t *, isil_chip_t *);
	void    (*get_audio_param)(audio_encode_control_t *, isil_chip_t *);
    };

    struct audio_encode_control {
	audio_pcm_encode_control_reg_value  pcm_value;
	u32     pcm_reg_offset;     //reg 0x4008
	audio_adpcm_encode_control_reg_value    adpcm_value;
	u32     adpcm_reg_offset;   //reg 0x400c
	pci_audio_chan_sel_reg_value            sel_pcm_or_adpcm_based_pci_push;
	u32	sel_pci_pcm_adpcm_reg_offset;	//reg 0x404c

	struct audio_encode_control_operation   *op;
    };

    struct chip_audio_encode_operation {
	void    (*init)(chip_audio_encode_t *audio_encode);

	void    (*set_enable_audio_encode_chan)(chip_audio_encode_t *audio_encode, int chan_id, int enable, isil_chip_t *chip);
	void    (*update_enable_audio_encode_chan)(chip_audio_encode_t *audio_encode, int chan_id, int enable);
	int     (*get_audio_encode_chan_enable_state)(chip_audio_encode_t *audio_encode, int chan_id);

	int     (*get_audio_encode_chan_rd_ptr)(chip_audio_encode_t *audio_encode, int chan_id, int *ext_flag, int *queue_size);
	int     (*get_audio_encode_chan_wr_ptr)(chip_audio_encode_t *audio_encode, int chan_id, int *ext_flag, int *queue_size);

	void    (*get_audio_encode_param)(chip_audio_encode_t *audio_encode, isil_chip_t *chip);
	void    (*set_audio_encode_param)(chip_audio_encode_t *audio_encode, isil_chip_t *chip);
	void    (*set_finish_rd_ptr)(chip_audio_encode_t *audio_encode, int finish_rd_ptr, isil_chip_t *chip);
    };

#define	to_get_chip_audio_encode_with_running_param(node)           container_of(node, chip_audio_encode_t, running_param)
#define	to_get_chip_audio_encode_with_pcm_audio_data_base(node)     container_of(node, chip_audio_encode_t, pcm_audio_data_base)
#define	to_get_chip_audio_encode_with_adpcm_audio_data_base(node)   container_of(node, chip_audio_encode_t, adpcm_audio_data_base)
    struct chip_audio_encode {
	atomic_t                            need_sync_param;
	isil_audio_config_descriptor_t      config_param;
	isil_audio_param_t                  running_param;
	audio_encode_control_t              audio_control;
	audio_pcm_encode_ptr_control_t      pcm_audio_ptr;
	audio_pcm_encode_data_addr_t        pcm_audio_data_base;
	audio_adpcm_encode_ptr_control_t    adpcm_audio_ptr;
	audio_adpcm_encode_data_addr_t      adpcm_audio_data_base;

	struct chip_audio_encode_operation	*op;
    };

    struct audio_decode_control_operation {
	void    (*init)(audio_decode_control_t *);

	void    (*update_decode_chan_mute)(audio_decode_control_t *decode_control, int chan_id, int mute);
	int     (*get_decode_chan_mute)(audio_decode_control_t *decode_control, int chan_id);
	void    (*set_decode_chan_mute)(audio_decode_control_t *decode_control, int chan_id, int mute, isil_chip_t *chip);

	void    (*update_decode_chan_bit_wide)(audio_decode_control_t *decode_control, int chan_id, int bit_wide);
	int     (*get_decode_chan_bit_wide)(audio_decode_control_t *decode_control, int chan_id);
	void    (*set_decode_chan_bit_wide)(audio_decode_control_t *decode_control, int chan_id, int bit_wide, isil_chip_t *chip);

	void    (*update_decode_chan_enable)(audio_decode_control_t *decode_control, int chan_id, int enable);
	int     (*get_decode_chan_enable)(audio_decode_control_t *decode_control, int chan_id);
	void    (*set_decode_chan_enable)(audio_decode_control_t *decode_control, int chan_id, int enable, isil_chip_t *chip);

	void    (*set_decode_param)(audio_decode_control_t *decode_control, isil_chip_t *chip);
	void    (*get_decode_param)(audio_decode_control_t *decode_control, isil_chip_t *chip);
    };

    struct audio_decode_control {
	aduio_decode_control_reg_value  value;
	u32     reg_offset;
	struct audio_decode_control_operation   *op;
    };

    struct audio_decode_ptr_control_operation {
	void    (*init)(audio_decode_ptr_control_t *);
	void    (*get_adpcm_wr_rd_ptr)(audio_decode_ptr_control_t *, isil_chip_t *);

	int     (*get_chan_wr_ptr)(audio_decode_ptr_control_t *, int , int *);
	int     (*get_chan_rd_ptr)(audio_decode_ptr_control_t *, int , int *);
	int     (*get_chan_section_number)(audio_decode_ptr_control_t *);
	void    (*set_finish_wr_ptr)(audio_decode_ptr_control_t *, int finish_id, isil_chip_t *chip, int chan_id);
    };

    struct audio_decode_ptr_control {
	audio_decode_wr_rd_ptr_reg_value    wr_rd_value;
	u32 wr_rd_reg_offset;
	int section_number;

	struct audio_decode_ptr_control_operation   *op;
    };

#define	AUDIO_DATA_ADDR_ADPCM_DECODE_TYPE   (0x18)
    struct audio_decode_data_addr_reg {
#if defined(__LITTLE_ENDIAN_BITFIELD)
	u32 section_offset : 7;
	u32 section_id : 3;
	u32 chan_id : 1;
	u32 decode_type : 5;
	u32 reserve : 16;
#elif defined (__BIG_ENDIAN_BITFIELD)
	u32 reserve : 16;
	u32 decode_type : 5;
	u32 chan_id : 1;
	u32 section_id : 3;
	u32 section_offset : 7;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
    }STRUCT_PACKET_ALIGN(1);

    typedef union {
	struct audio_decode_data_addr_reg	bit_value;
	u32 value;
    }STRUCT_PACKET_ALIGN(1) audio_decode_data_addr_reg_value;

    struct audio_decode_data_addr_operation {
	void    (*set_reg_bit_section_offset)(audio_decode_data_addr_t *, u32 );
	u32     (*get_reg_bit_section_offset)(audio_decode_data_addr_t *);
	void    (*set_reg_bit_section_id)(audio_decode_data_addr_t *, u32 );
	u32     (*get_reg_bit_section_id)(audio_decode_data_addr_t *);
	void    (*set_reg_bit_chan_id)(audio_decode_data_addr_t *, u32 );
	u32     (*get_reg_bit_chan_id)(audio_decode_data_addr_t *);
	void    (*set_reg_bit_decode_type)(audio_decode_data_addr_t *);
	u32     (*get_reg_bit_decode_type)(audio_decode_data_addr_t *);
	u32     (*get_decode_data_addr)(audio_decode_data_addr_t *);
	int     (*get_buf_page_id)(audio_decode_data_addr_t *);
	int     (*get_buf_in_chip_a_or_b)(audio_decode_data_addr_t *);
	u32     (*get_adpcm_decode_data_addr_in_host_end_offset)(audio_decode_data_addr_t *);
	u32     (*get_adpcm_decode_data_addr_in_ddr_end_offset)(audio_decode_data_addr_t *);
	void    (*init)(audio_decode_data_addr_t *);
    };

    struct audio_decode_data_addr {
	int buf_page_id, chip_a_or_b;
	audio_decode_data_addr_reg_value	value;
	struct audio_decode_data_addr_operation	*op;
    };

    struct chip_audio_decode_operation {
	void    (*init)(chip_audio_decode_t *audio_decode);

	void    (*update_enable_audio_decode_chan)(chip_audio_decode_t *audio_decode, int chan_id, int enable);
	int     (*get_audio_decode_chan_enable_state)(chip_audio_decode_t *audio_decode, int chan_id);
	void    (*set_enable_audio_decode_chan)(chip_audio_decode_t *audio_decode, int chan_id, int enable, isil_chip_t *chip);

	void    (*update_mute_audio_decode_chan)(chip_audio_decode_t *audio_decode, int chan_id, int mute);
	int     (*get_audio_decode_mute_state)(chip_audio_decode_t *audio_decode, int chan_id);
	void    (*set_mute_audio_decode_chan)(chip_audio_decode_t *audio_decode, int chan_id, int mute, isil_chip_t *chip);

	int     (*get_audio_decode_chan_rd_ptr)(chip_audio_decode_t *audio_decode, int chan_id, int *ext_flag, int *queue_size);
	int     (*get_audio_decode_chan_wr_ptr)(chip_audio_decode_t *audio_decode, int chan_id, int *ext_flag, int *queue_size);

	void    (*get_audio_decode_param)(chip_audio_decode_t *audio_decode, isil_chip_t *chip);
	void    (*set_audio_decode_param)(chip_audio_decode_t *audio_decode, isil_chip_t *chip);
    };

#define	to_get_chip_audio_decode_with_running_param(node)       container_of(node, chip_audio_decode_t, running_param)
#define	to_get_chip_audio_decode_with_audio_data_base(node)	container_of(node, chip_audio_decode_t, audio_data_base)
    struct chip_audio_decode {
	atomic_t                        need_sync_param;
	isil_audio_config_descriptor_t  config_param;
	isil_audio_param_t              running_param;
	audio_decode_control_t          audio_control;
	audio_decode_ptr_control_t      audio_ptr;
	audio_decode_data_addr_t        audio_data_base;

	struct chip_audio_decode_operation	*op;
    };

    struct chip_audio_operation {
	int     (*init)(chip_audio_t *chip_audio);
	int     (*reset)(chip_audio_t *chip_audio);
	int     (*release)(chip_audio_t *chip_audio);

	void    (*set_encode_audio_chan_type)(chip_audio_t *chip_audio, int type);
	void    (*update_encode_audio_chan_type)(chip_audio_t *chip_audio, int type);
	int     (*get_encode_audio_chan_type)(chip_audio_t *chip_audio);

	void    (*set_decode_audio_chan_type)(chip_audio_t *chip_audio, int type);
	void    (*update_decode_audio_chan_type)(chip_audio_t *chip_audio, int type);
	int     (*get_decode_audio_chan_type)(chip_audio_t *chip_audio);

	void    (*set_enable_audio_chan)(chip_audio_t *chip_audio, int chan_id, int enable);
	void    (*update_enable_audio_chan)(chip_audio_t *chip_audio, int chan_id, int enable);
	int     (*get_audio_chan_enable_state)(chip_audio_t *chip_audio, int chan_id);

	void    (*set_mute_audio_chan)(chip_audio_t *chip_audio, int chan_id, int mute);
	void    (*update_mute_audio_chan)(chip_audio_t *chip_audio, int chan_id, int mute);
	int     (*get_audio_chan_mute_state)(chip_audio_t *chip_audio, int chan_id);

	void    (*get_chip_audio_param)(chip_audio_t *chip_audio);
	void    (*set_chip_audio_param)(chip_audio_t *chip_audio);

	void    (*process_audio_encode)(chip_audio_t *chip_audio);
	void    (*process_audio_decode)(chip_audio_t *chip_audio);
	void    (*get_audio_chan_driver)(chip_audio_t *chip_audio, int chan_id, isil_audio_driver_t **ptr_audio_driver);

	int     (*get_ddr_pages)(chip_audio_t *);
	int     (*free_ddr_pages)(chip_audio_t *);
    };

    typedef int (*audio_top_irq)(int irq, void *context);
    struct chip_audio_push_isr_interface_operation{
	void    (*init)(chip_audio_push_isr_interface_t *, int irq, void *context);
	void    (*open_chip_audio)(chip_audio_push_isr_interface_t *, int logic_chan_id);
	void    (*close_chip_audio)(chip_audio_push_isr_interface_t *, int logic_chan_id);
	void    (*disable_chip_audio_chan)(chip_audio_push_isr_interface_t *, int logic_chan_id);
	void    (*enable_chip_audio_chan)(chip_audio_push_isr_interface_t *, int logic_chan_id);
	int     (*curr_open_chip_audio_chan_number)(chip_audio_push_isr_interface_t *);
    };

    struct chip_audio_push_isr_interface{
	atomic_t	counter;
	spinlock_t	lock;
	audio_top_irq   chip_audio_isr;
	void	*context;
	int	irq;
	char    *chan0_chan3_push_ping_addr, *chan4_chan7_push_ping_addr, *chan8_chan11_push_ping_addr, *chan12_chan15_push_ping_addr;
	char    *chan0_chan3_push_pong_addr, *chan4_chan7_push_pong_addr, *chan8_chan11_push_pong_addr, *chan12_chan15_push_pong_addr;
	dma_addr_t  chan0_chan3_push_ping_dma_addr, chan4_chan7_push_ping_dma_addr, chan8_chan11_push_ping_dma_addr, chan12_chan15_push_ping_dma_addr;
	dma_addr_t  chan0_chan3_push_pong_dma_addr, chan4_chan7_push_pong_dma_addr, chan8_chan11_push_pong_dma_addr, chan12_chan15_push_pong_dma_addr;
	struct chip_audio_push_isr_interface_operation	*op;
    };

#define	to_get_chip_audio_with_audio_enable(node)           container_of(node, chip_audio_t, audio_enable)
#define	to_get_chip_audio_with_audio_encode(node)           container_of(node, chip_audio_t, audio_encode)
#define	to_get_chip_audio_with_audio_decode(node)           container_of(node, chip_audio_t, audio_decode)
#define	to_get_chip_audio_with_audio_push_interface(node)   container_of(node, chip_audio_t, audio_push_interface)
#define to_get_chip_audio_with_chip_audio_robust(node)      container_of(node, chip_audio_t, chip_audio_robust)
    struct chip_audio {
	audio_enable_control_t  audio_enable;
	chip_audio_encode_t     audio_encode;
	chip_audio_decode_t     audio_decode;
	isil_chip_t  *chip;

	chip_audio_push_isr_interface_t audio_push_interface;
	robust_processing_control_t     chip_audio_robust;
	struct chip_audio_operation	*op;

	isil_register_table_t logic_chan_table;
	isil_register_table_t opened_logic_chan_table;
    };


    extern isil_audio_config_descriptor_t default_audio_encode_config_descriptor;
    extern isil_audio_config_descriptor_t default_audio_decode_config_descriptor;

    extern int  get_power_base(u32 value);
    extern void init_audio_enable_control(audio_enable_control_t *);
    extern void init_audio_pcm_encode_ptr_control(audio_pcm_encode_ptr_control_t *);
    extern void init_audio_pcm_encode_data_addr(audio_pcm_encode_data_addr_t *);
    extern void init_audio_adpcm_encode_ptr_control(audio_adpcm_encode_ptr_control_t *);
    extern void init_audio_adpcm_encode_data_addr(audio_adpcm_encode_data_addr_t *);
    extern void init_audio_encode_control(audio_encode_control_t *);
    extern void init_audio_decode_control(audio_decode_control_t *);
    extern void init_audio_decode_ptr_control(audio_decode_ptr_control_t *);
    extern void init_audio_decode_data_addr(audio_decode_data_addr_t *);
    extern void init_chip_audio_encode(chip_audio_encode_t *);
    extern void init_chip_audio_decode(chip_audio_decode_t *);
    extern void start_chip_audio_robust_process(chip_audio_t *chip_audio);
    extern void isil_chip_audio_robust_process_done(chip_audio_t *chip_audio);
    extern void isil_chip_audio_wait_robust_process_done(chip_audio_t *chip_audio);
    extern int  isil_chip_audio_is_in_robust_processing(chip_audio_t *chip_audio);
    extern int  init_chip_audio(chip_audio_t *, isil_chip_t *, int );
    extern void remove_chip_audio(chip_audio_t *chip_audio);

    extern void init_submit_recv_audio_bitstream_service(dpram_request_t *dpram_req, isil_audio_driver_t *audio_chan_driver);
    extern void init_submit_send_audio_bitstream_service(dpram_request_t *dpram_req, isil_audio_driver_t *audio_chan_driver);

#ifdef __cplusplus
}
#endif

#endif	//ISIL_AUDIO_IOP

