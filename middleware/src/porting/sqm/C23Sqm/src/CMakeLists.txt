#### Eagle 2011.01.17 ####

set( SQM_TARGET_NAME sqm.elf )

list( APPEND SQM_SRC_LIST sqm_main.c )

add_executable( ${SQM_TARGET_NAME} ${SQM_SRC_LIST} )

###removed by teddy, using Cmake default, 2011-1-19 13:46:51
#set( CMAKE_C_FLAGS "-g -Wall -O2 -Werror" )
if( ENABLE_Debug_Build )
	set( LIB_TYPE debug )
else( ENABLE_Debug_Build )
	set( LIB_TYPE release )
endif( ENABLE_Debug_Build )

	
if( SELECT_SQM_VER STREQUAL "C21" )
	set( SQM_VER C21 ) 
endif( SELECT_SQM_VER STREQUAL "C21"  )

if( SELECT_SQM_VER STREQUAL "C22" )
	set( SQM_VER C22 )
endif( SELECT_SQM_VER STREQUAL "C22"  )

if( SELECT_SQM_VER STREQUAL "C23" )
	set( SQM_VER C23 )
endif( SELECT_SQM_VER STREQUAL "C23"  )

	
if(${PLATFORM} STREQUAL "brcm7405")
	set( SQM_SHARED_LIBS_PATH ${PROJECT_SOURCE_DIR}/Build/${PLATFORM}/sdk/${LIB_TYPE}/shared_libs/SQM_lib/${SQM_VER} )
else(${PLATFORM} STREQUAL "brcm7405")
	set( SQM_SHARED_LIBS_PATH ${PROJECT_SOURCE_DIR}/Build/${PLATFORM}/sdk/shared_libs/SQM_lib/${SELECT_SQM_VER} )
endif(${PLATFORM} STREQUAL "brcm7405")


set( SQM_LIBS 
        ${SQM_SHARED_LIBS_PATH}/libsqmcbb.so 
        ${SQM_SHARED_LIBS_PATH}/libpcap.so 
        ${SQM_SHARED_LIBS_PATH}/libmdishared.so 
        )

add_definitions( -DLINUX_OS ) 

target_link_libraries( ${SQM_TARGET_NAME} pthread rt m gcc sqmcbb pcap mdishared )

add_custom_command( TARGET ${SQM_TARGET_NAME} PRE_BUILD 
            COMMAND sudo cp -arpf ${SQM_SHARED_LIBS_PATH}/libsqmcbb.so ${PLATFORM_SDK_PATH}/rootfs/home/${PLATFORM_HOME}/lib/ 
            COMMAND sudo cp -arpf ${SQM_SHARED_LIBS_PATH}/libpcap.so ${PLATFORM_SDK_PATH}/rootfs/home/${PLATFORM_HOME}/lib/ 
            COMMAND sudo cp -arpf ${SQM_SHARED_LIBS_PATH}/libmdishared.so ${PLATFORM_SDK_PATH}/rootfs/home/${PLATFORM_HOME}/lib/
            COMMENT "cp libsqmcbb.so libpcap.so libmdishared.so to ${PLATFORM_SDK_PATH}/rootfs/home/${PLATFORM_HOME}/lib/"
            VERBATIM
            )

add_custom_command( TARGET ${SQM_TARGET_NAME} POST_BUILD 
            COMMAND sudo cp -arpf ${SQM_TARGET_NAME} ${PLATFORM_SDK_PATH}/rootfs/home/${PLATFORM_HOME}/bin/ 
            COMMENT "cp -arpf ${SQM_TARGET_NAME} ${PLATFORM_SDK_PATH}/rootfs/home/${PLATFORM_HOME}/bin/"
            WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}
            VERBATIM 
            )
