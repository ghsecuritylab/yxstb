
IF (NOT BUILD_BrowserIndependence)
    return ()
ENDIF (NOT BUILD_BrowserIndependence)


############################################################################

LIST (APPEND IPC_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/ipc/AgentShell.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ipc/IPCAssertions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ipc/IPCCaller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ipc/IPC.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ipc/IPCServer.cpp
    )

SET (TARGET_IPC_NAME_A jseglue_ipcA)
add_library (${TARGET_IPC_NAME_A} ${IPC_SOURCES})


############################################################################

set (BROWSER_TARGET_NAME browser.elf )

include_directories (
    ${PROJECT_SOURCE_DIR}/hybroad/middleware/include
    ${iptv_BINARY_DIR}/third_party/include/cairo
    ${TAKIN_SDK_PATH}/include/porting/core
    ${TAKIN_SDK_PATH}/include/porting/middleware
    ${TAKIN_SDK_PATH}/include/base
    ${PLATFORM_SDK_PATH}/include/sdk
    ${PLATFORM_SDK_PATH}/include
    ${CMAKE_CURRENT_LIST_DIR}/
    ${CMAKE_CURRENT_LIST_DIR}/ipc
    ${CMAKE_CURRENT_LIST_DIR}/browser/JseCaller
    ${CMAKE_CURRENT_LIST_DIR}/../middleware/src/JseGlue/
    )

LIST (APPEND BROWSER_SRC_LIST 
    ${CMAKE_CURRENT_LIST_DIR}/browser/AgentHelper.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/browser/GlobalFunctions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/browser/JVMThreadFuncs.cpp
    ${CMAKE_CURRENT_LIST_DIR}/browser/BrowserRun.cpp
    ${CMAKE_CURRENT_LIST_DIR}/browser/JseCaller/IAuthenticationCaller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/browser/JseCaller/IDeviceCaller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/browser/JseCaller/IiPanelCaller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/browser/JseCaller/IiPanelPageWidgetCaller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/browser/JseCaller/IMediaPlayerCaller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/browser/JseCaller/IUBankDeviceCaller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/browser/JseCaller/IUtilityCaller.cpp
    )

link_directories( 
    ${PLATFORM_SDK_PATH}/rootfs/lib
    ${PLATFORM_SDK_PATH}/rootfs/usr/lib 
    ${PLATFORM_SDK_PATH}/rootfs/home/${PLATFORM_HOME}/lib
    ${iptv_BINARY_DIR}/third_party/lib
    ${TAKIN_SDK_PATH}/lib
    )

add_executable (${BROWSER_TARGET_NAME} ${BROWSER_SRC_LIST})

target_link_libraries( ${BROWSER_TARGET_NAME}  
    #jpeg #v5 no need
    #jpge #v5 no need
    #png  #v5 no need
    #gif  #v5 no need
    cairo
    curl 
    webkit-takin
    xml2
    player
    gcc_s 
    z 
    jseglue_ipcA
    )
IF (BUILD_Support_LittleSystem)
    target_link_libraries (${BROWSER_TARGET_NAME}
        polar_scbt
        )
ENDIF (BUILD_Support_LittleSystem)

foreach (LIBS ${PLATFORM_LINKFLAGS})
    target_link_libraries (${BROWSER_TARGET_NAME} ${LIBS})
endforeach (LIBS)



