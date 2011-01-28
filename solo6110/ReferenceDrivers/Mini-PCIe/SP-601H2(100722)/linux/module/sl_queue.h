#ifndef __SL_QUEUE_H
#define __SL_QUEUE_H

#include <linux/version.h>
#include <linux/slab.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
#include <linux/mempool.h>
#endif
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/wait.h>

struct SL_QUEUE
{
	char cache_name[64];
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20))
	struct kmem_cache *cache;
#else
	kmem_cache_t *cache;
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
	mempool_t *pool;
#endif

	spinlock_t lock;
	struct list_head lh;
	wait_queue_head_t wq;
};

int sl_queue_init(struct SL_QUEUE *queue, size_t size, int min_nr, const char *name, ...);
void sl_queue_exit(struct SL_QUEUE *queue);

#define sl_queue_head(queue, type)		list_entry((queue)->lh.prev, type, lh)
#define sl_queue_empty(queue)			list_empty(&(queue)->lh)
#define sl_queue_clear(queue, index, type)		\
	while(!sl_queue_empty(queue))				\
	{											\
		index = sl_queue_head(queue, type);		\
		list_del(&index->lh);					\
		sl_queue_free(queue, index);			\
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
#define sl_queue_alloc(queue)		mempool_alloc((queue)->pool, GFP_ATOMIC)
#define sl_queue_free(queue, index)	mempool_free(index, (queue)->pool)
#else
#define sl_queue_alloc(queue)		kmem_cache_alloc((queue)->cache, SLAB_ATOMIC)
#define sl_queue_free(queue, index)	kmem_cache_free((queue)->cache, index)
#endif

#endif
