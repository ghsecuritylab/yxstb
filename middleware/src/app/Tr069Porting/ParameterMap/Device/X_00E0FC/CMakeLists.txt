
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/Tr069AutoOnOffConfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Tr069PacketCapture.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Tr069LogParaConfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Tr069BandwidthDiagnostics.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Tr069X_00E0FC.cpp
    )

LIST (APPEND INCLUDES 
${CMAKE_CURRENT_LIST_DIR}../../../../CPEConfig/
${CMAKE_CURRENT_LIST_DIR}/
)



