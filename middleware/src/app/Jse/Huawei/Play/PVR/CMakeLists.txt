#IF (MODULE_cPVR) 移到前一目录

    LIST (APPEND IPTV_CXX_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/JseHWPVR.cpp
        ${CMAKE_CURRENT_LIST_DIR}/JseHWSchedule.cpp
        ${CMAKE_CURRENT_LIST_DIR}/JseHWRecordFiles.cpp
        #${CMAKE_CURRENT_LIST_DIR}/JseHWLocalTimeshift.cpp
        )
  
#ENDIF (MODULE_cPVR)
