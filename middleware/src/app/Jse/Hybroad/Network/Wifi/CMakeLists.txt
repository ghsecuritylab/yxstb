IF (NOT MODULE_Wifi)
    return()
ENDIF (NOT MODULE_Wifi)

LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/JseWifi.cpp
    )
    
