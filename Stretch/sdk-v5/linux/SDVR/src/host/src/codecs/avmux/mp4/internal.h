#ifndef _INTERNAL_H_
#define _INTERNAL_H_

#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "mp4mux.h"
#include "bytestrm.h"

// fragment mode implementation
#define FRAGMENT_MODE

// smff version
#define SMFF_VERSION        1

#define MP4_TIMESCALE    90000

// a/v sync
#define DTS_REF_CLOCK     100000
//#define DEBUG_AV_SYNC

// flushing
#if defined WIN32
#define DEFAULT_FLASH_PERIOD    (MP4_TIMESCALE * 2) // 2 seconds for Windows
#else
#define DEFAULT_FLASH_PERIOD    0
#endif

// log output disable mp4 log for linux because the log function hangs
#ifdef WIN32
#define LOG_OUTPUT
#endif

#define MP4_MAX_TRACKS              32
#define MP4_INDEX_CLUSTER_SIZE      16384
#define MP4_INDEX_CLUSTER_SIZE_FR   1024
#define MP4_FRAGMENT_SIZE_THR       (1024 * 1024 * 8)

#define MP4_STSD_SIZE_INC           16  // stsd size increment

#define MKTAG(a, b, c, d)   ((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))
#define MKBETAG(a, b, c, d) ((d) | ((c) << 8) | ((b) << 16) | ((a) << 24))
#define SX_ABS(a)           ((a) >= 0 ? (a) : (-a))

#define MP4_AV_TIME_BASE                1000000
#define INPUT_BUFFER_PADDING_SIZE       8

#define MP4ESDescrTag                   0x03
#define MP4DecConfigDescrTag            0x04
#define MP4DecSpecificDescrTag          0x05

// flag macros
#define CHK_FLAGS(flag, mask)           ((flag) & (mask))
#define SET_FLAGS(flag, bits)           ((flag) |= (bits))
#define CLR_FLAGS(flag, bits)           ((flag) &= (~(bits)))


typedef struct {
    sx_int32    id;         // codec id
    sx_uint32   tag;        // codec tag
} codec_tag_t;

typedef struct {
    sx_int64    offset;     // atom offset
    sx_int64    size;       // total size (excluding the size and type fields) 
    sx_uint32   type;       // atom type
} mp4_atom_t;

typedef struct {
    sx_int32 count;     // sample count
    sx_int32 duration;  // sample delta
} mp4_stts_t;

typedef struct {
    sx_int32 first;     // first chunk index
    sx_int32 count;     // samples per chunk
    sx_int32 index;     // sample description index
} mp4_stsc_t;

typedef struct {
    sx_int64 duration;      // segment duration in movie time scale
    sx_int64 media_time;    // media time in media time scale
    sx_int32 media_rate;    // media rate
} mp4_elst_t;

// sample flags
#define MP4_SAMPLE_KEYFRAME             0x00000001  // sample is key frame

typedef struct {
    sx_int64    pos;        // position in file
    sx_int64    dts;        // dts
    sx_int64    cts;        // cts
    sx_uint32   size;       // sample size
    sx_uint32   sd_index;   // sample description index
    sx_uint32   flags;      // flags
    union {
        sx_uint32   samples;    // samples in this entry
        sx_int32    min_dist;   // min distance between this and the previous key frame
    };
} sample_entry_t;

typedef struct {
    offset_t    offset;
    sx_int64    size;
} mp4_mdat_t;

// configuration data flags
#define CONFIG_FLAGS_VALID          0x00000001  // entry is valid
#define CONFIG_FLAGS_HAS_SPS        0x00000002  // entry has SPS
#define CONFIG_FLAGS_HAS_PPS        0x00000004  // entry has PPS
#define CONFIG_FLAGS_HAS_VOL        0x00000008  // entry has VOL
#define CONFIG_FLAGS_RESERVE        0x00000010  // entry is reserved

