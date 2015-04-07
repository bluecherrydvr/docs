#include "isil_channel_map.h"
#include "isil_codec_chip_ctl.h"
#include "config.h"
#if 1

static int g_totalchipcnt;
static int g_totalchncnt;
static CHIP_CHN_INFO chipchninfo[TOTAL_CHIPS_NUM];
static CHAN_MAP_T ChipChnMapTable[TOTAL_CHIPS_NUM][SD_HD_CHANNEL_PER_CHIP];

static void ISIL_ResetChpChnCnt(){
	g_totalchipcnt = 0;
	g_totalchncnt = 0;
}

int ISIL_GetChipCnt(){
	return g_totalchipcnt;
}

static int ISIL_SetChipCnt(int chipcnt){
	
	return g_totalchipcnt = chipcnt;
}

static int ISIL_GetChnCnt(){
	
	return g_totalchncnt;
}
static int ISIL_SetChnCnt(int chnCnt){
	
	return g_totalchncnt = chnCnt;
}

int ISIL_GetChnCntPerChip(unsigned int* SDchn, unsigned int* HDchn, unsigned int chipNo){

	int i;
	int chncnt = -1;
	if(NULL == SDchn || NULL == HDchn || chipNo > TOTAL_CHIPS_NUM) {
		return -1;
	} 
	#if 0
	for(i = 0; i < TOTAL_CHIPS_NUM; i++) {
		if(chipchninfo[i].chipNo == chipNo) {
			chncnt = chipchninfo[i].chntype.SD_ChnCnt + chipchninfo[i].chntype.HD_ChnCnt;
			*SDchn = chipchninfo[i].chntype.SD_ChnCnt;
			*HDchn = chipchninfo[i].chntype.HD_ChnCnt;
			fprintf(stderr, "%s chip %d sdchn %d  hdchn %d\n", __FUNCTION__, i, *SDchn, *HDchn);
			if(chncnt > SD_HD_CHANNEL_PER_CHIP) {
				fprintf(stderr, "%s chn too big\n", __FUNCTION__);
				return -1;
			}
			break;
		}
	}
	#endif
	chncnt = chipchninfo[chipNo].chntype.SD_ChnCnt + chipchninfo[chipNo].chntype.HD_ChnCnt;
	*SDchn = chipchninfo[chipNo].chntype.SD_ChnCnt;
	*HDchn = chipchninfo[chipNo].chntype.HD_ChnCnt;
	fprintf(stderr, "%s chip %d sdchn %d  hdchn %d total chn %d\n", __FUNCTION__, chipNo, *SDchn, *HDchn, chncnt);
	if(chncnt > SD_HD_CHANNEL_PER_CHIP) {
		fprintf(stderr, "%s chn too big\n", __FUNCTION__);
		return -1;
			}
	return chncnt;
}

