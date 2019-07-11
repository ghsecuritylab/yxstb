
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/JseGlueAssertions.cpp
    )

LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}
    )

include (${CMAKE_CURRENT_LIST_DIR}/SingleAgent/CMakeLists.txt)

IF (BUILD_BrowserIndependence)
    include (${CMAKE_CURRENT_LIST_DIR}/MultipAgent/CMakeLists.txt)
ENDIF (BUILD_BrowserIndependence)