// configuration data
typedef struct {
    sx_uint8    *sps_data;      // sps data
    sx_uint8    *pps_data;      // pps data
    sx_uint16   sps_len;        // sps length
    sx_uint16   pps_len;        // pps length
    sx_uint32   flags;          // flags
} stsd_data_t;

// mp4_track_t flags
#define TRACK_FLAG_NEED_CONFIG          0x00000001  // need config data
#define TRACK_FLAG_NEW_CONFIG           0x00000002  // new config data
#define TRACK_FLAG_HAS_B_FRAME          0x00000004  // track has b frame
#define TRACK_FLAG_DISABLED             0x00000008  // track is disabled
#define TRACK_FLAG_HAVE_SAMPLE          0x00000010  // track has sample

// track fragment flags
#define TF_FLAGS_BASE_DATA_OFFSET           0x000001
#define TF_FLAGS_SAMPLE_DESC_INDEX          0x000002
#define TF_FLAGS_FIRST_SAMPLE_FLAGS         0x000004
#define TF_FLAGS_DEFAULT_SAMPLE_DURATION    0x000008
#define TF_FLAGS_DEFAULT_SAMPLE_SIZE        0x000010
#define TF_FLAGS_DEFAULT_SAMPLE_FLAGS       0x000020
#define TF_FLAGS_DTS_OFFSET                 0x000040    // Stretch defined
#define TF_FLAGS_SAMPLE_OFFSET              0x000080    // Stretch defined
#define TF_FLAGS_SAMPLE_DURATION            0x000100
#define TF_FLAGS_SAMPLE_SIZE                0x000200
#define TF_FLAGS_SAMPLE_FLAGS               0x000400
#define TF_FLAGS_SAMPLE_COMP_TIME_OFFSET    0x000800
#define TF_FLAGS_SAMPLE_CONFIG_DATA         0x001000    // Stretch defined
#define TF_FLAGS_SAMPLE_CAMERA_INFO         0x002000    // Stretch defined
#define TF_FLAGS_DURATION_IS_EMPTY          0x010000

// track data structure
typedef struct {
    mp4_track_info_t info;      // track info
    mp4_file_mode_t mode;       // file mode
    sx_uint32   tag;            // tag
    sx_uint32   flags;          // flags
    sx_int64    time;           // time
    sx_int32    wr_entries;     // number of sample entries
    sx_int32    sample_count;   // sample count of the whole track
    sx_int32    key_frames;     // number of key frames
    sx_uint32   frame_size;     // audio frame size
    sample_entry_t *samples;    // sample entries
    sample_entry_t *samples_fr; // sample entries of fragment
    sx_int32    fr_entries;     // number of sample entries of a fragment
    stsd_data_t *stsd_data;     // stsd data
    sx_uint32   stsd_count;     // allocated stsd count
    sx_uint32   stsd_idx_wr;    // write stsd index
    sx_uint32   stsd_idx_rd;    // read stsd index;
    // for parsing
    sx_uint32   stss_count;     // key frame count
    sx_uint32   stts_count;     // stts count
    sx_uint32   ctts_count;     // ctts count
    sx_uint32   stco_count;     // chunk count
    sx_uint32   stsc_count;     // stsc count
    sx_uint32   edit_count;     // edit count
    mp4_stts_t  *stts_data;     // stts data
    mp4_stts_t  *ctts_data;     // ctts data
    sx_int64    *stco_data;     // stco data
    mp4_stsc_t  *stsc_data;     // stsc data
    sx_int32    *stsz_data;     // stsz data
    sx_int32    *stss_data;     // stss data
    mp4_elst_t  *elst_data;     // elst data
    // for playback
    sample_entry_t *samples_rd;     // sample entries
    sx_int32    ctts_index;         // ctts entry index
    sx_int32    ctts_sample;        // ctts sample index within an entry
    sx_int32    bits_per_sample;    // bits per sample of audio
    sx_uint32   bytes_per_frame;    // bytes per frame of audio
    sx_uint32   samples_per_frame;  // samples per frame of audio
    sx_int32    current_sample;     // index of current sample
    sx_int32    rd_entries;         // number of index entries
    sx_uint32   alloc_entries;      // allocated entry count
    sx_uint16   sample_rate;        // audio sampling rate
    // trick mode
    sx_int32    curr_key;           // current key frame
    sx_int32    next_key;           // next key frame
    double      cur_dts;            // current playing dts
    // fragment mode
    sx_int64    base_data_offset;   // tfhd base_data_offset
    sx_uint32   default_desc_idx;   // default_description_index
    sx_uint32   default_flags;      // default_sample_flags
    sx_uint32   stsd_idx_rd1;       // stsd read index for tivo mode
    // av sync
	sx_int64	accu_cnt;		    // accumulated sample count
    sx_int64    prev_dts;           // previous dts
	sx_int64	init_dts;		    // initial time stamp
	sx_int64	curr_dts;		    // current dts
    sx_int64    dts_thr;            // dts threshold
	sx_int64	pts_offset;	        // initial pts offset
    sx_int64    elst_pos;           // elst position
    sx_int32    drop_cnt;           // drop count
} mp4_track_t;

