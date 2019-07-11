
IF (MODULE_u_config)

LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/UDiskAssertions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/UDiskConfig.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/UDiskDetect.cpp
    ${CMAKE_CURRENT_LIST_DIR}/UDiskQuickInstall.cpp
    )

LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}
    )

ENDIF (MODULE_u_config)

