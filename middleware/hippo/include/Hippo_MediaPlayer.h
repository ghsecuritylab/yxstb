#ifndef MIDDLEWARE_MEDIAPLAYERMGR_H
#define MIDDLEWARE_MEDIAPLAYERMGR_H

#include <vector>
#include <map>
#include "Hippo_HString.h"
#include "base/Hippo_HEvent.h"

//#include "tmp/C05API_for_hippo.h"
#include "base/Hippo_HEventDispatcher.h"

class listener;
namespace Hippo{
class PlayerNode;
class Player;
/**
* 依赖于iPanel的结构体定义.
*/
#define int32 int
#define MAX_STARTTIME_LENTH 32
#define MAX_ENDTIME_LENTH 32
struct media{ /*电信2.2 媒体相关参数*/
const char * cMediaUrl;
int32 iMediaType;
int32 iAudioType;
int32 iVideoType;
int32 iStreamType;
int32 iDrmType;
int32 iFingerPrint;
int32 iCopyProtection;
int32 iAllowTrickmode;/*为1允许Trick操作  */
const char * cMediaCode;
const char * cEntryID;
char cStartTime[MAX_STARTTIME_LENTH];
char cEndTime[MAX_ENDTIME_LENTH];
int32 currplaying_media;/*对底层无用 ipanel需要*/
int32 nextplay_media;/*底层无用 ipanel需要*/
};
//interface for MediaPlayer instance manager.
typedef std::vector<Player*> PlayerVector;
class MediaPlayerMgr {
public:
	typedef enum chnl_node_type{
		ChnlNode_eUnknown,
		ChnlNode_eIptv,
		ChnlNode_eDvbs,
		ChnlNode_eDvbt,
		ChnlNode_eDvbc,
		ChnlNode_eMax,
	} chnl_node_type_e;

public:
	virtual ~MediaPlayerMgr( );


	/***********************************************************/
	/** createPlayerInstance
	* @param   aPlayerId: 取值-1时由MediaPlayerMgr负责选取PlayerId,其他值由调用者G指定,若出现冲突则强制解邦.
	*			此处为应对iPanel2.0所定,对于Takin和iPanel3.0调用该函数时永远为-1.
	* @return
	* @remark 创建一个Player对象.
	* @see
	* @author teddy      @date 2011/01/24
	************************************************************/
	virtual Player* createPlayerInstance( int& aPlayerId,const char* name = NULL, HActiveObjectClient* actObj = NULL, void* browserHandler = NULL );
	virtual Player* getPlayerInstanceById( int aPlayerId );
	virtual int releasePlayerInstanceById( int aPlayerId ) = 0;
	virtual int getPlayerInstanceNum( ) = 0;

	virtual int addChannelNode( const chnl_node_type_e aChnlType, HString& str ) = 0;
	virtual PlayerNode* getChannelNode( const chnl_node_type_e, int ) = 0;
	virtual int getPlayerEvent( ) = 0;
	virtual int createChannelList( const chnl_node_type_e aChnlType, const int cnt ) = 0;

protected:
	MediaPlayerMgr( );

	PlayerVector m_players;// nativeplayerId and nativePlayerinstance vector
};

struct HRect{
	//public:
	//   HRect( ): m_x(0), m_y(0), m_w(0), m_h(0) {}
	//	HRect( int x, int y, int w, int h ): m_x(x), m_y(y), m_w( w ), m_h(h) {}
	//	HRect& operator=( HRect* p );
	int m_x,m_y,m_w,m_h;
};
/*
   HRect& HRect::operator=( HRect* p )
   {
   this->m_x = p->m_x;
   this->m_y = p->m_y;
   this->m_w = p->m_w;
   this->m_h = p->m_h;
   return *this;
   }
*/
class Listener{
};
/*
 * 负责管理播放列表, 播放控制;
 * 为应对EPG只调用createPlayerInstance而不调用releasePlayerInstanceById,本地提供Player池.
 * Player按需创建, 但只有在调用releasePlayerInstanceById的情况下才会删除.
 */
class Player : public HActiveObject{
	friend class MediaPlayerMgr;
	friend class PlayerNode;
public:
	typedef enum {
		MediaInfoType_eUnknown,
		MediaInfoType_eJson,
		MediaInfoType_eUrl
	}media_info_type_e;

