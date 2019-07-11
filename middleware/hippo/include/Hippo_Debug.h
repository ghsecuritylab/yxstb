#ifndef HIPPO_DEBUG_H_
#define HIPPO_DEBUG_H_

#include <stdio.h>

#if 1
#define TDY_DEBUG( fmt, ... )	\
	do{	\
        break; \
	}while( 0 )

#define HIPPO_DEBUG( fmt, ... )	\
    do{	\
        break; \
	}while( 0 )


#define HIPPO_ASSERT(assertion)     \
    do{ \
        break; \
    }while( 0 )

#else
#define TDY_DEBUG( fmt, ... )	\
	do{	\
		 printf( "\r[%4d:clock=%lu:%s] ", __LINE__, a_Ticker_get_millis( ), __PRETTY_FUNCTION__ ); \
		 printf( fmt, ##__VA_ARGS__ );	\
	}while( 0 )

#define HIPPO_DEBUG( fmt, ... )	\
    do{	\
		 printf( "\r[%4d:clock=%lu:%s] ", __LINE__, a_Ticker_get_millis( ), __PRETTY_FUNCTION__ ); \
		 printf( fmt, ##__VA_ARGS__ );	\
	}while( 0 )


#define HIPPO_ASSERT(assertion)     \
    do{ \
        if( !(assertion )){ \
            HIPPO_DEBUG( "Run here, error." ); \
        }   \
    }while( 0 )
#endif
#define HIPPO_WARN HIPPO_DEBUG


#endif

