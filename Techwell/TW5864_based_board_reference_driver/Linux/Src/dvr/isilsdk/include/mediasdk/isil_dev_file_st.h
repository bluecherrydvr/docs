#ifndef  ISIL_DEV_FILE_ST_H
#define ISIL_DEV_FILE_ST_H

#include <linux/limits.h>

#include "linux_list.h"


#ifdef __cplusplus
extern "C" {
#endif

#define FILE_MAX_LEN  256

#define DEV_FILE_LIST_MAX_NUM 32

#define ISIL_SYS_ROOT_PATH "/sys/isil"

typedef enum
{
    IS_ENCODE_NODE = 1,
    IS_DECODE_NODE       ,
    IS_AUDIO_IN_NODE     ,
    IS_AUDIO_OUT_NODE  ,
    IS_VIDEO_IN_NODE     ,
    IS_VIDEO_OUT_NODE ,
    IS_CODEC_NODE  ,
}E_ISIL_DEV_TYPE;

typedef struct file_list_t file_list; 	

struct file_list_t
{
	struct list_head entry;
	struct list_head sub_file_list;
    char	file_name[FILE_MAX_LEN];   
    file_list* parent;
    char num_magic;
    char type;
    unsigned char dev_major;
    unsigned char dev_minor;
    void (*release)(file_list *fileList);
};

struct dev_list_t{
    unsigned char  chipNum;
    unsigned short num;
    file_list *dev_file_list[DEV_FILE_LIST_MAX_NUM];
};


extern file_list * malloc_init_file_list( void );

extern void add_sub_node_to_list(file_list *fileList ,file_list *subNode);

extern void init_root_file_list( void );

extern file_list * get_sys_root_file_node( void );

extern struct dev_list_t *get_glb_dev_list_t( void );

extern void add_dev_file_to_list(struct dev_list_t *devList, file_list * fileList);

extern void release_sys_root_file_tree( void );

extern void print_dev_file_list( void );

extern void add_dev_file_to_glb_list( file_list * fileList );

extern void add_dev_chip_num();

extern int get_dev_chip_num( void );

extern void release_dev_list_t( struct dev_list_t *dev_list_p);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif
