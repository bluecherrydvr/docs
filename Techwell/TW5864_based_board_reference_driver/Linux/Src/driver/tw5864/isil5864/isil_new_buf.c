#include	<isil5864/isil_common.h>

static void tcb_node_init(tcb_node_t *node)
{
    if(node != NULL){
        INIT_LIST_HEAD(&node->list);
        atomic_set(&node->ref_count, 0);
        if(node->op != NULL){
            node->op->update_state(node, TCB_IDLE_STATE);
        }
    }
}

static void tcb_node_set_priv(tcb_node_t *node, void *priv)
{
    if(node != NULL){
        node->tcb_priv = priv;
    }
}

static void tcb_node_get_priv(tcb_node_t *node, void **priv)
{
    if((node != NULL) && (priv != NULL)){
        *priv = node->tcb_priv;
    }
}

static void tcb_node_reference(tcb_node_t *src_node, tcb_node_t **dest_node)
{
    if((src_node!=NULL) && (dest_node!=NULL)){
        atomic_inc(&src_node->ref_count);
        *dest_node = src_node;
    }
}

static void tcb_node_release(tcb_node_t *node, tcb_node_pool_t *pool)
{
    if((node!=NULL) && (pool!=NULL)){
        if(atomic_read(&node->ref_count) > 1){
            atomic_dec(&node->ref_count);
        } else {
            atomic_set(&node->ref_count, 0);
            if(pool->op != NULL){
                pool->op->put(pool, node);
            } else {
                printk("%s.%d: pool->op is null\n", __FUNCTION__, __LINE__);
            }
        }
    }
}

static void tcb_node_update_state(tcb_node_t *node, int state)
{
    if(node != NULL){
        node->state = state;
    }
}

struct tcb_node_operation   tcb_node_op = {
    .init = tcb_node_init,
    .set_priv = tcb_node_set_priv,
    .get_priv = tcb_node_get_priv,
    .reference = tcb_node_reference,
    .release = tcb_node_release,
    .update_state = tcb_node_update_state,
};

static void tcb_node_pool_init(tcb_node_pool_t *pool, int entry_number)
{
    if(pool != NULL){
        INIT_LIST_HEAD(&pool->pool_header);
        init_waitqueue_head(&pool->wait);
        spin_lock_init(&pool->lock);
        atomic_set(&pool->curr_pool_entry_number, 0);
        pool->pool_entry_number = entry_number;
    }
}

static void tcb_node_pool_release(tcb_node_pool_t *pool)
{
    if(pool != NULL){
        unsigned long	flags;
again:
        spin_lock_irqsave(&pool->lock, flags);
        if(pool->pool_entry_number != atomic_read(&pool->curr_pool_entry_number)){
            spin_unlock_irqrestore(&pool->lock, flags);
            wait_event_interruptible(pool->wait, (pool->pool_entry_number == atomic_read(&pool->curr_pool_entry_number)));
            goto again;
        } else {
            INIT_LIST_HEAD(&pool->pool_header);
            pool->pool_entry_number = 0;
            atomic_set(&pool->curr_pool_entry_number, 0);
            spin_unlock_irqrestore(&pool->lock, flags);
        }
    }
}

static void tcb_node_pool_get(tcb_node_pool_t *pool, tcb_node_t **node)
{
    if(pool!=NULL && node!=NULL){
        unsigned long	flags;
again:
        spin_lock_irqsave(&pool->lock, flags);
        if(atomic_read(&pool->curr_pool_entry_number) > 0){
            atomic_dec(&pool->curr_pool_entry_number);
            *node = list_entry(pool->pool_header.next, tcb_node_t, list);
            list_del_init(&((*node)->list));
            atomic_inc(&((*node)->ref_count));
            spin_unlock_irqrestore(&pool->lock, flags);
        } else {
            spin_unlock_irqrestore(&pool->lock, flags);
            wait_event_interruptible(pool->wait, atomic_read(&pool->curr_pool_entry_number));
            goto again;
        }
    }
}

