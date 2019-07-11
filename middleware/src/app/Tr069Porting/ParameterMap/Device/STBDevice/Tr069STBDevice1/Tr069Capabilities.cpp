#include "Tr069Capabilities.h"

#include "Tr069FunctionCall.h"
#include "AppSetting.h"

#include <string.h>

#ifdef TR069_LIANCHUANG

static int getAppTr069PortCompositeVideoStandard(char* str, unsigned int val)
{
    app_tr069_port_get_CompositeVideoStandard(str, val);

    return 0;
}

static int setAppTr069PortCompositeVideoStandard(char* str, unsigned int val)
{
    app_tr069_port_set_CompositeVideoStandard(str);

    return 0;
}

static int getAppTr069PortAspectRatio(char* str, unsigned int val)
{
    if(str) {
		int aspectRatio = 0;
        appSettingGetInt("hd_aspect_mode", &aspectRatio, 0);
        if (0 == aspectRatio) {
           strcpy(str, "4:3");
        }else
            strcpy(str, "16:9");
    }
    return 0;
}

static int setAppTr069PortAspectRatio(char* str, unsigned int val)
{
    if(str) {
        if (!strncmp(str, "4:3", 3)) {
            appSettingSetInt("hd_aspect_mode", 0);
        }else
            appSettingSetInt("hd_aspect_mode", 2);
    }
    return 0;
}

#endif

Tr069Capabilities::Tr069Capabilities()
	: Tr069GroupCall("Capabilities")
{
#ifdef TR069_LIANCHUANG

    Tr069Call* CompositeVideoStandard = new Tr069FunctionCall("CompositeVideoStandard", getAppTr069PortCompositeVideoStandard, setAppTr069PortCompositeVideoStandard);
    Tr069Call* AspectRatio            = new Tr069FunctionCall("AspectRatio", getAppTr069PortAspectRatio, setAppTr069PortAspectRatio);


    regist(CompositeVideoStandard->name(), CompositeVideoStandard);
    regist(AspectRatio->name(), AspectRatio);

#endif

}

Tr069Capabilities::~Tr069Capabilities()
{
}
