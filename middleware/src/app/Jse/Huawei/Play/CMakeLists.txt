
LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/JseHWPlay.cpp
    ${CMAKE_CURRENT_LIST_DIR}/JseHWAdditional.cpp
    ${CMAKE_CURRENT_LIST_DIR}/JseHWMedia.cpp
    ${CMAKE_CURRENT_LIST_DIR}/JseHWStream.cpp
    )

include (${CMAKE_CURRENT_LIST_DIR}/Audio/CMakeLists.txt) 
include (${CMAKE_CURRENT_LIST_DIR}/Subtitle/CMakeLists.txt) 

IF (MODULE_cPVR)
    include (${CMAKE_CURRENT_LIST_DIR}/PVR/CMakeLists.txt) 
ENDIF (MODULE_cPVR)

IF (NOT MODULE_SQA STREQUAL "none")
    include (${CMAKE_CURRENT_LIST_DIR}/SQA/CMakeLists.txt) 
ENDIF (NOT MODULE_SQA STREQUAL "none")  

IF (NOT MODULE_SQM_VER STREQUAL "none")
    include (${CMAKE_CURRENT_LIST_DIR}/SQM/CMakeLists.txt) 
ENDIF (NOT MODULE_SQM_VER STREQUAL "none")

IF (MODULE_LocalPlayer)
    include (${CMAKE_CURRENT_LIST_DIR}/LocalPlayer/CMakeLists.txt) 
ENDIF (MODULE_LocalPlayer)

#IF (MODULE_DVBS_Tuner)
#include (${CMAKE_CURRENT_LIST_DIR}/DVB/CMakeLists.txt) 
#ENDIF (MODULE_DVBS_Tuner)

IF (MODULE_Download)
    include (${CMAKE_CURRENT_LIST_DIR}/Download/CMakeLists.txt)
ENDIF (MODULE_Download)