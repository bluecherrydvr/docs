#include <assert.h>
#include "internal.h"

/* http://www.mp4ra.org */
const codec_tag_t mp4_obj_type[] = {
    { CODEC_ID_MPEG4     ,  32 },
    { CODEC_ID_H264      ,  33 },
    { CODEC_ID_MPEG2VIDEO,  96 }, /* MPEG2 Simple */
    { CODEC_ID_MPEG2VIDEO,  97 }, /* MPEG2 Main */
    { CODEC_ID_MPEG2VIDEO,  98 }, /* MPEG2 SNR */
    { CODEC_ID_MPEG2VIDEO,  99 }, /* MPEG2 Spatial */
    { CODEC_ID_MPEG2VIDEO, 100 }, /* MPEG2 High */
    { CODEC_ID_MPEG2VIDEO, 101 }, /* MPEG2 422 */
    { CODEC_ID_JPEG      , 108 }, /* 10918-1 */
    { CODEC_ID_MJPEG     , 108 }, /* 10918-1 */
    { CODEC_ID_JPEG2000  , 110 }, /* 15444-1 */
    { CODEC_ID_VC1       , 163 },
    { CODEC_ID_PCM_S16LE , 224 },
    { CODEC_ID_PCM_ALAW  , 227 },
    { CODEC_ID_PCM_MULAW , 228 },
    { CODEC_ID_ADPCM_G723, 229 },
    { CODEC_ID_PCM_S16BE , 230 },
    { CODEC_ID_YV12_VIDEO, 240 },
    { CODEC_ID_H263      , 242 },
    { CODEC_ID_H261      , 243 },
    { 0, 0 },
};

const codec_tag_t codec_video_tags[] = {
    { CODEC_ID_RAWVIDEO, MKTAG('r', 'a', 'w', ' ') },   /* Uncompressed RGB */
    { CODEC_ID_RAWVIDEO, MKTAG('Y', 'u', 'v', '2') },   /* Uncompressed YUV422 */
    { CODEC_ID_RAWVIDEO, MKTAG('A', 'V', 'U', 'I') },   /* YUV with alpha-channel (AVID Uncompressed) */
    { CODEC_ID_RAWVIDEO, MKTAG('2', 'v', 'u', 'y') },   /* UNCOMPRESSED 8BIT 4:2:2 */
    { CODEC_ID_JPEG,     MKTAG('j', 'p', 'e', 'g') },   /* PhotoJPEG */
    { CODEC_ID_MJPEG,    MKTAG('m', 'j', 'p', 'a') },   /* Motion-JPEG (format A) */
    { CODEC_ID_MPEG4,    MKTAG('m', 'p', '4', 'v') },   /* MPEG-4 */
    { CODEC_ID_MPEG4,    MKTAG('X', 'V', 'I', 'D') },   /* MPEG-4 */
    { CODEC_ID_H263,     MKTAG('h', '2', '6', '3') },   /* H263 */
    { CODEC_ID_H264,     MKTAG('a', 'v', 'c', '1') },   /* AVC-1/H.264 */
    { CODEC_ID_MPEG1VIDEO, MKTAG('m', 'p', 'e', 'g') }, /* MPEG */
    { CODEC_ID_VC1,      MKTAG('v', 'c', '-', '1') },   /* SMPTE RP 2025 */
    { CODEC_ID_NONE, 0 },
};

const codec_tag_t codec_audio_tags[] = {
    { CODEC_ID_PCM_S16BE,    MKTAG('t', 'w', 'o', 's') }, /* 16 bits */
    { CODEC_ID_PCM_S16LE,    MKTAG('s', 'o', 'w', 't') }, /*  */
    { CODEC_ID_PCM_S16LE,    MKTAG('l', 'p', 'c', 'm') },
    { CODEC_ID_PCM_S8,       MKTAG('s', 'o', 'w', 't') },
    { CODEC_ID_PCM_U8,       MKTAG('r', 'a', 'w', ' ') }, /* 8 bits unsigned */
    { CODEC_ID_PCM_U8,       MKTAG('N', 'O', 'N', 'E') }, /* uncompressed */
    { CODEC_ID_PCM_MULAW,    MKTAG('u', 'l', 'a', 'w') }, /*  */
    { CODEC_ID_PCM_ALAW,     MKTAG('a', 'l', 'a', 'w') }, /*  */
    { CODEC_ID_ADPCM_IMA_QT, MKTAG('i', 'm', 'a', '4') }, /* IMA-4 ADPCM */
    { CODEC_ID_ADPCM_MS,     MKTAG('m', 's', 0x00, 0x02) }, /* MS ADPCM */
    { CODEC_ID_NONE, 0 },
};

/* map numeric codes from mdhd atom to ISO 639 */
static const sx_int8 *mp4_mdhd_language_map[] = {
    /* 0-9 */
    "eng", "fra", "ger", "ita", "dut", "sve", "spa", "dan", "por", "nor",
    "heb", "jpn", "ara", "fin", "gre", "ice", "mlt", "tur", "hr ", "chi",
    "urd", "hin", "tha", "kor", "lit", "pol", "hun", "est", "lav",  NULL,
    "fo ",  NULL, "rus", "chi",  NULL, "iri", "alb", "ron", "ces", "slk",
    "slv", "yid", "sr ", "mac", "bul", "ukr", "bel", "uzb", "kaz", "aze",
    "aze", "arm", "geo", "mol", "kir", "tgk", "tuk", "mon",  NULL, "pus",
    "kur", "kas", "snd", "tib", "nep", "san", "mar", "ben", "asm", "guj",
    "pa ", "ori", "mal", "kan", "tam", "tel",  NULL, "bur", "khm", "lao",
    "vie", "ind", "tgl", "may", "may", "amh", "tir", "orm", "som", "swa",
    NULL, "run",  NULL, "mlg", "epo",  NULL,  NULL,  NULL,  NULL,  NULL,
    /* 100 */
    NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,
    NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,
    NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL, "wel", "baq",
    "cat", "lat", "que", "grn", "aym", "tat", "uig", "dzo", "jav"
};

