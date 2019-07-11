#ifndef  __H_WIRELESS_DEVICE_H__
#define  __H_WIRELESS_DEVICE_H__

// Ralink defined OIDs
#if WIRELESS_EXT <= 11
#ifndef SIOCDEVPRIVATE
#define SIOCDEVPRIVATE                              0x8BE0
#endif
#define SIOCIWFIRSTPRIV								SIOCDEVPRIVATE
#endif

#define RT_PRIV_IOCTL								(SIOCIWFIRSTPRIV + 0x0E)  
#define RTPRIV_IOCTL_SET							(SIOCIWFIRSTPRIV + 0x02)
#define RTPRIV_IOCTL_GSITESURVEY					(SIOCIWFIRSTPRIV + 0x0D)

// IEEE 802.11 Structures and definitions
//
// new types for Media Specific Indications

#define OID_GET_SET_TOGGLE						            	0x8000
#define OID_802_11_ADD_WEP                          0x0112
#define OID_802_11_REMOVE_WEP                       0x0113
#define OID_802_11_DISASSOCIATE                     0x0114
#define OID_802_11_PRIVACY_FILTER                   0x0118
#define OID_802_11_ASSOCIATION_INFORMATION          0x011E
#define OID_802_11_BSSID_LIST_SCAN                  0x0508
#define OID_802_11_SSID                             0x0509
#define OID_802_11_BSSID                            0x050A
#define OID_802_11_WEP_STATUS                       0x0510
#define OID_802_11_AUTHENTICATION_MODE              0x0511
#define OID_802_11_INFRASTRUCTURE_MODE              0x0512
#define OID_802_11_TX_POWER_LEVEL                   0x0517
#define OID_802_11_REMOVE_KEY                       0x0519
#define OID_802_11_ADD_KEY                          0x0520
#define OID_802_11_BSSID_LIST                       0x0609
#define OID_GEN_MEDIA_CONNECT_STATUS                0x060B
#define RT_OID_802_11_QUERY_LINK_STATUS             0x060C
#define OID_802_11_SET_IEEE8021X_REQUIRE_KEY        0x0618  // For DynamicWEP in IEEE802.1x mode
#define RT_OID_DEVICE_NAME                          0x0607       
#define PACKED  __attribute__ ((packed))


#ifndef ULONG
#define CHAR            char
//#define INT             int
//#define SHORT           int
#define UINT            unsigned int
#define ULONG           unsigned long
#define USHORT          unsigned short
#define UCHAR           unsigned char

#define uint8		  unsigned char
#define u8        unsigned char  
#define LONG            int
#define ULONGLONG       unsigned long long
#endif

// put all proprietery for-query objects here to reduce # of Query_OID
typedef struct _RT_802_11_LINK_STATUS {
    ULONG   CurrTxRate;         // in units of 0.5Mbps
    ULONG   ChannelQuality;     // 0..100 %
    ULONG   TxByteCount;        // both ok and fail
    ULONG   RxByteCount;        // both ok and fail
} RT_802_11_LINK_STATUS, *PRT_802_11_LINK_STATUS;

#define NDIS_802_11_LENGTH_SSID         32
#define NDIS_802_11_LENGTH_RATES        8
#define NDIS_802_11_LENGTH_RATES_EX     16
#define MAX_LEN_OF_SSID                 32
#define MAC_ADDR_LEN                    6

typedef unsigned char   NDIS_802_11_MAC_ADDRESS[6];

// mask for authentication/integrity fields
#define NDIS_802_11_AUTH_REQUEST_AUTH_FIELDS        0x0f

#define NDIS_802_11_AUTH_REQUEST_REAUTH             0x01
#define NDIS_802_11_AUTH_REQUEST_KEYUPDATE          0x02
#define NDIS_802_11_AUTH_REQUEST_PAIRWISE_ERROR     0x06
#define NDIS_802_11_AUTH_REQUEST_GROUP_ERROR        0x0E

// Added new types for OFDM 5G and 2.4G
typedef enum _NDIS_802_11_NETWORK_TYPE
{
    Ndis802_11FH, 
    Ndis802_11DS, 
    Ndis802_11OFDM5,
    Ndis802_11OFDM24,
    Ndis802_11Automode,
    Ndis802_11NetworkTypeMax    // not a real type, defined as an upper bound
} NDIS_802_11_NETWORK_TYPE, *PNDIS_802_11_NETWORK_TYPE;

//
// Received Signal Strength Indication
//
typedef LONG    NDIS_802_11_RSSI;           // in dBm

