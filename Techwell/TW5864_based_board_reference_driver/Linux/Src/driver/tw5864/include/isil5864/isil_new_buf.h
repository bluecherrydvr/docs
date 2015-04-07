#ifndef	DVM_NEW_BUF_H
#define	DVM_NEW_BUF_H

#ifdef __cplusplus
extern "C"
{
#endif

    enum TCB_NODE_STATE {
	TCB_IDLE_STATE = 0,
	TCB_STANDBY_STATE,
	TCB_RUNNING_STATE,
	TCB_SUSPEND_STATE,
	TCB_CLOSE_STATE
    };

    struct tcb_node_operation{
	void	(*init)(tcb_node_t *);
	void	(*set_priv)(tcb_node_t *, void *);
	void	(*get_priv)(tcb_node_t *, void **);
	void	(*reference)(tcb_node_t *, tcb_node_t **);
	void	(*release)(tcb_node_t *, tcb_node_pool_t *);
	void	(*update_state)(tcb_node_t *, int );
    };

#define	to_get_tcb_node(node)   container_of(node, tcb_node_t, list)
    struct tcb_node{
	struct list_head	list;
	void		*tcb_priv;
	atomic_t	ref_count;
	enum TCB_NODE_STATE	state;

	struct tcb_node_operation	*op;
    };

    struct tcb_pool_operation{
	void	(*init)(tcb_node_pool_t *, int );
	void	(*release)(tcb_node_pool_t *);

	void	(*get)(tcb_node_pool_t *, tcb_node_t **);
	void	(*try_get)(tcb_node_pool_t *, tcb_node_t **);
	void	(*put)(tcb_node_pool_t *, tcb_node_t *);

	int	(*get_curr_pool_entry_number)(tcb_node_pool_t *);
    };

    struct tcb_pool{
	struct list_head	pool_header;
	wait_queue_head_t	wait;
	spinlock_t	lock;
	atomic_t	curr_pool_entry_number;
	int			pool_entry_number;

	struct tcb_pool_operation	*op;
    };

    struct tcb_node_queue_operation{
	int	(*get_queue_curr_entry_number)(tcb_node_queue_t *);

	void	(*init)(tcb_node_queue_t *);
	void	(*put)(tcb_node_queue_t *, tcb_node_t *);
	void	(*put_header)(tcb_node_queue_t *, tcb_node_t *);
	void	(*get)(tcb_node_queue_t *, tcb_node_t **);
	void	(*get_tailer)(tcb_node_queue_t *, tcb_node_t **);
	void	(*try_get)(tcb_node_queue_t *, tcb_node_t **);
	void	(*try_get_tailer)(tcb_node_queue_t *, tcb_node_t **);
	int     (*delete)(tcb_node_queue_t *, tcb_node_t *);
    };

    struct tcb_node_queue{
	spinlock_t	lock;
	struct list_head    queue_header;
	wait_queue_head_t   wait;
	atomic_t	curr_queue_entry_number;

	struct tcb_node_queue_operation	*op;
    };

    extern struct tcb_node_operation		tcb_node_op;
    extern struct tcb_pool_operation		tcb_node_pool_op;
    extern struct tcb_node_queue_operation	tcb_node_queue_op;

#ifdef __cplusplus
}
#endif

#endif	//DVM_NEW_BUF_H
