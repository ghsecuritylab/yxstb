
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/JseModules.cpp
    )
    
include (${CMAKE_CURRENT_LIST_DIR}/CA/CMakeLists.txt)
include (${CMAKE_CURRENT_LIST_DIR}/SafetyLine/CMakeLists.txt)
IF (MODULE_u_config)
include (${CMAKE_CURRENT_LIST_DIR}/UDisk/CMakeLists.txt)
ENDIF (MODULE_u_config)
include (${CMAKE_CURRENT_LIST_DIR}/IMS/CMakeLists.txt)