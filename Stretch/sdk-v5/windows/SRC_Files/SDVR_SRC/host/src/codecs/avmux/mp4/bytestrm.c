/* bytestrm.c */
#include <assert.h>
#include "internal.h"

/*
 * Initialize a byte stream
 *
 * Parameters:
 *   bs(in): pointer of a byte stream
 *   fd(in): file descriptor
 *   mode(in): access mode
 * Return:
 *   0 = success, otherwise fail
 */
sx_int32 bs_init(byte_strm_t *bs, sx_int32 fd, sx_uint32 flags, sx_uint32 buf_size)
{
    // error check by upper layer
    assert(bs != NULL);
    assert(fd != -1);

    memset(bs, 0, sizeof(byte_strm_t));
    bs->fd = fd;
    SET_FLAGS(bs->flags, flags);

#ifdef USE_LOCAL_CACHE
    // check buffer size
    if (buf_size < MIN_BS_BUF_SIZE)
        buf_size = MIN_BS_BUF_SIZE;
    if (buf_size > MAX_BS_BUF_SIZE)
        buf_size = MAX_BS_BUF_SIZE;

    // allocate write buffer
    if (CHK_FLAGS(bs->flags, BS_FLAG_WRITE)) {
        bs->wbuf = malloc(buf_size);
        if (bs->wbuf == NULL)
            return MP4MUX_ERR_MEMORY;
    }
    // allocate read buffer
    if (CHK_FLAGS(bs->flags, BS_FLAG_READ)) {
        bs->rbuf = malloc(buf_size);
        if (bs->rbuf == NULL)
            return MP4MUX_ERR_MEMORY;
    }
    bs->buf_size = buf_size;
    bs->wptr = bs->wbuf; // empty write buffer
    bs->wend = bs->wbuf + buf_size - 1;
    bs->rend = bs->rbuf + buf_size - 1;
    bs->rptr = bs->rend; // empty read buffer
#endif

    bs->wpos = bs->rpos = 0;
    return MP4MUX_ERR_OK;
}

/*
 * Close a byte stream
 *
 * Parameters:
 *   bs(in): pointer of a byte stream
 * Return:
 *   0 = success, otherwise fail
 */
sx_int32 bs_close(byte_strm_t *bs)
{
    bs_flush_buffer(bs);
#ifdef USE_LOCAL_CACHE
    mp4_freep((void *)&bs->wbuf);
    mp4_freep((void *)&bs->rbuf);
#endif
    return MP4MUX_ERR_OK;
}

/*
 * Flush buffer
 *
 * Parameters:
 *   bs(in): pointer of a byte stream
 * Return:
 *   Number of bytes written
 */
sx_int32 bs_flush_buffer(byte_strm_t *bs)
{
#ifdef USE_LOCAL_CACHE
    if (bs->wptr > bs->wbuf) {
        sx_int32 count = (sx_int32)(bs->wptr - bs->wbuf);
#ifdef STRETCH_TARGET
        bs->err = FS_Write((FS_FILE*)bs->fd, (void *)bs->wbuf, count);
#else
        bs->err = sx_write(bs->fd, bs->wbuf, count);
#endif
        bs->wpos += count;
    }
    bs->wptr = bs->wbuf;
#endif // #ifdef USE_LOCAL_CACHE

    return bs->err;
}

sx_int32 bs_wflush(byte_strm_t *bs)
{
    sx_flush(bs->fd);
    return bs->err;
}

/*
 * Fill buffer
 *
 * Parameters:
 *   bs(in): pointer of a byte stream
 * Return:
 *   None
 */
static void bs_fill_buffer(byte_strm_t *bs)
{
#ifdef USE_LOCAL_CACHE
    int len = 0;

    // do nothing if eof
    if (CHK_FLAGS(bs->flags, BS_FLAG_EOF))
        return;

#ifdef STRETCH_TARGET
    len = FS_Read((FS_FILE*)bs->fd, (void *)bs->rbuf, bs->buf_size);
#else
    len = sx_read(bs->fd, bs->rbuf, bs->buf_size);
#endif
    if (len <= 0) {
        SET_FLAGS(bs->flags, BS_FLAG_EOF);
        if (len < 0)
            bs->err = len;
    } 
    else {
        bs->rpos += len;
        bs->rptr = bs->rbuf;
        bs->rend = bs->rbuf + len;
    }
#endif // #ifdef USE_LOCAL_CACHE
}

