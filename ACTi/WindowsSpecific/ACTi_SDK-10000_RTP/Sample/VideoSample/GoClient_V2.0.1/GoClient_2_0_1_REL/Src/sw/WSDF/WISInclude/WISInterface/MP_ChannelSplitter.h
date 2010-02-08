
#ifndef  __MP_CHANNEL_SPLITTER__
#define  __MP_CHANNEL_SPLITTER__

#include	<../WSDF.H>
#include	<../MultiMedia.H>

#ifdef __cplusplus



	class IMP_ChannelSplitter
	{
	public:
		struct FrameBuffer
		{
			FrameBuffer() { _bfr = NULL; _len = 0; }
			~FrameBuffer() {}

			UINT8* _bfr;
			SINT32 _len;
		};

		// constructor
		STATIC	SINT32	CreateInstance(
					IMP_ChannelSplitter	**pp);	// return a pointer to CChannelSplitter object


		void	Release();

		virtual SINT32 InitSplitter(	IN SINT32 nWidth,		// go stream horizontal resolution
						IN SINT32 nHeight,		// go stream vertical resolution
						IN SINT8 nRows = 2,		// # of vertical splitting 
						IN SINT8 nCols = 2 ) PURE;	// # of horizontal splitting

		virtual void UninitSplitter() PURE;

		virtual SINT32 GetOutputWidth() PURE;
		virtual SINT32 GetOutputHeight() PURE;

		virtual SINT32 ChannelInputFrame( FrameBuffer& frame ) PURE;	// input frame data
		virtual SINT32 ChannelOutputFrame( IN SINT32 nRowId,		// target row of the specified output stream
						IN SINT32 nColId,				// target col of the specified output stream
						OUT FrameBuffer& frame ) PURE;		// output frame buffer for receiving data
	};

#endif

#endif