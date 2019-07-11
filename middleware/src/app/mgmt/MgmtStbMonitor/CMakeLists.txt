IF (MODULE_Management STREQUAL "hybroad")

	LIST (APPEND IPTV_C_SOURCES
    #stbMonitor
		${CMAKE_CURRENT_LIST_DIR}/Parser/mgmtModuleParam.c
		${CMAKE_CURRENT_LIST_DIR}/Functions/mgmtCliSelfDefineCmd.c
		)
	LIST (APPEND IPTV_CXX_SOURCES
    #stbMonitor
		${CMAKE_CURRENT_LIST_DIR}/Parser/mgmtModuleStbMonitor.cpp
		${CMAKE_CURRENT_LIST_DIR}/Parser/mgmtMsgDealFunc.cpp
		${CMAKE_CURRENT_LIST_DIR}/Functions/mgmtMonitorChannel.cpp
		${CMAKE_CURRENT_LIST_DIR}/Functions/mgmtModuleCmdLine.cpp

    ${CMAKE_CURRENT_LIST_DIR}/MonitorAssertions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/MonitorLog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/MonitorConfig.cpp
		)

	LIST (APPEND INCLUDES
		${CMAKE_CURRENT_LIST_DIR}
		${CMAKE_CURRENT_LIST_DIR}/Functions/
		${CMAKE_CURRENT_LIST_DIR}/Parser/
		)

IF (BUILD_Android)
ELSE (BUILD_Android)
LIST (APPEND IPTV_CXX_SOURCES

    ${PROJECT_TOP_DIR}/hybroad/monitor/MonitorAgent.cpp
    ${PROJECT_TOP_DIR}/hybroad/monitor/MonitorManager.cpp
    ${PROJECT_TOP_DIR}/hybroad/monitor/MonitorCmd.cpp

    #功能实现
    ${PROJECT_TOP_DIR}/hybroad/monitor/Functions/MonitorTool.cpp
    ${PROJECT_TOP_DIR}/hybroad/monitor/Functions/MonitorPing.cpp
    ${PROJECT_TOP_DIR}/hybroad/monitor/Functions/MonitorTraceroute.cpp
    ${PROJECT_TOP_DIR}/hybroad/monitor/Functions/MonitorTimer.cpp

    #命令解析
    ${PROJECT_TOP_DIR}/hybroad/monitor/Parser/ParseCmd.cpp
    ${PROJECT_TOP_DIR}/hybroad/monitor/Parser/ParseIoctl.cpp
    ${PROJECT_TOP_DIR}/hybroad/monitor/Parser/ParseInform.cpp
    ${PROJECT_TOP_DIR}/hybroad/monitor/Parser/ParseConnect.cpp
    ${PROJECT_TOP_DIR}/hybroad/monitor/Parser/ParseInitialize.cpp
    ${PROJECT_TOP_DIR}/hybroad/monitor/Parser/ParseRead.cpp
    ${PROJECT_TOP_DIR}/hybroad/monitor/Parser/ParseWrite.cpp
    ${PROJECT_TOP_DIR}/hybroad/monitor/Parser/ParseXML.cpp

    #解析XML工具TinyXML
    ${PROJECT_TOP_DIR}/hybroad/monitor/Parser/TinyXML/TinyStr.cpp
    ${PROJECT_TOP_DIR}/hybroad/monitor/Parser/TinyXML/TinyXML.cpp
    ${PROJECT_TOP_DIR}/hybroad/monitor/Parser/TinyXML/TinyXMLError.cpp
    ${PROJECT_TOP_DIR}/hybroad/monitor/Parser/TinyXML/TinyXMLParser.cpp
        )   

LIST (APPEND INCLUDES
    ${PROJECT_TOP_DIR}/hybroad/monitor/
    ${PROJECT_TOP_DIR}/hybroad/monitor/Parser/
    ${PROJECT_TOP_DIR}/hybroad/monitor/Parser/TinyXML/
    ${PROJECT_TOP_DIR}/hybroad/monitor/Functions/
    ) 

ENDIF (BUILD_Android)
ENDIF (MODULE_Management STREQUAL "hybroad")



