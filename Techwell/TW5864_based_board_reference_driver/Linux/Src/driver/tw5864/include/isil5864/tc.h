#ifndef  __ISIL_SERVICE_DEV_H__
#define  __ISIL_SERVICE_DEV_H__

#include <linux/device.h>
#include <asm/system.h>
#include <asm/types.h>
#include <linux/kobject.h>
#include <linux/kdev_t.h>

#define DMDEBUG


#ifdef DMDEBUG
#define dbg	printk
#define  tc_trace	dbg("in function:%s------------------------------------>\n",__FUNCTION__)
#else
#define dbg(format, arg...)	do{ }while(0)
#endif


#define U32LONG_MAX		0xFFFFFFFFUL


#define PATH_LEN		256

#define ISIL			0x00000001UL
/*chip id define goes here...*/
#define	TW2851			0x00000001UL
#define	TW5864			0x00000002UL


/*bus id defines*/
#define ISIL_BUS_PCI		0x00000001UL


/*func id define goes here...*/
#define FUNC_TYPE_OFFSET	20
#define	FUNC_STREAM_OFFSET	10
#define	FUNC_ALGORTM_OFFSET	0
#define MAKE_FUNC_ID(t,s,a)	(((t) << FUNC_TYPE_OFFSET) | ((s) << FUNC_STREAM_OFFSET) | ((a) << FUNC_ALGORTM_OFFSET))


/*key define goes here...*/
#define STREAM_OFFSET			26
#define ALGO_OFFSET			20
#define BUS_OFFSET			17
#define CHIP_OFFSET			11
#define PHYCHAN_OFFSET			5
#define LOGIC_OFFSET			0


#define MAX_CHIPS_NR    		(1 << CHIP_OFFSET)
#define MAX_PHYCHAN_PER_CHIP		(1 << 8)
#define MAX_LOGCHAN_PER_PHYCHAN		(1 << 8)
#define MAX_LOGCHAN_PER_PHYCHAN		(1 << 8)



#define MAX_NAME_LEN    		256
//#define ISIL_CHIP_MAJOR       		242
#define ISIL_MAJOR_SEARCH_BASE      46
#define ISIL_MAJOR_MAX              256
//#define ISIL_SERVICE_BASE_MAJOR		243
#define MAX_SERVICE_NR          	7
#define	MAX_MINOR_NR			255
#define CHIPDEV_ALIGN		        32
#define CHIPDEV_ALIGN_MASK              (CHIPDEV_ALIGN -1)


#define CHIPDEV_REGISTER 	0x0001
#define CHIPDEV_UNREGISTER	0x0002
#define CHIPDEV_UP		0x0003
#define CHIPDEV_GOING_DOWN	0x0004
#define CHIPDEV_REST		0x0005
#define CHIPDEV_DOWN		0x0006



#define TC_HEADER_BUFFER_POOL_LEN	0x20000 //128K



#define tc_min(x,y)	(((x) < (y))? (x): (y))
#define tc_max(x,y)	(((x) > (y))? (x): (y))
/*******************************************************************************************************/
/*******************************************************************************************************/
extern struct tc_hb_pool *thb_pool;
extern char *algorithm[];
extern char *stream[];
extern int nfm[MAX_SERVICE_NR];

struct tc_header_buf;
struct tc_hb_pool;



typedef enum
{
    /****video codec type*****/
    CODEC_VIDEO_H264 = 0,//h264 
    CODEC_VIDEO_MJPG,//mjpg
    //CODEC_VIDEO_JPEG,
    /*****audio codec type****/
    CODEC_AUDIO_ALAW,//G.711
    CODEC_AUDIO_ULAW,//G.711
    CODEC_AUDIO_ABADPCM,//G.722
    CODEC_AUDIO_MPMLQ,//G.723.1
    CODEC_AUDIO_ADPCM,//G.726
    CODEC_AUDIO_EMBED_ADPCM,//G.727 embeded adpcm
    CODEC_AUDIO_LDCELP,//G.728
    CODEC_AUDIO_CSACELP,//G.729
}CODEC_TYPE;


