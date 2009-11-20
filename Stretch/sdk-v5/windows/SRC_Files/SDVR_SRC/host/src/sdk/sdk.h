/************************************************************************
 *
 * Copyright C 2007 Stretch, Inc. All rights reserved.  Stretch products are
 * protected under numerous U.S. and foreign patents, maskwork rights,
 * copyrights and other intellectual property laws.  
 *
 * This source code and the related tools, software code and documentation, and
 * your use thereof, are subject to and governed by the terms and conditions of
 * the applicable Stretch IDE or SDK and RDK License Agreement
 * (either as agreed by you or found at www.stretchinc.com).  By using these
 * items, you indicate your acceptance of such terms and conditions between you
 * and Stretch, Inc.  In the event that you do not agree with such terms and
 * conditions, you may not use any of these items and must immediately destroy
 * any copies you have made.
 *
 ************************************************************************/

#ifndef STRETCH_SDK_H
#define STRETCH_SDK_H
#ifndef WIN32
#include <pthread.h>
#endif
#include "../codecs/avmux/mp4/mp4mux.h"

/************************************************************************
                                SDK VERSION
*************************************************************************/
#define SDVR_SDK_VERSION_MAJOR      5
#define SDVR_SDK_VERSION_MINOR      0
#define SDVR_SDK_VERSION_REVISION   1
#define SDVR_SDK_VERSION_BUILD      1

/**********************************************************************
  The SCT port ID to use to send large data information to the firmware.
  Currently this is used only for sending ROI map.
**********************************************************************/
#define SCT_PORT_SEND_DATA     0xFABE

/**********************************************************************
  The SCT port ID to use to send font table information to the firmware.
  This ID will be sent to the firmware everytime SDK needs to send a
  new font table.
**********************************************************************/
#define SCT_PORT_FONT_TABLE     0xABCD

/**********************************************************************
  The buffer size to allocate to send each font table chunk.
**********************************************************************/
#define XFR_SCT_FONT_TABLE_SIZE  (256 * 1024)
/**********************************************************************
    The hi word signature for channel handle created by the SDK
    NOTE: a channel handle consist of:
    bits 0  - 7 : Channel number
    bits 8  - 11: channel type
    bits 12 - 15: board index where the channel is on
    bits 16 - 31: The CHAN_HANDLE_SIGNATURE
***********************************************************************/
#define CHAN_HANDLE_SIGNITURE 0xBEEF
#define CH_GET_CHAN_NUM(handle)  (handle & 0xff)
#define CH_GET_CHAN_TYPE(handle) ((handle >> 8) & 0xf)
#define CH_GET_BOARD_ID(handle)  ((handle >> 12) & 0xf)
#define CH_IS_VALID_SIGNITURE(handle) (((handle >> 16) & 0x0000FFFF) == CHAN_HANDLE_SIGNITURE)

/************************************************************************
  These macros return different driver version parts given the
  32 bit driver version value.

    SDVR_MAJOR_DRV_VERSION - The driver major version number. A change in this field
    indicates major changes to functionality.

    SDVR_MINOR_DRV_VERSION - The driver minor version number. A change in this field 
    indicates minor changes to functionality.

    SDVR_REVISION_DRV_VERSION - The driver revision version number. A change in this
    field indicates significant bug fixes that was introduced in the minor 
    change functionality.

    SDVR_BUILD_DRV_VERSION - The driver build version number. A change in this field
    indicates only bug fixes that do not change functionality. 
************************************************************************/
#define SSDK_MAJOR_DRV_VERSION(dvr_version) (0x000000FF & dvr_version)
#define SSDK_MINOR_DRV_VERSION(dvr_version) (0x000000FF & (dvr_version >> 8))
#define SSDK_REVISION_DRV_VERSION(dvr_version) (0x000000FF & (dvr_version >> 16) )
#define SSDK_BUILD_DRV_VERSION(dvr_version) (0x000000FF & (dvr_version >> 24) )

/***********************************************************************
    The maximum number DVR PCI boards allowed to be installed on a PCI
************************************************************************/
#define MAX_PCI_BOARDS  8

/* The maximum file path */
#define MAX_FILE_PATH   256

/***********************************************************************
    The maximum number video encoder allowed per video channels
************************************************************************/
#define MAX_VIDEO_ENCODING 2

/***********************************************************************
   The maximum size of each SDK buffer.
***********************************************************************/
#define _SSDK_MAX_ENC_BUF   22
#define _SSDK_MAX_RAW_BUF   5
#define _SSDK_MAX_DEC_BUF   5

