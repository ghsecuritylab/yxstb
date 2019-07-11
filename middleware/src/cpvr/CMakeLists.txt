
IF (MODULE_cPVR)
    LIST (APPEND IPTV_CXX_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/CpvrAssertions.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Cpvr.cpp
        ${CMAKE_CURRENT_LIST_DIR}/CpvrJsCall.cpp
        ${CMAKE_CURRENT_LIST_DIR}/CpvrRes.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Tstv.cpp
        ${CMAKE_CURRENT_LIST_DIR}/CpvrAuxTools.cpp
        ${CMAKE_CURRENT_LIST_DIR}/CpvrDB.cpp
        ${CMAKE_CURRENT_LIST_DIR}/CpvrList.cpp
        ${CMAKE_CURRENT_LIST_DIR}/CpvrTaskManager.cpp
        ${CMAKE_CURRENT_LIST_DIR}/UserManager.cpp
        )

    LIST (APPEND INCLUDES
        ${CMAKE_CURRENT_LIST_DIR}/
        )
ENDIF (MODULE_cPVR)


