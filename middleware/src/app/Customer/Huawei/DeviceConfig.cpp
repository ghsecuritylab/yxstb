#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "Resource.h"
#include "ResourceUser.h"
#include "ResourceManager.h"

#include "NetworkDevice.h"
#include "TunerDevice.h"
#include "Concurrent.h"

#include "Hippo_api.h"


Hippo::NetworkDevice *gNetworkDeviceConfig = NULL;
static Hippo::TunerDevice  *gTunerDevice = NULL;
Hippo::Concurrent  *gConcurrentConfig = NULL;

extern "C" int
DeviceConfig()
{
    gNetworkDeviceConfig = new Hippo::NetworkDevice(10.0);
    Hippo::resourceManager().addNetworkResource(gNetworkDeviceConfig);

    gTunerDevice = new Hippo::TunerDevice(1);
    Hippo::resourceManager().addTunerResource(gTunerDevice);

	gConcurrentConfig = new Hippo::Concurrent(3);
    Hippo::resourceManager().addConcurrentResource(gConcurrentConfig);

    return 0;
}


