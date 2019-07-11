
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/Dialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/PowerOffDialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/DMRDialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/UpgradeDialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RestartDialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/NativeHandlerPublic.cpp
    ${CMAKE_CURRENT_LIST_DIR}/NativeHandlerRecovery.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ProgressBarDialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/EmergencyDialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/VisualizationDialog.cpp        
    
    )

LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}
    )

IF (BUILD_Support_LittleSystem)
    LIST (APPEND IPTV_CXX_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/NativeHandlerLittleSystem.cpp
        )
ENDIF (BUILD_Support_LittleSystem)

include (${CMAKE_CURRENT_LIST_DIR}/${BUILD_ContextType}/CMakeLists.txt)



