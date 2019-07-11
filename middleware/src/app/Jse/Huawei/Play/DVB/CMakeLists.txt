IF (NOT MODULE_DVBS_Tuner)
    return()
ENDIF (NOT MODULE_DVBS_Tuner)

LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/JseHWDVB.cpp
    )

include (${CMAKE_CURRENT_LIST_DIR}/Book/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Channel/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Motor/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Program/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Satellite/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Search/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/TP/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/Tuner/CMakeLists.txt)
