#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/init.h>
#include <linux/idr.h>
#include <linux/seq_file.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/completion.h>
#include <asm/uaccess.h>
#include <linux/kthread.h>
#include <linux/unistd.h>
#include <asm/unistd.h>
#include <linux/unistd.h>
#include <linux/fcntl.h>
#include <linux/namei.h>
#include <linux/radix-tree.h>
#include <linux/sysfs.h>
#include <asm/current.h>
#include <isil5864/tc_common.h>
//#include <linux/kprobes.h>

MODULE_LICENSE("GPL");

static struct kobject * kroot = NULL;
static LIST_HEAD(isil_chipdev);
static DEFINE_MUTEX(mutex_chipdev);

static LIST_HEAD(isil_chipdrv);
static DEFINE_MUTEX(mutex_chipdrv);

static LIST_HEAD(isil_tc_dev);
static DEFINE_MUTEX(mutex_tc_dev);

static LIST_HEAD(isil_tc_drv);
static DEFINE_MUTEX(mutex_tc_drv);

//static RAW_NOTIFIER_HEAD(isil_chipdev_chain);
//struct class *my_class;
//static struct radix_tree_root look_tb = RADIX_TREE_INIT(GFP_KERNEL);



static int init_nfm_bitmap(void);
static void free_nfm_bitmap(void);
static int get_free_minor(struct cmd_arg *arg);
static void put_free_minor(unsigned long minor,unsigned long type);
static void  make_service_key(epkey_t *key,struct isil_chip_device * tcd, struct cmd_arg * arg);
//static void isil_dir_release(struct kobject * kobj);
static struct kobject * isil_create_dir(char * name, struct kobject *parent);
static void isil_remove_dir(struct kobject * kobj);
static int bind_chip_driver(struct isil_chip_device * tcd);
static void unbind_chip_driver(struct isil_chip_device * tcd);

/*
static void chip_dev_get(struct isil_chip_device * tcd);
static void chip_dev_put(struct isil_chip_device * tcd);
static void service_dev_get(struct isil_service_device * tsd);
static void service_dev_put(struct isil_service_device * tsd);
*/
static int  get_dir(struct isil_chip_device *tcd, unsigned long type, struct kobject **dir);
//static struct tsd_file_operations * lookup_tsd_dev_fops_from_dev(struct isil_service_device *tsd);
static int request_devno_resource(void);
static void free_devno_resource(void);

struct tc_hb_pool *thb_pool = NULL;
char *algorithm[] = {
	"h264","mjpg","alaw","ulaw","abadpcm",
	"mpmlq","adpcm","ldcecp","csacelp",
};
char *stream[] ={"major","minor","pic","mv","audio",};
char *tc_dev_name[] = {"isil_chip","isil_encoder","isil_decoder","isil_video_in","isil_video_out","isil_audio_in","isil_audio_out",};





static LIST_HEAD(cl_head);
static unsigned long resource_major[MAX_SERVICE_NR];

/*this indicate the minor numbers which we can use, 0 is free, 1 is using*/
static unsigned long *nfm_bitmap[MAX_SERVICE_NR];


void lock_chipdev(void)
{
    mutex_lock(&mutex_chipdev);
}


void unlock_chipdev(void)
{
    mutex_unlock(&mutex_chipdev);
}


struct list_head * get_chipdev_listhead(void)
{
    return &isil_chipdev;
}


struct inode * tc_get_inode_from_file(struct file *f)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 20)
    return f->f_dentry->d_inode;
#else
    return f->f_path.dentry->d_inode;
#endif

}







static int init_nfm_bitmap(void)
{
    int idx;
    unsigned long len = tc_round_up(MAX_MINOR_NR,8);


    for(idx=0; idx<MAX_SERVICE_NR; idx++)
    {
        nfm_bitmap[idx] = kzalloc(len,GFP_KERNEL);
        if(!nfm_bitmap[idx])  goto clean;
    }
    return 0;
clean:
    dbg("in %s failed to get bitmap space!\n",__FUNCTION__);
    for(;idx > 0; idx--)
        kfree(nfm_bitmap[idx-1]);
    return -1;
}

static void free_nfm_bitmap(void)
{
    unsigned long i;

    for(i=0; i<MAX_SERVICE_NR; i++)
        kfree(nfm_bitmap[i]);

}


/*get the free minor bumber to use*/
static int get_free_minor(struct cmd_arg *arg)
{
    int ret = MAX_MINOR_NR;
    int idx = arg->type;

    ret = find_first_zero_bit(nfm_bitmap[idx],MAX_MINOR_NR);
    if(ret >= MAX_MINOR_NR){
        dbg("there is no free minor!\n");
        return -1;
    }
    //dbg("the first zero bit is %d\n",ret);
    set_bit(ret,nfm_bitmap[idx]);
    return ret;
}

/*return the minor number to free*/
static void put_free_minor(unsigned long minor,unsigned long type)
{	
    clear_bit(minor,nfm_bitmap[type]);
}


static int tc_major_2_type(unsigned long major)
{
    int i;

    for(i = 0; i < MAX_SERVICE_NR; i++){
        if(resource_major[i] == major)
        	return i;
    }
    return -1;

}


static void make_service_key(epkey_t *key ,struct isil_chip_device * tcd, struct cmd_arg * arg)
{
    int bus_id = get_bus_id(tcd->epobj.key);
    int chip_id = tcd->minor;
    int pid = 0;
    int lid = arg->channel_idx;	

    make_key(key,arg->stream,arg->algorithm,bus_id,chip_id, pid, lid);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 25)  
static void isil_dir_release(struct kobject * kobj)
{
    kfree(kobj);
}

static struct kobj_type  isil_dir = {
    .release        = isil_dir_release,
    .sysfs_ops      = NULL,
    .default_attrs  = NULL,
}; 
#endif




static struct kobject * isil_create_dir(char * name, struct kobject *parent)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 25)
    int ret;     
    struct kobject* kobj = kzalloc(sizeof(struct kobject),GFP_ATOMIC);

    if(!kobj)
        return NULL;

    kobj->parent = parent;
    kobj->ktype = &isil_dir;
    kobject_init(kobj);
    kobject_set_name(kobj, "%s", name);
    ret = kobject_register(kobj);
    if(ret){
        kobject_del(kobj);
        return NULL;
    }
    else
    {
        return kobj;
    }
#else

    return kobject_create_and_add(name,parent);
#endif
}


static void isil_remove_dir(struct kobject * kobj)
{
    if(!kobj)
        return;

    //tc_trace;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 25)
    kobject_unregister(kobj);
#else
    kobject_put(kobj);
#endif
} 



static int isil_device_match(struct device *dev, struct device_driver *drv)
{
    //struct w_chip_device *tcd = container_of(dev, struct isil_chip_device, dev);
    //struct isil_chip_driver *tcdr = container_of(drv, struct isil_chip_driver, driver);

    return 0;// strcmp(tcd->name, drv->name) == 0;
}


static ssize_t show_chip_name(struct device *dev, struct device_attribute *attr, char *buf)
{
    // struct  isil_chip_device *tcd = container_of(dev,struct  isil_chip_device, dev);

    return 0;//sprintf(buf, "%s\n", tcd->name);
}


static struct device_attribute isil_dev_attrs[] = {
    __ATTR(name, S_IRUGO, show_chip_name, NULL),
    { },
};


static struct bus_type isil_virtual_bus = {
    .name		= "isil_virtual_bus",
    .dev_attrs	= isil_dev_attrs,
    .match		= isil_device_match,
    //.shutdown	= isil_device_shutdown,
    //.suspend	= isil_device_suspend,
    //.resume		= isil_device_resume,
};

/*
   static void chip_dev_get(struct isil_chip_device * tcd)
   {
   get_device(&tcd->dev);
   }


   static void chip_dev_put(struct isil_chip_device * tcd)
   {
   put_device(&tcd->dev);
   }

   static void service_dev_get(struct isil_service_device * tsd)
   {
   get_device(&tsd->dev);
   }


   static void service_dev_put(struct isil_service_device * tsd)
   {
   put_device(&tsd->dev);
   }
   */

struct isil_chip_device * find_chip_dev(int minor)
{
    struct list_head * p = &isil_chipdev;
    struct isil_chip_device *tcd = NULL;

    if((p->next == p->prev) && (p->next == &isil_chipdev))
    {
        dbg("out from here? and minor=%d\n",minor);
        return NULL;
    }
    mutex_lock(&mutex_chipdev);
    p = p->next;
    for(; p != &isil_chipdev; p = p->next)
    {
        tcd = container_of(p,struct isil_chip_device, list);
        if(tcd->minor == minor){
            mutex_unlock(&mutex_chipdev);
            return tcd;
        }
    }

    mutex_unlock(&mutex_chipdev);
    return NULL;
}


int twc_open(struct inode *inode, struct file *file)
{
    unsigned int minor = iminor(inode);
    int ret = 0;
    struct isil_chip_device *tcd;

    //tc_trace;
    if(minor >= 256)
        return -ENODEV;

    tcd = find_chip_dev(minor);
    if(tcd == NULL)
        return -ENODEV;
    /*	if(atomic_read(&tcd->state) >= ISIL_DEV_IDLE){
        dbg("the dev [0x%08X] have beed opend!\n",tcd);
    //file->private_data = tcd;	
    return 0;
    }
    */		
    if(tcd->tcd_fops && tcd->tcd_fops->open)
        ret = tcd->tcd_fops->open(tcd,file);

    //dbg("opened and ret = %d\n",ret);
    /*	if(!ret)
        atomic_set(&tcd->state, ISIL_DEV_IDLE);
        */
    return ret;
}


int twc_release(struct inode *inode, struct file *filp)
{
    unsigned int minor = iminor(inode);
    int ret = 0;
    struct isil_chip_device *tcd;

    //tc_trace;
    if(minor >= 256)
        return -ENODEV;

    //dbg("minor = %d\n",minor);
    tcd = find_chip_dev(minor);
    if(!tcd)
        return -ENODEV;
    /*	
        if(atomic_read(&tcd->state) <= ISIL_DEV_CLOSED){
        dbg("the dev [0x%08X] have beed closed!\n",tcd);
        return 0;
        }	
        */	
    if(tcd->tcd_fops && tcd->tcd_fops->release)
        tcd->tcd_fops->release(tcd,filp);

    //	atomic_set(&tcd->state,ISIL_DEV_CLOSED);
    return ret;	

}

