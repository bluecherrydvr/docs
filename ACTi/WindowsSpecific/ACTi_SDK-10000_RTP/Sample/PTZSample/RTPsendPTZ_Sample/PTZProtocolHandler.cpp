#include "stdafx.h"
#include "PTZProtocolHandler.h"
#include "PTZParserDefine.h"

CPTZProtocolHandler::CPTZProtocolHandler()
{
	m_sCheckSum.clear();
	m_sAddIDPos.clear();
	m_sADDIDStart.clear();
	m_nCommandType = 0;

	m_dwRequestPackageSize = 0;
	m_dwRequestPackageHeaderSize = 0;
}

CPTZProtocolHandler::~CPTZProtocolHandler()
{
	m_sCheckSum.clear();
	m_sAddIDPos.clear();
	m_sADDIDStart.clear();
}

bool CPTZProtocolHandler::LoadProtocolRS(char* szProtocol)
{
	bool bRet = false;
	UnloadProtocol();
	bRet = m_cPTZProtocol.LoadProtocolRS(szProtocol);
	if(bRet)
	{
		m_cPTZProtocol.GetCheckSum(m_sCheckSum);
		m_cPTZProtocol.GetAddIDPos(m_sAddIDPos);
		m_cPTZProtocol.GetAddIDStart(m_sADDIDStart);

		m_cPTZProtocol.GetCommandType( m_nCommandType );
	}
	return bRet;
}

bool CPTZProtocolHandler::LoadProtocolRS(PTZ_VENDER_PROTOCOL PTZVenderProtocol)
{
	bool bRet = false;
	char szPTZVenderProtocol[128] = {0};
	if(!GetVenderProtocolStr(PTZVenderProtocol, szPTZVenderProtocol))
		return false;
	UnloadProtocol();
	bRet = m_cPTZProtocol.LoadProtocolRS(szPTZVenderProtocol);
	if(bRet)
	{
		m_cPTZProtocol.GetCheckSum(m_sCheckSum);
		m_cPTZProtocol.GetAddIDPos(m_sAddIDPos);
		m_cPTZProtocol.GetAddIDStart(m_sADDIDStart);

		m_cPTZProtocol.GetCommandType( m_nCommandType );
	}
	return bRet;
}

bool CPTZProtocolHandler::LoadProtocolFile(char* szProtocol)
{
	bool bRet = false;
	UnloadProtocol();
	bRet = m_cPTZProtocol.LoadProtocolFile(szProtocol);
	if(bRet)
	{
		m_cPTZProtocol.GetCheckSum(m_sCheckSum);
		m_cPTZProtocol.GetAddIDPos(m_sAddIDPos);
		m_cPTZProtocol.GetAddIDStart(m_sADDIDStart);

		m_cPTZProtocol.GetCommandType( m_nCommandType );
	}
	return bRet;
}

bool CPTZProtocolHandler::UnloadProtocol()
{
	m_cPTZProtocol.Reset();
	m_cEP.ResetExpression();
	m_sCheckSum.clear();
	m_sAddIDPos.clear();
	m_sADDIDStart.clear();
	return true;
}

bool CPTZProtocolHandler::GetCommandStr(PTZ_COMMAND PTZCmd, char* szPTZCommand)
{
	switch(PTZCmd)
	{
	case PTZ_MOVE_LEFT:
		strcpy(szPTZCommand, "PANLEFT\0");
		break;
	case PTZ_MOVE_RIGHT:
		sprintf(szPTZCommand, "PANRIGHT\0");
		break;
	case PTZ_MOVE_UP:
		sprintf(szPTZCommand, "TILTUP\0");
		break;
	case PTZ_MOVE_DOWN:
		sprintf(szPTZCommand, "TILTDOWN\0");
		break;
	case PTZ_MOVE_STOP:
		sprintf(szPTZCommand, "PANTILTSTOP\0");
		break;
	case PTZ_MOVE_HOME:
		sprintf(szPTZCommand, "PANTILTHOME\0");
		break;
	case PTZ_OSD_ON:
		sprintf(szPTZCommand, "OSDON\0");
		break;
	case PTZ_OSD_OFF:
		sprintf(szPTZCommand, "OSDOFF\0");
		break;
	case PTZ_OSD_UP:
		sprintf(szPTZCommand, "OSDUP\0");
		break;
	case PTZ_OSD_DOWN:
		sprintf(szPTZCommand, "OSDDOWN\0");
		break;
	case PTZ_OSD_LEFT:
		sprintf(szPTZCommand, "OSDLEFT\0");
		break;
	case PTZ_OSD_RIGHT:
		sprintf(szPTZCommand, "OSDRIGHT\0");
		break;
	case PTZ_OSD_LEAVE:
		sprintf(szPTZCommand, "OSDLEAVE\0");
		break;
	case PTZ_OSD_STOP:
		sprintf(szPTZCommand, "OSDSTOP\0");
		break;
	case PTZ_OSD_ENTER:
		sprintf(szPTZCommand, "OSDENTER\0");
		break;
	case PTZ_FOCUS_IN:
		sprintf(szPTZCommand, "FOCUSIN\0");
		break;
	case PTZ_FOCUS_OUT:
		sprintf(szPTZCommand, "FOCUSOUT\0");
		break;
	case PTZ_FOCUS_STOP:
		sprintf(szPTZCommand, "FOCUSSTOP\0");
		break;
	case PTZ_IRIS_OPEN:
		sprintf(szPTZCommand, "IRISOPEN\0");
		break;
	case PTZ_IRIS_CLOSE:
		sprintf(szPTZCommand, "IRISCLOSE\0");
		break;
	case PTZ_IRIS_STOP:
		sprintf(szPTZCommand, "IRISSTOP\0");
		break;
	case PTZ_IRIS_AUTO:
		sprintf(szPTZCommand, "IRISAUTO\0");
		break;
	case PTZ_PRESET_SET:
		sprintf(szPTZCommand, "PRESETSET\0");
		break;
	case PTZ_PRESET_GOTO:
		sprintf(szPTZCommand, "PRESETGOTO\0");
		break;
	case PTZ_PRESET_CLEAR:
		sprintf(szPTZCommand, "PRESETCLEAR\0");
		break;
	case PTZ_PRSSET_TOUR:
		sprintf(szPTZCommand, "PRESETTOUR\0");
		break;
	case PTZ_ZOOM_IN:
		sprintf(szPTZCommand, "ZOOMIN\0");
		break;
	case PTZ_ZOOM_OUT:
		sprintf(szPTZCommand, "ZOOMOUT\0");
		break;
	case PTZ_ZOOM_STOP:
		sprintf(szPTZCommand, "ZOOMSTOP\0");
		break;
	case PTZ_BLC_ON:
		sprintf(szPTZCommand, "BLC\0");
		break;
	case PTZ_BLC_OFF:
		sprintf(szPTZCommand, "BLC\0");
		break;
	case PTZ_DAYNIGHT_ON:
		sprintf(szPTZCommand, "DAYNIGHT\0");
		break;
	case PTZ_DAYNIGHT_OFF:
		sprintf(szPTZCommand, "DAYNIGHT\0");
		break;
	case PTZ_DAYNIGHT_AUTO_ON:
		sprintf(szPTZCommand, "DAYNIGHTAUTO\0");
		break;
	case PTZ_DAYNIGHT_AUTO_OFF:
		sprintf(szPTZCommand, "DAYNIGHTAUTO\0");
		break;
	case PTZ_MOVE_UPLEFT:
		sprintf(szPTZCommand, "MOVEUPLEFT\0");
		break;
	case PTZ_MOVE_UPRIGHT:
		sprintf(szPTZCommand, "MOVEUPRIGHT\0");
		break;
	case PTZ_MOVE_DOWNLEFT:
		sprintf(szPTZCommand, "MOVEDOWNLEFT\0");
		break;
	case PTZ_MOVE_DOWNRIGHT:
		sprintf(szPTZCommand, "MOVEDOWNRIGHT\0");
		break;
	default:
		return false;
	}
	return true;
}

