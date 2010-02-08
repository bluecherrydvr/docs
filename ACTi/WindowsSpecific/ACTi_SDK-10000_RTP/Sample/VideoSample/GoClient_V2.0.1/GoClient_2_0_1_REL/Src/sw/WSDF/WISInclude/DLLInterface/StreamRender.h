/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Thu Jan 09 17:28:52 2003
 */
/* Compiler settings for C:\WSDF\WISDevelopSpace\WISDLL\LiteStreamRender\StreamRender.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
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

#ifndef __StreamRender_h__
#define __StreamRender_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IStreamRender_FWD_DEFINED__
#define __IStreamRender_FWD_DEFINED__
typedef interface IStreamRender IStreamRender;
#endif 	/* __IStreamRender_FWD_DEFINED__ */


#ifndef __StreamRender_FWD_DEFINED__
#define __StreamRender_FWD_DEFINED__

#ifdef __cplusplus
typedef class StreamRender StreamRender;
#else
typedef struct StreamRender StreamRender;
#endif /* __cplusplus */

#endif 	/* __StreamRender_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

/* interface __MIDL_itf_StreamRender_0000 */
/* [local] */ 

typedef 
enum SR_RENDERSTATUS
    {	SR_STATUS_STOPPED	= 0,
	SR_STATUS_PAUSED	= 1,
	SR_STATUS_RUNNING	= 2
    }	SR_RENDERSTATUS;

typedef 
enum SR_FORMAT
    {	SR_FORMAT_NONE	= 0,
	SR_FORMAT_RGB24	= 0x1,
	SR_FORMAT_RGB32	= 0x2,
	SR_FORMAT_RGB16	= 0x3,
	SR_FORMAT_DIB24	= 0x11,
	SR_FORMAT_DIB32	= 0x12,
	SR_FORMAT_DIB16	= 0x13,
	SR_FORMAT_UYVY	= 0x59565955,
	SR_FORMAT_YUY2	= 0x32595559,
	SR_FORMAT_RAW8	= 0x38574152,
	SR_FORMAT_YV12	= 0x32315659,
	SR_FORMAT_YVU9	= 0x39555659,
	SR_FORMAT_IV32	= 0x32335649,
	SR_FORMAT_IV31	= 0x31335649,
	SR_FORMAT_IF09	= 0x39304649
    }	SR_FORMAT;

typedef 
enum SR_CALLBACKTYPE
    {	SR_RANGECHANGED	= 0x1,
	SR_DOUBLECLICK	= 0x2,
	SR_FILLBUFFER	= 0x3,
	SR_CLOSEWINDOW	= 0x4,
	SR_UPDATEUI	= 0x5,
	SR_COMMAND	= 0x6
    }	SR_CALLBACKTYPE;

typedef 
enum SR_DISABLEMENUTYPE
    {	SR_DISABLECLOSEMENU	= 0x1,
	SR_DISABLERANGECHANGEDMENU	= 0x2
    }	SR_DISABLEMENUTYPE;

typedef HRESULT ( __RPC_FAR *RANGECHANGEDCALLBACK )( 
    LONG pThis,
    LONG top,
    LONG left,
    LONG bottom,
    LONG right);

typedef HRESULT ( __RPC_FAR *DOUBLECLICKCALLBACK )( 
    LONG pThis,
    LONG xOff,
    LONG yOff);

typedef HRESULT ( __RPC_FAR *FILLBUFFERCALLBACK )( 
    LONG pThis,
    LONG lpBufferHandle,
    LONG lBufferSize,
    double __RPC_FAR *pdStartTime,
    double __RPC_FAR *pdStopTime,
    LONG __RPC_FAR *plRealBufferSize);

typedef HRESULT ( __RPC_FAR *CLOSEWINDOWCALLBACK )( 
    LONG pThis);