ssize_t twc_read(struct file *filp, char *buff, size_t count, loff_t *offp)
{
    ssize_t ret;
    struct isil_chip_device * tcd;

    tcd = find_chip_dev(iminor(tc_get_inode_from_file(filp)));
    if(!tcd)
        return -ENODEV;
    //	atomic_set(&tcd->state,ISIL_DEV_BUSY);
    //dbg("in %s---->\n",__FUNCTION__);
    if(!tcd->tcd_fops->read)
        return -EINVAL;

    ret = tcd->tcd_fops->read(filp,buff,count,offp);
    /*	if(ret >=0 )
        atomic_set(&tcd->state,ISIL_DEV_IDLE);
        else
        atomic_set(&tcd->state,ISIL_DEV_BROKEN);
        */	return ret;
}


ssize_t twc_write(struct file *filp,  const char __user *buff, size_t count, loff_t *offp)
{
    ssize_t ret;
    struct isil_chip_device * tcd;

    tcd = find_chip_dev(iminor(tc_get_inode_from_file(filp)));
    if(!tcd)
        return -ENODEV;

    //dbg("in %s---->\n",__FUNCTION__);
    //	atomic_set(&tcd->state,ISIL_DEV_BUSY);
    if(!tcd->tcd_fops->write)
        return -EINVAL;

    ret = tcd->tcd_fops->write(filp,buff,count,offp);
    /*	if(ret >= 0)
        atomic_set(&tcd->state,ISIL_DEV_IDLE);
        else
        atomic_set(&tcd->state,ISIL_DEV_BROKEN);
        */	return ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
long twc_ioctl(struct file * filp,unsigned int cmd, unsigned long arg)
#else
int twc_ioctl(struct inode * inode,struct file * filp,unsigned int cmd, unsigned long arg)
#endif
{
    int ret = 0;
    struct isil_chip_device *tcd;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
    tcd = find_chip_dev(iminor(filp->f_path.dentry->d_inode));
#else
    tcd = find_chip_dev(iminor(inode));
#endif

    if(!tcd){
        printk("can not find the tcd by minor number!!!!!!\n");
        return -ENODEV;
    }

    if(!tcd->tcd_fops  || !tcd->tcd_fops->ioctl){
        printk("you have not ioctl func registered!\n");
        return -EINVAL;
    }
    /*	
        if(copy_from_user(kargbuf,(void __user *)arg, _IOC_SIZE(cmd) )){
        ret = -EFAULT;
        goto out;
        }
        */
    switch(cmd)
    {
        case ISIL_CHIP_CREATE_CHAN:
            printk("will create channel!\n");
            ret = create_service_device(tcd,arg);
            break;
        case ISIL_CHIP_RELEASE_CHAN:
            printk("will release channel!");
            //dump_stack();
            release_service_device(tcd,(void *)arg);
            break;
       default:
            //dbg("other cmd %x\n",cmd);
            ret = tcd->tcd_fops->ioctl(filp,cmd,arg);
            break;
    }

    return ret;
}

struct file_operations isil_fops = {
    .owner = THIS_MODULE,
    .open = twc_open,
    .release = twc_release,
    .read = twc_read,
    .write = twc_write,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
    .unlocked_ioctl = twc_ioctl,
#else
    .ioctl = twc_ioctl,
#endif
};








struct isil_service_device * find_service_dev(int major,int minor)
{
    struct list_head  *p = &isil_chipdev;
    struct list_head  *q = NULL;
    int type; //= major-ISIL_SERVICE_BASE_MAJOR;
    struct isil_chip_device * tcd = NULL;
    struct isil_service_device *tsd = NULL;


    if((p->next == p->prev) && (p->next == &isil_chipdev))
    {
        dbg("nothing in device list!\n");
        return NULL;
    }

    type = tc_major_2_type(major);
    if(type <= 0 || type > MAX_SERVICE_NR - 1){
    	dbg("wrong major in %s\n", __FUNCTION__);
    	return NULL;
    }
    type = type - 1;//decrease the major index, because of the chip dev
    mutex_lock(&mutex_chipdev);
    p = p->next;
    for(; p != &isil_chipdev; p = p->next){
        tcd = container_of(p,struct isil_chip_device, list);
        if((q = &tcd->tsd_list[type]) == NULL)	
            goto error;

        q = q->next;
        for(; q != &tcd->tsd_list[type]; q = q->next){
            tsd = container_of(q,struct isil_service_device,list);
            if(tsd->minor == minor)
                goto find;
        }
    }

find:
    mutex_unlock(&mutex_chipdev);
    return tsd;
error:
    dbg("BUG! the listhead of given type is NULL!\n");
    mutex_unlock(&mutex_chipdev);
    return NULL;

}


int tws_open(struct inode *inode, struct file *file)
{
    int major = imajor(inode);
    int minor = iminor(inode);
    int ret = 0;
    struct isil_service_device *tsd = NULL;

    printk("in %s major:[%d]; minor:[%d]\n",__FUNCTION__,major,minor);
    tsd = find_service_dev(major,minor);
    if(tsd == NULL)
        return -ENODEV;

    /*	if(atomic_read(&tsd->state) >= ISIL_DEV_IDLE){
        dbg("the dev [0x%08X] have beed opend!\n",tsd);
        return 0;
        }
        */	if(!(tsd->tsd_fops)){
            dbg("you have no tsd_fops registered!\n");
            return -EINVAL;
        }
        if(!tsd->tsd_fops->open){
            dbg("BUG: have no open function");
            return -EINVAL;
        }

        dbg("major[%d],minor[%d]\n",major,minor);
        ret = tsd->tsd_fops->open(tsd,file);

        return ret;
}


int tws_release(struct inode *inode, struct file *filp)
{
    int ret = 0;
    int major = imajor(inode);
    int minor = iminor(inode);
    struct isil_service_device *tsd = NULL;

    printk("in %s major = %d; minor = %d\n",__FUNCTION__,major,minor);
    tsd = find_service_dev(major,minor);
    if(!tsd){
        printk("in %s can not find the tsd!!!!!!!!\n",__FUNCTION__);
        return -ENODEV;
    }

    /*
       if(atomic_read(&tsd->state) <= ISIL_DEV_CLOSED){
       dbg("the dev [0x%08X] have beed closed!\n",tsd);
       return 0;
       }	
       */
    if(!tsd->tsd_fops || !tsd->tsd_fops->release){
        dbg("no tsd_fops or release\n");
        return 0;
    }
    ret = tsd->tsd_fops->release(filp);
    /*	if(!ret){
        dbg("close ok\n");
        atomic_set(&tsd->state, ISIL_DEV_CLOSED);
        }
        */	return ret;	
}

ssize_t tws_read(struct file *filp, char *buff, size_t count, loff_t *offp)
{
    int ret = 0;
    struct isil_service_device * tsd = (struct isil_service_device *)filp->private_data; /*the tsd goes first in different dev structs*/	

    if(filp->private_data == NULL){
        dbg("error: you get call %s without opened!\n",__FUNCTION__);
        return -EINVAL;
    }

    if(NULL == tsd->tsd_fops){
        dbg("no tsd fop registered!\n");
        return -EINVAL;
    }

    if(tsd->tsd_fops->read)
        ret = tsd->tsd_fops->read(filp,buff,count,offp);					
    return ret;
}

ssize_t tws_write(struct file *filp, const char *buff, size_t count, loff_t *offp)
{
    int ret = 0;
    struct isil_service_device * tsd = (struct isil_service_device *)filp->private_data; /*the tsd goes first in different dev structs*/	

    if(filp->private_data == NULL){
        dbg("error: you get call %s without opened!\n",__FUNCTION__);
        return -EINVAL;
    }

    if(NULL == tsd->tsd_fops){
        dbg("no tsd fop registered!\n");
        return -EINVAL;
    }

    if(tsd->tsd_fops->write)
        ret = tsd->tsd_fops->write(filp,buff,count,offp);					


    return ret;


}


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
long tws_ioctl(struct file * filp,unsigned int cmd, unsigned long arg)
#else
int tws_ioctl(struct inode * inode,struct file * filp,unsigned int cmd, unsigned long arg)
#endif
{
    int ret; 
    struct isil_service_device * tsd = (struct isil_service_device *)filp->private_data;	

    if(filp->private_data == NULL){
        dbg("call %s without opened!\n",__FUNCTION__);
        return -EINVAL;
    }
    if(!tsd->tsd_fops || !tsd->tsd_fops->ioctl){
        dbg("no tsd fop registered!\n");
        return -EINVAL;
    }
    ret = tsd->tsd_fops->ioctl(filp,cmd,arg);
    //if(ret)
    //        dbg("@@@@@@ioctl failed@@@@@@ return[%d]\n",ret);
    //dbg("return value [%d]\n",ret);
    return ret;
}


unsigned int tws_poll(struct file *file, struct poll_table_struct *wait)
{
    unsigned int ret = 0;
    struct isil_service_device * tsd = (struct isil_service_device *)file->private_data; /*the tsd goes first in different dev structs*/	

    if(NULL == file->private_data){
        dbg("call %s without opened!\n",__FUNCTION__);
        return -EINVAL;
    }
    if(NULL == tsd->tsd_fops){
        dbg("no tsd fop registered!\n");
        return -EINVAL;
    }

    if(!tsd->tsd_fops->poll){
        printk("no poll function!\n");
        return ret;
    }
    //printk("minor = %d\n",tsd->minor);
    ret = tsd->tsd_fops->poll(file,wait);
    //printk("%s return value [%u] [%d]\n",__FUNCTION__,ret,ret);
    return ret;
}






#if 0
void tc_vm_open(struct vm_area_struct *vma)
{
    //struct tc_mapping *map = vma->vm_private_data;

    dbg("vm_open virtual:%X phy:%d\n",vma->vm_start,vma->vm_pgoff << PAGE_SHIFT);//%p [count=%d,vma=%08lx-%08lx]\n",map,map->count,vma->vm_start,vma->vm_end);
    //map->count++;
}

void tc_vm_close(struct vm_area_struct *vma)
{
    dbg("vm_close \n");//[count=%d,vma=%08lx-%08lx]\n",map,map->count,vma->vm_start,vma->vm_end);
    return;
}


struct page * tc_vm_nopage(struct vm_area_struct *vma, unsigned long vaddr,int *type)
{
    struct page *page = NOPAGE_SIGBUS;//NOPAGE_SIGBUS=NULL
    struct isil_audio_en *tae = vma->vm_private_data;
    unsigned long off = 0;
    struct tc_queue *q = tae->queue;
    struct tc_buffer_kern * tck = q->bufs[q->rp];

    dbg("nopage: fault @ %08lx [vma %08lx-%08lx]\n",vaddr,vma->vm_start,vma->vm_end);
    if (vaddr > vma->vm_end)
        return NOPAGE_SIGBUS;

    /* get pages(the low level driver handled specified) here!*/	
    off = (vaddr - vma->vm_start) + (vma->vm_pgoff << PAGE_SIZE);
    if(off > 4096*3){
        dbg("%s---------> out og range!\n",__FUNCTION__);
        return page;
    }

    dbg("%s--->off = %d\n",__FUNCTION__,off);
    mutex_lock(&tae->queue->lock);
    page = virt_to_page(tck->start);
    dbg("give the %d tck ;tck->off_to_page = %d\n",q->rp,tck->off_to_page);
    mutex_unlock(&tae->queue->lock);

    dbg("\n tck's page = %X;tc->start = %X and tck->off_to_page = %d\n",page,tck->start,tck->off_to_page);
    if(!page)
        return NOPAGE_OOM;
    get_page(page);
    if (type)
        *type = VM_FAULT_MINOR;
    return page;
}
struct vm_operations_struct tc_vm_ops =
{
    .open     = tc_vm_open,
    .close    = tc_vm_close,
    .nopage   = NULL,//tc_vm_nopage,
};

#endif


int mmap_scatter(struct vm_area_struct *vma,struct tc_queue *tq, unsigned long size)
{
    int ret=0;
    int idx = 0,cix = 0;
    unsigned long start,pfn;
    unsigned long cn = 0;
    struct tc_buffer_kern *tck;

    //tc_trace;
    start = vma->vm_start;
    tck = tq->bufs[0];

    /*first map header_buf*/
    if(!tck->thb || !tck->thb->data){
        dbg("tck has no header buf or thb->data is NULL\n");
        return -EINVAL;
    }
    pfn = (__pa(tck->thb->data)) >> PAGE_SHIFT;
    if(remap_pfn_range(vma,start,pfn,PAGE_SIZE,PAGE_SHARED)){
        dbg("remap header_buf failed\n");
        return -ENXIO;
    }
    start += PAGE_SIZE;


    /*then net payload...*/
    while((tck = tq->bufs[cix]) && (STATE_READY == get_tck_state(tck))){
        cn = tc_round_up(tck->size,PAGE_SIZE);
        set_tck_state(tck,STATE_BUSY);
        for(idx = 0; idx < cn; idx++){
            if(!tck->sg[idx].carrier || !tck->sg[idx].addr || !tck->sg[idx].len)
                break;
            pfn = (__pa(tck->sg[idx].addr)) >> PAGE_SHIFT;
            if(remap_pfn_range(vma,start,pfn,PAGE_SIZE,PAGE_SHARED)){
                dbg("remap page range failed\n");
                return -ENXIO;
            }			
            start += PAGE_SIZE;
        }
        cix++;
        if(cix >= tq->buf_nr)
            break;
    }
    return ret;
}

int mmap_single(struct vm_area_struct *vma,struct tc_queue *tq)
{
    unsigned long pfn,last_pfn = 0,start;
    struct tc_buffer_kern *tck = tq->bufs[0];
    int idx = 0;

    start = vma->vm_start;

    if(!tck->thb || !tck->thb->data){
        dbg("tck has no header buf or thb->data is NULL\n");
        return -EINVAL;
    }
    /*first map header_buf*/
    pfn = (__pa(tck->thb->data)) >> PAGE_SHIFT;
    if(remap_pfn_range(vma,start,pfn,PAGE_SIZE,PAGE_SHARED)){
        dbg("remap header_buf failed\n");
        return -ENXIO;
    }
    start += PAGE_SIZE;        

    while((tck = tq->bufs[idx])  && (STATE_READY == get_tck_state(tck))){
        pfn = (__pa(tck->start)) >> PAGE_SHIFT;
        set_tck_state(tck,STATE_BUSY);
        if(last_pfn != pfn){
            if (remap_pfn_range(vma,start,pfn,PAGE_SIZE,PAGE_SHARED)){
                dbg("remap page range failed\n");
                return -ENXIO;
            }
            start += PAGE_SIZE;
        }
        last_pfn = pfn;
        idx++;
        if(idx >= tq->buf_nr)
            break;
    }
    return 0;	
}

int tws_mmap(struct file *file, struct vm_area_struct *vma)
{
    int ret = 0;
    struct isil_service_device *tsd = (struct isil_service_device *)file->private_data;
    unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
    unsigned long size = vma->vm_end - vma->vm_start;
    struct tc_queue *q = tsd->queue;

#ifdef STATIC_COUNTING
    get_ic_map_start(tsd);
#endif
    if (offset & ~PAGE_MASK){
        dbg("offset not aligned: %ld\n", offset);
        return -ENXIO;
    }

    //dbg(" %s --vma->start = %X,the size =%d\n",__FUNCTION__,vma->vm_start,size);
    /*      if (size > 512){
            dbg("size too big\n");
    //return(-ENXIO);
    }
    */        
    if ((vma->vm_flags & VM_WRITE) && !(vma->vm_flags & VM_SHARED)){
        dbg("writeable mappings must be shared, rejecting\n");
        return(-EINVAL);
    }

    /*
       vma->vm_ops = &tc_vm_ops;
       tc_vm_open(vma);
       vma->vm_flags |= VM_DONTEXPAND | VM_RESERVED;
       vma->vm_flags &= ~VM_IO; // using shared anonymous pages 
       vma->vm_private_data = tae;
       dbg("mmap: range %X ----%X pgoff = %X\n",vma->vm_start,vma->vm_end, vma->vm_pgoff);
       return 0;
       */

    if(!q){
        dbg("tsd no queue\n");
        return -EINVAL;
    }


    if(q->unit_size >PAGE_SIZE)
        ret = mmap_scatter(vma,q,size);
    else
        ret = mmap_single(vma,q);

#ifdef STATIC_COUNTING
    calc_ic_map_time(tsd);
#endif
    return ret;
}


struct file_operations tws_fops = {
    .owner = THIS_MODULE,
    .open = tws_open,
    .release = tws_release,
    .read = tws_read,
    .write = tws_write,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
    .unlocked_ioctl = tws_ioctl,
#else
    .ioctl = tws_ioctl,
#endif
    .mmap  = tws_mmap,
    .poll = tws_poll,
};


#if 0
static void chip_release(struct class_device *cd)
{
    return;
}

static struct class isil_tcd = {                                              
	.name    = "isil",                                
    .release = chip_release,                                   
}; 


int register_chipdevice_notifier(struct notifier_block *nb)
{
    struct isil_chip_device *dev;
    struct list_head  * p;
    struct isil_chip_device *last;
    int err;

    mutex_lock(&isil_core_lists);
    err = raw_notifier_chain_register(&isil_chipdev_chain, nb);
    if (err)
        goto unlock;

    list_for_each(p, &isil_chipdev){
        dev = container_of(p, struct isil_chip_device,list);
        err = nb->notifier_call(nb, CHIPDEV_REGISTER, dev);
        err = notifier_to_errno(err);
        if (err)
            goto rollback;

        //if (!(dev->flags & IFF_UP))
        //	continue;

        nb->notifier_call(nb, CHIPDEV_UP, dev);
    }
unlock:
    mutex_unlock(&isil_core_lists);
    return err;

rollback:
    last = dev;
    list_for_each(p, &isil_chipdev){
        if (dev == last)
            break;

        //if (dev->flags & IFF_UP) {
        //	nb->notifier_call(nb, CHIPDEV_GOING_DOWN, dev);
        //	nb->notifier_call(nb, CHIPDEV_DOWN, dev);
        //}
        nb->notifier_call(nb, CHIPDEV_UNREGISTER, dev);
    }

    raw_notifier_chain_unregister(&isil_chipdev_chain, nb);
    goto unlock;
}
int unregister_chipdevice_notifier(struct notifier_block *nb)
{
    int err;

    mutex_lock(&isil_core_lists);
    err = raw_notifier_chain_unregister(&isil_chipdev_chain, nb);
    mutex_unlock(&isil_core_lists);
    return err;
}


void isil_chip_command(struct isil_chip_device *chip, unsigned int cmd, void *arg)
{
    struct list_head  *item;
    //struct isil_chip_device *client;

	chip->driver->command(chip,cmd,arg);
	return;
	notify others which is interested with this chip

	//...
	
//}
//EXPORT_SYMBOL(isil_chip_command);
#endif


void dumy_release(struct device * dev)
{
    return;
}



/* Allocates a struct tcd with private data area for driver use
 * and perform basic initialization.
 */
struct isil_chip_device * tc_chipdev_alloc(unsigned long priv_size)
{
    void * p;
    unsigned long size;
    struct isil_chip_device *tcd;

    size = (sizeof(*tcd) + CHIPDEV_ALIGN_MASK) & ~CHIPDEV_ALIGN_MASK;
    size += priv_size + CHIPDEV_ALIGN_MASK;
    p = (void *)__get_free_pages(GFP_KERNEL, get_order(size));//kzalloc(size, GFP_KERNEL);
    if(!p){
        printk("%s: no mem!, %lu\n",__FUNCTION__, size);
        return NULL;
    }
    memset(p, 0, size);
    dbg("p =%p\n",p);
    tcd = (struct isil_chip_device *)(((long)p + CHIPDEV_ALIGN_MASK) & ~CHIPDEV_ALIGN_MASK);
    dbg("tcd = %p\n",tcd);
    tcd->padded = (char *)tcd - (char *)p;
    tcd->priv_len = priv_size;
    tcd->total_len = size;
    dbg(" tcd->padded = %d\n",tcd->padded);
    if(priv_size)
        tcd->priv = tcd_priv(tcd);
    return tcd;
} 

void tc_chipdev_free(struct isil_chip_device * tcd)
{
    char *p = (char *)tcd;

    p = p- tcd->padded;
    dbg("free addr =%p\n",p);
    //kfree((char *)tcd - tcd->padded);
    free_pages((unsigned long)p,get_order(tcd->total_len));
    //kfree(p);
}





int check_chipdev_info(struct isil_chip_device * tcd)
{
    if(!tcd->name[0]){
        dbg("no name for chip device!\n");
        return -1;
    }
    return 0;
}



int setup_chip_dev(struct isil_chip_device * tcd)
{
    return 0;
}



static int bind_chip_driver(struct isil_chip_device * tcd)
{
    struct list_head *p;
    struct isil_chip_driver *tdr;
    struct isil_dev_table_id *table_id;

    /*walk the list to find the crosponding devices*/
    __list_for_each(p,&isil_chipdrv){
        tdr = list_entry(p,struct isil_chip_driver,list);
        table_id = &tdr->device_id;
        if(!table_id->tid){
            printk("BUG:no tid registered with isil_chip_driver!\n");
            return -EINVAL;
        }
        if(chip_device_match(&tcd->epobj,&table_id->tid->epobj)){
            tcd->driver = tdr;
            if(tdr->attach_chip)
                return tdr->attach_chip(tdr,tcd);//driver init	
        }
    }
    return 0;
}


static void unbind_chip_driver(struct isil_chip_device *tcd)
{
    struct list_head *p;
    struct isil_chip_driver *tdr;
    struct isil_dev_table_id *table_id;

    /*walk the list to find the crosponding devices*/
    __list_for_each(p,&isil_chipdrv){
        tdr = list_entry(p,struct isil_chip_driver,list);
        table_id = &tdr->device_id;
        if(!table_id->tid){
            printk("BUG:no tid registered with isil_chip_driver!\n");
            return;
        }
        if(chip_device_match(&tcd->epobj,&table_id->tid->epobj)){
            tcd->driver = tdr;
            if(tdr->detach_chip)
                tdr->detach_chip(tdr,tcd);	
        }
    }
}




int chip_device_match(struct epobj_t *a, struct epobj_t *b)
{
	//dbg("a->vid[%lu],a->bid[%lu],a->cid[%lu]---\n b->vid[%lu],b->bid[%lu],b->cid[%lu]---\n",a->vendor_id,
	//	a->bus_id,a->chip_id,b->vendor_id,b->bus_id,b->chip_id);
	if(a->vendor_id == b->vendor_id && a->bus_id == b->bus_id && a->chip_id == b->chip_id)
		return 1;
	else
		return 0;
}


/*note:the tcd->name must be filled before call this function*/
int register_chip_device(struct isil_chip_device *tcd)
{
    int ret = 0;
    int idx = 0; 
    int minor = -1;
    unsigned long flag = 0;
    char chip_dir_name[50]={0};
    struct kobject *kobj = NULL;
    unsigned long fix_id;
    struct cmd_arg ca = {0};


    if(check_chipdev_info(tcd))
        return -EINVAL;

    //if(setup_chip_dev(tcd))
    //	return -ENOMEM;

    ca.type = 0;
    minor = get_free_minor(&ca);
    if(minor < 0 || minor > 256)
    {
        dbg("minor is %d",minor);
        return -EINVAL;
    }

    snprintf(chip_dir_name,sizeof(chip_dir_name),"%s",tcd->name);
    kobj = isil_create_dir(chip_dir_name,kroot);
    if(kobj == NULL){
        dbg("create dir failed!\n");
        return -EINVAL;
    }

    tcd->cdev = (struct cdev *)cdev_alloc();
    if(tcd->cdev == NULL){
        dbg("cdev_alloc failed!\n");
        ret = -ENOMEM;
        goto cleanup;
    }

    cdev_init(tcd->cdev,&isil_fops);
    tcd->cdev->owner = THIS_MODULE;//tcd->tcd_fops->owner;
    ret = cdev_add(tcd->cdev,MKDEV(resource_major[ISIL_CHIP],minor),1);
    if(ret < 0){
        flag = 1;
        dbg("cdev_add failed!\n");
        goto cleanup;
    }

    for(idx = 0; idx < sizeof(tcd->tsd_list)/sizeof(struct list_head) ; idx++){
        INIT_LIST_HEAD(&tcd->tsd_list[idx]);
        tcd->dirs[idx] = NULL;
    }
    tcd->minor = minor;

    tcd->dev.parent = NULL;
    tcd->dev.kobj.parent = kobj;
    tcd->dev.devt =  MKDEV(resource_major[ISIL_CHIP],minor);
    tcd->dev.release = dumy_release;
    fix_id = get_chipid_from_epobj(&tcd->epobj);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 30)
    dev_set_name(&tcd->dev, "%scontrol%02lu", tcd->name, fix_id);
#else
    snprintf(tcd->dev.bus_id,sizeof(tcd->dev.bus_id),"%scontrol%02lu",tcd->name,fix_id);
#endif
    ret = device_register(&tcd->dev);
    if(ret < 0){
        dbg("device register failed!\n");
        goto cleanup;
    }
    ret = bind_chip_driver(tcd);
    if(ret)
        goto bind_error;

    mutex_lock(&mutex_chipdev);
    list_add_tail(&tcd->list,&isil_chipdev);
    mutex_unlock(&mutex_chipdev);
    sema_init(&tcd->sem, 1);
    return ret;
bind_error:
    printk("chip driver bind error!");
    device_unregister(&tcd->dev);
cleanup:
    if(kobj)
        kfree(kobj);
    if(tcd->cdev){
        if(flag == 0)
            cdev_del(tcd->cdev);
        kfree(tcd->cdev);
    }
    put_free_minor(tcd->minor,0);
    return ret;
}


/* need free all the sub resources such as tsd device...*/
void unregister_chip_device(struct isil_chip_device * tcd)
{
    struct kobject * dir_kobj = NULL;
    //int cnt;	

    dbg("passed tcd[0x%8p]\n",tcd);	
    if(!tcd){
        dbg("ungister with NULL pointer!\n");
        goto donothing;
    }
    if(find_chip_dev(tcd->minor) == NULL){
        dbg("bad ungister!\n");
        goto donothing;
    }

    dir_kobj = tcd->dev.kobj.parent; 
    if(!dir_kobj){
        dbg("this device is located wrong place!\n");
        goto donothing;
    }

    mutex_lock(&mutex_chipdev);
    list_del(&tcd->list);
    mutex_unlock(&mutex_chipdev);

    cdev_del(tcd->cdev);
    kfree(tcd->cdev);
    //dbg("before device_unregister!\n");
    device_unregister(&tcd->dev);
    //kfree(tcd);
    //dbg("before unbind chip_driver!\n");
    unbind_chip_driver(tcd);
    put_free_minor(tcd->minor,0);
    if(!tcd->clean_up){
        dbg("no clean_up function registed for tcd\n");
        isil_remove_dir(dir_kobj);
        dbg("after isil_remove_dir!\n");
        return;
    }
    tcd->clean_up(tcd);
    isil_remove_dir(dir_kobj);
    return;
donothing:
    return;	

}




int register_chip_driver(struct isil_chip_driver * driv)
{
    int res = 0;
    //struct list_head *p;
    //struct isil_chip_device * tcd = NULL;
    //struct isil_dev_table_id * table_id = &driv->device_id;


    driv->driver.bus = &isil_virtual_bus;
    INIT_LIST_HEAD(&driv->tcd_list);

    //dbg("the bus = %X\n",driv->driver.bus);
    res = driver_register(&driv->driver);
    if (res)
        return res;
    //dbg("after driver_register!\n");
    mutex_lock(&mutex_chipdrv);
    list_add_tail(&driv->list,&isil_chipdrv);
    mutex_unlock(&mutex_chipdrv);

    return res;
}

/*
   int register_chip_driver_instance(struct isil_chip_driver * driv, struct isil_chip_device *tcd)
   {
   int res = 0;
   struct list_head *p;
   struct isil_chip_device * tcd = NULL;
   struct isil_dev_table_id * table_id = &driv->device_id;

   if(chip_device_match(&tcd->epobj,&table_id->tid->epobj)){
   tcd->driver = driv;
   if(driv->attach_chip)
   driv->attach_chip(driv,tcd);//driver init instance
   }

   return res;
   }
   */


void unregister_chip_driver(struct isil_chip_driver * driv)
{
    //struct list_head * p = NULL;
    //struct isil_chip_device * tcd = NULL;
    //struct isil_dev_table_id * table_id = &driv->device_id;

    /*
       if(driv->detach_chip){
       __list_for_each(p,&isil_chipdev){
       tcd = list_entry(p, struct isil_chip_device, list);
       if(chip_device_match(&tcd->epobj,&table_id->tid->epobj))
       driv->detach_chip(driv,tcd); //driver remove
       }		
       }
       */
    driver_unregister(&driv->driver);
    mutex_lock(&mutex_chipdrv);	
    list_del(&driv->list);
    mutex_unlock(&mutex_chipdrv);

}


int check_valid(struct isil_chip_device* tcd,unsigned long type)
{
    //dbg("type is %d and tcd = %p\n",type,tcd);

    if((type > MAX_SERVICE_NR) || tcd == NULL)
        return -1;

    return 0;
}


int check_chipdev_limit(struct isil_chip_device *tcd, struct cmd_arg *arg)
{
    struct list_head *p;
    struct cmd_list *cl;

    list_for_each(p,&cl_head){
        cl = container_of(p, struct cmd_list, node);
        if(cmd_equal(&cl->arg,arg) && cl->tcd == tcd)
            return -EINVAL;
    }
    return 0;

}

/* return value: 
 * 0: means the dir is first created
 * 1: means the dir is aready created brefore
 * -1: falied!
 */
static int get_dir(struct isil_chip_device *tcd, unsigned long type, struct kobject **dir)
{
    char * name_prefix = NULL; 
    struct kobject *d = NULL;//tcd->dev.kobj.parent;	
    
    //tc_trace;
    *dir = NULL;
    if(tcd->dirs[type]){
	*dir = tcd->dirs[type];
        return 1;
    }

    switch(type+1)
    {
        case ISIL_ENCODER:
            name_prefix = "isil_encoder";
            break;
        case ISIL_DECODER:
            name_prefix = "isil_decoder";
            break;
        case ISIL_VIDEO_IN:
            name_prefix = "isil_video_in";
            break;
        case ISIL_VIDEO_OUT:
            name_prefix = "isil_video_out";
            break;
        case ISIL_AUDIO_IN:
            name_prefix = "isil_audio_in";
            break;
        case ISIL_AUDIO_OUT:
            name_prefix = "isil_audio_out";
            break;
        default:
            name_prefix = "unknown";
            break;
    }

    //dbg("dir name%s \n",name_prefix);

    /*arrive here, means no dir created , so we must create it here!*/
    d = isil_create_dir(name_prefix, tcd->dev.kobj.parent);
    if(d == NULL){
        dbg("create dir failed in %s\n", __FUNCTION__);
        return -1;
    }
//    kobject_uevent(d, KOBJ_ADD);
    //dbg("dir's kobject pointer = %x\n",dir);
    *dir = d;
    return 0;
}





static struct isil_dev_table_id * lookup_tcdev_tabe_id(struct epobj_t *ep)
{
    struct list_head *p = &isil_tc_dev;
    struct isil_dev_table_id	*dev;

    //dbg_epobj_print(ep);
    list_for_each(p, &isil_tc_dev){
        dev = list_entry(p, struct isil_dev_table_id, list);
        //dbg_epobj_print(&dev->tid->epobj);
        if (epobj_euqal(ep,&dev->tid->epobj))
            return dev;
    }
    return NULL;
}


int parse_arg(struct isil_chip_device *tcd, struct cmd_arg *arg)
{
    chip_driver_t *cd;
    struct isil_logic_phy_table *tlp;
    u32	logic = 0,phy = 0;
    int	ret = -1;
    enum  LOGIC_MAP_TABLE_TYPE  mt;

    if(!tcd || !arg || !tcd->ped){
        dbg("invalid parameters!\n");
        return -EINVAL;
    }


    cd = container_of(tcd->ped, chip_driver_t,opened_control_ed);
    tlp = &cd->logic_map_phy;
    if(!tlp->op || !tlp->op->find_phy_by_logic){
        printk("logic map no op\n");
        return -EINVAL;
    }

    if(ISIL_ENCODER == arg->type){
        if(CODEC_VIDEO_H264 == arg->algorithm){
            mt = LOGIC_MAP_TABLE_BIND_H264E;
        }
        else if(STREAM_TYPE_AUDIO == arg->stream){
            mt = LOGIC_MAP_TABLE_BIND_AENC;
        }
        else if(CODEC_VIDEO_MJPG == arg->algorithm){
            mt = LOGIC_MAP_TABLE_BIND_JPEGE;
        }
        else{
            goto out;
        }
    }
    else if(ISIL_DECODER == arg->type){
        if(STREAM_TYPE_AUDIO == arg->stream){
            mt = LOGIC_MAP_TABLE_BIND_ADEC;
        }
        else if(CODEC_VIDEO_H264 == arg->algorithm){
            mt = LOGIC_MAP_TABLE_BIND_H264D;
        }
        else{
            goto out;	
        }
    }
    else{
        goto out;
    }

    logic = arg->channel_idx;
    ret = tlp->op->find_phy_by_logic(tlp, mt,logic, &phy);
    if(ret){
        dbg("logic to phy find nothing!\n");
        goto out;
    }
    /*here we convert the logic channel id to phy id*/
    arg->channel_idx = phy;
    return 0;

out:
    return ret;	
}


int tc_driver_init(struct isil_chip_device *tcd, struct isil_service_device *tsd,struct isil_service_driver *driv, struct cmd_arg *arg)
{
    int ret = 0;
    struct tsd_driver_operations *tsdr;
    struct isil_dev_table_id	*table_id;
    struct cmd_arg  tmp_arg = {0};

    if(!driv){
        printk("--->in %s\n",__FUNCTION__);
        return -EINVAL;
    }
    table_id = (struct isil_dev_table_id *)&driv->device_id;
    if(!table_id->ops){
        dbg("driver with no ops!\n");
        return -EINVAL;
    }
    tsdr = (struct tsd_driver_operations *)table_id->ops;
    if(!tsdr->init){
        dbg("driver with no init function!\n");
        return -EINVAL;
    }

        if(copy_from_user((void *)&tmp_arg,(void __user *)arg, sizeof(struct cmd_arg)) != 0){
            printk("%s, %d: copy_from_user failed\n", __FUNCTION__, __LINE__);

        }
    if(ISIL_ENCODER == arg->type || ISIL_DECODER == arg->type){
        if(parse_arg(tcd,&tmp_arg))
            return -EINVAL;
    }

    /*now the tmp_arg->channel_idx is the phy channel id, so the tc driver can use it to init the driver structures by phy id*/
    //printk("[[logic id = %lu]]\n",tmp_arg.channel_idx);

    //ret = tsdr->init(tcd,tsd,table_id->para,&tmp_arg);
    ret = tsdr->init(tcd,tsd,tcd_priv(tcd),&tmp_arg);
    if(ret){
        dbg("driver can not alloc resource!\n");
        return -ENOMEM;
    }
    epobj_key_update_phy_id((epkey_t *)&(tsd->epobj.key),tmp_arg.channel_idx);
    return ret;   
}




int setup_service_device(struct isil_service_device *tsd, struct isil_chip_device *tcd, struct cmd_arg *arg, struct kobject *dir,struct isil_dev_table_id *table_id)
{
    int ret = 0,sma,minor;
    unsigned long isadd = 0;
    struct device_info di = {0};
    struct list_head *plist = NULL;
    int type = arg->type - 1;
    //char tmp[255] = {0};
    //char *cp = tmp;

    if(type < 0){
        printk("BUG: type error in %s!\n", __FUNCTION__);
        return -1;
    }

    tcd->dirs[type] = dir;
    epobj_set(&tsd->epobj,&tcd->epobj);
    tsd->epobj.type = arg->type;
    tsd->epobj.func_id = (arg->stream << 16) | arg->algorithm;
    //tsd->epobj.key = make_service_key(tcd,arg);
    memset((char *)&tsd->epobj.key,0,sizeof(epkey_t));
    make_service_key((epkey_t *)&tsd->epobj.key,tcd,arg);
    //dbg_epobj_print(&tsd->epobj);
        if((ret = tc_driver_init(tcd,tsd,table_id->priv,arg))){
        printk("service driver init failed!\n");
        return ret;
    }

    sma = resource_major[type + 1];
    if(type > 1){
	    snprintf(tsd->name, 20,"%s_%02lu",tc_dev_name[type+1],arg->channel_idx);
    }else{
        snprintf(tsd->name,sizeof(tsd->name),"chan%02lu-%s-%s",arg->channel_idx,stream[arg->stream],algorithm[arg->algorithm]);

	}
    tsd->cdev = cdev_alloc();
    if(tsd->cdev == NULL){
        dbg("cdev_alloc failed!\n");
        return -ENOMEM;
    }

    minor = get_free_minor(arg);
    if(minor < 0){
        dbg("service minor get failed!\n");
        kfree(tsd->cdev);
        return -EINVAL;
    }
    //dbg("the service device' major = %d; minor = %X\n",sma,minor);
    cdev_init(tsd->cdev,&tws_fops);
    tsd->cdev->owner = THIS_MODULE;
    ret = cdev_add(tsd->cdev,MKDEV(sma,minor),1);
    if(ret < 0){
        dbg("cdev_add failed!\n");
        goto error;
    }
    isadd = 1;

    tsd->minor = minor;
    tsd->ptcd = tcd;
    tsd->type = type;//type = arg->type -1
    tsd->dev.kobj.parent = dir;
    tsd->dev.class = NULL;
    tsd->dev.devt = MKDEV(sma,minor);
    tsd->dev.release = dumy_release;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 30)
    dev_set_name(&tsd->dev, "%s", tsd->name);
#else
    snprintf(tsd->dev.bus_id, 20,"%s",tsd->name);
#endif
    ret = device_register(&tsd->dev);
    if(ret<0){
        dbg("device_register failed in %s!\n",__FUNCTION__);
        goto error;
    }
#ifdef  STATIC_COUNTING
    memset((char *)&tsd->b_itval, 0, sizeof(struct tc_buf_interval));
    tsd->b_itval.u_time = 5;
#endif	
    plist = &tcd->tsd_list[type];// type = arg->type -1
    printk("added to tsd_list[%d]\n",type);
    list_add_tail(&tsd->list,plist);

    di.major = sma;
    di.minor = minor;
    memcpy(di.path,tsd->name,255);
    //printk("***@@@@@@--->dev: minor= %d,%s\n",minor,tsd->name);
	if(copy_to_user(arg->dev_info,&di,sizeof(struct device_info)) != 0){
		printk("%s, %d: copy_to_user failed\n", __FUNCTION__, __LINE__);
	}
    //atomic_set(&tsd->state, ISIL_DEV_NEEDINIT);
    return 0;
error:
    if(tsd->cdev){
        if(isadd)
            cdev_del(tsd->cdev);
        kfree(tsd->cdev);
        put_free_minor(minor,arg->type);
    }	
    return ret;	

}


