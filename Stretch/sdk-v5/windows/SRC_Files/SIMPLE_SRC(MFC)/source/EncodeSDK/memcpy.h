#ifndef __MEMCPY_H__
#define __MEMCPY_H__

void _cdecl memcpyMMX(void *Dest, void *Src, size_t nBytes);
void _cdecl memcpySSE(void *Dest, void *Src, size_t nBytes);

#endif
