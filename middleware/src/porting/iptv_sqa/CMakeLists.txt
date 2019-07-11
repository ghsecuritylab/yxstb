
IF (NOT MODULE_SQA STREQUAL "none") 
    LIST (APPEND IPTV_CXX_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/SqaAssertions.cpp
        )

    LIST (APPEND IPTV_C_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/SqaARQ.c
        ${CMAKE_CURRENT_LIST_DIR}/SqaFCC.c
        ${CMAKE_CURRENT_LIST_DIR}/SqaPublic.c
        ${CMAKE_CURRENT_LIST_DIR}/SqaRET.c
        ${CMAKE_CURRENT_LIST_DIR}/SqaRTCP.c
        ${CMAKE_CURRENT_LIST_DIR}/vde.c
        ${CMAKE_CURRENT_LIST_DIR}/vde_callback.c
        )

    LIST (APPEND INCLUDES
        ${CMAKE_CURRENT_LIST_DIR}/
        )
ELSE (NOT MODULE_SQA STREQUAL "none") 
    LIST (APPEND IPTV_C_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/sqa_dummy.c
        )
ENDIF (NOT MODULE_SQA STREQUAL "none") 

