#include "stdafx.h"
#include "Cpu.h"
#include "LibDeIntFilter.h"

static __int64 qwEdgeDetect;
static __int64 qwThreshold;
static const __int64 Mask = 0xfefefefefefefefe;
static const DWORD dwZero = 0;
static const __int64 qwZero = 0;
static __m128i m128EdgeDetect;
static __m128i m128JaggieThreshold;
static __m128i m128_YMask;


void DeinterlaceFieldBob_I420_MMX(DWORD LineLength, DWORD FieldHeight, unsigned char *bufImg);
void DeinterlaceFieldBob_I420_SSE(DWORD LineLength, DWORD FieldHeight, unsigned char *bufImg);

void DeinterlaceFieldBob_YUV2_MMX(DWORD LineLength, DWORD FieldHeight, unsigned char *bufImg);
void DeinterlaceFieldBob_YUV2_SSE(DWORD LineLength, DWORD FieldHeight, unsigned char *bufImg);


void (*DeIntAsmFunc) (DWORD LineLength, DWORD FieldHeight, unsigned char *bufImg);

int __stdcall InitDeIntFilter (int idxTypeCS, int nEdgeDetect, int nJaggieThreshold)
{
	UINT CpuFeatureFlags = CPU_SetupFeatureFlag ();

	if ((CpuFeatureFlags &FEATURE_SSE2) == FEATURE_SSE2)
	{
		switch (idxTypeCS)
		{
		case IDX_DEFLT_CS_I420:
		case IDX_DEFLT_CS_YV12:
			DeIntAsmFunc = DeinterlaceFieldBob_I420_SSE;
			break;
		case IDX_DEFLT_CS_YUY2:
			DeIntAsmFunc = DeinterlaceFieldBob_YUV2_SSE;
			break;
		default:
			return IDX_DEFLT_RET_ERR_NOT_SUPPORTED_CS;
		}
	}
	else
	{
		switch (idxTypeCS)
		{
		case IDX_DEFLT_CS_I420:
		case IDX_DEFLT_CS_YV12:
			DeIntAsmFunc = DeinterlaceFieldBob_I420_MMX;
			break;
		case IDX_DEFLT_CS_YUY2:
			DeIntAsmFunc = DeinterlaceFieldBob_YUV2_MMX;
			break;
		default:
			return IDX_DEFLT_RET_ERR_NOT_SUPPORTED_CS;
		}
	}

	qwEdgeDetect = nEdgeDetect;
	qwEdgeDetect += (qwEdgeDetect << 48) + (qwEdgeDetect << 32) + (qwEdgeDetect << 16);
	qwThreshold = nJaggieThreshold;
	qwThreshold += (qwThreshold << 48) + (qwThreshold << 32) + (qwThreshold << 16);

	int i;
	for (i=0; i<8; i++)
	{
		m128EdgeDetect.m128i_i16[i] = nEdgeDetect;
		m128JaggieThreshold.m128i_i16[i] = nJaggieThreshold;
		m128_YMask.m128i_i16[i] = 0xff;
	}

	return IDX_DEFLT_RET_OK;
}

void __stdcall ExecDeIntFilter (int szH, int szV, unsigned char *bufYUV)
{
	DeIntAsmFunc (szH, szV /2, bufYUV);
}