static void tcb_node_pool_try_get(tcb_node_pool_t *pool, tcb_node_t **node)
{
    if(pool!=NULL && node!=NULL){
        unsigned long	flags;
        spin_lock_irqsave(&pool->lock, flags);
        if(atomic_read(&pool->curr_pool_entry_number) > 0){
            atomic_dec(&pool->curr_pool_entry_number);
            *node = list_entry(pool->pool_header.next, tcb_node_t, list);
            list_del_init(&((*node)->list));
            atomic_inc(&((*node)->ref_count));
        } else {
            *node = NULL;
        }
        spin_unlock_irqrestore(&pool->lock, flags);
    }
}

static void tcb_node_pool_put(tcb_node_pool_t *pool, tcb_node_t *node)
{
    if(pool!=NULL && node!=NULL){
        unsigned long	flags;
        spin_lock_irqsave(&pool->lock, flags);
        node->op->init(node);
        list_add_tail(&node->list, &pool->pool_header);
        atomic_inc(&pool->curr_pool_entry_number);
        if(atomic_read(&pool->curr_pool_entry_number) == 1){
            wake_up_interruptible(&pool->wait);
        }
        spin_unlock_irqrestore(&pool->lock, flags);
    }
}

static int  tcb_node_pool_get_curr_pool_entry_number(tcb_node_pool_t *pool)
{
    if(pool != NULL){
        return atomic_read(&pool->curr_pool_entry_number);
    } else {
        return 0;
    }
}

struct tcb_pool_operation   tcb_node_pool_op = {
    .init = tcb_node_pool_init,
    .release = tcb_node_pool_release,

    .get = tcb_node_pool_get,
    .try_get = tcb_node_pool_try_get,
    .put = tcb_node_pool_put,

    .get_curr_pool_entry_number = tcb_node_pool_get_curr_pool_entry_number,
};

static int  tcb_node_queue_get_queue_curr_entry_number(tcb_node_queue_t *tcb_queue)
{
    if(tcb_queue != NULL){
        return atomic_read(&tcb_queue->curr_queue_entry_number);
    } else {
        return 0;
    }
}

static void tcb_node_queue_init(tcb_node_queue_t *tcb_queue)
{
    if(tcb_queue != NULL){
        spin_lock_init(&tcb_queue->lock);
        INIT_LIST_HEAD(&tcb_queue->queue_header);
        init_waitqueue_head(&tcb_queue->wait);
        atomic_set(&tcb_queue->curr_queue_entry_number, 0);
    }
}

static void tcb_node_queue_put(tcb_node_queue_t *tcb_queue, tcb_node_t *tcb_node)
{
    if((tcb_queue!=NULL) && (tcb_node!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&tcb_queue->lock, flags);
        list_add_tail(&tcb_node->list, &tcb_queue->queue_header);
        atomic_inc(&tcb_queue->curr_queue_entry_number);
        if(atomic_read(&tcb_queue->curr_queue_entry_number) == 1){
            wake_up_interruptible(&tcb_queue->wait);
        }
        spin_unlock_irqrestore(&tcb_queue->lock, flags);
    }
}

static void tcb_node_queue_put_header(tcb_node_queue_t *tcb_queue, tcb_node_t *tcb_node)
{
    if((tcb_queue!=NULL) && (tcb_node!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&tcb_queue->lock, flags);
        list_add(&tcb_node->list, &tcb_queue->queue_header);
        atomic_inc(&tcb_queue->curr_queue_entry_number);
        if(atomic_read(&tcb_queue->curr_queue_entry_number) == 1){
            wake_up_interruptible(&tcb_queue->wait);
        }
        spin_unlock_irqrestore(&tcb_queue->lock, flags);
    }
}

static void tcb_node_queue_get(tcb_node_queue_t *tcb_queue, tcb_node_t **tcb_node)
{
    if((tcb_queue!=NULL) && (tcb_node!=NULL)){
        unsigned long	flags;
again:
        spin_lock_irqsave(&tcb_queue->lock, flags);
        if(atomic_read(&tcb_queue->curr_queue_entry_number)){
            atomic_dec(&tcb_queue->curr_queue_entry_number);
            *tcb_node = list_entry(tcb_queue->queue_header.next, tcb_node_t, list);
            list_del_init(&(*tcb_node)->list);
            spin_unlock_irqrestore(&tcb_queue->lock, flags);
        } else {
            spin_unlock_irqrestore(&tcb_queue->lock, flags);
            wait_event_interruptible(tcb_queue->wait, atomic_read(&tcb_queue->curr_queue_entry_number));
            goto again;
        }
    } else {
        printk("\n%s.%d**attention**\n", __FUNCTION__, __LINE__);
    }
}

