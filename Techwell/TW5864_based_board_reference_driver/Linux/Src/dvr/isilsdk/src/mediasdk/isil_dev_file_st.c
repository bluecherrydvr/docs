#include <assert.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "isil_dev_file_st.h"

static file_list isil_sys_root_node;

static struct dev_list_t glb_dev_list_t;

static void release_file_node(file_list *fileList)
{
    struct list_head *ln1, *ln2;
    file_list *subNode = NULL;
    if( !fileList ) {
        return ;
    }
    if(!list_empty(&fileList->sub_file_list)){
        list_for_each_safe(ln1, ln2, &fileList->sub_file_list){
            subNode = list_entry(ln1,file_list,entry);
            if(subNode) {
                if(subNode->release) {
                    
                    subNode->release(subNode);
                }
                list_del_init(&subNode->entry);
                free(subNode);
                subNode = NULL;
            }
            
        }
    }
}

struct dev_list_t *get_glb_dev_list_t( void )
{
    return &glb_dev_list_t;
}


void add_dev_file_to_list(struct dev_list_t *devList, file_list * fileList)
{
    int i;
    for(i = 0 ; i < DEV_FILE_LIST_MAX_NUM; i++) {

        if(devList->dev_file_list[i] == NULL) {
            devList->dev_file_list[i] = fileList;
            devList->num++;
            return;
        }
    }
}

void add_dev_file_to_glb_list( file_list * fileList )
{
    struct dev_list_t *devList = get_glb_dev_list_t();
    add_dev_file_to_list(devList ,fileList);
}

void add_dev_chip_num( void )
{
    struct dev_list_t *devList = get_glb_dev_list_t();
    devList->chipNum++;
}

int get_dev_chip_num( void )
{
     struct dev_list_t *devList = get_glb_dev_list_t();
     return devList->chipNum;
}


file_list * malloc_init_file_list( void )
{
    file_list *tmpNode;
    tmpNode = (file_list *)malloc(sizeof(file_list));
    assert(tmpNode);
    memset(tmpNode , 0x00 ,sizeof(*tmpNode));
    INIT_LIST_HEAD(&tmpNode->entry);
	INIT_LIST_HEAD(&tmpNode->sub_file_list);	
    tmpNode->num_magic = -1;
    tmpNode->type            = 0;
    tmpNode->release = release_file_node;
    return tmpNode;
}



void add_sub_node_to_list(file_list *fileList ,file_list *subNode)
{
    if( !fileList || !subNode) {
        return ;
    }
    list_add_tail(&subNode->entry, &fileList->sub_file_list);
}

void init_root_file_list( void )
{
    
    INIT_LIST_HEAD(&isil_sys_root_node.entry);
	INIT_LIST_HEAD(&isil_sys_root_node.sub_file_list);	
    isil_sys_root_node.num_magic = -1;
    isil_sys_root_node.type            = 0;
    isil_sys_root_node.release = release_file_node;
    strcpy(isil_sys_root_node.file_name,ISIL_SYS_ROOT_PATH);
    isil_sys_root_node.parent = NULL;
}


file_list * get_sys_root_file_node( void )
{
    return &isil_sys_root_node;
}

void release_sys_root_file_tree( void )
{
    file_list * rootTree = get_sys_root_file_node();
    rootTree->release(rootTree);
    memset(rootTree,0x00,sizeof(file_list));
}

void release_dev_list_t( struct dev_list_t *dev_list_p)
{
    int i;
    dev_list_p->num = 0;
    for( i = 0 ; i < DEV_FILE_LIST_MAX_NUM ;i++) {
        dev_list_p->dev_file_list[i] = NULL;
    }
}


void print_dev_file_list( void )
{
    int i;
    file_list *tmpNode;
    struct dev_list_t *devList =get_glb_dev_list_t();
    for(i = 0 ; i < devList->num ; i++) {
        tmpNode = devList->dev_file_list[i];
        if( tmpNode ) {
            fprintf(stderr,"Node: ---fileName[%s]\n,\n\tnum[%u],\n\ttype[%d],\n\tmajor[%u],\n\tminor[%u] .\n",
                    tmpNode->file_name,
                    tmpNode->num_magic,
                    tmpNode->type,
                    tmpNode->dev_major,
                    tmpNode->dev_minor);
        }
    }
}