struct isil_service_device * tc_serdev_alloc(unsigned long priv_size)
{
    void * p;
    unsigned long size;
    struct isil_service_device *tsd;

    size = (sizeof(*tsd) + CHIPDEV_ALIGN_MASK) & ~CHIPDEV_ALIGN_MASK;
    size += priv_size + CHIPDEV_ALIGN_MASK;
    p = kzalloc(size, GFP_KERNEL);
    if(!p){
        printk("%s: no mem!\n",__FUNCTION__);
        return NULL;
    }
    tsd = (struct isil_service_device *)(((long)p + CHIPDEV_ALIGN_MASK) & ~CHIPDEV_ALIGN_MASK);
    tsd->padded = (char *)tsd - (char *)p;
    if(priv_size)
        tsd->priv = tsd_priv(tsd);
    //dbg("####---%s p[0x%8p],sizeof(*tsd)[%d],tsd->priv[0x%8p]\n",__FUNCTION__,p,sizeof(struct isil_service_device),tsd->priv);
    return tsd;
} 

void tc_serdev_free(struct isil_service_device * tsd)
{
    kfree((char *)tsd - tsd->padded);
}


/*only setup essential staff in tsd for every devs*/
int do_create_service_dev(struct isil_chip_device * tcd, struct cmd_arg * arg, struct kobject * dir)
{
    int ret = 0;
    struct epobj_t ep = {0};
    struct tsd_file_operations *tsd_ops = NULL;
    struct isil_service_device *tsd = NULL;
    struct isil_dev_table_id	*table_id;
    struct cmd_list  *cl = NULL;
    int type = arg->type - 1;

    if(type < 0){
        printk("BUG: type error in %s!\n", __FUNCTION__);
        return -1;
    }
    tcd->dirs[type] = dir;

    epobj_set(&ep,&tcd->epobj);
    ep.type = arg->type;
    ep.func_id = (arg->stream << 16) | arg->algorithm;
    //ep.key = make_service_key(tcd,arg);
    make_service_key((epkey_t *)&ep.key,tcd,arg);

    table_id = lookup_tcdev_tabe_id(&ep);
    if(!table_id){
        printk("we didn't support this type device!\n");
        return -ENODEV;
    }
    tsd_ops = (struct tsd_file_operations *)table_id->ops;

    if(!tsd_ops->init){
        dbg("no init function for service device!\n");
        return -EINVAL;
    }

    /*allocate the customer service device's structure*/
    ret = tsd_ops->init(tcd,arg,&tsd);
    if(ret || !tsd){
        tsd_ops->destory(tsd);
        return -ENOMEM;
    }
    tsd->tsd_fops = table_id->ops;
    tsd->private_data = table_id->para;//tc_dev 's table_id->para
    /*setup tsd, and call crosponding service driver's init*/
	ret = setup_service_device(tsd,tcd,arg,dir,table_id);
    if(ret){
        tsd_ops->destory(tsd);
        return ret;
    }

    /*if the service device create success, we can add it to cl_head for later(release) use*/
    cl = kzalloc(sizeof(*cl),GFP_KERNEL);
    if(!cl){
        dbg("cmd_list alloc failed!\n");
        ret = -ENOMEM;
        goto d_out;
        /*fall through???*/      
    }

    INIT_LIST_HEAD(&cl->node);
    cl->tcd = tcd;
    memcpy((char *)&cl->arg,(char *)arg,sizeof(struct cmd_arg));
    list_add_tail(&cl->node,&cl_head);
    //printk("cmd_type[%d],cmd_cix[%lu],cmd_algo[%d],cmd_stream[%d]\n",cl->arg.type,cl->arg.channel_idx,cl->arg.algorithm,cl->arg.stream);
    return 0;
d_out:
    release_service_device(tcd, (void *)arg);
    return ret;
}