bool CPTZProtocolHandler::CovertStrCmdToByte(std::string sCmd, BYTE* pCmd, DWORD& dwLen)
{
	int iBegin = 0;
	int iEnd = 0;
	std::string sSub;
	for(int i = 0; i < sCmd.length(); i++)
	{
		if(',' == sCmd[i])
		{
			if(0 != iBegin)
				iBegin += 1;
			sSub = sCmd.substr(iBegin, iEnd-iBegin);
			iBegin = iEnd;
			// Place BYTE value start from array pos 1 not 0
			dwLen += 1;
			sscanf(sSub.c_str(), "0x%X", &pCmd[dwLen]);
		}
		iEnd++;
	}
	if(0 != iBegin)
		iBegin += 1;
	sSub = sCmd.substr(iBegin, iEnd-iBegin);
	dwLen += 1;
	sscanf(sSub.c_str(), "0x%X", &pCmd[dwLen]);

	return true;
}

bool CPTZProtocolHandler::CalculateCheckSum(BYTE* bCmd, int nAddID, BYTE* bResult)
{
	if(m_sCheckSum.empty() || m_sADDIDStart.empty() || m_sAddIDPos.empty())
	{
		return false;
	}

	int nPos = m_sCheckSum.find('=');
	std::string sFunction = m_sCheckSum.substr(nPos+1);			// checksum function
	std::string sValue = m_sCheckSum.substr(0, nPos);			// checksum value
	// Get checksum position (S)
	DWORD dwChecksumPos;
	char szCheckSumPos[16] = {0};
	int nCSTemp = 0;
	for(int k = 0; k < sValue.length(); k++)
	{
		if(isdigit(sValue[k]))
		{
			szCheckSumPos[nCSTemp] = sValue[k];
			nCSTemp++;
		}
	}
	sscanf(szCheckSumPos, "%d", &dwChecksumPos);
	// Get checksum position (E)
	// Get Addr ID pos
	DWORD dwAddIDPos = atoi(m_sAddIDPos.c_str()); 
	
	// Handle Addr Start Pos with Hex value (S)
	bool bHex = false;
	DWORD dwAddIDStart;
	char szHex[16] = {0};
	for(int nHex = 0; nHex < m_sADDIDStart.length(); nHex++)
	{
		if('x' == m_sADDIDStart[nHex] || 'X' == m_sADDIDStart[nHex])
		{
			strcpy(szHex, m_sADDIDStart.c_str());
			sscanf(szHex, "0x%X", &dwAddIDStart);
			bHex = true;
			break;
		}
	}
	if(!bHex)
	{
		dwAddIDStart = atoi(m_sADDIDStart.c_str());
	}
	// Handle Addr Start Pos with Hex value (E)

	char szSubID[16] = {0};
	DWORD dwSubID = nAddID + dwAddIDStart - 1;
	sprintf(szSubID, "%d", dwSubID);
	// Fix the variable byte
	bCmd[dwAddIDPos] = (BYTE)dwSubID;
	bResult[dwAddIDPos] = (BYTE)dwSubID;

	std::string sTemp;
	for(int i = 0; i < sFunction.length(); i++)
	{
		if('$' == sFunction[i])
		{
			i++;
			if('C' == sFunction[i] || 'c' == sFunction[i])
			{
				i++;
				char szTemp[16] = {0};
				int nTemp = 0;
				for(int j = i; j < sFunction.length(); j++)
				{
					if('x' == sFunction[j] || 'X' == sFunction[j] || isdigit(sFunction[j]) || (sFunction[j] >= 'a' && sFunction[j] <= 'f') || (sFunction[j] >= 'A' && sFunction[j] <= 'F'))
					{
						szTemp[nTemp] = sFunction[j];
						i++;
						nTemp++;
					}
					else
					{
						i--;
						break;
					}
				}
				DWORD dwTemp;
				sscanf(szTemp, "0x%X", &dwTemp);
				memset(szTemp, 0x00, 16);
				sprintf(szTemp, "%d", dwTemp);
				sTemp.append(szTemp);
			}
			else if('B' == sFunction[i] || 'b' == sFunction[i])
			{
				i++;
				char szTemp[16] = {0};
				int nTemp = 0;
				for(int j = i; j < sFunction.length(); j++)
				{
					if(isdigit(sFunction[j]))
					{
						szTemp[nTemp] = sFunction[j];
						i++;
						nTemp++;
					}
					else
					{
						i--;
						break;
					}
				}
				DWORD dwTemp = 0;
				DWORD dwTemp2 = 0;
				sscanf(szTemp, "%d", &dwTemp);
				memset(szTemp, 0x00, 16);
				sprintf(szTemp, "%d", bCmd[dwTemp]);
				sTemp.append(szTemp);
			}
			else
			{
				return false;
			}
		}
		//else if('+' != sFunction[i] || '-' != sFunction[i] || '^' != sFunction[i] || '//' != sFunction[i] || ')' != sFunction[i] || '(' != sFunction[i] || '|' != sFunction[i])
		else
		{
			char c[2] = {0};
			sprintf(c, "%c", sFunction[i]);
			sTemp.append(c);
		}
	}
	
	int nResult = 0;
	if(m_cEP.SetExpression(sTemp))
	{
		// Set checksum value.
		int nResult = m_cEP.GetResult();
		bResult[dwChecksumPos] = nResult & 0x000000FF;
		m_cEP.ResetExpression();
	}
	else
	{
		return false;
	}

	return true;
}

bool CPTZProtocolHandler::GetCommand(PTZ_COMMAND PTZCmd, int nAddID, int iParam1, int iParam2, BYTE* pCommand, DWORD& dwCommandLen)
{
	bool bRet = false;
	char szPTZCommand[128] = {0};
	
	if(!GetCommandStr(PTZCmd, szPTZCommand))
		return false;

	if( m_nCommandType == 0 )
	{
		std::string sResult;
		DWORD dwResultCount;
		if(m_cPTZProtocol.GetFunctionValue(szPTZCommand, iParam1, sResult))
		{
			if(!sResult.empty())
			{
				BYTE bCommand[128] = {0};			// bCommand[0] is not using.
				DWORD dwLen = 0;

				CovertStrCmdToByte(sResult, bCommand, dwLen);

				// Calculate checksum
				std::string::size_type loc = m_sCheckSum.find( "$D", 0 );

				if(std::string::npos == loc)
				{
					BYTE bTemp[128] = {0};
					memcpy(bTemp, bCommand, 128);
					if(!CalculateCheckSum(bCommand, nAddID, bTemp))
						return false;
					memset(bCommand, 0x00, 128);
					// Value start from array pos 1 not 0 see CovertStrCmdToByte
					memcpy(bCommand+1, bTemp+1, dwLen);
				}

				// Pass out, copy it to pos 0
				memcpy(pCommand, bCommand+1, dwLen);
				dwCommandLen = dwLen;
				bRet = true;
			}	
		}
	}

	else if( m_nCommandType == 1 )
	{
		std::string sResult;
		DWORD dwResultCount;
		if(m_cPTZProtocol.GetFunctionValue(szPTZCommand, iParam1, sResult))
		{
			if(!sResult.empty())
			{
				BYTE bCommand[128] = {0};			// bCommand[0] is not using.
				DWORD dwLen = 0;

				CovertStrCmdToByte(sResult, bCommand, dwLen);

				memcpy(pCommand, bCommand+1, dwLen);
				dwCommandLen = dwLen;
				bRet = true;
			}	
		}
		/*
		std::string sResult;
		if(m_cPTZProtocol.GetFunctionValue(szPTZCommand, iParam1, sResult))
		{
			if(!sResult.empty())
			{
			DWORD dwResultCount = sResult.length();

			BYTE bCommand[128] = {0};
			memcpy( bCommand, sResult.c_str(), dwResultCount );
			bCommand[ dwResultCount ] = 0;

			int nCnt = 0;
			DWORD dwStrDataLen = dwResultCount;
			dwResultCount = 0;
			while( dwStrDataLen > nCnt )
			{
			pCommand[dwResultCount] = (HexChar( bCommand[nCnt] ) * 16) + HexChar( bCommand[nCnt+1] );
			dwResultCount++;
			nCnt += 2;
			}
			dwCommandLen = dwResultCount;
			}
		}
		*/
	}
	
	else if( m_nCommandType == 2 )
	{
		std::string sResult;
		if(m_cPTZProtocol.GetFunctionValue(szPTZCommand, iParam1, sResult))
		{
			if(!sResult.empty())
			{
				DWORD dwResultCount = sResult.length();
				memcpy( pCommand, sResult.c_str(), dwResultCount );
				pCommand[ dwResultCount ] = 0;
				dwCommandLen = dwResultCount;
			}
		}
	}
	return bRet;
}

