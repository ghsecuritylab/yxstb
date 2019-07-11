#ifndef IIPANELPAGEWIDGETCELL_H
#define IIPANELPAGEWIDGETCELL_H

#include "Hippo_HBrowserActiveObj.h"
#include "IiPanelPageWidget.h"

class IiPanelPageWidgetCell : public IiPanelPageWidget {
public:
    virtual void close();
    virtual void focus();

    ~IiPanelPageWidgetCell();
protected:
    IiPanelPageWidgetCell();
    IiPanelPageWidgetCell(const IiPanelPageWidgetCell&);
    IiPanelPageWidgetCell& operator = (const IiPanelPageWidgetCell&);
    
    Hippo::HBrowserActiveObj* m_impl;
    Hippo::HBrowserActiveObj* m_mainObj;

    friend IiPanelPageWidget* createIiPanelPageWidget(const char*);
};

#endif //IIPANELPAGEWIDGETSERVER_H