typedef enum
{
    STREAM_TYPE_MAJOR = 0,//major stream
    STREAM_TYPE_MINOR,//minor stream
    STREAM_TYPE_PIC,//picture stream, capture
    STREAM_TYPE_MV,//motion vector stream
    STREAM_TYPE_AUDIO,//audio stream
    STREAM_TYPE_NOTHING = 0xFFFF,
}STREAM_TYPE;

typedef enum isil_service_type
{
    ISIL_CHIP = 0,
    ISIL_ENCODER,
    ISIL_DECODER,
    ISIL_VIDEO_IN,
    ISIL_VIDEO_OUT,
    ISIL_AUDIO_IN,
    ISIL_AUDIO_OUT,
    ISIL_SERVICE_TYPE_NOTHING = 0XFFFF,
}ISIL_SERVICE_TYPE;

typedef enum isil_dev_state
{
    ISIL_DEV_NEEDINIT = 0,
    ISIL_DEV_CLOSED,
    ISIL_DEV_IDLE,
    ISIL_DEV_SUSPEND,
    ISIL_DEV_BUSY,
    ISIL_DEV_BROKEN,
}ISIL_DEV_STATE;


/*if create success, this info can be ignored*/
struct creation_status
{
    int fail_reasion;              //the failed reasion
    int finish;                    //if the falied reasion is no DDR, in tdm modle will blance,if blanced ok, this filed will be set 
};

#define to_get_creation_status_from_cmd(x)      (struct creation_status *)((char *)(x) + sizeof(struct cmd_arg) + sizeof(struct device_info))

struct device_info
{
    int major;                      //indicate the created device's major number
    int minor;                      //indicate the created device's minor number
    char path[256];                 //indicate the created device's name
    char status[0];                 //creation_status can be followed
};



struct cmd_arg
{
    ISIL_SERVICE_TYPE type;           //the service type
    unsigned long	channel_idx;    
    CODEC_TYPE	algorithm;
    STREAM_TYPE	stream;
    char 		dev_info[0];    //this is use for exchange data between uerland and kernel land.
    //the device_info always be here.  
};

struct cmd_list
{
    struct list_head                node;
    struct isil_chip_device           *tcd;
    struct cmd_arg                  arg;
};

struct end_point_obj
{
#if defined(__BIG_ENDIAN_BITFIELD)
    u32	stream:6;
    u32	algo:6;
    u32	bus_id:3;
    u32	chip_id:6;
    u32	phy_chan_id:6;
    u32	lgc_chan_id:5;
#elif defined (__LITTLE_ENDIAN_BITFIELD)
    u32	lgc_chan_id:5;
    u32	phy_chan_id:6;
    u32	chip_id:6;
    u32	bus_id:3;
    u32	algo:6;
    u32	stream:6;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
};
#define KEY_MAX_LGC_ID		0x20	/*max support up to 32 logic channel*/
#define KEY_MAX_PHY_ID		0x40    /*max support up to 64 phy channel*/
#define KEY_MAX_CHIP_ID		0x40    /*max support up to 64 chip ids*/
#define KEY_MAX_BUS_ID		0x08    /*max support up to 8 types of bus*/
#define KEY_MAX_STREAM_ID	0x40    /*max support up to 64 types of streams*/
#define KEY_MAX_ALGO_ID		0x40    /*max support up to 64 types of algorthims or codec types*/

typedef struct end_point_obj epkey_t;



/* this is the key struct of the isil_core, each object(chip_device,chip_driver,service_device,service_driver, bus driver....etc)
 * will use it as an embeded object to perform management.
 */
struct epobj_t
{
    unsigned long 	 vendor_id;			//alaways be isil right now
    unsigned long 	 bus_id;			//buses where we are.
    unsigned long 	 chip_id; 			//chipset id ie: #define TW5864 0x00000001 
    int  	 	 func_id;			//functional id  ie: stream|algorithm  used for service_device service_driver attch
    unsigned long	 type;                          //service type
    epkey_t	  	 key;				//global key for identity
};


