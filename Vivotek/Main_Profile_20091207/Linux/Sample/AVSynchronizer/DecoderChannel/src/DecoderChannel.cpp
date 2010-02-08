#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include "AVSynchronizer.h"
#include "CDataPacketParser.h"

#ifdef _WIN32
#define	CHAR_PATH_SEP			'\\'
#else
#define	CHAR_PATH_SEP			'/'
#endif //_WIN32

char g_szFolder[] = "DecodedOutput";


void CreateOutputFolder()
{
#ifdef _WIN32
	CreateDirectory(g_szFolder, NULL);
#else
	mkdir(g_szFolder, 0777);
	chmod(g_szFolder, 07777);
#endif //_WIN32
}

int SaveRawData(TFRAMEINFO *ptFrame, EPIXELFORMAT eOutFormat, EFRAMETYPE eFrameType)
{
	const char *pszVideoSubName[] = {"yuv", "rgb", "rgb", "rgb", "bmp", "bmp", "bmp", "jpg", "rgb",
												"bmp", "yuv", "bgr", "bgr", "bgr", "bgr", "yuv"};
	const char *pszAudioSubName = "pcm";
	char szFileName[256];
	FILE *pf = NULL;	
	
	if (eFrameType == VIDEO_FRAME)
	{
		static int nVideoFileIdx = 0;
		sprintf(szFileName, "%s%cImg%d.%s", g_szFolder, CHAR_PATH_SEP, nVideoFileIdx, 
												pszVideoSubName[(int)eOutFormat - 1]);
		pf = fopen(szFileName, "wb");
		fwrite(ptFrame->tVideoFrame.pbyPicture,1, ptFrame->tVideoFrame.dwSize, pf);
		fclose(pf);
		nVideoFileIdx++;
	}
	else
	{
		static int nAudioFileIdx = 0;
		sprintf(szFileName, "%s%cAudio%d.pcm", g_szFolder, CHAR_PATH_SEP, nAudioFileIdx);
		printf("file path %s\n", szFileName);
		pf = fopen(szFileName, "wb");
		fwrite(ptFrame->tAudioFrame.pbySound,1, ptFrame->tAudioFrame.dwSize, pf);
		fclose(pf);
		nAudioFileIdx++;
	}
												
	return 0;
}

int main(int argc, char *argv[])
{
	if (argc < 3 || atoi(argv[2]) > PF_YV12)
	{
		printf("Usage: DecoderChannel.out [Input file] [Output format] [Write file]\n");
		printf("Output format-> 1: YUY2, 2: RGB16565, 3: RGB24, 4: RGB32, 5: BMP16565\n");
		printf("                6: BMP24, 7: BMP32, 8: JPEG, 9: RGB16555, 10: BMP16555\n");
		printf("                11: YUV, 12: BGR16565, 13: BGR24, 14: BGR32, 15: BGR16555\n");
		printf("                16: YV12\n");
		printf("Write file->    T: write decoded data to file, F: Do not write decoded data\n");
		return 1;
	}

	CDataPacketParser *pParser = NULL;
	
	try 
	{
		pParser = new CDataPacketParser(argv[1]);
	}
	catch(...)
	{
		return 1;
	}
	
	HANDLE 				hAvSync = NULL;
	HANDLE 				hDecodeChannel = NULL;
	SCODE					scRet = S_OK;
	EPIXELFORMAT	eOutFormat = (EPIXELFORMAT)atoi(argv[2]);
	BOOL					bOutputFile = argv[3][0] == 'T' ? TRUE : FALSE;  
	
	if (bOutputFile)
	{
			CreateOutputFolder();
	}
 	
	scRet = AvSynchronizer_InitialEx(&hAvSync, NULL, NULL, NULL, AUDIOOUT_FOCUS_NORMAL, 0, AVSYNCHRONIZER_VERSION, 
						DECODER_CHANNEL_ONLY, 0x0FFFFFF);
	
	if (scRet != S_OK)
	{
		printf("AvSynchronizer_InitialEx fail %x\n", scRet);
		return 1;
	}
						
	TDECCHOPTION 	tDecChannelOptions ;
	DWORD 				dwBuffSize = 1600 * 1200 * 4 + 1000;
	BYTE					*pbyBuf = new BYTE[dwBuffSize];

	memset(&tDecChannelOptions, 0, sizeof(tDecChannelOptions));
	
	tDecChannelOptions.dwRawDataFormat = eOutFormat;
	scRet = AvSynchronizer_CreateDecoderChannel (hAvSync, &hDecodeChannel, &tDecChannelOptions);
	if (scRet != S_OK)
	{
		printf("AvSynchronizer_CreateDecoderChannel failed with error: %X", scRet);
		return -1;
	}
	
	TMediaDataPacketInfoV3 	tPacketInfo;
	TFRAMEINFO 				tFrameInfo;
	int						nVideoCount = 0, nAudioCount = 0;
	
	do
	{
		DWORD 	dwDecodedSize = 0;
		BOOL	bVideo = TRUE;
		
		scRet = pParser->GetOneMediaDataPacketInfo(&tPacketInfo);	
		
		if (scRet != S_OK)
		{
			printf("get one packet fail\n");
			break;
		}
		
		if (tPacketInfo.tIfEx.tInfo.dwStreamType < mctG7221)
		{
			memset(&tFrameInfo.tVideoFrame, 0, sizeof(tFrameInfo.tVideoFrame));
			tFrameInfo.tVideoFrame.pbyHeader = pbyBuf;
			tFrameInfo.tVideoFrame.dwSize = dwBuffSize;
			scRet = AvSynchronizer_DecodeVideo(hDecodeChannel, (TMediaDataPacketInfo *)&tPacketInfo, 
								&tFrameInfo.tVideoFrame, &dwDecodedSize);
			nVideoCount++;
			bVideo = TRUE;
		}
		else
		{
			memset(&tFrameInfo.tAudioFrame, 0, sizeof(tFrameInfo.tAudioFrame));
			tFrameInfo.tAudioFrame.pbySound = pbyBuf;
			tFrameInfo.tAudioFrame.dwSize = dwBuffSize;
			scRet = AvSynchronizer_DecodeAudio(hDecodeChannel, (TMediaDataPacketInfo *)&tPacketInfo, 
								&tFrameInfo.tAudioFrame, &dwDecodedSize);	
			nAudioCount++;
			bVideo = FALSE;
		}
		
		if (scRet == S_OK)
		{
			if (bOutputFile)
			{
				SaveRawData(&tFrameInfo, eOutFormat, bVideo ? (EFRAMETYPE) VIDEO_FRAME : AUDIO_FRAME);
			}
		}
		else
		{
			printf("decode error %s %x\n", bVideo ? "Video" : "Audio", scRet);
		}
		
	}while (scRet == S_OK);

	printf("Video frames: %d, Audio frames: %d\n", nVideoCount, nAudioCount);

	delete pParser;

	scRet = AvSynchronizer_DeleteDecoderChannel(hAvSync, &hDecodeChannel);

	scRet = AvSynchronizer_Release(&hAvSync);
	
	
	return 0;	
}
