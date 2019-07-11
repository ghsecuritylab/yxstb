
#include "TR069Assertions.h"

#include "tr069_api.h"
#include "tr069_port_alarm.h"

#include <stdlib.h>
#include <string.h>

/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
int tr069_port_alarm_post(int type, char *code, int level, char *location)
{
    char buffer[64];

    sprintf(buffer, "AlarmPost.%d.%s.%d", type, code, level);
    tr069_api_setValue(buffer, location, 0);

    return 0;
}

/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
void tr069_port_alarm_clear(int type)
{
    char buffer[64];

    LogTr069Debug("#######: type = %d\n", type);

    sprintf(buffer, "AlarmClear.%d", type);
    tr069_api_setValue(buffer, "", 0);
}
