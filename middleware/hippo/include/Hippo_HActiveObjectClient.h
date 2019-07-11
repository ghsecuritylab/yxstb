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
* Hippo通过此接口将所有ActiveObject的事件导给外部使用.
* 目前只有1中场景:  在发生某个事件时回调浏览器的JS函数.
*/
class HActiveObjectClient {
public:
    HActiveObjectClient( ) {}
    virtual ~HActiveObjectClient( ) {}
    virtual void InitActiveObject( ) {}

    /***********************************************************/
    /**
    * @param   event: event事件
    * @param   jsonStr: 对应event事件的JSON描述格式.
    *
    * @return
    * @remark	此接口通常用于和Javascript通讯,
    *			因此提供事件的JSON格式.对于普通接口可以忽略第二个参数.
    * @see
    ************************************************************/
	virtual HEventDispatcher::event_status_e HandleEvent( HEvent* event, HString& param ) = 0 ;//{ return HEventDispatcher::EventStatus_eUnknown; }

};
}
#endif

