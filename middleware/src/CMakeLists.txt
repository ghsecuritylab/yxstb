


LIST (APPEND IPTV_INIT_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/sys_init.c
    ${CMAKE_CURRENT_LIST_DIR}/takin/audio_takin.c
    ${CMAKE_CURRENT_LIST_DIR}/takin/jvm_main.c
    ${CMAKE_CURRENT_LIST_DIR}/takin/takin.cpp
    )


LIST (APPEND IPTV_C_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/sys_key_deal.c
    ${CMAKE_CURRENT_LIST_DIR}/sys_msg.c
    # ${CMAKE_CURRENT_LIST_DIR}/iptv_logging.cc
    )

#LIST (APPEND IPTV_CXX_SOURCES
#    )

IF (DEBUG__Use_instrument_functions)
    LIST (APPEND IPTV_C_SOURCES
        sys_instrument.c
        )
ENDIF (DEBUG__Use_instrument_functions)


LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/include/middle
    ${CMAKE_CURRENT_LIST_DIR}/include/middle_public
    ${CMAKE_CURRENT_LIST_DIR}/include/porting
    ${CMAKE_CURRENT_LIST_DIR}/include/app
    ${CMAKE_CURRENT_LIST_DIR}/include/config
    ${CMAKE_CURRENT_LIST_DIR}/include/browser
    ${CMAKE_CURRENT_LIST_DIR}/include/tstv
    ${CMAKE_CURRENT_LIST_DIR}/include/wifi
    ${CMAKE_CURRENT_LIST_DIR}/takin/include
    ${TAKIN_SDK_PATH}/include/porting/core
    ${TAKIN_SDK_PATH}/include/porting/middleware
    ${TAKIN_SDK_PATH}/include/porting/base
    )


include (${CMAKE_CURRENT_LIST_DIR}/init/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/JseGlue/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/tools/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/app/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/module/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/porting/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/bluetooth/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/cpvr/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/disk/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/xmpp/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/sdk/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/stbinfo/CMakeLists.txt)


