// PTZParser.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "PTZParser.h"
#include "PTZProtocolHandler.h"
#include "PTZparserDefine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

// CPTZParserApp

// CPTZParserApp construction

CPTZParserApp::CPTZParserApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

CPTZParserApp::~CPTZParserApp()
{

}

// The one and only CPTZParserApp object

CPTZParserApp theApp;


extern "C" __declspec(dllexport) HANDLE PTZOpenInterface()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return (HANDLE)new CPTZProtocolHandler();
}

extern "C" __declspec(dllexport) void PTZCloseInterface(HANDLE h)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CPTZProtocolHandler* p = (CPTZProtocolHandler*)h;
	if( p ) 
	{
		delete p;
	}
}

//extern "C" __declspec(dllexport) bool PTZLoadProtocolRS(HANDLE h, PTZ_VENDER_PROTOCOL PTZVenderProtocol)
//{
//	AFX_MANAGE_STATE(AfxGetStaticModuleState());
//	CPTZProtocolHandler* p = (CPTZProtocolHandler*)h;
//	if( p ) 
//	{
//		return p->LoadProtocolRS(PTZVenderProtocol);
//	}
//	return false;
//}

extern "C" __declspec(dllexport) bool PTZLoadProtocolRS(HANDLE h, char* szVenderProtocol)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CPTZProtocolHandler* p = (CPTZProtocolHandler*)h;
	if( p ) 
	{
		return p->LoadProtocolRS(szVenderProtocol);
	}
	return false;
}

extern "C" __declspec(dllexport) bool PTZLoadProtocolFile(HANDLE h, char* szProtocol)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CPTZProtocolHandler* p = (CPTZProtocolHandler*)h;
	if( p ) 
	{
		return p->LoadProtocolFile(szProtocol);
	}
	return false;
}

extern "C" __declspec(dllexport) bool PTZUnloadProtocol(HANDLE h)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CPTZProtocolHandler* p = (CPTZProtocolHandler*)h;
	if( p ) 
	{
		p->UnloadProtocol();
	}
	return true;
}

extern "C" __declspec(dllexport) bool PTZGetCommand(HANDLE h, PTZ_COMMAND PTZCmd, int nAddID, int iParam1, int iParam2, BYTE* pCommand, DWORD& dwCommandLen)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CPTZProtocolHandler* p = (CPTZProtocolHandler*)h;
	if( p ) 
	{
		return p->GetCommand(PTZCmd, nAddID, iParam1, iParam2, pCommand, dwCommandLen);
	}
	return true;
}

extern "C" __declspec(dllexport) bool PTZGetAbsPTZCommand(HANDLE h, 
														  char* pPTZCmd, 
														  int iParam1, 
														  int iParam2,
														  bool bPanCounterClock,
														  float fPanDegree,
														  float fTiltDegree,
														  float fZoomRatio,
														  BYTE* pCommand,
														  DWORD& dwCommandLen)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CPTZProtocolHandler* p = (CPTZProtocolHandler*)h;
	if( p ) 
	{
		return p->GetAbsPTZCommand( pPTZCmd, 
									iParam1, 
									iParam2,
									bPanCounterClock,
									fPanDegree,
									fTiltDegree,
									fZoomRatio,
									pCommand,
									dwCommandLen);
	}
	return true;
}

extern "C" __declspec(dllexport) bool PTZGetAbsPTZCommandByUnit(HANDLE h, 
														  char* pPTZCmd, 
														  int iParam1, 
														  int iParam2,
														  bool bPanCounterClock,
														  int iPanUnit,
														  int iTiltUnit,
														  int iZoomUnit,
														  BYTE* pCommand,
														  DWORD& dwCommandLen)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CPTZProtocolHandler* p = (CPTZProtocolHandler*)h;
	if( p ) 
	{
		return p->GetAbsPTZCommandByUnit( pPTZCmd, 
			iParam1, 
			iParam2,
			bPanCounterClock,
			iPanUnit,
			iTiltUnit,
			iZoomUnit,
			pCommand,
			dwCommandLen);

	}
	return true;
}