/* each dev structs on tc framework should include epobj_t!
 * each driver structs on tc framework should include isil_dev_id(it has the epobj_t).
 * 1: dev register with insmod, driv register with insmod, so we can lookup and do matching, set the private void pointer to the driv
 * 2: user create an service dev by ioctl, -> tsd->tsd_fops->open ->do really set up in each detailed dev open function.
 */
struct isil_dev_id
{
    struct epobj_t		epobj;
    unsigned long 		version;
};


/* tcd driver and tsd driver both can include this struct,  
 * 1:if tcd driver, the ops always is an tcd_driver_operations
 * 2:if tsd driver, the ops always can be tsd_driver_operations
 */
struct isil_dev_table_id
{
    struct list_head 		list;
    struct isil_dev_id		*tid;
    void				*ops;           //tcd driver and tsd driver can 
    void 				*priv;		//can be driver pointer
    void 				*para;          //driver_data;   
};


/* this struct must included in different detailed customer dev structs, so tc_core can 
 * handle all the management future needed.
 */
struct isil_chip_device
{
    struct list_head  	list;
    char   name[MAX_NAME_LEN];                      //name of the chip device.
    int    minor;
    struct epobj_t    	epobj;                 	//embeded epobj_t.
    struct device    	dev;                    //embeded dev struct.
    struct cdev	 	*cdev;
    //atomic_t 		state;		
    struct semaphore		sem;
    struct tcd_file_operations *tcd_fops;           //do what??
    struct list_head tsd_list[MAX_SERVICE_NR-1];    //used for linking all the tsd devices.in ISIL_SERVICE_TYPE type order, the control service is not included
    struct kobject  *dirs[MAX_SERVICE_NR-1];        //dir arrays
    __u32		  padded;			//how much chip_dev padded 
    struct isil_chip_driver  *driver;			
    struct endpoint_tcb    *ped;			//point to end point, all the management cmds to binded driver are via ped
    void (*clean_up)(void *);			//this cleanup function performs upper structs's release! every detailed dev struct must have this function
    void                    *priv;			//point to detailed customered private dev structs
    unsigned int            priv_len;               //customered private dev structs length
    unsigned int            total_len;              //total length  struct tcd + private dev + alligned padd  used to free_page to get order
};

struct  isil_chip_driver
{
    struct list_head	list;
    char  			name[MAX_NAME_LEN];
    struct device_driver 	driver;
    struct isil_dev_table_id  device_id;

    int (*attach_chip)(void *driv,struct isil_chip_device *tcd);
    int (*detach_chip)(void *driv,struct isil_chip_device *tcd);

    /* driver model interfaces  */
    void (*shutdown)(struct isil_chip_device *tcd);
    int (*suspend)(struct isil_chip_device *tcd, unsigned int spmsg);
    int (*resume)(struct isil_chip_device *tcd);

    /* a ioctl like command that can be used to perform specific functions with the device.*/
    int (*command)(struct isil_chip_device *ptcd,unsigned int cmd, void *cmd_arg);
    struct list_head 	tcd_list;

};




#ifdef STATIC_COUNTING
struct tc_buf_interval
{
    /* 4 system call's total timer*/
    u32		        min;                            //the min ic4_total_time
    u32		        max;                            //the max ic4_total_time
    u32		        u_time; 			//unit time (ms)
    u32		        curr_tick;                      
    u32 		        s_req_tick;                     //start tick of request mem ioctl
    u32                     s_map_tick;                     //start tick of mmap system call
    u32                     s_rel_tick;                     //start tick of release mem ioctl

    u32                     ic_req_time;                    //request mem ioctl used time
    u32                     ic_map_time;                    //mmap used time
    u32                     ic_rel_time;                    //release mem ioctl used time
    u32                     ic4_total_time;                 //each 4 system call(requeset_mem, mmap,unmap,release_mem) loop uesd time 
};
#endif


