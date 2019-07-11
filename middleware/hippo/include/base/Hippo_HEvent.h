#ifndef __HIPPO_EVNET_H
#define __HIPPO_EVNET_H

#include "Hippo_OS.h"
#include "Hippo_HString.h"
#include "Hippo_HActiveObjectClient.h"
#include "base/Hippo_HEventDispatcher.h"

namespace Hippo{

class HActiveObjectClient;

class HEvent : public HEventBase {
public:
    /* Ϊ���ֺ�C27�м���������, �¼�����ȫ��Ϊ��ֵ. */
    typedef enum {
        EventType_eUnknown,     //invalid event type MUST BE zero.
        EventType_eKeyReleased,
        EventType_ePlayerEvent,
        EventType_eUpgrade,
        EventType_eNetwork,
        EventType_eKeyText,
        EventType_eCommand,
        EventType_eUserEvent,

        EventType_eMax = -1
    } event_type_e;

public:
	typedef HEventDispatcher::event_status_e (*EventHanderFunc )( const HEvent*, void* );

    unsigned char       m_eType; //event_type_e
    unsigned char       m_eStatue; //event_status_e
    unsigned char       m_iPriority; //priority of this message.
    int m_wParam;
	int m_lParam;
    EventHanderFunc     m_cbHander;
    void*               m_pData; //parameter for m_cbHandler;

	/***********************************************************/
	/** �趨��Event�Ľ���Target,
	 *һ���¼���Target����ΪNULL,
	 *	ΪNULLʱ����HEventDispatcher�����������������¼����͸�ĳ��HActiveObject.
	 * @param   target
	 *
	 * @return
	 * @remark ��
	 * @see
	 * @author teddy      @date 2011-1-17 16:41:06
	 ************************************************************/
	void setTarget( HActiveObject* target ) { m_target = target;}
    HEventDispatcher::event_status_e handleBySelf( );
	void code( long code ){ m_code = code; }
	long code( ) const { return m_code; }
	void value( HString& value ){ m_value = value; }
	HString& value( ) { return m_value; }
private:
	HActiveObject* m_target;
	long m_code; //an int value describing this event. usually used in talking to JS.
	HString m_value; //an string describing this event. usually used in talking to JS.
};

class HActiveObject {
public:
	HActiveObject( HActiveObject* parent=NULL );
	virtual ~HActiveObject( ) {}

	virtual void InitActiveObject( ) {}
	virtual HEventDispatcher::event_status_e HandleEvent( HEvent& event );

protected:

	HActiveObjectClient* m_client;
	HActiveObject* parent_;
};

}
#endif

