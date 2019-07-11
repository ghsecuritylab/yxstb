#include <iostream>

#include <hippo_module_config.h>
#include <Hippo_OS.h>
#include "Hippo_Debug.h"
#include "Hippo_Context.h"
#include "Hippo_PlayerHWBase.h"

#include "mid_stream.h"


namespace Hippo {
PlayerMgrHWBase::PlayerMgrHWBase( )
{
}
PlayerMgrHWBase::~PlayerMgrHWBase( )
{
}
Player* PlayerMgrHWBase::createPlayerInstance(int& aPlayerId,const char* name = NULL, HActiveObjectClient* actObj = NULL, void* browserHandler = NULL)
{
	return NULL;
}
int PlayerMgrHWBase::releasePlayerInstanceById(int aPlayerId)
{
	return 0;
}
int PlayerMgrHWBase::getPlayerInstanceNum( )
{
	return 0;
}

int PlayerMgrHWBase::addChannelNode(const chnl_node_type_e aChnlType, HString& str)
{
	return 0;
}
PlayerNode* PlayerMgrHWBase::getChannelNode(const chnl_node_type_e, int)
{
	return NULL;
}
int PlayerMgrHWBase::getPlayerEvent( )
{
	return 0;
}
int PlayerMgrHWBase::createChannelList(const chnl_node_type_e aChnlType, const int cnt)
{
	return 0;
}

PlayerHWBase::PlayerHWBase(int aPlayerId)
	:Player(aPlayerId)
	,m_curNode(0)
	,m_magic(0)
	,m_nextNode(NULL)
{
	HIPPO_DEBUG("run here.\n");
}

PlayerHWBase::~PlayerHWBase()
{
	HIPPO_DEBUG("run here.\n");
}

int PlayerHWBase::open()
{
	HIPPO_DEBUG("run here.\n");
	return 0;
}

int PlayerHWBase::play(int aStartTime, time_type_e aType)
{
	HIPPO_DEBUG("run here.aStartTime=%d, aType=%d\n", aStartTime, aType);

	if (m_bPlaylistMode) {
		if (m_playlist.size()) {
			m_curNode = *(m_playlist.begin());
			m_curNode->play(aStartTime, aType);
		}
		return 0;
	}
	if (this->m_curNode)
		m_curNode->play(aStartTime, aType);
	return 0;
}

int PlayerHWBase::fastForward(int speed, unsigned long aPlayTime, time_type_e)
{
	HIPPO_DEBUG( "run here.speed=%d\n", speed );
	if(this->m_curNode)
		m_curNode->fastForward(speed);
	return 0;
}

int PlayerHWBase::fastRewind(int speed, unsigned long aPlayTime, time_type_e)
{
	HIPPO_DEBUG("run here.\n");
	if(this->m_curNode)
		m_curNode->fastRewind(speed);
	return 0;
}

int PlayerHWBase::seekTo(const char* aPlayTime, Player::time_type_e aType)
{
	HIPPO_DEBUG("run here.\n");
	if( this->m_curNode )
		m_curNode->seekTo(aPlayTime, aType);
	return 0;
}

int PlayerHWBase::seekTo( unsigned long aPlayTime, time_type_e aType )
{
	HIPPO_DEBUG("run here.\n");

	HString fieldName;
	HString fieldValue;
	switch( aType ){
		case TimeType_eMediaStart:
			fieldName = "gotoStart";
			fieldValue = "0";
			HippoContext::getContextInstance()->ioctlWrite(fieldName, fieldValue);
			break;
		case TimeType_eMediaEnd:
			//ipanel_porting_program_gotoEnd();
			break;
		case TimeType_eNPT:
			play( aPlayTime, aType );
			break;
		default:
			break;
	}

	return 0;
}

int PlayerHWBase::pause()
{
	HIPPO_DEBUG("run here.\n");
	if(this->m_curNode)
		m_curNode->pause();
	return 0;
}

int PlayerHWBase::resume()
{
	HIPPO_DEBUG("run here.\n");
	if(this->m_curNode)
		m_curNode->resume();
	return 0;
}

int PlayerHWBase::stop()
{
	HIPPO_DEBUG("run here.curNode=%#x\n", (unsigned int)m_curNode);

	if(m_curNode)
		m_curNode->stop();
	Player::stop();
	if(m_curNode)
		delete m_curNode;
	m_curNode = 0;
	return 0;
}

int PlayerHWBase::close()
{
	HIPPO_DEBUG("run here.\n");
	if( m_curNode )
		delete m_curNode;
	m_curNode = 0;
	return 0;
}

int PlayerHWBase::getState()
{
	HIPPO_DEBUG("run here.\n");
	return -1;
}

int PlayerHWBase::getProperty(player_property_type_e aType, HPlayerProperty& aResult)
{
	HString fieldName;

	switch (aType) {
    case PlayerPropertyType_eCurrentPlayTime:
		aResult.m_PlayTime.m_TimeStamp = mid_stream_get_currenttime(0);
		break;
	case PlayerPropertyType_eCurrentPlayBackMode:
		//aResult.m_Value.m_intVal = this->m_curState;
		break;
	default:
		return Player::getProperty(aType, aResult);
	}
	HIPPO_DEBUG("run here. aType = %s\n", getProertyString(aType));
	return 0;
}

void PlayerHWBase::statecall(int pIndex, STRM_STATE state, int rate, unsigned int magic, int callarg)
{
	HIPPO_DEBUG("run here.index=%d, state=%d, rate=%d, magic=%p, callarg=%p\n", pIndex, state, rate, magic, callarg);

	if (callarg && this->m_magic == magic) {
		if(this->m_curNode) {
            return;
		}
	}
	return;
}

void PlayerHWBase::msgcall(int pIndex, STRM_MSG msg, int arg, unsigned int magic, int callarg)
{
	HIPPO_DEBUG("run here.index=%d, msg=%d, arg=%d, magic=%p, callarg=%p\n", pIndex, msg, arg, magic, callarg);

	if( callarg && m_magic == magic ){
	    return;
	}
	return;
}

} //end namespace

