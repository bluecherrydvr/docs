// instrm.cpp

#include <windows.h>
#include "sdvr_player.h"
#include "instrm.h"

CInputStream::~CInputStream()
{
    if (m_nFd != -1) {
        sx_close(m_nFd);
        m_nFd = -1;
    }
}

CInputStream::CInputStream()
{
    m_nFd = -1;
}

sx_int32 CInputStream::Open(char* pchFileName)
{
    Close();
    m_nFd = sx_open(pchFileName, _O_BINARY | _O_RDONLY);
    if (m_nFd == -1)
        return 0;
    return 1;
}

void CInputStream::Close()
{
    if (m_nFd != -1) {
        sx_close(m_nFd);
        m_nFd = -1;
    }
}

sx_int64 CInputStream::GetFileLength(void)
{
    if (m_nFd == -1)
        return 0;
    return _filelengthi64(m_nFd);
}

sx_int32 CInputStream::Read(sx_uint8* pchBuf, sx_int32 nLength)
{
    if (m_nFd == -1)
        return 0;
    return sx_read(m_nFd, pchBuf, nLength);
}

offset_t CInputStream::Seek(offset_t offset, sx_int32 iWhence)
{
    if (m_nFd == -1)
        return 0;
    return sx_lseek64(m_nFd, offset, iWhence);
}
