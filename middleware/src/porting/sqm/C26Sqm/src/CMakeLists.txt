#### Eagle 2011.01.17 ####

set( SQM_TARGET_NAME sqm.elf )

list( APPEND SQM_SRC_LIST sqm_main.c )

foreach (LDS ${LIBS_DIRECTORIES})
    link_directories (${LDS})
endforeach (LDS)

add_executable( ${SQM_TARGET_NAME} ${SQM_SRC_LIST} )

###removed by teddy, using Cmake default, 2011-1-19 13:46:51
#set( CMAKE_C_FLAGS "-g -Wall -O2 -Werror" )
if( ENABLE_Debug_Build )
	set( LIB_TYPE debug )
else( ENABLE_Debug_Build )
	set( LIB_TYPE release )
endif( ENABLE_Debug_Build )

if( SELECT_SQM_VER STREQUAL "C26" )
	set( SQM_VER C26 )
endif( SELECT_SQM_VER STREQUAL "C26"  )

set( SQMPRO_PATH ${PROJECT_SOURCE_DIR}/huawei/${PLATFORM}/sqmbin/${MODULE_SQM_VER} )

if($ENV{PLATFORM} STREQUAL "brcm7405")
	set( SQM_SHARED_LIBS_PATH ${PROJECT_SOURCE_DIR}/Build/$ENV{PLATFORM}/sdk/${LIB_TYPE}/shared_libs/SQM_lib/${SQM_VER} )
else($ENV{PLATFORM} STREQUAL "brcm7405")
	set (SQM_SHARED_LIBS_PATH   ${PROJECT_TOP_DIR}/platform/${PLATFORM}/${LIB_TYPE}/SQM_lib/${MODULE_SQM_VER})
endif($ENV{PLATFORM} STREQUAL "brcm7405")

set( SQM_LIBS 
        ${SQM_SHARED_LIBS_PATH}/libpcap.so 
        )

 include_directories (
    ${CMAKE_CURRENT_LIST_DIR}/..
    ${PROJECT_TOP_DIR}/EC2108_C27/ipstb/src/include
    ${iptv_BINARY_DIR}
    )

target_link_libraries( ${SQM_TARGET_NAME} pthread rt m gcc pcap )

#add_custom_command( TARGET ${SQM_TARGET_NAME} PRE_BUILD
 #           COMMAND sudo cp -arpf ${SQM_SHARED_LIBS_PATH}/libpcap.so ${LIB_FILE_COPY_PATH}/
 #           COMMAND sudo cp -arpf ${SQMPRO_PATH}/sqmloader  ${BIN_FILE_COPY_PATH}/
  #          COMMAND sudo cp -arpf ${SQMPRO_PATH}/sqmpro     ${BIN_FILE_COPY_PATH}/
   #         COMMENT "cp  libpcap.so sqmloader sqmpro to ${LIB_FILE_COPY_PATH}/"
    #        VERBATIM
     #       )

#add_custom_command( TARGET ${SQM_TARGET_NAME} POST_BUILD 
  #          COMMAND sudo cp -arpf ${SQM_TARGET_NAME} ${BIN_FILE_COPY_PATH}/
    #        COMMENT "cp -arpf ${SQM_TARGET_NAME} ${BIN_FILE_COPY_PATH}/"
      #      WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}
        #    VERBATIM 
           # )