/*
 * Byte stream seek
 *
 * Parameters:
 *   bs(in): pointer of a byte stream
 *   offset(in): offset
 *   whence(in): whence
 * Return:
 *   Offset of the byte stream
 */
sx_int64 bs_wseek(byte_strm_t *bs, sx_int64 offset, int whence)
{
#ifdef USE_LOCAL_CACHE
    sx_int64 offset1;
#endif
    sx_int64 res;

    if (whence != SEEK_CUR && whence != SEEK_SET)
        return -1;

#ifdef USE_LOCAL_CACHE
    if (whence == SEEK_CUR) {
        offset1 = bs->wpos + (bs->wptr - bs->wbuf); // current write pos
        if (offset == 0)
            return offset1;
        offset += offset1;
    }
    
    // we need to flush whenever we do wseek
    bs_flush_buffer(bs);
    bs->wptr = bs->wbuf; // empty write buffer

#ifdef STRETCH_TARGET
    if ((res = FS_FSeek((FS_FILE *)bs->fd, offset, FS_SEEK_SET)) < 0)
#else
    if ((res = sx_lseek64(bs->fd, offset, SEEK_SET)) < 0)
#endif
        return res;

    bs->wpos = offset;
    CLR_FLAGS(bs->flags, BS_FLAG_EOF);
    return offset;
#else // #ifdef USE_LOCAL_CACHE
#ifdef STRETCH_TARGET
    res = FS_FSeek((FS_FILE *)bs->fd, offset, whence);
#else
    res = sx_lseek64(bs->fd, offset, whence);
#endif
    CLR_FLAGS(bs->flags, BS_FLAG_EOF);
    return res;
#endif //#ifdef USE_LOCAL_CACHE
}

sx_int64 bs_rseek(byte_strm_t *bs, sx_int64 offset, int whence)
{
#ifdef USE_LOCAL_CACHE
    sx_int64 pos, offset1;
#endif
    sx_int64 res;

    if (whence != SEEK_CUR && whence != SEEK_SET)
        return -1;

#ifdef USE_LOCAL_CACHE
    pos = bs->rpos - (bs->rend - bs->rbuf);
    if (whence == SEEK_CUR) {
        offset1 = pos + (bs->rptr - bs->rbuf); // current pos
        if (offset == 0)
            return offset1;
        offset += offset1;
    }
    offset1 = offset - pos;
    if (offset1 >= 0 && offset1 < (bs->rend - bs->rbuf)) {
        /* can do the seek inside the buffer */
        bs->rptr = bs->rbuf + offset1;
    } 
    else {
        bs->rend = bs->rbuf; // empty buffer for read
        bs->rptr = bs->rbuf;
#ifdef STRETCH_TARGET
        if ((res = FS_FSeek((FS_FILE *)bs->fd, offset, FS_SEEK_SET)) < 0)
#else
        if ((res = sx_lseek64(bs->fd, offset, SEEK_SET)) < 0)
#endif
            return res;
        bs->rpos = offset;
    }
    CLR_FLAGS(bs->flags, BS_FLAG_EOF);
    return offset;
#else // #ifdef USE_LOCAL_CACHE
#ifdef STRETCH_TARGET
    res = FS_FSeek((FS_FILE *)bs->fd, offset, whence);
#else
    res = sx_lseek64(bs->fd, offset, whence);
#endif
    CLR_FLAGS(bs->flags, BS_FLAG_EOF);
    return res;
#endif // #ifdef USE_LOCAL_CACHE
}

/*
 * Skip bytes of a byte stream
 *
 * Parameters:
 *   bs(in): pointer of a byte stream
 *   offset(in): offset
 * Return:
 *   None
 */
void bs_skip(byte_strm_t *bs, sx_int64 offset)
{
    bs_rseek(bs, offset, SEEK_CUR);
}

/*
 * Tell current position of a byte stream
 *
 * Parameters:
 *   bs(in): pointer of a byte stream
 * Return:
 *   Position of the byte stream
 */
sx_int64 bs_wtell(byte_strm_t *bs)
{
    return bs_wseek(bs, 0, SEEK_CUR);
}