#define IS_MMX	1
void DeinterlaceFieldBob_I420_MMX(DWORD LineLength, DWORD FieldHeight, unsigned char *bufImg)
{
    DWORD Line;
    BYTE* YVal1;
    BYTE* YVal2;
    BYTE* YVal3;
    
	DWORD LineLengthX2 = LineLength *2;
	DWORD LineLengthDiv4 = LineLength >>2;

    YVal1 = &bufImg[0];
    YVal2 = &bufImg[LineLength];
    YVal3 = &bufImg[LineLengthX2];
    for (Line = 0; Line < FieldHeight - 1; Line++)
    {
        _asm
        {
            mov ecx, LineLengthDiv4
            mov eax, dword ptr [YVal1]
            mov ebx, dword ptr [YVal2]
            mov edx, dword ptr [YVal3]

align 8
MAINLOOP_LABEL:         
            movd mm0, dword ptr[eax] 
            movd mm1, dword ptr[ebx] 
            movd mm2, dword ptr[edx]

			movd mm3, dwZero
			punpcklbw mm0, mm3
			punpcklbw mm1, mm3
			punpcklbw mm2, mm3

            // get intensities in mm3 - 4
            movq mm3, mm0
            movq mm4, mm1
            movq mm5, mm2

//            pand mm3, YMask
//            pand mm4, YMask
//            pand mm5, YMask

            // get average in mm0
            pand  mm0, Mask
            pand  mm2, Mask
            psrlw mm0, 01
            psrlw mm2, 01
            paddw mm0, mm2

            // work out (O1 - E) * (O2 - E) / 2 - EdgeDetect * (O1 - O2) ^ 2 >> 12
            // result will be in mm6

            psrlw mm3, 01
            psrlw mm4, 01
            psrlw mm5, 01

            movq mm6, mm3
            psubw mm6, mm4  //mm6 = O1 - E

            movq mm7, mm5
            psubw mm7, mm4  //mm7 = O2 - E

            pmullw mm6, mm7     // mm0 = (O1 - E) * (O2 - E)

            movq mm7, mm3
            psubw mm7, mm5      // mm7 = (O1 - O2)
            pmullw mm7, mm7     // mm7 = (O1 - O2) ^ 2
//            psrlw mm7, 12       // mm7 = (O1 - O2) ^ 2 >> 12
            psrlw mm7, 3       // mm7 = (O1 - O2) ^ 2 >> 12
            pmullw mm7, qwEdgeDetect        // mm7  = EdgeDetect * (O1 - O2) ^ 2 >> 12

            psubw mm6, mm7      // mm6 is what we want

            pcmpgtw mm6, qwThreshold

            movq mm7, mm6

            pand mm0, mm6

            pandn mm7, mm1

            por mm7, mm0

//            psrlw mm7, 8
			packuswb mm7, mm3

            movd dword ptr[ebx], mm7

            add eax, 4
            add ebx, 4
            add edx, 4
            dec ecx
            jne near MAINLOOP_LABEL
        }

		YVal1 += LineLengthX2;
		YVal2 += LineLengthX2;
		YVal3 += LineLengthX2;
    }

    _asm
    {
        emms
    }
}