bool CPTZProtocolHandler::AssembleABSPTZCMD(int iParam2,
											int iPanUnit, 
											int iTiltUnit, 
											int iZoomUnit, 
											bool bPanCounterClock, 
											std::string& sResult,
											std::vector<BYTE>& command_vector,
											int& vec_pos)
{
	bool bRet = false;

	std::string strPTZinfo;
	std::string strResult;

	DWORD dCounterClockFlag = 0;
	strPTZinfo = "PANCOUNTERCLOCK";
	m_cPTZProtocol.GetAttValue(strPTZinfo, strResult);
	sscanf(strResult.c_str(), "%X", &dCounterClockFlag); 

	//add pan direction bit
	if(bPanCounterClock == true)
	{
		iPanUnit |= dCounterClockFlag;
	}

	// calculate P_LO, P_HI
	BYTE P_LO = iPanUnit & 0x000000FF;
	BYTE P_HI = (iPanUnit & 0x0000FF00)>>8;
	// calculate T_LO, P_HI
	BYTE T_LO = iTiltUnit & 0x000000FF;
	BYTE T_HI = (iTiltUnit & 0x0000FF00)>>8;
	// calculate Z_LO, Z_HI
	int iZ_HI;
	int iZ_LO;
	BYTE Z_LO = 0;//(BYTE)fZoomRatio; 
	BYTE Z_HI = 0;//0xff; 
	
	if(iParam2!=0)
	{
		//Z_LO = (BYTE)fZoomRatio; 
		//Z_HI = 0xff; 
	}
	else
	{
		strPTZinfo = "ABSZOOM";
		m_cPTZProtocol.GetFunctionValue(strPTZinfo, iZoomUnit, strResult);
		sscanf(strResult.c_str(), "%X,%X", &iZ_LO, &iZ_HI);
		Z_LO = (BYTE)(iZ_LO & 0x000000FF);
		Z_HI = (BYTE)(iZ_HI & 0x000000FF);
	}
	


	// assemble the unsigned char commands
	int iCheckSumPos = -1;
	std::string::size_type idx_start;
	std::string::size_type idx;
	std::string::size_type idx2;
	idx_start = 0; 
	idx = idx_start;
	idx2 = idx_start;

	while(idx != std::string::npos)
	{
		std::string temp_str;
		idx2 = sResult.find(',',idx+1);
		if(idx2 == std::string::npos)
		{
			if(idx != idx_start)
				++idx;
			temp_str = sResult.substr(idx);
		}
		else
		{
			if(idx != idx_start)
				++idx;
			temp_str = sResult.substr(idx,idx2-(idx));
		}
		//deal with sub string
		if(temp_str == "P_HI")
		{
			command_vector[vec_pos] = P_HI;
		}
		else if(temp_str == "P_LO")
		{
			command_vector[vec_pos] = P_LO;
		}
		else if(temp_str == "T_LO")
		{
			command_vector[vec_pos] = T_LO;
		}
		else if(temp_str == "T_HI")
		{
			command_vector[vec_pos] = T_HI;
		}
		else if(temp_str == "Z_LO")
		{
			command_vector[vec_pos] = Z_LO;
		}
		else if(temp_str == "Z_HI")
		{
			command_vector[vec_pos] = Z_HI;
		}
		else if(temp_str == "CHEK")
		{
			unsigned char cmd = 0;
			command_vector[vec_pos] = cmd;
			iCheckSumPos = vec_pos;
		}
		else
		{
			if(temp_str.find("0x") != std::string::npos)
			{
				int cmd = 0;
				sscanf(temp_str.c_str(), "%X", &cmd);
				command_vector[vec_pos] = (unsigned char)cmd;
			}
		}


		idx = idx2;
		vec_pos++;
	}



	// calculate check sum
	unsigned char cCheckSum = 0;
	for(int i = 0; i<vec_pos; ++i)
	{
		cCheckSum = cCheckSum^command_vector[i];
	}
	// there should be a check sum
	if(iCheckSumPos >= 0)
	{
		command_vector[iCheckSumPos] = cCheckSum;
	}


	return bRet = true;
}
void CPTZProtocolHandler::InitDeg2PTZobject()
{

	if(!m_Deg2PTZobject.IsInit())
	{
		std::string strPTZinfo;
		std::string strResult;
		//get attribute for absolute position
		int iPMAX = 0;
		int iPMIN = 0;
		int iTMAX = 0;
		int iTMIN = 0;
		int iZMAX = 0;
		int iZMIN = 0;

		int iPMaxDegree = 0;
		int iTMaxDegree = 0;

		strPTZinfo = "PMAX";
		m_cPTZProtocol.GetAttValue(strPTZinfo, strResult);
		iPMAX = atoi (strResult.c_str());
		strPTZinfo = "PMIN";
		m_cPTZProtocol.GetAttValue(strPTZinfo, strResult);
		iPMIN = atoi (strResult.c_str());

		strPTZinfo = "TMAX";
		m_cPTZProtocol.GetAttValue(strPTZinfo, strResult);
		iTMAX = atoi (strResult.c_str());
		strPTZinfo = "TMIN";
		m_cPTZProtocol.GetAttValue(strPTZinfo, strResult);
		iTMIN = atoi (strResult.c_str());

		strPTZinfo = "ZMAX";
		m_cPTZProtocol.GetAttValue(strPTZinfo, strResult);
		iZMAX = atoi (strResult.c_str());
		strPTZinfo = "ZMIN";
		m_cPTZProtocol.GetAttValue(strPTZinfo, strResult);
		iZMIN = atoi (strResult.c_str());

		strPTZinfo = "PMAXDEGREE";
		m_cPTZProtocol.GetAttValue(strPTZinfo, strResult);
		iPMaxDegree = atoi (strResult.c_str());
		strPTZinfo = "TMAXDEGREE";
		m_cPTZProtocol.GetAttValue(strPTZinfo, strResult);
		iTMaxDegree = atoi (strResult.c_str());

		//Setup unit converter 		//CDegree2PTZunit m_Deg2PTZobject;
		m_Deg2PTZobject.SetPanUnit((float) iPMAX, (float) iPMIN);
		m_Deg2PTZobject.SetTiltUnit((float) iTMAX, (float) iTMIN);
		m_Deg2PTZobject.SetZoomUnit((float) iZMAX, (float) iZMIN);
		//set degree limit
		if(iPMaxDegree != 0 && iTMaxDegree != 0)
		{
			m_Deg2PTZobject.SetMaxPanDegreeAnsMaxTiltDegree((float)iPMaxDegree,(float)iTMaxDegree);
		}

	}
}
bool CPTZProtocolHandler::GetAbsPTZCommand( char* pPTZCmd, 
											int iParam1, 
											int iParam2,
											bool bPanCounterClock,
											float fPanDegree,
											float fTiltDegree,
											float fZoomRatio,
											BYTE* pCommand,
											DWORD& dwCommandLen)
{
	bool bRet = false;
	std::string strPTZCmd = pPTZCmd;
	std::string sResult;
	if(m_cPTZProtocol.GetFunctionValue(strPTZCmd, iParam1, sResult))
	{
		if(sResult.empty())
		{
			return bRet;
		}

		InitDeg2PTZobject();


		int iPanUnit = 0;
		int iTiltUnit = 0;
		int iZoomUnit = 0;

		// rotate Y-axis (0 ~ 360 ) mapping to ( fMaxPan ~ fMinPan )
		iPanUnit = (int)m_Deg2PTZobject.GetPanUnitFromYAXISRotateDegree(fPanDegree);
		// rotate X-axis ( 0 ~ 90 ) mapping to ( fMaxTilt ~ fMinTilt )
		iTiltUnit = (int)m_Deg2PTZobject.GetTiltUnitFromXAXISRotateDegree(fTiltDegree);
		// get zoom degree (0 ~ 100 ) mapping to ( fMinZoom ~ fMaxZoom )
		iZoomUnit = (int)m_Deg2PTZobject.GetZoomUnitFromZoomPercent(fZoomRatio);


		std::vector<BYTE> command_vector;
		command_vector.resize(128);
		int vec_pos = 0;// 改用resize , 所以要加 vector pos
		

		AssembleABSPTZCMD(iParam2,
			iPanUnit, 
			iTiltUnit, 
			iZoomUnit, 
			bPanCounterClock, 
			sResult,
			command_vector,
			vec_pos);



		for(int i = 0; i<vec_pos; ++i)
		{
			pCommand[i] = command_vector[i];
		}
		dwCommandLen = vec_pos;
		
		command_vector.clear();

	}





	return bRet = true;
}

