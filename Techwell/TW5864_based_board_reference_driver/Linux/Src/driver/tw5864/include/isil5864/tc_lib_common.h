#ifndef  __TC_LIB_COMMON_H__
#define  __TC_LIB_COMMON_H__


static inline void print_two(volatile char *p);






#define tdm_min(x,y)    ((x)<(y))? (x): (y)

static inline unsigned int tdm_algin_up_power_2(unsigned int n, unsigned int size)
{
    size--;
    return (n + size) & ~size;
}

static inline unsigned int tdm_round_up_common(unsigned int total,unsigned int unit_size)
{
    return  (total % unit_size)? (total / unit_size) + 1: total / unit_size;
}





static inline void tdm_set_bit(int nr, volatile unsigned int * addr)
{
    int	mask;
    volatile unsigned int *a = addr;

    a += nr >> 5;
    mask = 1 << (nr & 0x1f);
    *a |= mask;
}


static inline void tdm_clear_bit(int nr, volatile unsigned int * addr)
{
    int	mask;
    volatile unsigned int *a = addr;

    a += nr >> 5;
    mask = 1 << (nr & 0x1f);
    *a &= ~mask;
}


static inline void tdm_change_bit(int nr, volatile unsigned int * addr)
{
    int	mask;
    volatile unsigned int *a = addr;

    a += nr >> 5;
    mask = 1 << (nr & 0x1f);
    *a ^= mask;
}



static inline int tdm_test_and_change_bit(int nr, volatile  unsigned int * addr)
{
    int	mask, retval;
    volatile unsigned int *a = addr;

    a += nr >> 5;
    mask = 1 << (nr & 0x1f);
    retval = (mask & *a) != 0;
    *a ^= mask;

    return retval;
}




/*if the nr bit is 1, return ture*/
static inline int tdm_test_bit(int nr, volatile unsigned int *vaddr)
{
    //volatile char *p = (volatile char *)vaddr;

    return (vaddr[nr >> 5] & (1UL << (nr & 31))) != 0;
}


/* find the first bit postion with value 1, 
 * base: the address where to start to find
 * len : how long the find area is
 * return: if find nothing  return len+1 
 */
static inline int tdm_find_first_bit(volatile unsigned int *base, unsigned int len)
{
    int idx = 0;
    if(!len)
	return -1;
    while(idx < len){
	if(tdm_test_bit(idx,base))
	    return idx;
	idx++;       
    }
    return len;
}

/* find the xth bit with value 1,
 * if x = 2, means want find the second bit's postion with value 1, based from base
 */
static inline int tdm_find_xth_bit(volatile unsigned int *base, unsigned int len, unsigned int x)
{
    int idx = 0,c = 1;

    if(!len || !x)
	return -1;
    while(idx < len){
	if(tdm_test_bit(idx,base)){
	    if(x == c)
		return idx;
	    else
		c++;
	}
	idx++;       
    }
    return len;
}

static inline int tdm_find_first_zero_bit(volatile unsigned int *base, unsigned int len)
{
    int idx = 0;
    if(!len)
	return -1;
    while(idx < len){
	if(!tdm_test_bit(idx,base))
	    return idx;
	idx++;       
    }
    return len;
}

#define TDM_ISVALID(x,y)  (((x) >= 0) && (x < y))

static inline int tdm_find_next_zero_bit(volatile unsigned int *base, unsigned int size, unsigned int off)
{
    int idx;
    //unsigned int *p;
    if((off + size) < off)
	return -3;

    for(idx = off; idx < off + size; idx++){
	if(!tdm_test_bit(idx,base))
	    return idx;
    }

    return off-1; //may negative
}
#define TDM_INAREA(a,x,y)  (((a) >= (x)) && ((a) < (y)))


static const int bit_pos[] = // map a bit value mod 37 to its position
{
    32, 0, 1, 26, 2, 23, 27, 0, 3, 16, 24, 30, 28, 11, 0, 13, 4,
    7, 17, 0, 25, 22, 31, 15, 29, 10, 12, 6, 0, 21, 14, 9, 5,
    20, 8, 19, 18
};

/*counting contiguous zreo bits number from right*/
static inline int tdm_counting_conzreo_bits_r(volatile unsigned int target){

    return  bit_pos[(-target & target) % 37];
}



/*set the nr bit to 1,and return it's old value*/
static inline int tdm_test_and_set_bit(unsigned int nr, volatile unsigned int * addr)
{
    unsigned int mask = 1 << (nr & 0x1f);
    int *m = ((int *) addr) + (nr >> 5);
    int old = *m;

    *m = old | mask;
    return (old & mask) != 0;
}

/*clear the nr bit to 0, and return it's old value*/
static inline int tdm_test_and_clear_bit(unsigned int nr, volatile unsigned int * addr)
{
    unsigned int mask = 1 << (nr & 0x1f);
    int *m = ((int *) addr) + (nr >> 5);
    int old = *m;

    *m = old & ~mask;
    return (old & mask) != 0;
}




static inline void tdm_memset(volatile char *s,char c, unsigned int l)
{
    volatile char *p = s;
    int idx;

    for(idx = 0; idx < l; idx++,p++){
	*p = c;
    }
}


static inline int get_root_power_of2(unsigned int s)
{
    unsigned int i = 0,t = s;

    do{
	t >>= 1;
	i++;
    }while(t);

    return i;
}


static inline int square(unsigned int s)
{
    return 0;
}

static inline void print_two(volatile char *p)
{
    int i;
    volatile char t = 0x01,c = *p;
    for(i=0; i < 8; i++){
	if(c & t)
	    printk("1");
	else
	    printk("0");
	c >>= 1;
    }
    printk(",");
}


#if 0
size_t my_strncpy(char *dst, const char *src, size_t size)
{
    size_t bytes = 0;
    char *q = dst;
    const char *p = src;
    char ch;

    while ((ch = *p++)) {
	if (bytes+1 < size)
	    *q++ = ch;
	bytes++;
    }

    /* If size == 0 there is no space for a final null... */
    if (size)
	*q = '\0';
    return bytes;
}

char * my_strrchr(char * p, char c)
{
    unsigned int len;
    char * t = p;

    len = strlen(p);
    t = t + len;

    while(--len)
    {
	if(*t == c)
	    break;
    }

    if(!len)
	return NULL;

    return t;
}
#endif






#endif