int create_service_device(struct isil_chip_device * tcd, unsigned long data)
{
	int ret = 0,tmp;
	struct kobject *dir = NULL;
	struct cmd_arg *arg = (struct cmd_arg *)data;
	int it = arg->type -1;

    if(it < 0){
        printk("BUG:error type in %s \n",__FUNCTION__);
        return -EINVAL;
    }
    if(check_valid(tcd,arg->type)){
        dbg("invalid parameters\n");
        return -EINVAL;
    }

	if(check_chipdev_limit(tcd, arg)){
		dbg("no more resource!\n");
		return -EINVAL;
	}

	tmp = get_dir(tcd, it, &dir);
    if(dir == NULL){
        dbg("get dir failed\n");
        return -EEXIST; //this return value is not correct, doesn't means the dir is already exits.here just return an value < 0
    }
	ret = do_create_service_dev(tcd, arg,dir);
	if(ret && tmp == 0)
		isil_remove_dir(dir);
	return ret;
}



void tc_driver_instant_remove(struct isil_chip_device *tcd, struct isil_service_device *tsd,struct isil_service_driver *driv, struct cmd_arg *arg)
{
    struct tsd_driver_operations *tsdr;
    struct isil_dev_table_id	*driv_table_id;

    if(!driv){
        printk("NULL pointer--->in %s\n",__FUNCTION__);
        return;
    }
    driv_table_id = (struct isil_dev_table_id *)&driv->device_id;
    if(!driv_table_id->ops){
        dbg("driver with no ops!\n");
        return;
    }
    tsdr = (struct tsd_driver_operations *)driv_table_id->ops;
    if(!tsdr->remove){
        dbg("driver with no remove function!\n");
        return;
    }
    tsdr->remove(tcd,tsd,tcd_priv(tcd),arg);
}



