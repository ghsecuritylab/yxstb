
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/UltraPlayerBGMusic.cpp
    ${CMAKE_CURRENT_LIST_DIR}/UltraPlayerVodList.cpp
    ${CMAKE_CURRENT_LIST_DIR}/UltraPlayerMultipleHuawei.cpp
    ${CMAKE_CURRENT_LIST_DIR}/UltraPlayerVodHuawei.cpp
    ${CMAKE_CURRENT_LIST_DIR}/UltraPlayerWebPage.cpp
    )

include (${CMAKE_CURRENT_LIST_DIR}/${BUILD_ContextType}/CMakeLists.txt)

LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}
    )


