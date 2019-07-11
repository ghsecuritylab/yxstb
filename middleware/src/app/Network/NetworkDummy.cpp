#include "Assertions.h"
#include "app_sys.h"
#include <string.h>
#include <stdio.h>

extern "C" int network_init(); 
extern "C" void mid_dns_init(void);

extern "C" {
    int mid_net_init(void)
    {
        network_init();
        mid_dns_init();
        return 0;
    }
}
