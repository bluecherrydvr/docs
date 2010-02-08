#ifndef __PTZ_PROTOCOL_HANDLER_H__
#define __PTZ_PROTOCOL_HANDLER_H__

#include "ExpressionParser.h"
#include "PTZProtocol.h"
#include "PTZParserDefine.h"
#include "Degree2PTZunit.h"

#define MAX_PROTOCOL_COUNT 5

const char _VENDER_PROTOCOL[][6][16] = 
{{{"CAM-5130\0"},{"Kampro\0"},{"\0"},{""},{""},{""}},
{{"CAM-5140\0"},{"Kampro\0"},{"\0"},{""},{""},{""}},
{{"CAM-5150\0"},{"Kampro\0"},{"\0"},{""},{""},{""}},
{{"CAM-5500\0"},{"Kampro\0"},{"\0"},{""},{""},{""}},
{{"CAM-6100\0"},{"Pelco-D\0"},{"Pelco-P\0"},{"VCL\0"},{"\0"},{""}},
{{"CAM-6200\0"},{"EyeView-V1\0"},{"EyeView-V2\0"},{"Pelco-D\0"},{"Pelco-P\0"},{"\0"}},
{{"CAM-6500\0"},{"DynaColor\0"},{"Pelco-D\0"},{"Pelco-P\0"},{"\0"},{""}},
{{"CAM-6600\0"},{"DynaColor\0"},{"Pelco-D\0"},{"Pelco-P\0"},{"\0"},{""}},				
{{"EyeView\0"},{"EyeView-V1\0"},{"EyeView-V2\0"},{"Pelco-D\0"},{"Pelco-P\0"},{"\0"}},
{{"GE\0"},{"Kalatel\0"},{"\0"},{""},{""},{""}},
{{"HarrisonTech\0"},{"VISCA\0"},{"\0"},{""},{""},{""}},
{{"Kampro\0"},{"Kampro\0"},{"V2\0"},{"\0"},{""},{""}},
{{"LiLin\0"},{"LiLin\0"},{"\0"},{""},{""},{""}},
{{"Messoa\0"},{"Pelco-D\0"},{"Pelco-P\0"},{"VCL\0"},{"\0"},{""}},
{{"Nicecam\0"},{"Nicecam\0"},{"\0"},{""},{""},{""}},
{{"Panasonic\0"},{"Panasonic\0"},{"\0"},{""},{""},{""}},
{{"Pelco\0"},{"Pelco-D\0"},{"Pelco-P\0"},{"Pelco-P-V1\0"},{"Pelco-P-V2\0"},{"\0"}},
{{"Pixim\0"},{"Pixim\0"},{"\0"},{""},{""},{""}},
{{"SamSung\0"},{"SamSung\0"},{"\0"},{""},{""},{""}},
{{"TOA\0"},{"TOA\0"},{"\0"},{""},{""},{""}},
{{"VCL\0"},{"VCL\0"},{"\0"},{""},{""},{""}},
{{"Vicon\0"},{"Vicon\0"},{"\0"},{""},{""},{""}},
{{"Videotrec\0"},{"Videotrec\0"},{"\0"},{""},{""},{""}},
{{"\0"},{""},{""},{""},{""},{""}},			// End of _VENDER_PROTOCOL
};


const char _PROTOCOL[][16] = 
{{"Kampro\0"}, 
{"Pelco-D\0"},
{"Pelco-P\0"},
{"VCL\0"},
{"EyeView-V1\0"},
{"EyeView-V2\0"},
{"DynaColor\0"},
{"Kalatel\0"},
{"VISCA\0"},
{"V2\0"},
{"LiLin\0"},
{"Nicecam\0"},
{"Panasonic\0"},
{"Pelco-P-V1\0"},
{"Pelco-P-V2\0"},
{"Pixim\0"},
{"SamSung\0"},
{"TOA\0"},
{"Vicon\0"},
{"Videotrec\0"},
{"\0"},					// End of _PROTOCOL
};

const char _VENDER[][16] =
{{"CAM-5130\0"},
{"CAM-5140\0"},
{"CAM-5150\0"},
{"CAM-5500\0"},
{"CAM-6100\0"},
{"CAM-6200\0"},
{"CAM-6500\0"},
{"CAM-6600\0"},		
{"EyeView\0"},
{"GE\0"},
{"HarrisonTech\0"},
{"Kampro\0"},
{"LiLin\0"},
{"Messoa\0"},
{"Nicecam\0"},
{"Panasonic\0"},
{"Pelco\0"},
{"Pixim\0"},
{"SamSung\0"},
{"TOA\0"},
{"VCL\0"},
{"Vicon\0"},
{"Videotrec\0"},
{"\0"},		// End of _VENDER
};

enum ByteRef 
{
	P_LO,
	P_HI,
	T_LO,
	T_HI,
	Z_LO,
	Z_HI,
	CHEK
};

class CPTZProtocolHandler
{
public:
	CPTZProtocolHandler();
	~CPTZProtocolHandler();
	//load protocol from resource
	bool LoadProtocolRS(PTZ_VENDER_PROTOCOL PTZVenderProtocol);
	bool LoadProtocolRS(char* szProtocol);
	
	//load protocol from file
	bool LoadProtocolFile(char* szProtocol);

	//clean all member objects
	bool UnloadProtocol();

