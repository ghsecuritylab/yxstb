
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/JseHWModules.cpp
    )
    
IF (MODULE_SHA256)
    include (${CMAKE_CURRENT_LIST_DIR}/SHA256/CMakeLists.txt)
ENDIF (MODULE_SHA256)

IF (MODULE_TVMS)
    include (${CMAKE_CURRENT_LIST_DIR}/TVMS/CMakeLists.txt)
ENDIF (MODULE_TVMS)

IF (MODULE_Disk)
    include (${CMAKE_CURRENT_LIST_DIR}/HDDmangment/CMakeLists.txt)
ENDIF (MODULE_Disk)

IF (MODULE_PIP)
    include (${CMAKE_CURRENT_LIST_DIR}/PIP/CMakeLists.txt)
ENDIF (MODULE_PIP)

include (${CMAKE_CURRENT_LIST_DIR}/CA/CMakeLists.txt)

IF (MODULE_XMPP)
    include (${CMAKE_CURRENT_LIST_DIR}/Xmpp/CMakeLists.txt)
ENDIF (MODULE_XMPP)

IF (MODULE_PayShell)
    include (${CMAKE_CURRENT_LIST_DIR}/PayShell/CMakeLists.txt)   
ENDIF (MODULE_PayShell)

include (${CMAKE_CURRENT_LIST_DIR}/IMS/CMakeLists.txt)   