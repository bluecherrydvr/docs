// stdafx.cpp : source file that includes just the standard includes
//	SOLO6010App.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

static void memcpy_MMX (BYTE *pDest, BYTE *pSrc, int szCopy);
static void memcpy_SSE (BYTE *pDest, BYTE *pSrc, int szCopy);
static void memcpy_Norm (BYTE *pDest, BYTE *pSrc, int szCopy);
void (*FastMemcpy) (BYTE *pDest, BYTE *pSrc, int szCopy);

#define RANGE(x, x_min, x_max)		((x)>(x_max) ? (x_max) : ((x)<(x_min) ? (x_min) : x))

BOOL MiscBrowseForFolder(CString *pStrFolder, CString strTitle, HWND hWndOwner)
{
	BROWSEINFO bi;
	char szBuffer[_MAX_PATH];
	LPITEMIDLIST pList;

	memset(szBuffer, 0, sizeof(szBuffer));

	memset(&bi, 0, sizeof(bi));
	bi.hwndOwner = hWndOwner;
	bi.pszDisplayName = szBuffer;
	bi.lpszTitle = strTitle;
	bi.ulFlags = BIF_RETURNONLYFSDIRS;

	pList = SHBrowseForFolder(&bi);
	if ( pList )
	{
		BOOL bOk;
		HRESULT rval;
		LPMALLOC pMalloc;

		bOk = SHGetPathFromIDList(pList, szBuffer);

		rval = SHGetMalloc(&pMalloc);
		ASSERT( rval == NOERROR );
		pMalloc->Free(pList);

		if ( bOk )
		{
			*pStrFolder = szBuffer;
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else
		return FALSE;
}

void SetFastMemcpy ()
{
	int flags = 0;

	__asm
	{
		xor eax, eax;
		cpuid;
		or eax, eax;
		jz quit;
		mov eax, 1;
		cpuid;
		mov flags, edx;
		quit:
	}

	if (flags &(1 <<26))	// SSE2
	{
		FastMemcpy = memcpy_SSE;
	}
	else if (flags &(1 <<25))	// SSE
	{
		FastMemcpy = memcpy_SSE;
	}
	else if (flags &(1 <<23))	// MMX
	{
		FastMemcpy = memcpy_MMX;
	}
	else if (flags &(1 <<30))	// E3DNOW
	{
		FastMemcpy = memcpy_MMX;
	}
	else if (flags &(1 <<31))	// 3DNOW
	{
		FastMemcpy = memcpy_MMX;
	}
	else		// Is it a modern CPU? Are you using 486? --;;
	{
		FastMemcpy = memcpy_Norm;
	}
}

static void memcpy_Norm (BYTE *pDest, BYTE *pSrc, int szCopy)
{
	memcpy (pDest, pSrc, szCopy);
}

static void memcpy_MMX (BYTE *pDest, BYTE *pSrc, int szCopy)
{
    __asm
	{
        mov     esi, dword ptr[pSrc]
        mov     edi, dword ptr[pDest]
        mov     ecx, szCopy
        shr     ecx, 6
align 8
CopyLoop:
        movq    mm0, qword ptr[esi]
        movq    mm1, qword ptr[esi+8*1]
        movq    mm2, qword ptr[esi+8*2]
        movq    mm3, qword ptr[esi+8*3]
        movq    mm4, qword ptr[esi+8*4]
        movq    mm5, qword ptr[esi+8*5]
        movq    mm6, qword ptr[esi+8*6]
        movq    mm7, qword ptr[esi+8*7]
        movq    qword ptr[edi], mm0
        movq    qword ptr[edi+8*1], mm1
        movq    qword ptr[edi+8*2], mm2
        movq    qword ptr[edi+8*3], mm3
        movq    qword ptr[edi+8*4], mm4
        movq    qword ptr[edi+8*5], mm5
        movq    qword ptr[edi+8*6], mm6
        movq    qword ptr[edi+8*7], mm7
        add     esi, 64
        add     edi, 64
        loop CopyLoop

		emms
    }
}

static void memcpy_SSE (BYTE *pDest, BYTE *pSrc, int szCopy)
{
	if ((DWORD)pSrc %16 == 0)
	{
		__asm
		{
			mov     esi, dword ptr[pSrc]
			mov     edi, dword ptr[pDest]
			mov     ecx, szCopy
			shr     ecx, 7
	align 16
	CopyLoopAlign:
			movdqa xmm0, [esi +16 *0]
			movdqa xmm1, [esi +16 *1]
			movdqa xmm2, [esi +16 *2]
			movdqa xmm3, [esi +16 *3]
			movdqa xmm4, [esi +16 *4]
			movdqa xmm5, [esi +16 *5]
			movdqa xmm6, [esi +16 *6]
			movdqa xmm7, [esi +16 *7]

			movntdq XMMWORD PTR[edi +16 *0], xmm0
			movntdq XMMWORD PTR[edi +16 *1], xmm1
			movntdq XMMWORD PTR[edi +16 *2], xmm2
			movntdq XMMWORD PTR[edi +16 *3], xmm3
			movntdq XMMWORD PTR[edi +16 *4], xmm4
			movntdq XMMWORD PTR[edi +16 *5], xmm5
			movntdq XMMWORD PTR[edi +16 *6], xmm6
			movntdq XMMWORD PTR[edi +16 *7], xmm7

			add     esi, 128
			add     edi, 128
			loop CopyLoopAlign
			
			emms
		}
	}
	else
	{
		__asm
		{
			mov     esi, dword ptr[pSrc]
			mov     edi, dword ptr[pDest]
			mov     ecx, szCopy
			shr     ecx, 7
	align 16
	CopyLoopUnalign:
			movdqu xmm0, [esi +16 *0]
			movdqu xmm1, [esi +16 *1]
			movdqu xmm2, [esi +16 *2]
			movdqu xmm3, [esi +16 *3]
			movdqu xmm4, [esi +16 *4]
			movdqu xmm5, [esi +16 *5]
			movdqu xmm6, [esi +16 *6]
			movdqu xmm7, [esi +16 *7]

			movntdq XMMWORD PTR[edi +16 *0], xmm0
			movntdq XMMWORD PTR[edi +16 *1], xmm1
			movntdq XMMWORD PTR[edi +16 *2], xmm2
			movntdq XMMWORD PTR[edi +16 *3], xmm3
			movntdq XMMWORD PTR[edi +16 *4], xmm4
			movntdq XMMWORD PTR[edi +16 *5], xmm5
			movntdq XMMWORD PTR[edi +16 *6], xmm6
			movntdq XMMWORD PTR[edi +16 *7], xmm7

			add     esi, 128
			add     edi, 128
			loop CopyLoopUnalign
			
			emms
		}
	}

/*	__asm
	{
		mov     esi, dword ptr[pSrc]
		mov     edi, dword ptr[pDest]
		mov     ecx, szCopy
		shr     ecx, 7
align 16
CopyLoop:
		movdqu xmm0, [esi +16 *0]
		movdqu xmm1, [esi +16 *1]
		movdqu xmm2, [esi +16 *2]
		movdqu xmm3, [esi +16 *3]
		movdqu xmm4, [esi +16 *4]
		movdqu xmm5, [esi +16 *5]
		movdqu xmm6, [esi +16 *6]
		movdqu xmm7, [esi +16 *7]

		movntdq XMMWORD PTR[edi +16 *0], xmm0
		movntdq XMMWORD PTR[edi +16 *1], xmm1
		movntdq XMMWORD PTR[edi +16 *2], xmm2
		movntdq XMMWORD PTR[edi +16 *3], xmm3
		movntdq XMMWORD PTR[edi +16 *4], xmm4
		movntdq XMMWORD PTR[edi +16 *5], xmm5
		movntdq XMMWORD PTR[edi +16 *6], xmm6
		movntdq XMMWORD PTR[edi +16 *7], xmm7

		add     esi, 128
		add     edi, 128
		loop CopyLoop
		mov     ecx, szCopy
		and     ecx, 127
		cmp     ecx, 0
		je EndCopyLoop
align 16
CopyLoop2:
		mov dl, byte ptr[esi]
		mov byte ptr[edi], dl
		inc esi
		inc edi
		dec ecx
		jne near CopyLoop2
EndCopyLoop:
		emms
	}
*/
}

DWORD GetHexValueFromHexString (LPCTSTR strHex)
{
	DWORD i, lenStr, nRetVal = 0;

	lenStr = strlen (strHex);
	if (lenStr > 8)	// for DWORD
	{
		lenStr = 8;
	}
	for (i=0; i<lenStr; i++)
	{
		nRetVal <<= 4;
		if (strHex[i] >= '0' && strHex[i] <= '9')
		{
			nRetVal += strHex[i] -'0';
		}
		else if (strHex[i] >= 'A' && strHex[i] <= 'F')
		{
			nRetVal += strHex[i] -'A' +10;
		}
		else if (strHex[i] >= 'a' && strHex[i] <= 'f')
		{
			nRetVal += strHex[i] -'a' +10;
		}
	}

	return nRetVal;
}