struct isil_service_device
{
    struct 	list_head 	list;              	 	//embeded list head, used to link anywhere.
    char 			name[MAX_NAME_LEN];     	//name of the device node.
    unsigned long		type; 		                //the service type of the tsd device.
    struct epobj_t		epobj;				
    int			minor;                          //the minor number of the tsd device.
    //atomic_t 		state;
    struct device   	dev;                      	//embeded dev .
    __u32		 	padded;				//how much chip_dev padded 
    struct cdev     	*cdev;
    struct isil_chip_device 	*ptcd;           		//which chip dev we are belong.
    struct tsd_file_operations	*tsd_fops;
    struct endpoint_tcb     *ped;				//point to end point, all the management cmds to binded driver are via ped  
    struct tc_queue		*queue;
    void (*clean_up)(void *);			 	//this cleanup function performs upper structs's release! every detailed dev struct must have this function
    void    		*private_data;                  //typically is used for tc_dev's table_id->para
    void    		*priv;				//point to detailed customered private service dev structs
#ifdef  STATIC_COUNTING
    struct tc_buf_interval  b_itval;			//buf interval from get mem to release mem
#endif	
};


struct isil_service_driver
{
    struct list_head 	list;
    char			name[MAX_NAME_LEN];
    struct device_driver	driver;
    struct isil_dev_table_id		device_id;

    int (*attach_service)(void *drv, struct isil_service_device * tsd);
    int (*detach_service)(void *drv, struct isil_service_device * tsd);
    void (*shutdown)(struct isil_service_device *tsd);
    int (*suspend)(struct isil_service_device *tsd, unsigned int spmsg);
    int (*resume)(struct isil_service_device *tsd);
    int (*command)(struct isil_service_device *tsd,unsigned int cmd, void *cmd_arg);
    struct list_head	tsd_list;
};


struct tcd_file_operations
{
    struct module *owner;
    int (*open)(struct isil_chip_device *,struct file *);
    ssize_t (*read)(struct file *,char __user *,size_t, loff_t *);
    ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
    int (*ioctl) (struct file *, unsigned int, unsigned long);
    int (*release)(struct isil_chip_device *,struct file *);
};


struct tsd_file_operations
{
    struct module *owner;
    int (*init)(struct isil_chip_device *,struct cmd_arg *,struct isil_service_device **);
    void (*destory)(struct isil_service_device *);

    int (*open)(struct isil_service_device *,struct file *);
    ssize_t (*read)(struct file *,char __user *,size_t, loff_t *);
    ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);
    int (*ioctl) (struct file *, unsigned int, unsigned long);
    int (*release)(struct file *);
    unsigned int (*poll)(struct file *file, struct poll_table_struct *wait);	
};

struct tsd_driver_operations
{
    int (*init)(struct isil_chip_device *,struct isil_service_device *,void *,struct cmd_arg *);
    void (*remove)(struct isil_chip_device *,struct isil_service_device *,void *,struct cmd_arg *);
};

struct tcd_driver_operations
{
    int (*init)(struct isil_chip_device *,void *);
    void (*remove)(struct isil_chip_device *,void *);
};




#define TC_HEADER_BUFFER_LEN	PAGE_SIZE	

struct tc_hb_operation
{
    int 	(*init)(struct tc_hb_pool *,struct tc_header_buf *, int);
    void 	(*release)(struct tc_hb_pool *, struct tc_header_buf *);

};


struct tc_header_buf
{
    spinlock_t		lock;
    unsigned long		id;
    atomic_t		ref;
    tcb_node_t		node;
    unsigned long		unit_size;
    unsigned long		*data;
    struct tc_hb_pool 	*pool;
    struct tc_hb_operation	*op;
};

#define get_thb_from_node(x)	container_of(x,struct tc_header_buf,node)

struct tc_hb_pool_operation
{
    int (*create)(unsigned long size);
    void (*destory)(struct tc_hb_pool *);

