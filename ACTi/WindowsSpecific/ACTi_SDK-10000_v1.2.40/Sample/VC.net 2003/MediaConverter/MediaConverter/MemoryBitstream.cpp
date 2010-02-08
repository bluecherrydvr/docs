#include "StdAfx.h"
#include ".\memorybitstream.h"

CMemoryBitstream::CMemoryBitstream() 
{
	m_pBuf = NULL;
	m_bitPos = 0;
	m_numBits = 0;
}

CMemoryBitstream::~CMemoryBitstream()
{
	if( m_pBuf ) 
		delete [] m_pBuf;

	m_bitPos = 0;
	m_numBits = 0;
}

void CMemoryBitstream::AllocBytes( unsigned int numBytes ) 
{
	m_pBuf = new unsigned char[ numBytes ];
	if( !m_pBuf ) return;

	m_bitPos = 0;
	m_numBits = numBytes << 3;
}

void CMemoryBitstream::SetBytes( unsigned char* pBytes, unsigned int numBytes ) 
{
	m_pBuf = pBytes;
	m_bitPos = 0;
	m_numBits = numBytes << 3;
}

void CMemoryBitstream::PutBytes( unsigned char* pBytes, unsigned int numBytes )
{
	unsigned int numBits = numBytes << 3;

	if( numBits + m_bitPos > m_numBits ) return;

	if( (m_bitPos & 7) == 0 )
	{
		memcpy( &m_pBuf[m_bitPos >> 3], pBytes, numBytes );
		m_bitPos += numBits;
	}

	else
	{
		for( unsigned int i = 0; i < numBytes; i++ ) 
		{
			PutBits( pBytes[i], 8 );
		}
	}
}

void CMemoryBitstream::PutBits( unsigned int bits, unsigned int numBits )
{
	if( numBits + m_bitPos > m_numBits ) return;
	if( numBits > 32 ) return;

	for( int i = numBits - 1; i >= 0; i-- )
	{
		m_pBuf[ m_bitPos >> 3 ] |= (( bits >> i ) & 1 ) << ( 7 - ( m_bitPos & 7 ) );
		m_bitPos++;
	}
}

unsigned int CMemoryBitstream::GetBits( unsigned int numBits )
{
	if( numBits + m_bitPos > m_numBits ) return 0;
	if( numBits > 32 ) return 0;
	unsigned int bits = 0;

	for( int i = 0; i < numBits; i++ ) 
	{
		bits <<= 1;
		bits |= ( m_pBuf[ m_bitPos >> 3 ] >> ( 7 - ( m_bitPos & 7 ) ) ) & 1;
		m_bitPos++;
	}
	return bits;
}

void CMemoryBitstream::SkipBytes(unsigned int numBytes) 
{
	SkipBits(numBytes << 3);
}

void CMemoryBitstream::SkipBits(unsigned int numBits) 
{
	SetBitPosition(GetBitPosition() + numBits);
}

unsigned int CMemoryBitstream::GetBitPosition() 
{
	return m_bitPos;
}

void CMemoryBitstream::SetBitPosition(unsigned int bitPos) 
{
	if (bitPos > m_numBits) 
		return;

	m_bitPos = bitPos;
}

unsigned char* CMemoryBitstream::GetBuffer() 
{
	return m_pBuf;
}

unsigned int CMemoryBitstream::GetNumberOfBytes() 
{
	return (GetNumberOfBits() + 7) / 8;
}

unsigned int CMemoryBitstream::GetNumberOfBits() 
{
	return m_numBits;
}