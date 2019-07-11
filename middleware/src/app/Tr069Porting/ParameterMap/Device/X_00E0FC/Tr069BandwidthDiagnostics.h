#ifndef Tr069BandwidthDiagnostics_h
#define Tr069BandwidthDiagnostics_h

#include "Tr069GroupCall.h"

#ifdef __cplusplus

class Tr069BandwidthDiagnostics : public Tr069GroupCall {
public:
    Tr069BandwidthDiagnostics();
    ~Tr069BandwidthDiagnostics();
};

#define BANDWIDTHDIAGNOSTICS_DOWNLOADURL_MAX 512
#define BANDWIDTHDIAGNOSTICS_USERNAME_MAX 128
#define BANDWIDTHDIAGNOSTICS_PASSWORD_MAX 128
#define BANDWIDTHDIAGNOSTICS_ERRORCODE_MAX 128

enum {
    None = 1,
    Requested,
    Complete,
    ErrorState,
};

extern "C"
{
void tr069_port_bandwidthDiagnostics_init(void);

void tr069_diagnostics_set_Speed(int, int, int);
void tr069_diagnostics_set_StateAndErrorCode(int, char*);

int tr069_diagnostics_set_value(char *, char *, unsigned int);
int tr069_diagnostics_get_value(char *, char *, unsigned int);

void tr069_diagnostics_succeed(char*);
}

#endif // __cplusplus

#endif // Tr069BandwidthDiagnostics_h
