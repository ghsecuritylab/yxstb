#include "Tr069STBDevice.h"

#include "Tr069STBDevice1/Tr069STBDevice1.h"
#include "Tr069FunctionCall.h"


Tr069STBDevice::Tr069STBDevice()
	: Tr069GroupCall("STBDevice")
{

    Tr069Call* STBDevice1           = new Tr069STBDevice1();

    regist(STBDevice1->name(), STBDevice1);

}

Tr069STBDevice::~Tr069STBDevice()
{
}
