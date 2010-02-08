// IWis.h: interface for the IWis class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <stdio.h>
#include <stdarg.h>

#ifndef _HRESULT_DEFINED
typedef  long HRESULT;
#endif

#ifndef NULL
#define NULL 0
#endif

#define WIS_E_NOTIMPL   0x80004001L
#define WIS_S_OK		0x0
#define WIS_E_NOENOUGHBUFFER 0x70004001L
#define WIS_S_ERROR		0x80004002L

#define MAXSTRING 1000

//{{Windows
#include <windows.h>
// wrapper for whatever critical section we have
class CWisCritSec 
{
    CWisCritSec(const CWisCritSec &refCritSec);
    CWisCritSec &operator=(const CWisCritSec &refCritSec);

    CRITICAL_SECTION m_CritSec;

public:
    CWisCritSec() 
	{
        InitializeCriticalSection(&m_CritSec);
    };

    ~CWisCritSec() 
	{
        DeleteCriticalSection(&m_CritSec);
    };

    void Lock() 
	{
        EnterCriticalSection(&m_CritSec);
    };

    void Unlock() 
	{
        LeaveCriticalSection(&m_CritSec);
    };
};
//Windows}}
class CWisAutoLock 
{

    // make copy constructor and assignment operator inaccessible

    CWisAutoLock(const CWisAutoLock &refAutoLock);
    CWisAutoLock &operator=(const CWisAutoLock &refAutoLock);

protected:
    CWisCritSec * m_pLock;

public:
    CWisAutoLock(CWisCritSec * plock)
    {
        m_pLock = plock;
        m_pLock->Lock();
    };

    ~CWisAutoLock() 
	{
        m_pLock->Unlock();
    };
};

class IWis  
{
public:
	virtual unsigned long AddRef()=0;
	virtual unsigned long Release()=0;
	virtual void DbgOut(const char* szInfo){}
	int printf (const char *format,...)
	{
#if 1
		va_list arglist;

        va_start(arglist, format);
		char szBuffer[MAXSTRING];
		vsprintf(szBuffer,format,arglist);

		DbgOut(szBuffer);
#endif 
        return 0;
	}
};
class IWisGraph;
class IWisModule;
#define USING_WIS_CLASS(classname) \
extern IWisModule* classname##_CreateObject(IWisGraph* pGraph);\

#define CREATEWISCLASS(classname) classname##_CreateObject(NULL)
#define CreateWisModule(classname,pGraph) (IWisModule *)classname##_CreateObject(pGraph)


#define IMPLEMENT_WIS_CLASS(classname) \
	IWisModule* classname##_CreateObject(IWisGraph* pGraph) \
{\
	if( pGraph == NULL)\
		return new classname;\
	else\
	{	IWisModule* pModule =new classname;\
		pGraph->Add(pModule);\
		return pModule;\
	}\
}



enum MediaType
{
   MT_None,
   MT_Audio,
   MT_Video,
   MT_Stream
};

enum ModuleStatus
{
    MS_INIT,
	MS_OPEN,
	MS_RUN,
    MS_STOP,
    MS_PAUSE,
	MS_CLOSE,
    MS_ENDOFSTREAM,
	MS_UNINIT
};

class IWisMediaFormat:public IWis
{
public:
  IWisMediaFormat()
  {
	m_ulRef =1;
	FormatData =NULL;
	cbSize =0;
	Type = MT_None;
  }
  ~IWisMediaFormat()
  {
	if( FormatData )
		delete FormatData;
  }
  unsigned long AddRef()
  {
	m_ulRef++;
	return m_ulRef;
  }
  unsigned long Release()
  {
	m_ulRef--;
	if( m_ulRef == 0)
	{
		delete this;
	}
	return m_ulRef;
  }
  MediaType Type;
  unsigned short  cbSize; 
  unsigned char* FormatData;
protected:
  unsigned long m_ulRef;
};

