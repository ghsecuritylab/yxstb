#include "Hippo_Debug.h"
#include "Hippo_MediaPlayer.h"

namespace Hippo {

Player::HPlayerProperty::HPlayerProperty( )
{
	m_eType = Player::PlayerPropertyType_eUnknown;
	m_Value.m_intVal = -1;
}

MediaPlayerMgr::MediaPlayerMgr( )
{
}
MediaPlayerMgr::~MediaPlayerMgr( )
{
}
Player*
MediaPlayerMgr::createPlayerInstance( int& aPlayerId, const char* aName, HActiveObjectClient* actObj, void* browserHander )
{
	static int s_myId = 0;
	PlayerVector::iterator it;

    if( aPlayerId < 0 ){ //get free one.
        for( it = m_players.begin(); it != m_players.end(); ++it ){
    		if( (*it)->isAvailable() ){
    			/* added by teddy at 2011-1-17 16:31:18
    			 * 检查空闲Player, 如果Player可用, 则调用getInstanceId读取改Player的ID.
    			 * getInstanceId参数为C++引用类型,可以直接给aPlayerId赋值.
    			 */
    			(*it)->getInstanceId( aPlayerId );
    			(*it)->isAvailable( false );
    			//HIPPO_DEBUG( "found and return it.id=%d\n", aPlayerId );
    			return static_cast<Player*>(*it);
    		}
    	}
    }else{
        return getPlayerInstanceById( aPlayerId );
    }

    s_myId += 1;
    aPlayerId = s_myId;

    return NULL;
}

Player* MediaPlayerMgr::getPlayerInstanceById(int aPlayerId)
{
    PlayerVector::iterator it;

    HIPPO_DEBUG("playerId=%d\n", aPlayerId);
    for (it = m_players.begin(); it != m_players.end(); ++it) {
        int myId;
        (*it)->getInstanceId(myId);
        if( myId == aPlayerId )
            return static_cast<Player*>(*it);
    }
    return NULL;
}

int MediaPlayerMgr::releasePlayerInstanceById(int aPlayerId)
{
    PlayerVector::iterator it;

    HIPPO_DEBUG("playerId = %d\n", aPlayerId);
    for (it = m_players.begin(); it != m_players.end(); ++it) {
        int myId = -1;
        (*it)->getInstanceId(myId);
        if (myId == aPlayerId) {
            (*it)->isAvailable(true);
            break;
        }
    }
    return 0;
}

}