	// get ptz command bytes. 
	bool GetCommand(PTZ_COMMAND PTZCmd, int nAddID, int iParam1, int iParam2, BYTE* pCommand, DWORD& dwCommandLen);
	bool GetCommand(char* pPTZCmd, int nAddID, int iParam1, int iParam2, BYTE* pCommand, DWORD& dwCommandLen);

	// get ptz absolute_position command bytes
	// fPanDegree 0 ~ 360 fRiltDegree 0 ~ 90 fZoomRatio 0 ~ 100
	bool GetAbsPTZCommand( char* pPTZCmd, 
						   int iParam1, 
						   int iParam2,
						   bool bPanCounterClock,
						   float fPanDegree,
						   float fTiltDegree,
						   float fZoomRatio,
						   BYTE* pCommand,
						   DWORD& dwCommandLen);

	// get ptz absolute_position command bytes
	// iPanUnit (hardware unit) iTiltUnit (hardware unit) iZoomUnit (hardware unit)
	bool GetAbsPTZCommandByUnit(  char* pPTZCmd, 
							int iParam1, 
							int iParam2,
							bool bPanCounterClock,
							int iPanUnit,
							int iTiltUnit,
							int iZoomUnit,
							BYTE* pCommand,
							DWORD& dwCommandLen);
	//iPanUnit (hardware unit) iTiltUnit (hardware unit) iZoomUnit (hardware unit) => fPanDegree 0 ~ 360 fRiltDegree 0 ~ 90 fZoomRatio 0 ~ 100
	void UnitToDegree(	int iPanUnit,
						int iTiltUnit,
						int iZoomUnit,
						float& fPanDegree,
						float& fTiltDegree,
						float& fZoomRatio);
	//fPanDegree 0 ~ 360 fRiltDegree 0 ~ 90 fZoomRatio 0 ~ 100 => iPanUnit (hardware unit) iTiltUnit (hardware unit) iZoomUnit (hardware unit)
	void DegreeToUnit(	float fPanDegree,
						float fTiltDegree,
						float fZoomRatio,
						int& iPanUnit,
						int& iTiltUnit,
						int& iZoomUnit);
	//To analyse the buffer (came from RS232 callback).
	//This function find the header, and parse out the iPanUnit, iTiltUnit, iZoomUnit
	bool GetUnitFromBuffer( BYTE* pDataBufferFromRS232CallBack,
							DWORD dwLengthOfBuffer,
							int& iPanUnit,
							int& iTiltUnit,
							int& iZoomUnit);
	// Get request command from PTZ file. Sending request command to device will receive PTZ position from rs232 callback
	bool GetRequestAbsPTZCommand(int iParam1,BYTE* pCommand,DWORD& dwCommandLen);


	// Enumerate return format : Count,Data1,Data2....
	// Give vender - return protocol.
	bool PTZEnumerateProtocol(PTZ_VENDER Vender, char* pProtocol, DWORD& dwLen);
	bool PTZEnumerateProtocol(char* pVender, char* pProtocol, DWORD& dwLen);
	// Get all vender
	bool PTZEnumerateVender(char* pVender, DWORD& dwLen);
	// Get ptz file support function
	bool PTZEnumerateFunction(char* pFunction, DWORD& dwLen);

	// Utility
	bool GetCommandStr(PTZ_COMMAND PTZCmd, char* szPTZCommand);
	bool GetVenderProtocolStr(PTZ_VENDER_PROTOCOL PTZVenderProtocol, char* szPTZVenderProtocol);
	bool GetVenderStr(PTZ_VENDER PTZVender, char* szVender);
	bool GetProtocolStr(PTZ_PROTOCOL PTZProtocol, char* szProtocol);

private:
	bool CovertStrCmdToByte(std::string sCmd, BYTE* pCmd, DWORD& dwLen);
	// Calculate CheckSum by CExpressionParser
	bool CalculateCheckSum(BYTE* bCmd, int nAddID, BYTE* bResult);
	int HexChar( char c );
	bool AssembleABSPTZCMD( int iParam2,
							int iPanUnit, 
							int iTiltUnit, 
							int iZoomUnit, 
							bool bPanCounterClock,
							std::string& sResult,
							std::vector<BYTE>& command_vector,
							int& vec_pos);
	//check if m_Deg2PTZobject be init , if not , init it
	void InitDeg2PTZobject();
	
private:
	CExpressionParser m_cEP;				// Class for handling checksum.
	CPTZProtocol m_cPTZProtocol;			// Class for handling protocol file.
	std::string m_sCheckSum;
	std::string m_sAddIDPos;
	std::string m_sADDIDStart;
	int		m_nCommandType;


	// 做string compare 會慢, 有機會要改成定 enumeration, compare 才快

	CDegree2PTZunit m_Deg2PTZobject;    // Class for changing Pan Tilt Zoom unit

	//parse buffer for PTZ unit 用的
	std::string m_strPanRequest;
	std::vector<BYTE> m_PanReqHeader; 
	std::map<ByteRef,DWORD> m_mapPanReqRule;
	
	std::string m_strTiltRequest;
	std::vector<BYTE> m_TiltReqHeader; 
	std::map<ByteRef,DWORD> m_mapTiltReqRule;
	
	std::string m_strZoomRequest;
	std::vector<BYTE> m_ZoomReqHeader; 
	std::map<ByteRef,DWORD> m_mapZoomReqRule;

	DWORD m_dwRequestPackageSize;
	DWORD m_dwRequestPackageHeaderSize;





};
#endif	// __PTZ_PROTOCOL_HANDLER_H__