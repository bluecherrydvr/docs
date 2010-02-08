
#pragma warning( disable: 4049 )  /* more than 64k source lines */

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0347 */
/* at Tue Dec 17 19:02:11 2002
 */
/* Compiler settings for _StreamRender.idl:
    Os, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef ___StreamRender_h__
#define ___StreamRender_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IStreamRender_FWD_DEFINED__
#define __IStreamRender_FWD_DEFINED__
typedef interface IStreamRender IStreamRender;
#endif 	/* __IStreamRender_FWD_DEFINED__ */


#ifndef ___IStreamRenderEvents_FWD_DEFINED__
#define ___IStreamRenderEvents_FWD_DEFINED__
typedef interface _IStreamRenderEvents _IStreamRenderEvents;
#endif 	/* ___IStreamRenderEvents_FWD_DEFINED__ */


#ifndef __IPlayer_FWD_DEFINED__
#define __IPlayer_FWD_DEFINED__
typedef interface IPlayer IPlayer;
#endif 	/* __IPlayer_FWD_DEFINED__ */


#ifndef __CStreamRender_FWD_DEFINED__
#define __CStreamRender_FWD_DEFINED__

#ifdef __cplusplus
typedef class CStreamRender CStreamRender;
#else
typedef struct CStreamRender CStreamRender;
#endif /* __cplusplus */

#endif 	/* __CStreamRender_FWD_DEFINED__ */


#ifndef ___IPlayerEvents_FWD_DEFINED__
#define ___IPlayerEvents_FWD_DEFINED__
typedef interface _IPlayerEvents _IPlayerEvents;
#endif 	/* ___IPlayerEvents_FWD_DEFINED__ */


#ifndef __CPlayer_FWD_DEFINED__
#define __CPlayer_FWD_DEFINED__

#ifdef __cplusplus
typedef class CPlayer CPlayer;
#else
typedef struct CPlayer CPlayer;
#endif /* __cplusplus */

#endif 	/* __CPlayer_FWD_DEFINED__ */


/* header files for imported files */
#include "prsht.h"
#include "mshtml.h"
#include "mshtmhst.h"
#include "exdisp.h"
#include "objsafe.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

/* interface __MIDL_itf__StreamRender_0000 */
/* [local] */ 

typedef /* [public] */ 
enum SR_RENDERSTATUS
    {	SR_STATUS_STOPPED	= 0,
	SR_STATUS_PAUSED	= 1,
	SR_STATUS_RUNNING	= 2
    } 	SR_RENDERSTATUS;

typedef /* [public] */ 
enum SR_RENDERMODE
    {	SR_PUSH	= 1,
	SR_PULL	= 2
    } 	SR_RENDERMODE;

typedef /* [public] */ 
enum SR_FORMAT
    {	SR_FORMAT_COMPRESSED	= 0,
	SR_FORMAT_RGB24	= 1,
	SR_FORMAT_RGB32	= 2,
	SR_FORMAT_RGB16	= 3,
	SR_FORMAT_UYVY	= 1498831189,
	SR_FORMAT_YUY2	= 844715353,
	SR_FORMAT_RAW8	= 945242450,
	SR_FORMAT_YV12	= 842094169,
	SR_FORMAT_YVU9	= 961893977,
	SR_FORMAT_IV32	= 842225225,
	SR_FORMAT_IV31	= 825448009,
	SR_FORMAT_IF09	= 959465033
    } 	SR_FORMAT;

typedef /* [public] */ 
enum SR_CALLBACKTYPE
    {	SR_RANGECHANGED	= 1,
	SR_DOUBLECLICK	= 2,
	SR_FILLBUFFER	= 3,
	SR_CLOSEWINDOW	= 4,
	SR_UPDATEUI	= 5,
	SR_COMMAND	= 6
    } 	SR_CALLBACKTYPE;

typedef /* [public] */ 
enum SR_DISABLEMENUTYPE
    {	SR_DISABLECLOSEMENU	= 1,
	SR_DISABLERANGECHANGEDMENU	= 2
    } 	SR_DISABLEMENUTYPE;



extern RPC_IF_HANDLE __MIDL_itf__StreamRender_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf__StreamRender_0000_v0_0_s_ifspec;

#ifndef __IStreamRender_INTERFACE_DEFINED__
#define __IStreamRender_INTERFACE_DEFINED__

