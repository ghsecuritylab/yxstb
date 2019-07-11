/*************************************************
  Copyright (C), 1988-1999, Huawei Tech. Co., Ltd.
  File name:      mdi_derived_data_types.h    
  Author:          ALOK GUPTA  
  Version:         
  Date:         08 Aug. 2008
  Description:  It all the derived datatypes for MDICBB
  Others:         
  Function List: None
*************************************************/

#ifndef __MDI_DERIVED_DATA_TYPES_H__

#define  __MDI_DERIVED_DATA_TYPES_H__

#include "mdi_primitive_data_types.h"

/**
* @defgroup MDI_DERIVED_DATA_TYPES
* This section describes all the derived data types in MDI-CBB.
*/
/**
* @defgroup Enums
* @ingroup MDI_DERIVED_DATA_TYPES
* This section describes all the enums in MDI-CBB.
*/
/**
* @defgroup Structures
* @ingroup MDI_DERIVED_DATA_TYPES
* This section describes the structure in MDI-CBB.
*/
/**
* @defgroup MDI_DF_COMPUTATION_METHOD
* @ingroup Enums
* @par Prototype
* @code
* typedef enum MDI_DF_COMPUTATION_METHOD 
* { AVG_PCR_METHOD, MAX_PCR_METHOD} MDI_DF_COMPUTATION_METHOD;
* @endcode
* @par Purpose
* This enum lists the different methods to perform DF computation.
* @datastruct AVG_PCR_METHOD  0 (integer value): First MDI Computation method, 
    which takes the Avg PCR values in the interval. 
* @datastruct MAX_PCR_METHOD  1 (integer value): Second MDI Computation method, 
   which takes the Maximum PCR value from the interval. 
* 
*/
typedef enum MDI_DF_COMPUTATION_METHOD 
{ 
    AVG_PCR_METHOD, 
    MAX_PCR_METHOD
}MDI_DF_COMPUTATION_METHOD;

/**
* @defgroup MDI_MLR_COMPUTATION_METHOD
* @ingroup Enums
* @par Prototype
* @code
* typedef enum MDI_MLR_COMPUTATION_METHOD 
* { CC_METHOD, EXPCTD_MR_METHOD}  MDI_MLR_COMPUTATION_METHOD;
* @endcode
* @par Purpose
* This enum lists the different methods to perform MLR computation.
* @datastruct CC_METHOD  0 (integer value): First MLR Computation method, 
    which takes the CC values for MLR computation. 
* @datastruct EXPCTD_MR_METHOD  1 (integer value): Second MLR Computation 
    method, which takes the Expected MR values for MLR computation. 
* 
*/
typedef enum MDI_MLR_COMPUTATION_METHOD 
{ 
    CC_METHOD, 
    EXPCTD_MR_METHOD
}MDI_MLR_COMPUTATION_METHOD;

/**
* @defgroup MDI_COPY_BUFFER
* @ingroup Enums
* @par Prototype
* @code
* typedef enum MDI_COPY_BUFFER
* { COPY, NOT_CPY} MDI_COPY_BUFFER;
* @endcode
* @par Purpose
* This enum lists the (configuration values) for Need Buffer, 
    whether it will copy the contents or not.
* @datastruct COPY 0 (integer value): 
    Copy the contents to the Local buffer and free the Local buffer after use. 
* @datastruct NOT_CPY 1 (integer value): Use the pointer from the 
    Application buffer and free the Application buffer after use. 
*  
*/
typedef enum MDI_COPY_BUFFER
{
    COPY, 
    NOT_CPY
}MDI_COPY_BUFFER;

/**
* @defgroup MDI_LOGLEVELS
* @ingroup Enums
* @par Prototype
* @code
* typedef enum MDI_LOGLEVELS 
* {MDI_NONE, MDI_FATAL,MDI_ERROR,MDI_WARN,
* MDI_PRODUCT_L2, MDI_PRODUCT_L1, MDI_DEV_TEST}MDI_LOGLEVELS;
* @endcode
* @par Purpose
* This enum lists the configuration values for the different log levels.
* @datastruct MDI_NONE 0 (integer value): No Logs 
* @datastruct MDI_FATAL 1 (integer value): Only FATAL can be logged. 
* @datastruct MDI_ERROR 2 (integer value): Only FATAL and ERROR can be logged. 
* @datastruct MDI_WARN 3 (integer value): Only FATAL, ERROR, and WARN
    can be logged. 
* @datastruct MDI_PRODUCT_L2 4 (integer value): Only FATAL, ERROR, WARN, 
    and INFO can be logged. 
* @datastruct MDI_PRODUCT_L1 5 (integer value): Only FATAL, ERROR, WARN, 
    INFO, and DEBUG can be logged.  
* @datastruct MDI_DEV_TEST 6 (integer value): Application will not be able to 
    set this log level, it is internal for the development purpose .
* If the log level is set to MDI_DEV_TEST, initialization happens, but no logs 
    are generated corresponding to this log level.
* 
*/
typedef enum MDI_LOGLEVELS 
{
    MDI_NONE, 
    MDI_FATAL,
    MDI_ERROR,MDI_WARN,
    MDI_INFO, MDI_DEBUG, 
    MDI_PRODUCT_L2, MDI_PRODUCT_L1, 
    MDI_DEV_TEST
}MDI_LOGLEVELS;


