#include <isil5864/tc_common.h>


/*wapper of the alloc/realloc liked implements in kernel space*/
/************************************************************************************************/
/* The list of memory blocks (one simple list)*/
static LIST_HEAD(memblock_list);
static rwlock_t memblock_lock = RW_LOCK_UNLOCKED;

static int memblock_add(void *ptr, size_t size)
{
    struct memblock_s *m;

    if (!(m = kzalloc(sizeof(struct memblock_s), GFP_KERNEL)))
        return -ENOMEM;
    m->ptr = ptr;
    m->size = size;

    write_lock(&memblock_lock);
    list_add(&m->list, &memblock_list);
    write_unlock(&memblock_lock);

    return 0;
}

static void memblock_del(struct memblock_s *m)
{
    write_lock(&memblock_lock);
    list_del(&m->list);
    write_unlock(&memblock_lock);
    kfree(m);
}

static struct memblock_s *memblock_get(void *ptr)
{
    struct list_head *l;
    struct memblock_s *m;

    read_lock(&memblock_lock);
    list_for_each(l, &memblock_list) {
        m = list_entry(l, struct memblock_s, list);
        if (m->ptr == ptr) {
            /* HIT */
            read_unlock(&memblock_lock);
            return m;
        }
    }
    read_unlock(&memblock_lock);
    return NULL;
}


void *tc_malloc(size_t size)
{
    void *ptr;

    if (!(ptr = kzalloc(size, GFP_KERNEL)))
        return NULL;
    if (memblock_add(ptr, size)) {
        kfree(ptr);
        return NULL;
    }
    return ptr;
}

void tc_free(void *ptr)
{
    struct memblock_s *m;

    if (!ptr)
        return;
    if (!(m = memblock_get(ptr))) {
        printk(KERN_ERR "bug: free non-exist memory\n");
        return;
    }
    memblock_del(m);
    kfree(ptr);
}




/*now we just support for largen realloc.... the new_size @should large than@ prev size */
void *tc_realloc(void *ptr, size_t size)
{
    struct memblock_s *m;
    void *new = NULL;

    if (ptr) {
        if (!(m = memblock_get(ptr))) {
            printk(KERN_ERR "bug: realloc non-exist memory\n");
            return NULL;
        }

        if (size < m->size){
            printk(KERN_ERR "bug: realloc not support small realloc\n");
            return NULL;			
        }
        if (size == m->size)
            return ptr;
        if (size != 0) {
            if (!(new = kzalloc(size, GFP_KERNEL)))
                return NULL;
            memmove(new, ptr, m->size);
            if (memblock_add(new, size)) {
                kfree(new);
                return NULL;
            }
        }

        memblock_del(m);
        kfree(ptr);
    } else {
        if (size != 0) {
            if (!(new = kzalloc(size, GFP_KERNEL)))
                return NULL;
            if (memblock_add(new, size)) {
                kfree(new);
                return NULL;
            }
        }
    }

    return new;
}

EXPORT_SYMBOL(tc_realloc);

