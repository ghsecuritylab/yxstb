#ifndef XmppAssertions_h
#define XmppAssertions_h

#include "Assertions.h"

#ifdef __cplusplus

extern int gXmppModuleLever;

namespace gloox {
#define xmppLogError(args...)   LOG_ERROR(HLG_OPERATION, gXmppModuleLever, args)
#define xmppLogWarning(args...) LOG_WARNING(HLG_OPERATION, gXmppModuleLever, args)
#define xmppLogDebug(args...)   LOG(HLG_OPERATION, gXmppModuleLever, args)
} //namespace gloox

#endif //__cplusplus

#endif //XmppAssertions_h
