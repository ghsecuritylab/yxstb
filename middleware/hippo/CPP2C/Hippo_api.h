/*-------------------------------------------------------------------------

  -------------------------------------------------------------------------*/
#ifndef _HIPPO_API_HH_
#define _HIPPO_API_HH_

/*-------------------------------------------------------------------------
 *  Hippo提供消息处理类定义.
 -------------------------------------------------------------------------*/
typedef enum {
    EventType_eUnknown = 0x1000,		//invalid event type MUST BE zero.
    EventType_eKeyDown,
    EventType_eKeyReleased,
    EventType_ePlayerEvent,
    EventType_eUpgrade,
    EventType_eNetwork,
    EventType_eKeyText,
    EventType_eCommand,
    EventType_eUserEvent,

    EventType_eMax = 0x2000
} event_type_e;

typedef enum{
    EventProperty_eNormal = 0,
    EventProperty_eCombined,
    EventProperty_eSync,

} event_property_e;
typedef enum{
    EventStatus_eUnknown = 0,
    /* has been process, break up the event routing. */
    EventStatus_eProcessed,
    /* should continue routing. */
    EventStatus_eIgnored,
    EventStatus_eContinue,
} event_status_e;

#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------------*/
/**
 * 以下为Hippo提供给中间件的api,用于和老代码集成.
 */
/*-------------------------------------------------------------------------*/
int a_HippoContext_Init( );

/***********************************************************/
/**
 * @param   ctx: 输入参数,运行中的HippoContext实例句柄
 * @param   fieldName:  输入参数,ioctlWrite/Read中的Param;
 * $param	fieldParam: 对于符合命令,此处为命令参数;
 * @param   fieldValue: 输入参数, 对ioctlWrite是输入值, 对ioctlRead是输出String.
 * @param   aResult: 输入参数;
 					=0时为执行ioctlWrite,
 					>0的值为ioctlRead,并且其长度表示为fieldValue Buffer的长度, 输出时注意Null结尾符.
 *                  <0的值未定义.
 * @return  jse_return_val_e
 * @remark 无
 * @see
 * @author teddy      @date 2011/01/21
 ************************************************************/
typedef enum tagJseReturnVal{
	JseReturnVal_eFailed = -1, //JSE 函数执行失败
	JseReturnVal_eOk = 0, //JSE 函数执行成功
	JseReturnVal_eIgnore, //JSE 函数能够识别其内容,但不打算处理. 交给后面其他Channel中注册的JSE函数处理.
	JseReturnVal_eContinue, //JSE 函数虽然处理了, 但仍希望继续被后面其他Channel注册的函数执行.
}jse_return_val_e;
typedef int (*JseIoctlFunc)( const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult );

/***********************************************************/
/**
 * @param   ioName
 * @param   rfunc: ioctlRead回调函数;
 * @param	wfunc: ioctlWrite回调函数;
 *
 * @return 注册一个ioctl操作,目前支持以下类型.如果注册的ioName已经存在则以后注册为准.
 * @remark
 * @see
 * @author teddy      @date 2011/01/25
 ************************************************************/
typedef enum{
    IoctlContextType_Unknown,
    IoctlContextType_eAncestor, //所有版本都可见
    IoctlContextType_eCTC, //仅电信IPTV2.0版本
    IoctlContextType_eHW20, //仅华为2.0版本可以, 特指华为C53之前的版本.
    IoctlContextType_eHWBase, //华为海外和国内基线版本公用
    IoctlContextType_eHWBaseC10, //华为国内版本特有.
    IoctlContextType_eHWBaseC20	//华为海外版本特有.
} ioctl_context_type_e;

int a_Hippo_API_JseRegister( const char* ioName, JseIoctlFunc rfunc, JseIoctlFunc wfunc, ioctl_context_type_e eChnl );
int a_Hippo_API_UnJseRegister( const char* ioName, ioctl_context_type_e eChnl );



static inline int is_ioctlWrite( int aResult )
{
	//return Hippo_Context::s_ioctlWriteFlag == aResult;
	return 0 == aResult;
};

#ifdef __cplusplus
}
#endif

#endif