class IWisSample:public IWis
{
public:
  IWisSample()
  {
	m_ulRef =1;
	Data =NULL;
	Ext = NULL;

	Len =0;
	StartTime =0;
	EndTime =0;
	StartMediaTime =0;
	EndMediaTime =0;
	m_bAutoRelease = true;
	m_bAutoReleaseData = false;
	SampleType =0;
  }
  unsigned long AddRef()
  {
	m_ulRef++;
	return m_ulRef;
  }
  unsigned long Release()
  {
	m_ulRef--;
	if( m_ulRef == 0)
	{
		if( m_bAutoReleaseData )
		{
			if( Data )
				delete Data;
			//if( Ext )
			//	delete Ext;
		}
		if( m_bAutoRelease )
			delete this;
	}
	return m_ulRef;
  }
   virtual HRESULT SetActualSize(unsigned long size)
   {
		Len = size;
		return WIS_S_OK;
   }
   unsigned char* Data;
   unsigned long Len;
   double StartTime;
   double EndTime;
   double StartMediaTime;
   double EndMediaTime;
   unsigned char SampleType;
   void* Ext;
   bool m_bAutoRelease;
   bool m_bAutoReleaseData;
protected:
  unsigned long m_ulRef;
};

//The callback function implements should release the sample 
typedef HRESULT (*RECEIVESAMPLECALLBACK)(IWisSample* pSample,void* pContext);
typedef HRESULT (*ALLOCSAMPLECALLBACK)(IWisSample** ppSample,long lSize,void* pContext);
 
class IHost
{
public:
	virtual void DbgOut(const char* szInfo) =0;	
};

class IWisModule;
class IWisPin:public IWis
{
public:
	IWisPin(IWisModule* pModule)
	{
		m_pConnectedPin = NULL;

		m_ulRef =1;
		m_pModule = pModule;
		m_pHost = NULL;
		m_bHaveFormat = false;

		m_pReceiveSampleCB = NULL; 
		m_pReceiveCBContext = NULL;
		m_pAllocSampleCB = NULL;
		m_pAllocCBContext = NULL;
	}
	virtual ~IWisPin()
	{
		if( m_pConnectedPin )
			m_pConnectedPin->Release();
	}
	unsigned long AddRef()
	{
		m_ulRef++;
		return m_ulRef;
	}
	unsigned long Release()
	{
		m_ulRef--;
		if( m_ulRef == 0)
		{
			Disconnect();
			delete this;
		}
		return m_ulRef;
	}
	void SetHost(IHost* pHost){m_pHost = pHost;}
	bool IsConnected(){return (m_pConnectedPin!= NULL) || m_bHaveFormat;}
    virtual HRESULT SetFormat(IWisMediaFormat* pFormat){m_bHaveFormat = true;return WIS_S_OK;}
	virtual unsigned long GetFormatCount(){return 0;}
	virtual IWisMediaFormat* GetFormat(unsigned long lIndex){return NULL;}
	void DbgOut(const char* szInfo){if(m_pHost) m_pHost->DbgOut(szInfo); }

	virtual HRESULT Connect(IWisPin* pPin,IWisMediaFormat* pFormat = NULL)
	{
		if( m_pConnectedPin )
		{
			m_pConnectedPin->Disconnect();
		}
		m_pConnectedPin = pPin;
		m_pConnectedPin->AddRef();
		m_pConnectedPin->m_pConnectedPin = this;
		m_pConnectedPin->m_pConnectedPin->AddRef();
		if( pFormat)
			m_pConnectedPin->SetFormat(pFormat);
		else
		{
			if( GetFormatCount() >0)
			{
				pFormat = GetFormat(0);
				m_pConnectedPin->SetFormat(pFormat);
				pFormat->Release();
			}
		}
		return WIS_S_OK;
	}
    virtual HRESULT Disconnect()
	{
		if( m_pConnectedPin)
		{
			if( m_pConnectedPin->m_pConnectedPin &&  m_pConnectedPin->m_pConnectedPin == this)
			{
				m_pConnectedPin->m_pConnectedPin->Release();
				m_pConnectedPin->m_pConnectedPin = NULL;
			}
			m_pConnectedPin->Release();
			m_pConnectedPin = NULL;
		}
		else
		{
			m_bHaveFormat = false;
		}
		return WIS_S_OK;
	}
    virtual bool CanAlloc(){return (m_pAllocSampleCB != NULL);}
    virtual HRESULT Alloc(IWisSample** ppSample,long lSize)
	{
		if( m_pAllocSampleCB )
			return (*m_pAllocSampleCB)(ppSample,lSize,m_pAllocCBContext);
		else
			return WIS_E_NOTIMPL;
	}

