#ifndef __SOLO6x10_DEVICE_H
#define __SOLO6x10_DEVICE_H

#include <linux/version.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
#include <linux/cdev.h>
#else
#include <linux/devfs_fs_kernel.h>
#endif

#define SOLO6x10_NR_DEV				5

#define SOLO6x10_DISP_DEV_OFFSET	0
#define SOLO6x10_VIN_DEV_OFFSET		1
#define SOLO6x10_ENC_DEV_OFFSET		2
#define SOLO6x10_DEC_DEV_OFFSET		3
#define SOLO6x10_G723_DEV_OFFSET	4

#define SOLO6x10_NAME_LEN	32

typedef struct {
	char name[SOLO6x10_NAME_LEN];
	dev_t dev_num;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
	struct cdev cdev;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 26))
	struct device *classs_device;
#else
	struct class_device *classs_device;
#endif
#else
	struct list_head lh;
	devfs_handle_t devfs_handle;
#endif
	void *data;
} solo6x10_device_t;

typedef struct {
	int id;
	u16 device_id;
	char name[SOLO6x10_NAME_LEN];
	dev_t dev_base;
	solo6x10_device_t device[SOLO6x10_NR_DEV];
} solo6x10_chip_t;

int solo6x10_chip_init(solo6x10_chip_t *solo6x10_chip, int id);
void solo6x10_chip_exit(solo6x10_chip_t *solo6x10_chip);

#define solo6x10_device_data_get(device)		((device)->data)

int solo6x10_device_init(void);
void solo6x10_device_exit(void);
solo6x10_device_t *solo6x10_device_add(solo6x10_chip_t *solo6x10_chip,
	struct file_operations *fops, const char *name, int dev_offset, void *data);
void solo6x10_device_remove(solo6x10_device_t *device);

solo6x10_device_t *inode_to_solo6x10_device(struct inode *inode);

#endif
