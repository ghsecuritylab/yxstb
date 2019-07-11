
IF (MODULE_Management STREQUAL "hybroad")
    LIST (APPEND IPTV_CXX_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/Tr069Call.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Tr069FunctionCall.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Tr069GroupCall.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Tr069Root.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Tr069CallInit.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Tr069.cpp
        )

    LIST (APPEND INCLUDES
        ${CMAKE_CURRENT_LIST_DIR}/
        )

    #include (${CMAKE_CURRENT_LIST_DIR}/Tr069_v2/CMakeLists.txt)
    include (${CMAKE_CURRENT_LIST_DIR}/Device/CMakeLists.txt)
ENDIF (MODULE_Management STREQUAL "hybroad")

