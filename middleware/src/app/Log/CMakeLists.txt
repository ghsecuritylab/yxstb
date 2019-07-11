
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/Log.cpp
    ${CMAKE_CURRENT_LIST_DIR}/LogPool.cpp
    ${CMAKE_CURRENT_LIST_DIR}/LogFilter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/LogPrinter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/LogPostFTP.cpp
    ${CMAKE_CURRENT_LIST_DIR}/LogPostHTTP.cpp
    ${CMAKE_CURRENT_LIST_DIR}/LogPostUDP.cpp
    ${CMAKE_CURRENT_LIST_DIR}/LogModule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/LogModuleCore.cpp
    ${CMAKE_CURRENT_LIST_DIR}/LogModuleForMonitor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/LogModuleForTR069.cpp
    ${CMAKE_CURRENT_LIST_DIR}/LogThread.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Huawei/LogModuleHuawei.cpp
    )

LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/Huawei
    )