extern "C" __declspec(dllexport) void PTZUnitToDegree(HANDLE h, 
													  int iPanUnit,
													  int iTiltUnit,
													  int iZoomUnit,
													  float& fPanDegree,
													  float& fTiltDegree,
													  float& fZoomRatio)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CPTZProtocolHandler* p = (CPTZProtocolHandler*)h;
	if( p ) 
	{
		p->UnitToDegree( iPanUnit,
						iTiltUnit,
						iZoomUnit,
						fPanDegree,
						fTiltDegree,
						fZoomRatio);
	}
	return;
}
extern "C" __declspec(dllexport) void PTZDegreeToUnit(HANDLE h, 
													  float fPanDegree,
													  float fTiltDegree,
													  float fZoomRatio,
													  int& iPanUnit,
													  int& iTiltUnit,
													  int& iZoomUnit)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CPTZProtocolHandler* p = (CPTZProtocolHandler*)h;
	if( p ) 
	{
		p->DegreeToUnit( fPanDegree,
						fTiltDegree,
						fZoomRatio,
						iPanUnit,
						iTiltUnit,
						iZoomUnit);
	}
	return;
}
extern "C" __declspec(dllexport) bool PTZGetUnitFromBuffer(HANDLE h, 
														   BYTE* pDataBufferFromRS232CallBack,
														   DWORD dwLengthOfBuffer,
														   int& iPanUnit,
														   int& iTiltUnit,
														   int& iZoomUnit)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CPTZProtocolHandler* p = (CPTZProtocolHandler*)h;
	if( p ) 
	{
		return p->GetUnitFromBuffer( pDataBufferFromRS232CallBack,
								dwLengthOfBuffer,
								iPanUnit,
								iTiltUnit,
								iZoomUnit);
	}
	return true;
}
extern "C" __declspec(dllexport) bool PTZGetRequestAbsPTZCommand(HANDLE h, int iParam1,BYTE* pCommand,DWORD& dwCommandLen)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CPTZProtocolHandler* p = (CPTZProtocolHandler*)h;
	if( p ) 
	{
		return p->GetRequestAbsPTZCommand(  iParam1, pCommand, dwCommandLen);
	}
	return true;
}
extern "C" __declspec(dllexport) bool PTZGetCommandExt(HANDLE h, char* pPTZCmd, int nAddID, int iParam1, int iParam2, BYTE* pCommand, DWORD& dwCommandLen)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CPTZProtocolHandler* p = (CPTZProtocolHandler*)h;
	if( p ) 
	{
		return p->GetCommand(pPTZCmd, nAddID, iParam1, iParam2, pCommand, dwCommandLen);
	}
	return true;
}

extern "C" __declspec(dllexport) bool PTZEnumerateProtocol(HANDLE h, char* pVender, char* pProtocol, DWORD& dwLen)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CPTZProtocolHandler* p = (CPTZProtocolHandler*)h;
	if( p ) 
	{
		return p->PTZEnumerateProtocol(pVender, pProtocol, dwLen);
	}
	return true;
}

//extern "C" __declspec(dllexport) bool PTZEnumerateProtocol(HANDLE h, PTZ_VENDER Vender, char* pProtocol, DWORD& dwLen)
//{
//	AFX_MANAGE_STATE(AfxGetStaticModuleState());
//	CPTZProtocolHandler* p = (CPTZProtocolHandler*)h;
//	if( p ) 
//	{
//		return p->PTZEnumerateProtocol(Vender, pProtocol, dwLen);
//	}
//	return true;
//}

extern "C" __declspec(dllexport) bool PTZEnumerateVender(HANDLE h, char* pVender, DWORD& dwLen)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CPTZProtocolHandler* p = (CPTZProtocolHandler*)h;
	if( p ) 
	{
		return p->PTZEnumerateVender(pVender, dwLen);
	}
	return true;
}

extern "C" __declspec(dllexport) bool PTZEnumerateFunction(HANDLE h, char* pFunction, DWORD& dwLen)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CPTZProtocolHandler* p = (CPTZProtocolHandler*)h;
	if( p ) 
	{
		return p->PTZEnumerateFunction(pFunction, dwLen);
	}
	return true;
}

extern "C" __declspec(dllexport) bool PTZGetCommandStr(HANDLE h, PTZ_COMMAND PTZCmd, char* szPTZCommand)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CPTZProtocolHandler* p = (CPTZProtocolHandler*)h;
	if( p ) 
	{
		return p->GetCommandStr(PTZCmd, szPTZCommand);
	}
	return true;
}

extern "C" __declspec(dllexport) bool PTZGetVenderProtocolStr(HANDLE h, PTZ_VENDER_PROTOCOL PTZVenderProtocol, char* szPTZVenderProtocol)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CPTZProtocolHandler* p = (CPTZProtocolHandler*)h;
	if( p ) 
	{
		return p->GetVenderProtocolStr(PTZVenderProtocol, szPTZVenderProtocol);
	}
	return true;
}

extern "C" __declspec(dllexport) bool PTZGetVenderStr(HANDLE h, PTZ_VENDER PTZVender, char* szPTZVender)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CPTZProtocolHandler* p = (CPTZProtocolHandler*)h;
	if( p ) 
	{
		return p->GetVenderStr(PTZVender, szPTZVender);
	}
	return true;
}

extern "C" __declspec(dllexport) bool PTZGetProtocolStr(HANDLE h, PTZ_PROTOCOL PTZProtocol, char* szPTZProtocol)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CPTZProtocolHandler* p = (CPTZProtocolHandler*)h;
	if( p ) 
	{
		return p->GetProtocolStr(PTZProtocol, szPTZProtocol);
	}
	return true;
}