
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/BrowserPlayer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/BrowserPlayerManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/BrowserAssertions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/BrowserLayerZebra.cpp
    ${CMAKE_CURRENT_LIST_DIR}/BCMediaPlayerPrivatePorting.cpp	
    )

LIST (APPEND BROWSERAGENT_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/BrowserAgent.cpp
    ${CMAKE_CURRENT_LIST_DIR}/BrowserView.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Takin/BrowserViewTakin.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Takin/BrowserAgentTakin.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Takin/BrowserAgentTakinHelper.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Takin/BrowserJVMPlugin.cpp
    )

LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/Takin
    )

include (${CMAKE_CURRENT_LIST_DIR}/Huawei/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Reporter/CMakeLists.txt)



