#ifndef __SOLO6010_DEVICE_H
#define __SOLO6010_DEVICE_H

#include <linux/version.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
#include <linux/cdev.h>
#else
#include <linux/devfs_fs_kernel.h>
#endif

#define SOLO6010_NR_DEV				5

#define SOLO6010_DISP_DEV_OFFSET	0
#define SOLO6010_VIN_DEV_OFFSET		1
#define SOLO6010_ENC_DEV_OFFSET		2
#define SOLO6010_DEC_DEV_OFFSET		3
#define SOLO6010_G723_DEV_OFFSET	4

#define SOLO6010_NAME_LEN	32

typedef struct {
	char name[SOLO6010_NAME_LEN];
	dev_t dev_num;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
	struct cdev cdev;
	struct class_device *classs_device;
#else
	struct list_head lh;
	devfs_handle_t devfs_handle;
#endif
	void *data;
} solo6010_device_t;

typedef struct {
	int id;
	char name[SOLO6010_NAME_LEN];
	dev_t dev_base;
	solo6010_device_t device[SOLO6010_NR_DEV];
} solo6010_chip_t;

int solo6010_chip_init(solo6010_chip_t *solo6010_chip, int id);
void solo6010_chip_exit(solo6010_chip_t *solo6010_chip);

#define solo6010_device_data_get(device)		((device)->data)

int solo6010_device_init(void);
void solo6010_device_exit(void);
solo6010_device_t *solo6010_device_add(solo6010_chip_t *solo6010_chip,
	struct file_operations *fops, const char *name, int dev_offset, void *data);
void solo6010_device_remove(solo6010_device_t *device);

solo6010_device_t *inode_to_solo6010_device(struct inode *inode);

#endif