void DeinterlaceFieldBob_I420_SSE(DWORD LineLength, DWORD FieldHeight, unsigned char *bufImg)
{
    DWORD Line;
    BYTE* YVal1;
    BYTE* YVal2;
    BYTE* YVal3;
    
	DWORD LineLengthX2 = LineLength *2;
	DWORD LineLengthDiv8 = LineLength >>3;

    YVal1 = &bufImg[0];
    YVal2 = &bufImg[LineLength];
    YVal3 = &bufImg[LineLengthX2];
    for (Line = 0; Line < FieldHeight - 1; Line++)
    {
        _asm
        {
            mov ecx, LineLengthDiv8
            mov eax, dword ptr [YVal1]
            mov esi, dword ptr [YVal2]
            mov edx, dword ptr [YVal3]

align 8
MAINLOOP_LABEL:         
            movq xmm0, qword ptr[eax] 
            movq xmm1, qword ptr[esi] 
            movq xmm2, qword ptr[edx]

			movq xmm3, qwZero
			punpcklbw xmm0, xmm3
			punpcklbw xmm1, xmm3
			punpcklbw xmm2, xmm3

            // get intensities in mm3 - 4
            movdqa xmm3, xmm0
            movdqa xmm4, xmm1
            movdqa xmm5, xmm2

            // get average in mm0
            pavgb xmm0, xmm2
//            movdqa xmm0, XMMWORD PTR[bufFF]

            // work out (O1 - E) * (O2 - E) / 2 - EdgeDetect * (O1 - O2) ^ 2 >> 12
            // result will be in mm6

            psrlw xmm3, 01
            psrlw xmm4, 01
            psrlw xmm5, 01

            movdqa xmm6, xmm3
            psubw xmm6, xmm4  //mm6 = O1 - E

            movdqa xmm7, xmm5
            psubw xmm7, xmm4  //mm7 = O2 - E

            pmullw xmm6, xmm7     // mm0 = (O1 - E) * (O2 - E)

            movdqa xmm7, xmm3
            psubw xmm7, xmm5      // mm7 = (O1 - O2)
            pmullw xmm7, xmm7     // mm7 = (O1 - O2) ^ 2
//            psrlw xmm7, 12       // mm7 = (O1 - O2) ^ 2 >> 12
            psrlw xmm7, 3       // mm7 = (O1 - O2) ^ 2 >> 12

            pmullw xmm7, XMMWORD PTR[m128EdgeDetect]       // mm7  = EdgeDetect * (O1 - O2) ^ 2 >> 12

            psubw xmm6, xmm7      // mm6 is what we want

            movdqa xmm2, XMMWORD PTR[m128JaggieThreshold]
            pcmpgtw mm6, xmm2

            movdqa xmm7, xmm6
            pand xmm0, xmm6
            pandn xmm7, xmm1
            por xmm7, xmm0

			packuswb xmm7, xmm3

            movq qword ptr[esi], xmm7

//			packuswb xmm0, xmm3
//            movq qword ptr[esi], xmm0

            add eax, 8
            add esi, 8
            add edx, 8
            dec ecx
            jne near MAINLOOP_LABEL
        }

		YVal1 += LineLengthX2;
		YVal2 += LineLengthX2;
		YVal3 += LineLengthX2;
    }

    _asm
    {
        emms
    }
}

void DeinterlaceFieldBob_YUV2_MMX(DWORD LineLength, DWORD FieldHeight, unsigned char *bufImg)
{
    DWORD Line;
    BYTE* YVal1;
    BYTE* YVal2;
    BYTE* YVal3;
    
	DWORD LineLengthX4 = LineLength *4;
	DWORD LineLengthX2Div8 = LineLength *2 >>3;
    const __int64 YMask    = 0x00ff00ff00ff00ff;

    YVal1 = &bufImg[0];
    YVal2 = &bufImg[LineLength *2];
    YVal3 = &bufImg[LineLengthX4];
    for (Line = 0; Line < FieldHeight - 1; Line++)
    {
        _asm
        {
            mov ecx, LineLengthX2Div8
            mov eax, dword ptr [YVal1]
            mov ebx, dword ptr [YVal2]
            mov edx, dword ptr [YVal3]

align 8
MAINLOOP_LABEL:         
            movq mm0, qword ptr[eax] 
            movq mm1, qword ptr[ebx] 
            movq mm2, qword ptr[edx]

            // get intensities in mm3 - 4
            movq mm3, mm0
            movq mm4, mm1
            movq mm5, mm2

            pand mm3, YMask
            pand mm4, YMask
            pand mm5, YMask

            // get average in mm0
            pand  mm0, Mask
            pand  mm2, Mask
            psrlw mm0, 01
            psrlw mm2, 01
            paddw mm0, mm2

            // work out (O1 - E) * (O2 - E) / 2 - EdgeDetect * (O1 - O2) ^ 2 >> 12
            // result will be in mm6

            psrlw mm3, 01
            psrlw mm4, 01
            psrlw mm5, 01

            movq mm6, mm3
            psubw mm6, mm4  //mm6 = O1 - E

            movq mm7, mm5
            psubw mm7, mm4  //mm7 = O2 - E

            pmullw mm6, mm7     // mm0 = (O1 - E) * (O2 - E)

            movq mm7, mm3
            psubw mm7, mm5      // mm7 = (O1 - O2)
            pmullw mm7, mm7     // mm7 = (O1 - O2) ^ 2
            psrlw mm7, 3       // mm7 = (O1 - O2) ^ 2 >> 12
            pmullw mm7, qwEdgeDetect        // mm7  = EdgeDetect * (O1 - O2) ^ 2 >> 12

            psubw mm6, mm7      // mm6 is what we want

            pcmpgtw mm6, qwThreshold

            movq mm7, mm6

            pand mm0, mm6

            pandn mm7, mm1

            por mm7, mm0

            movq qword ptr[ebx], mm7

            add eax, 8
            add ebx, 8
            add edx, 8
            dec ecx
            jne near MAINLOOP_LABEL
        }

		YVal1 += LineLengthX4;
		YVal2 += LineLengthX4;
		YVal3 += LineLengthX4;
    }

    _asm
    {
        emms
    }
}

