
IF (MODULE_DLNA)
    IF (MODULE_DLNADMR)
        LIST (APPEND IPTV_CXX_SOURCES
            ${CMAKE_CURRENT_LIST_DIR}/DlnaAssertions.cpp
            ${CMAKE_CURRENT_LIST_DIR}/DLNARegister.cpp
            ${CMAKE_CURRENT_LIST_DIR}/DLNANat.cpp
            ${CMAKE_CURRENT_LIST_DIR}/DMSFileList.cpp
            ${CMAKE_CURRENT_LIST_DIR}/DMRManager.cpp
            ${CMAKE_CURRENT_LIST_DIR}/DMRPlayer.cpp
            ${CMAKE_CURRENT_LIST_DIR}/DMRPlayerHuawei.cpp
            ${CMAKE_CURRENT_LIST_DIR}/DMRPlayerHuaweiC30.cpp
            ${CMAKE_CURRENT_LIST_DIR}/DMRPlayerCTC.cpp
            )

        LIST (APPEND IPTV_C_SOURCES
            ${CMAKE_CURRENT_LIST_DIR}/mid_dlna.c
            ${CMAKE_CURRENT_LIST_DIR}/mid_dlna_ex.c
            ${CMAKE_CURRENT_LIST_DIR}/mid_dms.c
            ${CMAKE_CURRENT_LIST_DIR}/platform/avplayer.c
            ${CMAKE_CURRENT_LIST_DIR}/platform/imageplayer.c
            ${CMAKE_CURRENT_LIST_DIR}/platform/pvr_fs.c
            ${CMAKE_CURRENT_LIST_DIR}/platform/renderingcontrol.c
            )
        LIST (APPEND INCLUDES
            ${CMAKE_CURRENT_LIST_DIR}/include
            ${CMAKE_CURRENT_LIST_DIR}/include/dlna
            ${CMAKE_CURRENT_LIST_DIR}/include/platform
            ${CMAKE_CURRENT_LIST_DIR}/include/upnp
            ${CMAKE_CURRENT_LIST_DIR}/platform
            ${CMAKE_CURRENT_LIST_DIR}
            )
    ENDIF (MODULE_DLNADMR)
ENDIF (MODULE_DLNA)


