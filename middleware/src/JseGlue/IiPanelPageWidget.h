#ifndef IIPANELPAGEWIDGET_H
#define IIPANELPAGEWIDGET_H

#include "JseGlueMacro.h"

class IiPanelPageWidget {
public:
    virtual void close() = 0;
    virtual void focus() = 0;
    virtual ~IiPanelPageWidget() { }
};

extern IiPanelPageWidget * createIiPanelPageWidget(const char* name);

#endif //IIPANELWIDGET_H

