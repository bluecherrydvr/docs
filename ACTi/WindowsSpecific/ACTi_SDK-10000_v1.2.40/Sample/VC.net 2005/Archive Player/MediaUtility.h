#pragma once

//extern "C" __declspec(dllexport) bool ConvertRaw2Jpg(CString csFileName);
//extern "C" __declspec(dllexport) bool ConvertRaw2BMP(CString csFileName);
//extern "C" __declspec(dllexport) bool DisplayMP4(CString csFileName, HDC hDC, RECT &rcRect);
//extern "C" __declspec(dllexport) void DisplayMP4Clear();

bool DisplayMP4(CString csFileName, HDC hDC, RECT &rcRect)
{
	return true;
}
void DisplayMP4Clear(){}

bool ConvertRaw2Image(CString csFileName) 
{
	return true ;
}