	typedef enum {
		PlayerState_eUnknown,
		PlayerState_eReady,
		PlayerState_ePlay,
		PlayerState_ePause,
		PlayerState_eFast,
		PlayerState_eRewind
	}player_state_e;

	typedef enum{
		PlayerEventType_eUnknown,
		PlayerEventType_eModeChange,
		//VOD播放到头, 时移快退到左边界.
		PlayerEventType_eStreamStart, //
		//VOD快退到尾, 时移快进到右边界, 追上直播.
		PlayerEventType_eStreamEnd,

	}player_event_type_e;

    //Player属性控制
	typedef enum{
		PlayerPropertyType_eUnknown = 0,

		//playlist mode proerty
		PlayerPropertyType_eSingleOrPlaylistMode,
		PlayerPropertyType_eCycleFlag,
		PlayerPropertyType_eRandomFlag,
		PlayerPropertyType_eAutoDeleteFlag,

        //
		PlayerPropertyType_eAllowTrickPlayFlag,
		PlayerPropertyType_eMuteFlag,
		PlayerPropertyType_eSubtitleFlag,
		PlayerPropertyType_eVideoDisplayMode,
		PlayerPropertyType_eVideoDisplayArea,
		PlayerPropertyType_eVideoAlpha,

		//local ICON control flag.
		PlayerPropertyType_eNativeUIFlag,
		PlayerPropertyType_eMuteUIFlag,
		PlayerPropertyType_eChnlNoUIFlag,
		PlayerPropertyType_eProgressBarUIFlag,
		PlayerPropertyType_eAudioTrackUIFlag,
		PlayerPropertyType_eAudioVolumeUIFlag,
		PlayerPropertyType_eAudioVolume,
        PlayerPropertyType_eAudioChannel, //out.m_strVal
		PlayerPropertyType_eAudioTrackPID, //out.m_Value.m_intVal, m_intVal=-1 if no valid pid.
		PlayerPropertyType_eAudioTrackPIDs,
		PlayerPropertyType_eAudioTrackInfo, //out.m_strValue
		PlayerPropertyType_eSubtitlePID, //out.m_Value.m_intVal, getProperty-get current subtitle info. out.m_strVal;
        PlayerPropertyType_eSubtitlePIDs,
		PlayerPropertyType_eSubtitleInfo, //out.m_strValue. getProperty-get current subtitle info. out.m_strVal;
		PlayerPropertyType_eTeletext, //out.m_Value.m_intVal

		//get media count in playlist mode.
		PlayerPropertyType_eMediaCount,
		PlayerPropertyType_eMediaPlaylist,

		//play Infomation.
		PlayerPropertyType_eCurrentMediaCode,
		PlayerPropertyType_eCurrentMediaIdx,
		PlayerPropertyType_eCurrentMediaEntryId,
		PlayerPropertyType_eCurrentPlayChnlNum,
		PlayerPropertyType_eCurrentAudioChannel,
		PlayerPropertyType_eCurrentAudioTrack,
		PlayerPropertyType_eCurrentSubtitle,
		PlayerPropertyType_eCurrentPlayTime, //m_PlayTime.m_strTimeStamp
		PlayerPropertyType_eCurrentMediaDuration,
		PlayerPropertyType_eCurrentPlayBackMode,
		PlayerPropertyType_eMacrovisionFlag, //int
		PlayerPropertyType_eHDCPFlag, //int
		PlayerPropertyType_eCGMSAFlag, //int

		PlayerPropertyType_eMax
	}player_property_type_e;