/***********************************************************************
   The maximum number of SMO supported by each board.
***********************************************************************/
#define MAX_NUM_SMO         8
#define SSDK_SMO_SEND_BUF_COUNT 3
/***********************************************************************
   The size of Motion Values Queue. This is hard coded and cannot be 
   changed by the DVR Application.
***********************************************************************/
#define _SSDK_MOTION_VALUE_QUEUE_SIZE 6
/***********************************************************************
 The callback function definition used for display. This is an internal
 callback function which provides hi-level HMO display. The parameters
 to this callback function are identical to what SCT is returning
***********************************************************************/
typedef void (*sdvr_ui_callback)(sdvr_chan_handle_t handle,
                                 dvr_data_header_t *yuvHdr, 
                                 void * context);

typedef struct _ssdvr_size{
    int width;
    int height;
} ssdvr_size_t;

/***********************************************************************
    A data structure holding the BDF file format font table information.
************************************************************************/
typedef struct {
  double startfont;
  char font[256];
  int size;
  int chars;
  int encoding;
  int bbw, bbh, bbxoff0x, bbyoff0y;
  int fbbx, fbby, xoff, yoff;
  int dwx0, dwy0;
} ssdk_bdf_t;

/***********************************************************************
    Debugging - A structure to hold various frame count for a channel
************************************************************************/
typedef struct {
    unsigned int y_u_vBufferRcvd;
    unsigned int y_u_vBuffersFreed; // There is one y_u_vBuffersFreed count
                                    // for every yBufferRcvr, uBufferRcvd, &
                                    // vBufferRecvd;
    unsigned int yuvFrameRcvd;
    unsigned int yuvFrameFreed;
    unsigned int yuvMissedCount;
    unsigned int yuvFrameAppRcvd;
    unsigned int yuvFrameAppFreed;
    unsigned int eFrameAppRcvd;
    unsigned int eFrameAppFreed;
    unsigned int eFrameRcvd;
    unsigned int eFrameFreed;
    unsigned int totalFramesRcvd;
    unsigned int totalFramesFreed;
} ssdk_dbg_frame_count_t;

/***********************************************************************
Different types of PCI channels that can receive A/V buffer from firmware
************************************************************************/
typedef enum {
    SSDK_RECV_FRAME_TYPE_NONE = -1,
    SSDK_RECV_FRAME_TYPE_ENC_PRIMARY = 0,
    SSDK_RECV_FRAME_TYPE_ENC_SECONDARY,
    SSDK_RECV_FRAME_TYPE_RAW_VIDEO,
    SSDK_RECV_FRAME_TYPE_RAW_AUDIO,
    SSDK_RECV_FRAME_TYPE_ENC_AUDIO,
    SSDK_RECV_FRAME_TYPE_MOTION_VALUES,

    // Add new frame types before this item
    SSDK_RECV_FRAME_TYPE_NUMS
}ssdk_recv_frame_type_e;

/************************************************************************
    A structure to hold a complete YUV 4-2-0 frame. Each buffer in this
    structure is of type sdvr_av_buffer_t
************************************************************************/

typedef struct {
    dvr_data_header_t *yuvHdr;
    sdvr_av_buffer_t *y_buffer;
    sdvr_av_buffer_t *u_buffer;
    sdvr_av_buffer_t *v_buffer;
} ssdk_yuv_frame_t;


/************************************************************************
    Temporary memory buffer to hold a complete YUV 4-2-0 frame.
************************************************************************/
#define YUV_BUF_SIGNITURE 0xFEEDBEEF

typedef struct _ssdk_yuv_malloc_item {
    sdvr_yuv_buffer_t yuv_frame; // The structure to hold the YUV frame.
                                // NOTE: This structure must be at the
                                // begining of the ssdk_yuv_malloc_item_t
                                // structure. Later on the DVR Application
                                // is only looking at the yuv_frame and
                                // does not know anything about the fields
                                // following it.

    sx_uint32 signiture;        // This signiture 0xFEEDBEEF indicates
                                // that this is a valid YUV temp buf.

    void *yuvHdr;             // The actual sct frame buffer to be released
} ssdk_yuv_malloc_item_t;

/***********************************************************************
    Data structure defining the pre-allocated memory to be used to 
    malloc temporary memory buffer for YUV frames.
***********************************************************************/
typedef struct _ssdk_yuv_malloc_buf_t {
    sx_int16 buf_list_size;     // The number of temprorary YUV buffer
                                // allocated. NOTE: This number is
                                // 3 times size of actually YUV buffers.

    sx_int8 free_list_ndx;     // The index of the first free item in
                                // the buf_list. 
                                // -1 indicates free list is empty.

    sx_int8 reserver;

} ssdk_yuv_malloc_buf_t;

/***********************************************************************
    Data structure defining each item in the A/V queue. 
************************************************************************/
typedef struct {
    int         next;          // The index to the next item in the queue
    sx_bool     avail;         // A boolean to indicate if this item is
                               // available to add a new frame.
    void        *data;         // The frame buffer
} ssdk_frame_queue_item_t;

