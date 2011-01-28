#include <linux/kernel.h>
#include <linux/module.h>

#include "solo6x10.h"

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 13))
static struct class *solo6x10_class = NULL;
#else
static struct class_simple *solo6x10_class = NULL;
#endif

#else

static struct list_head solo6x10_device_list;

#endif

int solo6x10_device_init(void)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 13))
	solo6x10_class = class_create(THIS_MODULE, "solo6x10");
#else
	solo6x10_class = class_simple_create(THIS_MODULE, "solo6x10");
#endif
	if(IS_ERR(solo6x10_class))
		return PTR_ERR(solo6x10_class);

#else
	INIT_LIST_HEAD(&solo6x10_device_list);
#endif

	return 0;
}

void solo6x10_device_exit(void)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 13))
		class_destroy(solo6x10_class);
#else
		class_simple_destroy(solo6x10_class);
#endif

#else
#endif
}

int solo6x10_chip_init(solo6x10_chip_t *solo6x10_chip, int id)
{
	struct SOLO6x10 *solo6x10 = container_of(solo6x10_chip, struct SOLO6x10, chip);
	int ret_val;
	solo6x10_chip->id = id;
	solo6x10_chip->device_id = solo6x10->pdev->device;
	sprintf(solo6x10_chip->name, "solo%04x-%d", solo6x10_chip->device_id, id);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
	ret_val = alloc_chrdev_region(&solo6x10_chip->dev_base, 0, SOLO6x10_NR_DEV, solo6x10_chip->name);
#else
	ret_val = devfs_register_chrdev(0, solo6x10_chip->name, NULL);
	if(ret_val > 0)
	{
		solo6x10_chip->dev_base = MKDEV(ret_val, 0);
		ret_val = 0;
	}
#endif

	return ret_val;
}

void solo6x10_chip_exit(solo6x10_chip_t *solo6x10_chip)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
	unregister_chrdev_region(solo6x10_chip->dev_base, SOLO6x10_NR_DEV);
#else
	devfs_unregister_chrdev(MAJOR(solo6x10_chip->dev_base), solo6x10_chip->name);
#endif
}

solo6x10_device_t *solo6x10_device_add(solo6x10_chip_t *solo6x10_chip,
	struct file_operations *fops, const char *name, int dev_offset, void *data)
{
	solo6x10_device_t *device;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
	struct cdev *cdev;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 26))
	struct device *class_dev;
#else
	struct class_device *class_dev;
#endif
	int retval;
#else
#endif

	device = &solo6x10_chip->device[dev_offset];
	sprintf(device->name, "solo%04x_%s%d", solo6x10_chip->device_id, name, solo6x10_chip->id);
	device->dev_num = solo6x10_chip->dev_base + dev_offset;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
	cdev = &device->cdev;

	cdev_init(cdev, fops);
	cdev->owner = THIS_MODULE;
	retval = cdev_add(cdev, device->dev_num, 1);
	if(retval)
		return ERR_PTR(retval);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27))
	class_dev = device_create(solo6x10_class, NULL, cdev->dev, NULL, device->name);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 26))
	class_dev = device_create_drvdata(solo6x10_class, NULL, cdev->dev, NULL, device->name);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 15))
	class_dev = class_device_create(solo6x10_class, NULL, cdev->dev, NULL, device->name);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 13))
	class_dev = class_device_create(solo6x10_class, cdev->dev, NULL, device->name);
#else
	class_dev = class_simple_device_add(solo6x10_class, cdev->dev, NULL, device->name);
#endif
	if(IS_ERR(class_dev))
	{
		cdev_del(cdev);
		return ERR_PTR(PTR_ERR(class_dev));
	}

	device->classs_device = class_dev;
#else
	device->devfs_handle = devfs_register(NULL, device->name, DEVFS_FL_DEFAULT,
		MAJOR(device->dev_num), MINOR(device->dev_num), S_IFCHR | S_IRWXU, fops, NULL);
	if(device->devfs_handle == NULL)
		return NULL;

	list_add(&device->lh, &solo6x10_device_list);
#endif

	device->data = data;

	return device;
}

void solo6x10_device_remove(solo6x10_device_t *device)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 26))
	device_destroy(solo6x10_class, device->cdev.dev);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 13))
	class_device_destroy(solo6x10_class, device->cdev.dev);
#else
	class_simple_device_remove(device->cdev.dev);
#endif
	cdev_del(&device->cdev);

#else

	list_del(&device->lh);
	devfs_unregister(device->devfs_handle);

#endif
}

solo6x10_device_t *inode_to_solo6x10_device(struct inode *inode)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
	return (solo6x10_device_t *)container_of((inode)->i_cdev, solo6x10_device_t, cdev);
#else
	solo6x10_device_t *device;
	list_for_each_entry(device, &solo6x10_device_list, lh)
	{
		if(device->dev_num == inode->i_rdev)
			return device;
	}

	return NULL;
#endif
}


