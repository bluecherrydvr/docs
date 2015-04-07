#include	<isil5864/isil_common.h>

static void SignalTimeInit(isil_chip_timer_controller_t *chip_timer_cont);
static int  GetFreeSingleFireTmcb(isil_chip_timer_controller_t *chip_timer_cont);
static void PutFreeSingleFireTmcb(isil_chip_timer_controller_t *chip_timer_cont, int index);
static void ForTimeInit(isil_chip_timer_controller_t *chip_timer_cont);
static int  GetFreeForFireForTmcb(isil_chip_timer_controller_t *chip_timer_cont);
static void PutFreeForFireTmcb(isil_chip_timer_controller_t *chip_timer_cont, int index);

const unsigned int  sec_of_1day = 86400;
const unsigned int  sec_of_366day = 31622400;
const unsigned int  sec_of_365day = 31536000;
const unsigned int  sec_of_31day = 2678400;
const unsigned int  sec_of_30day = 2592000;
const unsigned int  sec_of_29day = 2505600;
const unsigned int  sec_of_28day = 2419200;
const unsigned int  sec_of_2000_to_2007 = 252460800;
const unsigned int  sec_of_1970_to_1999 = 946684800;
const unsigned int  sec_of_1970_to_2007 = 1199145600;

static struct timeval   old_system_walltimer, curr_system_walltimer;
static unsigned int     system_walltimer_counter;

void    init_system_walltimer(void)
{
    do_gettimeofday(&curr_system_walltimer);
    old_system_walltimer.tv_sec = curr_system_walltimer.tv_sec;
    old_system_walltimer.tv_usec = curr_system_walltimer.tv_usec;
    system_walltimer_counter = 0;
}

static void update_system_walltimer(void)
{
    int deltaT;

    do_gettimeofday(&curr_system_walltimer);
    if(curr_system_walltimer.tv_sec < old_system_walltimer.tv_sec){
        deltaT = 20;
    } else if((curr_system_walltimer.tv_sec - old_system_walltimer.tv_sec) <= 1){
        deltaT = (((curr_system_walltimer.tv_sec)*1000) + ((curr_system_walltimer.tv_usec)/1000));
        deltaT -= (((old_system_walltimer.tv_sec)*1000) + ((old_system_walltimer.tv_usec)/1000));
    } else {
        deltaT = 20;
    }
    system_walltimer_counter += deltaT;
    old_system_walltimer.tv_sec = curr_system_walltimer.tv_sec;
    old_system_walltimer.tv_usec = curr_system_walltimer.tv_usec;
}

unsigned int    get_system_walltimer(void)
{
    return system_walltimer_counter;
}

