#ifndef _ISIL_SDK_ERROR_DEFINE_H_
#define _ISIL_SDK_ERROR_DEFINE_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _eISIL_ERR_CODE
{
	ISIL_ERR_SUCCESS 			=       0 , //没有错误
	ISIL_ERR_FAILURE             =      -1,	//操作失败
	ISIL_ERR_PASSWORD_ERROR      =    -1000, //用户名密码错误
	ISIL_ERR_NOENOUGHPRI,  			        //权限不足
	ISIL_ERR_NOINIT, 					    //没有初始化
	ISIL_ERR_INITERROR,                      //初始化失败
	ISIL_ERR_NOLOGIN,                        //没有登陆
	ISIL_ERR_CHANNEL_ERROR, 			        //通道号错误
	ISIL_ERR_OVER_MAXLINK, 			        //连接到DVR的客户端个数超过最大
	ISIL_ERR_VERSIONNOMATCH,			        //版本不匹配
	ISIL_ERR_NETWORK_FAIL_CREATESOCKET,      //创建socket失败
	ISIL_ERR_NETWORK_FAIL_CONNECT,	        //连接服务器失败
	ISIL_ERR_NETWORK_SEND_ERROR,		        //向服务器发送失败
	ISIL_ERR_NETWORK_RECV_ERROR,		        //从服务器接收数据失败
	ISIL_ERR_NETWORK_RECV_TIMEOUT,	        //从服务器接收数据超时
	ISIL_ERR_NETWORK_ERRORDATA,		        //传送的数据有误
	ISIL_ERR_CREATEBUFFER_ERROR,             //内存分配失败
	ISIL_ERR_ORDER_ERROR,				    //调用次序错误
	ISIL_ERR_OPERNOPERMIT,			        //无此权限
	ISIL_ERR_COMMANDTIMEOUT,			        //DVR命令执行超时
	ISIL_ERR_ERRORSERIALPORT,			    //串口号错误
	ISIL_ERR_ERRORALARMPORT,			        //报警端口错误
	ISIL_ERR_PARAMETER_ERROR, 		        //参数错误
	ISIL_ERR_PARAMETERBUFFERTOOSMALL, 		//接收参数缓冲区太小
	ISIL_ERR_CHAN_EXCEPTION,				    //服务器通道处于错误状态
	ISIL_ERR_NODISK,						    //没有硬盘
	ISIL_ERR_ERRORDISKNUM,				    //硬盘号错误
	ISIL_ERR_DISK_FULL,					    //服务器硬盘满
	ISIL_ERR_DISK_ERROR,					    //服务器硬盘出错
	ISIL_ERR_NOSUPPORT,					    //服务器不支持
	ISIL_ERR_BUSY,						    //服务器忙
	ISIL_ERR_MODIFY_FAIL,					//服务器修改不成功
	ISIL_ERR_PASSWORD_FORMAT_ERROR,		    //密码输入格式不正确
	ISIL_ERR_DISK_FORMATING,				    //硬盘正在格式化，不能启动操作
	ISIL_ERR_DVRNORESOURCE,				    //DVR资源不足
	ISIL_ERR_DVROPRATEFAILED,			    //DVR操作失败
	ISIL_ERR_OPENHOSTSOUND_FAIL, 		    //打开PC声音失败
	ISIL_ERR_DVRVOICEOPENED, 			    //服务器语音对讲被占用
	ISIL_ERR_TIMEINPUTERROR,			        //时间输入不正确
	ISIL_ERR_NOSPECFILE,				        //回放时服务器没有指定的文件
	ISIL_ERR_DIR_ERROR,					    //路径错误
	ISIL_ERR_ALLOC_RESOUCE_ERROR,		    //资源分配错误
	ISIL_ERR_AUDIO_MODE_ERROR,			    //声卡模式错误
	ISIL_ERR_NOENOUGH_BUF,				    //缓冲区太小
	ISIL_ERR_CREATESOCKET_ERROR,			    //创建SOCKET出错
	ISIL_ERR_SETSOCKET_ERROR,				//设置SOCKET出错
	ISIL_ERR_MAX_NUM,						//个数达到最大
	ISIL_ERR_USERNOTEXIST,				    //用户不存在
	ISIL_ERR_WRITEFLASHERROR,			    //写FLASH出错
	ISIL_ERR_UPGRADEFAIL,				    //DVR升级失败
	ISIL_ERR_CARDHAVEINIT,			        //解码卡已经初始化过
	ISIL_ERR_PLAYERFAILED,				    //播放器中错误
	ISIL_ERR_MAX_USERNUM,			        //用户数达到最大
	ISIL_ERR_GETLOCALIPANDMACFAIL,	        //获得客户端的IP地址或物理地址失败
	ISIL_ERR_NOENCODEING,					//该通道没有编码
	ISIL_ERR_IPMISMATCH,					    //IP地址不匹配
	ISIL_ERR_MACMISMATCH,					//MAC地址不匹配
	ISIL_ERR_UPGRADELANGMISMATCH,		    //升级文件语言不匹配
	ISIL_ERR_TALKING,                        //当前设备正在对讲中...

	ISIL_ERR_FILE_CREATE        = -1200,		//创建文件出错
	ISIL_ERR_FILE_WRITE,                     //文件写错误
	ISIL_ERR_FILE_OPEN,			            //打开文件出错
	ISIL_ERR_FILE_READ,                      //文件读错误
	ISIL_ERR_FILE_CREATEINDEX,               //文件建索引错误
	ISIL_ERR_FILE_OPERATE,                   //操作错误
	ISIL_ERR_FILE_GETPLAYTIME,			    //获取当前播放的时间出错
	ISIL_ERR_FILE_FORMAT,		            //文件格式不正确
	ISIL_ERR_FILE_PLAY,				        //播放出错
	ISIL_ERR_FILE_READDATA_OVERSIZE,         //取文件数据块过大，不应大于64M
	ISIL_ERR_FILE_BEGIN,                     //文件头
 	ISIL_ERR_FILE_END,                       //文件尾
	ISIL_ERR_FILE_REMAIN_FRAME,              //还有余留帧，应当继续调用getnextframe,并且当前取得数据是不显示，主要用在帧退时
    ISIL_ERR_FILE_RULE_END,
	ISIL_ERR_FILE_NOT_SETRULE,

	// Record
	ISIL_ERR_FrameParam_ID     = -1300,		//录像ID参数值过小或过大
	ISIL_ERR_CreateConnection,				//初始化ADO失败,CreateInstance Connection
	ISIL_ERR_CreateRecordset,				//初始化ADO失败,CreateInstance Recordset

    ISIL_ERR_DEV_FAILURE       = -2000,	    //针对设备端的操作失败

}ISILERR_CODE;

#ifdef __cplusplus
}
#endif

#endif