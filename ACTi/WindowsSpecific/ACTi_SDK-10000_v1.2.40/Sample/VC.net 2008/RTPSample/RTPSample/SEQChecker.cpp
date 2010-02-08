#include "StdAfx.h"
#include "SEQChecker.h"

CSEQChecker::CSEQChecker(void)
{
}

CSEQChecker::~CSEQChecker(void)
{
}

MP4_STREAM_ATTR * CSEQChecker::GetSEQInfo( BYTE *pSEQData, int nLen )
{
	BitstreamInit( &m_MP4Bs, pSEQData, nLen );
	if( BitstreamReadHeaders( &m_MP4Bs, &m_ATTR ) == 0 )
		return &m_ATTR;
	return NULL;
}

int CSEQChecker::BitstreamReadHeaders( MP4_BIT_STREAM * bs, MP4_STREAM_ATTR * attr )
{
	unsigned long shape = 0;
	unsigned long aspect = 0;
	unsigned long start_code = 0;
	unsigned long vol_ver_id = 1;
	unsigned long time_inc_bits = 1;
	unsigned long time_increment_resolution = 0;

	do 
	{
		BitstreamByteAlign( bs );
		start_code = BitstreamShowBits( bs, 32 );
		if( ( start_code & ~0x0000001f ) == 0x00000100 )
			BitstreamSkip( bs, 32 );

		else if( ( start_code & ~0x0000000f ) == 0x00000120 ) 
		{
			BitstreamSkip( bs, 32 );
			BitstreamSkip( bs, 1 );
			BitstreamSkip( bs, 8 );
			if( BitstreamGetBit( bs ) )
			{
				vol_ver_id = BitstreamGetBits( bs, 4 );
				BitstreamSkip( bs, 3 );
			} 

			if( ( aspect = BitstreamGetBits( bs, 4 ) ) == 15 )
			{
				int w = BitstreamGetBits( bs, 8 );
				int h = BitstreamGetBits( bs, 8 );
			}

			if( BitstreamGetBit( bs ) )
			{
				BitstreamSkip( bs, 2 );
				BitstreamGetBit( bs );
				if( BitstreamGetBit( bs ) )
				{
					//BitstreamShowBits( bs, 15 );
					BitstreamSkip( bs, 15 );
					READ_MARKER( bs, 1 );
					//BitstreamShowBits( bs, 15 );
					BitstreamSkip( bs, 15 );
					READ_MARKER( bs, 1 );
					//BitstreamShowBits( bs, 15 );
					BitstreamSkip( bs, 15 );
					READ_MARKER( bs, 1 );
					//BitstreamShowBits( bs, 3 );
					BitstreamSkip( bs, 3 );
					//BitstreamShowBits( bs, 11 );
					BitstreamSkip( bs, 11 );
					READ_MARKER( bs, 1 );
					//BitstreamShowBits( bs, 15 );
					BitstreamSkip( bs, 15 );
					READ_MARKER( bs, 1 );
				}
			}
			shape = BitstreamGetBits( bs, 2 );
			if( shape == 3 && vol_ver_id != 1 ) 
				BitstreamSkip( bs, 4 );

			READ_MARKER( bs, 1 );
			time_increment_resolution = BitstreamGetBits( bs, 16 );
			attr->fps = ( time_increment_resolution > 1000 ) ? time_increment_resolution / 1000 : time_increment_resolution;
			
			if( time_increment_resolution > 0 )
				time_inc_bits = max( LogToBinary( time_increment_resolution - 1 ), 1 );

			READ_MARKER( bs, 1 );
			if( BitstreamGetBit( bs ) )
			{
// 				BitstreamShowBits( bs, time_inc_bits );
				BitstreamSkip( bs, time_inc_bits );
			}

//			if( shape == 0 )
//			{
				READ_MARKER( bs, 1 );
				attr->width = BitstreamGetBits( bs, 13 );
				READ_MARKER( bs, 1 );
				attr->height = BitstreamGetBits( bs, 13 );
				return 0;
//			}
		}

		else
			BitstreamSkip(bs, 8);
	}while( ( BitstreamPos(bs) >> 3 ) < bs->length );
	return -1;
}

void CSEQChecker::BitstreamInit( MP4_BIT_STREAM * const bs, void *const bitstream, unsigned long length )
{
	unsigned long tmp;
	unsigned long *s;

	s = (unsigned long *)((unsigned int)bitstream & (-4));
	if( s == (unsigned long *)bitstream )
	{
		bs->pike = 0;
	} 
	else 
	{
		bs->pike = ((unsigned long)bitstream - (unsigned long)s) * 8;
	}
	bs->start = (unsigned long *)bitstream;
	bs->tail = s;
	tmp = *s;
	MP4_BSWAP(tmp);
	bs->bufa = tmp;
	tmp = *(s + 1);
	MP4_BSWAP(tmp);
	bs->bufb = tmp;
	bs->buf = 0;
	bs->pos = bs->pike;
	bs->length = length;
}

void CSEQChecker::BitstreamSkip( MP4_BIT_STREAM * const bs, const unsigned long bits )
{
	bs->pos += bits;
	if( bs->pos >= 32 )
	{
		unsigned long tmp;
		bs->bufa = bs->bufb;
		tmp = *(bs->tail + 2);
		MP4_BSWAP(tmp);
		bs->bufb = tmp;
		bs->tail++;
		bs->pos -= 32;
	}
}

void CSEQChecker::READ_MARKER( MP4_BIT_STREAM * const bs, const unsigned long bits )
{
	BitstreamSkip(bs,bits);
}

unsigned long CSEQChecker::BitstreamPos( const MP4_BIT_STREAM * const bs )
{
	return((unsigned long)(8*((unsigned long)bs->tail - (unsigned long)bs->start) + bs->pos ));
}


unsigned long CSEQChecker::BitstreamShowBits( MP4_BIT_STREAM * const bs, const unsigned long bits )
{
	int nbit = (bits + bs->pos) - 32;
	if( nbit > 0 ) 
	{
		return ((bs->bufa & (0xffffffff >> bs->pos)) << nbit) | (bs->bufb >> (32 -nbit));
	} 
	else 
	{
		return (bs->bufa & (0xffffffff >> bs->pos)) >> (32 - bs->pos - bits);
	}
}

void CSEQChecker::BitstreamByteAlign( MP4_BIT_STREAM * const bs )
{
	unsigned long remainder = bs->pos % 8;

	if( remainder )
		BitstreamSkip(bs, 8 - remainder);
}

unsigned long CSEQChecker::BitstreamGetBit( MP4_BIT_STREAM * const bs )
{
	return BitstreamGetBits(bs, 1);
}

unsigned long CSEQChecker::BitstreamGetBits( MP4_BIT_STREAM * const bs, const unsigned long n )
{
	unsigned long ret = BitstreamShowBits(bs, n);
	BitstreamSkip(bs, n);
	return ret;
}

unsigned long CSEQChecker::LogToBinary(unsigned long value)
{
	int n = 0;
	if( value & 0xffff0000 )
	{
		value >>= 16;
		n += 16;
	}
	if( value & 0xff00 ) 
	{
		value >>= 8;
		n += 8;
	}
	if( value & 0xf0 ) 
	{
		value >>= 4;
		n += 4;
	}
	return n + log2_tab_16[value];
}