static inline int is_run_year(int year)
{
    if((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)
        return 1;
    return 0;
}

static inline int get_day_of_month(int year, int month)
{
    switch(month) {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
            return 31;
        case 4:
        case 6:
        case 9:
        case 11:
            return 30;
        case 2:
            if (1 == is_run_year(year))
                return 29;
            else
                return 28;
        default:
            break;
    }
    return 0;
}

static void trans_sec_to_time(isil_chip_timer_controller_t *chip_timer_cont)
{
    struct rtc_time *system_time;
    int year = 0, month = 0, day = 0, hour = 0, min = 0, sec = 0;
    int in_sec = chip_timer_cont->cur_walltime_sec;

    if (in_sec < 0)
        return;
    if (chip_timer_cont->cur_walltime_sec > sec_of_1970_to_2007) {
        year = 2008;
        in_sec -= sec_of_1970_to_2007;
    } else if (chip_timer_cont->cur_walltime_sec > sec_of_1970_to_1999) {
        year = 2000;
        in_sec -= sec_of_1970_to_1999;
    } else
        year = 1970;
    for (;;year++) {
        if (1 == is_run_year(year)) {
            if (in_sec >= sec_of_366day) {
                in_sec -= sec_of_366day;
            } else
                break;
        } else {
            if (in_sec >= sec_of_365day) {
                in_sec -= sec_of_365day;
            } else
                break;
        }
    }
    month = 1;
    for (;;month++) {
        if (31 == get_day_of_month(year, month)) {
            if (in_sec >= sec_of_31day) {
                in_sec -= sec_of_31day;	
            } else
                break;
        } else if (30 == get_day_of_month(year, month)) {
            if (in_sec >= sec_of_30day) {
                in_sec -= sec_of_30day;	
            } else
                break;
        } else if (28 == get_day_of_month(year, month)) {
            if (in_sec >= sec_of_28day) {
                in_sec -= sec_of_28day;	
            } else
                break;
        } else if (29 == get_day_of_month(year, month)) {
            if (in_sec >= sec_of_29day) {
                in_sec -= sec_of_29day;
            } else
                break;
        }
    }
    day = 1;
    for (;;day++) {
        if (in_sec >= sec_of_1day) {
            in_sec -= sec_of_1day;
        } else
            break;
    }
    hour = 0;
    for (;;hour++) {
        if (in_sec >= 3600) {
            in_sec -= 3600;
        } else
            break;
    }
    min = 0;
    for (;;min++) {
        if (in_sec >= 60) {
            in_sec -= 60;
        } else
            break;
    }
    sec = in_sec;

    system_time = &chip_timer_cont->system_time;
    system_time->tm_year = year;
    system_time->tm_mon = month;
    system_time->tm_mday = day;
    system_time->tm_hour = hour;
    system_time->tm_min = min;
    system_time->tm_sec = sec;
}

static void refresh_walltime_of_sec(isil_chip_timer_controller_t *chip_timer_cont)
{
    struct rtc_time *system_time;
    struct timeval  *tv1;
    unsigned int new_walltime_sec;

    system_time = &chip_timer_cont->system_time;
    tv1 = &chip_timer_cont->s_osd_tv1;
    do_gettimeofday(tv1);
    new_walltime_sec = (unsigned int)tv1->tv_sec;
    if(chip_timer_cont->cur_walltime_sec == new_walltime_sec)
        return;
    chip_timer_cont->cur_walltime_sec = new_walltime_sec;
    if ((new_walltime_sec - chip_timer_cont->cur_walltime_sec == 1)
            && (system_time->tm_hour < 23 || system_time->tm_min < 59 || system_time->tm_sec < 59)) {
        system_time->tm_sec++;
        if (system_time->tm_sec == 60) {
            system_time->tm_sec = 0;
            system_time->tm_min++;
            if (system_time->tm_min == 60) {
                system_time->tm_min = 0;
                system_time->tm_hour++;
            }
        }
    } else {
        trans_sec_to_time(chip_timer_cont);
    }
}

static void change_encode_osd_time(isil_chip_timer_controller_t *chip_timer_cont)
{
    osd_system_timer_t  *osd_timer;
    struct rtc_time     *timer;
    int	flag=0;
    osd_timer = &chip_timer_cont->osd_timer;
    timer = &chip_timer_cont->system_time;
    if(osd_timer->year != timer->tm_year){
        osd_timer->year = timer->tm_year;
        flag = 1;
    }
    if(osd_timer->month != timer->tm_mon){
        osd_timer->month = timer->tm_mon;
        flag = 1;
    }
    if(osd_timer->day != timer->tm_mday){
        osd_timer->day = timer->tm_mday;
        flag = 1;
    }
    if(osd_timer->hour != timer->tm_hour){
        osd_timer->hour = timer->tm_hour;
        flag = 1;
    }
    if(osd_timer->minute != timer->tm_min){
        osd_timer->minute = timer->tm_min;
        flag = 1;
    }
    if(osd_timer->second != timer->tm_sec){
        osd_timer->second = timer->tm_sec;
        flag = 1;
    }
    if(flag){
        osd_timer->need_update_map_table = 0xffffffff;
    }
}

static void check_rtc_time(isil_chip_timer_controller_t *chip_timer_cont)
{
    refresh_walltime_of_sec(chip_timer_cont);
    change_encode_osd_time(chip_timer_cont);
}

void    get_chip_osd_timer(osd_system_timer_t **ptr_osd_timer, isil_chip_t *chip)
{
    isil_chip_timer_controller_t *chip_timer_cont = &chip->chip_timer_cont;
    *ptr_osd_timer = &chip_timer_cont->osd_timer;
}

static void SignalTimeInit(isil_chip_timer_controller_t *chip_timer_cont)
{
    int	i;
    unsigned long   flags;

    spin_lock_irqsave(&chip_timer_cont->softtimer_lock, flags);
    for(i=0; i<MAXSINGLEFIREQUEUENUM; i++)
        PutFreeSingleFireTmcb(chip_timer_cont, i);
    spin_unlock_irqrestore(&chip_timer_cont->softtimer_lock, flags);
}

static int  GetFreeSingleFireTmcb(isil_chip_timer_controller_t *chip_timer_cont)
{
    TMCB    *ptr;
    int	i;

    ptr = chip_timer_cont->SingleFireQueue;
    for(i=0; i<MAXSINGLEFIREQUEUENUM; i++)
    {
        if(ptr->flag == TIMER_INVALIDE)
            return i;
        ptr++;
    }
    return GETFREETMCBERROR;
}

static void PutFreeSingleFireTmcb(isil_chip_timer_controller_t *chip_timer_cont, int index)
{
    TMCB	*ptr;

    if((index >= MAXSINGLEFIREQUEUENUM) || (index < 0))
        return;
    ptr = &chip_timer_cont->SingleFireQueue[index];
    ptr->hook = (void*)0;
    ptr->flag = TIMER_INVALIDE;
    ptr->count = 0;
}

static void ForTimeInit(isil_chip_timer_controller_t *chip_timer_cont)
{
    int	i;
    unsigned long   flags;

    spin_lock_irqsave(&chip_timer_cont->softtimer_lock, flags);
    for(i=0; i<MAXFORFIREQUEUENUM; i++)
        PutFreeForFireTmcb(chip_timer_cont, i);
    spin_unlock_irqrestore(&chip_timer_cont->softtimer_lock, flags);
}

static int  GetFreeForFireForTmcb(isil_chip_timer_controller_t *chip_timer_cont)
{
    FORTMCB *ptr;
    int	i;

    ptr = chip_timer_cont->ForFireQueue;
    for(i=0; i<MAXFORFIREQUEUENUM; i++)
    {
        if(ptr->flag == TIMER_INVALIDE)
            return i;
        ptr++;
    }
    return GETFREETMCBERROR;
}

static void PutFreeForFireTmcb(isil_chip_timer_controller_t *chip_timer_cont, int index)
{
    FORTMCB *ptr;

    if((index >= MAXSINGLEFIREQUEUENUM) || (index < 0))
        return;
    ptr = &chip_timer_cont->ForFireQueue[index];
    ptr->hook = (void*)0;
    ptr->flag = TIMER_INVALIDE;
    ptr->count = 0;
    ptr->percount = 0;
}

static void isil_chip_timer_controller_init(isil_chip_timer_controller_t *chip_timer_cont)
{
    osd_system_timer_t  *osd_timer;

    spin_lock_init(&chip_timer_cont->softtimer_lock);
    chip_timer_cont->isil_system_ticker = 0;
    SignalTimeInit(chip_timer_cont);
    ForTimeInit(chip_timer_cont);
    osd_timer = &chip_timer_cont->osd_timer;
    osd_timer->year = 1970;
    osd_timer->month = 1;
    osd_timer->day = 1;
    osd_timer->hour = 0;
    osd_timer->minute = 0;
    osd_timer->second = 0;
    osd_timer->need_update_map_table = 0xffffffff;
}

static u32  isil_chip_timer_controller_get_isil_chip_tick(isil_chip_timer_controller_t *chip_timer_cont)
{
    return chip_timer_cont->isil_system_ticker;
}

static void isil_chip_timer_controller_update_isil_chip_tick(isil_chip_timer_controller_t *chip_timer_cont)
{
    chip_timer_cont->isil_system_ticker++;
}

static void isil_chip_timer_controller_isr(isil_chip_timer_controller_t *chip_timer_cont)
{
    chip_timer_cont->op->update_isil_chip_tick(chip_timer_cont);
    chip_timer_cont->op->TigerSingleFireTimer(chip_timer_cont);
    chip_timer_cont->op->TigerForFireTimer(chip_timer_cont);
    check_rtc_time(chip_timer_cont);
    update_system_walltimer();
}

static void isil_chip_timer_controller_ResetSignalTimer(isil_chip_timer_controller_t *chip_timer_cont)
{
    SignalTimeInit(chip_timer_cont);
}

static int  isil_chip_timer_controller_AddSingleFireTimerJob(isil_chip_timer_controller_t *chip_timer_cont, __u32 tick, timerHook fun, void* context)
{
    TMCB    *ptr;
    int     index;
    unsigned long   flags;

    if((fun == (void*)0) || (tick == 0))
        return ADDJOBERROR;
    spin_lock_irqsave(&chip_timer_cont->softtimer_lock, flags);
    index = GetFreeSingleFireTmcb(chip_timer_cont);
    if(index == GETFREETMCBERROR)
    {
        spin_unlock_irqrestore(&chip_timer_cont->softtimer_lock, flags);
        return ADDJOBERROR;
    }
    ptr = &chip_timer_cont->SingleFireQueue[index];
    ptr->flag = TIMER_VALIDE;
    ptr->count = tick;
    ptr->context = context;
    ptr->hook = fun;
    spin_unlock_irqrestore(&chip_timer_cont->softtimer_lock, flags);
    return index;

}

static void isil_chip_timer_controller_DeleteSingleFireTimerJob(isil_chip_timer_controller_t *chip_timer_cont, int index)
{
    unsigned long   flags;

    spin_lock_irqsave(&chip_timer_cont->softtimer_lock, flags);
    PutFreeSingleFireTmcb(chip_timer_cont, index);
    spin_unlock_irqrestore(&chip_timer_cont->softtimer_lock, flags);
}

static void isil_chip_timer_controller_TigerSingleFireTimer(isil_chip_timer_controller_t *chip_timer_cont)
{
    TMCB    *ptr;
    int     i;

    ptr = chip_timer_cont->SingleFireQueue;
    for(i=0; i<MAXSINGLEFIREQUEUENUM; i++)
    {
        if(ptr->flag == TIMER_VALIDE)
        {
            if(ptr->hook != (void*)0)
            {
                if(ptr->count > 0)
                    ptr->count--;
                if(ptr->count == 0)
                {
                    struct timeval tv_base, tv_curr;
                    u32 duration = 0;

                    do_gettimeofday(&tv_base);
                    ptr->hook(ptr->context);
                    do_gettimeofday(&tv_curr);
                    duration = ((tv_curr.tv_sec - tv_base.tv_sec) * 1000 * 1000) + (tv_curr.tv_usec - tv_base.tv_usec); 
                    if(duration > 2000){
                        ISIL_DBG(ISIL_DBG_WARN, "duration %d us, at %pS\n", duration, ptr->hook);

                    }
                    isil_chip_timer_controller_DeleteSingleFireTimerJob(chip_timer_cont, i);
                }
            } else {
                isil_chip_timer_controller_DeleteSingleFireTimerJob(chip_timer_cont, i);
            }
        }
        ptr++;
    }
}

static void isil_chip_timer_controller_ResetForTimer(isil_chip_timer_controller_t *chip_timer_cont)
{
    ForTimeInit(chip_timer_cont);
}

static int  isil_chip_timer_controller_AddForFireTimerJob(isil_chip_timer_controller_t *chip_timer_cont, __u32 tick, timerHook fun, void* context)
{
    FORTMCB *ptr;
    int     index;
    unsigned long   flags;

    if((fun == (void*)0) || (tick == 0))
        return ADDJOBERROR;
    spin_lock_irqsave(&chip_timer_cont->softtimer_lock, flags);
    index = GetFreeForFireForTmcb(chip_timer_cont);
    if(index == GETFREETMCBERROR)
    {
        spin_unlock_irqrestore(&chip_timer_cont->softtimer_lock, flags);
        return ADDJOBERROR;
    }
    ptr = &chip_timer_cont->ForFireQueue[index];
    ptr->flag = TIMER_VALIDE;
    ptr->percount = ptr->count = tick;
    ptr->context = context;
    ptr->hook = fun;
    spin_unlock_irqrestore(&chip_timer_cont->softtimer_lock, flags);
    return index;
}

static void isil_chip_timer_controller_DeleteForFireTimerJob(isil_chip_timer_controller_t *chip_timer_cont, int index)
{
    unsigned long   flags;

    spin_lock_irqsave(&chip_timer_cont->softtimer_lock, flags);
    PutFreeForFireTmcb(chip_timer_cont, index);
    spin_unlock_irqrestore(&chip_timer_cont->softtimer_lock, flags);
}

static void isil_chip_timer_controller_TigerForFireTimer(isil_chip_timer_controller_t *chip_timer_cont)
{
    FORTMCB *ptr;
    int     i;

    ptr = chip_timer_cont->ForFireQueue;
    for(i=0; i<MAXFORFIREQUEUENUM; i++)
    {
        if(ptr->flag == TIMER_VALIDE)
        {
            if(ptr->hook != (void*)0)
            {
                if(ptr->count > 0)
                    ptr->count--;
                if(ptr->count == 0)
                {
                    struct timeval tv_base, tv_curr;
                    u32 duration = 0;

                    do_gettimeofday(&tv_base);
                    ptr->hook(ptr->context);
                    do_gettimeofday(&tv_curr);

                    duration = ((tv_curr.tv_sec - tv_base.tv_sec) * 1000 * 1000) + (tv_curr.tv_usec - tv_base.tv_usec); 
                    if(duration > 2000){
                        ISIL_DBG(ISIL_DBG_WARN, "duration %d us, at %pS\n", duration, ptr->hook);
                    }
                    ptr->count = ptr->percount;
                }
            } else {
                isil_chip_timer_controller_DeleteForFireTimerJob(chip_timer_cont, i);
            }
        }
        ptr++;
    }
}

static struct isil_chip_timer_controller_operation  isil_chip_timer_controller_op = {
    .init = isil_chip_timer_controller_init,
    .get_isil_chip_tick = isil_chip_timer_controller_get_isil_chip_tick,
    .update_isil_chip_tick = isil_chip_timer_controller_update_isil_chip_tick,
    .isr = isil_chip_timer_controller_isr,

    .ResetSignalTimer = isil_chip_timer_controller_ResetSignalTimer,
    .AddSingleFireTimerJob = isil_chip_timer_controller_AddSingleFireTimerJob,
    .DeleteSingleFireTimerJob = isil_chip_timer_controller_DeleteSingleFireTimerJob,
    .TigerSingleFireTimer = isil_chip_timer_controller_TigerSingleFireTimer,

    .ResetForTimer = isil_chip_timer_controller_ResetForTimer,
    .AddForFireTimerJob = isil_chip_timer_controller_AddForFireTimerJob,
    .DeleteForFireTimerJob = isil_chip_timer_controller_DeleteForFireTimerJob,
    .TigerForFireTimer = isil_chip_timer_controller_TigerForFireTimer,
};

void    init_isil_timer(isil_chip_timer_controller_t *chip_timer_cont)
{
    chip_timer_cont->op = &isil_chip_timer_controller_op;
    chip_timer_cont->op->init(chip_timer_cont);
}

void	remove_isil_timer(isil_chip_timer_controller_t *chip_timer_cont)
{

}

static isil_kthread_msg_queue_t	isil_msg_queue;
static isil_kthread_msg_pool_t	isil_msg_pool;

void    get_msg_queue_header(isil_kthread_msg_queue_t **ptr_queue)
{
    *ptr_queue = &isil_msg_queue;
}

void    get_msg_pool_header(isil_kthread_msg_pool_t **msg_pool)
{
    *msg_pool = &isil_msg_pool;
}

static void isil_kthread_msg_init(isil_kthread_msg_t *msg, isil_kthread_msg_pool_t *msg_pool)
{
    if((msg!=NULL) && (msg_pool!=NULL)){
        tcb_node_t	*node = &msg->msg_node;
        node->op = &tcb_node_op;
        node->op->init(node);
        node->op->set_priv(node, msg);
        atomic_set(&msg->ref_count, 0);
        spin_lock_init(&msg->lock);
        msg->msg_pool = msg_pool;
        msg->chip = NULL;
        msg->msg_type = REQ_ALGO_NULL;
        msg->msg_context = NULL;
        msg->owner_context = NULL;
    }
}

static void isil_kthread_msg_reset(isil_kthread_msg_t *msg)
{
    if(msg != NULL){
        atomic_set(&msg->ref_count, 0);
        msg->chip = NULL;
        msg->msg_type = REQ_ALGO_NULL;
        msg->msg_context = NULL;
        msg->owner_context = NULL;
    }
}

static void isil_kthread_msg_release(isil_kthread_msg_t **ptr_msg)
{
    if(ptr_msg != NULL){
        isil_kthread_msg_t *msg = *ptr_msg;
        unsigned long	flags;
        spin_lock_irqsave(&msg->lock, flags);
        if(atomic_read(&msg->ref_count) > 1){
            atomic_dec(&msg->ref_count);
        } else {
            tcb_node_t	*node;
            if(msg->op != NULL){
                msg->op->reset(msg);
            }
            node = &msg->msg_node;
            if(node->op != NULL){
                node->op->release(node, &msg->msg_pool->pool_node);
            }
            *ptr_msg = NULL;
        }
        spin_unlock_irqrestore(&msg->lock, flags);
    }
}

static void isil_kthread_msg_update_param(isil_kthread_msg_t *msg, isil_chip_t *chip, int msg_type, void *context, void *owner_context)
{
    if(msg != NULL){
        msg->chip = chip;
        msg->msg_type = msg_type;
        msg->msg_context = context;
        msg->owner_context = owner_context;
    }
}

static struct isil_kthread_msg_operation	isil_kthread_msg_op = {
    .init = isil_kthread_msg_init,
    .reset = isil_kthread_msg_reset,
    .release = isil_kthread_msg_release,
    .update_param = isil_kthread_msg_update_param,
};

static void isil_kthread_msg_queue_init(isil_kthread_msg_queue_t *msg_queue)
{
    if(msg_queue != NULL){
        tcb_node_queue_t    *queue;
        queue = &msg_queue->queue_node;
        queue->op = &tcb_node_queue_op;
        queue->op->init(queue);
        msg_queue->curr_consumer = NULL;
        spin_lock_init(&msg_queue->lock);
    }
}

static void isil_kthread_msg_queue_put(isil_kthread_msg_queue_t *msg_queue, isil_kthread_msg_t *msg)
{
    if((msg_queue!=NULL) && (msg!=NULL)){
        tcb_node_queue_t    *queue;
        queue = &msg_queue->queue_node;
        if(queue->op != NULL){
            queue->op->put(queue, &msg->msg_node);
        }
    }
}

static void isil_kthread_msg_queue_get(isil_kthread_msg_queue_t *msg_queue, isil_kthread_msg_t **ptr_msg)
{
    if((msg_queue!=NULL) && (ptr_msg!=NULL)){
        tcb_node_queue_t    *queue;
        queue = &msg_queue->queue_node;
        *ptr_msg = NULL;
        if(queue->op != NULL){
            tcb_node_t  *temp_node;
            queue->op->get(queue, &temp_node);
            if(temp_node != NULL){
                *ptr_msg = to_get_isil_kthread_msg(temp_node);
            }
        }
    }
}

static void isil_kthread_msg_queue_try_get(isil_kthread_msg_queue_t *msg_queue, isil_kthread_msg_t **ptr_msg)
{
    if((msg_queue!=NULL) && (ptr_msg!=NULL)){
        tcb_node_queue_t    *queue;
        queue = &msg_queue->queue_node;
        *ptr_msg = NULL;
        if(queue->op != NULL){
            tcb_node_t  *temp_node;
            queue->op->try_get(queue, &temp_node);
            if(temp_node != NULL){
                *ptr_msg = to_get_isil_kthread_msg(temp_node);
            }
        }
    }
}

static void isil_kthread_msg_queue_release(isil_kthread_msg_queue_t *msg_queue)
{
    if(msg_queue != NULL){
        msg_queue->op->release_curr_consumer(msg_queue);
        while(msg_queue->op->get_curr_queue_entry_number(msg_queue)){
            msg_queue->op->get_curr_consumer_from_queue(msg_queue);
            if(msg_queue->curr_consumer == NULL){
                break;
            }
            msg_queue->op->release_curr_consumer(msg_queue);
        }
    }
}

static void isil_kthread_msg_queue_get_curr_consumer_from_queue(isil_kthread_msg_queue_t *msg_queue)
{
    if(msg_queue != NULL){
        unsigned long	flags;
        spin_lock_irqsave(&msg_queue->lock, flags);
        if(msg_queue->curr_consumer == NULL){
            spin_unlock_irqrestore(&msg_queue->lock, flags);
            if(msg_queue->op != NULL){
                msg_queue->op->get(msg_queue, &msg_queue->curr_consumer);
            } else {
                isil_kthread_msg_queue_get(msg_queue, &msg_queue->curr_consumer);
            }
            spin_lock_irqsave(&msg_queue->lock, flags);
        }
        spin_unlock_irqrestore(&msg_queue->lock, flags);
    }
}

static void	isil_kthread_msg_queue_release_curr_consumer(isil_kthread_msg_queue_t *msg_queue)
{
    if(msg_queue != NULL){
        unsigned long	flags;
        spin_lock_irqsave(&msg_queue->lock, flags);
        if(msg_queue->curr_consumer != NULL){
            if(msg_queue->curr_consumer->op != NULL){
                msg_queue->curr_consumer->op->release(&msg_queue->curr_consumer);
            } else {
                isil_kthread_msg_release(&msg_queue->curr_consumer);
            }
        }
        spin_unlock_irqrestore(&msg_queue->lock, flags);
    }
}

static int	isil_kthread_msg_queue_get_curr_queue_entry_number(isil_kthread_msg_queue_t *msg_queue)
{
    int	ret = 0;
    if(msg_queue != NULL){
        ret = msg_queue->queue_node.op->get_queue_curr_entry_number(&msg_queue->queue_node);
    }
    return ret;
}

struct isil_kthread_msg_queue_operation	isil_kthread_msg_queue_op =
{
    .init = isil_kthread_msg_queue_init,
    .put = isil_kthread_msg_queue_put,
    .get = isil_kthread_msg_queue_get,
    .try_get = isil_kthread_msg_queue_try_get,
    .release = isil_kthread_msg_queue_release,

    .get_curr_consumer_from_queue = isil_kthread_msg_queue_get_curr_consumer_from_queue,
    .release_curr_consumer = isil_kthread_msg_queue_release_curr_consumer,
    .get_curr_queue_entry_number = isil_kthread_msg_queue_get_curr_queue_entry_number,
};

void    init_isil_kthread_msg_queue(isil_kthread_msg_queue_t *msg_queue)
{
    if(msg_queue != NULL){
        msg_queue->op = &isil_kthread_msg_queue_op;
        msg_queue->op->init(msg_queue);
    }
}

void    delete_all_msg_of_this_h264_encode_chan(isil_kthread_msg_queue_t *msg_queue, isil_h264_logic_encode_chan_t *encode_chan)
{
    if(msg_queue != NULL){
        isil_kthread_msg_t  *msg;
        unsigned long   flags;
        int             msg_number;

        spin_lock_irqsave(&msg_queue->lock, flags);
        msg_number = msg_queue->op->get_curr_queue_entry_number(msg_queue);
        while(msg_number){
            msg_number--;
            msg_queue->op->try_get(msg_queue, &msg);
            if(msg != NULL){
                switch(msg->msg_type){
                    case REQ_ALGO_SLICE_HEAD:
                    case REQ_AV_SYNC:
                    case REQ_ALGO_CHIP_RESET:
                        if((void *)encode_chan == msg->owner_context){
                            msg->op->release(&msg);
                        } else {
                            msg_queue->op->put(msg_queue, msg);
                        }
                        break;
                    default:
                        msg_queue->op->put(msg_queue, msg);
                        break;
                }
            }
        }
        spin_unlock_irqrestore(&msg_queue->lock, flags);
    }
}

void    delete_all_msg_of_this_chip(isil_kthread_msg_queue_t *msg_queue, isil_chip_t *chip)
{
    if(msg_queue != NULL){
        isil_kthread_msg_t  *msg;
        unsigned long   flags;
        int             msg_number;

        spin_lock_irqsave(&msg_queue->lock, flags);
        msg_number = msg_queue->op->get_curr_queue_entry_number(msg_queue);
        while(msg_number){
            msg_number--;
            msg_queue->op->try_get(msg_queue, &msg);
            if(msg != NULL){
                switch(msg->msg_type){
                    case REQ_ALGO_SLICE_HEAD:
                        if(chip == msg->chip){
                            msg->op->release(&msg);
                        } else {
                            msg_queue->op->put(msg_queue, msg);
                        }
                        break;
                    case REQ_ALGO_CHIP_RESET:
                        if(chip == msg->chip){
                            msg->op->release(&msg);
                        } else {
                            msg_queue->op->put(msg_queue, msg);
                        }
                        break;
                    default:
                        msg_queue->op->put(msg_queue, msg);
                        break;
                }
            }
        }
        spin_unlock_irqrestore(&msg_queue->lock, flags);
    }
}

void    remove_isil_kthread_msg_queue(isil_kthread_msg_queue_t *msg_queue)
{
    if(msg_queue != NULL){
        msg_queue->op->release(msg_queue);
    }
}

static void isil_kthread_msg_pool_get(isil_kthread_msg_pool_t *msg_pool, isil_kthread_msg_t **ptr_msg)
{
    if((msg_pool!=NULL) && (ptr_msg!=NULL)){
        tcb_node_pool_t *pool_node = &msg_pool->pool_node;
        *ptr_msg = NULL;
        if(pool_node->op != NULL){
            tcb_node_t	*temp_node;
            pool_node->op->get(pool_node, &temp_node);
            if(temp_node != NULL){
                *ptr_msg = to_get_isil_kthread_msg(temp_node);
                atomic_inc(&((*ptr_msg)->ref_count));
            }
        }
    }
}

static void isil_kthread_msg_pool_try_get(isil_kthread_msg_pool_t *msg_pool, isil_kthread_msg_t **ptr_msg)
{
    if((msg_pool!=NULL) && (ptr_msg!=NULL)){
        tcb_node_pool_t *pool_node = &msg_pool->pool_node;
        *ptr_msg = NULL;
        if(pool_node->op != NULL){
            tcb_node_t  *temp_node;
            pool_node->op->try_get(pool_node, &temp_node);
            if(temp_node != NULL){
                *ptr_msg = to_get_isil_kthread_msg(temp_node);
                atomic_inc(&((*ptr_msg)->ref_count));
            }
        }
    }
}

static void isil_kthread_msg_pool_put(isil_kthread_msg_pool_t *msg_pool, isil_kthread_msg_t *msg)
{
    if((msg_pool!=NULL) && (msg!=NULL)){
        tcb_node_pool_t *pool_node = &msg_pool->pool_node;
        if(pool_node->op != NULL){
            pool_node->op->put(pool_node, &msg->msg_node);
        }
    }
}

static void isil_kthread_msg_pool_release(isil_kthread_msg_pool_t *msg_pool)
{
    if(msg_pool != NULL){
        msg_pool->pool_node.op->release(&msg_pool->pool_node);
        if(msg_pool->msg_cache_pool != NULL){
            kfree(msg_pool->msg_cache_pool);
            msg_pool->msg_cache_pool = NULL;
        }
        msg_pool->msg_entry_number = 0;
    }
}

static int  isil_kthread_msg_pool_init(isil_kthread_msg_pool_t *msg_pool)
{
    int	ret = -ENOMEM;
    if(msg_pool != NULL){
        tcb_node_pool_t     *pool_node = &msg_pool->pool_node;
        isil_kthread_msg_t    *msg;
        int i;

        pool_node->op = &tcb_node_pool_op;
        msg_pool->msg_entry_number = 128;
        msg_pool->msg_cache_pool = (isil_kthread_msg_t*)kmalloc(sizeof(isil_kthread_msg_t)*msg_pool->msg_entry_number, GFP_KERNEL);
        if(msg_pool->msg_cache_pool == NULL){
            printk("%s.%d: can't create msg pool\n", __FUNCTION__, __LINE__);
        } else {
            pool_node->op->init(pool_node, msg_pool->msg_entry_number);
            for(i=0; i<msg_pool->msg_entry_number; i++) {
                msg = &msg_pool->msg_cache_pool[i];
                msg->op = &isil_kthread_msg_op;
                msg->op->init(msg, msg_pool);
                msg->op->release(&msg);
            }
            ret = 0;
        }
    }
    return ret;
}

static struct isil_kthread_msg_pool_operation isil_kthread_msg_pool_op =
{
    .get = isil_kthread_msg_pool_get,
    .try_get = isil_kthread_msg_pool_try_get,
    .put = isil_kthread_msg_pool_put,
    .release = isil_kthread_msg_pool_release,
    .init = isil_kthread_msg_pool_init,
};

int init_isil_kthread_msg_pool(isil_kthread_msg_pool_t *msg_pool)
{
    int	ret = -ENOMEM;
    if(msg_pool != NULL){
        msg_pool->op = &isil_kthread_msg_pool_op;
        ret = msg_pool->op->init(msg_pool);
    }
    return ret;
}

void    remove_isil_kthread_msg_pool(isil_kthread_msg_pool_t *msg_pool)
{
    if(msg_pool != NULL){
        msg_pool->op->release(msg_pool);
    }
}

static int  gen_driver_req_msg(isil_send_msg_controller_t *send_msg_contr)
{
    isil_kthread_msg_pool_t   *isil_msg_pool;
    isil_kthread_msg_t        *isil_msg;
    int	ret = -1;

    get_msg_pool_header(&isil_msg_pool);
    isil_msg_pool->op->get(isil_msg_pool, &isil_msg);
    if(isil_msg!= NULL) {
        isil_kthread_msg_queue_t    *isil_msg_queue;
        get_msg_queue_header(&isil_msg_queue);
        isil_msg->op->update_param(isil_msg, send_msg_contr->chip, send_msg_contr->msg_type, send_msg_contr->context, send_msg_contr->owner_context);
        isil_msg_queue->op->put(isil_msg_queue, isil_msg);
        ret = 0;
    }
    return ret;
}

static int  try_gen_driver_req_msg(isil_send_msg_controller_t *send_msg_contr)
{
    isil_kthread_msg_pool_t   *isil_msg_pool;
    isil_kthread_msg_t        *isil_msg;
    int	ret = -1;

    get_msg_pool_header(&isil_msg_pool);
    isil_msg_pool->op->try_get(isil_msg_pool, &isil_msg);
    if(isil_msg!= NULL) {
        isil_kthread_msg_queue_t    *isil_msg_queue;
        get_msg_queue_header(&isil_msg_queue);
        isil_msg->op->update_param(isil_msg, send_msg_contr->chip, send_msg_contr->msg_type, send_msg_contr->context, send_msg_contr->owner_context);
        isil_msg_queue->op->put(isil_msg_queue, isil_msg);
        ret = 0;
    }
    return ret;
}

static int  noblock_gen_slicehead_req_msg(void *context)
{
    isil_send_msg_controller_t *send_msg_contr = (isil_send_msg_controller_t*)context;
    isil_chip_t *chip = send_msg_contr->chip;
    isil_chip_timer_controller_t    *chip_timer_cont = &chip->chip_timer_cont;

    if(try_gen_driver_req_msg(send_msg_contr) != 0){
        send_msg_contr->nonblock_msg_timer_id[REQ_ALGO_SLICE_HEAD] = chip_timer_cont->op->AddSingleFireTimerJob(chip_timer_cont, NONBLOCK_COUNT, noblock_gen_slicehead_req_msg, send_msg_contr);
        if(send_msg_contr->nonblock_msg_timer_id[REQ_ALGO_SLICE_HEAD] == INVALIDTIMERID){
            printk("\n\n**************gen slicehead req msg fail[%d]***********\n\n", __LINE__);
            return -1;
        }
    } else {
        send_msg_contr->nonblock_msg_timer_id[REQ_ALGO_SLICE_HEAD] = INVALIDTIMERID;
    }
    return 0;
}

static int  noblock_gen_avSync_req_msg(void *context)
{
    isil_send_msg_controller_t *send_msg_contr = (isil_send_msg_controller_t*)context;
    isil_chip_t *chip = send_msg_contr->chip;
    isil_chip_timer_controller_t    *chip_timer_cont = &chip->chip_timer_cont;

    if(try_gen_driver_req_msg(send_msg_contr) != 0){
        send_msg_contr->nonblock_msg_timer_id[REQ_AV_SYNC] = chip_timer_cont->op->AddSingleFireTimerJob(chip_timer_cont, NONBLOCK_COUNT, noblock_gen_avSync_req_msg, send_msg_contr);
        if(send_msg_contr->nonblock_msg_timer_id[REQ_AV_SYNC] == INVALIDTIMERID){
            printk("\n\n**************gen avSync req msg fail[%d]***********\n\n", __LINE__);
            return -1;
        }
    } else {
        send_msg_contr->nonblock_msg_timer_id[REQ_AV_SYNC] = INVALIDTIMERID;
    }
    return 0;
}

static int  noblock_gen_chip_reset_req_msg(void *context)
{
    isil_send_msg_controller_t *send_msg_contr = (isil_send_msg_controller_t*)context;
    isil_chip_t *chip = send_msg_contr->chip;
    isil_chip_timer_controller_t    *chip_timer_cont = &chip->chip_timer_cont;

    if(try_gen_driver_req_msg(send_msg_contr) != 0){
        send_msg_contr->nonblock_msg_timer_id[REQ_ALGO_CHIP_RESET] = chip_timer_cont->op->AddSingleFireTimerJob(chip_timer_cont, NONBLOCK_COUNT, noblock_gen_chip_reset_req_msg, send_msg_contr);
        if(send_msg_contr->nonblock_msg_timer_id[REQ_ALGO_CHIP_RESET] == INVALIDTIMERID){
            printk("\n\n**************gen chip_reset req msg fail[%d]***********\n\n", __LINE__);
            return -1;
        }
    } else {
        send_msg_contr->nonblock_msg_timer_id[REQ_ALGO_CHIP_RESET] = INVALIDTIMERID;
    }
    return 0;
}

static int  noblock_gen_video_standard_changed_req_msg(void *context)
{
    isil_send_msg_controller_t *send_msg_contr = (isil_send_msg_controller_t*)context;
    isil_chip_t *chip = send_msg_contr->chip;
    isil_chip_timer_controller_t    *chip_timer_cont = &chip->chip_timer_cont;

    if(try_gen_driver_req_msg(send_msg_contr) != 0){
        send_msg_contr->nonblock_msg_timer_id[REQ_VIDEO_STANDARD_CHANGED] = chip_timer_cont->op->AddSingleFireTimerJob(chip_timer_cont, NONBLOCK_COUNT, noblock_gen_video_standard_changed_req_msg, send_msg_contr);
        if(send_msg_contr->nonblock_msg_timer_id[REQ_VIDEO_STANDARD_CHANGED] == INVALIDTIMERID){
            printk("\n\n**************gen REQ_VIDEO_STANDARD_CHANGED req msg fail[%d]***********\n\n", __LINE__);
            return -1;
        }
    } else {
        send_msg_contr->nonblock_msg_timer_id[REQ_VIDEO_STANDARD_CHANGED] = INVALIDTIMERID;
    }
    return 0;
}

static int  delay_gen_slicehead_req_msg(void *context)
{
    isil_send_msg_controller_t *send_msg_contr = (isil_send_msg_controller_t*)context;
    isil_chip_t *chip = send_msg_contr->chip;
    isil_chip_timer_controller_t    *chip_timer_cont = &chip->chip_timer_cont;

    send_msg_contr->delay_msg_timer_id[REQ_ALGO_SLICE_HEAD] = INVALIDTIMERID;
    if(try_gen_driver_req_msg(send_msg_contr) != 0){
        send_msg_contr->delay_msg_timer_id[REQ_ALGO_SLICE_HEAD] = chip_timer_cont->op->AddSingleFireTimerJob(chip_timer_cont, DELAY_COUNT, delay_gen_slicehead_req_msg, send_msg_contr);	
        if(send_msg_contr->delay_msg_timer_id[REQ_ALGO_SLICE_HEAD] == INVALIDTIMERID){
            printk("\n\n&&&&&&&&&&&&&gen delay slicehead req msg fail[%d]***********\n\n", __LINE__);
        }
    }
    return 0;
}

static int  delay_gen_avSync_req_msg(void *context)
{
    isil_send_msg_controller_t *send_msg_contr = (isil_send_msg_controller_t*)context;
    isil_chip_t *chip = send_msg_contr->chip;
    isil_chip_timer_controller_t    *chip_timer_cont = &chip->chip_timer_cont;

    send_msg_contr->delay_msg_timer_id[REQ_AV_SYNC] = INVALIDTIMERID;
    if(try_gen_driver_req_msg(send_msg_contr) != 0){
        send_msg_contr->delay_msg_timer_id[REQ_AV_SYNC] = chip_timer_cont->op->AddSingleFireTimerJob(chip_timer_cont, DELAY_COUNT, delay_gen_avSync_req_msg, send_msg_contr);	
        if(send_msg_contr->delay_msg_timer_id[REQ_AV_SYNC] == INVALIDTIMERID){
            printk("\n\n&&&&&&&&&&&&&gen delay avSync req msg fail[%d]***********\n\n", __LINE__);
        }
    }
    return 0;
}

static int  delay_gen_chip_reset_req_msg(void *context)
{
    isil_send_msg_controller_t *send_msg_contr = (isil_send_msg_controller_t*)context;
    isil_chip_t *chip = send_msg_contr->chip;
    isil_chip_timer_controller_t    *chip_timer_cont = &chip->chip_timer_cont;

    send_msg_contr->delay_msg_timer_id[REQ_ALGO_CHIP_RESET] = INVALIDTIMERID;
    if(try_gen_driver_req_msg(send_msg_contr) != 0){
        send_msg_contr->delay_msg_timer_id[REQ_ALGO_CHIP_RESET] = chip_timer_cont->op->AddSingleFireTimerJob(chip_timer_cont, DELAY_COUNT, delay_gen_chip_reset_req_msg, send_msg_contr);	
        if(send_msg_contr->delay_msg_timer_id[REQ_ALGO_CHIP_RESET] == INVALIDTIMERID){
            printk("\n\n&&&&&&&&&&&&&gen delay chip_reset req msg fail[%d]***********\n\n", __LINE__);
        }
    }
    return 0;
}

static int  delay_gen_video_standard_changed_req_msg(void *context)
{
    isil_send_msg_controller_t *send_msg_contr = (isil_send_msg_controller_t*)context;
    isil_chip_t *chip = send_msg_contr->chip;
    isil_chip_timer_controller_t    *chip_timer_cont = &chip->chip_timer_cont;

    send_msg_contr->delay_msg_timer_id[REQ_VIDEO_STANDARD_CHANGED] = INVALIDTIMERID;
    if(try_gen_driver_req_msg(send_msg_contr) != 0){
        send_msg_contr->delay_msg_timer_id[REQ_VIDEO_STANDARD_CHANGED] = chip_timer_cont->op->AddSingleFireTimerJob(chip_timer_cont, DELAY_COUNT, delay_gen_video_standard_changed_req_msg, send_msg_contr);	
        if(send_msg_contr->delay_msg_timer_id[REQ_VIDEO_STANDARD_CHANGED] == INVALIDTIMERID){
            printk("\n\n&&&&&&&&&&&&&gen delay REQ_VIDEO_STANDARD_CHANGED req msg fail[%d]***********\n\n", __LINE__);
        }
    }
    return 0;
}

static void isil_send_msg_controller_init(isil_send_msg_controller_t *send_msg_contr)
{
    if(send_msg_contr != NULL){
        int i;
        spin_lock_init(&send_msg_contr->lock);
        for(i=0; i<REQ_MSG_NUMBER; i++){
            send_msg_contr->nonblock_msg_timer_id[i] = INVALIDTIMERID;
            send_msg_contr->delay_msg_timer_id[i] = INVALIDTIMERID;        
        }
        send_msg_contr->chip = NULL;
        send_msg_contr->context = NULL;
        send_msg_contr->msg_type = REQ_ALGO_NULL;
        send_msg_contr->msg_send_owner = ISIL_NULL_OWNER;
        send_msg_contr->logic_chan_id = 0;
    }
}

static int  isil_send_msg_controller_send_msg(isil_send_msg_controller_t *send_msg_contr, isil_chip_t *chip, void *context, int msg_type, int block)
{
    int	ret = -1;

    if(msg_type >= INVALID_REQ_ALGO){
        return ret;
    }

    if(send_msg_contr != NULL){
        isil_chip_timer_controller_t    *chip_timer_cont = &chip->chip_timer_cont;
        unsigned long   flags;

        spin_lock_irqsave(&send_msg_contr->lock, flags);
        send_msg_contr->chip = chip;
        send_msg_contr->context = context;
        send_msg_contr->msg_type = msg_type;
        switch(block){
            case NONBLOCK_OP:
                if(send_msg_contr->nonblock_msg_timer_id[send_msg_contr->msg_type] == INVALIDTIMERID){
                    switch(send_msg_contr->msg_type){
                        case REQ_ALGO_SLICE_HEAD:
                            ret = noblock_gen_slicehead_req_msg(send_msg_contr);
                            break;
                        case REQ_AV_SYNC:
                            ret = noblock_gen_avSync_req_msg(send_msg_contr);
                            break;
                        case REQ_ALGO_CHIP_RESET:
                            ret = noblock_gen_chip_reset_req_msg(send_msg_contr);
                            break;
                        case REQ_VIDEO_STANDARD_CHANGED:
                            ret = noblock_gen_video_standard_changed_req_msg(send_msg_contr);
                            break;
                        default:
                            printk("%s.%d:no surpport msg %d noblock\n", __FUNCTION__, __LINE__, send_msg_contr->msg_type);
                            break;
                    }
                } else {
                    printk("%s.%d:%d-%d, have been added timer\n", __FUNCTION__, __LINE__, send_msg_contr->msg_type, block);
                }
                break;
            case DELAY_OP:
                if(send_msg_contr->delay_msg_timer_id[send_msg_contr->msg_type] == INVALIDTIMERID){
                    switch(send_msg_contr->msg_type){
                        case REQ_ALGO_SLICE_HEAD:
                            send_msg_contr->delay_msg_timer_id[REQ_ALGO_SLICE_HEAD] = chip_timer_cont->op->AddSingleFireTimerJob(chip_timer_cont, DELAY_COUNT, delay_gen_slicehead_req_msg, send_msg_contr);
                            if(send_msg_contr->delay_msg_timer_id[REQ_ALGO_SLICE_HEAD] == INVALIDTIMERID){
                                printk("\n\n&&&&&&&&&&&& gen slice_head req msg fail&&&&&&&&&&&&\n\n");
                            }
                            break;
                        case REQ_AV_SYNC:
                            send_msg_contr->delay_msg_timer_id[REQ_AV_SYNC] = chip_timer_cont->op->AddSingleFireTimerJob(chip_timer_cont, DELAY_COUNT, delay_gen_avSync_req_msg, send_msg_contr);
                            if(send_msg_contr->delay_msg_timer_id[REQ_AV_SYNC] == INVALIDTIMERID){
                                printk("\n\n&&&&&&&&&&&& gen av_sync req msg fail&&&&&&&&&&&&\n\n");
                            }
                            break;
                        case REQ_ALGO_CHIP_RESET:
                            send_msg_contr->delay_msg_timer_id[REQ_ALGO_CHIP_RESET] = chip_timer_cont->op->AddSingleFireTimerJob(chip_timer_cont, DELAY_COUNT, delay_gen_chip_reset_req_msg, send_msg_contr);
                            if(send_msg_contr->delay_msg_timer_id[REQ_ALGO_CHIP_RESET] == INVALIDTIMERID){
                                printk("\n\n&&&&&&&&&&&& gen chip_reset req msg fail&&&&&&&&&&&&\n\n");
                            }
                            break;
                        case REQ_VIDEO_STANDARD_CHANGED:
                            send_msg_contr->delay_msg_timer_id[REQ_VIDEO_STANDARD_CHANGED] = chip_timer_cont->op->AddSingleFireTimerJob(chip_timer_cont, DELAY_COUNT, delay_gen_video_standard_changed_req_msg, send_msg_contr);
                            if(send_msg_contr->delay_msg_timer_id[REQ_VIDEO_STANDARD_CHANGED] == INVALIDTIMERID){
                                printk("\n\n&&&&&&&&&&&& gen video standard change req msg fail&&&&&&&&&&&&\n\n");
                            }
                            break;
                        default:
                            printk("%s.%d: no surpport msg %d delay_noblock\n", __FUNCTION__, __LINE__, send_msg_contr->msg_type);
                            break;
                    }
                } else {
                    printk("%s.%d:%d-%d, have been added timer\n", __FUNCTION__, __LINE__, send_msg_contr->msg_type, block);
                }
                break;
            default:
            case BLOCK_OP:
                switch(send_msg_contr->msg_type){
                    case REQ_ALGO_SLICE_HEAD:
                    case REQ_AV_SYNC:
                    case REQ_ALGO_CHIP_RESET:
                    case REQ_VIDEO_STANDARD_CHANGED:
                        spin_unlock_irqrestore(&send_msg_contr->lock, flags);
                        ret = gen_driver_req_msg(send_msg_contr);
                        spin_lock_irqsave(&send_msg_contr->lock, flags);
                        break;
                    default:
                        printk("no surpport msg %d noblock\n", send_msg_contr->msg_type);
                        break;
                }
                break;
        }
        spin_unlock_irqrestore(&send_msg_contr->lock, flags);
    }
    return ret;
}

static void isil_send_msg_controller_set_owner(isil_send_msg_controller_t *send_msg_contr, int owner, int logic_chan_id, void *owner_context)
{
    if(send_msg_contr != NULL){
        unsigned long   flags;

        spin_lock_irqsave(&send_msg_contr->lock, flags);
        send_msg_contr->msg_send_owner = owner;
        send_msg_contr->logic_chan_id = logic_chan_id;
        send_msg_contr->owner_context = owner_context;
        spin_unlock_irqrestore(&send_msg_contr->lock, flags);
    } 
}

static void isil_send_msg_controller_delete_send_msg_controller(isil_send_msg_controller_t *send_msg_contr)
{
    if(send_msg_contr != NULL){
        isil_chip_t     *chip;
        isil_chip_timer_controller_t    *chip_timer_cont;
        unsigned long   flags;
        int i;

        spin_lock_irqsave(&send_msg_contr->lock, flags);
        chip = send_msg_contr->chip;
        if(chip != NULL){
            chip_timer_cont = &chip->chip_timer_cont;
            for(i=0; i<REQ_MSG_NUMBER; i++){
                chip_timer_cont->op->DeleteSingleFireTimerJob(chip_timer_cont, send_msg_contr->nonblock_msg_timer_id[i]);
                send_msg_contr->nonblock_msg_timer_id[i] = INVALIDTIMERID;
                chip_timer_cont->op->DeleteSingleFireTimerJob(chip_timer_cont, send_msg_contr->delay_msg_timer_id[i]);
                send_msg_contr->delay_msg_timer_id[i] = INVALIDTIMERID;
            }
        }
        send_msg_contr->chip = NULL;
        send_msg_contr->context = NULL;
        send_msg_contr->msg_type = REQ_ALGO_NULL;
        spin_unlock_irqrestore(&send_msg_contr->lock, flags);
    } 
}

static struct isil_send_msg_controller_operation   isil_send_msg_controller_op = {
    .init = isil_send_msg_controller_init,
    .send_msg = isil_send_msg_controller_send_msg,
    .set_owner = isil_send_msg_controller_set_owner,
    .delete_send_msg_controller = isil_send_msg_controller_delete_send_msg_controller,
};

void    init_isil_send_msg_controller(isil_send_msg_controller_t *send_msg_contr, int owner, int logic_chan_id, void *owner_context)
{
    if(send_msg_contr != NULL){
        send_msg_contr->op = &isil_send_msg_controller_op;
        send_msg_contr->op->init(send_msg_contr);
        send_msg_contr->op->set_owner(send_msg_contr, owner, logic_chan_id, owner_context);
    }
}

int encode_chan_gen_req_msg(isil_h264_logic_encode_chan_t *encode_chan, int msg_type, int block)
{
    isil_send_msg_controller_t *send_msg_contr;
    isil_chip_t *chip;
    int	ret = -1;

    if((msg_type>=INVALID_REQ_ALGO) || !encode_chan)
        return ret;
    chip = encode_chan->chip;
    send_msg_contr = &encode_chan->send_msg_contr;
    switch(msg_type){
        case REQ_ALGO_SLICE_HEAD:
        case REQ_AV_SYNC:
            ret = send_msg_contr->op->send_msg(send_msg_contr, chip, encode_chan, msg_type, block);
            break;
        case REQ_ALGO_CHIP_RESET:
            ret = send_msg_contr->op->send_msg(send_msg_contr, chip, chip, msg_type, block);
            break;
        default:
            printk("%s.%d: no surpport msg %d\n", __FUNCTION__, __LINE__, msg_type);
            break;
    }
    if(block == DELAY_OP){
        ed_tcb_t *opened_logic_chan_ed = &encode_chan->opened_logic_chan_ed;
	isil_ed_fsm_t   *ed_fsm = &opened_logic_chan_ed->ed_fsm;
        ed_fsm->op->feed_state_watchdog(ed_fsm);
    }
    return ret;
}

int jpeg_encode_chan_gen_req_msg(isil_jpeg_logic_encode_chan_t *encode_chan, int msg_type, int block)
{
    isil_send_msg_controller_t *send_msg_contr;
    isil_chip_t *chip;
    int	ret = -1;

    if((msg_type>=INVALID_REQ_ALGO) || !encode_chan)
        return ret;
    chip = encode_chan->chip;
    send_msg_contr = &encode_chan->send_msg_contr;
    switch(msg_type){
        case REQ_ALGO_CHIP_RESET:
            ret = send_msg_contr->op->send_msg(send_msg_contr, chip, chip, msg_type, block);
            break;
        default:
            printk("%s.%d: no surpport msg %d\n", __FUNCTION__, __LINE__, msg_type);
            break;
    }
    return ret;
}

int prev_chan_gen_req_msg(isil_prev_logic_chan_t *chan, int msg_type, int block)
{
    isil_send_msg_controller_t *send_msg_contr;
    isil_chip_t *chip;
    int	ret = -1;

    if(msg_type >= INVALID_REQ_ALGO || !chan)
        return ret;
    chip = chan->chip;
    send_msg_contr = &chan->send_msg_contr;
    switch(msg_type){
        case REQ_ALGO_CHIP_RESET:
            ret = send_msg_contr->op->send_msg(send_msg_contr, chip, chip, msg_type, block);
            break;
        default:
            printk("%s.%d: no surpport msg %d\n", __FUNCTION__, __LINE__, msg_type);
            break;
    }
    return ret;
}

int isil_vi_video_standard_gen_req(isil_chip_t *chip, int msg_type, int block)
{
    isil_send_msg_controller_t *send_msg_contr;
    int	ret = -1;

    if((msg_type>=INVALID_REQ_ALGO) || !chip)
        return ret;
    send_msg_contr = &chip->chip_vi_driver->send_msg_contr;
    switch(msg_type){
        case REQ_VIDEO_STANDARD_CHANGED:
            ret = send_msg_contr->op->send_msg(send_msg_contr, chip, chip, msg_type, block);
            break;
        default:
            printk("%s.%d: no surpport msg %d\n", __FUNCTION__, __LINE__, msg_type);
            break;
    }
    return ret;
}

int audio_chan_gen_req_msg(isil_audio_driver_t *audio_driver, int msg_type, int block)
{
    isil_send_msg_controller_t *send_msg_contr;
    isil_chip_t *chip;
    int	ret = -1;

    send_msg_contr = &audio_driver->send_msg_contr;
    chip = audio_driver->chip_audio->chip;
    switch(msg_type){
        case REQ_ALGO_CHIP_RESET:
            ret = send_msg_contr->op->send_msg(send_msg_contr, chip, (void *)chip, msg_type, block);
            break;
        default:
            printk("%s.%d: no surpport msg %d\n", __FUNCTION__, __LINE__, msg_type);
            break;
    }
    return ret;
}

