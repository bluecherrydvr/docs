#ifndef __TYPEDEF_H
#define __TYPEDEF_H


#include <wdm.h>        /* cannot be used in API */


/******************************************
 * Basic Type Definitions
 ******************************************/
#if !defined(VOID)
    typedef void                    VOID, *PVOID;
#endif

#if !defined(S8)
    typedef signed char             S8, *PS8;
#endif

#if !defined(U8)
    typedef unsigned char           U8, *PU8;
#endif

#if !defined(S16)
    typedef signed short            S16, *PS16;
#endif

#if !defined(U16)
    typedef unsigned short          U16, *PU16;
#endif

#if !defined(S32)
    typedef signed long             S32, *PS32;
#endif

#if !defined(U32)
    typedef unsigned long           U32, *PU32;
#endif

#if !defined(LONGLONG)
    typedef signed _int64           LONGLONG;
#endif

#if !defined(ULONGLONG)
    typedef unsigned _int64         ULONGLONG;
#endif

#if !defined(S64)
    typedef union _S64
    {
        struct
        {
            U32  LowPart;
            S32  HighPart;
        }u;

        LONGLONG QuadPart;
    } S64;
#endif

#if !defined(U64)
    typedef union _U64
    {
        struct
        {
            U32  LowPart;
            U32  HighPart;
        }u;

        ULONGLONG QuadPart;
    } U64;
#endif


/******************************************
 * Volatile Basic Type Definitions
 ******************************************/
#if !defined(VS8)
    typedef volatile signed char       VS8, *PVS8;
#endif

#if !defined(VU8)
    typedef volatile unsigned char     VU8, *PVU8;
#endif

#if !defined(VS16)
    typedef volatile signed short      VS16, *PVS16;
#endif

#if !defined(VU16)
    typedef volatile unsigned short    VU16, *PVU16;
#endif

#if !defined(VS32)
    typedef volatile signed long       VS32, *PVS32;
#endif

#if !defined(VU32)
    typedef volatile unsigned long     VU32, *PVU32;
#endif

#if !defined(VLONGLONG)
    typedef volatile signed _int64     VLONGLONG;
#endif

#if !defined(VULONGLONG)
    typedef volatile unsigned _int64   VULONGLONG;
#endif

#if !defined(VS64)
    typedef union _vS64
    {
        struct
        {
            volatile U32  LowPart;
            volatile S32  HighPart;
        }u;

        volatile LONGLONG QuadPart;
    } VS64;
#endif

#if !defined(VU64)
    typedef union _VU64
    {
        struct
        {
            volatile U32  LowPart;
            volatile U32  HighPart;
        }u;

        volatile ULONGLONG QuadPart;
    } VU64;
#endif


/******************************************
 * PCI SDK Defined Structures
 ******************************************/
/* Device Location Structure */
typedef struct _DEVICE_LOCATION
{
    U32 DeviceId;
    U32 VendorId;
    U32 BusNumber;
    U32 SlotNumber;
    U8  SerialNumber[16];
} DEVICE_LOCATION, *PDEVICE_LOCATION;


/* Power State Definitions */
typedef enum _PLX_POWER_LEVEL
{
    D0Uninitialized,
    D0,
    D1,
    D2,
    D3Hot,
    D3Cold
} PLX_POWER_LEVEL, *PPLX_POWER_LEVEL;

#endif
