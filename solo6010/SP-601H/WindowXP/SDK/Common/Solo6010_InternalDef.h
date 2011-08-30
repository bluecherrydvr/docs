#pragma once

// Thread[S]
enum
{
	S6010_IDX_TH_MP4,
	S6010_IDX_TH_JPEG,
	S6010_IDX_TH_G723,
	S6010_IDX_TH_OVERLAY_FLIP,
	S6010_IDX_TH_V_MOTION,
	S6010_IDX_TH_CAM_BLK_DET,
	S6010_IDX_TH_GPIO_INT_ALARM,
	S6010_IDX_TH_ADV_MOTION_DATA,
	S6010_NUM_THREAD
};
// Thread[E]

// Event[S]
enum
{
	IDX_LAST_COMMON_EVT = COMMON_EVT_ADV_MOTION_DATA,

	S6010_EVT_CAM_BLK_DET,
	S6010_NUM_EVENT
};

const char *const GL_STR_FORMAT_EVT[S6010_NUM_EVENT] = 
{
	"MP4E_%d",
	"JPEGE_%d",
	"G723E_%d",
	"OV_FL_%d",
	"V_MOT_%d",
	"MP4DC_%d",
	"GPINT_%d",
	"ADV_M_%d",

	"CBD_%d",
};
// Event[E]

// Callback[S]
enum
{
	S6010_CB_OVERLAY_FLIP,
	S6010_CB_MP4_CAPTURE,
	S6010_CB_JPEG_CAPTURE,
	S6010_CB_G723_CAPTURE,
	S6010_CB_V_MOTION_ALARM,
	S6010_CB_CAM_BLK_DET_ALARM,
	S6010_CB_GPIO_INT_ALARM,
	S6010_CB_ADV_MOT_DET_ALARM,
	S6010_NUM_CALLBACK
};
// Callback[E]

// Video Motion[S]
#define INTERVAL_VMOTION_CHECK		100
// Video Motion[E]

// Camera Block Detection[S]
#define DEF_ALARM_LEVEL				50			// Similarity(%) (Higher value = More sensitive, Lower value = Less sensitive)
#define DEF_CHECK_INTERVAL			1000
#define MIN_INTERVAL_CAM_BLK_DET	1000
// Camera Block Detection[E]
