#### pppd ####

set (PPPD_TARGET_NAME pppd)

#aux_source_directory (${CMAKE_CURRENT_LIST_DIR} PPPD_SRC_LIST)

add_definitions (-DINET6) 

include_directories (
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/include/missing
)

list (APPEND PPPD_SRC_LIST
    ${CMAKE_CURRENT_LIST_DIR}/main.c
    ${CMAKE_CURRENT_LIST_DIR}/magic.c
    ${CMAKE_CURRENT_LIST_DIR}/fsm.c
    ${CMAKE_CURRENT_LIST_DIR}/lcp.c
    ${CMAKE_CURRENT_LIST_DIR}/ipcp.c
    ${CMAKE_CURRENT_LIST_DIR}/ipv6cp.c
    ${CMAKE_CURRENT_LIST_DIR}/upap.c
    ${CMAKE_CURRENT_LIST_DIR}/chap-new.c
    ${CMAKE_CURRENT_LIST_DIR}/eap.c
    ${CMAKE_CURRENT_LIST_DIR}/ccp.c
    ${CMAKE_CURRENT_LIST_DIR}/ecp.c
    ${CMAKE_CURRENT_LIST_DIR}/auth.c
    ${CMAKE_CURRENT_LIST_DIR}/options.c
    ${CMAKE_CURRENT_LIST_DIR}/sys-linux.c
    ${CMAKE_CURRENT_LIST_DIR}/chap_ms.c
    ${CMAKE_CURRENT_LIST_DIR}/demand.c
    ${CMAKE_CURRENT_LIST_DIR}/utils.c
    ${CMAKE_CURRENT_LIST_DIR}/tty.c
    ${CMAKE_CURRENT_LIST_DIR}/chap-md5.c
    ${CMAKE_CURRENT_LIST_DIR}/session.c
    ${CMAKE_CURRENT_LIST_DIR}/eui64.c
    )

link_directories(
    ${PLATFORM_SDK_PATH}/rootfs/lib
    ${PLATFORM_SDK_PATH}/rootfs/usr/lib 
    ${PLATFORM_SDK_PATH}/rootfs/home/${PLATFORM_HOME}/lib
    ${iptv_BINARY_DIR}/third_party/lib
    ${TAKIN_SDK_PATH}/lib
)

add_executable (${PPPD_TARGET_NAME} ${PPPD_SRC_LIST})

#add_dependencies (${PPPD_TARGET_NAME} ${TARGET_NAME})

if(${PLATFORM} STREQUAL "hi3560e")
target_link_libraries (${PPPD_TARGET_NAME} crypto)
endif(${PLATFORM} STREQUAL "hi3560e")

target_link_libraries (${PPPD_TARGET_NAME} pthread rt m gcc ssl crypt)

add_custom_command( TARGET ${PPPD_TARGET_NAME} POST_BUILD 
    COMMAND rm ${PLATFORM_SDK_PATH}/rootfs/home/${PLATFORM_HOME}/bin/ppp -rf
    COMMAND mkdir ${PLATFORM_SDK_PATH}/rootfs/home/${PLATFORM_HOME}/bin/ppp
    COMMAND cp -arpf ${EXECUTABLE_OUTPUT_PATH}/${PPPD_TARGET_NAME} ${PLATFORM_SDK_PATH}/rootfs/home/${PLATFORM_HOME}/bin
    COMMAND cp -arpf ${CMAKE_CURRENT_LIST_DIR}/shell/ppp.connect.sh ${PLATFORM_SDK_PATH}/rootfs/home/${PLATFORM_HOME}/bin
    COMMAND cp -arpf ${CMAKE_CURRENT_LIST_DIR}/shell/ppp.disconnect.sh ${PLATFORM_SDK_PATH}/rootfs/home/${PLATFORM_HOME}/bin
    COMMAND cp -arpf ${CMAKE_CURRENT_LIST_DIR}/shell/auth-err ${PLATFORM_SDK_PATH}/rootfs/home/${PLATFORM_HOME}/bin/ppp
    COMMAND cp -arpf ${CMAKE_CURRENT_LIST_DIR}/shell/ip-up ${PLATFORM_SDK_PATH}/rootfs/home/${PLATFORM_HOME}/bin/ppp
    )



