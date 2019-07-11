FILE( GLOB_RECURSE platform_mid_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}		
	${CMAKE_CURRENT_SOURCE_DIR}/*.c
	)
	
include_directories(		
		${PROJECT_SOURCE_DIR}/EC2108_C27/include
		${PROJECT_SOURCE_DIR}/EC2108_C27/include/middle
		${PROJECT_SOURCE_DIR}/EC2108_C27/include/middle_public
		${PROJECT_SOURCE_DIR}/EC2108_C27/include/porting		
		${PROJECT_SOURCE_DIR}/EC2108_C27/include/tr069
		${PROJECT_SOURCE_DIR}/EC2108_C27/include/independs
		${PROJECT_SOURCE_DIR}/EC2108_C27/ipstb/src/include
		${PROJECT_SOURCE_DIR}/EC2108_C27/ipstb/src/include/app
		${PROJECT_SOURCE_DIR}/EC2108_C27/ipstb/src/include/broswer		
		${PROJECT_SOURCE_DIR}/EC2108_C27/ipstb/src/log
		${PROJECT_SOURCE_DIR}/EC2108_C27/7405/include
		${PROJECT_SOURCE_DIR}/EC2108_C27/7405/include/porting_include
		${PLATFORM_SDK_PATH}/include
	  ${PROJECT_SOURCE_DIR}/EC2108_C27/$ENV{PLATFORM}/include
	)
add_library( platform_mid STATIC ${platform_mid_SRCS} )
