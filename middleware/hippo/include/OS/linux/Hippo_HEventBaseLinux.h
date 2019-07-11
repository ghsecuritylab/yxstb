#ifndef Hippo_HEventBaseLinux_h
#define Hippo_HEventBaseLinux_h

namespace Hippo{
class HEventBase {
    typedef enum{
        EventProperty_eNormal = 0,
        EventProperty_eCombined,
        EventProperty_eSync,
        EventProperty_eMax
    } event_property_e;
protected:
    HEventBase( );
    ~HEventBase( );

protected:
    int  m_eProperty; //event_property_e
};
}

#endif

