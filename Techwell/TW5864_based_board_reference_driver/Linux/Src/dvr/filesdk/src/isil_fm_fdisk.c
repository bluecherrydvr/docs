#include <sys/types.h>
#include <sys/stat.h>           /* stat */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdint.h>        /* for uint32_t, uint16_t, uint8_t, int16_t, etc */
#include "isil_fm_fdisk.h"

#define MAX_PARTION 4
#define BLKGETSIZE _IO(0x12,96)    /* return device size */
#define BLKSSZGET  _IO(0x12,104)   /* get block device sector size */

#undef _IOR
#define _IOR(type,nr,size)      _IOC(_IOC_READ,(type),(nr),sizeof(size))
#define BLKGETSIZE64 _IOR(0x12,114,uint64_t)

#define HDIO_GETGEO             0x0301  /* get device geometry */

//fdisk
#define DEFAULT_SECTOR_SIZE     (512)

char DEV_NAME[MAX_PATH_LEN];
char MNT_PATH[MAX_PATH_LEN + 1];

struct hd_geometry
{
    unsigned char heads;
    unsigned char sectors;
    unsigned short cylinders;
    unsigned long start;
};

struct partition
{
	unsigned char boot_ind;         /* 0x80 - active */
	unsigned char head;             /* starting head */
	unsigned char sector;           /* starting sector */
	unsigned char cyl;              /* starting cylinder */
	unsigned char sys_ind;          /* What partition type */
	unsigned char end_head;         /* end head */
	unsigned char end_sector;       /* end sector */
	unsigned char end_cyl;          /* end cylinder */
	unsigned char start4[4];        /* starting sector counting from 0 */
	unsigned char size4[4];         /* nr of sectors in partition */
} __attribute__((__packed__));

static uint sector_size = DEFAULT_SECTOR_SIZE;
static uint heads, sectors, cylinders;
static int fd;/* the disk */
static int type_open = O_RDWR;
static unsigned long long total_number_of_sectors;

struct partition DptBuf[MAX_PARTION];//dpt buffer

#define hex_val(c)      ({ \
				char _c = (c); \
				isdigit(_c) ? _c - '0' : \
				tolower(_c) + 10 - 'a'; \
			})

#define sector(s)       ((s) & 0x3f)
#define cylinder(s, c)  ((c) | (((s) & 0xc0) << 2))

#define hsc2sector(h,s,c) (sector(s) - 1 + sectors * \
				((h) + heads * cylinder(s,c)))

#define set_hsc(h,s,c,sector) { \
				s = sector % sectors + 1;       \
				sector /= sectors;      \
				h = sector % heads;     \
				sector /= heads;        \
				c = sector & 0xff;      \
				s |= (sector >> 2) & 0xc0;      \
			}


static int32_t get_start_sect(const struct partition *p);
static int32_t get_nr_sects(const struct partition *p);

static unsigned char PARTION_TYPE = FS_TYPE_EXT3;


static int valid_part_table_flag(const unsigned char *b)
{
	return (b[510] == 0x55 && b[511] == 0xaa);
}

/* A valid partition table sector ends in 0x55 0xaa */
static unsigned int part_table_flag(const char *b)
{
	return ((uint) b[510]) + (((uint) b[511]) << 8);
}

static void
write_part_table_flag(char *b) {
	b[510] = 0x55;
	b[511] = 0xaa;
}

/* start_sect and nr_sects are stored little endian on all machines */
/* moreover, they are not aligned correctly */
static void store4_little_endian(unsigned char *cp, unsigned int val)
{
	cp[0] = (val & 0xff);
	cp[1] = ((val >> 8) & 0xff);
	cp[2] = ((val >> 16) & 0xff);
	cp[3] = ((val >> 24) & 0xff);
}

static unsigned int read4_little_endian(const unsigned char *cp)
{
	return (uint)(cp[0]) + ((uint)(cp[1]) << 8)
		+ ((uint)(cp[2]) << 16) + ((uint)(cp[3]) << 24);
}

static void set_start_sect(struct partition *p, unsigned int start_sect)
{
	store4_little_endian(p->start4, start_sect);
}

static int32_t get_start_sect(const struct partition *p)
{
	return read4_little_endian(p->start4);
}

static void set_nr_sects(struct partition *p, int32_t nr_sects)
{
	store4_little_endian(p->size4, nr_sects);
}

static int32_t get_nr_sects(const struct partition *p)
{
	return read4_little_endian(p->size4);
}