typedef struct _NDIS_802_11_CONFIGURATION_FH
{
   ULONG           Length;            // Length of structure
   ULONG           HopPattern;        // As defined by 802.11, MSB set 
   ULONG           HopSet;            // to one if non-802.11
   ULONG           DwellTime;         // units are Kusec
} NDIS_802_11_CONFIGURATION_FH, *PNDIS_802_11_CONFIGURATION_FH;

typedef struct _NDIS_802_11_CONFIGURATION
{
   ULONG                           Length;             // Length of structure
   ULONG                           BeaconPeriod;       // units are Kusec
   ULONG                           ATIMWindow;         // units are Kusec
   ULONG                           DSConfig;           // Frequency, units are kHz
   NDIS_802_11_CONFIGURATION_FH    FHConfig;
} NDIS_802_11_CONFIGURATION, *PNDIS_802_11_CONFIGURATION;

typedef  ULONG  NDIS_802_11_KEY_INDEX;
typedef ULONGLONG   NDIS_802_11_KEY_RSC;

// Key mapping keys require a BSSID
typedef struct _NDIS_802_11_KEY
{
    ULONG           Length;             // Length of this structure
    ULONG           KeyIndex;           
    ULONG           KeyLength;          // length of key in bytes
    NDIS_802_11_MAC_ADDRESS BSSID;
    NDIS_802_11_KEY_RSC KeyRSC;
    UCHAR           KeyMaterial[1];     // variable length depending on above field
} NDIS_802_11_KEY, *PNDIS_802_11_KEY;

typedef struct _NDIS_802_11_REMOVE_KEY
{
    ULONG                   Length;        // Length of this structure
    ULONG                   KeyIndex;           
    NDIS_802_11_MAC_ADDRESS BSSID;      
} NDIS_802_11_REMOVE_KEY, *PNDIS_802_11_REMOVE_KEY;

typedef struct PACKED _NDIS_802_11_WEP
{
   ULONG     Length;        // Length of this structure
    ULONG           KeyIndex;           // 0 is the per-client key, 1-N are the
                                        // global keys
   ULONG     KeyLength;     // length of key in bytes
   UCHAR     KeyMaterial[1];// variable length depending on above field
} NDIS_802_11_WEP, *PNDIS_802_11_WEP;


typedef enum _NDIS_802_11_NETWORK_INFRASTRUCTURE
{
   Ndis802_11IBSS,
   Ndis802_11Infrastructure,
   Ndis802_11AutoUnknown,
   Ndis802_11InfrastructureMax     // Not a real value, defined as upper bound
} NDIS_802_11_NETWORK_INFRASTRUCTURE, *PNDIS_802_11_NETWORK_INFRASTRUCTURE;

// PMKID Structures
typedef UCHAR   NDIS_802_11_PMKID_VALUE[16];

typedef struct _BSSID_INFO
{
	NDIS_802_11_MAC_ADDRESS BSSID;
	NDIS_802_11_PMKID_VALUE PMKID;
} BSSID_INFO, *PBSSID_INFO;

typedef struct _NDIS_802_11_PMKID
{
	ULONG Length;
	ULONG BSSIDInfoCount;
	BSSID_INFO BSSIDInfo[1];
} NDIS_802_11_PMKID, *PNDIS_802_11_PMKID;

//Added new types for PMKID Candidate lists.
typedef struct _PMKID_CANDIDATE {
	NDIS_802_11_MAC_ADDRESS BSSID;
	ULONG Flags;
} PMKID_CANDIDATE, *PPMKID_CANDIDATE;

typedef struct _NDIS_802_11_PMKID_CANDIDATE_LIST
{
	ULONG Version;       // Version of the structure
	ULONG NumCandidates; // No. of pmkid candidates
	PMKID_CANDIDATE CandidateList[1];
} NDIS_802_11_PMKID_CANDIDATE_LIST, *PNDIS_802_11_PMKID_CANDIDATE_LIST;

//Flags for PMKID Candidate list structure
#define NDIS_802_11_PMKID_CANDIDATE_PREAUTH_ENABLED	0x01

// Added new encryption types
// Also aliased typedef to new name
typedef enum _NDIS_802_11_WEP_STATUS
{
    Ndis802_11WEPEnabled,
    Ndis802_11Encryption1Enabled = Ndis802_11WEPEnabled,
    Ndis802_11WEPDisabled,
    Ndis802_11EncryptionDisabled = Ndis802_11WEPDisabled,
    Ndis802_11WEPKeyAbsent,
    Ndis802_11Encryption1KeyAbsent = Ndis802_11WEPKeyAbsent,
    Ndis802_11WEPNotSupported,
    Ndis802_11EncryptionNotSupported = Ndis802_11WEPNotSupported,
    Ndis802_11Encryption2Enabled,
    Ndis802_11Encryption2KeyAbsent,
    Ndis802_11Encryption3Enabled,
    Ndis802_11Encryption3KeyAbsent
} NDIS_802_11_WEP_STATUS, *PNDIS_802_11_WEP_STATUS,
  NDIS_802_11_ENCRYPTION_STATUS, *PNDIS_802_11_ENCRYPTION_STATUS;
  
