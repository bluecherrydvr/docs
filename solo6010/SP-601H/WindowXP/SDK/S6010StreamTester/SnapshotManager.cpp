// SnapshotManager.cpp: implementation of the CSnapshotManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SnapshotManager.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define RANGE(a, b, c)	(((a)<(b)) ? (b) : ((a)>(c) ? (c) : (a)))

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSnapshotManager::CSnapshotManager()
{

}

CSnapshotManager::~CSnapshotManager()
{

}

void CSnapshotManager::WriteSnapshotToFile (BYTE *bufYUVImg, int nWidth, int nHeight,
	const char *strFileName)
{
	BYTE *bufBGR = new BYTE[nWidth *nHeight *3];

	ConvertYUVToBGR (bufYUVImg, nWidth, nHeight, bufBGR);
	SaveBGRToBMP (bufBGR, nWidth, nHeight, strFileName);

	delete[] bufBGR;
}

void CSnapshotManager::ConvertYUVToBGR (BYTE *bufYUV, int nWidth, int nHeight,
	BYTE *bufBGR)
{
	unsigned char *bufSrcY, *bufSrcU, *bufSrcV;
	bufSrcY = &bufYUV[0];
	bufSrcU = &bufYUV[nWidth *nHeight];
	bufSrcV = &bufYUV[nWidth *nHeight +nWidth *nHeight /4];

	int i, j;
	for (i=0; i<nHeight; i++)
	{
		for (j=0; j<nWidth; j++)
		{
			bufBGR[((nHeight -i -1) *3 *nWidth) +3 *j +0] = (BYTE)RANGE(
				bufSrcY[i *nWidth +j]
				+2.032 *(bufSrcU[(i /2 *nWidth /2) +j /2] -128), 0, 255);	// B...
			bufBGR[((nHeight -i -1) *3 *nWidth) +3 *j +1] = (BYTE)RANGE(
				bufSrcY[i *nWidth +j]
				-0.395 *(bufSrcU[(i /2 *nWidth /2) +j /2] -128)
				-0.581 *(bufSrcV[(i /2 *nWidth /2) +j /2] -128), 0, 255);	// G...
			bufBGR[((nHeight -i -1) *3 *nWidth) +3 *j +2] = (BYTE)RANGE(
				bufSrcY[i *nWidth +j]
				+1.140 *(bufSrcV[(i /2 *nWidth /2) +j /2] -128), 0, 255);	// R...
		}
	}
}

void CSnapshotManager::SaveBGRToBMP (BYTE *bufBGR, int nWidth, int nHeight, const char *strFileName)
{
	HANDLE hf = CreateFile (LPCSTR(strFileName), GENERIC_WRITE, FILE_SHARE_READ, NULL,
		CREATE_ALWAYS, NULL, NULL);

	if (hf == INVALID_HANDLE_VALUE)
		return;

	// Bitmap File Header...
	BITMAPFILEHEADER bfh;
	memset (&bfh, 0, sizeof(bfh));
	bfh.bfType = 'MB';
	bfh.bfSize = sizeof(bfh) +nWidth *nHeight *3 +sizeof(BITMAPINFOHEADER);
	bfh.bfOffBits = sizeof(BITMAPFILEHEADER) +sizeof(BITMAPINFOHEADER);

	DWORD dwWritten = 0;
	WriteFile (hf, &bfh, sizeof(bfh), &dwWritten, NULL);

	// Bitmap Info Header...
	BITMAPINFOHEADER bih;
	memset (&bih, 0, sizeof(bih));
	bih.biSize = sizeof(bih);
	bih.biWidth = nWidth;
	bih.biHeight = nHeight;
	bih.biPlanes = 1;
	bih.biBitCount = 24;

	dwWritten = 0;
	WriteFile (hf, &bih, sizeof(bih), &dwWritten, NULL);

	// Bitmap Data...
	dwWritten = 0;
	WriteFile (hf, bufBGR, nWidth *nHeight *3, &dwWritten, NULL);
	
	// File Close...
	CloseHandle (hf);
}