    virtual HRESULT Receive(IWisSample* pSample)
	{
		if( m_pReceiveSampleCB )
			return (*m_pReceiveSampleCB)(pSample,m_pReceiveCBContext);
		else
			return WIS_E_NOTIMPL;
	}
    virtual HRESULT Send(IWisSample* pSample)
	{
		if( m_pConnectedPin )
			return m_pConnectedPin->Receive(pSample);
		else
		{
			pSample->Release();
			return WIS_S_OK;
		}
	} 
	void SetCallback(RECEIVESAMPLECALLBACK cb1,void* pContext1, ALLOCSAMPLECALLBACK cb2,void* pContext2)
	{
		m_pConnectedPin = new IWisPin(m_pModule);
		m_pConnectedPin->m_pReceiveSampleCB = cb1;
		m_pConnectedPin->m_pReceiveCBContext = pContext1;
		m_pConnectedPin->m_pAllocSampleCB = cb2;
		m_pConnectedPin->m_pAllocCBContext = pContext2;
	}
	IWisPin* m_pConnectedPin;
protected:
  unsigned long m_ulRef;
  IWisModule* m_pModule;
  bool m_bHaveFormat;
  IHost* m_pHost;
  RECEIVESAMPLECALLBACK m_pReceiveSampleCB;
  void* m_pReceiveCBContext;
  ALLOCSAMPLECALLBACK m_pAllocSampleCB;
  void* m_pAllocCBContext;
};
class IWisControl
{
public:
	IWisControl()
	{
		m_ulRef =0;
	}
	virtual unsigned long AddRef()
	{
		m_ulRef++;
		return m_ulRef;
	}
	virtual unsigned long Release()
	{
		m_ulRef--;
		if( m_ulRef == 0)
		{
			delete this;
		}
		return m_ulRef;
	}
    virtual HRESULT Notify(long EventCode,void* EventParam1,void* EventParam2,void* EventParam3)=0;
private:
  unsigned long m_ulRef;
};


class IWisModule:public IWis
{
public:
	IWisModule()
	{
		m_InputPinCount =0;
		m_aInputPin = NULL;
		m_OutputPinCount = 0;
		m_aOuputPin = NULL;
		m_pControl = NULL;

		m_status = MS_INIT;
		m_ulRef =1;
		m_pHost =NULL;
	}
	virtual ~IWisModule()
	{
		while(m_InputPinCount--)
		{
			if( m_aInputPin[m_InputPinCount])
				m_aInputPin[m_InputPinCount]->Release();
		}
		if( m_aInputPin )
			delete m_aInputPin;
		while(m_OutputPinCount--)
		{
			if( m_aOuputPin[m_OutputPinCount])
				m_aOuputPin[m_OutputPinCount]->Release();
		}
		if( m_aOuputPin )
			delete m_aOuputPin;
		if( m_pControl )
			m_pControl->Release();
		
	}
	unsigned long AddRef()
	{
		m_ulRef++;
		return m_ulRef;
	}
	unsigned long Release()
	{
		m_ulRef--;
		if( m_ulRef == 0)
		{
			if( m_status!= MS_UNINIT)
				Uninit();
			delete this;
		}
		return m_ulRef;
	}
	void SetHost(IHost* pHost)
	{
		long i;
		m_pHost = pHost;
		for( i=0;i<m_InputPinCount;i++)
		{
			m_aInputPin[i]->SetHost(m_pHost);
		}
		for( i=0;i<m_OutputPinCount;i++)
		{
			m_aOuputPin[i]->SetHost(m_pHost);
		}
	}
	void DbgOut(const char* szInfo){if(m_pHost) m_pHost->DbgOut(szInfo); }

    long m_InputPinCount;
    IWisPin** m_aInputPin;
    long m_OutputPinCount;
    IWisPin** m_aOuputPin;
	IWisControl* m_pControl;
	ModuleStatus m_status;