    void (*get_header_buf)(struct tc_hb_pool *, struct tc_header_buf **);
    void (*try_get_header_buf)(struct tc_hb_pool *, struct tc_header_buf **);
    void (*put_header_buf)(struct tc_hb_pool *, struct tc_header_buf *);
    int  (*get_hb_pool_entry_number)(struct tc_hb_pool *);	
};

struct tc_hb_pool
{
    spinlock_t		lock;
    int			order;
    unsigned long		*p;
    unsigned long 		nr;
    tcb_node_pool_t 	np;
    struct tc_header_buf	*bufs;
    struct tc_hb_pool_operation *op;	
};



#define epkey_equal(a,b)                ((a)->stream == (b)->stream && (a)->algo == (b)->algo && (a)->bus_id == (b)->bus_id \
	&& (a)->chip_id == (b)->chip_id && (a)->phy_chan_id == (b)->phy_chan_id && (a)->lgc_chan_id == (b)->lgc_chan_id) 

#define epkey_equal_without_phyid(a,b)  ((a)->stream == (b)->stream && (a)->algo == (b)->algo && (a)->bus_id == (b)->bus_id \
	&& (a)->chip_id == (b)->chip_id &&(a)->lgc_chan_id == (b)->lgc_chan_id) 


#define epobj_euqal(a,b)		((a)->vendor_id == (b)->vendor_id && (a)->bus_id == (b)->bus_id \
	&& (a)->chip_id == (b)->chip_id  && (a)->func_id == (b)->func_id \
	&& (a)->type == (b)->type )

#define cmd_equal(a,b)			(!memcmp((char *)(a),(char *)(b), sizeof(struct cmd_arg)))			

#define epobj_set(d,s)			(memcpy((char *)(d),(char *)(s),sizeof(struct epobj_t)))

#define get_bus_id(key)			(key.bus_id)
#define get_chip_id(key)		(key.chip_id)
#define get_phy_id(key)			(key.phy_chan_id)
#define get_log_id(key)			(key.lgc_chan_id)
#define dbg_epobj_print(ep)             dbg("vendor_id=%lu,bus_id=%lu,chip_id=%lu,func_id=%d,type=%lu,key=0x08%x stream=%d,algo=%d,bid=%d,cid=%d,p=%d,l=%d \n", \
	(ep)->vendor_id,(ep)->bus_id,(ep)->chip_id,(ep)->func_id,(ep)->type,*((u32 *)&(ep)->key),(ep)->key.stream,(ep)->key.algo,(ep)->key.bus_id,\
	(ep)->key.chip_id,(ep)->key.phy_chan_id,(ep)->key.lgc_chan_id)


static inline void make_key(epkey_t *key,unsigned long stream,unsigned long algo,unsigned long bid,unsigned long cid,unsigned long pid,unsigned long lid)
{

    if(stream >= KEY_MAX_STREAM_ID){
	dbg("err type stream passed!\n");
	return;
    }
    if(algo >= KEY_MAX_ALGO_ID){
	dbg("err algo type passed!\n");
	return;
    }
    key->stream = stream;
    key->algo = algo;
    key->bus_id = bid;
    key->chip_id = cid;
    key->phy_chan_id = pid;
    key->lgc_chan_id = lid;    
    return;
}

static inline int get_func_id_from_epobj(struct epobj_t *ep)
{
    return ep->func_id;
}

static inline void epobj_init(struct epobj_t *ep, unsigned long vid, unsigned long bid,unsigned long cid, unsigned long fid,unsigned long type, epkey_t *key)
{
    memset((char *)ep, 0, sizeof(struct epobj_t));
    ep->vendor_id = vid;
    ep->bus_id = bid;
    ep->chip_id = cid;
    ep->func_id = fid;
    ep->type = type;
    memcpy((char *)&ep->key,(char *)key,sizeof(epkey_t));
    //ep->key = key;
}

static inline void epobj_key_update_phy_id(epkey_t *ekey, unsigned int phy_id)
{
    if(phy_id >= KEY_MAX_PHY_ID)
	return;
    ekey->phy_chan_id = phy_id;
}


