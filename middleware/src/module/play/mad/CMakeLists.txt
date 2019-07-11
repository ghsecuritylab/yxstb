

LIST (APPEND IPTV_C_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/bit.c
    ${CMAKE_CURRENT_LIST_DIR}/fixed.c
    ${CMAKE_CURRENT_LIST_DIR}/frame.c
    ${CMAKE_CURRENT_LIST_DIR}/huffman.c
    ${CMAKE_CURRENT_LIST_DIR}/layer12.c
    ${CMAKE_CURRENT_LIST_DIR}/layer3.c
    ${CMAKE_CURRENT_LIST_DIR}/minimad.c
    ${CMAKE_CURRENT_LIST_DIR}/stream.c
    ${CMAKE_CURRENT_LIST_DIR}/synth.c
    ${CMAKE_CURRENT_LIST_DIR}/timer.c
    ${CMAKE_CURRENT_LIST_DIR}/version.c
    )

LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}
    )


