/*-------------------------------------------------------------------------

  -------------------------------------------------------------------------*/
#ifndef _HIPPO_API_HH_
#define _HIPPO_API_HH_

/*-------------------------------------------------------------------------
 *  Hippo�ṩ��Ϣ�����ඨ��.
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
 * ����ΪHippo�ṩ���м����api,���ں��ϴ��뼯��.
 */
/*-------------------------------------------------------------------------*/
int a_HippoContext_Init( );

/***********************************************************/
/**
 * @param   ctx: �������,�����е�HippoContextʵ�����
 * @param   fieldName:  �������,ioctlWrite/Read�е�Param;
 * $param	fieldParam: ���ڷ�������,�˴�Ϊ�������;
 * @param   fieldValue: �������, ��ioctlWrite������ֵ, ��ioctlRead�����String.
 * @param   aResult: �������;
 					=0ʱΪִ��ioctlWrite,
 					>0��ֵΪioctlRead,�����䳤�ȱ�ʾΪfieldValue Buffer�ĳ���, ���ʱע��Null��β��.
 *                  <0��ֵδ����.
 * @return  jse_return_val_e
 * @remark ��
 * @see
 * @author teddy      @date 2011/01/21
 ************************************************************/
typedef enum tagJseReturnVal{
	JseReturnVal_eFailed = -1, //JSE ����ִ��ʧ��
	JseReturnVal_eOk = 0, //JSE ����ִ�гɹ�
	JseReturnVal_eIgnore, //JSE �����ܹ�ʶ��������,�������㴦��. ������������Channel��ע���JSE��������.
	JseReturnVal_eContinue, //JSE ������Ȼ������, ����ϣ����������������Channelע��ĺ���ִ��.
}jse_return_val_e;
typedef int (*JseIoctlFunc)( const char* aFieldName, const char* aFieldParam, char* aFieldValue, int aResult );

/***********************************************************/
/**
 * @param   ioName
 * @param   rfunc: ioctlRead�ص�����;
 * @param	wfunc: ioctlWrite�ص�����;
 *
 * @return ע��һ��ioctl����,Ŀǰ֧����������.���ע���ioName�Ѿ��������Ժ�ע��Ϊ׼.
 * @remark
 * @see
 * @author teddy      @date 2011/01/25
 ************************************************************/
typedef enum{
    IoctlContextType_Unknown,
    IoctlContextType_eAncestor, //���а汾���ɼ�
    IoctlContextType_eCTC, //������IPTV2.0�汾
    IoctlContextType_eHW20, //����Ϊ2.0�汾����, ��ָ��ΪC53֮ǰ�İ汾.
    IoctlContextType_eHWBase, //��Ϊ����͹��ڻ��߰汾����
    IoctlContextType_eHWBaseC10, //��Ϊ���ڰ汾����.
    IoctlContextType_eHWBaseC20	//��Ϊ����汾����.
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

