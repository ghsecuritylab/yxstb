IF (MODULE_Management STREQUAL "hybroad")


    LIST (APPEND TR069_C_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/v2_src/tr069_api.c
        ${CMAKE_CURRENT_LIST_DIR}/v2_src/tr069_config.c
        ${CMAKE_CURRENT_LIST_DIR}/v2_src/tr069_diag.c
        ${CMAKE_CURRENT_LIST_DIR}/v2_src/tr069_ftp.c
        ${CMAKE_CURRENT_LIST_DIR}/v2_src/tr069_global.c
        ${CMAKE_CURRENT_LIST_DIR}/v2_src/tr069_hmac_sha1.c
        ${CMAKE_CURRENT_LIST_DIR}/v2_src/tr069_http.c
        ${CMAKE_CURRENT_LIST_DIR}/v2_src/tr069_main.c
        ${CMAKE_CURRENT_LIST_DIR}/v2_src/tr069_md5.c
        ${CMAKE_CURRENT_LIST_DIR}/v2_src/tr069_method.c
        ${CMAKE_CURRENT_LIST_DIR}/v2_src/tr069_param.c
        ${CMAKE_CURRENT_LIST_DIR}/v2_src/tr069_soap.c
        ${CMAKE_CURRENT_LIST_DIR}/v2_src/tr069_stdex.c
        ${CMAKE_CURRENT_LIST_DIR}/v2_src/tr069_stun.c
        ${CMAKE_CURRENT_LIST_DIR}/v2_src/tr069_timer.c
        ${CMAKE_CURRENT_LIST_DIR}/v2_src/tr069_tr106.c
        ${CMAKE_CURRENT_LIST_DIR}/v2_param/app_aes.c
        ${CMAKE_CURRENT_LIST_DIR}/v2_param/tr069_port0.c
        ${CMAKE_CURRENT_LIST_DIR}/v2_param/tmerTask.c
        ${CMAKE_CURRENT_LIST_DIR}/v2_param/extendConfig.c
        ${CMAKE_CURRENT_LIST_DIR}/v2_param/tr069_port_alarm.c
        ${CMAKE_CURRENT_LIST_DIR}/v2_param/tr069_port_alarmCPU.c
        ${CMAKE_CURRENT_LIST_DIR}/v2_param/tr069_port_alarmDisk.c
        ${CMAKE_CURRENT_LIST_DIR}/v2_param/tr069_port_alarmHuawei.c
        ${CMAKE_CURRENT_LIST_DIR}/v2_param/tr069_port_alarmMemory.c
        ${CMAKE_CURRENT_LIST_DIR}/v2_param/tr069_port_alarmSichuan.c
        ${CMAKE_CURRENT_LIST_DIR}/v2_param/tr069_port_module.c
        ${CMAKE_CURRENT_LIST_DIR}/v2_param/tr069_port_errorcode.c
        ${CMAKE_CURRENT_LIST_DIR}/v2_param/tr069_port_sqm.c
        )
    include_directories( 
        ${CMAKE_CURRENT_LIST_DIR}/v2_src
        ${iptv_BINARY_DIR}/third_party/include
        )
    IF (CUSTOMER STREQUAL "Android")
        add_definitions(-DANDROID_LOGCAT_OUTPUT)
    ELSEIF (1)
        include_directories(
            ${CMAKE_CURRENT_LIST_DIR}/../middleware/src/porting/tr069
            ${CMAKE_CURRENT_LIST_DIR}/../middleware/src/app
            )
    ENDIF (CUSTOMER STREQUAL "Android")

    LIST (APPEND INCLUDES
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/v2_src
        )

    foreach (INC ${INCLUDES})
        # MESSAGE (STATUS "INCLUDE: ${INC}")
        include_directories (${INC})
    endforeach (INC)

    foreach (DEF ${DEFINITIONS})
        # MESSAGE (STATUS "MACRO: ${DEF}")
        add_definitions (${DEF})
    endforeach (DEF)

    add_library (tr069 ${TR069_C_SOURCES})    

    add_dependencies(tr069 platform_sdk)
ENDIF (MODULE_Management STREQUAL "hybroad")



