
IF (MODULE_TVMS)
    LIST (APPEND IPTV_CXX_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/TvmsAessertions.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tvms.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tvms_define.cpp
        ${CMAKE_CURRENT_LIST_DIR}/tvms_setting.cpp
        )
    LIST (APPEND INCLUDES
        ${CMAKE_CURRENT_LIST_DIR}
        )
ENDIF (MODULE_TVMS)

