#ifndef _AUDIOCAPTURE_MUX_H_
#define _AUDIOCAPTURE_MUX_H_

#include <afx.h>
//#include <Vfw.h>
#include <streams.h>
#include <mmreg.h>	

#define RELEASE(ptr)\
	if(ptr)\
	{\
		ptr->Release();\
		ptr =  NULL;\
	}
#define Leave goto Finally

#define FailReturnHR(hr) if(FAILED(hr)) return hr;
#define FailLeaveHR(hr) if(FAILED(hr)) Leave;

HRESULT  HostCallback(char cType,LONG lParam1,LONG lParam2);

class CAudioCaptureMux
{
protected:
	IGraphBuilder *m_pGraph;
	IMediaControl *m_pMediaCtr;
	IBaseFilter *m_pTerminatorFilter;
	IBaseFilter *m_pMP3Filter;
	IBaseFilter *m_pCaptureFilter;
public:
	HRESULT BuildGraph(int nBitrate, int inputType);
	HRESULT RemoveGraph();
	HRESULT Run();
	HRESULT Stop();
protected:
	HRESULT BuildGraphFromFile(CString strFileName,int nType, int nBitrate, int inputType);
	HRESULT CreateCaptureGraph(IGraphBuilder* pGraph,CString strSource,int nType);
	HRESULT ReplaceFilter(IGraphBuilder* pGraph, IBaseFilter* &pFilter, const CLSID* pCLSID,BSTR szNewName);
	HRESULT InsertFilter(IGraphBuilder* pGraph,IBaseFilter* pAfterFilter, IBaseFilter* pFilter,BSTR szNewName,AM_MEDIA_TYPE* pConnectType);
	HRESULT AddFilter(IGraphBuilder* pGraph,const CLSID clsid, IPin *pPrevOutPin, IBaseFilter **ppFilter,BSTR szName);
	HRESULT HookSampleTerminator(IGraphBuilder* pGraph,IBaseFilter* pFilter,int nID/*,StreamInfo* pStreamInfo*/);
	HRESULT FindPinByEnum(IBaseFilter *pFilter, PIN_DIRECTION dr, IPin **ppPin);
	HRESULT RemoveAllFilter(IGraphBuilder* pGraph);
	HRESULT FindDeconectPin(IBaseFilter *pFilter, IPin **ppPin,PIN_DIRECTION dir);
	HRESULT JoinFilter(IGraphBuilder* pGraph,IBaseFilter *pFilter, IPin *pPrevOutPin, BSTR szName);
	HRESULT EnablePinByName(IBaseFilter *pFilter, CString pinName);
};

#endif

