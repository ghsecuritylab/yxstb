/*-------------------------------------------------------------------------

-------------------------------------------------------------------------*/
#ifndef MIDDLEWARE_HActiveObjectClient_H
#define MIDDLEWARE_HActiveObjectClient_H

#include "Hippo_HString.h"
#include "base/Hippo_HEvent.h"
#include "base/Hippo_HEventDispatcher.h"
namespace Hippo{
class HEvent;
/**
* Hippoͨ���˽ӿڽ�����ActiveObject���¼������ⲿʹ��.
* Ŀǰֻ��1�г���:  �ڷ���ĳ���¼�ʱ�ص��������JS����.
*/
class HActiveObjectClient {
public:
    HActiveObjectClient( ) {}
    virtual ~HActiveObjectClient( ) {}
    virtual void InitActiveObject( ) {}

    /***********************************************************/
    /**
    * @param   event: event�¼�
    * @param   jsonStr: ��Ӧevent�¼���JSON������ʽ.
    *
    * @return
    * @remark	�˽ӿ�ͨ�����ں�JavascriptͨѶ,
    *			����ṩ�¼���JSON��ʽ.������ͨ�ӿڿ��Ժ��Եڶ�������.
    * @see
    ************************************************************/
	virtual HEventDispatcher::event_status_e HandleEvent( HEvent* event, HString& param ) = 0 ;//{ return HEventDispatcher::EventStatus_eUnknown; }

};
}
#endif

