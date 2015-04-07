#ifndef __HALSOFTTIMERLIB_H
#define __HALSOFTTIMERLIB_H

#ifdef __cplusplus
extern "C"
{
#endif

#define	MAXSINGLEFIREQUEUENUM   (64)
#define	MAXFORFIREQUEUENUM      (64)

#define	ADDJOBERROR             (MAXSINGLEFIREQUEUENUM+2)
#define	GETFREETMCBERROR        ADDJOBERROR
#define	INVALIDTIMERID          ADDJOBERROR

#define	TIMER_INVALIDE	(-1)
#define	TIMER_VALIDE	(1)

    typedef	int	(*timerHook)(void* context);

    struct fortmcb
    {
        __s32       flag;
        __u32       count;
        __u32       percount;
        void*       context;
        timerHook   hook;
    };

    struct tmcb
    {
        __s32       flag;
        __u32       count;
        void*       context;
        timerHook   hook;
    };

    struct isil_chip_timer_controller_operation{
        void    (*init)(isil_chip_timer_controller_t *);
        u32     (*get_isil_chip_tick)(isil_chip_timer_controller_t *);
        void    (*update_isil_chip_tick)(isil_chip_timer_controller_t *);
        void    (*isr)(isil_chip_timer_controller_t *);

        void    (*ResetSignalTimer)(isil_chip_timer_controller_t *);
        int	(*AddSingleFireTimerJob)(isil_chip_timer_controller_t *, __u32 tick, timerHook fun, void* context);
        void	(*DeleteSingleFireTimerJob)(isil_chip_timer_controller_t *, int index);
        void    (*TigerSingleFireTimer)(isil_chip_timer_controller_t *);

        void    (*ResetForTimer)(isil_chip_timer_controller_t *);
        int	(*AddForFireTimerJob)(isil_chip_timer_controller_t *, __u32 tick, timerHook fun, void* context);
        void	(*DeleteForFireTimerJob)(isil_chip_timer_controller_t *, int index);
        void    (*TigerForFireTimer)(isil_chip_timer_controller_t *);
    };

    typedef struct osd_system_timer osd_system_timer_t;
    struct osd_system_timer{
	short   year;
	short   month;
	char    day;
	char    hour;
	char    minute;
	char    second;
	u32     need_update_map_table;
    };

    struct isil_chip_timer_controller
    {
        TMCB            SingleFireQueue[MAXSINGLEFIREQUEUENUM];
        FORTMCB         ForFireQueue[MAXFORFIREQUEUENUM];
        spinlock_t      softtimer_lock;
        unsigned int    isil_system_ticker;
        unsigned int    cur_walltime_sec;
        struct rtc_time system_time;
        struct timeval  s_osd_tv1;
        osd_system_timer_t  osd_timer;

        struct isil_chip_timer_controller_operation *op;
    };

    extern void get_chip_osd_timer(osd_system_timer_t **ptr_osd_timer, isil_chip_t *chip);
    extern void	init_isil_timer(isil_chip_timer_controller_t *chip_timer_cont);
    extern void	remove_isil_timer(isil_chip_timer_controller_t *chip_timer_cont);
    extern void init_system_walltimer(void);
    extern unsigned int get_system_walltimer(void);
#ifdef __cplusplus
}
#endif


#endif 


