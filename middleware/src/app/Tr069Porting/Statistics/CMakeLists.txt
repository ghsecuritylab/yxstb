
IF (MODULE_Management STREQUAL "hybroad")
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/tr069_port_alarm.c
    ${CMAKE_CURRENT_LIST_DIR}/app_tr069_alarm.c
    )
    
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/StatisticRoot.cpp
    )

include (${CMAKE_CURRENT_LIST_DIR}/StatisticLog/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/StatisticHW/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/StatisticCTC/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/StatisticCU/CMakeLists.txt)

LIST (APPEND INCLUDES 
    ${CMAKE_CURRENT_LIST_DIR}
    )
ENDIF (MODULE_Management STREQUAL "hybroad")