// Add new authentication modes
typedef enum _NDIS_802_11_AUTHENTICATION_MODE
{
   Ndis802_11AuthModeOpen,
   Ndis802_11AuthModeShared,
   Ndis802_11AuthModeAutoSwitch,
   Ndis802_11AuthModeWPA,
   Ndis802_11AuthModeWPAPSK,
   Ndis802_11AuthModeWPANone,
   Ndis802_11AuthModeWPA2,
   Ndis802_11AuthModeWPA2PSK,    
   Ndis802_11AuthModeMax           // Not a real mode, defined as upper bound
} NDIS_802_11_AUTHENTICATION_MODE, *PNDIS_802_11_AUTHENTICATION_MODE;

typedef UCHAR  NDIS_802_11_RATES[NDIS_802_11_LENGTH_RATES];        // Set of 8 data rates
typedef UCHAR  NDIS_802_11_RATES_EX[NDIS_802_11_LENGTH_RATES_EX];  // Set of 16 data rates

typedef struct PACKED _NDIS_802_11_SSID 
{
    ULONG   SsidLength;         // length of SSID field below, in bytes;
                                // this can be zero.
    UCHAR   Ssid[NDIS_802_11_LENGTH_SSID];           // SSID information field
} NDIS_802_11_SSID, *PNDIS_802_11_SSID;


typedef struct PACKED _NDIS_WLAN_BSSID
{
   unsigned long                               Length;     // Length of this structure
   NDIS_802_11_MAC_ADDRESS             MacAddress; // BSSID
   UCHAR                               Reserved[2];
   NDIS_802_11_SSID                    Ssid;       // SSID
   ULONG                               Privacy;    // WEP encryption requirement
    NDIS_802_11_RSSI                    Rssi;               // receive signal
                                                            // strength in dBm
   NDIS_802_11_NETWORK_TYPE            NetworkTypeInUse;
   NDIS_802_11_CONFIGURATION           Configuration;
   NDIS_802_11_NETWORK_INFRASTRUCTURE  InfrastructureMode;
   NDIS_802_11_RATES                   SupportedRates;
} NDIS_WLAN_BSSID, *PNDIS_WLAN_BSSID;

typedef struct PACKED _NDIS_802_11_BSSID_LIST
{
   unsigned long           NumberOfItems;      // in list below, at least 1
   NDIS_WLAN_BSSID Bssid[1];
} NDIS_802_11_BSSID_LIST, *PNDIS_802_11_BSSID_LIST;

// Added Capabilities, IELength and IEs for each BSSID
typedef struct PACKED _NDIS_WLAN_BSSID_EX
{
    unsigned long                               Length;             // Length of this structure
    NDIS_802_11_MAC_ADDRESS             MacAddress;         // BSSID
    unsigned char                               Reserved[2];
    NDIS_802_11_SSID                    Ssid;               // SSID
    unsigned long                               Privacy;            // WEP encryption requirement
    NDIS_802_11_RSSI                    Rssi;               // receive signal
                                                            // strength in dBm
    NDIS_802_11_NETWORK_TYPE            NetworkTypeInUse;
    NDIS_802_11_CONFIGURATION           Configuration;
    NDIS_802_11_NETWORK_INFRASTRUCTURE  InfrastructureMode;
    NDIS_802_11_RATES_EX                SupportedRates;
    unsigned long                               IELength;
    unsigned char                               IEs[1];
} NDIS_WLAN_BSSID_EX, *PNDIS_WLAN_BSSID_EX;

typedef struct _NDIS_802_11_BSSID_LIST_EX
{
    unsigned long                   NumberOfItems;      // in list below, at least 1
    NDIS_WLAN_BSSID_EX      Bssid[1];
} NDIS_802_11_BSSID_LIST_EX, *PNDIS_802_11_BSSID_LIST_EX;

typedef struct _NDIS_802_11_FIXED_IEs 
{
    UCHAR Timestamp[8];
    USHORT BeaconInterval;
    USHORT Capabilities;
} NDIS_802_11_FIXED_IEs, *PNDIS_802_11_FIXED_IEs;


#endif /* End of #ifndef  __H_WIRELESS_DEVICE_H__ */
