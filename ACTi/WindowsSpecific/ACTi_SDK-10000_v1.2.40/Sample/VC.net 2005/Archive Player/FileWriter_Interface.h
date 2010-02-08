#pragma once


namespace FRAWADP
{
	typedef struct structural_MP4FILE_HEADER_INFO
	{
		DWORD			dwFirstB2;			// '00' '00' '01' 'B2'
		DWORD			dwStreamType;		// 11 : TCP-1.0; 22 TCP-2.0
		DWORD			dwVideoType;		// 11 : ISO 14496; 22 : ...
		DWORD			dwAudioType;		// 00 : NONE; 11 : PCM; ...
		DWORD			dwControlType;		// 00 : NONE; 11 : TCP-2.0
		DWORD			dwAudio2Type;		// Reserve
		DWORD			dwControl2Type;		// Reserve
		DWORD			dwBiteRate;			// Bite rate 0 - 15
		DWORD 			dwFps;				// Fps :1 - 30
		DWORD 			dwResolution;		// Resolution :0 - 5
		DWORD 			dwReserve1;			// Reserve from 1 - 6 :FF
		DWORD 			dwReserve2;			// Reserve from 1 - 6 :FF
		DWORD 			dwReserve3;			// Reserve from 1 - 6 :FF
		DWORD 			dwReserve4;			// Reserve from 1 - 6 :FF
		DWORD 			dwReserve5;			// Reserve from 1 - 6 :FF
		DWORD 			dwReserve6;			// Reserve from 1 - 6 :FF
	} MP4FILE_HEADER_INFO;

	typedef struct structural_MP4FILE_TAIL_INFO
	{
		DWORD			dw01B2;
		DWORD			dwHeader;			
		DWORD			dwVersion;			//Must be 0x01000001 (1.0.0.1)
		time_t			tBeginTime;
		time_t			tEndTime;
		DWORD			dwGOP;
		DWORD			dwGOPSize;
		DWORD			dwFPS;
		DWORD			dwWidth;
		DWORD			dwHeight;
		DWORD			dwFrameCount;
		DWORD			dwTimeZone;
		DWORD			dwReserve1;
		DWORD			dwReserve2;
	} MP4FILE_TAIL_INFO;


	typedef struct structural_MP4FILE_RECORD_INFO
	{
		time_t			tBeginTime;
		time_t			tEndTime;
		BYTE			btTimeZone;
		DWORD			dwGOP;
		DWORD			dwFrameCount;
		ULONGLONG		FileSize;
	} MP4FILE_RECORD_INFO;


	extern "C" __declspec(dllexport) void FGetVersion( char* VersionInfo );
	extern "C" __declspec(dllexport) HANDLE FInit();
	extern "C" __declspec(dllexport) bool FStartRecord( HANDLE h, char* FileName, MP4FILE_HEADER_INFO* mhi );
	extern "C" __declspec(dllexport) void FStopRecord( HANDLE h, MP4FILE_RECORD_INFO* mri );
	extern "C" __declspec(dllexport) void FWriteData( HANDLE h, BYTE* pData, DWORD dwLen );
	extern "C" __declspec(dllexport) void FExit( HANDLE h );
}

namespace FAVIADP
{
	typedef struct structural_MP4FILE_HEADER_INFO
	{
		DWORD			dwFirstB2;			// '00' '00' '01' 'B2'
		DWORD			dwStreamType;		// 11 : TCP-1.0; 22 TCP-2.0
		DWORD			dwVideoType;		// 11 : ISO 14496; 22 : ...
		DWORD			dwAudioType;		// 00 : NONE; 11 : PCM; ...
		DWORD			dwControlType;		// 00 : NONE; 11 : TCP-2.0
		DWORD			dwAudio2Type;		// Reserve
		DWORD			dwControl2Type;		// Reserve
		DWORD			dwBiteRate;			// Bite rate 0 - 15
		DWORD 			dwFps;				// Fps :1 - 30
		DWORD 			dwResolution;		// Resolution :0 - 5
		DWORD 			dwReserve1;			// Reserve from 1 - 6 :FF
		DWORD 			dwReserve2;			// Reserve from 1 - 6 :FF
		DWORD 			dwReserve3;			// Reserve from 1 - 6 :FF
		DWORD 			dwReserve4;			// Reserve from 1 - 6 :FF
		DWORD 			dwReserve5;			// Reserve from 1 - 6 :FF
		DWORD 			dwReserve6;			// Reserve from 1 - 6 :FF
	} MP4FILE_HEADER_INFO;