void DeinterlaceFieldBob_YUV2_SSE(DWORD LineLength, DWORD FieldHeight, unsigned char *bufImg)
{
    DWORD Line;
    BYTE* YVal1;
    BYTE* YVal2;
    BYTE* YVal3;
    
	DWORD LineLengthX4 = LineLength *4;
	DWORD LineLengthX2Div16 = LineLength *2 >>4;

    YVal1 = &bufImg[0];
    YVal2 = &bufImg[LineLength *2];
    YVal3 = &bufImg[LineLengthX4];
    for (Line = 0; Line < FieldHeight - 1; Line++)
    {
        _asm
        {
            mov ecx, LineLengthX2Div16
            mov eax, dword ptr [YVal1]
            mov esi, dword ptr [YVal2]
            mov edx, dword ptr [YVal3]

align 16
MAINLOOP_LABEL:         
            movdqu xmm0, XMMWORD PTR[eax] 
            movdqu xmm1, XMMWORD PTR[esi] 
            movdqu xmm2, XMMWORD PTR[edx]

            // get intensities in mm3 - 4
            movdqa xmm3, xmm0
            movdqa xmm4, xmm1
            movdqa xmm5, xmm2

            pand xmm3, XMMWORD PTR[m128_YMask]
            pand xmm4, XMMWORD PTR[m128_YMask]
            pand xmm5, XMMWORD PTR[m128_YMask]

            // get average in mm0
            pavgb xmm0, xmm2
//            movdqa xmm0, XMMWORD PTR[bufFF]

            // work out (O1 - E) * (O2 - E) / 2 - EdgeDetect * (O1 - O2) ^ 2 >> 12
            // result will be in mm6

            psrlw xmm3, 01
            psrlw xmm4, 01
            psrlw xmm5, 01

            movdqa xmm6, xmm3
            psubw xmm6, xmm4  //mm6 = O1 - E

            movdqa xmm7, xmm5
            psubw xmm7, xmm4  //mm7 = O2 - E

            pmullw xmm6, xmm7     // mm0 = (O1 - E) * (O2 - E)

            movdqa xmm7, xmm3
            psubw xmm7, xmm5      // mm7 = (O1 - O2)
            pmullw xmm7, xmm7     // mm7 = (O1 - O2) ^ 2
            psrlw xmm7, 3       // mm7 = (O1 - O2) ^ 2 >> 12

            pmullw xmm7, XMMWORD PTR[m128EdgeDetect]       // mm7  = EdgeDetect * (O1 - O2) ^ 2 >> 12

            psubw xmm6, xmm7      // mm6 is what we want

            movdqa xmm2, XMMWORD PTR[m128JaggieThreshold]
            pcmpgtw mm6, xmm2

            movdqa xmm7, xmm6
            pand xmm0, xmm6
            pandn xmm7, xmm1
            por xmm7, xmm0

            movdqu XMMWORD PTR[esi], xmm7

            add eax, 16
            add esi, 16
            add edx, 16
            dec ecx
            jne near MAINLOOP_LABEL
        }

		YVal1 += LineLengthX4;
		YVal2 += LineLengthX4;
		YVal3 += LineLengthX4;
    }

    _asm
    {
        emms
    }
}

