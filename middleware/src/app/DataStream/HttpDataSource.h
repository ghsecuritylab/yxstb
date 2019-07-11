#ifndef _HttpDataSource_H_
#define _HttpDataSource_H_


#include <string>
#include <cstdlib>
#include "DataSource.h"
#ifdef __cplusplus

namespace Hippo {

class HttpDataSource : public DataSource {
public:
    HttpDataSource();
    ~HttpDataSource();

    virtual bool start();
    virtual bool stop();
	void setCookieInfo(char* cookieInfo) { mcookieInfo = cookieInfo; }
	void receiveData(int total, char *buf, int len);
	void receiveEnd();
	void receiveError();

private:
	std::string mcookieInfo;
};

} /* namespace Hippo */

#endif /* __cplusplus */

#endif /* _HttpDataSource_H_ */