/***********************************************************************
    Data structure defining the A/V queue. This data structure is shared
    between both encoded and raw A/V frames that are sent to the host.
   
************************************************************************/
typedef struct {
    int     start_item;          // The first item of the queue;
    int     top_item;            // The top of the queue where we should
                                 // place the new frame.
    sx_uint8 size;               // The size of the queue buffer.

    sx_uint8 num_recvd_frames;   // The number of frames recieved
                                 // and not freed yet.

    ssdk_frame_queue_item_t *frame_queue;  // A list of A/V frames that was 
                                 // just received from the PCI driver. 
} ssdk_frame_queue_t;

typedef enum _ssdk_recording_state_e
{
    SSDK_REC_STATE_STOP = 0,
    SSDK_REC_STATE_STOP_PENDING,
    SSDK_REC_STATE_START
} ssdk_recording_state_e;

typedef struct
{
    ssdk_recording_state_e rec_state; // The current state of recording

	sx_int32 vtrack;            // Video track handle within mp4 file
    sx_int32 atrack;            // Audio track handle within mp4 file
    mp4_handle fh;              // The file handle to the mp4 file.
    mp4_handle new_fh;          // The new file handle to be used when
                                // we are in transission of saving the GOP
                                // for a the previous file and moving to the
                                // the file.
//    sx_int64 dts;
} mp4File_t;

/***********************************************************************
    Data structure defining the sub-encoder definitions assocated to
    an encoder channel.
   
************************************************************************/

typedef struct {
                                // The type of video encoding for this
                                // sub-encoder
    dvr_vc_format_e     venc_type;

                                // A flag to indicate if the encoded A/V is
                                // streaming. If this flag is set to zero
                                // Any encoded A/V video frame on this channel
                                // will be discarded.
    sx_bool             is_enc_av_streaming;

                                // The current resolution decimation of 
                                // this encoding.
    int                 res_decimation;
                
                                // File information related to recording.
    mp4File_t           mp4_file_info;
                                // A flag to make sure a recorded file
                                // always starts with SPS
    sx_bool             has_sps_vol;
} ssdk_sub_enc_info_t;
/***********************************************************************
    Data structure defining the encoded PCI port channel
************************************************************************/

typedef struct {        
                                    // The channel handle needed for all
                                    // the sdk calls.
    sdvr_chan_handle_t sdk_chan_handle;

    sct_channel_t  pci_chan_handle; // This handle is needed for all the
                                    // communications with the SCT calls

    // A list of buffers to queue up different frame types that are sent 
    // to the SDK. The memory for each of these buffers are allocated at
    // the time the channel is setup and then will be release when the
    // channel is destroyed.
    // NOTE: Each item in frame queue of enc_av_frame_queue, 
    //       and raw_audio_frame_queue should be type
    //       casted to *sdvr_av_buffer_t.
    //       Each item in frame queue of yuv_frame_queue should be
    //       type casted to *ssdk_yuv_frame_t
    //       enc_av_frame_queue is and array of frame queue for
    //       each supported encoder on this channel.
    ssdk_frame_queue_t  enc_av_frame_queue[MAX_VIDEO_ENCODING];

    ssdk_frame_queue_t  yuv_frame_queue;

    ssdk_frame_queue_t  raw_audio_frame_queue;

    ssdk_frame_queue_t  enc_audio_frame_queue;

    ssdk_frame_queue_t  motion_values_frame_queue;

                                // A flag to indicate if SMO is enabled
                                // on this channel. This is needed for
                                // deciding whether we should re-start a
                                // channel after we change any of its
                                // attributes/
    sx_bool             is_smo_enable;

                                // A flag to indicate if the raw A/V is
                                // streaming. If this flag is set to zero
                                // Any raw A/V video frame on this channel
                                // will be discarded.
    sx_bool             is_raw_av_streaming[MAX_VIDEO_ENCODING];

                                // A flag to indicate whether to stream raw or
                                // encoded audio. Once the raw audio is enabled
                                // for a channel, the encoded audio will be paused.
    sx_bool             is_raw_audio_streaming;

                                // information specific to a sub-encoder on this
                                // encoder channel.
    ssdk_sub_enc_info_t sub_enc_info[MAX_VIDEO_ENCODING];

								// A flag to indicate if the channels is in
								// middle of raw A/V, encoded streaming or
								// SMO is enabled.
	sx_bool				is_channel_running;


                                // The type of audio encoder;
    dvr_ac_format_e     aenc_type;

                                // Structure to hold different frames 
                                // information for debugging purposes.
    ssdk_dbg_frame_count_t dbg_frame_count;

                                // Arrays holding which regions are defined
                                // Any array item with value of 1 means
                                // the region is defined.
    sx_bool         md_defined_regions[SDVR_MAX_MD_REGIONS];
    sx_bool         bd_defined_regions[SDVR_MAX_MD_REGIONS];
    sx_bool         pr_defined_regions[SDVR_MAX_MD_REGIONS];

    sx_uint64       user_data;  // A 64 bit user specified data assoicated
                                // to this channel.

                                // Map to indicate which OSD item was defined.
                                // Note: If an OSD is not configured, it can
                                //       not be shown.
    sx_bool         osd_defined_map[SDVR_MAX_OSD];
    sdvr_ui_callback    ui_cb_func;
    void *              ui_cb_context;
                                // The mutex to handle changing of
                                // current state of recoridng.
#ifdef WIN32
    CRITICAL_SECTION cs_chan_rec;
#else
    pthread_mutex_t  cs_chan_rec;
#endif

                                // The SDK caches the vpp actions
                                // so that the DVR Application can enable or
                                // disable each one individually without 
                                // affecting other actions. By default
                                // all the actions are enabled.
    sx_uint32       vpp_action_flags;
} ssdk_pci_channel_info_t;

