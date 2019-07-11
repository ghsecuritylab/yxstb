
#include "ProgramVOD.h"

#include "ProgramAssertions.h"

using namespace std;
namespace Hippo {

ProgramVOD::ProgramVOD()
{
}

ProgramVOD::~ProgramVOD()
{
	vector<VodSource*>::iterator iit;
	iit = m_advlist.begin();
	for(; iit < m_advlist.end(); iit++){
		delete *iit;
	}
}

int
ProgramVOD::getNumberID()
{
    return -1;
}

const char *
ProgramVOD::getStringID()
{
    VodSource *source = m_advlist[0];
    if (!source)
        return 0;
    return (source->GetEntryID()).c_str();
}

int ProgramVOD::addVODSource(VodSource *pNode)
{
	m_advlist.push_back(pNode);

	return 0;
}

VodSource* ProgramVOD::getVodSource( int index )
{
	PROGRAM_LOG("ProgramVOD::getVodSource by index(%d)\n", index);
	if(index >= getVodSourceCount())
		return NULL;
	return m_advlist.at(index);
}

int ProgramVOD::getVodSourceCount()
{
	return m_advlist.size();
}

} // namespace Hippo