// get ptz absolute_position command bytes
// iPanUnit (hardware unit) iTiltUnit (hardware unit) iZoomUnit (hardware unit)
bool CPTZProtocolHandler::GetAbsPTZCommandByUnit(  char* pPTZCmd, 
					  int iParam1, 
					  int iParam2,
					  bool bPanCounterClock,
					  int iPanUnit,
					  int iTiltUnit,
					  int iZoomUnit,
					  BYTE* pCommand,
					  DWORD& dwCommandLen)
{
	bool bRet = false;

	std::string strPTZCmd = pPTZCmd;
	std::string sResult;
	if(m_cPTZProtocol.GetFunctionValue(strPTZCmd, iParam1, sResult) == false)
	{
		return false;
	}


	std::vector<BYTE> command_vector;
	command_vector.resize(128);
	int vec_pos = 0;// 改用resize , 所以要加 vector pos

	AssembleABSPTZCMD(iParam2,
		iPanUnit, 
		iTiltUnit, 
		iZoomUnit, 
		bPanCounterClock, 
		sResult,
		command_vector,
		vec_pos);

	for(int i = 0; i<vec_pos; ++i)
	{
		pCommand[i] = command_vector[i];
	}
	dwCommandLen = vec_pos;

	command_vector.clear();

	return bRet = true;
}

//iPanUnit (hardware unit) iTiltUnit (hardware unit) iZoomUnit (hardware unit) => fPanDegree 0 ~ 360 fRiltDegree 0 ~ 90 fZoomRatio 0 ~ 100
void CPTZProtocolHandler::UnitToDegree(	int iPanUnit,
				  int iTiltUnit,
				  int iZoomUnit,
				  float& fPanDegree,
				  float& fTiltDegree,
				  float& fZoomRatio)
{
	InitDeg2PTZobject();
	// ( fMaxPan ~ fMinPan ) mapping to rotate Y-axis (0 ~ 360 ) 
	fPanDegree = m_Deg2PTZobject.GetYAXISRotateDegreeFromPanUnit((float) iPanUnit);
	// ( fMaxTilt ~ fMinTilt ) mapping to rotate X-axis ( 0 ~ 90 ) 
	fTiltDegree = m_Deg2PTZobject.GetXAXISRotateDegreeFromTiltUnit((float) iTiltUnit);
	// ( fMinZoom ~ fMaxZoom ) mapping to get zoom degree (0 ~ 100 ) 
	fZoomRatio = m_Deg2PTZobject.GetZoomPercentFromZoomUnit((float) iZoomUnit);

}
//fPanDegree 0 ~ 360 fRiltDegree 0 ~ 90 fZoomRatio 0 ~ 100 => iPanUnit (hardware unit) iTiltUnit (hardware unit) iZoomUnit (hardware unit)
void CPTZProtocolHandler::DegreeToUnit(	float fPanDegree,
				  float fTiltDegree,
				  float fZoomRatio,
				  int& iPanUnit,
				  int& iTiltUnit,
				  int& iZoomUnit)
{
	InitDeg2PTZobject();
	// rotate Y-axis (0 ~ 360 ) mapping to ( fMaxPan ~ fMinPan )
	iPanUnit = (int)m_Deg2PTZobject.GetPanUnitFromYAXISRotateDegree(fPanDegree);
	// rotate X-axis ( 0 ~ 90 ) mapping to ( fMaxTilt ~ fMinTilt )
	iTiltUnit = (int)m_Deg2PTZobject.GetTiltUnitFromXAXISRotateDegree(fTiltDegree);
	// get zoom degree (0 ~ 100 ) mapping to ( fMinZoom ~ fMaxZoom )
	iZoomUnit = (int)m_Deg2PTZobject.GetZoomUnitFromZoomPercent(fZoomRatio);

}