/***********************************************************************
    Data structure defining the decoded PCI port channel
************************************************************************/

typedef struct _ssdk_pci_decode_chan_info_t {        
                                    // The channel handle needed for all
                                    // the sdk calls.
    sdvr_chan_handle_t sdk_chan_handle;

    sct_channel_t  pci_chan_handle; // This handle is needed for all the
                                    // message communications with the SCT 
                                    // calls
    sct_channel_t pci_send_chan_handle; // The handle to be used to send
                                    // A/V frames using SCT calls.



    // A list of buffers to queue up different frame types that are sent 
    // to the SDK. The memory for each of these buffers are allocated at
    // the time the channel is setup and then will be released when the
    // channel is destroyed.
    // NOTE: Each item in frame queue of raw_audio_frame_queue should
    //       be type casted to *sdvr_av_buffer_t
    //       Each item in frame queue of yuv_frame_queue should be
    //       type casted to *ssdk_yuv_frame_t
    ssdk_frame_queue_t  yuv_frame_queue;

    ssdk_frame_queue_t  raw_audio_frame_queue;

                                // A flag to indicate if the decoding of A/V
                                // is enabled. This is needed for
                                // deciding whether we should re-start a
                                // channel after we change any of its
                                // attributes
    sx_bool             is_dec_av_enable;


                                // A flag to indicate if SMO is enabled
                                // on this channel. This is needed for
                                // deciding whether we should re-start a
                                // channel after we change any of its
                                // attributes/
    sx_bool             is_smo_enable;

                                // A flag to indicate if the raw A/V is
                                // streaming. If this flag is set to zero
                                // Any raw A/V video frame on this channel
                                // will be discarded.
    sx_bool             is_raw_av_streaming;

                                // The video size supported by this decoder
                                // The decoder can not decode frame that
                                // are not exactly this size. These are
                                // set at the time the decoder is created.
    int                 vsize_width;
    int                 vsize_lines;

                                // Number of buffers availabe to send to FW
                                // for decoding. If this number is zero
                                // we should not attempt to send any new
                                // decode buffer.
    sx_uint32           avail_decode_buf_count;

								// A flag to indicate if the channels is in
								// middle of raw A/V, encoded streaming or
								// SMO is enabled.
	sx_bool				is_channel_running;

                                // The type of video decoder;
    dvr_vc_format_e     vdecoder_type;

                                // The type of audio decoder;
    dvr_ac_format_e     adecoder_type;

                                // Structure to hold different frames 
                                // information for debugging purposes.
    ssdk_dbg_frame_count_t dbg_frame_count;

                                // Arrays holding which regions are defined
                                // Any array item with value of 1 means
                                // the region is defined.
    sx_bool             md_defined_regions[SDVR_MAX_MD_REGIONS];
    sx_bool             bd_defined_regions[SDVR_MAX_MD_REGIONS];
    sx_bool             pr_defined_regions[SDVR_MAX_MD_REGIONS];

    sx_uint64           user_data; // A 64 bit user specified data assoicated
                                   // to this channel.

                                // Map to indicate which OSD item was defined.
                                // Note: If an OSD is not configured, it can
                                //       not be showned.
    sx_bool             osd_defined_map[SDVR_MAX_OSD];

    sdvr_ui_callback    ui_cb_func;
    void *              ui_cb_context;

} ssdk_pci_decode_chan_info_t;

