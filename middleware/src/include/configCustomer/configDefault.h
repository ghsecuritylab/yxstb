#ifndef _configDefault_H_
#define _configDefault_H_


#include "default/configDefaultApp.h"
#include "configCustomer/default/configDefaultNetwork.h"
#include "default/configDefaultPlayer.h"
#include "default/configDefaultUpgrade.h"

#ifdef INCLUDE_DLNA
#include "default/configDefaultDlna.h"
#endif
#ifdef INCLUDE_DVBS
#include "default/configDefaultDvb.h"
#endif
#if defined(INCLUDE_TR069) || defined(INCLUDE_HMWMGMT)
#include "default/configDefaultTr069.h"
#endif
#ifdef PAY_SHELL
#include "default/configDefaultPayShell.h"
#endif

#endif // _configDefault_H_
