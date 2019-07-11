#ifndef __HIPPO_HBrowserActiveObj_H
#define __HIPPO_HBrowserActiveObj_H

#include "base/Hippo_HEvent.h"
//#include "base/Hippo_HEventDispatcher.h"

namespace Hippo{
class HEventDispatcher;
class HBrowserActiveObj : public HActiveObject{
typedef enum{
	ActiveOjbType_eUnknown,
	ActiveOjbType_eWidget = 1,
	ActiveOjbType_eBrowser
} activeObj_type_e;
public:
	static HBrowserActiveObj* createBrowserObj( );

	~HBrowserActiveObj( );

	HBrowserActiveObj* createWidget( );

	void setCurrentActiveObj( const HBrowserActiveObj* aObj );
    void setDefaultActiveObj( const HBrowserActiveObj* aObj );

    HActiveObject* getCurrentActiveObj( );
    void releaseCurrentActiveObj( );
	virtual HEventDispatcher::event_status_e HandleEvent( HEvent& event );
protected:
	HBrowserActiveObj( );
private:
	void InitWidget( );
	void InitBrowser( );

protected:
	//��������,
	activeObj_type_e mType; //widget or Browser.

	//����ΪBrowser����ʱʹ��.
    HEventDispatcher* m_dispatcher; //Browserʱ������widget���ɷ���Ϣ;

};


}

#endif

