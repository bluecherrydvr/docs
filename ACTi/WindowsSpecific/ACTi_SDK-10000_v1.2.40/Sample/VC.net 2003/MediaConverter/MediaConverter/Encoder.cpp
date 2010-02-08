#include "StdAfx.h"
#include "encoder.h"


CXEncoder::CXEncoder()
{
	///////////////////////////////////////////
	// Codec Interface
	///////////////////////////////////////////
	m_XVID_hinstDecoderAdapter = NULL;
	m_MJPG_hinstDecoderAdapter = NULL;
	m_H264_hinstDecoderAdapter = NULL;

	m_XVID_lpfnCInit = NULL;
	m_XVID_lpfnCExit = NULL;
	m_XVID_lpfnCDecode = NULL;

	m_MJPG_lpfnCInit = NULL;
	m_MJPG_lpfnCExit = NULL;
	m_MJPG_lpfnCDecode = NULL;

	m_H264_lpfnCInit = NULL;
	m_H264_lpfnCExit = NULL;
	m_H264_lpfnCDecode = NULL;

	m_XVID_nDecoderHandle = -1;
	m_MJPG_nDecoderHandle = -1;
	m_H264_nDecoderHandle = -1;

	InitAllCodec();
}

CXEncoder::~CXEncoder()
{
	ReleaseAllCodec();
}

void CXEncoder::ReleaseAllCodec()
{
	if( m_XVID_nDecoderHandle >= 0 )
		m_XVID_lpfnCExit( m_XVID_nDecoderHandle );
	m_XVID_nDecoderHandle = -1;

	if( m_MJPG_nDecoderHandle >= 0 )
		m_MJPG_lpfnCExit( m_MJPG_nDecoderHandle );
	m_MJPG_nDecoderHandle = -1;

	if( m_H264_nDecoderHandle >= 0 )
		m_H264_lpfnCExit( m_H264_nDecoderHandle );
	m_H264_nDecoderHandle = -1;

}
bool CXEncoder::InitAllCodec()
{
	ReleaseAllCodec();

	//XVID
	m_XVID_hinstDecoderAdapter = LoadLibrary( DECODER_XVID );
	if( m_XVID_hinstDecoderAdapter )
	{
		m_XVID_lpfnCInit				= (XXCInit)	GetProcAddress( m_XVID_hinstDecoderAdapter, "XCInit"  );
		m_XVID_lpfnCExit				= (XXCExit)	GetProcAddress( m_XVID_hinstDecoderAdapter, "XCExit"  );
		m_XVID_lpfnCDecode				= (XXDecode)	GetProcAddress( m_XVID_hinstDecoderAdapter, "XDecode" );	
	}
	else
	{
		return false;
	}
	//M JPEG
	m_MJPG_hinstDecoderAdapter = LoadLibrary( DECODER_MJPEG );
	if( m_MJPG_hinstDecoderAdapter )
	{
		m_MJPG_lpfnCInit				= (XXCInit)	GetProcAddress( m_MJPG_hinstDecoderAdapter, "XCInit"  );
		m_MJPG_lpfnCExit				= (XXCExit)	GetProcAddress( m_MJPG_hinstDecoderAdapter, "XCExit"  );
		m_MJPG_lpfnCDecode				= (XXDecode)	GetProcAddress( m_MJPG_hinstDecoderAdapter, "XDecode" );	
	}
	else
	{
		return false;
	}
	//H264
	m_H264_hinstDecoderAdapter = LoadLibrary( DECODER_IH264 );
	if( m_H264_hinstDecoderAdapter )
	{
		m_H264_lpfnCInit				= (XXCInit)	GetProcAddress( m_H264_hinstDecoderAdapter, "XCInit"  );
		m_H264_lpfnCExit				= (XXCExit)	GetProcAddress( m_H264_hinstDecoderAdapter, "XCExit"  );
		m_H264_lpfnCDecode				= (XXDecode)	GetProcAddress( m_H264_hinstDecoderAdapter, "XDecode" );	
	}
	else
	{
		return false;
	}

	m_XVID_nDecoderHandle = m_XVID_lpfnCInit();
	m_MJPG_nDecoderHandle = m_MJPG_lpfnCInit();
	m_H264_nDecoderHandle = m_H264_lpfnCInit();

	return true;
}
