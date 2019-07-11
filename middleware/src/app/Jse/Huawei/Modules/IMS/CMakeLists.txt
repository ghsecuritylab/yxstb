
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/JseHWIMS.cpp

    )

include (${CMAKE_CURRENT_LIST_DIR}/Call/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/AddrBook/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Camera/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Friends/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/FriendTV/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Msg/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/TempData/CMakeLists.txt)