
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/JseMaintenance.cpp
    )
	
include (${CMAKE_CURRENT_LIST_DIR}/Log/CMakeLists.txt) 
include (${CMAKE_CURRENT_LIST_DIR}/NetDiagnoseTool/CMakeLists.txt) 
include (${CMAKE_CURRENT_LIST_DIR}/Upgrade/CMakeLists.txt) 
IF (MODULE_Management STREQUAL "hybroad") 
include (${CMAKE_CURRENT_LIST_DIR}/Tr069/CMakeLists.txt) 
ENDIF (MODULE_Management STREQUAL "hybroad")

IF (MODULE_Management STREQUAL "hmwMgmt") 
include (${CMAKE_CURRENT_LIST_DIR}/Tr069/CMakeLists.txt) 
ENDIF (MODULE_Management STREQUAL "hmwMgmt")