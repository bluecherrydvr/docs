#ifndef __ISIL_CHANNEL_MAP_H__
#define __ISIL_CHANNEL_MAP_H__


#ifdef __cplusplus
extern "C" {
#endif
#include "isil_ap_mediasdk.h"

#define TEST_CHIP_CNT 1

#define TEST_CHIP0_SD (16)

#define TEST_CHIP0_HD (0)



#define TEST_CHIP1_SD (8)

#define TEST_CHIP1_HD (2)


#define TEST_CHIP2_SD (8)

#define TEST_CHIP2_HD (2)


#define TEST_CHIP3_SD (8)

#define TEST_CHIP3_HD (2)


#define TEST_HD_CHN_BASE_NUM 16


typedef struct
{
    unsigned int SD_ChnCnt;
    unsigned int HD_ChnCnt;
}ISIL_CHANNEL_CNT_PER_CHIP;

typedef struct
{
	unsigned int chipNo;
	ISIL_CHANNEL_CNT_PER_CHIP chntype;
}CHIP_CHN_INFO;

extern int ISIL_GetSysMaxChnCnt();
extern int ISIL_GetChipCnt();
extern int ISIL_GetChipChnIDByLogicID(unsigned int log_id , CHIP_CHN_PHY_ID *phy_id_ptr);
extern int ISIL_GetLogicIDByChipChnID(unsigned int *log_id , CHIP_CHN_PHY_ID *phy_id_ptr);
extern int ISIL_GetLogicChnIDByPhyChnID(int chipid, int phychnid);
extern void ISIL_TestChnMap();

extern int ISIL_GetChnCntPerChip(unsigned int* SDchn, unsigned int* HDchn, unsigned int chipNo);

#ifdef __cplusplus
}
#endif

#endif
