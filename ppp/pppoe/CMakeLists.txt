#### pppoe ####

set (PPPOE_TARGET_NAME pppoe)

#aux_source_directory (${CMAKE_CURRENT_LIST_DIR} PPPOE_SRC_LIST)

list (APPEND PPPOE_SRC_LIST
    ${CMAKE_CURRENT_LIST_DIR}/pppoe.c
    ${CMAKE_CURRENT_LIST_DIR}/ppp.c
    ${CMAKE_CURRENT_LIST_DIR}/common.c
    ${CMAKE_CURRENT_LIST_DIR}/discovery.c
    ${CMAKE_CURRENT_LIST_DIR}/debug.c
    ${CMAKE_CURRENT_LIST_DIR}/if.c
    )

link_directories(
    ${PLATFORM_SDK_PATH}/rootfs/lib
    ${PLATFORM_SDK_PATH}/rootfs/usr/lib 
    ${PLATFORM_SDK_PATH}/rootfs/home/${PLATFORM_HOME}/lib
    ${iptv_BINARY_DIR}/third_party
    ${TAKIN_SDK_PATH}/lib
)

add_executable (${PPPOE_TARGET_NAME} ${PPPOE_SRC_LIST})

#add_dependencies (${PPPOE_TARGET_NAME} ${TARGET_NAME})

target_link_libraries (${PPPOE_TARGET_NAME} pthread rt m gcc)

add_custom_command( TARGET ${PPPOE_TARGET_NAME} POST_BUILD 
    COMMAND cp -arpf ${EXECUTABLE_OUTPUT_PATH}/${PPPOE_TARGET_NAME} ${PLATFORM_SDK_PATH}/rootfs/home/${PLATFORM_HOME}/bin
    )



