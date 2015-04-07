#ifndef __ISIL_FM_H264_H__
#define __ISIL_FM_H264_H__

#ifdef __cplusplus
extern "C" {
#endif


typedef enum
{
	E_H264_FRAME_TYPE_NULL = 0x00,
	//master stream of h264
	E_FRAME_TYPE_MAIN	= 0x10,
	E_H264_FRAME_TYPE_MAIN_SPS,
	E_H264_FRAME_TYPE_MAIN_PPS,
    E_H264_FRAME_TYPE_MAIN_IDR,
    E_H264_FRAME_TYPE_MAIN_I,
    E_H264_FRAME_TYPE_MAIN_P,
    E_H264_FRAME_TYPE_MAIN_B,
    E_H264_FRAME_TYPE_MAIN_SI,
    E_H264_FRAME_TYPE_MAIN_SP,
    E_H264_FRAME_TYPE_MAIN_MV,
	//add new type here of h264 main
	E_FRAME_TYPE_MAIN_END = 0x1f,

	//sub stream of h264
	E_FRAME_TYPE_SUB = 0x20,
	E_H264_FRAME_TYPE_SUB_SPS,
	E_H264_FRAME_TYPE_SUB_PPS,
	E_H264_FRAME_TYPE_SUB_IDR,
    E_H264_FRAME_TYPE_SUB_I,
    E_H264_FRAME_TYPE_SUB_P,
    E_H264_FRAME_TYPE_SUB_B,
    E_H264_FRAME_TYPE_SUB_SI,
    E_H264_FRAME_TYPE_SUB_SP,
    E_H264_FRAME_TYPE_SUB_MV,
	//add new type here of h264 sub
	E_FRAME_TYPE_SUB_END = 0x2f,

	//mjpg stream
	E_FRAME_TYPE_MJPG = 0x30,
    E_H264_MJPEG_FRAME_TYPE,
	//add new type here of mjpg
	E_FRAME_TYPE_MJPG_END = 0x3f,

	//audio stream
	E_FRAME_TYPE_AUDIO = 0x40,
    E_AUDIO_FRAME_TYPE_PCM,
	E_AUDIO_FRAME_TYPE_ADPCM,
	E_AUDIO_FRAME_TYPE_ALAW,
	E_AUDIO_FRAME_TYPE_ULAW,
	//add new type here of audio
	E_FRAME_TYPE_AUDIO_END = 0x4f,

	//add new type
}eframe_type;


typedef struct
{
    unsigned char   *payload;      //指向视频数据的地址
    unsigned int    type;            //视频帧的类型
    unsigned int    len;             //视频数据的大小，以byte为单位
    unsigned int    timestamp;       //视频的时戳
}stDriverFrame;


/*H264文件记录及播放需要共同用到的数据结构定义如下*/

/*定义录像文件后缀名*/
#define H264_FILE_SUFFIX  "264"

//Windows 文件格式
//h264: (sps,pps,idr,p, .. ,GopNal), ..., FileIdxNal,FileTailNal

//linux 文件格式
//h264:    (sps,pps,idr,p, .. ,GopNal), ...,FileTailNal
//IdxFile: FileIdxNal

//其中：
//GopNal:     ISIL_GOP_NAL,ISIL_NAL_INDEX,...
//FileIdxNal: ISIL_FILE_INDEX_NAL,ISIL_FILE_INDEX_NAL,...

/*录像文件中帧类型定义*/
/*enum eISIL_FILE_NAL_TYPE
{
	ISIL_NAL_UNKNOWN      	= 0,
	ISIL_NAL_SPS          	= 0xA0,
	ISIL_NAL_PPS          	= 0xA1,
	ISIL_NAL_SLICE        	= 0xB0,
	ISIL_NAL_SLICE_IDR   	= 0xB1,
	ISIL_NAL_SLICE_DPA  	    = 0xB2,
	ISIL_NAL_SLICE_DPB    	= 0xB3,
	ISIL_NAL_SLICE_DPC    	= 0xB4,
	ISIL_NAL_SLICE_P      	= 0xB5,
	ISIL_NAL_AUDIO_PCM    	= 0xC0,
	ISIL_NAL_AUDIO_ALAW      = 0xC1,
	ISIL_NAL_AUDIO_ULAW  	= 0xC2,
	ISIL_NAL_AUDIO_ADPCM 	= 0xC3
};
*/
enum eISIL_NAL_TYPE
{
    ISIL_NAL_UNKNOWN      = 0,
    ISIL_NAL_MJPEG        = 0x80,
    ISIL_NAL_BMP          = 0x81,
    ISIL_NAL_SPS          = 0xA0,
    ISIL_NAL_PPS          = 0xA1,
    ISIL_NAL_SLICE        = 0xB0,
    ISIL_NAL_SLICE_IDR    = 0xB1,
    ISIL_NAL_SLICE_DPA    = 0xB2,
    ISIL_NAL_SLICE_DPB    = 0xB3,
    ISIL_NAL_SLICE_DPC    = 0xB4,
    ISIL_NAL_SLICE_P      = 0xB5,
    ISIL_NAL_SLICE_I      = 0xB6,
    ISIL_NAL_AUDIO_PCM    = 0xC0,
    ISIL_NAL_AUDIO_ALAW   = 0xC1,
    ISIL_NAL_AUDIO_ULAW   = 0xC2,
    ISIL_NAL_AUDIO_ADPCM  = 0xC3,
    ISIL_NAL_AUDIO_IMAADPCM  = 0xC4,
};


/*H264 Nal 结构*/
typedef struct _tagISIL_NAL_INDEX
{
	unsigned long cNalType:8;
	unsigned long lNalSize:24;
	unsigned long lNalOffset;//该NAL在整个H264文件中的位置
	unsigned long lTimeStamp;
}ISIL_NAL_INDEX;

/*define H264 GopNal data struct*/
typedef struct _tagISIL_GOP_NAL      //一级索引
{
	unsigned long  lGopFlag;        //NAL_HEAD 00 00 00 01 78

#if 0
defined(__LITTLE_ENDIAN_BITFIELD)
	unsigned long  cGopFlag:8;
	unsigned long  lLastGopSize:24; //上一个GopNal大小
//#else

	unsigned long  lLastGopSize:24; //上一个GopNal大小
    unsigned long  cGopFlag:8;

#else
    unsigned long  cGopFlag:8;
	unsigned long  lLastGopSize:24; //上一个GopNal大小
#endif
	unsigned long  lLastGopOffset;  //上一个GopNal偏移位置
	unsigned short nGopID;          //当前Gop ID 号
	unsigned short nNalCount;       //当前GopNal 中对应用多个ISIL_NAL_INDEX//
	ISIL_NAL_INDEX  Nal_Index[0];
}ISIL_GOP_NAL;

/**Gop index*/
typedef struct _tagISIL_GOP_INDEX
{
	unsigned long  lGopOffset;//该GOP在整个H264文件中的位置
	unsigned short nGopSize;
	unsigned short nIFrameCount;  //I frame count in gop
}ISIL_GOP_INDEX;


typedef struct _tagISIL_FILE_INDEX_NAL  //二级索引,可以写在文件尾，也可以单独写成文件
{
	unsigned long  lIndexFlag;         //NAL_HEAD 00 00 00 01 79
	unsigned long  cIndexFlag:8;
	unsigned long  lGopCount:24;
	ISIL_GOP_INDEX  Gop_Index[0];
}ISIL_FILE_INDEX_NAL;

typedef struct _tagISIL_FILE_TAIL_NAL
{
	unsigned long lTailFlag;       //NAL_HEAD 00 00 00 01 7A
	unsigned long cTailFlag:8;
	unsigned long lIndexSize:24;   //索引块在文件中的大小,如果文件中没有FileIdxNal,则表示文件最后一个GopNal大小
	unsigned long lIndexOffset;    //索引块在文件中的偏移,如果文件中没有FileIdxNal,则表示文件最后一个GopNal偏移位置
	unsigned long lAllNalCount;    //在文件中除去索引Nal外，总共有Nal数量, 即：ISIL_NAL_INDEX 个数
  	unsigned long lTimeBegin;      //文件视频的起始时间
	unsigned long lTimeEnd;        //文件视频的结束时间
	unsigned long lMaxNalSize;     //文件中视频或音频最大的NAL大小
	unsigned long lMaxGopNalSize;  //文件中Gop最大的NAL大小
	unsigned long lMaxKeyFrameInterval; //最大关键帧间隔
}ISIL_FILE_TAIL_NAL;





#ifdef __cplusplus
    }
#endif

#endif

