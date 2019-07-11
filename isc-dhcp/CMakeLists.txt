#### dhcpcd ####

IF (BUILD_Android)
    return()
ENDIF (BUILD_Android)

set (DHCLIENT_TARGET_NAME dhclient)

add_definitions (-DCLIENT_PATH="/home/hybroad/bin")
add_definitions (-DLOCALSTATEDIR="/var")
add_definitions (-D_PATH_RESCONF="/var")
add_definitions (-DHAVE_CONFIG_H)
add_definitions (-DMINIRES_LIB)
add_definitions (-DHMAC_MD5)
add_definitions (-DHYBROAD_USED)

include_directories (
    ${CMAKE_CURRENT_LIST_DIR}/
    ${CMAKE_CURRENT_LIST_DIR}/includes
    ${CMAKE_CURRENT_LIST_DIR}/../middleware/include/
    ${iptv_BINARY_DIR}/third_party/include
    )

include (${CMAKE_CURRENT_LIST_DIR}/common/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/omapip/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/minires/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/dst/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/client/CMakeLists.txt)

link_directories( 
    ${PLATFORM_SDK_PATH}/rootfs/lib
    ${PLATFORM_SDK_PATH}/rootfs/usr/lib 
    ${PLATFORM_SDK_PATH}/rootfs/home/${PLATFORM_HOME}/lib
    ${iptv_BINARY_DIR}/third_party/lib
    ${TAKIN_SDK_PATH}/lib
)

add_executable (${DHCLIENT_TARGET_NAME} ${DHCLIENT_SRC_LIST})

target_link_libraries (${DHCLIENT_TARGET_NAME} pthread rt m gcc ssl)

#TODO 
add_custom_command( TARGET ${DHCLIENT_TARGET_NAME} POST_BUILD 
    #COMMAND rm ${PLATFORM_SDK_PATH}/rootfs/home/${PLATFORM_HOME}/bin/dhcpcd-hooks -rf
    #COMMAND mkdir ${PLATFORM_SDK_PATH}/rootfs/home/${PLATFORM_HOME}/bin/dhcpcd-hooks
    COMMAND cp -arpf ${EXECUTABLE_OUTPUT_PATH}/${DHCLIENT_TARGET_NAME} ${PLATFORM_SDK_PATH}/rootfs/home/${PLATFORM_HOME}/bin
    COMMAND cp -arpf ${CMAKE_CURRENT_LIST_DIR}/client/dhclient.conf ${PLATFORM_SDK_PATH}/rootfs/home/${PLATFORM_HOME}/share
    COMMAND cp -arpf ${CMAKE_CURRENT_LIST_DIR}/client/scripts/dhclient.script ${PLATFORM_SDK_PATH}/rootfs/home/${PLATFORM_HOME}/bin/
    COMMAND cp -arpf ${CMAKE_CURRENT_LIST_DIR}/client/scripts/dhclient.connect ${PLATFORM_SDK_PATH}/rootfs/home/${PLATFORM_HOME}/bin/
    COMMAND cp -arpf ${CMAKE_CURRENT_LIST_DIR}/client/scripts/dhclient.disconnect ${PLATFORM_SDK_PATH}/rootfs/home/${PLATFORM_HOME}/bin/
    #WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}
    #VERBATIM 
    )