sx_int64 bs_rtell(byte_strm_t *bs)
{
    return bs_rseek(bs, 0, SEEK_CUR);
}

/*
 * Get file size of a byte stream
 *
 * Parameters:
 *   bs(in): pointer of a byte stream
 * Return:
 *   File size of the byte stream
 */
sx_int64 bs_get_size(byte_strm_t *bs)
{
    sx_int64 size, pos;
#ifdef WIN32
    pos = _filelengthi64(bs->fd);
    return pos;
#elif STRETCH_TARGET
    pos = FS_FTell ((FS_FILE *)bs->fd);
    FS_FSeek((FS_FILE *)bs->fd, 0, FS_SEEK_END);
    size = FS_FTell ((FS_FILE *)bs->fd);
    FS_FSeek((FS_FILE *)bs->fd, pos, FS_SEEK_SET);
#else
    pos = sx_lseek64(bs->fd, 0, SEEK_CUR);
    size = sx_lseek64(bs->fd, 0, SEEK_END);
    sx_lseek64(bs->fd, pos, SEEK_SET);
#endif
    return size;
}

/*
 * Get EOF flag
 *
 * Parameters:
 *   bs(in): pointer of a byte stream
 * Return:
 *   EOF flag
 */
int bs_eof(byte_strm_t *bs)
{
    return CHK_FLAGS(bs->flags, BS_FLAG_EOF) ? 1 : 0;
}

/*
 * Get error code
 *
 * Parameters:
 *   bs(in): pointer of a byte stream
 * Return:
 *   Error code
 */
int bs_get_error(byte_strm_t *bs)
{
    return bs->err;
}

/*
 * Put one byte into byte stream
 *
 * Parameters:
 *   bs(in): pointer of a byte stream
 *   b(in): byte value
 * Return:
 *   None
 */
void bs_put_byte(byte_strm_t *bs, int b)
{
#ifdef USE_LOCAL_CACHE
    *(bs->wptr)++ = b;
    if (bs->wptr >= bs->wend)
        bs_flush_buffer(bs);
#else
	sx_write(bs->fd, &b, 1);
#endif
}

/*
 * Put buffer of bytes into byte stream
 *
 * Parameters:
 *   bs(in): pointer of a byte stream
 *   buf(in): byte buffer
 *   size(in): number of bytes
 * Return:
 *   None
 */
void bs_put_buffer(byte_strm_t *bs, const sx_uint8 *buf, sx_int32 size)
{
#ifdef USE_LOCAL_CACHE
    sx_int32 len;
    while (size > 0) {
        len = (sx_int32)(bs->wend - bs->wptr);
        if (len > size)
            len = size;
        memcpy(bs->wptr, buf, len);
        bs->wptr += len;

        if (bs->wptr >= bs->wend)
            bs_flush_buffer(bs);

        buf += len;
        size -= len;
    }
#else
	sx_write(bs->fd, buf, size);
#endif
}

/*
 * Put 16-bit little-endian value into byte stream
 *
 * Parameters:
 *   bs(in): pointer of a byte stream
 *   val(in): value
 * Return:
 *   None
 */
void bs_put_le16(byte_strm_t *bs, sx_uint32 val)
{
#ifdef USE_LOCAL_CACHE
    bs_put_byte(bs, val);
    bs_put_byte(bs, val >> 8);
#else
	sx_write(bs->fd, &val, 2);
#endif
}

/*
 * Put 24-bit little-endian value into byte stream
 *
 * Parameters:
 *   bs(in): pointer of a byte stream
 *   val(in): value
 * Return:
 *   None
 */
void bs_put_le24(byte_strm_t *bs, sx_uint32 val)
{
#ifdef USE_LOCAL_CACHE
    bs_put_le16(bs, val & 0xffff);
    bs_put_byte(bs, val >> 16);
#else
	sx_write(bs->fd, &val, 3);
#endif
}

/*
 * Put 32-bit little-endian value into byte stream
 *
 * Parameters:
 *   bs(in): pointer of a byte stream
 *   val(in): value
 * Return:
 *   None
 */
void bs_put_le32(byte_strm_t *bs, sx_uint32 val)
{
#ifdef USE_LOCAL_CACHE
    bs_put_byte(bs, val);
    bs_put_byte(bs, val >> 8);
    bs_put_byte(bs, val >> 16);
    bs_put_byte(bs, val >> 24);
#else
	sx_write(bs->fd, &val, 4);
#endif
}

