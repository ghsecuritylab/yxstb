
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/NativeHandlerPublicC10.cpp
    ${CMAKE_CURRENT_LIST_DIR}/NativeHandlerBootC10.cpp
    ${CMAKE_CURRENT_LIST_DIR}/NativeHandlerConfigC10.cpp
    ${CMAKE_CURRENT_LIST_DIR}/NativeHandlerErrorC10.cpp
    ${CMAKE_CURRENT_LIST_DIR}/NativeHandlerRunningC10.cpp
    ${CMAKE_CURRENT_LIST_DIR}/NativeHandlerStandbyC10.cpp
    ${CMAKE_CURRENT_LIST_DIR}/NativeHandlerUpgradeC10.cpp
    ${CMAKE_CURRENT_LIST_DIR}/NativeHandlerLocalC10.cpp
    ${CMAKE_CURRENT_LIST_DIR}/NativeHandlerNetworkDiagnoseC10.cpp
    ${CMAKE_CURRENT_LIST_DIR}/NativeHandlerRelinkC10.cpp
    )
    
IF (MODULE_u_config)
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/NativeHandlerUConfigC10.cpp
    )
ENDIF (MODULE_u_config)

LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}
    )

IF (${CUSTOMER} STREQUAL "Shanghai_HD")
    LIST (APPEND IPTV_CXX_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/ShanghaiHD/NativeHandlerCustomer.cpp
        )
    LIST (APPEND INCLUDES
        ${CMAKE_CURRENT_LIST_DIR}/ShanghaiHD
        )
ELSE (${CUSTOMER} STREQUAL "Shanghai_HD")
    LIST (APPEND IPTV_CXX_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/NativeHandlerCustomer.cpp)
ENDIF (${CUSTOMER} STREQUAL "Shanghai_HD")

