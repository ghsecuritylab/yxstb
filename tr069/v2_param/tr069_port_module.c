
#include "tr069_api.h"
#include "tr069_port.h"
#include "tr069_port_alarm.h"
#include "tr069_port_errorcode.h"
#include "tr069_port_sqm.h"

void tr069_port_moduleInit(void)
{
    tr069_port_alarmInit( );
    tr069_port_errorCodeInit( );
    tr069_port_sqmInit( );

    tr069_port_memoryStatusInit( );
    tr069_port_processStatusInit( );
}
