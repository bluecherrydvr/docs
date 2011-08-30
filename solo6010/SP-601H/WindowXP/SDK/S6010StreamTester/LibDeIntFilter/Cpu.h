#define FEATURE_CPUID           0x00000001
#define FEATURE_STD_FEATURES    0x00000002
#define FEATURE_EXT_FEATURES    0x00000004
#define FEATURE_TSC             0x00000010
#define FEATURE_MMX             0x00000020
#define FEATURE_CMOV            0x00000040
#define FEATURE_3DNOW           0x00000080
#define FEATURE_3DNOWEXT        0x00000100
#define FEATURE_MMXEXT          0x00000200
#define FEATURE_SSEFP           0x00000400
#define FEATURE_K6_MTRR         0x00000800
#define FEATURE_P6_MTRR         0x00001000
#define FEATURE_SSE             0x00002000
#define FEATURE_SSE2            0x00004000

UINT CPU_SetupFeatureFlag(void);
