#ifndef __ISIL__VERSION__H__
#define __ISIL__VERSION__H__

#define  __ISIL__CODE__VERSION__        (1 << 26) | (2 << 16) | (0 << 0) 
#define  __ISIL_INTER_VERSION__(a,b,c)  (((a) << 26) | ((b) << 16) | ((c) << 0)) 

#define get_version_major(v)            ((v) >> 26)
#define get_version_submajor(v)         (((v) >> 16) & 0x3ff)
#define get_version_minor(v)            ((v) & 0xffff)

#ifndef __ISIL_SVN_VERSION__
#define  __ISIL_SVN_VERSION__           61
#endif

#ifndef __ISIL_ARCH__
#define  __ISIL_ARCH__                  "unknown"
#endif




























#endif
