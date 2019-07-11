
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/BrowserPlayerManagerC10.cpp
    ${CMAKE_CURRENT_LIST_DIR}/BrowserPlayerC10.cpp
    ${CMAKE_CURRENT_LIST_DIR}/CTC/CTCServiceEntry.cpp
    )

LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}
    )

include (${CMAKE_CURRENT_LIST_DIR}/CTC/CMakeLists.txt)

