#ifndef IIPANEL_H
#define IIPANEL_H

#include "JseGlueMacro.h"

class IiPanel {
public:
    virtual int ioctlRead(const char*, char*) = 0;
    virtual int ioctlWrite(const char*, const char*) = 0;
    virtual int debug() = 0;
    virtual ~IiPanel() { }
};

extern IiPanel* createIiPanel(const char* name);

#endif //IIPANEL_H

