#pragma once
#define DLL_API extern "C" __declspec(dllimport)

namespace FFRAW {
	typedef struct _tagRAWFILE_HEADER_INFO
	{
		DWORD			dwFirstB2;			// '00' '00' '01' 'B2'
		DWORD			dwStreamType;		// 11 : TCP-1.0; 22 TCP-2.0
		DWORD			dwVideoType;		// 11 : ISO 14496; 22 : ...
											// (user should check every B2 header, here is 0x11 always)
		DWORD			dwAudioType;		// 00 : NONE; 22 : PCM
											// (user should check every B2 header, here is 0x22 always)
		DWORD			dwControlType;		// 00 : NONE; 11 : TCP-2.0
		DWORD			dwAudio2Type;		// Reserve
		DWORD			dwControl2Type;		// Reserve
		DWORD			dwBiteRate;			// Bite rate 0 - 18 (Check out the programming guild for details)
		DWORD 			dwFps;				// Fps :1 - 30
		DWORD 			dwResolution;		// Resolution : same as B2 header
											// (Check out the programming guild for details)
		DWORD 			dwReserve1;			// Reserve from 1 - 6 :FF
		DWORD 			dwReserve2;			// Reserve from 1 - 6 :FF
		DWORD 			dwReserve3;			// Reserve from 1 - 6 :FF
		DWORD 			dwReserve4;			// Reserve from 1 - 6 :FF
		DWORD 			dwReserve5;			// Reserve from 1 - 6 :FF
		DWORD 			dwReserve6;			// Reserve from 1 - 6 :FF
	} RAWFILE_HEADER_INFO;

	typedef struct _tagRAWFILE_RECORD_INFO
	{
		time_t			tBeginTime;
		time_t			tEndTime;
		BYTE			btTimeZone;
		DWORD			dwGOP;
		DWORD			dwFrameCount;
		ULONGLONG		FileSize;
	}RAWFILE_RECORD_INFO;

	DLL_API void FGetVersion( char* VersionInfo );
	DLL_API HANDLE FInit();
	DLL_API bool FStartRecord( HANDLE h, char* FileName, RAWFILE_HEADER_INFO* mhi );
	DLL_API void FStopRecord( HANDLE h, RAWFILE_RECORD_INFO* mri );
	DLL_API void FWriteData( HANDLE h, BYTE* pData, DWORD dwLen );
	DLL_API void FExit( HANDLE h );
};