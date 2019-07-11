
IF (BUILD_Android)
    LIST (APPEND IPTV_CXX_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/JseHWNetDiagnoseToolAndroid.cpp
        )
ELSEIF (MODULE_NetworkDiagnose)
    LIST (APPEND IPTV_CXX_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/JseHWNetDiagnoseTool.cpp
        )
ELSE (BUILD_Android)
ENDIF (BUILD_Android)

