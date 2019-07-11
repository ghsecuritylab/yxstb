
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/NativeHandlerCustomer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/NativeHandlerBootC20.cpp
    ${CMAKE_CURRENT_LIST_DIR}/NativeHandlerConfigC20.cpp
    ${CMAKE_CURRENT_LIST_DIR}/NativeHandlerErrorC20.cpp
    ${CMAKE_CURRENT_LIST_DIR}/NativeHandlerPublicC20.cpp
    ${CMAKE_CURRENT_LIST_DIR}/NativeHandlerRunningC20.cpp
    ${CMAKE_CURRENT_LIST_DIR}/NativeHandlerStandbyC20.cpp
    ${CMAKE_CURRENT_LIST_DIR}/NativeHandlerUConfigC20.cpp
    ${CMAKE_CURRENT_LIST_DIR}/NativeHandlerUpgradeC20.cpp
    )

LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}
    )


