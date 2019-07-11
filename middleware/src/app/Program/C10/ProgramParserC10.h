#ifndef _ProgramParserC10_H_
#define _ProgramParserC10_H_

#include "ProgramParser.h"

#ifdef __cplusplus

namespace Hippo {

class ProgramParserC10 : public ProgramParser {
public:
    ProgramParserC10();
    ~ProgramParserC10();

    virtual Program *parseSingleChannel(const char *);

};

} // namespace Hippo

#endif // __cplusplus

#endif // _ProgramParserC10_H_
