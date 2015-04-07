#ifndef	__TC_HELPER_H__
#define	__TC_HELPER_H__

#ifdef __cplusplus
extern "C"
{
#endif





    struct memblock_s
    {
	struct list_head        list;
	void                    *ptr;
	size_t                  size;
    };





    void *tc_malloc(size_t size);
    void tc_free(void *ptr);
    void *tc_realloc(void *ptr, size_t size);












#ifdef __cplusplus
}
#endif

#endif	
