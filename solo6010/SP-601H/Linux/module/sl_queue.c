#include <linux/kernel.h>
#include <linux/module.h>

#include "sl_queue.h"

int sl_queue_init(struct SL_QUEUE *queue, size_t size, int min_nr, const char *name, ...)
{
	va_list args;

	va_start(args, name);
	vsnprintf(queue->cache_name, 64, name, args);
	va_end(args);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 23))
	queue->cache = kmem_cache_create(queue->cache_name, size, 0, SLAB_HWCACHE_ALIGN, NULL);
#else
	queue->cache = kmem_cache_create(queue->cache_name, size, 0, SLAB_HWCACHE_ALIGN, NULL, NULL);
#endif
	if(!queue->cache)
		return -1;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
	queue->pool = mempool_create(min_nr, mempool_alloc_slab, mempool_free_slab, queue->cache);
	if(!queue->pool)
	{
		kmem_cache_destroy(queue->cache);
		return -1;
	}
#endif

	spin_lock_init(&queue->lock);
	INIT_LIST_HEAD(&queue->lh);
	init_waitqueue_head(&queue->wq);

	return 0;
}

void sl_queue_exit(struct SL_QUEUE *queue)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0))
	mempool_destroy(queue->pool);
#endif
	kmem_cache_destroy(queue->cache);
}

