#ifndef GOLOMB_H
#define GOLOMB_H

#ifdef __cplusplus
extern "C"
{
#endif

    extern const __u8 ff_log2_tab[256];
    extern const __u8 ff_golomb_vlc_len[512];
    extern const __u8 ff_ue_golomb_vlc_code[512];
    extern const __s8 ff_se_golomb_vlc_code[512];

    static inline int av_log2(unsigned int v)
    {
	int n;

	n = 0;
	if(v & 0xffff0000)
	{
	    v >>= 16;
	    n += 16;
	}
	if(v & 0xff00)
	{
	    v >>= 8;
	    n += 8;
	}
	n += ff_log2_tab[v];
	return n;
    }

    static inline int get_ue_golomb(GetBitContext *gb)
    {
	unsigned int buf;
	int log;

	OPEN_READER(re, gb);
	UPDATE_CACHE(re, gb);
	buf=GET_CACHE(re, gb);
	if(buf >= (1<<27))
	{
	    buf >>= 32 - 9;
	    LAST_SKIP_BITS(re, gb, ff_golomb_vlc_len[buf]);
	    CLOSE_READER(re, gb);

	    return ff_ue_golomb_vlc_code[buf];
	}
	else
	{
	    log= 2*av_log2(buf) - 31;
	    buf>>= log;
	    buf--;
	    LAST_SKIP_BITS(re, gb, 32 - log);
	    CLOSE_READER(re, gb);

	    return buf;
	}
    }

    /**
     * read signed exp golomb code.
     */
    static inline int get_se_golomb(GetBitContext *gb)
    {
	unsigned int buf;
	int log;

	OPEN_READER(re, gb);
	UPDATE_CACHE(re, gb);
	buf=GET_CACHE(re, gb);
	if(buf >= (1<<27))
	{
	    buf >>= 32 - 9;
	    LAST_SKIP_BITS(re, gb, ff_golomb_vlc_len[buf]);
	    CLOSE_READER(re, gb);

	    return ff_se_golomb_vlc_code[buf];
	}
	else
	{
	    log= 2*av_log2(buf) - 31;
	    buf>>= log;

	    LAST_SKIP_BITS(re, gb, 32 - log);
	    CLOSE_READER(re, gb);

	    if(buf&1)
		buf = -(buf>>1);
	    else
		buf =  (buf>>1);

	    return buf;
	}
    }

#ifdef __cplusplus
}
#endif

#endif	//GOLOMB_H

