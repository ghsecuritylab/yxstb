

LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/customer.cpp
    )
include (${CMAKE_CURRENT_LIST_DIR}/${CUSTOMER}/${STBTYPE}/CMakeLists.txt)

return()

# c10, c20
string (TOLOWER ${BUILD_ContextType} TEMP)
set (STDCONFFILE ${CMAKE_CURRENT_LIST_DIR}/${TEMP}.c)

IF (EXISTS ${STDCONFFILE})
    list (APPEND IPTV_C_SOURCES ${STDCONFFILE})
    MESSAGE (STATUS "Standard configure file ${STDCONFFILE} found.")
ELSE (EXISTS ${STDCONFFILE})
    MESSAGE (STATUS "Standard configure file ${STDCONFFILE} NOT found.")
ENDIF (EXISTS ${STDCONFFILE})

LIST (APPEND INITFILES_C
    ${CMAKE_CURRENT_LIST_DIR}/${CUSTOMER}.c
    ${CMAKE_CURRENT_LIST_DIR}/${CUSTOMER}.${STBTYPE}.c
    )

LIST (APPEND INITFILES_CXX
    ${CMAKE_CURRENT_LIST_DIR}/${CUSTOMER}.cpp
    ${CMAKE_CURRENT_LIST_DIR}/${CUSTOMER}.${STBTYPE}.cpp
    )

set (IFFLAG OFF)
foreach (F ${INITFILES_C})
    IF (EXISTS ${F})
        list (APPEND IPTV_C_SOURCES ${F})
        MESSAGE (STATUS "Customer configure file ${F} found.")
        set (IFFLAG ON)
        break()
    ENDIF (EXISTS ${F})
endforeach (F)
foreach (F ${INITFILES_CXX})
    IF (EXISTS ${F})
        list (APPEND IPTV_CXX_SOURCES ${F})
        MESSAGE (STATUS "Customer configure file ${F} found.")
        set (IFFLAG ON)
        break()
    ENDIF (EXISTS ${F})
endforeach (F)

IF (NOT IFFLAG)
    MESSAGE (FATAL_ERROR " No customer configure init file found. You need a ${CUSTOMER}.c(pp) or a ${CUSTOMER}.${STBTYPE}.c(pp)")
ENDIF (NOT IFFLAG)


