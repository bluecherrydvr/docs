#define H264_BUF_CNT 2
#define AUDIO_BUF_CNT 8
#define PREVIEW_BUF_CNT 16
#define MV_BUF_CNT 2
#define JPEG_BUF_CNT 1

#define PREVIEW_BUF_SIZE 0x100000
#define H264_BUF_SIZE 0x80000
#define AUDIO_BUF_SIZE 0x1000
#define MV_BUF_SIZE 0x40000
#define JPEG_BUF_SIZE 0x80000

#define PREVIEW_BUF_ORDER 8
#define H264_BUF_ORDER 7
#define AUDIO_BUF_ORDER 0
#define MV_BUF_ORDER 6
#define JPEG_BUF_ORDER 7

#define D1_PUSH_SIZE 0x100000
#define CIF_PUSH_SIZE (D1_PUSH_SIZE >> 2)
#define QCIF_PUSH_SIZE (CIF_PUSH_SIZE >> 2)

#define D1_CNT_PER_BUF 1
#define CIF_CNT_PER_BUF (D1_CNT_PER_BUF << 2)
#define QCIF_CNT_PER_BUF (CIF_CNT_PER_BUF << 2)

enum _push_buffer_type {
    BUF_TYPE_DEFAULT = 0,
    BUF_TYPE_H264,
    BUF_TYPE_AUDIO,
    BUF_TYPE_PREVIEW,
    BUF_TYPE_MV,
    BUF_TYPE_JPEG,
};

#define BUF_STATE_FREE 0
#define BUF_STATE_BUSY 1

#define MASK_LOW16 0xffff 

typedef struct _push_buffer {
    unsigned long buf_addr;
    int state;
} push_buffer_t;

typedef struct _push_prev_buffer {
    unsigned long buf_addr;
    u32 bit_map;
    int fmt;
} push_prev_buffer_t;

typedef struct _push_buf_pool {
    push_buffer_t h264_bufs[H264_BUF_CNT];
    u32 h264_free_cnt;
    spinlock_t h264_queue_lock;

    push_buffer_t audio_bufs[AUDIO_BUF_CNT];
    u32 audio_free_cnt;
    spinlock_t audio_queue_lock;

    push_prev_buffer_t preview_bufs[PREVIEW_BUF_CNT];
    u32 preview_free_cnt;
    spinlock_t preview_queue_lock;

    push_buffer_t mv_bufs[MV_BUF_CNT];
    u32 mv_free_cnt;
    spinlock_t mv_queue_lock;	

    push_buffer_t jpeg_bufs[JPEG_BUF_CNT];
    u32 jpeg_free_cnt;
    spinlock_t jpeg_queue_lock;		
} push_buf_pool_t;
/*
   typedef struct _push_buf_pool {
   void *buf;
   u32 use_count;
   } push_buf_pool_t;
   */
extern int push_buf_pool_init(isil_chip_t *chip);
extern void push_buf_pool_release(isil_chip_t *chip);

extern void *get_push_buf(int type, int fmt, isil_chip_t *chip);
extern void release_push_buf(int type, int fmt, void *p, isil_chip_t *chip);
