
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/JseHWMaintenance.cpp
    )


include (${CMAKE_CURRENT_LIST_DIR}/Log/CMakeLists.txt) 
include (${CMAKE_CURRENT_LIST_DIR}/NetDiagnoseTool/CMakeLists.txt) 
include (${CMAKE_CURRENT_LIST_DIR}/Upgrade/CMakeLists.txt) 

IF (MODULE_Management STREQUAL "hybroad") 
include (${CMAKE_CURRENT_LIST_DIR}/Tr069/CMakeLists.txt) 
ENDIF (MODULE_Management STREQUAL "hybroad")