static int ISIL_InitChipChnTableByCfg(){
	/*read system confige file*/
	
	int i, j, chn_num = 0, totalchn = 0;
	unsigned int SDchn;
	unsigned int HDchn;
	memset(chipchninfo, 0x00, sizeof(chipchninfo));
	memset(ChipChnMapTable,0xff, sizeof(ChipChnMapTable));

	ISIL_ResetChpChnCnt();
	ISIL_SetChipCnt(TEST_CHIP_CNT);

	chipchninfo[0].chipNo = 0;
	chipchninfo[0].chntype.HD_ChnCnt = TEST_CHIP0_HD;
	chipchninfo[0].chntype.SD_ChnCnt = TEST_CHIP0_SD;

	chipchninfo[1].chipNo = 1;
	chipchninfo[1].chntype.HD_ChnCnt = TEST_CHIP1_HD;
	chipchninfo[1].chntype.SD_ChnCnt = TEST_CHIP1_SD;

	chipchninfo[2].chipNo = 2;
	chipchninfo[2].chntype.HD_ChnCnt = TEST_CHIP2_HD;
	chipchninfo[2].chntype.SD_ChnCnt = TEST_CHIP2_SD;

	chipchninfo[3].chipNo = 3;
	chipchninfo[3].chntype.HD_ChnCnt = TEST_CHIP3_HD;
	chipchninfo[3].chntype.SD_ChnCnt = TEST_CHIP3_SD;
	
	for(i = 0; i < ISIL_GetChipCnt(); i++) {
		chn_num = ISIL_GetChnCntPerChip(&SDchn, &HDchn, i);
		fprintf(stderr, "%s chip %d sdchn %d  hdchn %d\n", __FUNCTION__,  i, SDchn, HDchn);
		if(chn_num != -1) {
			
			/*设置SD部分的map*/
			if(SDchn != 0) {
				for(j = 0; j < SDchn; j++) {
					ChipChnMapTable[i][j].logic_slot_id = j+totalchn;
					ChipChnMapTable[i][j].phy_slot_id = j;
					printf("set chip %d sd log %d phy %d\n", i, ChipChnMapTable[i][j].logic_slot_id, ChipChnMapTable[i][j].phy_slot_id);
				}
			}
			/*设置HD部分的map*/
			if(HDchn != 0) {
				for(j = 0; j < HDchn; j ++) {
					ChipChnMapTable[i][SDchn+j].logic_slot_id = totalchn + SDchn + j;
					ChipChnMapTable[i][SDchn+j].phy_slot_id = TEST_HD_CHN_BASE_NUM+j;
					printf("set chip %d hd log %d phy %d\n", i, ChipChnMapTable[i][SDchn+j].logic_slot_id, ChipChnMapTable[i][SDchn+j].phy_slot_id);
				}
			}
			totalchn += chn_num;
		}
	}
	ISIL_SetChnCnt(totalchn);
	return 0;
}

static int ISIL_InitChnMap(){
	int i, j;
    int ret = 0;
    unsigned int SDchn;
	unsigned int HDchn;
    char tmpbuff[sizeof(CODEC_CHIP_CHLOGIC2PHY_MAP) + SD_HD_CHANNEL_PER_CHIP*sizeof(CHAN_MAP_T)];
    CODEC_CHIP_CHLOGIC2PHY_MAP *MapTbl_p;
    CHAN_MAP_T *pchanMapTbl,*tmpchanMapTbl;

    ISIL_InitChipChnTableByCfg();
   
    /*设置CHN MAP*/
    for(i = 0; i < ISIL_GetChipCnt(); i++)
    {
		ret = ISIL_GetChnCntPerChip(&SDchn, &HDchn, i);
		fprintf(stderr, "%s chip %d total chncnt %d\n", __FUNCTION__, i, ret);
		if(ret != -1) {

			memset(tmpbuff, 0x00, sizeof(tmpbuff));
			MapTbl_p = (CODEC_CHIP_CHLOGIC2PHY_MAP *)tmpbuff;

			MapTbl_p->e_bind_type = LOGIC_MAP_TABLE_BIND_H264E;
			MapTbl_p->i_chan_number = ret;
			pchanMapTbl = tmpchanMapTbl = MapTbl_p->map_table;
			
			for(j = 0; j < ret; j++) {
				pchanMapTbl->logic_slot_id = ChipChnMapTable[i][j].logic_slot_id;
				pchanMapTbl->phy_slot_id = ChipChnMapTable[i][j].phy_slot_id;
				printf("%s map chip %d log %d -> phy %d\n", __FUNCTION__, i, pchanMapTbl->logic_slot_id, pchanMapTbl->phy_slot_id);
				pchanMapTbl++;
			}
			ret = ISIL_CODEC_CTL_SetChLogic2PhyMap(i, MapTbl_p);
			MapTbl_p->e_bind_type = LOGIC_MAP_TABLE_BIND_JPEGE;
			ret = ISIL_CODEC_CTL_SetChLogic2PhyMap(i, MapTbl_p);
		}
    }
    return ret;
}

