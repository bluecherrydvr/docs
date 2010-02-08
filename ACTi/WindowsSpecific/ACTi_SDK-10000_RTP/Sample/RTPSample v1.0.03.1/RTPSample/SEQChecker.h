#pragma once

#ifdef _WIN32_WINDOWS
	static const unsigned char log2_tab_16[16] =  { 0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4 };
	#define MP4_BSWAP(a) __asm mov eax,a __asm bswap eax __asm mov a, eax
#else
	static const unsigned char log2_tab_16[16] =  { 0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4 };
	#define MP4_BSWAP(a) ((a) = (((a) & 0xff) << 24)  | (((a) & 0xff00) << 8) | (((a) >> 8) & 0xff00) | (((a) >> 24) & 0xff))
#endif

typedef struct
{
	int     width;
	int     height;
	int     fps;
} MP4_STREAM_ATTR;

typedef struct
{
	unsigned long bufa;
	unsigned long bufb;
	unsigned long buf;
	unsigned long pos;
	unsigned long pike;
	unsigned long *tail;
	unsigned long *start;
	unsigned long length;
} MP4_BIT_STREAM;

class CSEQChecker
{
public:
	CSEQChecker(void);
	~CSEQChecker(void);

	MP4_STREAM_ATTR * GetSEQInfo( BYTE *pSEQData, int nLen = 21 );

private:
	MP4_BIT_STREAM	m_MP4Bs;
	MP4_STREAM_ATTR m_ATTR;
	void BitstreamInit( MP4_BIT_STREAM * const bs, void *const bitstream, unsigned long length );
	int  BitstreamReadHeaders( MP4_BIT_STREAM * bs, MP4_STREAM_ATTR * attr );
	void BitstreamSkip( MP4_BIT_STREAM * const bs, const unsigned long bits );
	void READ_MARKER( MP4_BIT_STREAM * const bs, const unsigned long bits );
	void BitstreamByteAlign( MP4_BIT_STREAM * const bs );
	unsigned long BitstreamPos( const MP4_BIT_STREAM * const bs );
	unsigned long BitstreamShowBits( MP4_BIT_STREAM * const bs, const unsigned long bits );
	unsigned long BitstreamGetBit( MP4_BIT_STREAM * const bs );
	unsigned long BitstreamGetBits( MP4_BIT_STREAM * const bs, const unsigned long n );
	unsigned long LogToBinary( unsigned long value );

public:
	int GetSequenceHeader( BYTE* pRaw, int nRawLen, BYTE* pSEQ )
	{
		DWORD dw00 = 0x00010000;
		DWORD dw20 = 0x20010000;
		DWORD dw00_IPP = 0x02010000;
		DWORD dw20_IPP = 0x22010000;

		int iSeqPlace = 0;
		int tLen = nRawLen - 21;
		//m_dwSEQCheckerWakeUpCounter = 0;
		do 
		{
			if( ( memcmp( &pRaw[iSeqPlace], &dw00, 4 ) == 0 && memcmp( &pRaw[iSeqPlace+4], &dw20, 4 ) == 0 ) ||
				( memcmp( &pRaw[iSeqPlace], &dw00_IPP, 4 ) == 0 && memcmp( &pRaw[iSeqPlace+4], &dw20_IPP, 4 ) == 0 ) )
			{
				memcpy( pSEQ, &pRaw[iSeqPlace], 21 );
				return 21;
			}
		} while( tLen > iSeqPlace++ );
		return 0;
	}
};
