
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/JseHWIO.cpp
    )
    
IF (MODULE_BLUETOOTH)
include (${CMAKE_CURRENT_LIST_DIR}/Bluetooth/CMakeLists.txt) 
ENDIF (MODULE_BLUETOOTH)

include (${CMAKE_CURRENT_LIST_DIR}/Analog/CMakeLists.txt) 
include (${CMAKE_CURRENT_LIST_DIR}/Digital/CMakeLists.txt) 