sx_int32 mp4mux_iso639_to_lang(const sx_int8 *lang, sx_int32 mp4)
{
    sx_int32 i, code = 0;

    if (!mp4) {
        for (i = 0; i < (sx_int32)(sizeof(mp4_mdhd_language_map)/sizeof(sx_int8 *)); i++) {
            if (mp4_mdhd_language_map[i] && !strcmp(lang, mp4_mdhd_language_map[i]))
                return i;
        }
    }
    else {
        /* handle undefined as such */
        if (lang[0] == '\0')
            lang = "und";
        /* 5bit ascii */
        for (i = 0; i < 3; i++) {
            sx_uint8 c = (sx_uint8)lang[i];
            if (c < 0x60)
                return 0;
            if (c > 0x60 + 0x1f)
                return 0;
            code <<= 5;
            code |= (c - 0x60);
        }
    }
    return code;
}

sx_int32 mp4_lang_to_iso639(sx_int32 code, sx_int8 *to)
{
    sx_int32 i;
    if (code > 138) {
        for (i = 2; i >= 0; i--) {
            to[i] = 0x60 + (code & 0x1f);
            code >>= 5;
        }
        return 1;
    }
    /* old fashion apple lang code */
    if (code >= (sx_int32)(sizeof(mp4_mdhd_language_map)/sizeof(sx_int8 *)))
        return 0;
    if (!mp4_mdhd_language_map[code])
        return 0;
    strncpy(to, mp4_mdhd_language_map[code], 4);
    return 1;
}

sx_uint32 codec_get_tag(const codec_tag_t *tags, sx_int32 id)
{
    while (tags->id != CODEC_ID_NONE) {
        if (tags->id == id)
            return tags->tag;
        tags++;
    }
    return 0;
}

codec_id_t codec_get_id(const codec_tag_t *tags, sx_uint32 tag)
{
    sx_int32 i;
    for (i=0; tags[i].id != CODEC_ID_NONE;i++) {
        if (tag == tags[i].tag)
            return tags[i].id;
    }
    for (i=0; tags[i].id != CODEC_ID_NONE; i++) {
        if (   toupper((tag >> 0) & 0xFF) == toupper((tags[i].tag >> 0) & 0xFF)
            && toupper((tag >> 8) & 0xFF) == toupper((tags[i].tag >> 8) & 0xFF)
            && toupper((tag >>16) & 0xFF) == toupper((tags[i].tag >>16) & 0xFF)
            && toupper((tag >>24) & 0xFF) == toupper((tags[i].tag >>24) & 0xFF))
            return tags[i].id;
    }
    return CODEC_ID_NONE;
}

void mp4_freep(void **p)
{
    if (*p != NULL)
        free(*p);
    *p = NULL;
}

sx_int64 mp4_gcd(sx_int64 a, sx_int64 b)
{
    if (b) 
        return mp4_gcd(b, a%b);
    return a;
}

sx_int64 rescale_rnd(sx_int64 a, sx_int64 b, sx_int64 c)
{
    sx_int64 ret;
    assert(c > 0);
    assert(b >= 0);
    ret = (a * b + c / 2) / c;
    return ret;
}

// fast realloc
// ptr(in/out): pointer of buffer
// alloc(in/out): currently allocated count
// min(in): requested count
// unit_size(in): unit size
void *mp4_fast_realloc(void *ptr, sx_uint32 *alloc, sx_uint32 min, sx_uint32 unit_size)
{
    if (min < *alloc)
        return ptr;
    *alloc = MAX(min + (min >> 3), min + 1024);
    return realloc(ptr, *alloc * unit_size);
}

// fast realloc with specified increment
void *mp4_fast_realloc1(void *ptr, sx_uint32 *alloc, sx_uint32 min, 
                        sx_uint32 unit_size, sx_uint32 inc)
{
    if (min < *alloc)
        return ptr;
    *alloc = min + inc;
    return realloc(ptr, *alloc * unit_size);
}

// fast realloc with specified increment and initialization
void *mp4_fast_realloc2(void *ptr, sx_uint32 *alloc, sx_uint32 min, 
                        sx_uint32 unit_size, sx_uint32 inc)
{
    sx_uint32 diff;
    if (min < *alloc)
        return ptr;
    diff = (min + inc - *alloc) * unit_size;
    *alloc = min + inc;
    ptr = realloc(ptr, *alloc * unit_size);
    if (ptr != NULL) {
        memset((sx_uint8 *)ptr + diff, 0, diff);
    }
    return ptr;
}


#ifdef LOG_OUTPUT
void mp4_log(FILE *pf, const char *fmt, ...)
{
    if (pf != NULL) {
        va_list vl;
        va_start(vl, fmt);
        vfprintf(pf, fmt, vl);
        va_end(vl);
        fflush(pf);
    }
}
#else
void mp4_log(FILE *pf, const char *fmt, ...)
{
    /* Suppress compiler warnings */
    const char *p = fmt;
    p = p;
    pf = pf;
}
#endif

