#pragma once


#define SDK_DLL_MAJOR_VERSION 5
#define SDK_DLL_MINOR_VERSION 0
#define SDK_DLL_REVISION_VERSION 1
#define SDK_DLL_BUILD_VERSION 0

/*when modify version number, make sure the SDK_DLL__VERION STRING matches the individual numbers*/
#define SDK_DLL_VERSION_STRING "5.0.1.0\0"


#define SDVR_SDK_DLL_VERSION ((SDK_DLL_BUILD_VERSION << 24) | \
                             (SDK_DLL_REVISION_VERSION << 16) | \
                             (SDK_DLL_MINOR_VERSION << 8)  | \
                             (SDK_DLL_MAJOR_VERSION) )

