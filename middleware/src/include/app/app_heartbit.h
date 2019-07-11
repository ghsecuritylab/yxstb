#ifndef __APP_HEART_BIT__
#define	__APP_HEART_BIT__

typedef struct _EPG_MESSAGE_	EPG_MSG;

struct _EPG_MESSAGE_
{
	EPG_MSG*	prev;
	EPG_MSG*	next;
	int			MsgID;
	char		MsgVer[32];
	int			MsgType;
	char		MsgContent[1024 + 4];
	int			RollInternal;
	int			RollTimes;
	int			RollType;
};

typedef struct _Heart_Bit_
{
  int					UserValid;
  unsigned int			NextCallInterval;
#ifdef HUAWEI_C20
  char                  Channelver[32];
  char                  PPVversin[32];
  char		      PVRVersion[16];
  char                  Bulletinver[32];
  char                  AdText[1024+4];
  int                   Vnet;
  int                   MsgNum;
  //  EPG_MSG               *Msg;
#endif
}HEARTBIT;

typedef struct _http_message_
{
	int type;
	char msg[1024];
}HTTPMESSAGE;

#ifdef __cplusplus
extern "C" {
#endif

void midSTBLoginInit( void );

void httpHeartBit( int );
int httpHeartClr(void);

void midUserStatSet(int stat);
int midUserValidGet(void);
void midServiceIDSet(int id);
int midServiceIDGet(void);

int midHeartBitIntervalSet( unsigned int msec );
unsigned int midHeartBitIntervalGet( void );
int STBLoginRecv( char* p, int type );
int httpSTBLogin( void );
int midecmipSet( char* buf );
char* midecmipGet( void );
int midecmportSet( unsigned short port );
unsigned short midecmportGet( void );

int midemmportSet( unsigned short port );
unsigned short midemmportGet( void );
int midpisysipSet( char* buf );
char* midpisysipGet( void );
int midpisysportSet( unsigned short port );
unsigned short midpisysportGet( void );

void httpRequestInputMsg(const char *msg, int type);
int httpRequestgetMsg(char *msg, int *type);
void httpRequestMsgCreat(void);
#ifdef HUAWEI_C20
int channel_array_get_version(char *pBuf, int bufLen);
void channel_array_set_version(char *ver, int update);
void channel_array_request(int type);
#endif


#ifdef __cplusplus
}
#endif

#endif
