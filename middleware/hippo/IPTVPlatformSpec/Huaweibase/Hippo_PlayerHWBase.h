#ifndef HIPPO_PlayerMgrHWBase_H
#define HIPPO_PlayerMgrHWBase_H

#include <map>
#include <list>
#include <Hippo_HString.h>
#include "Hippo_MediaPlayer.h"
#include <json/json_public.h>
#include "mid_stream.h"


namespace Hippo{
class HActiveObjectClient;
class PlayerNode;
class PlayerHWBase;

class PlayerMgrHWBase : public MediaPlayerMgr {
public:
	PlayerMgrHWBase( );
	virtual ~PlayerMgrHWBase( );

	virtual Player* createPlayerInstance( int& aPlayerId,const char* name, HActiveObjectClient* actObj, void* browserHandler );
	virtual int releasePlayerInstanceById( int aPlayerId );
	virtual int getPlayerInstanceNum( );

	virtual int addChannelNode( const chnl_node_type_e aChnlType, HString& str );
	virtual PlayerNode* getChannelNode( const chnl_node_type_e, int ) ;
	virtual int getPlayerEvent( );
	virtual int createChannelList( const chnl_node_type_e aChnlType, const int cnt );

};

/*
 * 负责管理播放列表, 播放控制;
 * 一个JS MediaPlayer对象必须绑定一个Player对象后才可以工作.
 */
class PlayerHWBase : public Player {
	friend class PlayerMgrHWBase;
public:

	virtual ~PlayerHWBase();
	//bool isAvailable( ) const { return m_bAvailable; }
	//void isAvailable( bool it ) { m_bAvailable = it; }

	virtual int open( );
	virtual int play( int aStartTime, time_type_e );
	virtual int fastForward( int speed, unsigned long aPlayTime, time_type_e );
	virtual int fastRewind( int speed, unsigned long aPlayTime, time_type_e );
	virtual int seekTo( unsigned long aPlayTime, time_type_e );
	virtual int seekTo( const char*, time_type_e );
	virtual int pause( );
	virtual int resume( );
	virtual int stop( );
	virtual int close( );
	virtual int getState( );

	virtual int getProperty( player_property_type_e aType, HPlayerProperty& aResult );
	void statecall(int pIndex, STRM_STATE state, int rate, unsigned int magic,int callarg);
	void msgcall(int pIndex, STRM_MSG msg, int arg, unsigned int magic, int callarg);
	virtual void setMagic(int magic){m_magic = magic;}
	void setMediaCode(HString &mediacode) {m_mediaCode = mediacode;}
	HString& getMediaCode() {return m_mediaCode;}
	void setEntryID(HString &entryid) {m_entryID = entryid;}
	HString& getEntryID() {return m_entryID;}

//		virtual int PlayListFun(playlistfun_idx_type_e eType, int Mode, int nIndex, int nToIndex, int nEntryID, int nOffset, const char *pchMediaStr);


protected:
	PlayerHWBase(int aPlayerId);

	PlayerNode* m_curNode; //current play node.
	PlayerNode* m_nextNode; //next play node.
	//bool    m_bAvailable; //indicate this player available.
	int m_magic;
	HString m_mediaCode;
	HString m_entryID;

	std::list<PlayerNode*> m_playlist;
	std::map<const HString, PlayerNode*> m_playlistEntryIdMap;
};

}

#endif

