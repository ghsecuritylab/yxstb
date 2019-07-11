#ifndef _ProgramParser_H_
#define _ProgramParser_H_

#include "json/json_public.h"
#include "VodSource.h"
#include <vector>
#include <stdlib.h>
#include <string>

#ifdef __cplusplus

namespace Hippo {

class Program;
class ProgramList;

class ProgramParser {
    typedef enum{
            TokenState_eBeforeKey,
            TokenState_eInKey,
            TokenState_eAfterKey,
            TokenState_eInTag,
            TokenState_eBeforeValue,
            TokenState_eInValue,
            TokenState_eIgnore,
        }token_state_e;

public:
    static bool m_parseChannellistOK;
    ProgramParser();
    ~ProgramParser();

    virtual Program *parseSingleMedia(const char *);
    virtual int parseMediaList(ProgramList *, const char *);

    virtual Program *parseSingleChannel(const char *);
    virtual int parseChannelList(ProgramList *, const char *);

    VodSource* getVodSourceByJsonObj(struct json_object *object);
    VodSource* getVodSourceByStr(std::string& sDiscription);

    static int tokenize(std::string& str, /*out*/ std::vector<std::string>& aKey, /*out*/ std::vector<std::string>& aValue, int aTag = '=');
};

ProgramParser &programParser();

} // namespace Hippo

#endif // __cplusplus

#endif // _ProgramParser_H_
