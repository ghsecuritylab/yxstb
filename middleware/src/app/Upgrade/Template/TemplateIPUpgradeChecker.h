#ifndef _TemplateIPUpgradeChecker_H_
#define _TemplateIPUpgradeChecker_H_

#include "UpgradeChecker.h"

#ifdef __cplusplus

namespace Hippo {

class UpgradeManager;
class HttpDataSource;

class TemplateIPUpgradeChecker : public UpgradeChecker {	
public:
    TemplateIPUpgradeChecker(UpgradeManager*);
    ~TemplateIPUpgradeChecker();	
    
    virtual bool start();
    virtual bool stop();
protected:
	virtual void handleMessage(Message *msg) { return; } 
private:
    HttpDataSource* m_httpDataSource;
    
};

}

#endif

#endif //_TemplateIPUpgradeChecker_H_