//To analyse the buffer (came from RS232 callback).
//This function find the header, and parse out the iPanUnit, iTiltUnit, iZoomUnit
bool CPTZProtocolHandler::GetUnitFromBuffer( BYTE* pDataBufferFromRS232CallBack,
					   DWORD dwLengthOfBuffer,
					   int& iPanUnit,
					   int& iTiltUnit,
					   int& iZoomUnit)
{
	bool bRet = false;
	if(pDataBufferFromRS232CallBack == NULL || dwLengthOfBuffer == 0)
	{
		return bRet;
	}



	
	std::string strResult;
	m_cPTZProtocol.GetAttValue("REQUESTPACKAGEHEADERSIZE", strResult);
	m_dwRequestPackageHeaderSize = atoi (strResult.c_str());

	m_cPTZProtocol.GetAttValue("REQUESTPACKAGESIZE", strResult);
	m_dwRequestPackageSize = atoi (strResult.c_str());

	

	//晚點這段可以丟入獨立function
	std::string strPTZCmd;
	int iParam1 = 0; // reserve for latter expansion
	if(m_strPanRequest.empty())
	{
		// get pan header
		strPTZCmd = "GETABSOLUTEPAN";
		if(m_cPTZProtocol.GetFunctionValue(strPTZCmd, iParam1, m_strPanRequest))
		{
			std::string::size_type idx_start;
			std::string::size_type idx;
			std::string::size_type idx2;
			idx_start = 0; 
			idx = idx_start;
			idx2 = idx_start;

			DWORD dwPosInCmd = 0;
			while(idx != std::string::npos)
			{
				std::string temp_str;
				idx2 = m_strPanRequest.find(',',idx+1);
				if(idx2 == std::string::npos)
				{
					if(idx != idx_start)
						++idx;
					temp_str = m_strPanRequest.substr(idx);
				}
				else
				{
					if(idx != idx_start)
						++idx;
					temp_str = m_strPanRequest.substr(idx,idx2-(idx));
				}

				// deal the sub string
				if((temp_str.find("0x") != std::string::npos) && (m_PanReqHeader.size() < m_dwRequestPackageHeaderSize))
				{
					int cmd = 0;
					sscanf(temp_str.c_str(), "%X", &cmd);
					m_PanReqHeader.push_back((BYTE)cmd);
				}
				else if( temp_str == "P_LO" )
				{
					m_mapPanReqRule.insert(std::pair<ByteRef,DWORD>(P_LO,dwPosInCmd));
				}
				else if( temp_str == "P_HI" )
				{
					m_mapPanReqRule.insert(std::pair<ByteRef,DWORD>(P_HI,dwPosInCmd));
				}
				else if( temp_str == "CHEK")
				{
					m_mapPanReqRule.insert(std::pair<ByteRef,DWORD>(CHEK,dwPosInCmd));
				}


				idx = idx2;
				++dwPosInCmd;
			}
			
		}

		// get tilt header
		strPTZCmd = "GETABSOLUTETILT";
		if(m_cPTZProtocol.GetFunctionValue(strPTZCmd, iParam1, m_strTiltRequest))
		{
			std::string::size_type idx_start;
			std::string::size_type idx;
			std::string::size_type idx2;
			idx_start = 0; 
			idx = idx_start;
			idx2 = idx_start;

			DWORD dwPosInCmd = 0;
			while(idx != std::string::npos)
			{
				std::string temp_str;
				idx2 = m_strTiltRequest.find(',',idx+1);
				if(idx2 == std::string::npos)
				{
					if(idx != idx_start)
						++idx;
					temp_str = m_strTiltRequest.substr(idx);
				}
				else 
				{
					if(idx != idx_start)
						++idx;
					temp_str = m_strTiltRequest.substr(idx,idx2-(idx));
				}

				// deal the sub string
				if((temp_str.find("0x") != std::string::npos) && (m_TiltReqHeader.size() < m_dwRequestPackageHeaderSize) )
				{
					int cmd = 0;
					sscanf(temp_str.c_str(), "%X", &cmd);
					m_TiltReqHeader.push_back((BYTE)cmd);
				}
				else if( temp_str == "T_LO" )
				{
					m_mapTiltReqRule.insert(std::pair<ByteRef,DWORD>(T_LO,dwPosInCmd));
				}
				else if( temp_str == "T_HI" )
				{
					m_mapTiltReqRule.insert(std::pair<ByteRef,DWORD>(T_HI,dwPosInCmd));
				}
				else if( temp_str == "CHEK")
				{
					m_mapTiltReqRule.insert(std::pair<ByteRef,DWORD>(CHEK,dwPosInCmd));
				}


				idx = idx2;
				++dwPosInCmd;
			}

		}

		//get zoom header
		strPTZCmd = "GETABSOLUTEZOOM";
		if(m_cPTZProtocol.GetFunctionValue(strPTZCmd, iParam1, m_strZoomRequest))
		{
			std::string::size_type idx_start;
			std::string::size_type idx;
			std::string::size_type idx2;
			idx_start = 0; 
			idx = idx_start;
			idx2 = idx_start;

			DWORD dwPosInCmd = 0;
			while(idx != std::string::npos)
			{
				std::string temp_str;
				idx2 = m_strZoomRequest.find(',',idx+1);
				if(idx2 == std::string::npos)
				{
					if(idx != idx_start)
						++idx;
					temp_str = m_strZoomRequest.substr(idx);
				}
				else
				{
					if(idx != idx_start)
						++idx;
					temp_str = m_strZoomRequest.substr(idx,idx2-(idx));
				}

				// deal the sub string
				if((temp_str.find("0x") != std::string::npos) && (m_ZoomReqHeader.size() < m_dwRequestPackageHeaderSize))
				{
					int cmd = 0;
					sscanf(temp_str.c_str(), "%X", &cmd);
					m_ZoomReqHeader.push_back((BYTE)cmd);
				}
				else if( temp_str == "Z_LO" )
				{
					m_mapZoomReqRule.insert(std::pair<ByteRef,DWORD>(Z_LO,dwPosInCmd));
				}
				else if( temp_str == "Z_HI" )
				{
					m_mapZoomReqRule.insert(std::pair<ByteRef,DWORD>(Z_HI,dwPosInCmd));
				}
				else if( temp_str == "CHEK")
				{
					m_mapZoomReqRule.insert(std::pair<ByteRef,DWORD>(CHEK,dwPosInCmd));
				}


				idx = idx2;
				++dwPosInCmd;
			}
		}

	}


	// 下面開始檢查輸入的 buffer

	int nPos = 0;
	int nSize = dwLengthOfBuffer;

	while( nPos <= nSize - (int)m_dwRequestPackageSize )
	{
		std::vector<BYTE> temp_BYTEvec;
		int i = 0;
		for(i=0; i < (int)m_dwRequestPackageHeaderSize; i++)
		{
			temp_BYTEvec.push_back(pDataBufferFromRS232CallBack[nPos+i]);
		}

		if( (temp_BYTEvec == m_PanReqHeader) )
		{
			std::map<ByteRef,DWORD>::iterator curPos, endPos;
			curPos = m_mapPanReqRule.begin();
			endPos = m_mapPanReqRule.end();
			iPanUnit = 0;
			
			for(; curPos!=endPos; ++curPos)
			{
				//每個跑一次, 去塞 rule 的位置
				//去找 rule 的位置, 塞進答案中
				if(curPos->first == P_LO)
				{
					iPanUnit += pDataBufferFromRS232CallBack[ nPos + curPos->second ];		
				}
				if(curPos->first == P_HI)
				{
					iPanUnit += (pDataBufferFromRS232CallBack[ nPos + curPos->second ] * 0x100);
				}
				if(curPos->first == CHEK)
				{
					// 還不想算 checksum
				}
			}
			
			nPos += m_dwRequestPackageSize;
		}
		else if( temp_BYTEvec == m_TiltReqHeader )
		{
			std::map<ByteRef,DWORD>::iterator curPos, endPos;
			curPos = m_mapTiltReqRule.begin();
			endPos = m_mapTiltReqRule.end();
			iTiltUnit = 0;
			for(; curPos!=endPos; ++curPos)
			{
				//每個跑一次, 去塞 rule 的位置
				//去找 rule 的位置, 塞進答案中
				if(curPos->first == T_LO)
				{
					iTiltUnit += pDataBufferFromRS232CallBack[ nPos + curPos->second ];
				}
				if(curPos->first == T_HI)
				{
					iTiltUnit += (pDataBufferFromRS232CallBack[ nPos + curPos->second ] * 0x100);
				}
				if(curPos->first == CHEK)
				{
					// 還不想算 checksum
				}
			}
			
			nPos += m_dwRequestPackageSize;
		}
		else if( temp_BYTEvec == m_ZoomReqHeader )
		{
			
			std::map<ByteRef,DWORD>::iterator curPos, endPos;
			curPos = m_mapZoomReqRule.begin();
			endPos = m_mapZoomReqRule.end();
			BYTE bZ_HI = 0;
			BYTE bZ_LO = 0;
			iZoomUnit = 0;
			for(; curPos!=endPos; ++curPos)
			{
				//每個跑一次, 去塞 rule 的位置
				//去找 rule 的位置, 塞進答案中
				if(curPos->first == Z_LO)
				{
					bZ_LO = pDataBufferFromRS232CallBack[ nPos + curPos->second ];
				}
				if(curPos->first == Z_HI)
				{
					bZ_HI = pDataBufferFromRS232CallBack[ nPos + curPos->second ];
				}
				if(curPos->first == CHEK)
				{
					// 還不想算 checksum
				}
			}
			//去找 z_hi z_ lo
			std::string strPTZinfo;
			int iZMAX = 0;
			int iZMIN = 0;
			strPTZinfo = "ZMAX";
			m_cPTZProtocol.GetAttValue(strPTZinfo, strResult);
			iZMAX = atoi (strResult.c_str());
			strPTZinfo = "ZMIN";
			m_cPTZProtocol.GetAttValue(strPTZinfo, strResult);
			iZMIN = atoi (strResult.c_str());

			strPTZinfo = "ABSZOOM";
			for(i = iZMIN; i < iZMAX; ++i)
			{
				int iZ_LO;
				int iZ_HI;
				m_cPTZProtocol.GetFunctionValue(strPTZinfo, i, strResult);
				sscanf(strResult.c_str(), "%X,%X", &iZ_LO, &iZ_HI);
				if( (bZ_LO == (BYTE)(iZ_LO & 0x000000FF)) && (bZ_HI == (BYTE)(iZ_HI & 0x000000FF)))
				{
					iZoomUnit = i;
					break;
				}

			}


			nPos += m_dwRequestPackageSize;
		}
		//it's not header
		else
		{
			nPos++;

		}

	}//end of while


	

	return bRet = true;
}

