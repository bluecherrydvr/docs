#ifndef _BYTESTRM_H_
#define _BYTESTRM_H_

#include "common.h"

//#define USE_LOCAL_CACHE

#define MIN_BS_BUF_SIZE     (64 * 1024)
#define MAX_BS_BUF_SIZE     (8 * 1024 * 1024)

#define BS_FLAG_READ        0x00000001
#define BS_FLAG_WRITE       0x00000002
#define BS_FLAG_EOF         0x00000004

typedef struct {
    offset_t    wpos;           // write pos
    offset_t    rpos;           // read pos
    sx_int32    fd;             // file descriptor
    sx_int32    err;            // error code
    sx_int32    flags;          // flags
#ifdef USE_LOCAL_CACHE
    sx_int32    buf_size;       // buffer size
    sx_uint8    *wbuf;          // write buffer
    sx_uint8    *wptr;          // write pointer
    sx_uint8    *wend;          // write buffer end pointer
    sx_uint8    *rbuf;          // read buffer
    sx_uint8    *rptr;          // read pointer
    sx_uint8    *rend;          // read buffer end pointer
#endif
} byte_strm_t;


sx_int32 bs_init(byte_strm_t *bs, sx_int32 fd, sx_uint32 flags, sx_uint32 buf_size);
sx_int32 bs_close(byte_strm_t *bs);
sx_int32 bs_flush_buffer(byte_strm_t *bs);
sx_int32 bs_wflush(byte_strm_t *bs);

sx_int64 bs_wseek(byte_strm_t *bs, sx_int64 offset, int whence);
sx_int64 bs_rseek(byte_strm_t *bs, sx_int64 offset, int whence);
sx_int64 bs_wtell(byte_strm_t *bs);
sx_int64 bs_rtell(byte_strm_t *bs);
void bs_skip(byte_strm_t *bs, sx_int64 offset);
sx_int64 bs_get_size(byte_strm_t *bs); // get file size
sx_int32 bs_eof(byte_strm_t *bs);
sx_int32 bs_get_error(byte_strm_t *bs);

void bs_put_byte(byte_strm_t *bs, int b);
void bs_put_le16(byte_strm_t *bs, sx_uint32 val);
void bs_put_le24(byte_strm_t *bs, sx_uint32 val);
void bs_put_le32(byte_strm_t *bs, sx_uint32 val);
void bs_put_le64(byte_strm_t *bs, sx_uint64 val);
void bs_put_be16(byte_strm_t *bs, sx_uint32 val);
void bs_put_be24(byte_strm_t *bs, sx_uint32 val);
void bs_put_be32(byte_strm_t *bs, sx_uint32 val);
void bs_put_be64(byte_strm_t *bs, sx_uint64 val);
void bs_put_strz(byte_strm_t *bs, const sx_uint8 *str);
void bs_put_tag(byte_strm_t *bs, const sx_uint8 *tag);
void bs_put_buffer(byte_strm_t *bs, const sx_uint8 *buf, sx_int32 size);

sx_uint32 bs_get_byte(byte_strm_t *bs);
sx_uint32 bs_get_le16(byte_strm_t *bs);
sx_uint32 bs_get_le24(byte_strm_t *bs);
sx_uint32 bs_get_le32(byte_strm_t *bs);
sx_uint64 bs_get_le64(byte_strm_t *bs);
sx_uint32 bs_get_be16(byte_strm_t *bs);
sx_uint32 bs_get_be24(byte_strm_t *bs);
sx_uint32 bs_get_be32(byte_strm_t *bs);
sx_uint64 bs_get_be64(byte_strm_t *bs);
sx_int32 bs_get_buffer(byte_strm_t *bs, sx_uint8 *buf, sx_int32 size);

#endif // #ifndef _BYTESTRM_H_