static void tcb_node_queue_get_tailer(tcb_node_queue_t *tcb_queue, tcb_node_t **tcb_node)
{
    if((tcb_queue!=NULL) && (tcb_node!=NULL)){
        unsigned long	flags;
again:
        spin_lock_irqsave(&tcb_queue->lock, flags);
        if(atomic_read(&tcb_queue->curr_queue_entry_number)){
            atomic_dec(&tcb_queue->curr_queue_entry_number);
            *tcb_node = list_entry(tcb_queue->queue_header.prev, tcb_node_t, list);
            list_del_init(&(*tcb_node)->list);
            spin_unlock_irqrestore(&tcb_queue->lock, flags);
        } else {
            spin_unlock_irqrestore(&tcb_queue->lock, flags);
            wait_event_interruptible(tcb_queue->wait, atomic_read(&tcb_queue->curr_queue_entry_number));
            goto again;
        }
    }
}

static void tcb_node_queue_try_get(tcb_node_queue_t *tcb_queue, tcb_node_t **tcb_node)
{
    if((tcb_queue!=NULL) && (tcb_node!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&tcb_queue->lock, flags);
        if(atomic_read(&tcb_queue->curr_queue_entry_number)){
            atomic_dec(&tcb_queue->curr_queue_entry_number);
            *tcb_node = list_entry(tcb_queue->queue_header.next, tcb_node_t, list);
            list_del_init(&(*tcb_node)->list);
        } else {
            *tcb_node = NULL;
        }
        spin_unlock_irqrestore(&tcb_queue->lock, flags);
    }
}

static void tcb_node_queue_try_get_tailer(tcb_node_queue_t *tcb_queue, tcb_node_t **tcb_node)
{
    if((tcb_queue!=NULL) && (tcb_node!=NULL)){
        unsigned long	flags;
        spin_lock_irqsave(&tcb_queue->lock, flags);
        if(atomic_read(&tcb_queue->curr_queue_entry_number)){
            atomic_dec(&tcb_queue->curr_queue_entry_number);
            *tcb_node = list_entry(tcb_queue->queue_header.prev, tcb_node_t, list);
            list_del_init(&(*tcb_node)->list);
        } else {
            *tcb_node = NULL;
        }
        spin_unlock_irqrestore(&tcb_queue->lock, flags);
    }
}

static int  tcb_node_queue_delete(tcb_node_queue_t *tcb_queue, tcb_node_t *tcb_node)
{
    int ret = 0;

    if((tcb_queue!=NULL) && (tcb_node!=NULL)){
        tcb_node_t      *temp_tcb_node;
        int             node_number;
        unsigned long   flags;

        spin_lock_irqsave(&tcb_queue->lock, flags);
        node_number = tcb_queue->op->get_queue_curr_entry_number(tcb_queue);
        while(node_number){
            node_number--;
            atomic_dec(&tcb_queue->curr_queue_entry_number);
            temp_tcb_node = list_entry(tcb_queue->queue_header.next, tcb_node_t, list);
            list_del_init(&temp_tcb_node->list);

            if(temp_tcb_node == tcb_node){
                ret = 1;
                break;
            } else {
                list_add_tail(&temp_tcb_node->list, &tcb_queue->queue_header);
                atomic_inc(&tcb_queue->curr_queue_entry_number);
            }
        }
        spin_unlock_irqrestore(&tcb_queue->lock, flags);
    }
    return ret;
}

struct tcb_node_queue_operation	tcb_node_queue_op = {
    .get_queue_curr_entry_number = tcb_node_queue_get_queue_curr_entry_number,

    .init = tcb_node_queue_init,
    .put = tcb_node_queue_put,
    .put_header = tcb_node_queue_put_header,
    .get = tcb_node_queue_get,
    .get_tailer = tcb_node_queue_get_tailer,
    .try_get = tcb_node_queue_try_get,
    .try_get_tailer = tcb_node_queue_try_get_tailer,
    .delete = tcb_node_queue_delete,
};