typedef HRESULT ( __RPC_FAR *UPDATEUICALLBACK )( 
    LONG pThis,
    LONG nID,
    VARIANT_BOOL __RPC_FAR *bEnable,
    VARIANT_BOOL __RPC_FAR *bCheck,
    BSTR __RPC_FAR *pszNewName,
    LONG __RPC_FAR *plResult);

typedef HRESULT ( __RPC_FAR *COMMANDCALLBACK )( 
    LONG pThis,
    LONG nID);



extern RPC_IF_HANDLE __MIDL_itf_StreamRender_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_StreamRender_0000_v0_0_s_ifspec;

#ifndef __IStreamRender_INTERFACE_DEFINED__
#define __IStreamRender_INTERFACE_DEFINED__

/* interface IStreamRender */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IStreamRender;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5E5C1B8E-6C8A-4ee1-8856-C5EEFFA0E5C4")
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
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Start( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AllocBuffer( 
            /* [in] */ LONG lFlags,
            /* [retval][out] */ LONG __RPC_FAR *lpBufferHandle) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Stop( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Pause( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AddPopupMenu( 
            /* [in] */ BSTR szName,
            /* [retval][out] */ LONG __RPC_FAR *handle) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AddMenu( 
            /* [in] */ LONG ParentHandle,
            /* [in] */ BSTR szName,
            /* [in] */ LONG hBitmap,
            /* [retval][out] */ LONG __RPC_FAR *handle) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetCaption( 
            /* [in] */ BSTR szCaption) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE StartSelectRange( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE EndSelectRange( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetSelectRange( 
            /* [out][in] */ LONG __RPC_FAR *top,
            /* [out][in] */ LONG __RPC_FAR *left,
            /* [out][in] */ LONG __RPC_FAR *bottom,
            /* [out][in] */ LONG __RPC_FAR *right) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE DeleteSubMenu( 
            /* [in] */ LONG handle) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Window( 
            /* [retval][out] */ LONG __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetCallback( 
            /* [in] */ SR_CALLBACKTYPE type,
            /* [in] */ LONG callback,
            /* [in] */ LONG param) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE DisableDefaultMenu( 
            DWORD dwFlag) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetRangeChangedMenuText( 
            BSTR szStartText,
            BSTR szEndText) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE NegotiateFormat( 
            /* [in] */ SR_FORMAT __RPC_FAR *pPreferedFormatsSet,
            /* [in] */ LONG lPreferedFormatsNumber,
            /* [out][in] */ SR_FORMAT __RPC_FAR *pBestFormat,
            /* [retval][out] */ LONG __RPC_FAR *plPitch) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IStreamRenderVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IStreamRender __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IStreamRender __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IStreamRender __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IStreamRender __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IStreamRender __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IStreamRender __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IStreamRender __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DeliverSample )( 
            IStreamRender __RPC_FAR * This,
            /* [in] */ LONG lSize,
            /* [in] */ LONGLONG llStartTime,
            /* [in] */ LONGLONG llStopTime);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetFrameSize )( 
            IStreamRender __RPC_FAR * This,
            /* [in] */ LONG lWidth,
            /* [in] */ LONG lHeight);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Start )( 
            IStreamRender __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AllocBuffer )( 
            IStreamRender __RPC_FAR * This,
            /* [in] */ LONG lFlags,
            /* [retval][out] */ LONG __RPC_FAR *lpBufferHandle);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Stop )( 
            IStreamRender __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Pause )( 
            IStreamRender __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddPopupMenu )( 
            IStreamRender __RPC_FAR * This,
            /* [in] */ BSTR szName,
            /* [retval][out] */ LONG __RPC_FAR *handle);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddMenu )( 
            IStreamRender __RPC_FAR * This,
            /* [in] */ LONG ParentHandle,
            /* [in] */ BSTR szName,
            /* [in] */ LONG hBitmap,
            /* [retval][out] */ LONG __RPC_FAR *handle);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetCaption )( 
            IStreamRender __RPC_FAR * This,
            /* [in] */ BSTR szCaption);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *StartSelectRange )( 
            IStreamRender __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EndSelectRange )( 
            IStreamRender __RPC_FAR * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSelectRange )( 
            IStreamRender __RPC_FAR * This,
            /* [out][in] */ LONG __RPC_FAR *top,
            /* [out][in] */ LONG __RPC_FAR *left,
            /* [out][in] */ LONG __RPC_FAR *bottom,
            /* [out][in] */ LONG __RPC_FAR *right);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DeleteSubMenu )( 
            IStreamRender __RPC_FAR * This,
            /* [in] */ LONG handle);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Window )( 
            IStreamRender __RPC_FAR * This,
            /* [retval][out] */ LONG __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetCallback )( 
            IStreamRender __RPC_FAR * This,
            /* [in] */ SR_CALLBACKTYPE type,
            /* [in] */ LONG callback,
            /* [in] */ LONG param);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DisableDefaultMenu )( 
            IStreamRender __RPC_FAR * This,
            DWORD dwFlag);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetRangeChangedMenuText )( 
            IStreamRender __RPC_FAR * This,
            BSTR szStartText,
            BSTR szEndText);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NegotiateFormat )( 
            IStreamRender __RPC_FAR * This,
            /* [in] */ SR_FORMAT __RPC_FAR *pPreferedFormatsSet,
            /* [in] */ LONG lPreferedFormatsNumber,
            /* [out][in] */ SR_FORMAT __RPC_FAR *pBestFormat,
            /* [retval][out] */ LONG __RPC_FAR *plPitch);
        
        END_INTERFACE
    } IStreamRenderVtbl;

    interface IStreamRender
    {
        CONST_VTBL struct IStreamRenderVtbl __RPC_FAR *lpVtbl;
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

#define IStreamRender_Start(This)	\
    (This)->lpVtbl -> Start(This)

#define IStreamRender_AllocBuffer(This,lFlags,lpBufferHandle)	\
    (This)->lpVtbl -> AllocBuffer(This,lFlags,lpBufferHandle)

#define IStreamRender_Stop(This)	\
    (This)->lpVtbl -> Stop(This)

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

#define IStreamRender_SetCallback(This,type,callback,param)	\
    (This)->lpVtbl -> SetCallback(This,type,callback,param)

#define IStreamRender_DisableDefaultMenu(This,dwFlag)	\
    (This)->lpVtbl -> DisableDefaultMenu(This,dwFlag)

#define IStreamRender_SetRangeChangedMenuText(This,szStartText,szEndText)	\
    (This)->lpVtbl -> SetRangeChangedMenuText(This,szStartText,szEndText)

#define IStreamRender_NegotiateFormat(This,pPreferedFormatsSet,lPreferedFormatsNumber,pBestFormat,plPitch)	\
    (This)->lpVtbl -> NegotiateFormat(This,pPreferedFormatsSet,lPreferedFormatsNumber,pBestFormat,plPitch)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IStreamRender_DeliverSample_Proxy( 
    IStreamRender __RPC_FAR * This,
    /* [in] */ LONG lSize,
    /* [in] */ LONGLONG llStartTime,
    /* [in] */ LONGLONG llStopTime);


void __RPC_STUB IStreamRender_DeliverSample_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IStreamRender_SetFrameSize_Proxy( 
    IStreamRender __RPC_FAR * This,
    /* [in] */ LONG lWidth,
    /* [in] */ LONG lHeight);


void __RPC_STUB IStreamRender_SetFrameSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IStreamRender_Start_Proxy( 
    IStreamRender __RPC_FAR * This);


void __RPC_STUB IStreamRender_Start_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IStreamRender_AllocBuffer_Proxy( 
    IStreamRender __RPC_FAR * This,
    /* [in] */ LONG lFlags,
    /* [retval][out] */ LONG __RPC_FAR *lpBufferHandle);


void __RPC_STUB IStreamRender_AllocBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IStreamRender_Stop_Proxy( 
    IStreamRender __RPC_FAR * This);


void __RPC_STUB IStreamRender_Stop_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IStreamRender_Pause_Proxy( 
    IStreamRender __RPC_FAR * This);


void __RPC_STUB IStreamRender_Pause_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IStreamRender_AddPopupMenu_Proxy( 
    IStreamRender __RPC_FAR * This,
    /* [in] */ BSTR szName,
    /* [retval][out] */ LONG __RPC_FAR *handle);


void __RPC_STUB IStreamRender_AddPopupMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IStreamRender_AddMenu_Proxy( 
    IStreamRender __RPC_FAR * This,
    /* [in] */ LONG ParentHandle,
    /* [in] */ BSTR szName,
    /* [in] */ LONG hBitmap,
    /* [retval][out] */ LONG __RPC_FAR *handle);


void __RPC_STUB IStreamRender_AddMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IStreamRender_SetCaption_Proxy( 
    IStreamRender __RPC_FAR * This,
    /* [in] */ BSTR szCaption);


void __RPC_STUB IStreamRender_SetCaption_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IStreamRender_StartSelectRange_Proxy( 
    IStreamRender __RPC_FAR * This);


void __RPC_STUB IStreamRender_StartSelectRange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IStreamRender_EndSelectRange_Proxy( 
    IStreamRender __RPC_FAR * This);


void __RPC_STUB IStreamRender_EndSelectRange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IStreamRender_GetSelectRange_Proxy( 
    IStreamRender __RPC_FAR * This,
    /* [out][in] */ LONG __RPC_FAR *top,
    /* [out][in] */ LONG __RPC_FAR *left,
    /* [out][in] */ LONG __RPC_FAR *bottom,
    /* [out][in] */ LONG __RPC_FAR *right);


void __RPC_STUB IStreamRender_GetSelectRange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IStreamRender_DeleteSubMenu_Proxy( 
    IStreamRender __RPC_FAR * This,
    /* [in] */ LONG handle);


void __RPC_STUB IStreamRender_DeleteSubMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IStreamRender_get_Window_Proxy( 
    IStreamRender __RPC_FAR * This,
    /* [retval][out] */ LONG __RPC_FAR *pVal);


void __RPC_STUB IStreamRender_get_Window_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IStreamRender_SetCallback_Proxy( 
    IStreamRender __RPC_FAR * This,
    /* [in] */ SR_CALLBACKTYPE type,
    /* [in] */ LONG callback,
    /* [in] */ LONG param);


void __RPC_STUB IStreamRender_SetCallback_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IStreamRender_DisableDefaultMenu_Proxy( 
    IStreamRender __RPC_FAR * This,
    DWORD dwFlag);


void __RPC_STUB IStreamRender_DisableDefaultMenu_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IStreamRender_SetRangeChangedMenuText_Proxy( 
    IStreamRender __RPC_FAR * This,
    BSTR szStartText,
    BSTR szEndText);


void __RPC_STUB IStreamRender_SetRangeChangedMenuText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IStreamRender_NegotiateFormat_Proxy( 
    IStreamRender __RPC_FAR * This,
    /* [in] */ SR_FORMAT __RPC_FAR *pPreferedFormatsSet,
    /* [in] */ LONG lPreferedFormatsNumber,
    /* [out][in] */ SR_FORMAT __RPC_FAR *pBestFormat,
    /* [retval][out] */ LONG __RPC_FAR *plPitch);


void __RPC_STUB IStreamRender_NegotiateFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IStreamRender_INTERFACE_DEFINED__ */



#ifndef __STREAMRENDERLib_LIBRARY_DEFINED__
#define __STREAMRENDERLib_LIBRARY_DEFINED__

/* library STREAMRENDERLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_STREAMRENDERLib;

EXTERN_C const CLSID CLSID_StreamRender;

#ifdef __cplusplus

class DECLSPEC_UUID("B1221B3B-DEBD-472e-9304-FEB2FC7D34F7")
StreamRender;
#endif
#endif /* __STREAMRENDERLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
