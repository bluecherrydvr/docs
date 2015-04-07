#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h> 

#include "isil_sys_config.h"

static struct boardsMgt glbBoardMgt;

static struct allChipInfMgts glbAllChipInfMgts;

int read_board_config(const char *path,struct boardsMgt *bMgt)
{
    int ret ,fd;
    if(bMgt == NULL || path == NULL) {
        return -1;
    }

    memset(bMgt,0x00, sizeof(*bMgt));
    fd = open(path ,READ_FLAGS );
    if( fd < 0) {
        fprintf(stderr,"read open path[%s] failed .\n",path);
        return -1;
    }

    ret = read(fd ,(void *)bMgt,sizeof(*bMgt));
    if( ret < 0 ){
        fprintf(stderr,"read_board_config failed .\n");
        close(fd);
        return -1;
    }
    return 0;
}

int read_glb_board_config(const char *path)
{
    return read_board_config(path ,&glbBoardMgt);
}

struct boardsMgt * get_glb_boardsMgt( void )
{
    return &glbBoardMgt;
}


void malloc_int_glbAllChipInfMgt( int num)
{
    int i;
    if(num <= 0) {
        return;
    }
    
    glbAllChipInfMgts.number = num;
    glbAllChipInfMgts.chipsInf = (struct chipInfMgt *)calloc(num ,sizeof(struct chipInfMgt) );
    if( !glbAllChipInfMgts.chipsInf ) {
        fprintf(stderr,"Malloc glbAllChipInfMgts.chipsInf failed .\n");
        exit(-1);
    }

    return;
}

void free_glbAllChipInfMgt(void)
{
    if(glbAllChipInfMgts.chipsInf) {
        free(glbAllChipInfMgts.chipsInf);
        glbAllChipInfMgts.chipsInf = NULL;
    }
    glbAllChipInfMgts.number = 0;
}

struct  allChipInfMgts *getGlbAllChipInfMgtsPtr(void )
{
    return &glbAllChipInfMgts;
}






