static int write_dpt(char *buf)
{
    if(lseek(fd, 446, SEEK_SET) < 0)
    {
        fprintf(stderr, "seek dpt fail\n\n");
        return -1;
    }

	if (write(fd, buf, sizeof(DptBuf)) != sizeof(DptBuf))
	{
        fprintf(stderr, "write dpt fail\n\n");
		return -1;
	}

    if(lseek(fd, (sector_size - 2), SEEK_SET) < 0)
    {
        fprintf(stderr, "seek end flag fail\n\n");
        return -1;
    }

    unsigned char flag[] = {0x55, 0xaa};

    if (write(fd, flag, 2) != 2)
	{
        fprintf(stderr, "write flag fail\n\n");
		return -1;
	}
    return 0;
}

static int open_dev(void)
{
    fd = open(DEV_NAME, type_open);
    if(fd < 0)
    {
        perror("fdisk open: ");
        fprintf(stderr, "open fail\n");
        return -1;
    }
    return fd;
}

static int close_dev(void)
{
    if(close(fd) < 0)
    {
        fprintf(stderr, "close fail\n");
        return -1;
    }
    return 0;
}

static inline void get_sectorsize(void)
{
    int arg;

    if(ioctl(fd, BLKSSZGET, &arg) == 0)
    {
        sector_size = arg;
    }

    if(sector_size != DEFAULT_SECTOR_SIZE)
    {
        fprintf(stderr, "Note: sector size is %d (not %d)\n",sector_size, DEFAULT_SECTOR_SIZE);
    }
}

static inline int  get_kernel_geometry(void)
{
	struct hd_geometry geometry;

	if (!ioctl(fd, HDIO_GETGEO, &geometry))
    {
		heads = geometry.heads;
		sectors = geometry.sectors;
		/* never use geometry.cylinders - it is truncated */
	}

    if(!heads || !sectors)
    {
        fprintf(stderr, "get kernel geometry fail\n");
        return -1;
    }

    return 0;
}

static int get_geometry(void)
{
	int sec_fac;
	unsigned long long bytes;       /* really u64 */

	get_sectorsize();
	sec_fac = sector_size / 512;
	heads = cylinders = sectors = 0;

	if(get_kernel_geometry() == -1)
	{
        return -1;
    }

	if (ioctl(fd, BLKGETSIZE64, &bytes) == 0)
    {
		/* got bytes */
	}
    else
    {
		unsigned long longsectors;

	    if (ioctl(fd, BLKGETSIZE, &longsectors))
	    {
		    longsectors = 0;
	    }

        bytes = ((unsigned long long) longsectors) << 9;
	}

	total_number_of_sectors = (bytes >> 9);
	//sector_offset = 1;
	cylinders = total_number_of_sectors / (heads * sectors * sec_fac);
    return 0;
}

//返回以KB为单位的值
unsigned int ISIL_FM_DISK_GetDiskCapacity()
{
    unsigned long long ullDiskCapacity = 0;

    if(open_dev() < 0)
    {
        fprintf(stderr, "open_dev fail\n");
        return -1;
    }

    if(get_geometry() < 0)
    {
        fprintf(stderr, "get_geometry fail\n");
        return -1;
    }

    close_dev();

    fprintf(stderr, "*******************Get Disk Info**************\n");
    fprintf(stderr, "head: %d, sector: %d, sector_size: %d, cylinders: %d\n", heads, sectors, sector_size, cylinders);
    fprintf(stderr, "totle sectors: %lld\n", total_number_of_sectors);

    ullDiskCapacity = (total_number_of_sectors * sector_size)>>10;

    if((ullDiskCapacity>>10) < 1024)//< 1G
    {
        fprintf(stderr, "----Get Disk Capacity : %lld(MB)---\n", (ullDiskCapacity>>10));
    }
    else
    {
        fprintf(stderr, "----Get Disk Capacity : %lld(GB)---\n", ((ullDiskCapacity>>20)));
    }

    return ullDiskCapacity;
}


