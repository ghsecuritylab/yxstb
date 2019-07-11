#include "Tr069LAN.h"

#include "Tr069Call.h"
#include "Tr069TCPSYNDiagnostics.h"

Tr069Call* g_cusLan = new Tr069LAN();

Tr069LAN::Tr069LAN()
	: Tr069GroupCall("LAN")
{
    Tr069Call* diag  = new Tr069TCPSYNDiagnostics();

    regist(diag->name(), diag);
}

Tr069LAN::~Tr069LAN()
{
}
