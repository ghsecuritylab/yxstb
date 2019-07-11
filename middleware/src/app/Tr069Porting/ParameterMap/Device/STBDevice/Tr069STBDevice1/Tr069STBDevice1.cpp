#include "Tr069STBDevice1.h"

#include "Tr069Capabilities.h"
#include "Tr069FunctionCall.h"


Tr069STBDevice1::Tr069STBDevice1()
	: Tr069GroupCall("1")
{

    Tr069Call* Capabilities           = new Tr069Capabilities();

    regist(Capabilities->name(), Capabilities);

}

Tr069STBDevice1::~Tr069STBDevice1()
{
}