	typedef struct structural_MP4FILE_TAIL_INFO
	{
		DWORD			dw01B2;
		DWORD			dwHeader;		
		DWORD			dwVersion;			//Must be 0x01000001 (1.0.0.1)
		time_t			tBeginTime;
		time_t			tEndTime;
		DWORD			dwGOP;
		DWORD			dwGOPSize;
		DWORD			dwFPS;
		DWORD			dwWidth;
		DWORD			dwHeight;
		DWORD			dwFrameCount;
		DWORD			dwTimeZone;
		DWORD			dwReserve1;
		DWORD			dwReserve2;
	} MP4FILE_TAIL_INFO;


	typedef struct structural_MP4FILE_RECORD_INFO
	{
		time_t			tBeginTime;
		time_t			tEndTime;
		BYTE			btTimeZone;
		DWORD			dwGOP;
		DWORD			dwFrameCount;
		ULONGLONG		FileSize;
	} MP4FILE_RECORD_INFO;


	extern "C" __declspec(dllexport) void FAVIGetVersion( char* VersionInfo );
	extern "C" __declspec(dllexport) HANDLE FAVIInit();
	extern "C" __declspec(dllexport) bool FAVIStartRecord( HANDLE h, char* FileName, MP4FILE_HEADER_INFO* mhi );
	extern "C" __declspec(dllexport) void FAVIStopRecord( HANDLE h, MP4FILE_RECORD_INFO* mri );
	extern "C" __declspec(dllexport) void FAVIWriteData( HANDLE h, BYTE* pData, DWORD dwLen );
	extern "C" __declspec(dllexport) void FAVIExit( HANDLE h );
	extern "C" __declspec(dllexport) void FAVIGetAVIInfo( HANDLE h, char *caFileName );
}


namespace FRAW2
{
	typedef struct structural_MP4FILE_HEADER_INFO
	{
		DWORD			dwFirstB2;			// '00' '00' '01' 'B2'
		DWORD			dwStreamType;		// 11 : TCP-1.0; 22 TCP-2.0
		DWORD			dwVideoType;		// 11 : ISO 14496; 22 : ...
		DWORD			dwAudioType;		// 00 : NONE; 11 : PCM; ...
		DWORD			dwControlType;		// 00 : NONE; 11 : TCP-2.0
		DWORD			dwAudio2Type;		// Reserve
		DWORD			dwControl2Type;		// Reserve
		DWORD			dwBiteRate;			// Bite rate 0 - 15
		DWORD 			dwFps;				// Fps :1 - 30
		DWORD 			dwResolution;		// Resolution :0 - 5
		DWORD 			dwReserve1;			// Reserve from 1 - 6 :FF
		DWORD 			dwReserve2;			// Reserve from 1 - 6 :FF
		DWORD 			dwReserve3;			// Reserve from 1 - 6 :FF
		DWORD 			dwReserve4;			// Reserve from 1 - 6 :FF
		DWORD 			dwReserve5;			// Reserve from 1 - 6 :FF
		DWORD 			dwReserve6;			// Reserve from 1 - 6 :FF
	} MP4FILE_HEADER_INFO;

	typedef struct structural_MP4FILE_TAIL_INFO
	{
		DWORD			dw01B2;
		DWORD			dwHeader;			
		DWORD			dwVersion;			//Must be 0x01000001 (1.0.0.1)
		time_t			tBeginTime;
		time_t			tEndTime;
		DWORD			dwGOP;
		DWORD			dwGOPSize;
		DWORD			dwFPS;
		DWORD			dwWidth;
		DWORD			dwHeight;
		DWORD			dwFrameCount;
		DWORD			dwTimeZone;
		DWORD			dwReserve1;
		DWORD			dwReserve2;
	} MP4FILE_TAIL_INFO;


	typedef struct structural_MP4FILE_RECORD_INFO
	{
		time_t			tBeginTime;
		time_t			tEndTime;
		BYTE			btTimeZone;
		DWORD			dwGOP;
		DWORD			dwFrameCount;
		ULONGLONG		FileSize;
	} MP4FILE_RECORD_INFO;


	extern "C" __declspec(dllexport) void F2GetVersion( char* VersionInfo );
	extern "C" __declspec(dllexport) HANDLE F2Init();
	extern "C" __declspec(dllexport) bool F2StartRecord( HANDLE h, char* FileName, MP4FILE_HEADER_INFO* mhi );
	extern "C" __declspec(dllexport) void F2StopRecord( HANDLE h, MP4FILE_RECORD_INFO* mri );
	extern "C" __declspec(dllexport) void F2WriteData( HANDLE h, BYTE* pData, DWORD dwLen );
	extern "C" __declspec(dllexport) void F2Exit( HANDLE h );
}