	/*
	 *  0: 按setVideoDisplayArea()中设定的Height, Width, Left, Top属性所指
	 *  定的位置和大小来显示视频;
	 *  1: 全屏显示，按全屏高度和宽度显示(默认值);
	 *  2: 按宽度显示，指在不改变原有图像纵横比的情况下按全屏宽度显示;
	 *  3: 按高度显示，指在不改变原有图像纵横比的情况下按全屏高度显示;
	 *  255: 视频显示窗口将被关闭。它将在保持媒体流连接的前提下，隐藏视频窗
	 *  口。如果流媒体播放没有被暂停，将继续播放音频。
	 **/
	typedef enum{
		PlayerVideoMode_eUnknown,
		PlayerVideoMode_eVideoByArea = 0,
		PlayerVideoMode_eFullScreen = 1,
		PlayerVideoMode_eByWidth = 2,
		PlayerVideoMode_eByHeight = 3,
		PlayerVideoMode_eVideoHide = 255,
	}player_video_mode_e;
	typedef enum{
		PlaylistOpType_eUnknown = 0,

		//change index operator,
		/*
		 * 根据index将指定的媒体在播放列表中移动
		 * entryID:播放列表中某个媒体条目的唯一标识。
		 * toIndex，整数，需要移动到的索引值。0：表示播放列表的顶端
		**/
		PlaylistOpType_eIdxMoveByIndex,

		/*
		 * 根据index将指定的媒体在播放列表中移动
		 * index，整数，指定的媒体在播放列表中的索引值。0：表示播放列表的顶端。
		 * toIndex，整数，需要移动到的索引值。0：表示播放列表的顶端。
		 */
		PlaylistOpType_eIdxMoveByIndex1,
		/*
		 * 根据偏移量将指定的媒体在播放列表中移动
		 * entryID:播放列表中某个媒体条目的唯一标识。
		 * offset，偏移量，正整数表示从指定媒体向列表末端移动，
		 *                 负整数表示从指定媒体向列表起始端移动
		 **/
		PlaylistOpType_eIdxMoveByOffset,
		/*
		 * 根据偏移量将指定的媒体在播放列表中移动
		 * index，整数，指定的媒体在播放列表中的索引值。0：表示播放列表的顶端。
		 * offset，偏移量，正整数表示从指定媒体向列表末端移动，
		 *                 负整数表示从指定媒体向列表起始端移动
		 **/
		PlaylistOpType_eIdxMoveByOffset1,

		/*
		 * 将指定的媒体下移.
		 * entryID:播放列表中某个媒体条目的唯一标识。
		 */
		PlaylistOpType_eIdxMoveToNext,

		/*
		 * 将指定的媒体上移.
		 * entryID[in]:播放列表中某个媒体条目的唯一标识。
		 */
		PlaylistOpType_eIdxMoveToPrevious,
		/*
		 * 将指定的媒体移到列表顶端.
		 * entryID[in]:播放列表中某个媒体条目的唯一标识。
		 */
		PlaylistOpType_eIdxMoveToFirst,
		/*
		 * 将指定的媒体移到列表末端.
		 * entryID[in]:播放列表中某个媒体条目的唯一标识。
		 */
		PlaylistOpType_eIdxMoveToLast,

		/*
		 * 将指定的媒体下移.
		 * index，整数，指定的媒体在播放列表中的索引值。
		 *		  0：表示播放列表的顶端.
		 */
		PlaylistOpType_eIdxMoveToNext1,
		/*
		 * 将指定的媒体上移.
		 * index，整数，指定的媒体在播放列表中的索引值。
		 *		  0：表示播放列表的顶端.
		 */
		PlaylistOpType_eIdxMoveToPrevious1,
		/*
		 * 将指定的媒体移到列表顶端.
		 * index，整数，指定的媒体在播放列表中的索引值。
		 *		  0：表示播放列表的顶端.
		 */
		PlaylistOpType_eIdxMoveToFirst1,
		/*
		 * 将指定的媒体下移.
		 * index，整数，指定的媒体在播放列表中的索引值。
		 *		  0：表示播放列表的顶端.
		 */
		PlaylistOpType_eIdxMoveToLast1,
		/*
		 * 按媒体在播放列表中的索引选中为当前候选播放节目
		 * index:媒体在播放列表中的索引，0：表示播放列表的顶端。
		 */
		PlaylistOpType_eIdxSelectByIndex,
		/*
		 * 按与当前媒体索引的偏移量选中媒体，作为当前候选播放节目
		 * offset，偏移量，正整数表示从当前媒体向列表末端跳转，
		 *				   负整数表示从当前媒体向列表起始端跳转
		 */
		PlaylistOpType_eIdxSelectOffset,
		/*
		 * 按某个媒体条目的唯一标识选中为当前候选播放节目.
		 * entryID:播放列表中某个媒体条目的唯一标识
		 * (在添加媒体时设置并且在该播放列表中保持不变)
		 */
		PlaylistOpType_eIdxSelectByEntryId,

		/*
		 * 选取播放列表中的下一个媒体，作为当前候选播放节目
		 */
		PlaylistOpType_eIdxSelectNext,
		/*
		 * 选取播放列表中的上一个媒体，作为当前候选播放节目
		 */
		PlaylistOpType_eIdxSelectPrevious,
		/*
		 * 选取播放列表中的第一个媒体，作为当前候选播放节目
		 */
		PlaylistOpType_eIdxSelectFirst,
		/*
		 * 选取播放列表中的最后一个媒体，作为当前候选播放节目
		 */
		PlaylistOpType_eIdxSelectLast,

		/*
		 * 清空播放列表
		 */
		PlaylistOpType_eIdxRemoveAll,
		/*
		 *
		 */
		PlaylistOpType_eIdxRemoveByOffset,
		/*
		 * 删除Index指定的媒体播放文件
		 * entryID:播放列表中某个媒体条目的唯一标识。
		 */
		PlaylistOpType_eIdxRemoveByEntryId,
		/*
		 * 删除Index指定的媒体播放文件
		 * index，整数，指定的媒体在播放列表中的索引值。0：表示播放列表的顶端。
		 */
		PlaylistOpType_eIdxRemoveByIndex,


		/*
		 * 删除从Fromindex到ToIndex范围内连续的媒体
		 * Fromindex，整数，指定的起始删除媒体在播放列表中的索引值。
		 *		 0：表示播放列表的顶端。
		 * ToIndex，整数，指定的结束删除媒体在播放列表中的索引值。
		 *		 0：表示播放列表的顶端。
		 */
		 //removed by teddy at 2011.02.22 Tue 11:29:15.
		 //可以通过PlaylistOpType_eIdxRemoveByIndex组合实现,因此删除.
		//PlaylistOpType_eIdxRemoveByIndex1,
		/*
		 * 获取一个PlayNode.
		 * index，整数，指定的媒体在播放列表中的索引值。0：表示播放列表的顶端。
		 */
		PlaylistOpType_eGetPlayNodeByIndex = 50,
		/*
		 * 获取一个PlayNode,
		 * entryID:播放列表中某个媒体条目的唯一标识。
		 */
		PlaylistOpType_eGetPlayNodeByEntryId,
		PlaylistOpType_eInvalidIdx,

	} playlist_op_type_e;