// smff flags
#define SMFF_FLAG_CLOSED                0x000001  // file closed
#define SMFF_FLAG_CLOSING_MOOV          0x000002  // closing moov written

// mp4_context_t flags
// write flags
#define MP4_FLAG_MOOV_WRITTEN           0x00000001  // moov has been written
#define MP4_FLAG_MDAT_OPEN              0x00000002  // mdat is open
#define MP4_FLAG_REBUILD                0x00000004  // rebuild sample index
#define MP4_FLAG_APPEND_MOOV            0x00000008  // append closing moov
#define MP4_FLAG_INITIAL_MDAT           0x00000010  // initial mdat
// read flags
#define MP4_FLAG_FOUND_MOOV             0x00010000  // found moov
#define MP4_FLAG_FOUND_MDAT             0x00020000  // found mdat
#define MP4_FLAG_PARSE_SAMPLE_TABLE     0x00040000  // parse sample table
#define MP4_FLAG_TRICK_MODE             0x00080000  // trick mode
#define MP4_FLAG_FOUND_MVEX             0x00100000  // found mvex
#define MP4_FLAG_FRAGMENT_PLAY          0x00200000  // fragment play (tivo) mode
#define MP4_FLAG_WRITE_CLOSED           0x00400000  // write closed
#define MP4_FLAG_FORCE_PARSE_MOOF       0x00800000  // force to parse moof
#define MP4_FLAG_PARSE_MOOF             0x01000000  // parse moof
#define MP4_FLAG_REPAIR                 0x02000000  // repair box
#define MP4_FLAG_ISOM                   0x04000000  // iso mp4

// mp4 context
typedef struct {
    sx_int32        version;        // version
    mp4_mux_mode_t  mux_mode;       // mux mode
    mp4_file_mode_t mode;           // file mode
    char            *file_name;     // file name, only support ascii now.
    sx_int32        fd;             // file descriptor
    sx_uint32       bs_buf_size;    // byte stream buffer size
    byte_strm_t     *stream;        // byte stream pointer
    sx_int32        timescale;      // time scale
    sx_uint64       file_size;      // file size in bytes
    sx_int64        sys_time;       // system time since year 1970
    sx_int64        time;           // creation time since year 1904
    sx_int64        mod_time;       // modification time
    sx_int64        duration;       // duration of the longest track
    offset_t        mdat_pos;       // mdat position
    offset_t        smff_pos;       // smff position
    sx_uint64       mdat_size;      // mdat size
    sx_uint64       init_mdat_pos;  // init mdat pos
    sx_uint32       flags;          // flags
    sx_uint32       smff_flags;     // smff flags
    offset_t        sx_pos;         // sx position
    sx_uint32       seq_number;     // movie fragment sequence number
    sx_int32        nb_tracks;      // number of tracks
    mp4_track_t     *tracks[MP4_MAX_TRACKS];    // tracks
    // trick mode
    float           play_speed;     // play speed, negtive value means backward
    sx_uint32       seek_flags;     // seek flags
    mp4_track_t     *trick_track;   // trick play track
    // fragment mode playback
    mp4_atom_t      *moof_list;     // moof pos list
    sx_uint32       alloc_moof_cnt; // allocated moof count
    sx_int32        moof_cnt;       // parsed moof count
    sx_int32        next_moof_idx;  // next moof index to parse
    // av sync
	double			dts_factor;     // dts clock factor
    sx_int64        init_dts;       // movie initial time stamp
#ifdef DEBUG_AV_SYNC
	sx_int64		video_dts;		// video dts
#endif
    // flushing
    sx_int64        flush_period;   // flush period
    sx_int64        last_flush_ts;  // last flush timestamp
    // debug
    FILE            *log;           // log file
} mp4_context_t;