typedef struct _ssdk_smo_info_t {
    sct_channel_t pci_send_chan_handle; // The handle to be used to send
                                        // A/V frames using SCT calls.

                                    // The channel handle needed for all
                                    // the sdk calls.
    sdvr_chan_handle_t sdk_chan_handle;

                                // Number of buffers availabe to send to FW.
                                // If this number is zero, we should not attempt
                                //  to send any new frame buffer.
    sx_uint32     avail_send_buf_count;
                                // The raw video format that is being sent to
                                // the output monitor. (i.e. 4-2-0)
    sx_uint8      video_format;
    sx_uint16     width;        // Width of the SMO screen.
    sx_uint16     height;       // Height of the SMO screen.

                                // The Y, U, V size according to the
                                // requested video_format.
    sx_uint32 y_buf_size;   
    sx_uint32 u_buf_size;
    sx_uint32 v_buf_size;

                                // Map to indicate which OSD item was defined.
                                // Note: If an OSD is not configured, it can
                                //       not be showned.
    sx_bool       osd_defined_map[SDVR_MAX_OSD];
}ssdk_smo_info_t;
/***********************************************************************
    Data structure defining the encoded/decoded PCI port channels
    description per DVR PCI board
************************************************************************/

typedef struct {
    const char *name;           // The PCI board name that that is returned
                                // from scr_get_board_name

    sct_board_t handle;         // The board handle needed to make SCT calls
                                // for general commands

    sct_channel_t pci_send_data_handle; // The SCT channel handle to be used
                                // to send large buffers to the firmware.
                                // This mostly used for sending of different
                                // ROIs map. NOTE: This handle is SCT channel
                                // gets openned on the first call to send any of the
                                // ROIs map and keeps open until the board is closed.
    int pci_send_data_buf_size; // The maximum size of each sct send data buffer.
                                // This is usually the size of a D1 video frame for
                                // the current video standard.

    ml_versions boot_loader_ver; // The boot-loader version. 
                                 // This gets sets everytime we load a firmware.

    sdvr_firmware_ver_t version; // The firmware, bsp, boot-loader version. This gets
                                // set every time we connect to a board.

    sdvr_video_std_e video_std; // The video standard for this board
    int         vstd_width;     // The width of the current video std
    int         vstd_lines;     // The number of lines of the current video std

    sx_uint32 board_index;      // The current board index. The address of
                                // this field is used to send to the driver
                                // to save board_index for callback functions.

    sx_uint16    max_num_video_in;   // The maximum number of video-in channels
                                    // on the board. (i.e camera connections)

    sx_uint16    max_num_decoders; // The maximum number of decoding channel
                                // supported by the DVR board.

    sx_int32 slot_number;       // The PCI slot number that this boars is on
                                
                                // A list containing different PCI buffer 
                                // information for each camera channel.
                                // NOTE: Assoicated to each camera we
                                //       recieve A/V frame for the primary
                                //       or secondary encoded channel or just
                                //       raw A/V frame.
                                // The index to this array corresponds to
                                // the items in the ssdk_recv_chan_type_e
    ssdk_pci_channel_info_t *recv_channels;
	
                                // A list containing all the decode channels
                                // informat.
                                // NOTE: Associated to each decoder we can
                                //       recieve raw A/V frames. The
                                //       encode A/V frames to be sent for 
                                //       decoding are not being buffered.
    ssdk_pci_decode_chan_info_t *decode_channels;

								// A flag to indicate whether on board connect
								// we must issue sct_reset.
	sx_bool			is_reset_required;

                                // A flag to indicate the last call to 
                                // recieve a reply, timed-out. We assume
                                // the firmware is crashed. All the future
                                // calls to _ssdk_message_recv will be ingnored
    sx_bool         is_fw_dead;

    sx_bool         is_h264_SCE;// If true, the SDK performs  start code 
                                // emulation for all the h.264 video frames;
                                // except SPS and PPS frames.

                                // The key to be used in H.264 frame authentication.
    sx_uint8        auth_key[DVR_AUTH_KEY_LEN];
    sx_uint32       auth_key_len; // Number of bytes used in auth_key[]

    sx_uint32       max_num_smos; // Number of SMOs supported by this board
    ssdk_smo_info_t  smo_info[MAX_NUM_SMO];
    sx_uint16       max_num_encoders;

    sx_uint32 max_sct_buf_size;       // The size of largest SCT buffer on this board.
    
    sdvr_audio_rate_e audio_rate; // The sampling audio rate used on this board
                                  // default is DVR_AUDIO_RATE_8KHZ

} ssdk_dvr_board_info_t;