struct isil_dev_table_id	*find_tc_serdriv_from_tcd(struct isil_chip_device *tcd,struct cmd_arg *arg)
{
        struct epobj_t ep = {0};

        epobj_set(&ep,&tcd->epobj);
	ep.type = arg->type;
	ep.func_id = (arg->stream << 16) | arg->algorithm;
	//ep.key = make_service_key(tcd,arg);
        make_service_key((epkey_t *)&ep.key,tcd,arg);

        //dbg_epobj_print(&ep);
        return lookup_tcdev_tabe_id(&ep);
        
}


void destroy_syn_normal_one(struct isil_service_device *tsd)
{
	return;
}

void tsd_rest_destroy(struct isil_service_device *tsd)
{
        //struct isil_chip_device *tcd = tsd->ptcd;

        if(tsd->type == ISIL_DECODER && ((get_stream_from_epobj(&tsd->epobj) == STREAM_TYPE_AUDIO)
                || get_algo_from_epobj(&tsd->epobj) == CODEC_VIDEO_H264))
                destroy_syn_normal_one(tsd);


}

void do_release_service_device(struct isil_service_device *tsd,void *data)
{
    struct kobject *k;
    struct isil_chip_device *t;
    struct list_head *p,*tmp;
    struct cmd_list *cl;
    struct cmd_arg *arg = (struct cmd_arg *)data;
    struct isil_dev_table_id	*table_id;

    k = tsd->dev.kobj.parent;
    cdev_del(tsd->cdev);
    kfree(tsd->cdev);
    device_unregister(&tsd->dev);
    //printk("tsd_type +1 = %d\n",tsd->type + 1);
    put_free_minor(tsd->minor,tsd->type + 1);
    t = tsd->ptcd;
    if(!t)
        printk("BUG! in %s tsd has inside none tcd!\n",__FUNCTION__);
    if(data){
        if(list_empty(&t->tsd_list[tsd->type])){
            dbg("remove service's parent!\n");              
            isil_remove_dir(k);
            t->dirs[tsd->type] = NULL; 
        }
        list_for_each_safe(p, tmp, &cl_head){
            cl = container_of(p, struct cmd_list, node);
            if(cmd_equal(&cl->arg,data) && cl->tcd == t){
                list_del(p);
                kfree(cl);
                break;
            }
        }
    }

    table_id = find_tc_serdriv_from_tcd(t,arg);
    if(table_id){
        printk("remove driver instance!\n");
        tc_driver_instant_remove(t,tsd,table_id->priv,arg);
    }else{
        dbg("can not find driver in tc_dev list\n");
    }

    /*perform future release*/
    if(!tsd->clean_up){
        dbg("you guys have no clean_up function registed\n");
        goto out;
    }
    tsd->clean_up(tsd);
 out:
    printk("after perforem release service -device tsd[0x%8p]\n\n\n\n\n",tsd);
    return;
}




