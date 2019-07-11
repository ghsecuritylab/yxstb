IF (NOT MODULE_NetworkDiagnose)
    return()
ENDIF (NOT MODULE_NetworkDiagnose)

LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/JseNetDiagnoseTool.cpp
)
