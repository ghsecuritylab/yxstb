
IF (MODULE_XMPP)
    LIST (APPEND DEFINITIONS
        "-DXMPP"
        )
    LIST (APPEND IPTV_CXX_SOURCES 
        ${CMAKE_CURRENT_LIST_DIR}/XmppAssertions.cpp
        ${CMAKE_CURRENT_LIST_DIR}/XmppService.cpp
        ${CMAKE_CURRENT_LIST_DIR}/XmppControlMessageParser.cpp
        ${CMAKE_CURRENT_LIST_DIR}/XmppMessageBridge.cpp
        ${CMAKE_CURRENT_LIST_DIR}/XmppandroidControlMessageParser.cpp
        )
    LIST (APPEND INCLUDES
        ${iptv_BINARY_DIR}/third_party/include/gloox
        ${CMAKE_CURRENT_LIST_DIR}
        )
ENDIF (MODULE_XMPP)

