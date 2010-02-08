
#ifndef __WISVIDEOCONTROLEX_H__
#define __WISVIDEOCONTROLEX_H__

#include "struct.h"

///////////////////////////////////////////////////////////
// because the interface id should be the same of the class
// id for some reason only kernel streaming guys know.
// and if I put the same iid and clsid, I can not make midl
// compiler to be quiet about it. so I had to make the 
// IWisVideoControl as a dummy interface and put real stuff
// in the IWisVideoControlEx interface.
MIDL_INTERFACE("5C28C78C-6187-4A32-9BEC-C9698E82B3F2")
IWISVideoControlEx : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE GetVideoConfig(TVIDEOCFGEX* pConfig) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetVideoConfig(TVIDEOCFGEX* pConfig) = 0;
	virtual HRESULT STDMETHODCALLTYPE EnablePiggyBackAudioSignature(BOOL bEnable) = 0;
};

#endif