void  release_service_device(struct isil_chip_device *tcd, void *data)
{
    struct cmd_arg *arg = (struct cmd_arg *)data;
    //int major;
    epkey_t new_key = {0};
    struct isil_service_device *tmp;
    struct list_head *p,*head;
    int type = arg->type - 1;
    //struct isil_video_de *tvd;
    //struct file *f;

    tc_trace;
    if(type < 0){
        printk("BUG: error type in %s \n", __FUNCTION__);
        return;
    }
    mutex_lock(&mutex_chipdev);	
    //major = resource_major[type + 1];// + ISIL_SERVICE_BASE_MAJOR;
    //new_key = make_service_key(tcd,arg);
    make_service_key((epkey_t *)&new_key,tcd,arg);
    //printk("in %s ,type =%d\n",__FUNCTION__,type);
    head = &tcd->tsd_list[type];
    list_for_each(p,head){
        tmp = container_of(p,struct isil_service_device,list);
		if(tmp && epkey_equal_without_phyid(&tmp->epobj.key,&new_key)){
			goto find;
        }
    }
    printk("@@@@@@@@@@in %s ---->invalid parameter! \n",__FUNCTION__);
    printk("@@@@@@@@@@type[%d],cix[%lu],algo[%d],stream[%d]\n",arg->type,arg->channel_idx,arg->algorithm,arg->stream);
    mutex_unlock(&mutex_chipdev);	
    return;
find:
    list_del(&tmp->list);
    mutex_unlock(&mutex_chipdev);

    do_release_service_device(tmp,data);

    /*
       k = tmp->dev.kobj.parent;
       cdev_del(tmp->cdev);
       kfree(tmp->cdev);
       device_unregister(&tmp->dev);
       put_free_minor(tmp->minor,arg->type);
       t = tmp->ptcd;
       if(list_empty(&t->tsd_list[type])){
       dbg("remove service's parent!\n");              
       isil_remove_dir(k);
       }
    //perform future release
    if(!tmp->clean_up){
    dbg("you guys have no clean_up function registed\n");
    return;
    }
    tmp->clean_up(tmp);
    printk("after perforem release service -device tcd[0x%08X]\n",tcd);
    */
    return ;
}


void release_all_service_device(void)
{
    struct list_head  *p,*q,*tmp,*tmt;
    //struct list_head  remove_list;
    struct isil_chip_device * tcd = NULL;
    struct isil_service_device *tsd = NULL;
    int idx = 0;
    struct kobject *k;
    struct cmd_list *cl;

    LIST_HEAD(rm_list);
    INIT_LIST_HEAD(&rm_list);

    mutex_lock(&mutex_chipdev);
    list_for_each(p,&isil_chipdev){
        tcd = container_of(p,struct isil_chip_device, list);
        for(idx=0; idx < MAX_SERVICE_NR-1; idx++){
            q = &tcd->tsd_list[idx];
            if(list_empty(q))	
                continue;
            list_for_each_safe(tmp,tmt,q){
                list_move_tail(tmp,&rm_list);
            }
        }
    }
    mutex_unlock(&mutex_chipdev);


    list_for_each_safe(p, tmp, &rm_list){
        //printk("----->in %s\n",__FUNCTION__);
        list_del(p);  
        tsd = list_entry(p, struct isil_service_device, list);
        do_release_service_device(tsd,NULL);//use NULL in internal of this function will never touch the tcd->dirs, 
    } 

    /*arrive here all service device have been removed, we need do more things
     *1: clean cmd_list 
     *2: update all the service device's info in tcd
     */
    if(!list_empty(&rm_list))
        printk("BUG: have service_dev lefted! %s\n",__FUNCTION__);

    list_for_each_safe(p,tmp,&cl_head){
        list_del(p);
        cl = container_of(p, struct cmd_list, node);
        kfree(cl);
    }

    mutex_lock(&mutex_chipdev);
    list_for_each(p,&isil_chipdev){
        tcd = container_of(p,struct isil_chip_device, list);
        for(idx=0; idx < MAX_SERVICE_NR-1; idx++){
            //q = &tcd->tsd_list[idx];
            k = tcd->dirs[idx];
            //if(list_empty(q)){	
            if(k){
                //printk("&&&&&&&&&&&&&&&&&&& k = %p\n",k);
                isil_remove_dir(k);

            }
            //}
            tcd->dirs[idx] = NULL;
        }
    }
    mutex_unlock(&mutex_chipdev);
    return;
}