/***********************************************************************
    Data structure defining the PCI driver and boards information.
************************************************************************/
typedef struct {
    
    sx_uint8 board_num;        // Number of boards installed                               
    

                                // An array holding all information about 
                                // channels on every  PCIe boards that are 
                                // connected.
    ssdk_dvr_board_info_t boards[MAX_PCI_BOARDS];

                                // Mutex variable to handle critical section.
                                // Note: There is only one mutex to handle
                                //       frame data for all the boards.
#ifdef WIN32
    CRITICAL_SECTION cs_board_info;
#else
    pthread_mutex_t  cs_board_info;
#endif

                                // Mutex variable to handle critical section.
                                // Note: There is only one mutex to handle
                                //       decode buffer management count for
                                //       all the deocoders.
#ifdef WIN32
    CRITICAL_SECTION    cs_decode_buf_count;
#else
    pthread_mutex_t     cs_decode_buf_count;
#endif
                                // The callback function provided by the DVR
                                // application to be called everytime 
                                // a new A/V frame is arrived.
    void (*av_frame_callback)(sdvr_chan_handle_t handle,
                              sdvr_frame_type_e frame_type, 
                              sx_bool primary_frame);

                                // The callback function provided by the DVR
                                // application to be called every time any 
                                // sensor is activated on this board
    void (*sensor_callback) (sx_uint32 board_index, 
                             sx_uint32 sensor_map);

                                // The callback function provided by the DVR
                                // application to be called every time an 
                                // alarm is detected on this board
    void (*video_alarm_callback) (sdvr_chan_handle_t handle,
                                  sdvr_video_alarm_e alarm_type,
                                  sx_uint32 data);


                                // The callback function provided by the DVR
                                // application to be called every time  
                                // sct confirms the frame sent.
    void (*conf_callback) (sdvr_chan_handle_t handle);

                                // The callback function provided by the DVR
                                // application to be called every an asynch
                                // message is sent on this board
    void (*signals_callback) (sx_uint32 board_index,
                             sdvr_signal_info_t * signal_info);

                                // The callback function provided by the DVR
                                // application to be called every time  
                                // we need to display a tracing message. 
    void (*disp_debug_callback) (char *message);

                                // The full path to the log file.
    char log_file_name[MAX_FILE_PATH + 1];

                                // The last custom font table defined.
                                // Current there can only be one font
                                // table defined. The range of this
                                // field is 8 - 16. 0 means not defined yet.
    sx_uint8 custom_font_table_id;

    unsigned char *temp_venc_buf; // The buffer which holds video encoded
                                  // SCE.
    sx_uint32 temp_venc_buf_size; // The size of temp venc buffer.

                                // This flag only is kept within the SDK
                                // and never sent to the board.
    sx_bool         is_auth_key_enabled;

                                // This flag indicates whether SDK to convert
                                // The raw video stream from 4-2-0 to 4-2-2
    sx_uint8        yuv_format;

} ssdk_dvr_boards_info_t;


#ifdef STRETCH_SDK_C
#define SDK_EXTERN

/***********************************************************************
    The user specified SDK settings
************************************************************************/
static sdvr_sdk_params_t m_sdkParams = {
    20,             // enc_buf_num
    2,              // raw_buf_num
    5,              // dec_buf_num - maximum #buffer allowed by sct
    414720,         // dec_buf_size = 720 x 576
    5,              // timeout
    0,              // debug_flag
    NULL,           // debug_file_name;
};

static void _ssdk_decoder_send_conf_callback_func( void * context, void * buffer, int size);
static void _ssdk_av_frame_callback_func( void * context, void * buffer, int size);
static void _ssdk_av_frame_decoder_callback_func( void * context, void * buffer, int size);
static void _ssdk_message_callback_func( void * context, sx_int32  msg_class, void * buffer);
static void *_ssdk_new_yuv_buf( sdvr_chan_handle_t handle, dvr_data_header_t *yuvHdr);
static sx_uint32 _ssdk_remove_yuv_padding( sdvr_chan_handle_t handle,
                              sx_uint16 padded_width,
                              sx_uint16 active_width,
                              sx_uint16 active_height,
                              sx_uint8 *dest_buf,
                              sx_uint8 *src_buf);
static sdvr_err_e _ssdk_delete_yuv_buf( ssdk_yuv_malloc_item_t *yuv_buffer);
static void _ssdk_convert_YUV420_YUV422(dvr_data_header_t *yuvHdr);
static char *_ssdk_rec_frame_type_text(ssdk_recv_frame_type_e recv_frame_type);
static ssdk_frame_queue_t *_ssdk_get_frame_queue(sdvr_chan_handle_t handle, ssdk_recv_frame_type_e recv_frame_type, sx_bool *is_raw_video);
static sdvr_err_e _ssdk_queue_frame( sdvr_chan_handle_t handle, void * buffer, ssdk_recv_frame_type_e frame_type);
static sdvr_err_e _ssdk_message_send(sdvr_chan_handle_t handle, sx_uint8 b_index, sx_int32  msg_class, void * buffer, sx_int32 buf_size );
static sdvr_err_e _ssdk_message_recv( sdvr_chan_handle_t handle, sx_uint8 b_index,
      sx_int32 msg_class, void *buffer, sct_board_t *psrc_board, sx_int32 *pmsg_class );
