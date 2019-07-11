
IF (NOT BUILD_Support_LittleSystem)
    return ()
ENDIF (NOT BUILD_Support_LittleSystem)

#############################

LIST (APPEND START_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/SystemStart.c
    )

set (START_TARGET_NAME start.elf)

add_executable (${START_TARGET_NAME} ${START_SOURCES})


#############################

LIST (APPEND LS_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/LittleSystem.c
    ${CMAKE_CURRENT_LIST_DIR}/sharedFunction.c
    )

include_directories (
    ${PROJECT_TOP_DIR}/hybroad/middleware/include
    ${PROJECT_TOP_DIR}/hybroad/middleware/include/cairo
    ${PROJECT_TOP_DIR}/hybroad/middleware/include/middle
    ${PROJECT_TOP_DIR}/hybroad/middleware/include/independs
    ${PROJECT_TOP_DIR}/hybroad/middleware/src/include
    ${PROJECT_TOP_DIR}/hybroad/middleware/src/include/app
    ${PROJECT_TOP_DIR}/hybroad/middleware/src/include/browser
    ${PROJECT_TOP_DIR}/hybroad/middleware/src/
    ${PROJECT_TOP_DIR}/hybroad/middleware/src/tr069
    ${PROJECT_TOP_DIR}/hybroad/middleware/src/tools
    ${PROJECT_TOP_DIR}/hybroad/middleware/src/takin/include
    ${PROJECT_TOP_DIR}/hybroad/middleware/src/app
    ${PROJECT_TOP_DIR}/hybroad/middleware/src/app/Message
    ${PROJECT_TOP_DIR}/hybroad/middleware/src/app/Upgrade/
    ${PROJECT_TOP_DIR}/hybroad/middleware/src/app/Upgrade/BootImages
    ${PROJECT_TOP_DIR}/hybroad/middleware/src/app/Devices
    ${PROJECT_TOP_DIR}/hybroad/middleware/src/app/Views
    ${PROJECT_TOP_DIR}/hybroad/middleware/src/app/Views/Widgets
    ${PROJECT_TOP_DIR}/hybroad/middleware/src/app/BrowserBridge
    ${PROJECT_TOP_DIR}/hybroad/middleware/src/app/NativeHandler
    ${PROJECT_TOP_DIR}/hybroad/middleware/src/app/Util
    ${PROJECT_TOP_DIR}/hybroad/middleware/src/porting/tr069
    ${PROJECT_TOP_DIR}/hybroad/middleware/platform/include
    ${PROJECT_TOP_DIR}/hybroad/middleware/hippo/CPP2C/
    ${PLATFORM_SDK_PATH}/include
    ${PLATFORM_SDK_PATH}/include/sdk
    ${TAKIN_SDK_PATH}/include
    ${TAKIN_SDK_PATH}/include/base
    ${TAKIN_SDK_PATH}/include/porting/core
    ${TAKIN_SDK_PATH}/include/porting/middleware
    ${iptv_BINARY_DIR}/third_party/include
    ${iptv_BINARY_DIR}/third_party/include/pixman
    ${iptv_BINARY_DIR}
    )


foreach (LDS ${LIBS_DIRECTORIES})
    link_directories (${LDS})
endforeach (LDS)
link_directories (${LIBRARY_OUTPUT_PATH})

foreach (DEF ${DEFINITIONS})
    add_definitions (${DEF})
endforeach (DEF)

set (LS_TARGET_NAME littleSystem.elf)
add_executable (${LS_TARGET_NAME} ${LS_SOURCES})

foreach (LIBS ${PLATFORM_LINKFLAGS})
    target_link_libraries (${LS_TARGET_NAME} ${LIBS})
endforeach (LIBS)

target_link_libraries (${LS_TARGET_NAME} 
    polar_scbt 
    cairo
    freetype
    -Wl,-whole-archive
    takinAgentN 
    middleSoftwareS
    _dhcp
    _ppp
    -lmiddleSoftwareS
    -Wl,-no-whole-archive
    )
IF (${MODULE_IPV6})
    target_link_libraries (${LS_TARGET_NAME} _dhcpv6)
ENDIF (${MODULE_IPV6})
IF (${BUILD_BrowserIndependence})
    target_link_libraries (${LS_TARGET_NAME} jseglue_ipcA)
ENDIF (${BUILD_BrowserIndependence})

IF (${BUILD_ContextType} STREQUAL "C20")
    target_link_libraries (${LS_TARGET_NAME}
        sqlite3
        xml2
        )
ENDIF (${BUILD_ContextType} STREQUAL "C20")
    