void release_all_service_device_on_chip(struct isil_chip_device *tcd)
{
    struct list_head  *p,*q,*tmp,*tmt;
    //struct list_head  remove_list;
    //struct isil_chip_device * tcd = NULL;
    struct isil_service_device *tsd = NULL;
    int idx = 0;
    struct kobject *k;
    struct cmd_list *cl;

    LIST_HEAD(rm_list);
    INIT_LIST_HEAD(&rm_list);

    if(!tcd)
        return;


    mutex_lock(&mutex_chipdev);
    for(idx=0; idx < MAX_SERVICE_NR-1; idx++){
        q = &tcd->tsd_list[idx];
        if(list_empty(q))	
            continue;
        list_for_each_safe(tmp,tmt,q){
            list_move_tail(tmp,&rm_list);
        }
    }
    mutex_unlock(&mutex_chipdev);


    list_for_each_safe(p, tmp, &rm_list){
        //printk("----->in %s\n",__FUNCTION__);
        list_del(p);  
        tsd = list_entry(p, struct isil_service_device, list);
        do_release_service_device(tsd,NULL);//use NULL in internal of this function will never touch the tcd->dirs, 
    } 

    /*arrive here all service device have been removed, we need do more things
     *1: clean cmd_list 
     *2: update all the service device's info in tcd
     */
    if(!list_empty(&rm_list))
        printk("BUG: have service_dev lefted! %s\n",__FUNCTION__);

    list_for_each_safe(p,tmp,&cl_head){
        cl = container_of(p, struct cmd_list,node);
        if(cl->tcd == tcd){
            list_del(p);
            kfree(cl);
        }
    }

    mutex_lock(&mutex_chipdev);
    for(idx=0; idx < MAX_SERVICE_NR-1; idx++){
        //q = &tcd->tsd_list[idx];
        k = tcd->dirs[idx];
        //if(list_empty(q)){	
        if(k){
            //printk("&&&&&&&&&&&&&&&&&&& k = %p\n",k);
            isil_remove_dir(k);

        }
        //}
        tcd->dirs[idx] = NULL;
    }
    mutex_unlock(&mutex_chipdev);
    return;
}





void release_all_service_device_ext(void)
{
    struct list_head  *p,*q,*tmp;
    //struct list_head  remove_list;
    struct isil_chip_device * tcd = NULL;
    //struct isil_service_device *tsd = NULL;
    //int idx = 0;
    //struct kobject *k;
    struct cmd_list *cl;

    mutex_lock(&mutex_chipdev);
    list_for_each(p,&isil_chipdev){
        tcd = container_of(p,struct isil_chip_device, list);
        list_for_each_safe(q,tmp,&cl_head){
            //dbg("@@@@@@@@@\n");
            cl = container_of(q,struct cmd_list,node);
            if(cl->tcd == tcd){
                mutex_unlock(&mutex_chipdev);       
                release_service_device(cl->tcd,(void *)&cl->arg);
                mutex_lock(&mutex_chipdev);
            }
        }
    }
    mutex_unlock(&mutex_chipdev);

    /*since we call release_service_device with cl->arg as the second parameter, so we need do nothing here */
    return;
}


void release_all_service_device_ext_on_chip(struct isil_chip_device *tcd)
{
    struct list_head  *q,*tmp;
    //struct list_head  remove_list;
    //struct isil_chip_device * tcd = NULL;
    //struct isil_service_device *tsd = NULL;
    //int idx = 0;
    //struct kobject *k;
    struct cmd_list *cl;

    if(!tcd)
        return;
    mutex_lock(&mutex_chipdev);
    list_for_each_safe(q,tmp,&cl_head){
        //dbg("@@@@@@@@@\n");
        cl = container_of(q,struct cmd_list,node);
        if(cl->tcd == tcd){
            mutex_unlock(&mutex_chipdev);       
            release_service_device(cl->tcd,(void *)&cl->arg);
            mutex_lock(&mutex_chipdev);
        }
    }
    mutex_unlock(&mutex_chipdev);

    /*since we call release_service_device with cl->arg as the second parameter, so we need do nothing here */
    return;
}







/*add by version order, the bigger one goes first */
int register_tc_device(struct isil_dev_table_id *new)
{
    int ret = 0;
    struct list_head *p = &isil_tc_dev;
    struct isil_dev_table_id	*tb,*prev;


    mutex_lock(&mutex_tc_dev);
    if(p->next == &isil_tc_dev && p->prev == &isil_tc_dev){
        list_add_tail(&new->list,&isil_tc_dev);
        ret = 0;
        goto out;
    }

    p = p->next;
    if(p == &isil_tc_dev){
        list_add_tail(&new->list,&isil_tc_dev);
        ret = 0;
        goto out;
    }

    for(;p != &isil_tc_dev; prev = tb, p = p->next)
    {
        tb = container_of(p,struct isil_dev_table_id,list);
        if(tb->tid->epobj.vendor_id != new->tid->epobj.vendor_id || tb->tid->epobj.bus_id != new->tid->epobj.bus_id 
                || tb->tid->epobj.chip_id != new->tid->epobj.chip_id || tb->tid->epobj.func_id != new->tid->epobj.func_id)
            continue;
        if(tb->tid->version != new->tid->version){
            printk("already exist!\n");
            ret = -EEXIST;
            goto out;
        }
        if(new->tid->version > tb->tid->version){
            list_add(&new->list,&prev->list); 
            ret = 0;
            goto out;
        }
    }
    dbg_epobj_print(&new->tid->epobj);
    list_add(&new->list,&prev->list);
out:
    mutex_unlock(&mutex_tc_dev);
    return ret;	

}



void unregister_all_tc_device(void)
{
    struct list_head *_p,*tmp;
    struct isil_dev_table_id *dev;

    LIST_HEAD(remove_list);
    INIT_LIST_HEAD(&remove_list);

    mutex_lock(&mutex_tc_dev);
    while(!list_empty(&isil_tc_dev)){
        list_for_each_safe(_p, tmp, &isil_tc_dev){
            //dev = list_entry(_p, struct isil_dev_table_id, list);
            list_move_tail(_p,&remove_list);
        }
    }
    mutex_unlock(&mutex_tc_dev);


    list_for_each_safe(_p, tmp, &remove_list){
        dbg("now deleting...\n");
        list_del(_p);  
        dev = list_entry(_p, struct isil_dev_table_id, list);
        kfree(dev);               
    } 
    if(!list_empty(&remove_list))
        printk("BUG: have tc_dev lefted\n");    
    return;
}




void unregister_tc_device(struct isil_dev_table_id *tb)
{
    mutex_lock(&mutex_tc_dev);
    list_del(&tb->list);
    mutex_unlock(&mutex_tc_dev);
    kfree(tb);
}




int tc_driver_match(struct isil_dev_table_id * driv,struct isil_dev_table_id * dev)
{
    if(epobj_euqal(&driv->tid->epobj,&dev->tid->epobj))                
        return 1;
    else
        return 0;
}



int register_tc_driver(struct isil_service_driver * driv)
{
    int ret= 0;
    struct list_head *_p;
    struct isil_dev_table_id *dev;

    dev = &driv->device_id;
    dbg_epobj_print(&dev->tid->epobj);
    driv->driver.bus = &isil_virtual_bus;
    ret = driver_register(&driv->driver);
    if(ret){
        if(-EEXIST != ret)
            return ret;
        else
            return 0;
    }
    mutex_lock(&mutex_tc_drv);
    list_add_tail(&driv->list,&isil_tc_drv);
    mutex_unlock(&mutex_tc_drv);

    list_for_each(_p, &isil_tc_dev){
        dev = list_entry(_p, struct isil_dev_table_id, list);
        if (tc_driver_match(&driv->device_id,dev)){
            //printk("attached!\n");
            dev->priv = driv;   //tc_dev->table_id->priv = isil_service_driver
            break;
        }
    }
    return ret;	
}




void unregister_tc_driver(struct isil_service_driver *driv)
{
    struct list_head *_p;
    struct isil_dev_table_id *dev;

    list_for_each(_p, &isil_tc_dev){
        dev = list_entry(_p, struct isil_dev_table_id, list);
        if (tc_driver_match(&driv->device_id,dev)){
            dev->priv = NULL;
            break;
        }
    }        
    driver_unregister(&driv->driver);
    mutex_lock(&mutex_tc_drv);
    list_del(&driv->list);
    mutex_unlock(&mutex_tc_drv);
    printk("%s\n",__FUNCTION__);
}




//EXPORT_SYMBOL(register_chipdevice_notifier);
//EXPORT_SYMBOL(unregister_chipdevice_notifier);
EXPORT_SYMBOL(register_chip_device);
EXPORT_SYMBOL(unregister_chip_device);

EXPORT_SYMBOL(register_chip_driver);
EXPORT_SYMBOL(unregister_chip_driver);

EXPORT_SYMBOL(create_service_device);
EXPORT_SYMBOL(release_service_device);

EXPORT_SYMBOL(register_tc_device);
EXPORT_SYMBOL(unregister_tc_device);

EXPORT_SYMBOL(register_tc_driver);
EXPORT_SYMBOL(unregister_tc_driver);

EXPORT_SYMBOL(find_chip_dev);
EXPORT_SYMBOL(find_service_dev);

EXPORT_SYMBOL(tc_chipdev_alloc);
EXPORT_SYMBOL(tc_chipdev_free);






static int request_devno_resource()
{
    int ret = -1,i,j = 0;

    ret = init_nfm_bitmap();
    if(-1 == ret)
        return -1;
    for(i = ISIL_MAJOR_SEARCH_BASE; i < ISIL_MAJOR_MAX && j < MAX_SERVICE_NR; i++){
        ret = register_chrdev_region(MKDEV(i,0), MAX_MINOR_NR, tc_dev_name[j]);
        if (ret >= 0)
        	resource_major[j++] = i;
    }
    if(j != MAX_SERVICE_NR)
    	goto fail;
    return 0;
fail:
    for(i = 0; i < j; i++){
        unregister_chrdev_region(MKDEV(resource_major[i],0), MAX_MINOR_NR);
        free_nfm_bitmap();
    }
    return -1;
}


