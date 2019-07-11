IF (MODULE_Management STREQUAL "hybroad")
    LIST (APPEND STBMONITOR_CXX_SOURCES
        
        ${CMAKE_CURRENT_LIST_DIR}/MonitorManager.cpp
        ${CMAKE_CURRENT_LIST_DIR}/MonitorAgent.cpp
        ${CMAKE_CURRENT_LIST_DIR}/MonitorCmd.cpp

        #功能实现
        ${CMAKE_CURRENT_LIST_DIR}/Functions/MonitorTimer.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Functions/MonitorTool.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Functions/MonitorPing.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Functions/MonitorTraceroute.cpp

        #命令解析
        ${CMAKE_CURRENT_LIST_DIR}/Parser/ParseCmd.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Parser/ParseIoctl.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Parser/ParseInform.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Parser/ParseConnect.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Parser/ParseInitialize.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Parser/ParseRead.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Parser/ParseWrite.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Parser/ParseXML.cpp

        #解析XML工具TinyXML
        ${CMAKE_CURRENT_LIST_DIR}/Parser/TinyXML/TinyStr.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Parser/TinyXML/TinyXML.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Parser/TinyXML/TinyXMLError.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Parser/TinyXML/TinyXMLParser.cpp
        )

    include_directories( 
        ../middleware/src/include
        ../middleware/src/include/config
        ../middleware/src/app/mgmt/MgmtStbMonitor
        ${iptv_BINARY_DIR}
        ${iptv_BINARY_DIR}/third_party/include
        )

    LIST (APPEND INCLUDES
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/Parser/
        ${CMAKE_CURRENT_LIST_DIR}/Parser/TinyXML/
        ${CMAKE_CURRENT_LIST_DIR}/Functions/
        )

    foreach (INC ${INCLUDES})
        # MESSAGE (STATUS "INCLUDE: ${INC}")
        include_directories (${INC})
    endforeach (INC)

    foreach (DEF ${DEFINITIONS})
        # MESSAGE (STATUS "MACRO: ${DEF}")
        add_definitions (${DEF})
    endforeach (DEF)

    add_library (stbMonitor STATIC ${STBMONITOR_CXX_SOURCES})    
ENDIF (MODULE_Management STREQUAL "hybroad")



