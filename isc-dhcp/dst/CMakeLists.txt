
include_directories (
    ${CMAKE_CURRENT_LIST_DIR}/
    )

list (APPEND DHCLIENT_SRC_LIST 
    ${CMAKE_CURRENT_LIST_DIR}/base64.c
    ${CMAKE_CURRENT_LIST_DIR}/dst_api.c
    ${CMAKE_CURRENT_LIST_DIR}/dst_support.c
    ${CMAKE_CURRENT_LIST_DIR}/hmac_link.c
    ${CMAKE_CURRENT_LIST_DIR}/prandom.c
    )
