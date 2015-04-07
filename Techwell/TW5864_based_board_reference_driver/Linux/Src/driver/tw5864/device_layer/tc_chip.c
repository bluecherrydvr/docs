#include <isil5864/tc_common.h>


int test_open(struct isil_chip_device *tcd,struct file * file)
{
    int ret = 0;

    tc_trace;
    if(!tcd->ped || !tcd->ped->op)
        dbg("int %s have driver attached!\n",__FUNCTION__);

    ret = tcd->ped->op->open(tcd->ped);
    if(ret){
        printk("in %s driver open failed!\n", __FUNCTION__);
        return ret;
    }
    file->private_data = tcd;	
    return ret;
}


/* after attach in open, so we can retrive our driver data struct from file->private_data
 * we must release all the resources by this chip dev and his chidren service dev's
 */
int test_release(struct isil_chip_device *chip_dev, struct file *file)
{
    int ret;
    struct isil_chip_device *tcd = (struct isil_chip_device *)file->private_data;

    printk("--->function %s called\n",__FUNCTION__);
    if(!tcd->ped || !tcd->ped->op){
        printk("tcd->ped is NULL or ped->op is NULL\n");
        return -EINVAL;

    }
    ret = tcd->ped->op->close(tcd->ped);
    file->private_data = NULL;
    return ret;
}

int test_ioctl(struct file * filp,unsigned int cmd, unsigned long arg)
{
    struct isil_chip_device *tcd = (struct isil_chip_device *)filp->private_data;
    int ret = 0;

    if(!tcd->ped || !tcd->ped->op){
        printk("tcd->ped is NULL or ped->op is NULL\n");
        return -EINVAL;
    }

    down(&tcd->sem);
    ret = tcd->ped->op->ioctl(tcd->ped,cmd,arg);
    up(&tcd->sem);
    return ret;
}


struct tcd_file_operations  test_fop = {
    .open = test_open,
    .release = test_release,
    .ioctl = test_ioctl,
};






int  tc_chip_init(struct isil_chip_device *tcd,unsigned long bus_id,unsigned long chip_id)
{
    int ret = 0;
    epkey_t key = {0};
    unsigned long idx = 0;

    dbg("@@@@:bus_id[%lu];chip_id[%lu]\n",bus_id,chip_id);
    for(;idx < 1; idx++){
		sprintf(tcd->name,"%s-%02lu","tw5864",chip_id);
		make_key((epkey_t *)&key,0x3f,0x3f,bus_id,0,chip_id,0);
		epobj_init(&tcd->epobj, ISIL, bus_id,TW5864,0,0,&key);
		tcd->tcd_fops = &test_fop;
		ret = register_chip_device(tcd);
		if(ret){
			printk("regisrter chip dev failed!\n");
			return ret;
		}
	}
	
        return ret;
}




void  tc_chip_exit(struct isil_chip_device *tcd)
{
    unregister_chip_device(tcd);
}





