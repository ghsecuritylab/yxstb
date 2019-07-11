#ifndef _TRUNK_EC2108_C27_IPSTB_SRC_APP_PROGRAM_C20_ProgramParserC20_H_
#define _TRUNK_EC2108_C27_IPSTB_SRC_APP_PROGRAM_C20_ProgramParserC20_H_

#include <string.h>
#include "ProgramParser.h"
#include "VodSource.h"
#include "Program.h"
#include "ProgramChannelC20.h"
#ifdef __cplusplus

namespace Hippo {

class ProgramParserC20 : public ProgramParser {
	typedef enum{
	    	TokenState_eBeforeKey,
	    	TokenState_eInKey,
	    	TokenState_eInTag,
	    	TokenState_eInValue,
	    	TokenState_eIgnore,
	}token_state_e;
public:
	ProgramParserC20();
	~ProgramParserC20();

	virtual Program *parseSingleMedia(const char *);
	virtual Program *playurlParse(std::string& sUrl);
	virtual int parseChannelList(ProgramList *, const char *);

	int searchNodeFromProgramList(VodSource *source, Program **program);
	int setOpenSqaCode(ProgramChannelC20*pNode);
	int ChannelListTokenize( /*in*/ std::string& store, /*out*/ std::vector<std::string>& line);
	void upgradeProgram(ProgramChannelC20 *);
};

} // namespace Hippo

#endif // __cplusplus

#endif // _ProgramParserC20_H_