/*
 * Put 64-bit little-endian value into byte stream
 *
 * Parameters:
 *   bs(in): pointer of a byte stream
 *   val(in): value
 * Return:
 *   None
 */
void bs_put_le64(byte_strm_t *bs, sx_uint64 val)
{
#ifdef USE_LOCAL_CACHE
    bs_put_le32(bs, (sx_uint32)(val & 0xffffffff));
    bs_put_le32(bs, (sx_uint32)(val >> 32));
#else
	sx_write(bs->fd, &val, 8);
#endif
}

/*
 * Put 16-bit little-endian value into byte stream
 *
 * Parameters:
 *   bs(in): pointer of a byte stream
 *   val(in): value
 * Return:
 *   None
 */
void bs_put_be16(byte_strm_t *bs, sx_uint32 val)
{
    bs_put_byte(bs, val >> 8);
    bs_put_byte(bs, val);
}

/*
 * Put 24-bit big-endian value into byte stream
 *
 * Parameters:
 *   bs(in): pointer of a byte stream
 *   val(in): value
 * Return:
 *   None
 */
void bs_put_be24(byte_strm_t *bs, sx_uint32 val)
{
    bs_put_be16(bs, val >> 8);
    bs_put_byte(bs, val);
}

/*
 * Put 32-bit big-endian value into byte stream
 *
 * Parameters:
 *   bs(in): pointer of a byte stream
 *   val(in): value
 * Return:
 *   None
 */
void bs_put_be32(byte_strm_t *bs, sx_uint32 val)
{
    bs_put_byte(bs, val >> 24);
    bs_put_byte(bs, val >> 16);
    bs_put_byte(bs, val >> 8);
    bs_put_byte(bs, val);
}

/*
 * Put 64-bit big-endian value into byte stream
 *
 * Parameters:
 *   bs(in): pointer of a byte stream
 *   val(in): value
 * Return:
 *   None
 */
void bs_put_be64(byte_strm_t *bs, sx_uint64 val)
{
    bs_put_be32(bs, (sx_uint32)(val >> 32));
    bs_put_be32(bs, (sx_uint32)(val & 0xffffffff));
}

/*
 * Put zero-end string into byte stream
 *
 * Parameters:
 *   bs(in): pointer of a byte stream
 *   str(in): pointer of zero-end string
 * Return:
 *   None
 */
void bs_put_strz(byte_strm_t *bs, const sx_uint8 *str)
{
    if (str)
        bs_put_buffer(bs, (const sx_uint8 *) str, (sx_int32)strlen((char*)str) + 1);
    else
        bs_put_byte(bs, 0);
}

/*
 * Put zero-end tag into byte stream
 *
 * Parameters:
 *   bs(in): pointer of a byte stream
 *   tat(in): zero-end tag
 * Return:
 *   None
 */
void bs_put_tag(byte_strm_t *bs, const sx_uint8 *tag)
{
    while (*tag) {
        bs_put_byte(bs, *tag++);
    }
}

/*
 * Get one byte from byte stream
 *
 * Parameters:
 *   bs(in): pointer of a byte stream
 * Return:
 *   Byte value
 */
sx_uint32 bs_get_byte(byte_strm_t *bs)
{
#ifdef USE_LOCAL_CACHE
    if (bs->rptr < bs->rend) {
        return *bs->rptr++;
    } 
    else {
        bs_fill_buffer(bs);
        if (bs->rptr < bs->rend)
            return *bs->rptr++;
        else
            return 0;
    }
    return 0;               // unreachable - prevent compiler warning
#else
	sx_uint8 ret;
	sx_read(bs->fd, &ret, 1);
    return (sx_uint32)ret;
#endif
}

/*
 * Get 16-bit little endian from byte stream
 *
 * Parameters:
 *   bs(in): pointer of a byte stream
 * Return:
 *   16-bit value
 */
sx_uint32 bs_get_le16(byte_strm_t *bs)
{
    sx_uint32 val;
    val = bs_get_byte(bs);
    val |= bs_get_byte(bs) << 8;
    return val;
}

/*
 * Get 24-bit little endian from byte stream
 *
 * Parameters:
 *   bs(in): pointer of a byte stream
 * Return:
 *   24-bit value
 */
