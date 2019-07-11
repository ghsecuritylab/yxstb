#ifndef Tr069PlayDiagnostics_h
#define Tr069PlayDiagnostics_h

#include "Tr069GroupCall.h"

#ifdef __cplusplus

/*------------------------------------------------------------------------------
 * 在CTC,CU,HUAWEI会统一调用的DiagnosticsState相关函数
 ------------------------------------------------------------------------------*/
extern Tr069Call* g_tr069PlayDiagnostics;

class Tr069PlayDiagnostics : public Tr069GroupCall {
public:
    Tr069PlayDiagnostics();
    ~Tr069PlayDiagnostics();
};

#define PLAY_DIANOSTICS_URL_LEN               1024

enum {
    DIAGNOSTATICS_STATE_NONE = 0,
    DIAGNOSTATICS_STATE_REQUESTED,
    DIAGNOSTATICS_STATE_COMPLETE,
    DIAGNOSTATICS_STATE_9822,
    DIAGNOSTATICS_STATE_9823,
    DIAGNOSTATICS_STATE_9824,
    DIAGNOSTATICS_STATE_9825,
    DIAGNOSTATICS_STATE_9826,
    DIAGNOSTATICS_STATE_9827
};
extern "C" 
{
void tr069_diagnostics_get_state(char *value, int size);
void tr069_diagnostics_set_state_by_enum(int d_state);
void tr069_diagnostics_set_state(char *value);
void tr069_diagnostics_task(int arg);

void tr069_play_start(void);
void tr069_get_PlayURL(char *value, int size);
void tr069_set_PlayURL(char *value);

void tr069_set_playurl_mode(int mode);
int tr069_get_playurl_mode(void);

}
#endif // __cplusplus

#endif // Tr069PlayDiagnostics_h