	typedef enum{
		TimeType_eIgnore,
        TimeType_eNPT,
        TimeType_eUTC,
		TimeType_eMediaStart,
		TimeType_eMediaEnd,
		TimeType_eCurrentTime,
		TimeType_eDefault,
	}time_type_e;

	struct PlayTime {
		//PlayTime( ): m_TimeFormat(TimeType_eDefault), m_TimeStamp( 0 ), m_strTimeStamp( "" )  { }
		//PlayTime( PlayTime& self ) { m_TimeFormat = self.m_TimeFormat; m_TimeStamp = self.m_TimeStamp; m_strTimeStamp = self.m_strTimeStamp; }

		time_type_e m_TimeFormat;   //0=NPT, 1=UTC,
		int m_TimeStamp;
		HString m_strTimeStamp;
	};

	class HPlayerProperty{
	public:
		HPlayerProperty( );
		player_property_type_e m_eType;
		HString m_strVal;
		PlayTime m_PlayTime;
		union{
			int m_intVal;
			HRect rect;
			player_video_mode_e vMode;
		} m_Value;
	};
	/*
	 * 操作播放列表输入参数
	 * m_old* : 为将要操作的PlayNode.
	 * m_new* : PlayNode新的属性.
	 **/
	class HPlaylistProperty{
	public:
		playlist_op_type_e m_eType;
		/*
		 * 要操作的PlayNode输入, 当为输入参数为字符串时(比如EntryId), Hippo据此搜索要操作的PlayNode.
		 */
		HString m_oldStrVal;
		/*
		 * 操作完以后的值,
		 */
		HString m_newStrVal;
		union{
			int m_intVal;
		} m_oldValue,m_newValue;
	};

	virtual ~Player( ) { }
    virtual const char* getProertyString( player_property_type_e e ) const;

    //生命周期管理函数.
	virtual int getInstanceId( int& aPlayerId ) { aPlayerId = m_InstatncdId; return 0;}
	virtual bool isAvailable( ) const { return m_bAvailable; }
	virtual void isAvailable( bool it ) { m_bAvailable = it; }

