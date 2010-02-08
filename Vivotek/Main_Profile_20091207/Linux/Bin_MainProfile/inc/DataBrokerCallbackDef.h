
#ifndef _DATABROKER2_CALBACKDEF_H_
#define _DATABROKER2_CALBACKDEF_H_

#include "datapacketdef.h"

#ifdef _BOOST
#include "boost/function.hpp"
#endif

/*! This enumeration indicates the status of DataBroker */
typedef enum {
	/*! Indicate connection info when connecting to the server */
	eOnConnectionInfo,
	/*! Can not pass the authorization of server */
	eOnAuthFailed,
	/*! Begin to receive media stream */
	eOnStartMediaChannel,
	/*! Audio or Video channel closed */
	eOnChannelClosed,
	/*! Audio or Video channel receives data timeout */
	eOnTimeout,
	/*! The protocol of receiving media changed */
	eOnProtocolChanged,
	/*! Packet loss */
	eOnPacketLoss,
	//	eOnDiAlert,
	/*! Receiving the digital input alert and digital output status */
	eOnDiDo,
	/*! Detecting the change of location */
	eOnLocationChanged,
	/*! Indicate the width and height of the image when using Input to unpacketize and parse the receiving packets */
	eOnInputInfo,
	/*! Other error occurs */
	eOnOtherError,
	/*! The Connection stopped */
	eOnStopped,
	/*! Notify the audio mode set on server. This notification is only available when connecting to 4000/5000/6000 series servers. The pvParam1 contains the integer value that indicates the audio mod. Please refer to TMediaAudioMode. This status is also notified when someone changes the server audio mode. So it is not subjected to called when connecting */
	eOnAudioMode,
	/*! Notify that due to server settings or users¡¯ permission, the media is changed. This notification is only available when connecting to 4000/5000/6000 series servers. The pvParam1 contains reason. Please refer to TMediaChangeReason. */
	eOnChangeMediaType,
	/*! This status code is similar to eOnChangeMediaType. But when users get this notification, it means only control channel is established. In such case, no audio or video data would be available */
	eOnAudioDisabled,
	/*! When users start talk and DataBroker finds that the talk-channel is already used by other user, this status code will be sent to users by callback. This notification is only available when connecting to 4000/5000/6000 series servers */
	eOnAudioUpstreamOccupied,
	/*! Notify the privilege for current user. This notification is only available when connecting to 4000/5000/6000 series servers. The privilege type is a ORed double word of the privilege define in TUserPrivilege. */
	eOnGetPrivilege,
	/*! Notify the Talk connection is established. This notification is only available when connecting to 4000/5000/6000 series servers */
	eOnTxChannelStart,
	/*! Notify the control channel has been closed */
	eOnTxChannelClosed,
	/*! Notify the control channel has been closed */
	eOnControlChannelClosed,
	/*! Notify that there is no signal for video input of Video server model. The parameter would either be signal lost or signal restored */
	eOnVideoSignalChange,
	/*! Notify that the server is now serving 10 streaming clients and no more new client could request for streaming now unless one of the previous connection closed */
	eOnServiceUnavailable,
	/*! Notify that the upstream channel is turned off by server. The parameter is the new audio mode that server is set currently */
	eOnAudioUpstreamDisabled,

	// for RTSP
	/*! This is the notification for RTSP server¡¯s playing range for media. This is only made if the requested media is file based. For live streaming, there will not be such status notified */
	eOnMediaRange,

	/*! This is the MP4 CI value for RTSP, pvParam1 is the CI starting address, pvParam2 is the length for CI */
	eOnMP4VConfig,

	/*! This is the AAC info value for RTSP, pvParam1 is the DWORD value for the type value */
	eOnMP4AConfig,

	/*! This is the GAMR info value for RTSP, pvParam1 is the sample rate */
	eOnGAMRConfig,
  
	/*! This is the H264 info value for RTSP, pvParam1 is the TDataBrokerH264Info pointer address */
	eOnH264Config,

	eOnConnectionOptionError,

	// perkins 2006/11/1 proxy authentication failed
	eOnProxyAuthFailed,

	//steven 2007/03/30 for dual stream. pvParam1: 1 indicates the connection type is 7k,  pvParam1: 2 indicates the connection type is 2k
	eOnConnectionType,

	//steven 2007/05/07 If user has connected to camera and camera does not send frames, DataBroker will callback this status
	eOnConnectionTimeout,

	eOnTxChannelAuthFail,
	/*! Notify the control channel autenication fail */
} TDataBrokerStatusType;

#ifdef _BOOST

typedef boost::function<SCODE (DWORD dwContext, TMediaDataPacketInfo *pMediaDataPacket)> TDataBrokerAVCallback;
typedef boost::function<SCODE (DWORD dwContext, TDataBrokerStatusType tStatusType, PVOID pvParam1, PVOID pvParam2)> TDataBrokerStatusCallback;
typedef boost::function<SCODE (DWORD dwContext, BYTE **ppbyDataBuffer, DWORD *pdwLen, DWORD *pdwDataTimePeriod)> TDataBrokerTxCallback;

#else

typedef SCODE (__stdcall *TDataBrokerAVCallback)(DWORD dwContext, TMediaDataPacketInfo *pMediaDataPacket);

typedef SCODE (__stdcall *TDataBrokerAVCallbackV3)(DWORD dwContext, TMediaDataPacketInfoV3 *pMediaDataPacket);

typedef SCODE (__stdcall *TDataBrokerStatusCallback)(DWORD dwContext, TDataBrokerStatusType tStatusType, PVOID pvParam1, PVOID pvParam2);

typedef SCODE (__stdcall *TDataBrokerTxCallback)(DWORD dwContext, BYTE **ppbyDataBuffer, DWORD *pdwLen, DWORD *pdwDataTimePeriod);

#endif // BOOST

typedef SCODE (__stdcall *TDataBrokerNetPacketCallback)(DWORD dwContext, DWORD dwMediaType, DWORD dwLen, BYTE *pbyPacket);


#endif // _DATABROKER2_CALBACKDEF_H_

