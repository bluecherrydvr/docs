// instrm.h

#ifndef _INSTRM_H_
#define _INSTRM_H_

#include "common.h"

#ifdef WIN32

#include <io.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/stat.h>
#define ANSI

#include <stdarg.h>

#pragma warning(disable : 4996)

#define INT32_MAX   0x7fffffff
#define UINT32_MAX  0xffffffff
#define INT64_MAX  (__int64)0x7fffffffffffffff

#define O_CREAT     _O_CREAT
#define O_RDONLY    _O_RDONLY
#define O_WRONLY    _O_WRONLY
#define O_RDWR      _O_RDWR
#define O_BINAARY   _O_BINARY

#define sx_open         _open
#define sx_read         _read
#define sx_write        _write
#define sx_lseek        _lseek
#define sx_lseek64      _lseeki64
#define sx_close        _close
#define sx_stat         _stat
#define sx_flush        _commit
typedef struct _stat    fstat_t;

#else // #if WIN32

#error Only support WIN32 currently

#endif // #if WIN32


class CInputStream
{
public:
    ~CInputStream();
    CInputStream();

    sx_int32 Open(char* pchFileName);
    void Close();
	sx_int64 GetFileLength(void);
    sx_int32 Read(sx_uint8* pchBuf, sx_int32 nLength);
    offset_t Seek(offset_t offset, sx_int32 iWhence);

protected:
    sx_int32  m_nFd;
};

#endif // #ifndef _INSTRM_H_
