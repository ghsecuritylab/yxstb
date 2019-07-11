
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/BrowserPlayerReporter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/BrowserPlayerReporterHuawei.cpp
    ${CMAKE_CURRENT_LIST_DIR}/BrowserPlayerMsgCode.cpp
    )
    
LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}
    )

include (${CMAKE_CURRENT_LIST_DIR}/${BUILD_ContextType}/CMakeLists.txt)