static inline void epobj_key_update_lgc_id(epkey_t *ekey, unsigned int lgc_id)
{
    if(lgc_id >= KEY_MAX_LGC_ID)
	return;
    ekey->lgc_chan_id = lgc_id;
}


static inline unsigned long tc_round_up(unsigned long total,unsigned long unit_size)
{
    return  (total % unit_size)? (total / unit_size) + 1: total / unit_size;
}

static inline unsigned long tc_round_low(unsigned long total,unsigned long unit_size)
{
    return (total/unit_size);
}

static inline int get_stream_from_epobj(struct epobj_t * ept)
{
    return  (ept->key.stream);
}

static inline int get_algo_from_epobj(struct epobj_t * ept)
{
    return (ept->key.algo);
}

static inline int get_busid_from_epobj(struct epobj_t * ept)
{
    return (ept->key.bus_id);
}

static inline int get_chipid_from_epobj(struct epobj_t * ept)
{
    return (ept->key.chip_id);
}

static inline int get_phchan_id_from_epobj(struct epobj_t * ept)
{
    return (ept->key.phy_chan_id);
}

static inline int get_lgcchan_id_from_epobj(struct epobj_t * ept)
{
    return (ept->key.lgc_chan_id);
}

static inline void *tcd_priv(struct isil_chip_device *tcd)
{
    return (char *)tcd + ((sizeof(struct isil_chip_device) + CHIPDEV_ALIGN_MASK) & ~CHIPDEV_ALIGN_MASK);
}

static inline struct isil_chip_device *priv_2_tcd(void *priv)
{
    char *p =(char *)(priv) -((sizeof(struct isil_chip_device) + CHIPDEV_ALIGN_MASK) & ~CHIPDEV_ALIGN_MASK);

    return (struct isil_chip_device *)p;
}

static inline void *tsd_priv(struct isil_service_device *tsd)
{
    return (char *)tsd + ((sizeof(struct isil_service_device) + CHIPDEV_ALIGN_MASK) & ~CHIPDEV_ALIGN_MASK);
}

/*chip dev  ----c
 *type      ----t
 *video/a   ----v
 *bus id    ----b
 *channle id----l
 */

/*desc: find the correct tsd according to specified parameters
 *para: @tcd: from which chip device to find.
 *      @type:ISIL_SERVICE_TYPE
 *      @av_type:audio or video, 0:h264_video, 1:audio
 *      @channel:the logic channel id
 */
static inline struct isil_service_device* get_service_dev_by_ctvl(struct isil_chip_device *tcd, int type, int av_type, int channel)
{
    struct isil_service_device * tsd = NULL;
    struct list_head  *p,*h;

    if(type < 1)
	return NULL;

    h = &tcd->tsd_list[type-1];
    dbg("type = %d\n",type);
    list_for_each(p,h){
	tsd = container_of(p,struct isil_service_device, list);
	if(get_lgcchan_id_from_epobj(&tsd->epobj) == channel){
	    if(0 == av_type){
		if(CODEC_VIDEO_H264 == get_algo_from_epobj(&tsd->epobj))
		    return tsd;
	    }
	    if(1 == av_type){
		if(STREAM_TYPE_AUDIO == get_stream_from_epobj(&tsd->epobj))
		    return tsd;
	    }
	}
    }
    return NULL;
}


static inline int tc_align_up(int size, int align)
{
    int ret;

    ret = (size + (align-1)) & (~(align-1));
    return ret;

}

#ifdef STATIC_COUNTING
static inline void get_loop_start(struct isil_service_device *tsd)
{
    struct tc_buf_interval *tbi;

    tbi = (struct tc_buf_interval *)&tsd->b_itval;
    tbi->s_req_tick = get_isil_system_tick();	
}


static inline void calc_ic_req_time(struct isil_service_device *tsd)
{
    struct tc_buf_interval *tbi = (struct tc_buf_interval *)&tsd->b_itval;
    u32 tick;

    tick = get_isil_system_tick();
    if(tick >= tbi->s_req_tick)
	tbi->ic_req_time = tick - tbi->s_req_tick;
    else
	tbi->ic_req_time =  U32LONG_MAX - (tbi->s_req_tick - tick);
}