// parsing table
typedef struct {
    sx_uint32 type;
    sx_int32 (*parse)(mp4_context_t *ctx, mp4_atom_t atom);
} mp4_parse_table_t;


// function prototypes
sx_int32 mp4_create(mp4_context_t *ctx);
sx_int32 mp4_add_track(mp4_context_t *ctx, mp4_track_info_t *track_info);
sx_int32 mp4_put_packet(mp4_context_t *ctx, sx_uint32 track_no, mp4_packet_t *pkt);
sx_int32 mp4_write_open(mp4_context_t *ctx);
sx_int32 mp4_write_close(mp4_context_t *ctx);
sx_int32 mp4_authorize(mp4_context_t *ctx);
sx_int32 mp4_authorize_close(mp4_context_t *ctx);
sx_int32 mp4_parse_moov_sx(mp4_context_t *ctx, mp4_atom_t *moov);
sx_int32 mp4_find_moov_sx(byte_strm_t *bs, mp4_atom_t *atom, mp4_atom_t *moov);
sx_int32 mp4_parse_moof(mp4_context_t *ctx, mp4_atom_t *moof);

sx_int32 mp4_read_open(mp4_context_t *ctx);
sx_int32 mp4_read_info(mp4_context_t *ctx);
sx_int32 mp4_read_close(mp4_context_t *ctx);
sx_int32 mp4_read_seek(mp4_context_t *ctx, sx_int32 track_id, sx_int64 sample_time, sx_uint32 flags);
sx_int32 mp4_read_seek_trick_mode(mp4_context_t *ctx);

sx_uint32 codec_get_tag(const codec_tag_t *tags, sx_int32 id);
codec_id_t codec_get_id(const codec_tag_t *tags, sx_uint32 tag);

sx_int32 mp4_lang_to_iso639(sx_int32 code, sx_int8 *to);
sx_int32 mp4_get_packet(mp4_context_t *ctx, mp4_packet_t *pkt);

sx_int64 mp4_gcd(sx_int64 a, sx_int64 b);
sx_int64 rescale_rnd(sx_int64 a, sx_int64 b, sx_int64 c);
void mp4_freep(void **p);
void mp4_log(FILE *pf, const char *fmt, ...);
void* mp4_fast_realloc(void *ptr, sx_uint32 *alloc, sx_uint32 min, sx_uint32 unit_size);
void *mp4_fast_realloc1(void *ptr, sx_uint32 *alloc, sx_uint32 min, 
                        sx_uint32 unit_size, sx_uint32 inc);
void *mp4_fast_realloc2(void *ptr, sx_uint32 *alloc, sx_uint32 min, 
                        sx_uint32 unit_size, sx_uint32 inc);

sx_int32 mp4_av_sync(mp4_context_t *ctx, sx_uint32 track_no, mp4_packet_t *pkt);

#endif // #ifndef _INTERNAL_H_