int ISIL_GetChipChnIDByLogicID(unsigned int log_id , CHIP_CHN_PHY_ID *phy_id_ptr){
	int ret, i;
	unsigned int chncnt, chipcnt;
	unsigned int tmpchncnt1 = 0, tmpchncnt2 =0;
	unsigned int SDchn, HDchn;

	chncnt = ISIL_GetChnCnt();
	fprintf(stderr, "%s total chncnt %d\n", __FUNCTION__, chncnt);

	if(log_id >= chncnt) {
		fprintf(stderr, "%s log_ch %d > total %d\n", __FUNCTION__, log_id, chncnt);
		return -1;
	}


	chipcnt = ISIL_GetChipCnt();
	chncnt = 0;
	fprintf(stderr, "%s total chip %d\n", __FUNCTION__, chipcnt);
	for(i = 0; i < chipcnt; i++) {
		chncnt = ISIL_GetChnCntPerChip(&SDchn, &HDchn, i);
		tmpchncnt1 += chncnt;
		if(log_id >= tmpchncnt1) {
			tmpchncnt2 = chncnt;
			continue;
		}
		else{
			phy_id_ptr->chipid = i;
			phy_id_ptr->chanid = log_id - tmpchncnt2;
			fprintf(stderr, "%s logic chn %d find chip %d chn %d\n", __FUNCTION__, log_id, phy_id_ptr->chipid, phy_id_ptr->chanid);
			break;
		}
	}
	//fprintf(stderr, "%s logic chn %d find chip %d chn %d\n", __FUNCTION__, log_id, phy_id_ptr->chipid, phy_id_ptr->chanid);
	return 0;
}

int ISIL_GetLogicIDByChipChnID(unsigned int *log_id , CHIP_CHN_PHY_ID *phy_id_ptr){

	if(NULL == log_id || NULL == phy_id_ptr) {
		return -1;
	}

	if(phy_id_ptr->chipid >= TOTAL_CHIPS_NUM || phy_id_ptr->chanid >= SD_HD_CHANNEL_PER_CHIP) {

		fprintf(stderr, "%s chip %d chn %d too big \n", __FUNCTION__, phy_id_ptr->chipid, phy_id_ptr->chanid);
		return -1;
	}
	*log_id = ChipChnMapTable[phy_id_ptr->chipid][phy_id_ptr->chanid].logic_slot_id;
	fprintf(stderr, "%s chip %d chn %d find logic chn %d \n", __FUNCTION__, phy_id_ptr->chipid, phy_id_ptr->chanid , *log_id);
	return 0;
}

int ISIL_GetSysMaxChnCnt(){
	return ISIL_GetChnCnt();
}

int ISIL_GetLogicChnIDByPhyChnID(int chipid, int phychnid){

	int i, chncnt;
	unsigned int SDchn, HDchn;

	chncnt = ISIL_GetChnCntPerChip(&SDchn, &HDchn, chipid);
	if(chncnt > 0) {
		for(i = 0; i < chncnt; i++) {
			if(ChipChnMapTable[chipid][i].phy_slot_id == phychnid) {
				fprintf(stderr, "%s phyid %d -> logicid %d found\n", 
						__FUNCTION__, ChipChnMapTable[chipid][i].phy_slot_id,
						ChipChnMapTable[chipid][i].logic_slot_id);
				return ChipChnMapTable[chipid][i].logic_slot_id;
			}
		}
	}

	return -1;
}
void ISIL_TestChnMap(){
	#define LOGICAL_ID 10
	#define CHIP_ID 1
	#define CHAN_ID 2
	CHIP_CHN_PHY_ID phy_id_ptr;
	unsigned int log_id;
	ISIL_InitChnMap();

	#if 0
	ISIL_GetChipChnIDByLogicID(LOGICAL_ID, &phy_id_ptr);

	phy_id_ptr.chipid = CHIP_ID;
	phy_id_ptr.chanid = CHAN_ID;
	ISIL_GetLogicIDByChipChnID(&log_id, &phy_id_ptr);
	#endif
}
#endif
