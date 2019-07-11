
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/JseHWSTB.cpp
    )

include (${CMAKE_CURRENT_LIST_DIR}/Time/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Version/CMakeLists.txt)

IF (BUILD_Android)
include (${CMAKE_CURRENT_LIST_DIR}/AppManager/CMakeLists.txt)
ENDIF (BUILD_Android)