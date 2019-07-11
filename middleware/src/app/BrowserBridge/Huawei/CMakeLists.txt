
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/BrowserPlayerHuawei.cpp
    ${CMAKE_CURRENT_LIST_DIR}/BrowserEventQueue.cpp
    ${CMAKE_CURRENT_LIST_DIR}/KeyTableParser.cpp
    )

include (${CMAKE_CURRENT_LIST_DIR}/${BUILD_ContextType}/CMakeLists.txt)

LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}
    )


