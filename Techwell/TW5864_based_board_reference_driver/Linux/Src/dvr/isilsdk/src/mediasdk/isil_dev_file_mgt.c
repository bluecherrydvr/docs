#include <fcntl.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "isil_dev_file_st.h"
#include "isil_dev_file_mgt.h"


int  IsDir (char * name) {
   struct  stat  buff;  
   if (lstat(name,&buff)<0)
      return 0; //if not exist name ,ignore
   /*if is directory return 1 ,else return 0*/ 
   return S_ISDIR(buff.st_mode);
}

int  IsReg (char * name) {
   struct  stat  buff;  
   if (lstat(name,&buff)<0)
      return 0; //if not exist name ,ignore
   return S_ISREG(buff.st_mode);
}


static int get_dev_file_devNum(file_list *devFile)
{
    FILE* f;
    int major =0,minor = 0;
    int result = 0;

    f = fopen(devFile->file_name, "r");
    if (!f) {
        fprintf(stdout, "could not open %s\n",devFile->file_name);
        return -1;
    }

    result = fscanf(f, "%d:%d", &major,&minor);
    if(result != 2) {
        fprintf(stdout, "Read file  %s err\n",devFile->file_name);
        fclose(f);
        return -1;
    }

    if(major == 0) {
        fprintf(stdout, "Get  file  %s major[%c] err\n",devFile->file_name,major);
        fclose(f);
        return -1;
    }

    devFile->dev_major = major;
    devFile->dev_minor = minor;
    
    fclose(f);
    return 0;
}



int search_sys_video_node(file_list * parentNode)
{
    DIR         *directory;
   	struct      dirent * dir_entry ;

    if(parentNode == NULL) {
        fprintf(stderr,"Open parentNode err .\n");
        return -1;
    }

    if(!strlen(parentNode->file_name)) {
        fprintf(stderr,"get file path err .\n");
        return -1;
    }


    if ( (directory=opendir(parentNode->file_name) ) == NULL) {
       perror("Opendir:");
       return -1 ;
    }

    while ((dir_entry=readdir(directory)) != NULL) {
        if  (!strcmp(dir_entry->d_name,".")||!strcmp(dir_entry->d_name,"..")) {
                continue;
        }

        file_list * subNode = malloc_init_file_list();
        sprintf(subNode->file_name,"%s/%s",parentNode->file_name,dir_entry->d_name);
//        fprintf(stderr,"FILE[%s].\n",subNode->file_name);
        subNode->parent = parentNode;
        if(IsDir(subNode->file_name)) {
            if(subNode->parent->parent == NULL) {
                subNode->num_magic = get_dev_chip_num();
                add_dev_chip_num();
            }
            else{
                 subNode->num_magic = parentNode->num_magic;
            }

            if(strstr(dir_entry->d_name,ISIL_VIDEO_IN_SUBNAME)) {
                subNode->type = IS_VIDEO_IN_NODE;
            }
            else if(strstr(dir_entry->d_name,ISIL_VIDEO_OUT_SUBNAME)) {
                subNode->type = IS_VIDEO_OUT_NODE;
            }
            else if(strstr(dir_entry->d_name,ISIL_AUDIO_IN_SUBNAME)) {
                subNode->type = IS_AUDIO_IN_NODE;
            }
            else if(strstr(dir_entry->d_name,ISIL_AUDIO_OUT_SUBNAME)) {
                subNode->type = IS_AUDIO_OUT_NODE;
            }
            else if(strstr(dir_entry->d_name,ISIL_CONTROL_SUBNAME)) {
                subNode->type = IS_CODEC_NODE;
            }
            else if(strstr(dir_entry->d_name,ISIL_ENCODE_SUBNAME)) {
                free(subNode);
                subNode = NULL;
                continue;
            }
            else if(strstr(dir_entry->d_name,ISIL_DECODE_SUBNAME)) {
                free(subNode);
                subNode = NULL;
                continue;
            }
            
            add_sub_node_to_list(parentNode,subNode);
            search_sys_video_node(subNode);
        }
        else if(IsReg(subNode->file_name)) {
            if(!strstr(dir_entry->d_name,ISIL_DEV_NAME)) {
                free(subNode);
                subNode = NULL;
                continue;
            }
            add_sub_node_to_list(parentNode,subNode);
            subNode->type = parentNode->type;
            subNode->num_magic = parentNode->num_magic;
            if( get_dev_file_devNum(subNode) < 0){
                free(subNode);
                subNode = NULL;
                continue;
            }
            add_dev_file_to_glb_list(subNode);
        }
        else{
             free(subNode);
             subNode = NULL;
            continue;
        }
    }
    closedir(directory);
    return 0;
}

int init_sys_dev_inf_tree( void )
{
    file_list * rootNode;
    struct dev_list_t *dev_root_list;
    init_root_file_list();
    rootNode = get_sys_root_file_node();
    dev_root_list = get_glb_dev_list_t();
    if( search_sys_video_node(rootNode) < 0){
        fprintf(stderr,"search_sys_video_node failed .\n");
        return -1;
    }
    //print_dev_file_list();
    return 0;
}




#if 0

int main( int argc , char **argv)
{
    
    file_list * rootNode;
    struct dev_list_t *dev_root_list;
    init_root_file_list();
    rootNode = get_sys_root_file_node();
    dev_root_list = get_glb_dev_list_t();
    search_sys_video_node(rootNode);
    print_dev_file_list();
    release_sys_root_file_tree();
    return 0;
}

#endif







