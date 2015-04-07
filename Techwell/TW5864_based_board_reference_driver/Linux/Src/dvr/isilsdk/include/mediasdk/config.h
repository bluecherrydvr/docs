#ifndef _CONFIG_H
#define _CONFIG_H


#ifdef __cplusplus
extern "C" {
#endif


#define APP_VERSION_NUMBER "13"

#define TOTAL_CHIPS_NUM (4)
#define SD_HD_CHANNEL_PER_CHIP (18)
#define PER_CHIP_MAX_CHAN_NUM (16)

#define ISIL_USE_SYS_CFG_FILE

#define CHANNEL_NUMBER (TOTAL_CHIPS_NUM * PER_CHIP_MAX_CHAN_NUM)


#define WRITE_FLAGS     (O_WRONLY | O_CREAT | O_EXCL | O_TRUNC)
#define READ_FLAGS       O_RDONLY
#define WRITE_PERMS    (S_IRWXU | S_IROTH)


#define DEBUG_FUNCTION() \
do{\
   fprintf(stderr,"[file] = %s ,[func] %s,[line] =%d.\n",__FILE__,__FUNCTION__,__LINE__); \
}while(0)

#define SINGLE_DEBUG() \
do{\
    fprintf(stderr,"[file] = %s ,[func] %s,[line] =%d.\n",__FILE__,__FUNCTION__,__LINE__); \
    pause(); \
}while(0)


#define CHECK_BIT_VALUE(n,x)  (((n>>(x))&0x01) ? (1) :(0))

#define CLEAR_BIT_VALUE(n,x) ((n)&(~(1L <<(x))))

#define SET_BIT_VALUE(n,x) ((n) | (1L << (x)))



#ifndef MAX

#define MAX(a,b) (((a) > (b)) ? (a) : (b))

#endif

#ifndef MIN

#define MIN(a,b) (((a) < (b)) ? (a) : (b))

#endif




#ifdef __cplusplus
}
#endif //__cplusplus

#endif

