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
* ������iPanel�Ľṹ�嶨��.
*/
#define int32 int
#define MAX_STARTTIME_LENTH 32
#define MAX_ENDTIME_LENTH 32
struct media{ /*����2.2 ý����ز���*/
const char * cMediaUrl;
int32 iMediaType;
int32 iAudioType;
int32 iVideoType;
int32 iStreamType;
int32 iDrmType;
int32 iFingerPrint;
int32 iCopyProtection;
int32 iAllowTrickmode;/*Ϊ1����Trick����  */
const char * cMediaCode;
const char * cEntryID;
char cStartTime[MAX_STARTTIME_LENTH];
char cEndTime[MAX_ENDTIME_LENTH];
int32 currplaying_media;/*�Եײ����� ipanel��Ҫ*/
int32 nextplay_media;/*�ײ����� ipanel��Ҫ*/
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
	* @param   aPlayerId: ȡֵ-1ʱ��MediaPlayerMgr����ѡȡPlayerId,����ֵ�ɵ�����Gָ��,�����ֳ�ͻ��ǿ�ƽ��.
	*			�˴�ΪӦ��iPanel2.0����,����Takin��iPanel3.0���øú���ʱ��ԶΪ-1.
	* @return
	* @remark ����һ��Player����.
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
 * ����������б�, ���ſ���;
 * ΪӦ��EPGֻ����createPlayerInstance��������releasePlayerInstanceById,�����ṩPlayer��.
 * Player���贴��, ��ֻ���ڵ���releasePlayerInstanceById������²Ż�ɾ��.
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
		//VOD���ŵ�ͷ, ʱ�ƿ��˵���߽�.
		PlayerEventType_eStreamStart, //
		//VOD���˵�β, ʱ�ƿ�����ұ߽�, ׷��ֱ��.
		PlayerEventType_eStreamEnd,

	}player_event_type_e;

    //Player���Կ���
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
	 *  0: ��setVideoDisplayArea()���趨��Height, Width, Left, Top������ָ
	 *  ����λ�úʹ�С����ʾ��Ƶ;
	 *  1: ȫ����ʾ����ȫ���߶ȺͿ����ʾ(Ĭ��ֵ);
	 *  2: �������ʾ��ָ�ڲ��ı�ԭ��ͼ���ݺ�ȵ�����°�ȫ�������ʾ;
	 *  3: ���߶���ʾ��ָ�ڲ��ı�ԭ��ͼ���ݺ�ȵ�����°�ȫ���߶���ʾ;
	 *  255: ��Ƶ��ʾ���ڽ����رա������ڱ���ý�������ӵ�ǰ���£�������Ƶ��
	 *  �ڡ������ý�岥��û�б���ͣ��������������Ƶ��
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
		 * ����index��ָ����ý���ڲ����б����ƶ�
		 * entryID:�����б���ĳ��ý����Ŀ��Ψһ��ʶ��
		 * toIndex����������Ҫ�ƶ���������ֵ��0����ʾ�����б�Ķ���
		**/
		PlaylistOpType_eIdxMoveByIndex,

		/*
		 * ����index��ָ����ý���ڲ����б����ƶ�
		 * index��������ָ����ý���ڲ����б��е�����ֵ��0����ʾ�����б�Ķ��ˡ�
		 * toIndex����������Ҫ�ƶ���������ֵ��0����ʾ�����б�Ķ��ˡ�
		 */
		PlaylistOpType_eIdxMoveByIndex1,
		/*
		 * ����ƫ������ָ����ý���ڲ����б����ƶ�
		 * entryID:�����б���ĳ��ý����Ŀ��Ψһ��ʶ��
		 * offset��ƫ��������������ʾ��ָ��ý�����б�ĩ���ƶ���
		 *                 ��������ʾ��ָ��ý�����б���ʼ���ƶ�
		 **/
		PlaylistOpType_eIdxMoveByOffset,
		/*
		 * ����ƫ������ָ����ý���ڲ����б����ƶ�
		 * index��������ָ����ý���ڲ����б��е�����ֵ��0����ʾ�����б�Ķ��ˡ�
		 * offset��ƫ��������������ʾ��ָ��ý�����б�ĩ���ƶ���
		 *                 ��������ʾ��ָ��ý�����б���ʼ���ƶ�
		 **/
		PlaylistOpType_eIdxMoveByOffset1,

		/*
		 * ��ָ����ý������.
		 * entryID:�����б���ĳ��ý����Ŀ��Ψһ��ʶ��
		 */
		PlaylistOpType_eIdxMoveToNext,

		/*
		 * ��ָ����ý������.
		 * entryID[in]:�����б���ĳ��ý����Ŀ��Ψһ��ʶ��
		 */
		PlaylistOpType_eIdxMoveToPrevious,
		/*
		 * ��ָ����ý���Ƶ��б���.
		 * entryID[in]:�����б���ĳ��ý����Ŀ��Ψһ��ʶ��
		 */
		PlaylistOpType_eIdxMoveToFirst,
		/*
		 * ��ָ����ý���Ƶ��б�ĩ��.
		 * entryID[in]:�����б���ĳ��ý����Ŀ��Ψһ��ʶ��
		 */
		PlaylistOpType_eIdxMoveToLast,

		/*
		 * ��ָ����ý������.
		 * index��������ָ����ý���ڲ����б��е�����ֵ��
		 *		  0����ʾ�����б�Ķ���.
		 */
		PlaylistOpType_eIdxMoveToNext1,
		/*
		 * ��ָ����ý������.
		 * index��������ָ����ý���ڲ����б��е�����ֵ��
		 *		  0����ʾ�����б�Ķ���.
		 */
		PlaylistOpType_eIdxMoveToPrevious1,
		/*
		 * ��ָ����ý���Ƶ��б���.
		 * index��������ָ����ý���ڲ����б��е�����ֵ��
		 *		  0����ʾ�����б�Ķ���.
		 */
		PlaylistOpType_eIdxMoveToFirst1,
		/*
		 * ��ָ����ý������.
		 * index��������ָ����ý���ڲ����б��е�����ֵ��
		 *		  0����ʾ�����б�Ķ���.
		 */
		PlaylistOpType_eIdxMoveToLast1,
		/*
		 * ��ý���ڲ����б��е�����ѡ��Ϊ��ǰ��ѡ���Ž�Ŀ
		 * index:ý���ڲ����б��е�������0����ʾ�����б�Ķ��ˡ�
		 */
		PlaylistOpType_eIdxSelectByIndex,
		/*
		 * ���뵱ǰý��������ƫ����ѡ��ý�壬��Ϊ��ǰ��ѡ���Ž�Ŀ
		 * offset��ƫ��������������ʾ�ӵ�ǰý�����б�ĩ����ת��
		 *				   ��������ʾ�ӵ�ǰý�����б���ʼ����ת
		 */
		PlaylistOpType_eIdxSelectOffset,
		/*
		 * ��ĳ��ý����Ŀ��Ψһ��ʶѡ��Ϊ��ǰ��ѡ���Ž�Ŀ.
		 * entryID:�����б���ĳ��ý����Ŀ��Ψһ��ʶ
		 * (�����ý��ʱ���ò����ڸò����б��б��ֲ���)
		 */
		PlaylistOpType_eIdxSelectByEntryId,

		/*
		 * ѡȡ�����б��е���һ��ý�壬��Ϊ��ǰ��ѡ���Ž�Ŀ
		 */
		PlaylistOpType_eIdxSelectNext,
		/*
		 * ѡȡ�����б��е���һ��ý�壬��Ϊ��ǰ��ѡ���Ž�Ŀ
		 */
		PlaylistOpType_eIdxSelectPrevious,
		/*
		 * ѡȡ�����б��еĵ�һ��ý�壬��Ϊ��ǰ��ѡ���Ž�Ŀ
		 */
		PlaylistOpType_eIdxSelectFirst,
		/*
		 * ѡȡ�����б��е����һ��ý�壬��Ϊ��ǰ��ѡ���Ž�Ŀ
		 */
		PlaylistOpType_eIdxSelectLast,

		/*
		 * ��ղ����б�
		 */
		PlaylistOpType_eIdxRemoveAll,
		/*
		 *
		 */
		PlaylistOpType_eIdxRemoveByOffset,
		/*
		 * ɾ��Indexָ����ý�岥���ļ�
		 * entryID:�����б���ĳ��ý����Ŀ��Ψһ��ʶ��
		 */
		PlaylistOpType_eIdxRemoveByEntryId,
		/*
		 * ɾ��Indexָ����ý�岥���ļ�
		 * index��������ָ����ý���ڲ����б��е�����ֵ��0����ʾ�����б�Ķ��ˡ�
		 */
		PlaylistOpType_eIdxRemoveByIndex,


		/*
		 * ɾ����Fromindex��ToIndex��Χ��������ý��
		 * Fromindex��������ָ������ʼɾ��ý���ڲ����б��е�����ֵ��
		 *		 0����ʾ�����б�Ķ��ˡ�
		 * ToIndex��������ָ���Ľ���ɾ��ý���ڲ����б��е�����ֵ��
		 *		 0����ʾ�����б�Ķ��ˡ�
		 */
		 //removed by teddy at 2011.02.22 Tue 11:29:15.
		 //����ͨ��PlaylistOpType_eIdxRemoveByIndex���ʵ��,���ɾ��.
		//PlaylistOpType_eIdxRemoveByIndex1,
		/*
		 * ��ȡһ��PlayNode.
		 * index��������ָ����ý���ڲ����б��е�����ֵ��0����ʾ�����б�Ķ��ˡ�
		 */
		PlaylistOpType_eGetPlayNodeByIndex = 50,
		/*
		 * ��ȡһ��PlayNode,
		 * entryID:�����б���ĳ��ý����Ŀ��Ψһ��ʶ��
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
	 * ���������б��������
	 * m_old* : Ϊ��Ҫ������PlayNode.
	 * m_new* : PlayNode�µ�����.
	 **/
	class HPlaylistProperty{
	public:
		playlist_op_type_e m_eType;
		/*
		 * Ҫ������PlayNode����, ��Ϊ�������Ϊ�ַ���ʱ(����EntryId), Hippo�ݴ�����Ҫ������PlayNode.
		 */
		HString m_oldStrVal;
		/*
		 * �������Ժ��ֵ,
		 */
		HString m_newStrVal;
		union{
			int m_intVal;
		} m_oldValue,m_newValue;
	};

	virtual ~Player( ) { }
    virtual const char* getProertyString( player_property_type_e e ) const;

    //�������ڹ�����.
	virtual int getInstanceId( int& aPlayerId ) { aPlayerId = m_InstatncdId; return 0;}
	virtual bool isAvailable( ) const { return m_bAvailable; }
	virtual void isAvailable( bool it ) { m_bAvailable = it; }

	//������غ���
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

	//������Ƶλ��.
	virtual int refreshVideoDisplay( ) = 0;
	//Player��������/��ȡ
	virtual int setProperty( player_property_type_e aType, HPlayerProperty& aValue );
	virtual int getProperty( player_property_type_e aType, HPlayerProperty& aResult );
    virtual int set(const char * ioStr, const char * wrStr);
    virtual const char * get(const char * ioStr);

	//��Ӳ���Node
	virtual int setSingleMedia( media_info_type_e eType, const char *aInfo ) = 0; //str maybe a json array.
	virtual int addSingleMedia( media_info_type_e eType, int aIdx, const char* str ) = 0;
	virtual int removePlayNode( playlist_op_type_e, HPlaylistProperty& aValue ) = 0;
	virtual int movePlayNode( playlist_op_type_e, HPlaylistProperty& aValue) = 0;
	virtual int selectPlayNode( playlist_op_type_e, HPlaylistProperty& aValue ) = 0;
	virtual int playSelectedNode( PlayerNode*, playlist_op_type_e eType, int aValue  );
	virtual PlayerNode* getPlayerNode( playlist_op_type_e eType, int aValue ) { return 0;}

    //Ϊ����streamģ��ص���Ϣ����.
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

	//�����๫������.
   	int		m_InstatncdId;
	int	m_bAllowTrickPlay;
	bool	m_bNativeUIFlag;
	bool	m_bAudioTrackUIFlag;
	bool	m_bMuteUIFlag;
	bool	m_bAudioVolumeUIFlag;
    bool    m_bChnlNoUIFlag;
	bool	m_bCycleFlag; //ѭ������
	bool	m_bPlaylistMode;
	HRect	m_VideoArea;

	player_video_mode_e m_eVideoMode;

	//������PlayerMgr.
	MediaPlayerMgr* m_pPlayerMgr;

	//��ʶ�����Ƿ��ڿ���״̬. false��ʾ���ں�JS MediaPlayer�����ڰ�״̬.
	bool m_bAvailable;
protected:
    //�¼�����������privateȨ��, ��������protected.
	std::vector<Listener*> m_listeners;
	Listener* m_listener;

	HActiveObjectClient* m_client;
	void* m_browserHandler;
};

/**
 * <interface>
 *  ʵ�ʵĲ��Ŷ���;
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
	 * �˽ṹ����iPanel2.0֧��IPTV2.0����ҵ��������, �������������
	 *
	 */
	struct media* m_mediaInfo;
private:

};
}
#endif