static sx_int16 _int16_little_endian_TO_big_endian(sx_int16 nValue);
static sx_int32 _int32_little_endian_TO_big_endian(sx_int32 nLongNumber);
static void  _ssdk_do_message_byte_swap(sx_uint32 board_index, sx_int32  msg_class, void * buffer);
static sx_bool _is_byte_swap;

unsigned int sdbg_totalFrame_count_rcvd;
unsigned int sdbg_totalFrame_count_freed;
#else
#if defined(__cplusplus)
   #define SDK_EXTERN              extern "C"
#else
   #define SDK_EXTERN              extern
#endif

#endif 


/***********************************************************************
    The PCI DVR Board informations. Such as the number of boards, etc.
************************************************************************/
SDK_EXTERN ssdk_dvr_boards_info_t _ssdvr_boards_info;

/***********************************************************************
	A flag indicating if we are in the message or A/V frame callback.
	If this flag is set, no debugging information will be send to display.
	This is needed to fix a deadlock situation between SCT and DVR app.
************************************************************************/
SDK_EXTERN sx_bool ssdvr_in_callback;
/************************************************************************
                      Internal SDK Function Prototypes
*************************************************************************/

SDK_EXTERN ssdk_dbg_frame_count_t *ssdk_get_dbg_frame_count(sdvr_chan_handle_t handle);
SDK_EXTERN sdvr_err_e ssdk_validate_board_ndx(sx_uint32 board_index);
SDK_EXTERN sdvr_err_e ssdk_validate_chan_handle(sdvr_chan_handle_t handle, int chk_chan_type); 
SDK_EXTERN sdvr_err_e ssdk_validate_board_chan_type(sx_uint8 board_index, sx_uint8 chan_num, sx_uint8 chan_type);
SDK_EXTERN int ssdk_get_max_chan_num(int board_index, sx_uint8 chan_type);

SDK_EXTERN sdvr_err_e ssdk_validate_enc_sub_chan(sdvr_chan_handle_t handle,
                                              sdvr_sub_encoders_e sub_chan_enc);
SDK_EXTERN dvr_job_type_e ssdk_chan_to_job_type(sdvr_chan_type_e chan_type);
SDK_EXTERN dvr_vc_format_e ssdk_to_fw_video_frmt(sdvr_venc_e video_format);
SDK_EXTERN dvr_ac_format_e ssdk_to_fw_audio_frmt(sdvr_aenc_e audio_format);
SDK_EXTERN void ssdk_register_sdk_chan_callback(sct_channel_t pci_handle,sdvr_chan_handle_t handle);
SDK_EXTERN sdvr_err_e ssdk_set_board_config(sx_uint32 board_index, 
                                 sdvr_board_settings_t *board_settings);

SDK_EXTERN sdvr_err_e ssdvr_get_supported_vstd(sx_uint32 board_index, 
                                               sx_uint16  *video_stds);
SDK_EXTERN sdvr_err_e ssdk_drv_open_rcv(sdvr_chan_handle_t handle, sx_int8 chan_enc_num);
SDK_EXTERN sdvr_err_e ssdk_drv_close_rcv(sdvr_chan_handle_t handle);
SDK_EXTERN sdvr_err_e ssdk_drv_open_decode_xfr(sdvr_chan_handle_t handle);
SDK_EXTERN sdvr_err_e ssdk_drv_close_decode_xfr(sdvr_chan_handle_t handle);

SDK_EXTERN void ssdk_reset_enc_chan_info(sdvr_chan_handle_t handle);

SDK_EXTERN sdvr_err_e ssdk_dequeue_frame( sdvr_chan_handle_t handle, void **buffer, 
                                ssdk_recv_frame_type_e recv_frame_type);
SDK_EXTERN sdvr_err_e ssdk_release_frame(void *frame_buf, sx_bool is_encoded_frame);

SDK_EXTERN void _ssdk_mutex_lock();
SDK_EXTERN void _ssdk_mutex_unlock();
SDK_EXTERN void _ssdk_mutex_decode_buf_lock();
SDK_EXTERN void _ssdk_mutex_decode_buf_unlock();
SDK_EXTERN void _ssdk_mutex_chan_rec_lock(sdvr_chan_handle_t handle);
SDK_EXTERN void _ssdk_mutex_chan_rec_unlock(sdvr_chan_handle_t handle);

SDK_EXTERN sdvr_display_debug_callback ssdvr_get_display_bebug_callback();
SDK_EXTERN sx_uint32 _ssdk_get_debug_flag();
SDK_EXTERN sdvr_chan_handle_t ssdk_create_chan_handle(sx_uint8 board_index,
                                                  sdvr_chan_type_e  chan_type,
                                                  sx_uint8          chan_num);
SDK_EXTERN sdvr_err_e ssdk_disable_channel(sdvr_chan_handle_t handle, 
                                           sx_bool *was_contrl_running);
