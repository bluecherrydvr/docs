#ifndef BITSTREAM_H
#define BITSTREAM_H

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct GetBitContext
    {
	const uint8_t *buffer, *buffer_end;
	int index;
	int size_in_bits;
    }GetBitContext;

#define	NEG_SSR32(a,s) ((( int32_t)(a))>>(32-(s)))
#define	NEG_USR32(a,s) (((uint32_t)(a))>>(32-(s)))


    static inline int unaligned32_be(const void *v)
    {
	const uint8_t *p=v;
	return (((p[0]<<8) | p[1])<<16) | (p[2]<<8) | (p[3]);
    }

#define	OPEN_READER(name, gb)\
    int name##_index= (gb)->index;\
    int name##_cache= 0;\

#define	CLOSE_READER(name, gb)\
    (gb)->index= name##_index;\

#define	UPDATE_CACHE(name, gb)\
    name##_cache= unaligned32_be( ((const uint8_t *)(gb)->buffer)+(name##_index>>3) ) << (name##_index&0x07);\

#define SKIP_COUNTER(name, gb, num)\
    name##_index += (num);\

#define LAST_SKIP_BITS(name, gb, num)		SKIP_COUNTER(name, gb, num)

#define SHOW_UBITS(name, gb, num)\
    NEG_USR32(name##_cache, num)

#define GET_CACHE(name, gb)\
    ((uint32_t)name##_cache)

    static inline int get_bits_count(GetBitContext *s)
    {
	return s->index;
    }

    static inline unsigned int get_bits(GetBitContext *s, int n)
    {
	register int tmp;
	OPEN_READER(re, s)
	    UPDATE_CACHE(re, s)
	    tmp= SHOW_UBITS(re, s, n);
	LAST_SKIP_BITS(re, s, n)
	    CLOSE_READER(re, s)
	    return tmp;
    }

    static inline unsigned int get_bits1(GetBitContext *s)
    {
	int index= s->index;
	uint8_t result= s->buffer[ index>>3 ];
	result <<= (index&0x07);
	result >>= 7;
	s->index++;
	return result;
    }

    static inline void init_get_bits(GetBitContext *s, const uint8_t *buffer, int bit_size)
    {
	int buffer_size = (bit_size+7)>>3;

	if(buffer_size < 0 || bit_size < 0)
	{
	    buffer_size = bit_size = 0;
	    buffer = NULL;
	}
	s->buffer = buffer;
	s->size_in_bits = bit_size;
	s->buffer_end = buffer + buffer_size;
	s->index=0;
    }

#ifdef __cplusplus
}
#endif


#endif /* BITSTREAM_H */

