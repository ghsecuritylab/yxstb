

#include <unistd.h>

#include "tr069_api.h"


int main(int argc, char* argv[])
{
    tr069_api_init( );

    while (1)
        sleep(1);

    return 0;
}


void tr069_port_getValue(char *name, char *str, unsigned int *pval)
{
}

void tr069_port_setValue(char *name, char *str, unsigned int val)
{
}