// Get request command from PTZ file. Sending request command to device will receive PTZ position from rs232 callback
bool CPTZProtocolHandler::GetRequestAbsPTZCommand(int iParam1,BYTE* pCommand,DWORD& dwCommandLen)
{
	bool bRet = false;
	std::string strPTZCmd = "REQUESTABSOLUTEPTZ";
	std::string sResult;
	if(m_cPTZProtocol.GetFunctionValue(strPTZCmd, iParam1, sResult) == false)
	{
		return false;
	}

	std::vector<BYTE> command_vector;
	command_vector.resize(128);
	int vec_pos = 0;// 改用resize , 所以要加 vector pos
	AssembleABSPTZCMD(0,
		0, 
		0, 
		0, 
		false, 
		sResult,
		command_vector,
		vec_pos);
	for(int i = 0; i<vec_pos; ++i)
	{
		pCommand[i] = command_vector[i];
	}
	dwCommandLen = vec_pos;

	command_vector.clear();


	bRet = true;
	return bRet;
}


bool CPTZProtocolHandler::GetCommand(char* pPTZCmd, int nAddID, int iParam1, int iParam2, BYTE* pCommand, DWORD& dwCommandLen)
{
	bool bRet = false;
	char szPTZCommand[128] = {0};

	if(strlen(pPTZCmd) > 128)
		return false;

	strcpy(szPTZCommand, pPTZCmd);

	if( m_nCommandType == 0 )
	{
		std::string sResult;
		DWORD dwResultCount;
		if(m_cPTZProtocol.GetFunctionValue(szPTZCommand, iParam1, sResult))
		{
			if(!sResult.empty())
			{
				BYTE bCommand[128] = {0};			// bCommand[0] is not using.
				DWORD dwLen = 0;

				CovertStrCmdToByte(sResult, bCommand, dwLen);

				// Calculate checksum
				std::string::size_type loc = m_sCheckSum.find( "$D", 0 );

				if(std::string::npos == loc)
				{
					BYTE bTemp[128] = {0};
					memcpy(bTemp, bCommand, 128);
					if(!CalculateCheckSum(bCommand, nAddID, bTemp))
						return false;
					memset(bCommand, 0x00, 128);
					// Value start from array pos 1 not 0 see CovertStrCmdToByte
					memcpy(bCommand+1, bTemp+1, dwLen);
				}

				// Pass out, copy it to pos 0
				memcpy(pCommand, bCommand+1, dwLen);
				dwCommandLen = dwLen;
				bRet = true;
			}	
		}
	}

	else if( m_nCommandType == 1 )
	{
		std::string sResult;
		DWORD dwResultCount;
		if(m_cPTZProtocol.GetFunctionValue(szPTZCommand, iParam1, sResult))
		{
			if(!sResult.empty())
			{
				BYTE bCommand[128] = {0};			// bCommand[0] is not using.
				DWORD dwLen = 0;

				CovertStrCmdToByte(sResult, bCommand, dwLen);

				memcpy(pCommand, bCommand+1, dwLen);
				dwCommandLen = dwLen;
				bRet = true;
			}	
		}
		/*
		std::string sResult;
		if(m_cPTZProtocol.GetFunctionValue(szPTZCommand, iParam1, sResult))
		{
			if(!sResult.empty())
			{
			DWORD dwResultCount = sResult.length();

			BYTE bCommand[128] = {0};
			memcpy( bCommand, sResult.c_str(), dwResultCount );
			bCommand[ dwResultCount ] = 0;

			int nCnt = 0;
			DWORD dwStrDataLen = dwResultCount;
			dwResultCount = 0;
			while( dwStrDataLen > nCnt )
			{
			pCommand[dwResultCount] = (HexChar( bCommand[nCnt] ) * 16) + HexChar( bCommand[nCnt+1] );
			dwResultCount++;
			nCnt += 2;
			}
			dwCommandLen = dwResultCount;
			}
			}
			*/
	}

	else if( m_nCommandType == 2 )
	{
		std::string sResult;
		if(m_cPTZProtocol.GetFunctionValue(szPTZCommand, iParam1, sResult))
		{
			if(!sResult.empty())
			{
				DWORD dwResultCount = sResult.length();
				memcpy( pCommand, sResult.c_str(), dwResultCount );
				pCommand[ dwResultCount ] = 0;
				dwCommandLen = dwResultCount;
			}
		}
	}
	
	return bRet;
}

