
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/IAuthenticationCell.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/IDeviceCell.cpp 
    ${CMAKE_CURRENT_LIST_DIR}/IiPanelCell.cpp
    ${CMAKE_CURRENT_LIST_DIR}/IiPanelPageWidgetCell.cpp
    ${CMAKE_CURRENT_LIST_DIR}/IMediaPlayerCell.cpp
    ${CMAKE_CURRENT_LIST_DIR}/IUBankDeviceCell.cpp
    ${CMAKE_CURRENT_LIST_DIR}/IUtilityCell.cpp
)

LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}
    )


