
IF (MODULE_Wifi_Transform_Configure)
    LIST (APPEND IPTV_CXX_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/CConfig.cpp
        ${CMAKE_CURRENT_LIST_DIR}/TransformWifiConfig.cpp
        )
ENDIF (MODULE_Wifi_Transform_Configure)