bool CPTZProtocolHandler::GetVenderProtocolStr(PTZ_VENDER_PROTOCOL PTZVenderProtocol, char* szPTZVenderProtocol)
{
	switch(PTZVenderProtocol)
	{
	case CAM5130_KAMPRO:
		strcpy(szPTZVenderProtocol, "CAM-5130_Kampro\0");
		break;
	case CAM5140_KAMPRO:
		sprintf(szPTZVenderProtocol, "CAM-5140_Kampro\0");
		break;
	case CAM5150_KAMPRO:
		sprintf(szPTZVenderProtocol, "CAM-5150_Kampro\0");
		break;
	case CAM5500_KAMPRO:
		sprintf(szPTZVenderProtocol, "CAM-5500_Kampro\0");
		break;
	case CAM6100_PELCOD:
		sprintf(szPTZVenderProtocol, "CAM-6100_Pelco-D\0");
		break;
	case CAM6100_PELCOP:
		sprintf(szPTZVenderProtocol, "CAM-6100_Pelco-P\0");
		break;
	case CAM6100_VCL:
		sprintf(szPTZVenderProtocol, "CAM-6100_VCL\0");
		break;
	case CAM6200_EYEVIEWV1:
		sprintf(szPTZVenderProtocol, "CAM-6200_EyeView-V1\0");
		break;
	case CAM6200_EYEVIEWV2:
		sprintf(szPTZVenderProtocol, "CAM-6200_EyeView-V2\0");
		break;
	case CAM6200_PELCOD:
		sprintf(szPTZVenderProtocol, "CAM-6200_Pelco-D\0");
		break;
	case CAM6200_PELCOP:
		sprintf(szPTZVenderProtocol, "CAM-6200_Pelco-P\0");
		break;
	case CAM6500_DYNACOLOR:
		sprintf(szPTZVenderProtocol, "CAM-6500_DynaColor\0");
		break;
	case CAM6500_PELCOD:
		sprintf(szPTZVenderProtocol, "CAM-6500_Pelco-D\0");
		break;
	case CAM6500_PELCOP:
		sprintf(szPTZVenderProtocol, "CAM-6500_Pelco-P\0");
		break;
	case CAM6600_DYNACOLOR:
		sprintf(szPTZVenderProtocol, "CAM-6600_DynaColor\0");
		break;
	case CAM6600_PELCOD:
		sprintf(szPTZVenderProtocol, "CAM-6600_Pelco-D\0");
		break;
	case CAM6600_PELCOP:
		sprintf(szPTZVenderProtocol, "CAM-6600_Pelco-P\0");
		break;
	case DYNACOLOR_DYNACOLOR:
		sprintf(szPTZVenderProtocol, "DynaColor_DynaColor\0");
		break;
	case DYNACOLOR_PELCOD:
		sprintf(szPTZVenderProtocol, "DynaColor_Pelco-D\0");
		break;
	case DYNACOLOR_PELCOP:
		sprintf(szPTZVenderProtocol, "DynaColor_Pelco-P\0");
		break;
	case EYEVIEW_EYEVIEWV1:
		sprintf(szPTZVenderProtocol, "EyeView_EyeView-V1\0");
		break;
	case EYEVIEW_EYEVIEWV2:
		sprintf(szPTZVenderProtocol, "EyeView_EyeView-V2\0");
		break;
	case EYEVIEW_PELCOD:
		sprintf(szPTZVenderProtocol, "EyeView_Pelco-D\0");
		break;
	case EYEVIEW_PELCOP:
		sprintf(szPTZVenderProtocol, "EyeView_Pelco-P\0");
		break;
	case GE_KALATEL:
		sprintf(szPTZVenderProtocol, "GE_Kalatel\0");
		break;
	case HARRISONTECH_VICSA:
		sprintf(szPTZVenderProtocol, "HarrisonTech_VISCA\0");
		break;
	case KAMPRO_KAMPRO:
		sprintf(szPTZVenderProtocol, "Kampro_Kampro\0");
		break;
	case KAMPRO_V2:
		sprintf(szPTZVenderProtocol, "Kampro_V2\0");
		break;
	case LILIN_LILIN:
		sprintf(szPTZVenderProtocol, "LiLin_Lilin\0");
		break;
	case MESSOA_PELCOD:
		sprintf(szPTZVenderProtocol, "Messoa_Pelco-D\0");
		break;
	case MESSOA_PELCOP:
		sprintf(szPTZVenderProtocol, "Messoa_Pelco-P\0");
		break;
	case MESSOA_VCL:
		sprintf(szPTZVenderProtocol, "Messoa_VCL\0");
		break;
	case NICECAM_NICECAM:
		sprintf(szPTZVenderProtocol, "Nicecam_Nicecam\0");
		break;
	case PANASONIC_PANASONIC:
		sprintf(szPTZVenderProtocol, "Panasonic_Panasonic\0");
		break;
	case PELCO_PELCOD:
		sprintf(szPTZVenderProtocol, "Pelco_Pelco-D\0");
		break;
	case PELCO_PELCOP:
		sprintf(szPTZVenderProtocol, "Pelco_Pelco-P\0");
		break;
	case PELCO_PELCOPV1:
		sprintf(szPTZVenderProtocol, "Pelco_Pelco-P-V1\0");
		break;
	case PELCO_PELCOPV2:
		sprintf(szPTZVenderProtocol, "Pelco_Pelco-P-V2\0");
		break;
	case PIXIM_PIXIM:
		sprintf(szPTZVenderProtocol, "Pixim_Pixim\0");
		break;
	case SAMSUNG_SAMSUNG:
		sprintf(szPTZVenderProtocol, "Samsung_Samsung\0");
		break;
	case TOA_TOA:
		sprintf(szPTZVenderProtocol, "TOA_TOA\0");
		break;
	case VICON_VICON:
		sprintf(szPTZVenderProtocol, "Vicon_Vicon\0");
		break;
	case VIDEOTREC_VIDEOTREC:
		sprintf(szPTZVenderProtocol, "Videotrec_Videotrec\0");
		break;
	case VCL_VCL:
		sprintf(szPTZVenderProtocol, "VCL_VCL\0");
		break;
	default:
		return false;
	}
	return true;
}

bool CPTZProtocolHandler::GetVenderStr(PTZ_VENDER PTZVender, char* szVender)
{
	switch(PTZVender)
	{
	case CAM5130_V:
		strcpy(szVender, "CAM-5130\0");
		break;
	case CAM5140_V:
		sprintf(szVender, "CAM-5140\0");
		break;
	case CAM5150_V:
		sprintf(szVender, "CAM-5150\0");
		break;
	case CAM5500_V:
		sprintf(szVender, "CAM-5500\0");
		break;
	case CAM6100_V:
		sprintf(szVender, "CAM-6100\0");
		break;
	case CAM6200_V:
		sprintf(szVender, "CAM-6200\0");
		break;
	case CAM6500_V:
		sprintf(szVender, "CAM-6500\0");
		break;
	case CAM6600_V:
		sprintf(szVender, "CAM-6600\0");
		break;
	case DYNACOLOR_V:
		sprintf(szVender, "DynaColor\0");
		break;
	case EYEVIEW_V:
		sprintf(szVender, "EyeView\0");
		break;
	case GE_V:
		sprintf(szVender, "GE\0");
		break;
	case HARRISONTECH_V:
		sprintf(szVender, "HarrisonTech\0");
		break;
	case KAMPRO_V:
		sprintf(szVender, "Kampro\0");
		break;
	case LILIN_V:
		sprintf(szVender, "LiLin\0");
		break;
	case MESSOA_V:
		sprintf(szVender, "Messoa\0");
		break;
	case NICECAM_V:
		sprintf(szVender, "Nicecam\0");
		break;
	case PANASONIC_V:
		sprintf(szVender, "Panasonic\0");
		break;
	case PELCO_V:
		sprintf(szVender, "Pelco\0");
		break;
	case PIXIM_V:
		sprintf(szVender, "Pixim\0");
		break;
	case SAMSUNG_V:
		sprintf(szVender, "Samsung\0");
		break;
	case TOA_V:
		sprintf(szVender, "TOA\0");
		break;
	case VICON_V:
		sprintf(szVender, "Vicon\0");
		break;
	case VIDEOTREC_V:
		sprintf(szVender, "Videotrec\0");
		break;
	case VCL_V:
		sprintf(szVender, "VCL\0");
		break;
	default:
		return false;
	}
	return true;
}

bool CPTZProtocolHandler::GetProtocolStr(PTZ_PROTOCOL PTZProtocol, char* szProtocol)
{
	switch(PTZProtocol)
	{
	case KAMPRO_P:
		strcpy(szProtocol, "Kampro\0");
		break;
	case PELCOD_P:
		sprintf(szProtocol, "Pelco-D\0");
		break;
	case PELCOP_P:
		sprintf(szProtocol, "Pelco-P\0");
		break;
	case VCL_P:
		sprintf(szProtocol, "VCL\0");
		break;
	case EYEVIEWV1_P:
		sprintf(szProtocol, "EyeView-V1\0");
		break;
	case EYEVIEWV2_P:
		sprintf(szProtocol, "EyeView-V2\0");
		break;
	case DYNACOLOR_P:
		sprintf(szProtocol, "DynaColor\0");
		break;
	case KALATEL_P:
		sprintf(szProtocol, "Kalatel\0");
		break;
	case VICSA_P:
		sprintf(szProtocol, "VISCA\0");
		break;;
	case V2_P:
		sprintf(szProtocol, "V2\0");
		break;
	case LILIN_P:
		sprintf(szProtocol, "LiLin\0");
		break;
	case NICECAM_P:
		sprintf(szProtocol, "Nicecam\0");
		break;
	case PANASONIC_P:
		sprintf(szProtocol, "Panasonic\0");
		break;
	case PELCOPV1_P:
		sprintf(szProtocol, "Pelco-P-V1\0");
		break;
	case PELCOPV2_P:
		sprintf(szProtocol, "Pelco-P-V2\0");
		break;
	case PIXIM_P:
		sprintf(szProtocol, "Pixim\0");
		break;
	case SAMSUNG_P:
		sprintf(szProtocol, "SamSung\0");
		break;
	case TOA_P:
		sprintf(szProtocol, "TOA\0");
		break;
	case VICON_P:
		sprintf(szProtocol, "Vicon\0");
		break;
	case VIDEOTREC_P:
		sprintf(szProtocol, "Videotrec\0");
		break;
	default:
		return false;
	}
	return true;
}

