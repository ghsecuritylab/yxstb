
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/Tr069SQMConfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Tr069ServiceInfo.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Tr069PlayDiagnostics.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Tr069ServiceStatistics.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Tr069StatisticConfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Tr069StatisticConfiguration1.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Tr069StatisticConfiguration2.cpp
    )

LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/
    )
    
include (${CMAKE_CURRENT_LIST_DIR}/Tr069LAN/CMakeLists.txt)
