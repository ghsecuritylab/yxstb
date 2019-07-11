#include "XmppAssertions.h"
#include "LogModule.h"

static int s_XmppModuleFlag = 0;
int gXmppModuleLever = LOG_LEVEL_ERROR;

namespace gloox {
static LogModule XMPPQuickInstallModule("xmpp", s_XmppModuleFlag, gXmppModuleLever);
} //namespace gloox

