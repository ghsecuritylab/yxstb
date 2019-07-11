#ifndef __TDY_DEBUG_H__
#define __TDY_DEBUG_H__
#include <stdio.h>
#include <yx_type.h>
#include <yx_return_code.h>

//to yx all debug info control
YXEXTERN int yxprint_flag;

#define yx_middle_dbgprintf(A,...) 	\
	{			\
		if(yxprint_flag)	\
		{		\
		  usleep(5);	\
		  printf("YX_MIDDLE_PRINTF[%s]"A, __FUNCTION__, ##__VA_ARGS__);\
		  usleep(5);	\
		}		\
	}


#define TDY_DEBUG(X...) \
do{	\
	printf("[%d:%s] ", __LINE__, __FUNCTION__);	\
	printf( X );	\
}while( 0 )

#define TDY_PERROR(X...) \
do{	\
	printf("[%d:%s] ", __LINE__, __FUNCTION__);	\
	printf( X );	\
	perror( "" );	\
}while( 0 )

#endif /*__TDY_DEBUG_H__*/
