// demux_raw.h
#ifndef _DEMUX_RAW_H_
#define _DEMUX_RAW_H_

#include "stsplitter.h"

/***************************************************************************
    NAL type per H.264 standard
***************************************************************************/
typedef enum {
    H264_NAL_UNKNOWN		= 0,
    H264_NAL_SLICE			= 1,
    H264_NAL_SLICE_DPA		= 2,
    H264_NAL_SLICE_DPB		= 3,
    H264_NAL_SLICE_DPC		= 4,
    H264_NAL_SLICE_IDR		= 5,    
    H264_NAL_SEI			= 6,    
    H264_NAL_SPS			= 7,
    H264_NAL_PPS			= 8,
    H264_NAL_AUD			= 9,
    H264_NAL_END_OF_SEQ     = 10,       
    H264_NAL_END_OF_STREAM  = 11,       
    H264_NAL_FILLER_DATA    = 12,       
    H264_NAL_PREFIX         = 14,       
    H264_NAL_SUBSET_SPS     = 15,
    H264_NAL_SLICE_SCALABLE = 20,
} h264_nal_type_e;


// pci packet header definition copied from sdvr_ui_sdk.h
// modify timestamp and remove payload pointer to reflect
// header size only
typedef struct {
    sx_uint8        board_id;
    sx_uint8		channel_type;
    sx_uint8        channel_id;
    sx_uint8		frame_type;
    sx_uint8        motion_detected;
    sx_uint8        blind_detected;
    sx_uint8        night_detected;
    sx_uint8        av_state_flags;
    sx_uint8        stream_id;
    sx_uint8        reserved1;
    sx_uint16       reserved2;
    sx_uint32       payload_size;
    sx_uint64       timestamp;
    sx_uint32       reserved4;
    sx_uint32       reserved5;
} sdvr_av_buffer_t1;

// splitter class
class CDemuxRawFile : public CDemuxFile
{
public:
    ~CDemuxRawFile();
    CDemuxRawFile();

    // open/close file
    virtual sdvr_player_err_e Open(LPSTR pstrFileName);    
	virtual sdvr_player_err_e Close(void);

    // set play position
    virtual sdvr_player_err_e SetPlayPos(float fRelativePos);
    // set current time
    virtual sdvr_player_err_e SetCurrentTime(DWORD dwTime);
    // set current frame number
    virtual sdvr_player_err_e SetCurrentFrameNum(DWORD dwFrameNum);

    virtual sdvr_player_err_e ParseFile(void);
	virtual sdvr_player_err_e ParseEsFile(void);


	// for debugging
	virtual void PrintInfo();

public:
	unsigned int m_uiIsEs;
	unsigned int m_uiIsAvc;

	// For parsing AVC elementary stream	
	unsigned int	m_uiRdBytePos;
	char			*m_pBsStart;
	sx_uint8		*m_pDataBuffer;
	sx_uint64		m_uiTimestamp;

	unsigned int	m_uiRdBitPos;
	unsigned int	m_uiBufferSize;


private:
	sdvr_player_err_e ParsePacket();
	sdvr_player_err_e ParseEsPacket();
	void			  SetBsStart(char *start, int size);
	unsigned char	  SeekNextNalHdr();
	int				  ParseNextByteAligned();

};


#endif