sx_uint32 bs_get_le24(byte_strm_t *bs)
{
    sx_uint32 val;
    val = bs_get_le16(bs);
    val |= bs_get_byte(bs) << 16;
    return val;
}

/*
 * Get 32-bit little endian from byte stream
 *
 * Parameters:
 *   bs(in): pointer of a byte stream
 * Return:
 *   32-bit value
 */
sx_uint32 bs_get_le32(byte_strm_t *bs)
{
    sx_uint32 val;
    val = bs_get_le16(bs);
    val |= bs_get_le16(bs) << 16;
    return val;
}

/*
 * Get 64-bit little endian from byte stream
 *
 * Parameters:
 *   bs(in): pointer of a byte stream
 * Return:
 *   64-bit value
 */
sx_uint64 bs_get_le64(byte_strm_t *bs)
{
    sx_uint64 val;
    val = (sx_uint64)bs_get_le32(bs);
    val |= (sx_uint64)bs_get_le32(bs) << 32;
    return val;
}

/*
 * Get 16-bit big endian from byte stream
 *
 * Parameters:
 *   bs(in): pointer of a byte stream
 * Return:
 *   16-bit value
 */
sx_uint32 bs_get_be16(byte_strm_t *bs)
{
    sx_uint32 val;
    val = bs_get_byte(bs) << 8;
    val |= bs_get_byte(bs);
    return val;
}

/*
 * Get 24-bit big endian from byte stream
 *
 * Parameters:
 *   bs(in): pointer of a byte stream
 * Return:
 *   24-bit value
 */
sx_uint32 bs_get_be24(byte_strm_t *bs)
{
    sx_uint32 val;
    val = bs_get_be16(bs) << 8;
    val |= bs_get_byte(bs);
    return val;
}

/*
 * Get 32-bit big endian from byte stream
 *
 * Parameters:
 *   bs(in): pointer of a byte stream
 * Return:
 *   32-bit value
 */
sx_uint32 bs_get_be32(byte_strm_t *bs)
{
    sx_uint32 val;
    val = bs_get_be16(bs) << 16;
    val |= bs_get_be16(bs);
    return val;
}

sx_uint64 bs_get_be64(byte_strm_t *bs)
{
    sx_uint64 val;
    val = (sx_uint64)bs_get_be32(bs) << 32;
    val |= (sx_uint64)bs_get_be32(bs);
    return val;
}

/*
 * Get buffer of bytes from byte stream
 *
 * Parameters:
 *   bs(in): pointer of a byte stream
 *   buf(in/out): pointer of buffer
 *   size(in): number of bytes to read
 * Return:
 *   Number of bytes read
 */
sx_int32 bs_get_buffer(byte_strm_t *bs, sx_uint8 *buf, sx_int32 size)
{
#ifdef USE_LOCAL_CACHE
    sx_int32 len, size1 = size;
    while (size > 0) {
        len = (sx_int32)(bs->rend - bs->rptr);
        if (len > size)
            len = size;
        if (len == 0) {
            if (size > bs->buf_size) {
#ifdef STRETCH_TARGET
                len = FS_Read((FS_FILE *)bs->fd, (void *)buf, size);
#else
                len = sx_read(bs->fd, buf, size);
#endif
                if (len <= 0) {
                    SET_FLAGS(bs->flags, BS_FLAG_EOF);
                    if (len < 0)
                        bs->err = len;
                    break;
                } 
                else {
                    bs->rpos += len;
                    size -= len;
                    buf += len;
                    bs->rptr = bs->rbuf;
                    bs->rend = bs->rbuf/* + len*/;
                }
            }
            else {
                bs_fill_buffer(bs);
                len = (sx_int32)(bs->rend - bs->rptr);
                if (len == 0)
                    break;
            }
        } 
        else {
            memcpy(buf, bs->rptr, len);
            buf += len;
            bs->rptr += len;
            size -= len;
        }
    }
    return size1 - size;
#else // #ifdef USE_LOCAL_CACHE
	sx_int32 len;
#ifdef STRETCH_TARGET
    len = FS_Read((FS_FILE *)bs->fd, (void *)buf, size);
#else
    len = sx_read(bs->fd, buf, size);
#endif
	return len;
#endif // #ifdef USE_LOCAL_CACHE
}