bool CPTZProtocolHandler::PTZEnumerateProtocol(char* pVender, char* pProtocol, DWORD& dwLen)
{
	if(0 == dwLen)
		return false;

	bool bRet = true;
	std::string sCompletePcol;
	std::string sTempPcol;
	bool bRun = true;
	char szVender[128] = {0};
	if(NULL != pVender)
	{
		strcpy(szVender, pVender);
	}
	else
	{
		return false;
	}

	int i = 0;
	int nCount = 0;
	while (bRun)
	{
		if(_VENDER_PROTOCOL[i][0] != '\0')		// check for end of const
		{
			if(strncmp(_VENDER_PROTOCOL[i][0], szVender, strlen(szVender)) == 0)
			{
				for(int j = 1; j <= MAX_PROTOCOL_COUNT; j++)
				{
					if(_VENDER_PROTOCOL[i][j][0] != '\0')
					{
						sTempPcol.append(_VENDER_PROTOCOL[i][j]);
						sTempPcol.append(",");
						nCount++;
					}
					else
					{
						bRun = false;
						break;
					}
				}
				bRun = false;
			}
			else
			{
				i++;
			}
		}
		else
		{
			bRun = false;
		}
	}

	char szTemp[16] = {0};
	sprintf(szTemp, "%d,", nCount);
	sCompletePcol.append(szTemp);
	sCompletePcol.append(sTempPcol.c_str());
	if(sCompletePcol.length() > (dwLen-1))
	{
		strncpy(pProtocol, sCompletePcol.c_str(), (dwLen-1));
		pProtocol[dwLen-1] = '\0';
	}
	else
	{
		strncpy(pProtocol, sCompletePcol.c_str(), sCompletePcol.length());
		dwLen = sCompletePcol.length();
		pProtocol[dwLen-1] = '\0';
	}
	return bRet;
}

bool CPTZProtocolHandler::PTZEnumerateProtocol(PTZ_VENDER Vender, char* pProtocol, DWORD& dwLen)
{
	if(0 == dwLen)
		return false;

	bool bRet = true;
	std::string sCompletePcol;
	std::string sTempPcol;
	bool bRun = true;
	char szVender[128] = {0};
	if(!GetVenderStr(Vender, szVender))
		return false;
	int i = 0;
	int nCount = 0;
	while (bRun)
	{
		if(_VENDER_PROTOCOL[i][0] != '\0')		// check for end of const
		{
			if(strncmp(_VENDER_PROTOCOL[i][0], szVender, strlen(szVender)) == 0)
			{
				for(int j = 1; j <= MAX_PROTOCOL_COUNT; j++)
				{
					if(_VENDER_PROTOCOL[i][j][0] != '\0')
					{
						sTempPcol.append(_VENDER_PROTOCOL[i][j]);
						sTempPcol.append(",");
						nCount++;
					}
					else
					{
						bRun = false;
						break;
					}
				}
				bRun = false;
			}
			else
			{
				i++;
			}
		}
		else
		{
			bRun = false;
		}
	}

	char szTemp[16] = {0};
	sprintf(szTemp, "%d,", nCount);
	sCompletePcol.append(szTemp);
	sCompletePcol.append(sTempPcol.c_str());
	if(sCompletePcol.length() > (dwLen-1))
	{
		strncpy(pProtocol, sCompletePcol.c_str(), (dwLen-1));
		pProtocol[dwLen-1] = '\0';
	}
	else
	{
		strncpy(pProtocol, sCompletePcol.c_str(), sCompletePcol.length());
		dwLen = sCompletePcol.length();
		pProtocol[dwLen-1] = '\0';
	}
	return bRet;
}

bool CPTZProtocolHandler::PTZEnumerateVender(char* pVender, DWORD& dwLen)
{
	if(0 == dwLen)
		return false;
	
	bool bRet = true;
	std::string sCompleteVender;
	std::string sTempVender;
	bool bRun = true;
	char szVender[128] = {0};
	int i = 0;
	int nCount = 0;
	while (bRun)
	{
		if(_VENDER[i][0] != '\0')		// check for end of const
		{
			sTempVender.append(_VENDER[i]);
			sTempVender.append(",");
			i++;
			nCount++;
		}
		else
		{
			bRun = false;
		}
	}

	char szTemp[16] = {0};
	sprintf(szTemp, "%d,", nCount);
	sCompleteVender.append(szTemp);
	sCompleteVender.append(sTempVender.c_str());
	if(sCompleteVender.length() > (dwLen-1))
	{
		strncpy(pVender, sCompleteVender.c_str(), (dwLen-1));
		pVender[dwLen-1] = '\0';
	}
	else
	{
		strncpy(pVender, sCompleteVender.c_str(), sCompleteVender.length());
		dwLen = sCompleteVender.length();
		pVender[dwLen-1] = '\0';
	}
	return bRet;
}

bool CPTZProtocolHandler::PTZEnumerateFunction(char* pFunction, DWORD& dwLen)
{
	if(0 == dwLen)
		return false;
	
	int nCount = 1;
	bool bRet = true;
	std::string sResult;
	m_cPTZProtocol.GetPanel(sResult);
	if(bRet)
	{
		if(sResult.length() > 0)
		{
			for(int i = 0; i < sResult.length(); i++)
			{
				if(',' == sResult[i])
				{
					if(i != (sResult.length()-1))
					{
						nCount++;
					}
				}
			}
		}
		else
		{
			nCount = 0;
		}
	}
	std::string sCompleteFunction;
	char szCount[16] = {0};
	sprintf(szCount, "%d,", nCount);
	sCompleteFunction.append(szCount);
	sCompleteFunction.append(sResult.c_str());
	if(sCompleteFunction.length() > (dwLen-1))
	{
		strncpy(pFunction, sCompleteFunction.c_str(), (dwLen-1));
		pFunction[dwLen-1] = '\0';
	}
	else
	{
		strncpy(pFunction, sCompleteFunction.c_str(), sCompleteFunction.length());
		dwLen = sCompleteFunction.length();
		pFunction[dwLen-1] = '\0';
	}
	return bRet;
}
int CPTZProtocolHandler::HexChar( char c )
{
	int nT = 0;
	switch( c )
	{
	case '0':
		nT = 0;
		break;
	case '1':
		nT = 1;
		break;
	case '2':
		nT = 2;
		break;
	case '3':
		nT = 3;
		break;
	case '4':
		nT = 4;
		break;
	case '5':
		nT = 5;
		break;
	case '6':
		nT = 6;
		break;
	case '7':
		nT = 7;
		break;
	case '8':
		nT = 8;
		break;
	case '9':
		nT = 9;
		break;
	case 'a':
	case 'A':
		nT = 10;
		break;
	case 'b':
	case 'B':
		nT = 11;
		break;
	case 'c':
	case 'C':
		nT = 12;
		break;
	case 'd':
	case 'D':
		nT = 13;
		break;
	case 'e':
	case 'E':
		nT = 14;
		break;
	case 'f':
	case 'F':
		nT = 15;
		break;
	default :
		break;
	}
	return nT;
}