	//播放相关函数
	virtual int open( ) = 0;
	virtual int play( int aStartTime, time_type_e );
	virtual int fastForward( int speed, unsigned long aPlayTime, time_type_e ) = 0;
	virtual int fastRewind( int speed, unsigned long aPlayTime, time_type_e ) = 0;
	virtual int seekTo( unsigned long aPlayTime, time_type_e ) = 0;
	virtual int seekTo( const char* aPlayTime, time_type_e );
	virtual int pause( ) = 0;
	virtual int resume( ) = 0;
	virtual int stop( );
	virtual int close( ) = 0;
	virtual int getState( ) = 0;
	virtual int joinChannel( int aChnlUserNum ) = 0;
	virtual int leaveChannel( ) = 0;

	//更新视频位置.
	virtual int refreshVideoDisplay( ) = 0;
	//Player属性设置/获取
	virtual int setProperty( player_property_type_e aType, HPlayerProperty& aValue );
	virtual int getProperty( player_property_type_e aType, HPlayerProperty& aResult );
    virtual int set(const char * ioStr, const char * wrStr);
    virtual const char * get(const char * ioStr);

	//添加播放Node
	virtual int setSingleMedia( media_info_type_e eType, const char *aInfo ) = 0; //str maybe a json array.
	virtual int addSingleMedia( media_info_type_e eType, int aIdx, const char* str ) = 0;
	virtual int removePlayNode( playlist_op_type_e, HPlaylistProperty& aValue ) = 0;
	virtual int movePlayNode( playlist_op_type_e, HPlaylistProperty& aValue) = 0;
	virtual int selectPlayNode( playlist_op_type_e, HPlaylistProperty& aValue ) = 0;
	virtual int playSelectedNode( PlayerNode*, playlist_op_type_e eType, int aValue  );
	virtual PlayerNode* getPlayerNode( playlist_op_type_e eType, int aValue ) { return 0;}

    //为接收stream模块回调消息所加.
	static HEventDispatcher::event_status_e handleEvent( HEvent::event_type_e eType, int wParam, int lParam, void* other);
	virtual HEventDispatcher::event_status_e onEvent( int key );

    //interface of HActiveObject.
    virtual HEventDispatcher::event_status_e HandleEvent( HEvent& event );
    virtual int instanceId() { return m_InstatncdId; }
    virtual void clearForRecycle() = 0;
	//add EventListener.
	int addEventListener( int aEvent, Listener* );
	int removedEventListener( int aEvent, Listener* );
	int clearAllEventListener( );
protected:
	Player( int aPlayerId );
	Player( int aPlayerId, HActiveObjectClient*, void* browserHandler );

	//播放类公共属性.
   	int		m_InstatncdId;
	int	m_bAllowTrickPlay;
	bool	m_bNativeUIFlag;
	bool	m_bAudioTrackUIFlag;
	bool	m_bMuteUIFlag;
	bool	m_bAudioVolumeUIFlag;
    bool    m_bChnlNoUIFlag;
	bool	m_bCycleFlag; //循环播放
	bool	m_bPlaylistMode;
	HRect	m_VideoArea;

	player_video_mode_e m_eVideoMode;

	//所属的PlayerMgr.
	MediaPlayerMgr* m_pPlayerMgr;

	//标识自身是否处于空闲状态. false表示处于和JS MediaPlayer对象处于绑定状态.
	bool m_bAvailable;
protected:
    //事件监听函数用private权限, 其他都用protected.
	std::vector<Listener*> m_listeners;
	Listener* m_listener;

	HActiveObjectClient* m_client;
	void* m_browserHandler;
};

/**
 * <interface>
 *  实际的播放动作;
 */
class PlayerNode {
	friend class Player;
public:
	typedef enum{
		PlayNodeType_eUnknown,
		PlayNodeType_eIPTV = 0,
		PlayNodeType_eVOD = 1,
	}play_node_type_e;

	virtual ~PlayerNode( );
	virtual int play( int aStartTime, Player::time_type_e );
	virtual int fastForward( int );
	virtual int fastRewind( int );
	virtual int pause( );
	virtual int resume( );
	virtual int seekTo( const char*, Player::time_type_e );
	virtual int stop( );
	virtual int close( ) = 0;
	virtual int onEvent( ){return 0;}
	virtual int getState( ) = 0;
	virtual play_node_type_e getType() = 0;
	virtual unsigned int getTotalTime()= 0;
protected:
	PlayerNode( );


	/**
	 * 此结构体是iPanel2.0支持IPTV2.0播放业务所定义, 其他情况下勿用
	 *
	 */
	struct media* m_mediaInfo;
private:

};
}
#endif

