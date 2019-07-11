#ifndef Tr069PacketCapture_h
#define Tr069PacketCapture_h

#include "Tr069GroupCall.h"

#ifdef __cplusplus

class Tr069PacketCapture : public Tr069GroupCall {
public:
    Tr069PacketCapture();
    ~Tr069PacketCapture();
};

extern "C"
{
int getTr069OpenPacketCaptureFlag();
void setTr069OpenPacketCaptureFlag(int flag);

}
#endif // __cplusplus

#endif // Tr069PacketCapture_h
