#pragma once

class CMemoryBitstream 
{
public:
	CMemoryBitstream();
	~CMemoryBitstream();
	
	void AllocBytes(unsigned int numBytes);
	void SetBytes(unsigned char* pBytes, unsigned int numBytes);
	void PutBytes(unsigned char* pBytes, unsigned int numBytes);
	void PutBits(unsigned int bits, unsigned int numBits);
	unsigned int GetBits(unsigned int numBits);
	void SkipBytes(unsigned int numBytes);
	void SkipBits(unsigned int numBits);
	unsigned int GetBitPosition();
	void SetBitPosition(unsigned int bitPos);
	unsigned char* GetBuffer();
	unsigned int GetNumberOfBytes();
	unsigned int GetNumberOfBits();

protected:
	unsigned char*	m_pBuf;
	unsigned int	m_bitPos;
	unsigned int	m_numBits;
};
