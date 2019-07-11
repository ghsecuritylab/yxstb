#ifndef IIPANELSERVERCELL_H
#define IIPANELSERVERCELL_H

#include "IiPanel.h"

class IiPanelCell : public IiPanel {
public:
    virtual int ioctlRead(const char*, char*);
    virtual int ioctlWrite(const char*, const char*);
    virtual int debug();

    ~IiPanelCell();

protected:
    IiPanelCell();
    IiPanelCell(const IiPanelCell&);
    IiPanelCell& operator = (const IiPanelCell&);

    friend IiPanel* createIiPanel(const char*);
};

#endif //IIPANELSERVER_H