SDK_EXTERN sdvr_err_e ssdk_enable_channel(sdvr_chan_handle_t handle);
SDK_EXTERN void ssdk_enable_raw_av(sdvr_chan_handle_t handle, sx_uint8 stream_id, sx_bool enable,
                        sdvr_video_res_decimation_e res_decimation);

SDK_EXTERN void ssdk_enable_enc_av(sdvr_chan_handle_t handle, 
                                   sdvr_sub_encoders_e sub_chan_enc,
                                   sx_bool enable);
SDK_EXTERN sdvr_err_e ssdvr_send_chan_param(sdvr_chan_handle_t handle, 
											sx_uint32 command, 
											void *data, 
											sx_uint32 data_size);
SDK_EXTERN sdvr_err_e ssdvr_enable_chan(sdvr_chan_handle_t handle);
SDK_EXTERN sdvr_err_e ssdk_message_chan_send(sdvr_chan_handle_t handle, 
					   sx_int32  msg_class, 
					   void * buffer, 
					   sx_int32 buf_size );
SDK_EXTERN sdvr_err_e ssdk_message_board_send(sx_uint8 b_index,
					   sx_int32  msg_class, 
					   void * buffer, 
					   sx_int32 buf_size );
SDK_EXTERN sdvr_err_e ssdk_message_chan_recv(sdvr_chan_handle_t handle,
                  sx_int32        msg_class,
                  void *          buffer,
                  sct_board_t *   psrc_board,
                  sx_int32 *      pmsg_class );
SDK_EXTERN sdvr_err_e ssdk_message_board_recv(sx_uint8 b_index,
                  sx_int32        msg_class,
                  void *          buffer,
                  sct_board_t *   psrc_board,
                  sx_int32 *      pmsg_class );
SDK_EXTERN void ssdk_sct_buffer_free( sct_channel_t    channel, void * p );

SDK_EXTERN sx_bool ssdk_is_chan_streaming(sdvr_chan_handle_t handle);
SDK_EXTERN sx_bool ssdk_is_chan_running(sdvr_chan_handle_t handle);

SDK_EXTERN void ssdk_get_width_hieght_vstd(sdvr_video_std_e video_std, int *width,
                           int *lines);
SDK_EXTERN int ssdk_get_vstd_frame_rate(sdvr_video_std_e video_std, int *frame_rate);
SDK_EXTERN sdvr_err_e ssdk_get_chan_av_codec(sdvr_chan_handle_t handle,
                                        sdvr_sub_encoders_e   stream_id,
                                        sdvr_venc_e *video_codec,
                                        sdvr_aenc_e *audio_codec);

SDK_EXTERN sdvr_err_e ssdk_queue_yuv_frame(sdvr_chan_handle_t handle,  dvr_data_header_t *yuvHdr);
SDK_EXTERN int sdvr_avail_decoder_buf_count(sdvr_chan_handle_t handle);
SDK_EXTERN sx_bool ssdk_is_min_version(sx_uint8 major, sx_uint8 minor, sx_uint8 revision, sx_uint8 build,
                 sx_uint8 majorChk, sx_uint8 minorChk, sx_uint8 revisionChk, sx_uint8 buildChk);

SDK_EXTERN ssdk_pci_decode_chan_info_t * ssdk_get_dec_chan_strct(sx_uint8 board_index, sx_uint8 chan_num);
SDK_EXTERN ssdk_pci_channel_info_t * ssdk_get_enc_chan_strct(sx_uint8 board_index, sx_uint8 chan_num);

SDK_EXTERN ssdvr_size_t get_frame_size(sdvr_video_std_e vs, sdvr_video_res_decimation_e dec);
SDK_EXTERN int ssdk_get_video_rec_duration(sdvr_video_std_e video_standard);

SDK_EXTERN sdvr_err_e ssdvr_write_Enc_VFrames(sdvr_chan_handle_t handle, sdvr_av_buffer_t *buf);
SDK_EXTERN sdvr_err_e ssdvr_write_Enc_AFrames(sdvr_chan_handle_t handle, sdvr_av_buffer_t *buf);

SDK_EXTERN sdvr_err_e ssdvr_read_auth_key(sx_uint32 board_index);

/****************************************************************************
  Internal API used only by ui display dll
****************************************************************************/
SDK_EXTERN sx_bool ssdk_set_ui_callback(sdvr_chan_handle_t handle, sdvr_ui_callback callback_func, void * context);
SDK_EXTERN void *ssdk_get_ui_context(sdvr_chan_handle_t handle);
SDK_EXTERN void ssdk_free_ui_context(void *pdi);
SDK_EXTERN void *ssdk_malloc_ui_context(size_t size);
#endif //STRETCH_SDK_H