typedef enum SQA_CONTENT_TYPE
{ 
    RTP_NORMAL = 1, 
    RTP_RET_RESUME = 2,
    RTP_FEC_RESUME = 3
}SQA_CONTENT_TYPE;
/**
* @defgroup MDI_CONTENT_TYPE
* @ingroup Enums
* @par Prototype
* @code
* typedef enum MDI_CONTENT_TYPE
* { MDI_IP, MDI_RTP, MDI_TS} MDI_CONTENT_TYPE;
* @endcode
* @par Purpose
* This enum lists the configuration values for the different parameters of 
    SendMsg.
* @datastruct MDI_IP 0 (integer value): Raw IP packet.  
* @datastruct MDI_RTP 1 (integer value):Input packet contains RTP content.   
* @datastruct MDI_TS 2 (integer value): Input packet contains TS content. 
* 
*/
typedef enum MDI_CONTENT_TYPE
{ 
    MDI_IP, 
    MDI_RTP, MDI_TS
}MDI_CONTENT_TYPE;

/**
* @defgroup MDI_PACKET_TYPE
* @ingroup Enums
* @par Prototype
* @code
* typedef enum MDI_PACKET_TYPE 
* { MDI_IPPACKET, MDI_NONIPPACKET} MDI_PACKET_TYPE ;
* @endcode
* @par Purpose
* This enum lists the configuration values for the different kind of packets
    in SendMsg.
* @datastruct MDI_IPPACKET  0 (integer value): IP packet, then in send msg 
    Application has to pass complete Ip packet.  
* @datastruct MDI_NONIPPACKET 1 (integer value): Non IP packet, Application has 
    to pass only payload i.e. either one RTP packet or one TS packet.   
* 
*/
typedef enum MDI_PACKET_TYPE 
{ 
    MDI_IPPACKET, 
    MDI_NONIPPACKET
}MDI_PACKET_TYPE;

/**
* @defgroup MDI_STOP_TYPE
* @ingroup Enums
* @par Prototype
* @code
* typedef enum MDI_STOP_TYPE 
* {GRACEFUL_STOP, ABRUPT_STOP} MDI_STOP_TYPE;
* @endcode
* @par Purpose
* This enum lists the configuration values for the different methods to
    stop MDI.
* @datastruct GRACEFUL_STOP   0 (integer value): This will output 
    all the computed value upto that time and stops the Library.  
* @datastruct ABRUPT_STOP  1 (integer value): This will stop the 
    library immediately.   
* 
*/
typedef enum MDI_STOP_TYPE 
{
    GRACEFUL_STOP, 
    ABRUPT_STOP
}MDI_STOP_TYPE;

/**
* @defgroup MDI_RETURN_VALUE
* @ingroup Enums
* @par Prototype
* @code
* typedef enum MDI_RETURN_VALUE
* {MDI_SUCCESS, MDI_FAILURE} MDI_RETURN_VALUE;
* @endcode
* @par Purpose
* This enum lists the configuration values for the return types of 
    MDI functions.
* @datastruct MDI_SUCCESS 0 (integer value).  
* @datastruct MDI_FAILURE 1 (integer value). 
* 
*/
typedef enum MDI_RETURN_VALUE
{
    MDI_SUCCESS, 
    MDI_FAILURE
}MDI_RETURN_VALUE;


/**
* @defgroup TIMEVAL
* @ingroup Structures
* @par Prototype
* @code
* typedef struct time_val {
* MDI_INT32 tv_sec;
* MDI_INT32 tv_usec;
* } TIMEVAL;
* @endcode
* @par Purpose
* This structure stores the time stamp in seconds and micro seconds.
* @datastruct tv_sec 4 bytes value: Seconds value for the Timestamp .
* @datastruct tv_usec 4 bytes value: Micro Seconds value for the Timestamp .
* 
*/
typedef struct time_val 
{
    MDI_INT32 tv_sec;
    MDI_INT32 tv_usec;
} TIMEVAL;

/**
* @defgroup IPV4
* @ingroup Structures
* @par Prototype
* @code
* typedef struct IPAddressAndPort
* {
* MDI_UINT32 ulSrcIP;		
* MDI_UINT32 ulDestIP;
* MDI_UINT16 unSrcPort;
* MDI_UINT16 unDestPort;
}IPV4;
* @endcode
* @par Purpose
* This structure stores the source and destination address of the channel.
* @datastruct ulSrcIP 4 bytes value: Source IP Address 
* @datastruct ulDestIP 4 bytes value: Destination IP Address 
* @datastruct unSrcPort Source Port
* @datastruct unDestPort Destination Port
*/
/* Structure which stores the source and destination address of the channel */
typedef struct IPAddressAndPort
{
    /* Source IP Address - 4 bytes value  */
    MDI_UINT32 ulSrcIP;
    
    /* Destination IP Address - 4 bytes value */
    MDI_UINT32 ulDestIP;
    
    /* Source Port */
    MDI_UINT16 unSrcPort;
    
    /* Destination Port */
    MDI_UINT16 unDestPort;
}IPV4;


#endif












