/*填充分区表*/
/*输入分区类型及需要分区的总数目,最大4个分区，而且均为主分区*/
int fill_partion_table(unsigned char SysType)
{
    int i = 0;
    uint cly = (cylinders > 1024)? 1024:cylinders;
    uint start_sector = 1;
    uint end_sector = 0;

    start_sector = sectors;//保留一个head
    end_sector = cly * heads * sectors - 1;

    DptBuf[i].sys_ind = SysType;

    set_start_sect(&DptBuf[i], start_sector);

    set_nr_sects(&DptBuf[i], (cylinders*heads*sectors - start_sector));

    set_hsc(DptBuf[i].head, DptBuf[i].sector, DptBuf[i].cyl, start_sector);

    set_hsc(DptBuf[i].end_head, DptBuf[i].end_sector, DptBuf[i].end_cyl, end_sector);

    //fprintf(stderr, "dpt[%d] start h:%d,s:%d,c:%d\n", i , DptBuf[i].head, DptBuf[i].sector, DptBuf[i].cyl);
    //fprintf(stderr, "dpt[%d] end h:%d,s:%d,c:%d\n", i , DptBuf[i].end_head, DptBuf[i].end_sector, DptBuf[i].end_cyl);
    //fprintf(stderr, "%x, %x, %x, %x\n", DptBuf[i].start4[0], DptBuf[i].start4[1], DptBuf[i].start4[2], DptBuf[i].start4[3]);
    //fprintf(stderr, "%x, %x, %x, %x\n", DptBuf[i].size4[0], DptBuf[i].size4[1], DptBuf[i].size4[2], DptBuf[i].size4[3]);

    return 0;
}

int ISIL_FM_DISK_Fdisk(char PartitionType)
{
    fprintf(stderr, "---enter %s--\n", __FUNCTION__);
    if(open_dev() < 0)
    {
        fprintf(stderr, "open_dev fail\n");
        return -1;
    }

    if(get_geometry() < 0)
    {
        fprintf(stderr, "get_geometry fail\n");
        return -1;
    }

    if(fill_partion_table(PartitionType))
    {
        fprintf(stderr, "fill_partion_table fail\n");
        return -1;
    }

    if(write_dpt((char *)DptBuf) < 0)
    {
        fprintf(stderr, "write_dpt fail\n");
        return -1;
    }

    close_dev();

    return 0;
}

#define MAX_LIMITED_NUM (3)

int ISIL_FM_DISK_Mkfs(char *strFsType)
{
    //int value ,ret = 0 ,count = 0;

    char path[64] = {};
    fprintf(stderr, "---enter %s--\n", __FUNCTION__);
    memset( &path ,0x00 ,64);
    sprintf(path, "umount %s", DEV_NAME);
    system(path);


    //count = 0;
    memset( &path ,0x00 ,64);
    sprintf(path, "mkfs.%s %s", strFsType, DEV_NAME);
    system(path);


    //count = 0;
    memset( &path ,0x00 ,64);
    sprintf(path, "mount %s %s", DEV_NAME, MNT_PATH);
    system(path);


    return 0;
}

int ISIL_FM_DISK_SetPartionType(unsigned char PartionType)
{
   return(PARTION_TYPE = PartionType);
}

int ISIL_FM_DISK_Format()
{
    char str[16] = {};

    fprintf(stderr, "---enter %s--\n", __FUNCTION__);
    if(ISIL_FM_DISK_Fdisk(PARTION_TYPE) < 0)
    {
        return -1;
    }

    switch(PARTION_TYPE)
    {
        case FS_TYPE_EXT3:
            strcpy(str, "ext3");
            break;

        case FS_TYPE_FAT32:
            strcpy(str, "vfat");
            break;

        default:
            fprintf(stderr, "unkown partion type\n");
            break;
    }

    if(ISIL_FM_DISK_Mkfs(str) < 0)
    {
        return -1;
    }
    return 0;
}


int ISIL_FM_DISK_SetNodePath(char *NodePath, char *MntPath)
{
    if(NodePath == NULL)
    {
        fprintf(stderr, "---NodePath is null---\n");
        return -1;
    }

    if(MntPath == NULL)
    {
        fprintf(stderr, "---MntPath is null---\n");
        return -1;
    }

    memset(DEV_NAME, 0x00, MAX_PATH_LEN);
    memcpy(DEV_NAME, NodePath, MAX_PATH_LEN);
    fprintf(stderr, "---NodePath: %s---\n", DEV_NAME);

    memset(MNT_PATH, 0x00, MAX_PATH_LEN);
    memcpy(MNT_PATH, MntPath, MAX_PATH_LEN);
    sprintf(MNT_PATH, "%s/", MNT_PATH);

    fprintf(stderr, "---MntPath: %s---\n", MNT_PATH);
    return 0;
}