/* interface IStreamRender */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IStreamRender;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("66AC1BE9-A8E6-4684-80A8-678B60A1D276")
    IStreamRender : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE DeliverSample( 
            /* [in] */ LONG lSize,
            /* [in] */ LONGLONG llStartTime,
            /* [in] */ LONGLONG llStopTime) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetFrameSize( 
            /* [in] */ LONG lWidth,
            /* [in] */ LONG lHeight) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetFormat( 
            /* [in] */ enum SR_FORMAT format,
            /* [in] */ LONG lFourccCode,
            /* [retval][out] */ LONG *plPitch) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Start( 
            /* [in] */ enum SR_RENDERMODE mode) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AllocBuffer( 
            /* [in] */ LONG lCopyFrontBuffer,
            /* [retval][out] */ LONG *lpBufferHandle) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Stop( void) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_FourCCCount( 
            /* [retval][out] */ LONG *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetFourCC( 
            /* [in] */ LONG lIndex,
            /* [retval][out] */ enum SR_FORMAT *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_FrameRate( 
            /* [retval][out] */ double *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_FrameRate( 
            /* [in] */ double newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Pause( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AddPopupMenu( 
            /* [in] */ BSTR szName,
            /* [retval][out] */ LONG *handle) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AddMenu( 
            /* [in] */ LONG ParentHandle,
            /* [in] */ BSTR szName,
            /* [in] */ LONG hBitmap,
            /* [retval][out] */ LONG *handle) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetCaption( 
            /* [in] */ BSTR szCaption) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE StartSelectRange( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE EndSelectRange( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetSelectRange( 
            /* [out][in] */ LONG *top,
            /* [out][in] */ LONG *left,
            /* [out][in] */ LONG *bottom,
            /* [out][in] */ LONG *right) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE DeleteSubMenu( 
            /* [in] */ LONG handle) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Window( 
            /* [retval][out] */ LONG *pVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ZoomTo100OnStart( 
            /* [retval][out] */ VARIANT_BOOL *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ZoomTo100OnStart( 
            /* [in] */ VARIANT_BOOL newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetCallback( 
            /* [in] */ enum SR_CALLBACKTYPE type,
            /* [in] */ LONG callback,
            /* [in] */ LONG param) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE DisableDefaultMenu( 
            /* [in] */ DWORD dwFlag) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetRangeChangedMenuText( 
            /* [in] */ BSTR szStartText,
            /* [in] */ BSTR szEndText) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE NegotiateFormat( 
            /* [in] */ enum SR_FORMAT *pPreferedFormatsSet,
            /* [in] */ LONG lPreferedFormatsNumber,
            /* [out][in] */ enum SR_FORMAT *pBestFormat,
            /* [retval][out] */ LONG *plPitch) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE PreStop( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IStreamRenderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IStreamRender * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IStreamRender * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IStreamRender * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IStreamRender * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IStreamRender * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IStreamRender * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IStreamRender * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *DeliverSample )( 
            IStreamRender * This,
            /* [in] */ LONG lSize,
            /* [in] */ LONGLONG llStartTime,
            /* [in] */ LONGLONG llStopTime);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetFrameSize )( 
            IStreamRender * This,
            /* [in] */ LONG lWidth,
            /* [in] */ LONG lHeight);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetFormat )( 
            IStreamRender * This,
            /* [in] */ enum SR_FORMAT format,
            /* [in] */ LONG lFourccCode,
            /* [retval][out] */ LONG *plPitch);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Start )( 
            IStreamRender * This,
            /* [in] */ enum SR_RENDERMODE mode);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *AllocBuffer )( 
            IStreamRender * This,
            /* [in] */ LONG lCopyFrontBuffer,
            /* [retval][out] */ LONG *lpBufferHandle);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IStreamRender * This);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_FourCCCount )( 
            IStreamRender * This,
            /* [retval][out] */ LONG *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetFourCC )( 
            IStreamRender * This,
            /* [in] */ LONG lIndex,
            /* [retval][out] */ enum SR_FORMAT *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_FrameRate )( 
            IStreamRender * This,
            /* [retval][out] */ double *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_FrameRate )( 
            IStreamRender * This,
            /* [in] */ double newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Pause )( 
            IStreamRender * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *AddPopupMenu )( 
            IStreamRender * This,
            /* [in] */ BSTR szName,
            /* [retval][out] */ LONG *handle);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *AddMenu )( 
            IStreamRender * This,
            /* [in] */ LONG ParentHandle,
            /* [in] */ BSTR szName,
            /* [in] */ LONG hBitmap,
            /* [retval][out] */ LONG *handle);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetCaption )( 
            IStreamRender * This,
            /* [in] */ BSTR szCaption);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *StartSelectRange )( 
            IStreamRender * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *EndSelectRange )( 
            IStreamRender * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetSelectRange )( 
            IStreamRender * This,
            /* [out][in] */ LONG *top,
            /* [out][in] */ LONG *left,
            /* [out][in] */ LONG *bottom,
            /* [out][in] */ LONG *right);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *DeleteSubMenu )( 
            IStreamRender * This,
            /* [in] */ LONG handle);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Window )( 
            IStreamRender * This,
            /* [retval][out] */ LONG *pVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ZoomTo100OnStart )( 
            IStreamRender * This,
            /* [retval][out] */ VARIANT_BOOL *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ZoomTo100OnStart )( 
            IStreamRender * This,
            /* [in] */ VARIANT_BOOL newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetCallback )( 
            IStreamRender * This,
            /* [in] */ enum SR_CALLBACKTYPE type,
            /* [in] */ LONG callback,
            /* [in] */ LONG param);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *DisableDefaultMenu )( 
            IStreamRender * This,
            /* [in] */ DWORD dwFlag);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetRangeChangedMenuText )( 
            IStreamRender * This,
            /* [in] */ BSTR szStartText,
            /* [in] */ BSTR szEndText);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *NegotiateFormat )( 
            IStreamRender * This,
            /* [in] */ enum SR_FORMAT *pPreferedFormatsSet,
            /* [in] */ LONG lPreferedFormatsNumber,
            /* [out][in] */ enum SR_FORMAT *pBestFormat,
            /* [retval][out] */ LONG *plPitch);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *PreStop )( 
            IStreamRender * This);
        
        END_INTERFACE
    } IStreamRenderVtbl;

    interface IStreamRender
    {
        CONST_VTBL struct IStreamRenderVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStreamRender_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IStreamRender_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IStreamRender_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IStreamRender_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IStreamRender_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IStreamRender_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IStreamRender_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IStreamRender_DeliverSample(This,lSize,llStartTime,llStopTime)	\
    (This)->lpVtbl -> DeliverSample(This,lSize,llStartTime,llStopTime)

#define IStreamRender_SetFrameSize(This,lWidth,lHeight)	\
    (This)->lpVtbl -> SetFrameSize(This,lWidth,lHeight)

#define IStreamRender_SetFormat(This,format,lFourccCode,plPitch)	\
    (This)->lpVtbl -> SetFormat(This,format,lFourccCode,plPitch)

#define IStreamRender_Start(This,mode)	\
    (This)->lpVtbl -> Start(This,mode)

#define IStreamRender_AllocBuffer(This,lCopyFrontBuffer,lpBufferHandle)	\
    (This)->lpVtbl -> AllocBuffer(This,lCopyFrontBuffer,lpBufferHandle)

#define IStreamRender_Stop(This)	\
    (This)->lpVtbl -> Stop(This)

#define IStreamRender_get_FourCCCount(This,pVal)	\
    (This)->lpVtbl -> get_FourCCCount(This,pVal)

#define IStreamRender_GetFourCC(This,lIndex,pVal)	\
    (This)->lpVtbl -> GetFourCC(This,lIndex,pVal)

#define IStreamRender_get_FrameRate(This,pVal)	\
    (This)->lpVtbl -> get_FrameRate(This,pVal)

#define IStreamRender_put_FrameRate(This,newVal)	\
    (This)->lpVtbl -> put_FrameRate(This,newVal)

#define IStreamRender_Pause(This)	\
    (This)->lpVtbl -> Pause(This)

#define IStreamRender_AddPopupMenu(This,szName,handle)	\
    (This)->lpVtbl -> AddPopupMenu(This,szName,handle)

#define IStreamRender_AddMenu(This,ParentHandle,szName,hBitmap,handle)	\
    (This)->lpVtbl -> AddMenu(This,ParentHandle,szName,hBitmap,handle)

#define IStreamRender_SetCaption(This,szCaption)	\
    (This)->lpVtbl -> SetCaption(This,szCaption)

#define IStreamRender_StartSelectRange(This)	\
    (This)->lpVtbl -> StartSelectRange(This)

#define IStreamRender_EndSelectRange(This)	\
    (This)->lpVtbl -> EndSelectRange(This)

#define IStreamRender_GetSelectRange(This,top,left,bottom,right)	\
    (This)->lpVtbl -> GetSelectRange(This,top,left,bottom,right)

#define IStreamRender_DeleteSubMenu(This,handle)	\
    (This)->lpVtbl -> DeleteSubMenu(This,handle)

#define IStreamRender_get_Window(This,pVal)	\
    (This)->lpVtbl -> get_Window(This,pVal)

#define IStreamRender_get_ZoomTo100OnStart(This,pVal)	\
    (This)->lpVtbl -> get_ZoomTo100OnStart(This,pVal)

#define IStreamRender_put_ZoomTo100OnStart(This,newVal)	\
    (This)->lpVtbl -> put_ZoomTo100OnStart(This,newVal)

#define IStreamRender_SetCallback(This,type,callback,param)	\
    (This)->lpVtbl -> SetCallback(This,type,callback,param)

#define IStreamRender_DisableDefaultMenu(This,dwFlag)	\
    (This)->lpVtbl -> DisableDefaultMenu(This,dwFlag)

#define IStreamRender_SetRangeChangedMenuText(This,szStartText,szEndText)	\
    (This)->lpVtbl -> SetRangeChangedMenuText(This,szStartText,szEndText)

#define IStreamRender_NegotiateFormat(This,pPreferedFormatsSet,lPreferedFormatsNumber,pBestFormat,plPitch)	\
    (This)->lpVtbl -> NegotiateFormat(This,pPreferedFormatsSet,lPreferedFormatsNumber,pBestFormat,plPitch)

#define IStreamRender_PreStop(This)	\
    (This)->lpVtbl -> PreStop(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IStreamRender_DeliverSample_Proxy( 
    IStreamRender * This,
    /* [in] */ LONG lSize,
    /* [in] */ LONGLONG llStartTime,
    /* [in] */ LONGLONG llStopTime);


void __RPC_STUB IStreamRender_DeliverSample_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IStreamRender_SetFrameSize_Proxy( 
    IStreamRender * This,
    /* [in] */ LONG lWidth,
    /* [in] */ LONG lHeight);


void __RPC_STUB IStreamRender_SetFrameSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IStreamRender_SetFormat_Proxy( 
    IStreamRender * This,
    /* [in] */ enum SR_FORMAT format,
    /* [in] */ LONG lFourccCode,
    /* [retval][out] */ LONG *plPitch);


void __RPC_STUB IStreamRender_SetFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IStreamRender_Start_Proxy( 
    IStreamRender * This,
    /* [in] */ enum SR_RENDERMODE mode);


void __RPC_STUB IStreamRender_Start_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IStreamRender_AllocBuffer_Proxy( 
    IStreamRender * This,
    /* [in] */ LONG lCopyFrontBuffer,
    /* [retval][out] */ LONG *lpBufferHandle);


void __RPC_STUB IStreamRender_AllocBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IStreamRender_Stop_Proxy( 
    IStreamRender * This);


void __RPC_STUB IStreamRender_Stop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IStreamRender_get_FourCCCount_Proxy( 
    IStreamRender * This,
    /* [retval][out] */ LONG *pVal);


void __RPC_STUB IStreamRender_get_FourCCCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IStreamRender_GetFourCC_Proxy( 
    IStreamRender * This,
    /* [in] */ LONG lIndex,
    /* [retval][out] */ enum SR_FORMAT *pVal);


void __RPC_STUB IStreamRender_GetFourCC_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IStreamRender_get_FrameRate_Proxy( 
    IStreamRender * This,
    /* [retval][out] */ double *pVal);


void __RPC_STUB IStreamRender_get_FrameRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IStreamRender_put_FrameRate_Proxy( 
    IStreamRender * This,
    /* [in] */ double newVal);


void __RPC_STUB IStreamRender_put_FrameRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IStreamRender_Pause_Proxy( 
    IStreamRender * This);


void __RPC_STUB IStreamRender_Pause_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IStreamRender_AddPopupMenu_Proxy( 
    IStreamRender * This,
    /* [in] */ BSTR szName,
    /* [retval][out] */ LONG *handle);


void __RPC_STUB IStreamRender_AddPopupMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IStreamRender_AddMenu_Proxy( 
    IStreamRender * This,
    /* [in] */ LONG ParentHandle,
    /* [in] */ BSTR szName,
    /* [in] */ LONG hBitmap,
    /* [retval][out] */ LONG *handle);


void __RPC_STUB IStreamRender_AddMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IStreamRender_SetCaption_Proxy( 
    IStreamRender * This,
    /* [in] */ BSTR szCaption);


void __RPC_STUB IStreamRender_SetCaption_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IStreamRender_StartSelectRange_Proxy( 
    IStreamRender * This);


void __RPC_STUB IStreamRender_StartSelectRange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IStreamRender_EndSelectRange_Proxy( 
    IStreamRender * This);


void __RPC_STUB IStreamRender_EndSelectRange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IStreamRender_GetSelectRange_Proxy( 
    IStreamRender * This,
    /* [out][in] */ LONG *top,
    /* [out][in] */ LONG *left,
    /* [out][in] */ LONG *bottom,
    /* [out][in] */ LONG *right);


void __RPC_STUB IStreamRender_GetSelectRange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IStreamRender_DeleteSubMenu_Proxy( 
    IStreamRender * This,
    /* [in] */ LONG handle);


void __RPC_STUB IStreamRender_DeleteSubMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IStreamRender_get_Window_Proxy( 
    IStreamRender * This,
    /* [retval][out] */ LONG *pVal);


void __RPC_STUB IStreamRender_get_Window_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IStreamRender_get_ZoomTo100OnStart_Proxy( 
    IStreamRender * This,
    /* [retval][out] */ VARIANT_BOOL *pVal);


void __RPC_STUB IStreamRender_get_ZoomTo100OnStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IStreamRender_put_ZoomTo100OnStart_Proxy( 
    IStreamRender * This,
    /* [in] */ VARIANT_BOOL newVal);


void __RPC_STUB IStreamRender_put_ZoomTo100OnStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IStreamRender_SetCallback_Proxy( 
    IStreamRender * This,
    /* [in] */ enum SR_CALLBACKTYPE type,
    /* [in] */ LONG callback,
    /* [in] */ LONG param);


void __RPC_STUB IStreamRender_SetCallback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IStreamRender_DisableDefaultMenu_Proxy( 
    IStreamRender * This,
    /* [in] */ DWORD dwFlag);


void __RPC_STUB IStreamRender_DisableDefaultMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IStreamRender_SetRangeChangedMenuText_Proxy( 
    IStreamRender * This,
    /* [in] */ BSTR szStartText,
    /* [in] */ BSTR szEndText);


void __RPC_STUB IStreamRender_SetRangeChangedMenuText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IStreamRender_NegotiateFormat_Proxy( 
    IStreamRender * This,
    /* [in] */ enum SR_FORMAT *pPreferedFormatsSet,
    /* [in] */ LONG lPreferedFormatsNumber,
    /* [out][in] */ enum SR_FORMAT *pBestFormat,
    /* [retval][out] */ LONG *plPitch);


void __RPC_STUB IStreamRender_NegotiateFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IStreamRender_PreStop_Proxy( 
    IStreamRender * This);


void __RPC_STUB IStreamRender_PreStop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IStreamRender_INTERFACE_DEFINED__ */


#ifndef ___IStreamRenderEvents_INTERFACE_DEFINED__
#define ___IStreamRenderEvents_INTERFACE_DEFINED__

/* interface _IStreamRenderEvents */
/* [object][unique][helpstring][dual][uuid] */ 


EXTERN_C const IID IID__IStreamRenderEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E5CFD04B-C007-402E-91E4-901FCBF6A9B8")
    _IStreamRenderEvents : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE DoubleClick( 
            /* [in] */ LONG xOff,
            /* [in] */ LONG yOff) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE FillBuffer( 
            /* [in] */ LONG lpBufferHandle,
            /* [in] */ LONG lBufferSize,
            /* [out][in] */ double *pdStartTime,
            /* [out][in] */ double *pdStopTime,
            /* [retval][out] */ LONG *plRealBufferSize) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CloseWindow( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE UpdateUI( 
            /* [in] */ LONG nID,
            /* [out][in] */ VARIANT_BOOL *bEnable,
            /* [out][in] */ VARIANT_BOOL *bCheck,
            /* [out][in] */ BSTR *pszNewName,
            /* [out][in] */ LONG *plResult) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Command( 
            /* [in] */ LONG nID) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct _IStreamRenderEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _IStreamRenderEvents * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _IStreamRenderEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _IStreamRenderEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _IStreamRenderEvents * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _IStreamRenderEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _IStreamRenderEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _IStreamRenderEvents * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *DoubleClick )( 
            _IStreamRenderEvents * This,
            /* [in] */ LONG xOff,
            /* [in] */ LONG yOff);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *FillBuffer )( 
            _IStreamRenderEvents * This,
            /* [in] */ LONG lpBufferHandle,
            /* [in] */ LONG lBufferSize,
            /* [out][in] */ double *pdStartTime,
            /* [out][in] */ double *pdStopTime,
            /* [retval][out] */ LONG *plRealBufferSize);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *CloseWindow )( 
            _IStreamRenderEvents * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *UpdateUI )( 
            _IStreamRenderEvents * This,
            /* [in] */ LONG nID,
            /* [out][in] */ VARIANT_BOOL *bEnable,
            /* [out][in] */ VARIANT_BOOL *bCheck,
            /* [out][in] */ BSTR *pszNewName,
            /* [out][in] */ LONG *plResult);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Command )( 
            _IStreamRenderEvents * This,
            /* [in] */ LONG nID);
        
        END_INTERFACE
    } _IStreamRenderEventsVtbl;

    interface _IStreamRenderEvents
    {
        CONST_VTBL struct _IStreamRenderEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _IStreamRenderEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _IStreamRenderEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _IStreamRenderEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _IStreamRenderEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _IStreamRenderEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _IStreamRenderEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _IStreamRenderEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define _IStreamRenderEvents_DoubleClick(This,xOff,yOff)	\
    (This)->lpVtbl -> DoubleClick(This,xOff,yOff)

#define _IStreamRenderEvents_FillBuffer(This,lpBufferHandle,lBufferSize,pdStartTime,pdStopTime,plRealBufferSize)	\
    (This)->lpVtbl -> FillBuffer(This,lpBufferHandle,lBufferSize,pdStartTime,pdStopTime,plRealBufferSize)

#define _IStreamRenderEvents_CloseWindow(This)	\
    (This)->lpVtbl -> CloseWindow(This)

#define _IStreamRenderEvents_UpdateUI(This,nID,bEnable,bCheck,pszNewName,plResult)	\
    (This)->lpVtbl -> UpdateUI(This,nID,bEnable,bCheck,pszNewName,plResult)

#define _IStreamRenderEvents_Command(This,nID)	\
    (This)->lpVtbl -> Command(This,nID)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE _IStreamRenderEvents_DoubleClick_Proxy( 
    _IStreamRenderEvents * This,
    /* [in] */ LONG xOff,
    /* [in] */ LONG yOff);


void __RPC_STUB _IStreamRenderEvents_DoubleClick_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE _IStreamRenderEvents_FillBuffer_Proxy( 
    _IStreamRenderEvents * This,
    /* [in] */ LONG lpBufferHandle,
    /* [in] */ LONG lBufferSize,
    /* [out][in] */ double *pdStartTime,
    /* [out][in] */ double *pdStopTime,
    /* [retval][out] */ LONG *plRealBufferSize);


void __RPC_STUB _IStreamRenderEvents_FillBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE _IStreamRenderEvents_CloseWindow_Proxy( 
    _IStreamRenderEvents * This);


void __RPC_STUB _IStreamRenderEvents_CloseWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE _IStreamRenderEvents_UpdateUI_Proxy( 
    _IStreamRenderEvents * This,
    /* [in] */ LONG nID,
    /* [out][in] */ VARIANT_BOOL *bEnable,
    /* [out][in] */ VARIANT_BOOL *bCheck,
    /* [out][in] */ BSTR *pszNewName,
    /* [out][in] */ LONG *plResult);


void __RPC_STUB _IStreamRenderEvents_UpdateUI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE _IStreamRenderEvents_Command_Proxy( 
    _IStreamRenderEvents * This,
    /* [in] */ LONG nID);


void __RPC_STUB _IStreamRenderEvents_Command_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* ___IStreamRenderEvents_INTERFACE_DEFINED__ */


/* interface __MIDL_itf__StreamRender_0735 */
/* [local] */ 

typedef /* [public] */ 
enum RESERVE_MENU_ID
    {	RMD_ZOOM50	= 309,
	RMD_ZOOM100	= 310,
	RMD_ZOOM200	= 311,
	RMD_FULLSCREEN	= 211,
	RMD_CLOSEWINDOW	= 300
    } 	RESERVE_MENU_ID;

typedef /* [public] */ 
enum MENUUPDATEUIINFO_MASKTYPE
    {	MM_ENABLEFIELD	= 1,
	MM_CHECKFIELD	= 2,
	MM_NAMEFIELD	= 4
    } 	MENUUPDATEUIINFO_MASKTYPE;

typedef /* [public] */ struct MENUUPDATEUIINFO
    {
    DWORD dwMask;
    BOOL bEnable;
    BOOL bCheck;
    LPCSTR szNewName;
    } 	MENUUPDATEUIINFO;

typedef /* [public] */ 
enum MP_CALLBACK_TYPE
    {	CT_CHANGECHNL	= 1,
	CT_CLOSEWINDOW	= 2,
	CT_DECRESULT	= 3,
	CT_UPDATEUI	= 4,
	CT_COMMAND	= 5
    } 	MP_CALLBACK_TYPE;



extern RPC_IF_HANDLE __MIDL_itf__StreamRender_0735_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf__StreamRender_0735_v0_0_s_ifspec;

#ifndef __IPlayer_INTERFACE_DEFINED__
#define __IPlayer_INTERFACE_DEFINED__

/* interface IPlayer */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IPlayer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D469F09C-08AE-4E49-826A-8AB3073E8972")
    IPlayer : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Init( 
            /* [in] */ LONG channels,
            /* [in] */ LONG DisplayWidth,
            /* [in] */ LONG DisplayHeight,
            /* [in] */ VARIANT_BOOL bLiveMode) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetChnl( 
            /* [in] */ LONG chnl,
            /* [in] */ BYTE mode,
            /* [in] */ LONG offsetX,
            /* [in] */ LONG offsetY,
            /* [defaultvalue][in] */ LONG *fifoLvl = 0,
            /* [defaultvalue][in] */ double ratioSpeed = 1) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SourceInput( 
            /* [in] */ LONG chnl,
            /* [in] */ BYTE *srcStr,
            /* [in] */ LONG quota,
            /* [retval][out] */ LONG *retQuota) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Play( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Pause( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Stop( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetFiFoRatio( 
            /* [in] */ LONG chnl,
            /* [out][in] */ LONG *bufferredFrm,
            /* [retval][out] */ double *plPercentage) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ConfigurateUI( 
            /* [in] */ VARIANT_BOOL bShowPPMenu,
            /* [in] */ VARIANT_BOOL bShowDecLevelMenu) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetCallback( 
            /* [in] */ LONG pCallback) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetRender( 
            /* [retval][out] */ IStreamRender **ppRender) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_PostprocessLevel( 
            /* [retval][out] */ LONG *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_PostprocessLevel( 
            /* [in] */ LONG newVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetPostprocessArea( 
            /* [in] */ LONG left,
            /* [in] */ LONG top,
            /* [in] */ LONG Width,
            /* [in] */ LONG Height) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_DecodeLevel( 
            /* [retval][out] */ LONG *pVal) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_DecodeLevel( 
            /* [in] */ LONG newVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPlayerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPlayer * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPlayer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPlayer * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IPlayer * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IPlayer * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IPlayer * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IPlayer * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Init )( 
            IPlayer * This,
            /* [in] */ LONG channels,
            /* [in] */ LONG DisplayWidth,
            /* [in] */ LONG DisplayHeight,
            /* [in] */ VARIANT_BOOL bLiveMode);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetChnl )( 
            IPlayer * This,
            /* [in] */ LONG chnl,
            /* [in] */ BYTE mode,
            /* [in] */ LONG offsetX,
            /* [in] */ LONG offsetY,
            /* [defaultvalue][in] */ LONG *fifoLvl,
            /* [defaultvalue][in] */ double ratioSpeed);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SourceInput )( 
            IPlayer * This,
            /* [in] */ LONG chnl,
            /* [in] */ BYTE *srcStr,
            /* [in] */ LONG quota,
            /* [retval][out] */ LONG *retQuota);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Play )( 
            IPlayer * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Pause )( 
            IPlayer * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IPlayer * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetFiFoRatio )( 
            IPlayer * This,
            /* [in] */ LONG chnl,
            /* [out][in] */ LONG *bufferredFrm,
            /* [retval][out] */ double *plPercentage);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *ConfigurateUI )( 
            IPlayer * This,
            /* [in] */ VARIANT_BOOL bShowPPMenu,
            /* [in] */ VARIANT_BOOL bShowDecLevelMenu);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetCallback )( 
            IPlayer * This,
            /* [in] */ LONG pCallback);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetRender )( 
            IPlayer * This,
            /* [retval][out] */ IStreamRender **ppRender);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_PostprocessLevel )( 
            IPlayer * This,
            /* [retval][out] */ LONG *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_PostprocessLevel )( 
            IPlayer * This,
            /* [in] */ LONG newVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetPostprocessArea )( 
            IPlayer * This,
            /* [in] */ LONG left,
            /* [in] */ LONG top,
            /* [in] */ LONG Width,
            /* [in] */ LONG Height);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_DecodeLevel )( 
            IPlayer * This,
            /* [retval][out] */ LONG *pVal);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_DecodeLevel )( 
            IPlayer * This,
            /* [in] */ LONG newVal);
        
        END_INTERFACE
    } IPlayerVtbl;

    interface IPlayer
    {
        CONST_VTBL struct IPlayerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPlayer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPlayer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPlayer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPlayer_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IPlayer_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IPlayer_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IPlayer_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IPlayer_Init(This,channels,DisplayWidth,DisplayHeight,bLiveMode)	\
    (This)->lpVtbl -> Init(This,channels,DisplayWidth,DisplayHeight,bLiveMode)

#define IPlayer_SetChnl(This,chnl,mode,offsetX,offsetY,fifoLvl,ratioSpeed)	\
    (This)->lpVtbl -> SetChnl(This,chnl,mode,offsetX,offsetY,fifoLvl,ratioSpeed)

#define IPlayer_SourceInput(This,chnl,srcStr,quota,retQuota)	\
    (This)->lpVtbl -> SourceInput(This,chnl,srcStr,quota,retQuota)

#define IPlayer_Play(This)	\
    (This)->lpVtbl -> Play(This)

#define IPlayer_Pause(This)	\
    (This)->lpVtbl -> Pause(This)

#define IPlayer_Stop(This)	\
    (This)->lpVtbl -> Stop(This)

#define IPlayer_GetFiFoRatio(This,chnl,bufferredFrm,plPercentage)	\
    (This)->lpVtbl -> GetFiFoRatio(This,chnl,bufferredFrm,plPercentage)

#define IPlayer_ConfigurateUI(This,bShowPPMenu,bShowDecLevelMenu)	\
    (This)->lpVtbl -> ConfigurateUI(This,bShowPPMenu,bShowDecLevelMenu)

#define IPlayer_SetCallback(This,pCallback)	\
    (This)->lpVtbl -> SetCallback(This,pCallback)

#define IPlayer_GetRender(This,ppRender)	\
    (This)->lpVtbl -> GetRender(This,ppRender)

#define IPlayer_get_PostprocessLevel(This,pVal)	\
    (This)->lpVtbl -> get_PostprocessLevel(This,pVal)

#define IPlayer_put_PostprocessLevel(This,newVal)	\
    (This)->lpVtbl -> put_PostprocessLevel(This,newVal)

#define IPlayer_SetPostprocessArea(This,left,top,Width,Height)	\
    (This)->lpVtbl -> SetPostprocessArea(This,left,top,Width,Height)

#define IPlayer_get_DecodeLevel(This,pVal)	\
    (This)->lpVtbl -> get_DecodeLevel(This,pVal)

#define IPlayer_put_DecodeLevel(This,newVal)	\
    (This)->lpVtbl -> put_DecodeLevel(This,newVal)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IPlayer_Init_Proxy( 
    IPlayer * This,
    /* [in] */ LONG channels,
    /* [in] */ LONG DisplayWidth,
    /* [in] */ LONG DisplayHeight,
    /* [in] */ VARIANT_BOOL bLiveMode);


void __RPC_STUB IPlayer_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IPlayer_SetChnl_Proxy( 
    IPlayer * This,
    /* [in] */ LONG chnl,
    /* [in] */ BYTE mode,
    /* [in] */ LONG offsetX,
    /* [in] */ LONG offsetY,
    /* [defaultvalue][in] */ LONG *fifoLvl,
    /* [defaultvalue][in] */ double ratioSpeed);


void __RPC_STUB IPlayer_SetChnl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IPlayer_SourceInput_Proxy( 
    IPlayer * This,
    /* [in] */ LONG chnl,
    /* [in] */ BYTE *srcStr,
    /* [in] */ LONG quota,
    /* [retval][out] */ LONG *retQuota);


void __RPC_STUB IPlayer_SourceInput_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IPlayer_Play_Proxy( 
    IPlayer * This);


void __RPC_STUB IPlayer_Play_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IPlayer_Pause_Proxy( 
    IPlayer * This);


void __RPC_STUB IPlayer_Pause_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IPlayer_Stop_Proxy( 
    IPlayer * This);


void __RPC_STUB IPlayer_Stop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IPlayer_GetFiFoRatio_Proxy( 
    IPlayer * This,
    /* [in] */ LONG chnl,
    /* [out][in] */ LONG *bufferredFrm,
    /* [retval][out] */ double *plPercentage);


void __RPC_STUB IPlayer_GetFiFoRatio_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IPlayer_ConfigurateUI_Proxy( 
    IPlayer * This,
    /* [in] */ VARIANT_BOOL bShowPPMenu,
    /* [in] */ VARIANT_BOOL bShowDecLevelMenu);


void __RPC_STUB IPlayer_ConfigurateUI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IPlayer_SetCallback_Proxy( 
    IPlayer * This,
    /* [in] */ LONG pCallback);


void __RPC_STUB IPlayer_SetCallback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IPlayer_GetRender_Proxy( 
    IPlayer * This,
    /* [retval][out] */ IStreamRender **ppRender);


void __RPC_STUB IPlayer_GetRender_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IPlayer_get_PostprocessLevel_Proxy( 
    IPlayer * This,
    /* [retval][out] */ LONG *pVal);


void __RPC_STUB IPlayer_get_PostprocessLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IPlayer_put_PostprocessLevel_Proxy( 
    IPlayer * This,
    /* [in] */ LONG newVal);


void __RPC_STUB IPlayer_put_PostprocessLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IPlayer_SetPostprocessArea_Proxy( 
    IPlayer * This,
    /* [in] */ LONG left,
    /* [in] */ LONG top,
    /* [in] */ LONG Width,
    /* [in] */ LONG Height);


void __RPC_STUB IPlayer_SetPostprocessArea_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IPlayer_get_DecodeLevel_Proxy( 
    IPlayer * This,
    /* [retval][out] */ LONG *pVal);


void __RPC_STUB IPlayer_get_DecodeLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IPlayer_put_DecodeLevel_Proxy( 
    IPlayer * This,
    /* [in] */ LONG newVal);


void __RPC_STUB IPlayer_put_DecodeLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IPlayer_INTERFACE_DEFINED__ */



#ifndef __StreamRender_LIBRARY_DEFINED__
#define __StreamRender_LIBRARY_DEFINED__

/* library StreamRender */
/* [helpstring][uuid][version] */ 


EXTERN_C const IID LIBID_StreamRender;

EXTERN_C const CLSID CLSID_CStreamRender;

#ifdef __cplusplus

class DECLSPEC_UUID("923BE43C-6595-4C88-A50E-508665528EBC")
CStreamRender;
#endif

#ifndef ___IPlayerEvents_DISPINTERFACE_DEFINED__
#define ___IPlayerEvents_DISPINTERFACE_DEFINED__

/* dispinterface _IPlayerEvents */
/* [helpstring][uuid] */ 


EXTERN_C const IID DIID__IPlayerEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("39F10990-844D-48F8-B410-C4BBC58C365E")
    _IPlayerEvents : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct _IPlayerEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            _IPlayerEvents * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            _IPlayerEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            _IPlayerEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            _IPlayerEvents * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            _IPlayerEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            _IPlayerEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            _IPlayerEvents * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } _IPlayerEventsVtbl;

    interface _IPlayerEvents
    {
        CONST_VTBL struct _IPlayerEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define _IPlayerEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define _IPlayerEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define _IPlayerEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define _IPlayerEvents_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define _IPlayerEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define _IPlayerEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define _IPlayerEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* ___IPlayerEvents_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_CPlayer;

#ifdef __cplusplus

class DECLSPEC_UUID("B43B7A54-B7C0-43FB-B02B-1BA5D623DF78")
CPlayer;
#endif
#endif /* __StreamRender_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