    virtual HRESULT Init(void* InitParameter = NULL)
	{m_status = MS_INIT;return WIS_S_OK;}
    virtual HRESULT Uninit(void* Parameter = NULL)
	{	
		if( m_status!=MS_CLOSE)
				Close();
		m_status = MS_UNINIT;
		return WIS_S_OK;
	}
    virtual HRESULT Run(void* Parameter = NULL)
	{	m_status = MS_RUN;	return WIS_S_OK;}
    virtual HRESULT Stop(void* Parameter = NULL)
	{m_status = MS_STOP;return WIS_S_OK;}
    virtual HRESULT Pause(void* Parameter = NULL)
	{m_status = MS_PAUSE;return WIS_S_OK;}
    virtual HRESULT Close(void* Parameter = NULL)
	{
		for(long i =0;i<m_InputPinCount;i++)
		{
			if( m_aInputPin[i])
				m_aInputPin[i]->Disconnect();
		}
		for(i =0;i<m_OutputPinCount;i++)
		{
			if( m_aOuputPin[i])
				m_aOuputPin[i]->Disconnect();
		}
		m_status = MS_CLOSE;
		return WIS_S_OK;
	}
    virtual HRESULT Working(){return WIS_E_NOTIMPL;}
//Helpers
	IWisPin* InputPin(long lIndex)
	{return lIndex<m_InputPinCount?m_aInputPin[lIndex]:NULL;}
	IWisPin* OuputPin(long lIndex)
	{return lIndex<m_OutputPinCount?m_aOuputPin[lIndex]:NULL;}
	bool IsAlone()
	{
		for(long i =0;i<m_InputPinCount;i++)
		{
			if( m_aInputPin[i])
			{
				if( m_aInputPin[i]->IsConnected() )
					return false;
			}
		}
		for(i =0;i<m_OutputPinCount;i++)
		{
			if( m_aOuputPin[i])
			{
				if( m_aOuputPin[i]->IsConnected() )
					return false;
			}
		}
		return true;
	}
protected:
  unsigned long m_ulRef;
  IHost* m_pHost;
};
class IWisGraph:public IWis
{
public:
	IWisGraph(int nMoudleNum)
	{
		m_ulRef =1;
		m_ppModule = new IWisModule*[nMoudleNum];
		m_nModuleNum =0;
	}
	unsigned long AddRef()
	{
		m_ulRef++;
		return m_ulRef;
	}
	unsigned long Release()
	{
		m_ulRef--;
		if( m_ulRef == 0)
		{
			for( int i=0;i<m_nModuleNum;i++)
			{
				m_ppModule[i]->Release();
			}
			delete m_ppModule;
			m_ppModule = NULL;
			delete this;
		}
		return m_ulRef;
	}
    HRESULT Run()
	{	
		HRESULT hr = WIS_S_OK;
		for( int i=m_nModuleNum-1;i>=0;i--)
		{
			if(! m_ppModule[i]->IsAlone())
			{
				hr = m_ppModule[i]->Run();
				//if( hr != WIS_S_OK)
				//	return hr;
			}
		}
		return WIS_S_OK;
	}
    HRESULT Stop()
	{
		HRESULT hr = WIS_S_OK;
		for( int i=0;i<m_nModuleNum;i++)
		{
			if(! m_ppModule[i]->IsAlone())
			{
				hr = m_ppModule[i]->Stop();
				if( hr != WIS_S_OK)
					return hr;
			}
		}
		return WIS_S_OK;
	}
    HRESULT Pause()
	{
		HRESULT hr = WIS_S_OK;
		for( int i=0;i<m_nModuleNum;i++)
		{
			if(! m_ppModule[i]->IsAlone())
			{
				hr = m_ppModule[i]->Pause();
				if( hr != WIS_S_OK)
					return hr;
			}
		}
		return WIS_S_OK;
	}
	int Add(IWisModule* pModule)
	{
		m_ppModule[m_nModuleNum] = pModule;
		return m_nModuleNum++;
	}
protected:
  unsigned long m_ulRef;
  IWisModule** m_ppModule;
  int m_nModuleNum;
};
#define WIS_WAVE_FORMAT_PCM 1
class IWisAudioMediaFormat:public IWisMediaFormat
{ 
public:
  IWisAudioMediaFormat()
  {
	m_ulRef =1;
	FormatData =NULL;
	cbSize =0;
	Type = MT_Audio;
  }
  unsigned short  FormatTag; 
  unsigned short  Channels; 
  unsigned long   SamplesPerSec; 
  unsigned long   BitRate;
  unsigned short  BlockAlign; 
  unsigned short  BitsPerSample; 
} ; 

class IWisVideoMediaFormat:public IWisMediaFormat
{
public:
	IWisVideoMediaFormat()
	{
		m_ulRef =1;
		FormatData =NULL;
		cbSize =0;
		Pitch =0;
		Type = MT_Video;
	}
	unsigned long BitRate;
	double FrameRate;
	unsigned long Width;
    unsigned long Height;
	unsigned short BitCount;
	unsigned long FourCC;
	unsigned long ImageSize;
	unsigned long Pitch;
};
