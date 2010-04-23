#ifndef __DEFINE_H__
#define __DEFINE_H__

#if defined(__GNUC__)
/* used for inline assembly stuff with GCC syntax */
  #define HAVE_MMX 1
  #define HAVE_SSSE3 1

  #define ARCH_X86 1

  #ifdef ARCH_X86_64
    #define HAVE_FAST_64BIT 1
  #else
    #define ARCH_X86_32 1
  #endif
#endif

#define restrict 
#define CONFIG_MEMALIGN_HACK


#endif
