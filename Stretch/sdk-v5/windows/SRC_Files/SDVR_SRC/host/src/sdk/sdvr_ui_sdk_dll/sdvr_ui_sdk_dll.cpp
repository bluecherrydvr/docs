// sdvr_sdk_dll.cpp : Defines the initialization routines for the DLL.
//

#define _WIN32_WINNT 0x500
#include <windows.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/**********************************************************************
    DLL entry point.
**********************************************************************/
BOOL APIENTRY
DllMain( HANDLE, 
         DWORD  ul_reason_for_call, 
         LPVOID )
{
    switch( ul_reason_for_call )
    {
    case DLL_PROCESS_ATTACH:

        //Trace( "DllMain() : DLL_PROCESS_ATTACH\n" );       
        break;

    case DLL_PROCESS_DETACH:

        //Trace( "DllMain() : DLL_PROCESS_DETACH\n" );      

        break;

    case DLL_THREAD_ATTACH:

        //Trace( "DllMain() : DLL_THREAD_ATTACH\n" );
        break;

    case DLL_THREAD_DETACH:

        //Trace( "DllMain() : DLL_THREAD_DETACH\n" );
        break;
    }

    return TRUE;
}
