/**
 * @brief    ��Ƶ��ϵͳ�ն���ֲ��ͷ�ļ�
 * @version  1.3.1
 * @date     2012.07.01
 */

#ifndef CYBER_CLOUD_API_H
#define CYBER_CLOUD_API_H

#ifdef __cplusplus
extern "C"
{
#endif

#define IN                                  ///<����
#define OUT                                 ///<���
#define INOUT                               ///<�������

/// Լ��ָ��
#define CLOUD_VERSION_LEN           24      ///<�汾��Ϣ����
#define CLOUD_VENDOR_LEN            32      ///<�������Ϣ����
#define CLOUD_KEYBOARD_VALUE_SIZE   6       ///<���̰���ֵ�Ĵ�С
#define CLOUD_JOYSTICK_VALUE_SIZE   4       ///<��Ϸ�˰���ֵ�Ĵ�С
#define MSG_MAX_LEN                 1024    ///<����ʾ��Ϣ��󳤶�

///FLASH����
#define CLOUD_FLASH_BLOCK_A        1        ///<BLOCK A���洢�ռ��СΪ64KBytes
#define CLOUD_FLASH_BLOCK_B        2        ///<BLOCK B���洢�ռ��СΪ256KBytes

/// ���ش���
#define CLOUD_OK                    0x0000  ///<�ɹ�
#define CLOUD_FAILURE               0x0001  ///<ʧ��
#define CLOUD_TIMEOUT               0x8001  ///<��ʱ

#define C_TRUE  1
#define C_FALSE 0

/// ������������
typedef signed char    C_S8;
typedef unsigned char  C_U8;
typedef unsigned short C_U16;
typedef unsigned long  C_U32;
typedef unsigned long  C_RESULT;            ///<���庬������ش���
typedef int            C_BOOL;

/// �ź������
typedef void* C_SemaphoreHandle;

/// �߳̾��
typedef void * C_ThreadHandle;

/// ң������������
typedef enum
{
    CloudKey_0              = 0x27, ///<���ּ�0
    CloudKey_1              = 0x1E, ///<���ּ�1
    CloudKey_2              = 0x1F, ///<���ּ�2
    CloudKey_3              = 0x20, ///<���ּ�3
    CloudKey_4              = 0x21, ///<���ּ�4
    CloudKey_5              = 0x22, ///<���ּ�5
    CloudKey_6              = 0x23, ///<���ּ�6
    CloudKey_7              = 0x24, ///<���ּ�7
    CloudKey_8              = 0x25, ///<���ּ�8
    CloudKey_9              = 0x26, ///<���ּ�9
    CloudKey_OK             = 0x28, ///<ȷ�ϼ�.
    CloudKey_Back           = 0x29, ///<���ؼ�.
    CloudKey_Up             = 0x52, ///<�ϼ�.
    CloudKey_Down           = 0x51, ///<�¼�.
    CloudKey_Left           = 0x50, ///<���.
    CloudKey_Right          = 0x4F, ///<�Ҽ�.
    CloudKey_PageUp         = 0x4B, ///<���Ϸ�ҳ��.
    CloudKey_PageDown       = 0x4E, ///<���·�ҳ��.
    CloudKey_Red            = 0xA1, ///<��ɫ���ܼ�.
    CloudKey_Green          = 0xA2, ///<��ɫ���ܼ�.
    CloudKey_Yellow         = 0xA3, ///<��ɫ���ܼ�.
    CloudKey_Blue           = 0xA4, ///<��ɫ���ܼ�.
    CloudKey_ChannelUp      = 0xA5, ///<Ƶ���Ӽ�.
    CloudKey_ChannelDown    = 0xA6, ///<Ƶ������.
    CloudKey_Forward        = 0xA7, ///<�����.
    CloudKey_Backward       = 0xA8, ///<���˼�.
    CloudKey_Play           = 0xA9, ///<���ż�.
    CloudKey_Pause          = 0xAA, ///<��ͣ��.
    CloudKey_PlayPause      = 0xAB, ///<����/��ͣ��.
    CloudKey_Stop           = 0xAC, ///<ֹͣ��.
    ///ɽ��������Ŀ��ʼ���ӵİ���
    CloudKey_Track			= 0xAD, ///<������
    CloudKey_Music			= 0xAE, ///<���ּ�
    CloudKey_Av				= 0xAF, ///<���Ӽ�
    CloudKey_Setup			= 0xB0, ///<���ü�
    CloudKey_LiveTv			= 0xB1, ///<ֱ����
    CloudKey_Vod			= 0xB2,	///<�㲥��
    CloudKey_Info			= 0xB3, ///<��Ϣ��
    CloudKey_Desktop		= 0xB4, ///<�����
	CloudKey_BreakBack		= 0xB5,	///<���м�
	CloudKey_Location		= 0xB6,	///<��λ��
	CloudKey_Help			= 0xB7, ///<������
	CloudKey_Store			= 0xB8, ///<�ղؼ�
	CloudKey_Star			= 0xB9, ///<*��
	CloudKey_Pound			= 0xBA, ///<#��
	CloudKey_F1				= 0xBB, ///<F1��
	CloudKey_F2				= 0xBC, ///<F2��
	CloudKey_F3				= 0xBD, ///<F3��
	CloudKey_F4				= 0xBE, ///<F4��
    ///��������ֲ���ڲ����ƽ���Ļ���ʱ����Ҫ�������������������ֲ�⴦��
    CloudKey_Menu           = 0xF0, ///<�˵���.
    CloudKey_Exit           = 0xF1, ///<�˳���.
    CloudKey_VolUp          = 0xF2, ///<�����Ӽ�.
    CloudKey_VolDown        = 0xF3, ///<��������.
    CloudKey_Mute           = 0xF4  ///<������.
}C_RCKey;

/// ����IP��ַ
typedef C_U32 C_NetworkIP;               ///<�ֽ�˳��Ϊ�����ֽ�˳�򣬼����ģʽ

/// ����˿ں�
typedef C_U16 C_NetworkPort;             ///<�ֽ�˳��Ϊ�����ֽ�˳�򣬼����ģʽ

/// �׽��ֵ�ַ
typedef struct
{
    C_NetworkIP   uIP;                   ///<����IP��ַ
    C_NetworkPort uPort;                 ///<����˿ں�
}C_SocketAddr;

/// �׽�������
typedef enum
{
    SocketType_Stream = 1,               ///<�������׽���
    SocketType_Datagram                  ///<���ݱ��׽���
}C_SocketType;

///�׽�������ѡ��Ĳ��
typedef enum
{
    SocketOptLevel_SOL_SOCKET = 0,  ///<�׽��ֽӿڲ�
    SocketOptLevel_IPPROTO_TCP,     ///<TCP��
    SocketOptLevel_FILEIO,          ///<�ļ�IO��
}C_SocketOptLevel;

///�׽�������ѡ��
typedef enum
{
    SocketOptName_SO_BROADCAST,     ///<��������:C_BOOL;�����׽��ִ��͹㲥��Ϣ
    SocketOptName_SO_RCVBUF,        ///<��������:C_U32;���ý��ջ�������С
    SocketOptName_SO_SNDBUF,        ///<��������:C_U32;���÷��ͻ�������С
    SocketOptName_SO_REUSEADDR,     ///<��������:C_BOOL;�����׽ӿں�һ������ʹ���еĵ�ַ����,��socket������
    SocketOptName_TCP_NODELAY,      ///<��������:C_BOOL;��ֹNagle�㷨
    SocketOptName_FILEIO_NBIO,      ///<��������:C_BOOL;��ֹ����ģʽ
}C_SocketOptName;

/// �׽��־��
typedef void* C_SocketHandle;

/// ���Ʒ�ʽ
typedef enum
{
    ModType_QAM16 = 1,     ///<16 QAM
    ModType_QAM32,         ///<32 QAM
    ModType_QAM64,         ///<64 QAM
    ModType_QAM128,        ///<128 QAM
    ModType_QAM256         ///<256 QAM
}C_ModulationType;

/// ��Ƶ����
typedef enum
{
    VideoType_MPEG2 = 1,
    VideoType_H264
}C_VideoType;

/// ��Ƶ����
typedef enum
{
    AudioType_MPEG2 = 1,
    AudioType_MP3,
    AudioType_MPEG2_AAC
}C_AudioType;

/// ����������
typedef enum
{
    StreamType_TS = 1,
    StreamType_RTP
}C_StreamType;

/// ���ߴ���Ƶ�����
typedef struct
{
    C_U32                    uFrequency;        ///<Ƶ�ʣ���λ��MHz
    C_U32                    uSymbolRate;       ///<�����ʣ���λ��KSymbol/s
    C_ModulationType         uModulationType;   ///<���Ʒ�ʽ
} C_FreqParam;

/// PID�������
typedef struct
{
    C_VideoType         uVideoType;     ///<��Ƶ����
    C_U16               uVideoPid;      ///<��ƵPID
    C_AudioType         uAudioType;     ///<��Ƶ����
    C_U16               uAudioPid;      ///<��ƵPID
    C_U16               uPcrPid;        ///<PCR PID
} C_PIDParam;

/// ��Ϣ����
typedef enum
{
    MsgCode_ShowInfo,    ///<��ʾ��Ϣ
    MsgCode_HideInfo,    ///<������Ϣ
    MsgCode_ShowCursor,  ///<��ʾ��꣬����ն�֧�ֹ��㣬����Ҫ���ݹ��ľ���������Ϣ
    MsgCode_HideCursor   ///<���ع�꣬�����ն�ʵ��������������������߾���������Ϣ
} C_MessageCode;

/// ��������
typedef enum
{
    KeyType_Unknown    = 0,     ///<δ֪�豸
    KeyType_HID        = 1,     ///<HID���豸
    KeyType_Keyboard   = 2,     ///<����
    KeyType_MouseRel   = 3,     ///<����������
    KeyType_MouseAbs   = 4,     ///<����������꣨�����㴥������
    KeyType_Joystick   = 5,     ///<��Ϸ��
    KeyType_MultiTouch = 6,     ///<��㴥�������豸
    KeyType_Flingpc    = 7,     ///<��������ֱ�   
    KeyType_RC         = 8      ///<ң����
}C_KeyType;

typedef struct
{
    C_U8        uButton;    ///<����ָʾ:ֵΪ1ʱ����ʾ����������;ֵΪ0ʱ����ʾ��������;
    C_RCKey     uKeyValue;  ///<����ֵ
} C_RC;

/**
 * @brief  ���̰�����Ϣ
 * @param    uModifierKey ���ⰴ��ָʾ��ָʾλΪ1ʱ����ʾ�ü�Ϊ����״̬
 *             bit0 ---- left  ctrl
 *             bit1 ---- left  shift
 *             bit2 ---- left  alt
 *             bit3 ---- left  windows
 *             bit4 ---- right ctrl
 *             bit5 ---- right shift
 *             bit6 ---- right alt
 *             bit7 ---- right windows
 * @param    uLeds        ����LED��ָʾ��ָʾλΪ1ʱ����ʾ�Ʊ�����
 *             bit0 ---- caps   lock
 *             bit1 ---- num    lock
 *             bit2 ---- scroll lock
 *             bit3~bit7 ---- 0
 * @param    uKeyValue    ����ֵ
 */
typedef struct
{
    C_U32   handle_;
    C_U8    uModifierKey;                         ///< ���ⰴ��ָʾ���˲����ڰ���������Ϣʱ��Ч
    C_U8    uLeds;                                ///< ����LED��ָʾ���˲���������ָʾ��Ϣʱ��Ч
    C_U8    uKeyValue[CLOUD_KEYBOARD_VALUE_SIZE]; ///< ����ֵ���˲����ڰ���������Ϣʱ��Ч
} C_Keyboard;

/**
 * @brief    ������������Ϣ
 * @param    uButton ��갴��ָʾ��ָʾλֵΪ1ʱ����ʾ����������
 *             bit0 ---- button1 ��� ���,����ʱuButton = 0x01
 *             bit1 ---- button2 ��� �Ҽ�,����ʱuButton = 0x02
 *             bit2 ---- button3 �������,����ʱuButton = 0x04
 *             bit3~bit7 ---- 0
 * @param    uXPosition ����ƶ���X����룬�����ƶ�����ֵΪ��ֵ�������ƶ�����ֵΪ��ֵ
 * @param    uYPosition ����ƶ���Y����룬�����ƶ�����ֵΪ��ֵ�������ƶ�����ֵΪ��ֵ
 * @param    uWheel     �������ƶ��ľ��룬���Ϲ���һ�Σ�����ֵΪ+1�����¹���һ�Σ�����ֵΪ-1
 */
typedef struct
{
    C_U32               handle_;
    C_U8                uButton;      ///<��갴��ָʾ
    C_S8                uXPosition;   ///<����ƶ���X�����
    C_S8                uYPosition;   ///<����ƶ���Y�����
    C_S8                uWheel;       ///<�������ƶ��ľ���
} C_MouseRel;

/**
 * @brief    �������������Ϣ
  * @param    uButton ����ָʾ��ָʾλֵΪ1ʱ����ʾ����������
 *             bit0 ---- button1
 *             bit1 ---- button2
 *             bit2~bit7 ---- 0
 * @param    uXPosition �����ԭ���X����룬��������
 * @param    uYPosition �����ԭ���X����룬��������
 * @param    uWheel     �����ƶ��ľ��룬���Ϲ���һ�Σ�����ֵΪ+1�����¹���һ�Σ�����ֵΪ-1
 * @note     �ն���Ƶ�Ʒ������ʾ����Ӧ�ľ������귶ΧΪ��0-4096��
             ���ʵ���豸�ľ������귶Χ��ͬ����Ҫ����ת����
*/
typedef struct
{
    C_U32       handle_;
    C_U8        uButton;    ///<����ָʾ
    C_U16       uXPosition; ///<�ƶ���X�����
    C_U16       uYPosition; ///<�ƶ���Y�����
    C_S8        uWheel;     ///<�����ƶ��ľ���
} C_MouseAbs;

/**
 * @brief    �����׼��Ϸ���������ݸ�ʽ
 * @param    uJoystickId ��Ϸ��ID���̶�ֵ��1-��һ����Ϸ�ˣ�2-�ڶ�����Ϸ�ˣ�3-��������Ϸ�ˣ�4-���ĸ���Ϸ�ˣ�
 * @param    uX  X ��λ�ã�Ĭ��ֵΪ0x7F�������ƶ���ֵ������������0x00�������ƶ���ֵ������������0xFF
 * @param    uY  Y ��λ�ã�Ĭ��ֵΪ0x7F�������ƶ���ֵ������������0x00�������ƶ���ֵ������������0xFF
 * @param    uZ  Z ��λ�ã�Ĭ��ֵΪ0x7F����ǰ�ƶ���ֵ������������0x00������ƶ���ֵ������������0xFF
 * @param    uRx Rx��λ�ã�Ĭ��ֵΪ0x7F��������ת��ֵ������������0x00��������ת��ֵ������������0xFF
 * @param    uRy Ry��λ�ã�Ĭ��ֵΪ0x7F��������ת��ֵ������������0x00��������ת��ֵ������������0xFF
 * @param    uRz Rz��λ�ã�Ĭ��ֵΪ0x7F����ǰ��ת��ֵ������������0x00�������ת��ֵ������������0xFF
 * @param    uHatSwitch ��׼��ͷ��
 *             0 ---- 0/360 degree
 *             1 ---- 45 degree
 *             2 ---- 90 degree
 *             3 ---- 135 degree
 *             4 ---- 185 degree
 *             5 ---- 230 degree
 *             6 ---- 275 degree
 *             7 ---- 315 degree
 *             8 ---- ��Ч
 * @param    uButton[0] ����ָʾ��ָʾλΪ1ʱ����ʾ�ü�������
 *             bit0 ---- button1
 *             bit1 ---- button2
 *             bit2 ---- button3
 *             bit3 ---- button4
 *             bit4 ---- button5
 *             bit5 ---- button6
 *             bit6 ---- button7
 *             bit7 ---- button8
 * @param    uButton[1] ����ָʾ��ָʾλΪ1ʱ����ʾ�ü�������
 *             bit0 ---- button9
 *             bit1 ---- button10
 *             bit2 ---- button11
 *             bit3 ---- button12
 *             bit4 ---- button13
 *             bit5 ---- button14
 *             bit6 ---- button15
 *             bit7 ---- button16
 * @param    uButton[2] ����ָʾ��ָʾλΪ1ʱ����ʾ�ü�������
 *             bit0 ---- button17
 *             bit1 ---- button18
 *             bit2 ---- button19
 *             bit3 ---- button20
 *             bit4 ---- button21
 *             bit5 ---- button22
 *             bit6 ---- button23
 *             bit7 ---- button24
 * @param    uButton[3] ����ָʾ��ָʾλΪ1ʱ����ʾ�ü�������
 *             bit0 ---- button25
 *             bit1 ---- button26
 *             bit2 ---- button27
 *             bit3 ---- button28
 *             bit4 ---- button29
 *             bit5 ---- button30
 *             bit6 ---- button31
 *             bit7 ---- button32
 */
 typedef struct
{
    C_U32   handle_;
    C_U8    uJoystickId;                          ///<��Ϸ��ID���̶�ֵ��1-��һ����Ϸ�ˣ�2-�ڶ�����Ϸ�ˣ�3-��������Ϸ�ˣ�4-���ĸ���Ϸ�ˣ�
    C_U8    uX;                                   ///<X��λ��
    C_U8    uY;                                   ///<Y��λ��
    C_U8    uZ;                                   ///<Z��λ��
    C_U8    uRx;                                  ///<Rx������
    C_U8    uRy;                                  ///<Ry������
    C_U8    uRz;                                  ///<Rz��λ��
    C_U8    uHatSwitch;                           ///<��׼��ͷ��
    C_U8    uButton[CLOUD_JOYSTICK_VALUE_SIZE];   ///<����ָʾ
}C_JoyStick;

/**
 * @brief ����һ�����ص�����ݸ�ʽ
 */
typedef struct
{
    C_U32 touch_id_;            ///<���ص��ʶ��
    C_U16 x_;                   ///<X������
    C_U16 y_;                   ///<Y������
    C_U32 flag_;                ///<һ��λ��־,�μ���: MTI_xxxx
    C_U16 area_width_;          ///<��������Ŀ�ȣ�[0-4096]
    C_U16 area_height_;         ///<��������ĸ߶ȣ�[0-4096]
}C_Touch;

/**
 * @brief �����׼��㴥�����ݸ�ʽ
 */
typedef struct
{
    C_U32         handle_;
    C_U16         touch_count_;   ///<�������
    C_Touch       * touch_buffer_;///<ÿ��������Ϣ������
}C_MultiTouch;


/// ��������ֱ���Ϣ
typedef struct
{
    C_U32   handle_;
    C_U16   uPacketCount;   ///<FingPC�������ݰ�����
    C_U16   uPacketSize;    ///<FingPC�������ݰ���С
    C_U8    *pdata;         ///<�������ݣ����ݳ���=uPacketCount*uPacketSize
} C_FlingPC;

/// HID�豸������Ϣ
typedef struct
{
    C_U8        *pdata;      ///<����
    C_U16       uDataSize;   ///<���ݴ�С
} C_HID;

/**
 * @brief ��������������Ϣ��װ���ݽṹ
 */
typedef union C_DeviceInfo
{
    C_HID    hid_;      ///<���HID���豸
                        ///<����������룬���ݸ�ʽ�������£�
                        ///<            count(4),{dev_id(4),handle(4),rpt_desc_len(2),rpt_desc(rpt_desc_len)}*
                        ///<��������γ������ݸ�ʽ�������£�
                        ///<            count(4),{dev_id(4),handle(4)}*
    C_U32    handle_;   ///<����������͵��豸
}C_DeviceInfo;

/// ������״̬��
typedef enum
{
    DecoderStatusCode_OK = 0x01,    ///<��������
    DecoderStatusCode_DecodeError,  ///<�������
    DecoderStatusCode_DataError,    ///<���ݴ���
    DecoderStatusCode_DataOverflow, ///<��������
    DecoderStatusCode_DataUnderflow ///<��������
}C_DecoderStatusCode;

/// Ts�ź���������
typedef struct
{
    C_U32       uLevel;     ///<�źŵ�ƽ����λ��dB��V��
    C_U32       uCN;        ///<����ȣ���λ��dB��
    C_U32       uErrRate;   ///<������ = uErrRate * 10-9
}C_TsSignal;

/// ��ɫģʽ
typedef enum
{
    ColorMode_RGB565    = 0x01,
    ColorMode_ARGB1555  = 0x02,
    ColorMode_ARGB4444  = 0x03,
    ColorMode_RGB888    = 0x04,
    ColorMode_ARGB8888  = 0x05
}C_ColorMode;

/// �Ʒ���״̬
typedef enum
{
    Status_Portal = 1,   ///<�Ż�״̬
    Status_Application,  ///<Ӧ��״̬
    Status_Unknown       ///<״̬δ֪
}C_Status;

/***************** ����Ϊ��ֲ��ʵ�֣��ṩ���ն�ʹ�õĺ����ӿ�  *************************/
/**
 * @brief  ��ֲ���ʼ��
 * @return   ���س�ʼ�����
 * @retval   CLOUD_OK           ��ʼ���ɹ�
             CLOUD_FAILURE      ��ʼ��ʧ��
 * @note     �������ʧ�ܣ���ֲ�ⲻ������������
 *           �ú���Ϊ��ֲ���ʼ����������¼�Ʒ���֮ǰ���á�
 */
C_RESULT Cloud_Init(void);

/**
 * @brief  ��ֲ�ⷴ��ʼ��
 * @return   ���ؽ��
 * @retval   CLOUD_OK           ����ʼ���ɹ�
             CLOUD_FAILURE      ����ʼ��ʧ��
 */
C_RESULT Cloud_Final(void);

/**
 * @brief  ��ѯ��ֲ��汾
 * @note   ��ȡ��ֲ��İ汾�ż�������Ϣ
 * @param    szVersion  �汾��Ϣ�ַ��������ص��ַ�����ʽ�ο���"CloudLib-1.2.2.29"��
 * @param    szVendor   ������Ϣ�ַ��������ص��ַ�����ʽ�ο���"CyberCloud"��

 * @retval   CLOUD_OK               �ɹ�
 * @retval   CLOUD_FAILURE          ʧ��
 */
C_RESULT Cloud_GetVersion(
    OUT char szVersion[CLOUD_VERSION_LEN + 1],
    OUT char szVendor[CLOUD_VENDOR_LEN + 1]
);

/**
 * @brief  �����Ʒ���
 * @param    szCloudInfo �Ʒ����������ʽ��CloudURL?parameter1=value1&parameter2=value2��
             ����:"CyberCloud://192.168.16.11:10531?UserID=1234567890123456&Password=123456&
               STBID=1234567890123456&MAC=00-FF-B4-A9-90-7E"
 * @return   �����������
 * @retval   CLOUD_OK               �ɹ�
 * @retval   CLOUD_FAILURE          ʧ��
 */
C_RESULT Cloud_Start( IN char const* szCloudInfo);

/**
 * @brief ֹͣ��Ƶ�Ʒ���
 * @note  ֹͣ�Ʒ���
 * @return ����ֹͣ�Ʒ�����
 * @retval   CLOUD_OK               �ɹ�
 * @retval   CLOUD_FAILURE          ʧ��
 */
C_RESULT Cloud_Stop( void );

/**
 * @brief ��Ӧ������Ϣ����������Ϣӳ�����Ƶ�Ʒ��������д���
 * @param  uType      �������ͣ�
 * @param  uLen	      ������Ϣ���ֽ�����
 * @param  puKeyData  ������Ϣ�����ݰ������ͣ�ѡ����ʵ����ݽṹ��
                    C_RC��C_Keyboard�� C_Mouse��C_Touch��C_JoyStick��C_FlingPC����
                    C_HID���а�����Ϣ���죬Ȼ�󽫹���õ���Ϣת����C_U8�����鴫�뱾������
 * @note   C_RCKey��������а���������ô˺���������ֲ����д���
           ������������ֹͣ��Ƶ�Ʒ�������Ȼ��������״̬��
 */
void Cloud_OnKey(
    IN C_KeyType    uType,
    IN C_U8         uLen,
    IN C_U8*        puKeyData
);

/**
 * @brief USBHID�豸���롣֪ͨ��ֲ��һ���µ�USBHID�豸�����նˡ�
 * @param    key_type   ��������
 * @param    dev_info   ����������Ϣ
 * @note   ����ն�ƽ̨��ֲ��USB���ݲɼ��⣬��ô�˺�������ʵ�֡�
 */
void Cloud_DeviceConnect(
    IN C_KeyType    key_type,
    IN C_DeviceInfo *dev_info
);

/**
 * @brief USBHID �豸�γ���֪ͨ��ֲ��һ��USBHID�豸�γ��նˡ�
 * @param    key_type   ��������
 * @param    dev_info   ����������Ϣ
 * @note   ����ն�ƽ̨��ֲ��USB���ݲɼ��⣬��ô�˺�������ʵ�֡�
 *         uhandle ��Cloud_DeviceConnect()���豸������Ӧ
 */
void Cloud_DeviceDisconnect( 
    IN C_KeyType    key_type,
    IN C_DeviceInfo *dev_info
);

/**
 * @brief  �Ʒ����˳��ص���������
 * @param  uExitFlag   �˳���ʾ: 1����ʾ���˳����˳���2����ʾ���˵����˳���
 * @param  szBackURL   ���ص�ַ�������и��ݸò���������Ӧ��ҳ��(���ն˲�֧�������ʱ��
                        �ò���ΪNULL)
 * @note
           ���û�Ҫ�˳��Ʒ���ʱ����ֲ������ע��Ļص�����֪ͨ�����У��������ڸú�����
           ����Cloud_Stop()�����Ҹ��ݷ��ص�ַ���˵���Ӧ��ҳ�档
           ����:
           void exit_callback_func( IN C_U8 uFlag, IN char const *szBackURL )
           {
                Cloud_Stop();
                if( uExitFlag == 1 )
                {
                    //�����н����˳����˳����߼�����
                }
                if( uExitFlag == 2 )
                {
                    //�����н��в˵����˳����߼�����
                }
           }
 */
typedef void (*CStb_CloudExitNotify)( IN C_U8 uFlag, IN char const *szBackURL );

/**
 * @brief  ע���Ʒ����˳��ص�����
 * @param  notifyFunc  �ص�����ָ��
 * @note
           �ú���������Cloud_Init֮��Cloud_Start֮ǰ���ã�
           ���û�Ҫ�˳��Ʒ���ʱ����ֲ������ע��Ļص�����֪ͨ������
 */
void Cloud_RegisterExitCallback( IN CStb_CloudExitNotify notifyFunc );

/**
 * @brief ���̵��Խӿ�,����ͨ���ⲿ����ķ�ʽ������ֲ��ĵ��ԡ�
 * @param szCmd ���������ַ�����
 * @return �������ý��
 * @retval   CLOUD_OK               �ɹ�
 * @retval   CLOUD_FAILURE          ʧ��
 * @return �������ý��
 * @retval   CLOUD_OK                  �ɹ�
 * @retval   CLOUD_FAILURE          ʧ��
 */
C_RESULT Cloud_DebugSet ( IN char*  szCmd );

/**
 * @brief �����Ż�
 * @note  �ն˵��ô˺������Ʒ��񷵻ص��Ż���
 * @return �����Ż����
 * @retval   CLOUD_OK               �ɹ�
 * @retval   CLOUD_FAILURE          ʧ��
 */
C_RESULT Cloud_BackHome(void);

/**
 * @brief  ��ȡ�Ʒ���״̬
 * @return �����Ʒ���״̬
 */
C_Status Cloud_GetStatus(void);


/***************** ����Ϊ��ֲ��ʵ�֣��ṩ���ն�ʹ�õĺ����ӿ�  *************************/

/***************** ����Ϊ�ն�ʵ�֣��ṩ����ֲ��ʹ�õĺ����ӿ�  *************************/

 ///�����ӿ�
/**
 * @brief ��ӡ�����Ϣ
 * @param   szFmt   �����Ϣ��ʽ�����ı��������ʽ�μ���׼C��printf()����
 * @param   ��       �����Ϣ�еĲ����б�
 */
void CStb_Print( IN char *szFmt, IN ... );

/**
 * @brief ����ָʾ��Ϣ
 * @param   uType       �������ͣ�
 * @param   uLen       ��Ϣ���ֽ�����
 * @param   puKeyData   ָʾ��Ϣ�������ֽ��򣩣�
                        �����������ͣ��������е�����ת������Ӧ�����ݽṹ��
                        C_Keyboard��C_FlingPC����C_HID��
 */                                                 
void CStb_OnKeyOutput(
    IN C_U32        handle,
    IN C_KeyType    uType,
    IN C_U8         uLen,
    IN C_U8*        puKeyData
);

/**
 * @brief ��Ϣ֪ͨ
 * @param uMsgCode  ��Ϣ����
 * @param uMessage  ��ʾ��Ϣ����
 * @param uErrCode  �����룬ʮ����
 * @note 
 				 ��������ֲ���ⲿ���ƽ���Ļ���ʱ����Ҫʵ�ִ˺�����
 				 ��ֲ���ṩ���ն��豸Ӧ�õ���Ϣ֪ͨ���ն�Ӧ����ʵ�ִ˺���ʱ��
      ������Ϣ�롢��ʾ��Ϣ�����Լ�����������Ӧ����ʾ���߲�����
         ��uMsgCodeΪMsgCode_ShowInfoʱ��uMessage�����Ż���Ч����ʱ��
      ���uErrCodeΪ0�����ʾ����ϢΪ��ʾ��Ϣ���ն�ֻ��Ҫ������ʾ���ɣ����uErrCodeΪ��0ֵ��
      ���ʾ����ϢΪ������ʾ����Ҫ�ն���ʾ����ȷ�ϰ�ť����ʾ�򣬲����û�ֻ�ܰ�ȷ�ϼ��˳��Ʒ���        
 				��uMsgCodeΪ����ֵʱ��uMessage��uErrCode�����������ԣ�
 */
void CStb_Notify(
    IN C_MessageCode    uMsgCode,
    IN C_U8            	uMessage[MSG_MAX_LEN],
    IN C_U16            uErrCode
);

/**
 * @brief  ��ȡϵͳ����ʱ�䣬��λΪms��
 * @return �����ն�ϵͳ����ʱ�䣬��λms
 */
C_U32 CStb_GetUpTime();

/**
 * @brief �����ڴ�.�������ָ����С���ڴ�飬���䵽���ڴ�ռ�����������������ڴ档
 * @param uLength ��Ҫ������ڴ��С�����ֽ�Ϊ��λ
 * @return  ���ؽ��
 * @retval  �ɹ�    ���ط����ڴ��ַ
            ʧ��    ����0

 */
void * CStb_Malloc( IN C_U32 uLength );

/**
 * @brief  �ͷ��ڴ�.�ͷ�pBufָ�����ڴ�ռ䣬���Ҹ��ڴ�ռ��������CStb_Malloc����ġ�
 * @param pBuf ��Ҫ�ͷŵ��ڴ�ռ�ĵ�ַ
 */
void CStb_Free(IN void * pBuf);


#define Priority_Lowest    0 ///<������ȼ�.
#define Priority_Low       1 ///<�����ȼ�.
#define Priority_Normal    2 ///<�������ȼ�.
#define Priority_High      3 ///<�����ȼ�.
#define Priority_Highest   4 ///<������ȼ�.
/**
 * @brief �����߳�.���ն��豸���봴��������һ���̵߳����С�
 * @param  pThreadHandle     ָ���߳̾����ָ��
 * @param  szName            �߳�����
 * @param  uPriority         �̵߳����ȼ���
                const U8  Priority_Lowest  =  0;    ///<������ȼ�.
                const U8  Priority_Low     =  1;    ///<�����ȼ�.
                const U8  Priority_Normal  =  2;    ///<�������ȼ�.
                const U8  Priority_High    =  3;    ///<�����ȼ�.
                const U8  Priority_Highest =  4;    ///<������ȼ�.
 * @param  pThreadFunc	    �̺߳�����ַ
 * @param  pParam           �̺߳�������
 * @param  uStackSize       �߳�ջ�Ĵ�С
 * @return  ���ؽ��
 * @retval  CLOUD_OK	      �ɹ�
            CLOUD_FAILURE	  ʧ��
 */
C_RESULT CStb_CreateThread (
    OUT C_ThreadHandle  *pThreadHandle,
    IN char const*      szName,
    IN C_U8             uPriority,
    IN void             (*pThreadFunc)(void*),
    IN void *           pParam,
    IN C_U16            uStackSize
);

/**
 * @brief ɾ���߳�
 * @param hThreadHandle �߳̾��
 */
void CStb_DeleteThread ( IN  C_ThreadHandle   hThreadHandle );

/**
 * @brief ����.���뵱ǰ�߳�����һ��ʱ�䡣
 * @param uMicroSeconds   �߳�����ʱ�䣬��λΪ΢��
 */
void CStb_Sleep( IN C_U32 uMicroSeconds );

/**
 * @brief �����ź���
        ���봴���ź������������ź�������ʼ����ֵ��
 * @param  pHandle  ָ������ź��������ָ��
 * @param  uCount   �ź����ĳ�ʼ����ֵ
 * @return ���ؽ��
 * @retval CLOUD_OK         �ɹ�
           CLOUD_FAILURE    ʧ��
 * @note ��ֲ��ʹ�ü����ź������ƶԹ�����Դ�ķ��ʣ�
        ���ź�����������0ʱ���̲߳���ʹ����Դ������ȴ�״̬��
        ���ź�����������0ʱ���߳̿���ʹ����Դ����ͬʱ���ź�����1��
 */
C_RESULT CStb_CreateSemaphore (
    OUT C_SemaphoreHandle *pHandle,
    IN  C_U32              uCount
);

/**
 * @brief ���ź������ź�
 * @param  hHandle  �ź������
 */
void CStb_SemaphoreSignal( IN C_SemaphoreHandle   hHandle );

/**
 * @brief �ȴ��ź���,���ź�����������0ʱ�ȴ��ɹ����ź���������1��
 * @param   hHandle	 �ź������
 * @param   uTimeout �ȴ��ź����ĳ�ʱʱ�䣬��λΪ����
                    ���uTimeout = 0xFFFFFFFF�����ʾ������ʱ��ֱ������ź���Ϊֹ��
 * @return ���ؽ��
 * @retval CLOUD_OK         �ɹ�
           CLOUD_TIMEOUT    ��ʱ
           CLOUD_FAILURE    ʧ��
 */
C_RESULT CStb_SemaphoreWait(
    IN C_SemaphoreHandle   hHandle,
    IN C_U32               uTimeout
);

/**
 * @brief ɾ���ź���
 */
void CStb_DeleteSemaphore( IN C_SemaphoreHandle   hHandle );

///����ӿ�
/**
 * @brief  �����׽���
 * @param  pSocketHandle  �׽��־��
 * @param  uType          �׽�������
 * @param  bIsBlocking    �Ƿ񴴽������׽��� 
 * @return ���ؽ��
 * @retval CLOUD_OK        �ɹ�
           CLOUD_FAILURE   ʧ��
 */
C_RESULT CStb_SocketOpen(
    OUT C_SocketHandle     *pSocketHandle,
    IN  C_SocketType       uType,
    IN  C_BOOL             bIsBlocking
);

/**
 * @brief  ��Ŀ���׽��ֽ�������
 * @param  pSocketHandle  �׽��־��
 * @param  pDstSocketAddr �׽���Ŀ�ĵ�ַ
 * @return ���ؽ��
 * @retval CLOUD_OK        �ɹ�
           CLOUD_FAILURE   ʧ��
 * @note 
        ���ڷ������׽��֣���������������CLOUD_OK������״̬����ͨ��CStb_SocketSelect��ѯ��
        ���������׽��ָú���Ҫ�ȵ����ӳɹ���ʧ�ܺ�ŷ��ء�
 */
C_RESULT CStb_SocketConnect( 
    IN  C_SocketHandle      hSocketHandle, 
    IN  C_SocketAddr const *pDstSocketAddr
);

/**
 * @brief  �����׽��ֵ�ѡ��
 * @param  pSocketHandle  �׽��־��
 * @param  uLevel         ѡ���Ĳ�Σ�
 * @param  uOptname       ���õ�ѡ��
 * @param  pOptval        ָ����ѡ��ֵ�Ļ�����ָ��
 * @param  uOptlen        ����������
 * @return ���ؽ��
 * @retval CLOUD_OK        �ɹ�
           CLOUD_FAILURE   ʧ��
 */
C_RESULT CStb_SocketSetOpt(
    IN  C_SocketHandle    hSocketHandle,
    IN  C_SocketOptLevel  uLevel, 
    IN  C_SocketOptName   uOptname,
    IN  C_U8  const *     pOptval, 
    IN  C_U32             uOptlen
);

/**
 * @brief  ����������IP��ַ
 * @param  pDomainName  �����ַ���
 * @return ����IP��ַ
 */
C_NetworkIP CStb_SocketGetHostByName( IN  char const *pDomainName );

/**
 * @brief  ���һ�������׽���״̬
 * @param  pReadSockets         ����:Ҫ��������׽�������;����:��״̬�仯���׽�������
 * @param  uReadSocketCount     Ҫ���ж������׽��ָ���
 * @param  pWriteSockets        ����:Ҫ��д����׽�������;����:д״̬�仯���׽�������
 * @param  uWriteSocketCount    Ҫ����д�����׽��ָ��� 
 * @param  pExceptSockets       ����:Ҫ���������׽�������;����:������׽�������
 * @param  uExceptSocketCount   Ҫ���д�������׽��ָ���  
 * @param  uTimeout             ��ʱʱ�䣬��λ����
 * @return ���ؽ��
 * @retval CLOUD_OK        �ɹ�
           CLOUD_TIMEOUT   ��ʱ
 * @note  ����pReadSockets,pWriteSockets��pExceptSockets�᷵��״̬�仯���׽�������,
      ״̬û�仯���׽��ֽ�������ΪNULL��
 */
C_RESULT CStb_SocketSelect(      
    INOUT C_SocketHandle *pReadSockets,
    IN    C_U32          uReadSocketCount,
    INOUT C_SocketHandle *pWriteSockets,
    IN    C_U32          uWriteSocketCount,
    INOUT C_SocketHandle *pExceptSockets, 
    IN    C_U32          uExceptSocketCount,    
    IN    C_U32          uTimeout 
);

/**
 * @brief �������׽��ַ�������
 * @param  hSocketHandle  �׽��־��
 * @param  pBuf	          ����Buffer
 * @param  uBytesToSend	  Ҫ���͵��ֽ���
 * @param  puBytesSent	  ʵ�ʷ��͵��ֽ���
 * @return ���ؽ��
 * @retval CLOUD_OK       �ɹ�
           CLOUD_FAILURE  ʧ��
 */
C_RESULT  CStb_SocketSend(
    IN  C_SocketHandle   hSocketHandle,
    IN  C_U8 const       *pBuf,
    IN  C_U32            uBytesToSend,
    OUT C_U32            *puBytesSent
);

/**
 * @brief �������׽��ֽ�������
 * @param  hSocketHandle    �׽��־��
 * @param  pBuf             ����Buffer
 * @param  uBytesToReceive  Ҫ���յ��ֽ���
 * @param  puBytesReceived  ʵ�ʽ��յ����ֽ���
 * @return ���ؽ��
 * @retval CLOUD_OK        �ɹ�
           CLOUD_TIMEOUT   ��ʱ
           CLOUD_FAILURE   ʧ��
 */
C_RESULT  CStb_SocketRecv(
    IN  C_SocketHandle  hSocketHandle,
    OUT C_U8            *pBuf,
    IN  C_U32           uBytesToReceive,
    OUT C_U32           *puBytesReceived
);

/**
 * @brief �������׽��ַ�������
 * @param  hSocketHandle  �׽��־��
 * @param  pSocketAddr	  Ŀ�ĵص�ַ
 * @param  pBuf           ����Buffer
 * @param  uBytesToSend	  Ҫ���͵��ֽ���
 * @param  puBytesSent	  ʵ�ʷ��͵��ֽ���
 * @return ���ؽ��
 * @retval CLOUD_OK           �ɹ�
           CLOUD_FAILURE      ʧ��
 * @note ������Ҫ���׽����������ӵġ�
 */
C_RESULT  CStb_SocketSendTo(
    IN  C_SocketHandle      hSocketHandle,
    IN  C_SocketAddr const  *pSocketAddr,
    IN  C_U8 const          *pBuf,
    IN  C_U32               uBytesToSend,
    OUT C_U32               *puBytesSent
);

/**
 * @brief �������׽��ֽ�������,�������ݱ���������Դ��ַ��
 * @param  hSocketHandle    �׽��־��
 * @param  pSocketAddr	    Դ��ַ
 * @param  pBuf             ����Buffer
 * @param  uBytesToReceive  Ҫ���յ��ֽ���
 * @param  puBytesReceived  ʵ�ʽ��յ����ֽ���
 * @param  uTimeout         �������ݵĳ�ʱʱ��,��λ����
 * @return ���ؽ��
 * @retval CLOUD_OK        �ɹ�
           CLOUD_TIMEOUT   ��ʱ
           CLOUD_FAILURE   ʧ��
 * @note ������Ҫ���׽����������ӵġ�
 */
C_RESULT  CStb_SocketReceiveFrom(
    IN  C_SocketHandle  hSocketHandle,
    OUT C_SocketAddr    *pSocketAddr,
    OUT C_U8                *pBuf,
    IN  C_U32                   uBytesToReceive,
    OUT C_U32               *pBytesReceived
);

/**
 * @brief �����׽���
 * @param  hSocketHandle  �׽��־��
 * @return ���ؽ��
 * @retval CLOUD_OK         �ɹ�
           CLOUD_FAILURE    ʧ��
 */
C_RESULT  CStb_SocketClose( IN  C_SocketHandle  hSocketHandle );

///���ݴ洢�ӿ�
/**
 * @brief ��ȡ�洢�ռ�,��ȡFlash�洢�ռ��е����ݡ�
 * @param  uBlockID         �洢�ռ�ID
                            CLOUD_FLASH_BLOCK_A:��һ��64KBytes�洢�ռ�Block��
                            CLOUD_FLASH_BLOCK_B:�ڶ���256KBytes�洢�ռ�Block��
                            ����:��Ч��
 * @param  pBuf             ����Buffer
 * @param  uBytesToRead     Ҫ��ȡ���ֽ���
 * @param  puBytesRead      ʵ�ʶ�ȡ���ֽ��� 
 * @return   ���ض�ȡ���
 * @retval   CLOUD_OK          �ɹ�
             CLOUD_FAILURE     ʧ��
 * @note ÿ�ζ�ȡ����ʼλ����ͬ����Ϊ��ָ�������ʼλ��
 */
C_RESULT  CStb_FlashRead( 
    IN  C_U8            uBlockID,
    OUT C_U8            *pBuf,
    IN  C_U32           uBytesToRead,
    OUT C_U32           *puBytesRead
);

/**
 * @brief д��洢�ռ�,��Flash�洢�ռ���д�����ݡ�
 * @param  uBlockID         �洢�ռ�ID
                            CLOUD_FLASH_BLOCK_A:��һ��64KBytes�洢�ռ�Block��
                            CLOUD_FLASH_BLOCK_B:�ڶ���256KBytes�洢�ռ�Block��
                            ����:��Ч��
 * @param  pBuf             ����Buffer
 * @param  uBytesToWrite    Ҫд����ֽ���
 * @return   ���ض�ȡ���
 * @retval   CLOUD_OK          �ɹ�
             CLOUD_FAILURE     ʧ��
 * @note ÿ��д�����ʼλ����ͬ����Ϊ��ָ�������ʼλ��
 */
C_RESULT  CStb_FlashWrite ( 
    IN  C_U8            uBlockID,
    IN  C_U8 const      *pBuf,
    IN  C_U32           uBytesToWrite
);

///DEMUX������ȡ
/**
 * @brief �ӹ���������ȡָ��PID��һ��section���ݡ�
 * @param    pFreq      Ƶ�����ָ��
 * @param    uPID       ���ݵ�PID
 * @param    uTableId   Table id
 * @param    pBuf       �洢section���ݵĻ�����ָ��
 * @param    uBufMaxLen �洢section���ݵĻ����������ߴ�
 * @param    pBytesReceived ���յ���section���ݵ�ʵ�ʳ���
 * @param    uTimeout       �������ݵĳ�ʱʱ��,��λ����
 * @return   ���ؽ��
 * @retval   CLOUD_OK          �ɹ�
             CLOUD_TIMEOUT	   ��ʱ
             CLOUD_FAILURE     ʧ��
 * @note �˺���ֻ��Cable�ն���Ҫʵ�֡�
 */
C_RESULT  CStb_DemuxReceiveData (
    IN    C_FreqParam  const    *pFreq,
    IN    C_U16                 uPID,
    IN    C_U8                  uTableId,
    OUT   C_U8                  *pBuf,
    IN    C_U32                 uBufMaxLen,
    OUT   C_U32                 *pBytesReceived,
    IN    C_U32                 uTimeout
);

///���Žӿ�
/**
 * @brief ͨ��PID��������Ƶ
 * @param pFreq         Ƶ�����ָ��
 * @param pPidParams    PID�������ָ��
 * @return   ���ؽ��
 * @retval   CLOUD_OK          �ɹ�
             CLOUD_FAILURE     ʧ��
 * @note �˺���Ҫ����ٷ��أ�Ӧ�ÿɽ���Ϣ��¼���������д���
         �����ڴ˺����ж���Ϣ�����������˺���ֻ��Cable�ն���Ҫʵ�֡�
 */
C_RESULT CStb_AVPlayByPid( IN C_FreqParam  const *pFreq, IN C_PIDParam  const *pPidParams );

/**
 * @brief ͨ����Ŀ�Ų�������Ƶ
 * @param pFreq     Ƶ�����ָ��
 * @param uProgNo   ��Ŀ��
 * @return   ���ؽ��
 * @retval   CLOUD_OK          �ɹ�
             CLOUD_FAILURE     ʧ��
 * @note �˺���Ҫ����ٷ��أ�Ӧ�ÿɽ���Ϣ��¼���������д���
         �����ڴ˺����ж���Ϣ�����������˺���ֻ��Cable�ն���Ҫʵ�֡�
 */
C_RESULT CStb_AVPlayByProgNo( IN C_FreqParam  const *pFreq, IN C_U32 uProgNo );

///AV���Žӿ�(Cable�ն�)
/**
 * @brief ��������Ƶ��CW
 * @param uType     ��������ֶ�Ӧ������
                    0:��Ƶ��
                    1:��Ƶ��
                    ����:��Ч
 * @param pEven     ż���ڽ�����
 * @param pOdd      �����ڽ�����
 * @param uKeyLen   �����ֵĳ���
 * @return   ���ؽ��
 * @retval   CLOUD_OK          �ɹ�
             CLOUD_FAILURE     ʧ��
 * @note �ú����ڲ��ź���֮�����(CStb_AVPlayByPid����CStb_AVPlayByProgNo)������uType�ҵ���Ӧ��
         ��ƵPID������ƵPID����������Ӧ�Ľ����֡�
 */
C_RESULT CStb_AVSetCW(
    IN C_U8         uType, 
    IN C_U8 const   *pEven, 
    IN C_U8 const   *pOdd,
    IN C_U8         uKeyLen
);


///������Ҫ���Ƕ���̵��������ˣ�����ģ����Ҫ������װ��һ���⣬��Ҫͨ�����ɷ�����Э���Ĳ���
///���ݸ����Ž����У���ͨ�����ſ���н��ܵĲ�����

/**
 * @brief ע��TS�����ݻص�����
 * @param pInjectData   ����ָ��
 * @param uDataLen      ���ݳ���
 * @return   ���ؽ��
 * @retval   CLOUD_OK          �ɹ�
             CLOUD_FAILURE     ʧ��
 * @note �˺���ע��TS�����ݣ�ֻ����Ҫͨ��TS����������Ƶ���ն���Ҫʵ�֡�
         �˺���Ҫ����ٷ��أ��ڲ����ö��л������ݣ������ڴ˺����ж���Ϣ����������
 */
typedef C_RESULT (*CStb_AVInjectTSData)( IN C_U8 *pInjectData, IN C_U32 uDataLen );

/**
 * @brief ��TS����������Ƶ
 * @param pPidParams        PID�������ָ��
 *        InjectTSDataFun   ע��TS���ݻص�����
 * @return   ���ؽ��
 * @retval   CLOUD_OK          �ɹ�
             CLOUD_FAILURE     ʧ��
 * @note �˺�����ɲ��ų�ʼ��������ʼ�ȴ�����TS�����ݣ�ֻ����Ҫͨ��TS����������Ƶ���ն���Ҫʵ�֡�
         �˺���Ҫ����ٷ��أ������ڴ˺����ж���Ϣ����������
 */
C_RESULT CStb_AVPlayTSOverIP(IN C_PIDParam const *pPidParams, OUT CStb_AVInjectTSData *InjectTSDataFun);

/**
 * @brief ������������Χ0-100֮�䣬0��ʾ��С������100��ʾ�������
 * @param   uVol    ����ֵ�����ֵ����0-100֮�䣬�򷵻�ʧ��
 * @return   ���ؽ��
 * @retval   CLOUD_OK          �ɹ�
             CLOUD_FAILURE     ʧ��
 * @note
        �Ʒ����������Χ��0-100�����ɳ�����Ҫ�����Լ�ƽ̨���ص������Ӧ��ת����
 */
C_RESULT CStb_SetVolume( IN C_U8 uVol );

/**
 * @brief ��ȡ��ǰ����,��Χ0-100֮�䣬0��ʾ��С������100��ʾ�������
 * @return   ���ص�ǰ����
 * @note
        �ú����������Ʒ���ʱ�ᱻ���ã����ڱ����Ʒ���ĳ�ʼ�����ͽ���ǰһ�¡�
        �뽫�����Ʒ���ǰ������ת��Ϊ0-100��Χ�ڵ�ֵ�����Ʒ���
 */
C_U8 CStb_GetVolume(void);

/**
 * @brief ���þ���״̬
 * @param   bMuteStatus     C_TRUE  ����
                            C_FALSE �Ǿ���
 * @return   ���ؽ��
 * @retval   CLOUD_OK          �ɹ�
             CLOUD_FAILURE     ʧ��
 * @note
        �Ʒ���ͨ���ú������þ���״̬�����ɳ��ҿ��Լ�¼��״̬�����˳��Ʒ���ʱ��
        ������״̬��������������Ӧ�á�
 */
C_RESULT CStb_SetMuteState( IN C_BOOL bMuteStatus );

/**
 * @brief ��ȡ��ǰ����״̬
 * @return   ���ص�ǰ����״̬
 * @retval   C_TRUE     ����
             C_FALSE    �Ǿ���
 * @note
        �ú����������Ʒ���ʱ�ᱻ���ã����ڻ�û����н����Ʒ���ǰ�ľ���״̬��
 */
C_BOOL CStb_GetMuteState(void);

/**
 * @brief ������Ƶ����λ��
 * @param    uX         ��ʼ�������
 * @param    uY         ��ʼ��������
 * @param    uWidth     ��Ƶ���ڵĿ��
 * @param    uHeight    ��Ƶ���ڵĸ߶�

 * @return   ���ؽ��
 * @retval   CLOUD_OK          �ɹ�
             CLOUD_FAILURE     ʧ��
 * @note
        ��������Ϊ��Ƶ�������ʾ���������غϣ�����Ϊ�����ͼ����ʾ��������ꣻ
        ����������겻�غϣ���Ҫ���ɳ��ҽ�����Ӧ��ת����
 */
C_RESULT CStb_SetVideoWindow
(
    IN C_U32    uX,
    IN C_U32    uY,
    IN C_U32    uWidth,
    IN C_U32    uHeight
);

/**
 * @brief ֹͣ����
 */
void CStb_AVStop(void);

/**
 * @brief ����ź�����
 * @param   pSignal  �ź�������Ϣ
 * @return  ���ؽ��
 * @retval  CLOUD_OK          �ɹ�
            CLOUD_FAILURE     ʧ��
 * @note ��Cable�ն˲�������Ƶʱ����ֲ��ᶨ�ڵ��øú�����ȡ��ǰTS���ź�������
 */
C_RESULT CStb_GetTsSignalInfo( OUT C_TsSignal *pSignal );

/**
 * @brief ��ȡ������
 * @param   pRate    ��ǰ������ = pRate/10000
 * @return   ���ؽ��
 * @retval   CLOUD_OK          �ɹ�
             CLOUD_FAILURE     ʧ��
 * @note ��IP�ն˲�������Ƶʱ����ֲ��ᶨ�ڵ��øú�����ȡ��ǰ�����ʡ�
 */
C_RESULT CStb_GetPacketLossRate ( OUT C_U32 *pRate );

/**
 * @brief ��ý�����״̬
 * @param uDecoderType  ���������ͣ�
                        const   U8  Video_Decoder  = 1; ///<��Ƶ������
                        const   U8  Audio_ Decoder = 2; ///<��Ƶ������
 * @return ���ؽ�����״̬
 * @note �ú����ĵ���Ƶ��Ϊ1��һ�Ρ�

 */
C_DecoderStatusCode CStb_GetDecoderStatus (  IN C_U8 uDecoderType  );

///ͼ����ʾ�ӿ�
/**
 * @brief  ��ȡ�ն���ʾ�������Ϣ��
 * @param pBuff ��Ƶ�Ʒ�����ʾ�������ĵ�ַ���˻������Ĵ�С�Ǹ��ݿ����Լ���ɫģʽ��ͬȷ���ģ�
 		������Ƶ�Ʒ���Ľ�����ʾ����Ƶ�Ʒ����ڴ˻������и��������ݺ󣬻����"CStb_UpdateRegion"����֪ͨ�����Դ档
    ����û�������ַΪ�Դ��ַ������Ҫʵ��"CStb_UpdateRegion"�����������û����ܻῴ��������ƹ��̡�
    ���Ʒ���ʹ�ù����У��˻��������ܱ��ͷš�
    ���˳��Ʒ���ʱ���˻��������Ա��ͷš�
 * @param pWidth ��ʾ����Ŀ��
 * @param pHeight ��ʾ����ĸ߶�
 * @param pColorMode ��ɫģʽ
 * @note ���鲻Ҫֱ�ӷ����Դ��ַ��

 */
void CStb_GraphicsGetInfo(
    OUT C_U8             **pBuff,
    OUT C_U32           *pWidth,
    OUT C_U32        *pHeight,
    OUT C_ColorMode  *pColorMode
);

/**
 * @brief  ������Ļָ������
 * @param uX    ������Ļ�ľ��������x ��λ��
 * @param uY    ������Ļ�ľ��������y ��λ��
 * @param uWidth    ������Ļ�ľ�������Ŀ��
 * @param uHeight   ������Ļ�ľ�������ĸ߶�
 * @return   ���ؽ��
 * @retval   CLOUD_OK          �ɹ�
             CLOUD_FAILURE     ʧ��

 */
C_RESULT CStb_UpdateRegion(
    IN C_U32    uX,
    IN C_U32    uY,
    IN C_U32    uWidth,
    IN C_U32    uHeight
);

/**
 * @brief �����Ļ,��������ʾ������Ϊ͸����
 */
void CStb_CleanScreen( void );

/***************** ����Ϊ�ն�ʵ�֣��ṩ����ֲ��ʹ�õĺ����ӿ�  *************************/

#define CLOUD_LOG_TRACE(x...) \
	do{	\
		fprintf(stdout, "[%s,%d]", __FUNCTION__, __LINE__);	\
		fprintf(stdout, x);		\
	}while(0)


int CStb_CyberCloudTaskStart(char* cloud_url);
void CStb_CyberCloudTaskStop(void);

C_BOOL CStb_CloudKeyLoopIsEnable(void);
C_BOOL CStb_SendKey2CyberCloud(int status, int keyValue);

void cybercloud_flash_init(void);
void Device_Callback_Superhid(void);
void CStb_PreviousGraphicsPosSet(void);

#ifdef __cplusplus
}
#endif

#endif //CYBER_CLOUD_API_H


