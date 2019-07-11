#ifndef _SubsectionCall_H_
#define _SubsectionCall_H_


#include <string>
#include <map>

#ifdef __cplusplus

namespace Hippo {

class SubsectionCall {
public:
    SubsectionCall();
    ~SubsectionCall();

    virtual const char *subsection();

    int registerNextSubsection(SubsectionCall *sub);
    int unregisterNextSubsection(SubsectionCall *sub);

    virtual int collisionReport(const char *subsections);

    virtual int call(const char *subsections);

private:
    std::map<std::string, SubsectionCall *> mSubsectionMap;
};

} // namespace Hippo

#endif // __cplusplus

#endif // _SubsectionCall_H_