static inline void get_ic_map_start(struct isil_service_device *tsd)
{
    struct tc_buf_interval *tbi = (struct tc_buf_interval *)&tsd->b_itval;

    tbi->s_map_tick = get_isil_system_tick();        
}

static inline void calc_ic_map_time(struct isil_service_device *tsd)
{
    struct tc_buf_interval *tbi = (struct tc_buf_interval *)&tsd->b_itval;
    u32 tick;

    tick = get_isil_system_tick();
    if(tick >= tbi->s_map_tick)
	tbi->ic_map_time = tick - tbi->s_map_tick;
    else
	tbi->ic_map_time =  U32LONG_MAX - (tbi->s_map_tick - tick);
}

static inline void get_ic_rel_start(struct isil_service_device *tsd)
{
    struct tc_buf_interval *tbi = (struct tc_buf_interval *)&tsd->b_itval;

    tbi->s_rel_tick = get_isil_system_tick();
}

static inline void calc_ic_rel_time(struct isil_service_device *tsd)
{
    struct tc_buf_interval *tbi = (struct tc_buf_interval *)&tsd->b_itval;
    u32 tick;

    tick = get_isil_system_tick();
    if(tick >= tbi->s_rel_tick)
	tbi->ic_rel_time = tick - tbi->s_rel_tick;
    else
	tbi->ic_rel_time =  U32LONG_MAX - (tbi->s_rel_tick - tick);
}



static inline void get_loop_end(struct isil_service_device *tsd)
{
    struct tc_buf_interval *tbi;
    unsigned long tick,span;

    tbi = (struct tc_buf_interval *)&tsd->b_itval;
    tick = get_isil_system_tick();
    if(tick >= tbi->s_req_tick)
	span = tick - tbi->s_req_tick;
    else
	span = U32LONG_MAX - (tbi->s_req_tick - tick);

    tbi->ic4_total_time = span;
    if(span < tbi->min)	tbi->min = span;
    if(span > tbi->max)	tbi->max = span;
}
#endif


void lock_chipdev(void);
void unlock_chipdev(void);
struct list_head * get_chipdev_listhead(void);
int do_create_service_dev(struct isil_chip_device * tcd, struct cmd_arg * arg, struct kobject * dir);
int do_add_to_kernel(struct isil_chip_device * tcd, void *arg);
struct isil_chip_device * find_chip_dev(int minor);
struct isil_service_device * find_service_dev(int major,int minor);
int register_chip_device(struct isil_chip_device *tcd);
void unregister_chip_device(struct isil_chip_device * tcd);
int register_chip_driver(struct isil_chip_driver * driv);
void unregister_chip_driver(struct isil_chip_driver * driv);
int create_service_device(struct isil_chip_device * tcd, unsigned long data);
void release_service_device(struct isil_chip_device *tcd, void *data);
int register_tc_device(struct isil_dev_table_id *new);
void unregister_tc_device(struct isil_dev_table_id *tb);
int register_tc_driver(struct isil_service_driver * driv);
void unregister_tc_driver(struct isil_service_driver *driv);
int relink_tsd(struct isil_service_device *old, struct isil_service_device *newer);
struct isil_chip_device * tc_chipdev_alloc(unsigned long priv_size);
void tc_chipdev_free(struct isil_chip_device * tcd);
int chip_device_match(struct epobj_t *a, struct epobj_t *b);
void do_release_service_device(struct isil_service_device *tsd,void *data);
void release_all_service_device(void);
void release_all_service_device_on_chip(struct isil_chip_device *tcd);
void release_all_service_device_ext(void);
void release_all_service_device_ext_on_chip(struct isil_chip_device *tcd);

int isil_core_init(void);
void isil_core_exit(void);

//int  kprobe_init(void);
//void kprobe_exit(void);


#endif
