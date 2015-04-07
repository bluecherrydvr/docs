#ifndef __ISIL_FM_FDISK_H_
#define __ISIL_FM_FDISK_H_

#ifdef __cplusplus
extern "C" {
#endif

//define partion type
enum _eFSTYPE
{
    FS_TYPE_EXT3 = 0x83,//ext3
    FS_TYPE_FAT32= 0x01,//fat32
}ISIL_FM_FDISK_FS_TYPE;


#ifndef MAX_PATH_LEN

#define MAX_PATH_LEN (32)//设置设备节点及mount设备路径的最大长度

#endif

//设置分区类型
extern int ISIL_FM_DISK_SetPartionType(unsigned char PartionType);

//分区,目前只分一个区
extern int ISIL_FM_DISK_Fdisk(char SysType);

//建立文件系统
extern int ISIL_FM_DISK_Mkfs(char *strFsType);

//分区并建立文件系统
extern int ISIL_FM_DISK_Format();

//返回以KB为单位的值
extern unsigned int ISIL_FM_DISK_GetDiskCapacity();

//设置设备节点
extern int ISIL_FM_DISK_SetNodePath(char *NodePath, char *MntPath);

#ifdef __cplusplus
}
#endif
#endif



