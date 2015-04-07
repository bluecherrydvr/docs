#ifndef  _ISIL_SYS_CONFIG_H
#define _ISIL_SYS_CONFIG_H



#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"

#define ISIL_CHIP_NAME_LEN (16)
#define MAX_CHANS_NUM_PER_CHIP (16)

#define BOARD_CONFIG_PATH "/flash/board.config"

enum portProperty{
		PortIs27M = 0,
		PortIs54M,
		PortIs108M,
};

enum portType{
	PORTUNDEFINE = 0,
	PORTISVI   = 1,
    PORTISVO  = 2,
};



struct boardsMgt{
    int board_num;  //board numbers
    int start_num;
    int chips_num;
};

struct chipInfMgt{
//    char name[ISIL_CHIP_NAME_LEN];
    int boardId;
    int chipId;  //Continuous num
    int maxFrameRate;  //100 frame rate(D1) etc.
    int vEncChanNum;
    int vDecChanNum;
    int aEncChanNum; 
    int aDecChanNum; 
    int viChanNum;
    int voChanNum;
    char vEncEnableT[MAX_CHANS_NUM_PER_CHIP]; //0:disalbe 1:enable
    char aEncEnableT[MAX_CHANS_NUM_PER_CHIP];
    char vDecEnableT[MAX_CHANS_NUM_PER_CHIP];
    char aDecEnableT[MAX_CHANS_NUM_PER_CHIP];
    char viEnableT[MAX_CHANS_NUM_PER_CHIP];
    char voEnableT[MAX_CHANS_NUM_PER_CHIP];
};


struct allChipInfMgts{
    int number;
    struct chipInfMgt *chipsInf;
};

struct vbListT{
    char havaPre; //0: no , 1: have
    unsigned char preBoardId;
    unsigned char preChipId;
    unsigned char preId;
    unsigned short preType;
    char havaNext; //0: no , 1: have
    unsigned char nextBoardId;
    unsigned char nextChipId;
    unsigned char nextId;
    unsigned short nextType;
};


struct vbRelationT{
    unsigned char boardId;
    unsigned char chipId;
    unsigned short id;
    unsigned short pro; //property , look Enum portProperty
    unsigned short type; // enum portType
	struct vbListT vbList;
};


extern int read_glb_board_config(const char *path);

extern struct  allChipInfMgts *getGlbAllChipInfMgtsPtr(void );

void free_glbAllChipInfMgt(void);

extern void malloc_int_glbAllChipInfMgt( int num);

extern struct boardsMgt * get_glb_boardsMgt( void );

extern int read_glb_board_config(const char *path);

extern int read_board_config(const char *path,struct boardsMgt *bMgt);





#ifdef __cplusplus
}
#endif //__cplusplus

#endif