static void free_devno_resource()
{
    unsigned long i;

    free_nfm_bitmap();
    for(i = 0; i < MAX_SERVICE_NR; i++)
        unregister_chrdev_region(MKDEV(resource_major[i],0), MAX_MINOR_NR);
}





int tc_header_buffer_init(struct tc_hb_pool *pool,struct tc_header_buf *thb, int index)
{
    tcb_node_t	*node;
    //char * cp = NULL;

    if(!pool || !thb){
        printk("invalid para in %s\n",__FUNCTION__);
        return EINVAL;
    }
    atomic_set(&thb->ref, 0);
    node = &thb->node;
    node->op = &tcb_node_op;
    node->op->init(node);
    node->op->set_priv(node, thb);
    spin_lock_init(&thb->lock);
    thb->id = index;
    thb->unit_size= PAGE_SIZE;
    //thb->payload_len = 0;
#ifdef	USE_DISCRETE_BUF
        thb->data = (unsigned long *)__get_free_page(GFP_KERNEL);
        if(thb->data == NULL) {
            while(1) {
                printk("%s.%d: can't get one page, only for debug\n", __FUNCTION__, __LINE__);
            }
        }
#else
    cp = (char *)pool->p;
    thb->data = (unsigned long *)(cp + (thb->unit_size * index));
#endif
	if(thb->data != NULL) {
		memset((char *)thb->data,0,thb->unit_size);
	}
	return 0;
	
}



void tc_header_buffer_release(struct tc_hb_pool *pool,struct tc_header_buf *thb)
{
    struct tc_header_buf * tb;
    tcb_node_t * node;
    unsigned long flags;	

    if(thb){
        tb = thb;
        if(!pool)
            pool = tb->pool;

        spin_lock_irqsave(&tb->lock, flags);
        if(atomic_read(&tb->ref) > 1){
            atomic_dec(&tb->ref);
        }
        else
        {
            //if(tb->op && tb->op->reset())
            //	tb->op->reset();
            node = &tb->node;
            if(node->op){
                //dbg("pool =%p and ->release =%p\n",&pool->np,node->op->release);
                node->op->release(node,&pool->np);
                //dbg("after!\n");
            }
        }
        //*thb = NULL;
        spin_unlock_irqrestore(&tb->lock, flags);
    }
}

struct tc_hb_operation    tc_hb_op =
{
    .init = tc_header_buffer_init,
    .release = tc_header_buffer_release,
};


int tc_create_header_buffer_pool(unsigned long len)
{
    int ret = 0,idx;
    tcb_node_pool_t	*np;
    struct tc_header_buf *thb;

    thb_pool->order = get_order(len);
#ifdef	USE_DISCRETE_BUF
	thb_pool->p = NULL;
#else
    thb_pool->p = (unsigned long *)__get_free_pages(GFP_KERNEL,thb_pool->order);
    if(!thb_pool->p){
        dbg("no mem for tc header buffer pool!\n");
        ret = -ENOMEM;
        goto err;
    }
#endif
    thb_pool->nr = len/TC_HEADER_BUFFER_LEN;
    np = &thb_pool->np;
    np->op = &tcb_node_pool_op;
    thb_pool->bufs = kzalloc(sizeof(struct tc_header_buf) * thb_pool->nr, GFP_KERNEL);
    if(!thb_pool->bufs){
        ret = -ENOMEM;
        goto err;
    }
    np->op->init(np,thb_pool->nr);
    thb = thb_pool->bufs;
    for(idx = 0; idx < thb_pool->nr; idx++,thb++){
        //thb++;//= &(thb_pool->bufs[idx]);
        thb->op = &tc_hb_op;
        thb->pool = thb_pool;
        thb->op->init(thb_pool,thb,idx);
        thb->op->release(thb_pool,thb);
    }
    return ret;
err:
    if(thb_pool->bufs)
        kfree(thb_pool->bufs);
    if(thb_pool->p)
        free_pages((unsigned long)thb_pool->p,thb_pool->order);
    return ret;
}



void tc_destory_header_buffer_pool(struct tc_hb_pool *tp)
{
    tcb_node_pool_t *pn;

    if(!tp)
        return;

    pn = &tp->np;
    if(pn->op){
        pn->op->release(pn);
    }
    if(tp->bufs){
#ifdef  USE_DISCRETE_BUF
		struct tc_header_buf *thb;
		int i;
		for(i=0; i<tp->nr; i++) {
			thb = &tp->bufs[i];
			if(thb->data != NULL) {
				free_page((unsigned long)thb->data);
				thb->data = NULL;
			}
		}
#endif
		kfree(tp->bufs);
	}
    if(tp->p)
        free_pages((unsigned long)tp->p,tp->order);
    tp->nr = 0;
    tp->p = NULL;
    tp->bufs = NULL;
    tp->op = NULL;
}

void tc_get_header_buf(struct tc_hb_pool *tp, struct tc_header_buf **thb)
{
	tcb_node_pool_t *tnp;
	tcb_node_t *tn;
	unsigned long fg;

	if(!tp || !thb){
		printk("invalid parameter in %s!\n",__FUNCTION__);
		return;
	}
	spin_lock_irqsave(&tp->lock, fg);
	tnp = &tp->np;
	*thb = NULL;
	if(!tnp->op){
		printk("pool node has no op!\n");
		goto out;
	}
	tnp->op->get(tnp,&tn);
	if(tn){
		*thb = get_thb_from_node(tn);
		atomic_inc(&((*thb)->ref));
	}
	else
	{
		*thb = NULL;
	}
out:        
	spin_unlock_irqrestore(&tp->lock, fg);
}
void tc_try_get_header_buf(struct tc_hb_pool *tp, struct tc_header_buf **thb)
{
	tcb_node_pool_t *tnp;
	tcb_node_t *tn;
	unsigned long fg;

	if(!tp || !thb){
		printk("invalid parameter in %s!\n",__FUNCTION__);
		return;
	}

	spin_lock_irqsave(&tp->lock, fg);  
	tnp = &tp->np;
	*thb = NULL;
	if(!tnp->op){
		printk("pool node has no op!\n");
		goto out;
	}
	tnp->op->try_get(tnp,&tn);
	if(tn){
		*thb = get_thb_from_node(tn);
		atomic_inc(&((*thb)->ref));
	}
	else
	{
		*thb = NULL;
	}
out:
	spin_unlock_irqrestore(&tp->lock, fg);

}

void tc_put_header_buf(struct tc_hb_pool *tp, struct tc_header_buf *thb)
{
	tcb_node_pool_t *tnp;
	unsigned long fg;

	if(!tp || !thb){
		printk("invalid parameter in %s!\n",__FUNCTION__);
		return;
	}
	spin_lock_irqsave(&tp->lock, fg);         
	tnp = &tp->np;
	if(!tnp->op){
		printk("pool node has no op!\n");
		goto out;
	} 
	tnp->op->put(tnp,&thb->node);
out:
	spin_unlock_irqrestore(&tp->lock, fg);

}

int tc_get_hb_pool_entry_number(struct tc_hb_pool *tp)
{
	tcb_node_pool_t *tnp;

    if(!tp){
        printk("invalid parameter in %s!\n",__FUNCTION__);
        return EINVAL;
    }
    tnp = &tp->np;
    if(!tnp->op){
        printk("pool node has no op!\n");
        return EINVAL;
    }
    return tnp->op->get_curr_pool_entry_number(tnp);        
}


struct tc_hb_pool_operation    thb_op = 
{
    .create                         = tc_create_header_buffer_pool,
    .destory                        = tc_destory_header_buffer_pool,
    .get_header_buf                 = tc_get_header_buf,
    .try_get_header_buf             = tc_try_get_header_buf,
    .put_header_buf                 = tc_put_header_buf,
    .get_hb_pool_entry_number       = tc_get_hb_pool_entry_number,
};

int setup_thb_pool(void)
{
    int ret = 0;

    thb_pool = kzalloc(sizeof(*thb_pool),GFP_KERNEL);
    if(!thb_pool)
        return -ENOMEM;
	spin_lock_init(&thb_pool->lock);
    thb_pool->op = &thb_op;
    ret = thb_pool->op->create(TC_HEADER_BUFFER_POOL_LEN);
    return ret;
}

void cleanup_thb_pool(struct tc_hb_pool *tp)
{
    if(tp == NULL){
        printk("invalid parameter! in %s\n",__FUNCTION__);
        return;
    }

    if(tp->op != NULL)
        tp->op->destory(tp);
    else 
        tc_destory_header_buffer_pool(tp);

    kfree(tp);
}


/*static*/ int /*__init*/ isil_core_init(void)
{
    int ret = -1;

    INIT_LIST_HEAD(&isil_chipdev);
    INIT_LIST_HEAD(&isil_chipdrv);
    INIT_LIST_HEAD(&isil_tc_dev);
    INIT_LIST_HEAD(&isil_tc_drv);
    ret = bus_register(&isil_virtual_bus);
    if(ret){
        dbg("isil_virtual bus register failed!\n");
        return ret;
    }

    if(request_devno_resource())
        goto err_devno;
    if(setup_thb_pool())
        goto err_hb_pool;	

    //test_cmd();
	kroot = isil_create_dir("isil",NULL);
    if(kroot == NULL) {
		dbg("Err: failed in root dir isil!\n");
        goto err_isil_dir;
    }



    //isil_chip_init(ISIL_ED_HOST_BUS, 0);
    return 0;
    /*
       st = kthread_run(twellcore_work_thread,NULL,"twellcore_thread");
       if(IS_ERR(st)){
       dbg("create thread failed!\n");
       return PTR_ERR(st);
       }
       */	
err_isil_dir:
    isil_remove_dir(kroot);
err_hb_pool:
    cleanup_thb_pool(thb_pool);
err_devno:
    free_devno_resource();
    bus_unregister(&isil_virtual_bus);	
    return ret;
}


/*static*/ void /*__exit*/ isil_core_exit(void)
{
    isil_remove_dir(kroot);
    printk("dir removed!******\n");
    cleanup_thb_pool(thb_pool);
    free_devno_resource();
    printk("bus unregister!******\n");
    bus_unregister(&isil_virtual_bus);
    printk("all are released!\n");
}

//module_init(isil_core_init);
//module_exit(isil_core_exit);



