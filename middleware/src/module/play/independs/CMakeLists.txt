
LIST (APPEND IPTV_C_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/ind_time.c
    ${CMAKE_CURRENT_LIST_DIR}/ind_pcm.c
    ${CMAKE_CURRENT_LIST_DIR}/ind_net.c
    ${CMAKE_CURRENT_LIST_DIR}/ind_rtp.c
    ${CMAKE_CURRENT_LIST_DIR}/ind_cfg.c
    ${CMAKE_CURRENT_LIST_DIR}/ind_mem.c
    ${CMAKE_CURRENT_LIST_DIR}/ind_tmr.c
    ${CMAKE_CURRENT_LIST_DIR}/ind_string.c
    ${CMAKE_CURRENT_LIST_DIR}/gfx/img_jpg.c
    ${CMAKE_CURRENT_LIST_DIR}/gfx/img_bmp.c
    ${CMAKE_CURRENT_LIST_DIR}/gfx/img_bio.c
    ${CMAKE_CURRENT_LIST_DIR}/gfx/img_inflate.c
    ${CMAKE_CURRENT_LIST_DIR}/gfx/ind_gfx.c
    ${CMAKE_CURRENT_LIST_DIR}/gfx/img_gif.c
    ${CMAKE_CURRENT_LIST_DIR}/gfx/img_decode.c
    # ${CMAKE_CURRENT_LIST_DIR}/gfx/ind_gfx32.c
    # ${CMAKE_CURRENT_LIST_DIR}/gfx/ind_gfx16.c
    ${CMAKE_CURRENT_LIST_DIR}/gfx/ind_gif.c
    ${CMAKE_CURRENT_LIST_DIR}/gfx/img_png.c
    ${CMAKE_CURRENT_LIST_DIR}/ts/ts_pts.c
    ${CMAKE_CURRENT_LIST_DIR}/ts/ts_iframe.c
    ${CMAKE_CURRENT_LIST_DIR}/ts/ts_psi.c
    ${CMAKE_CURRENT_LIST_DIR}/ts/ts_audio.c
    ${CMAKE_CURRENT_LIST_DIR}/ts/ts_buffer.c
    ${CMAKE_CURRENT_LIST_DIR}/ts/ts_size.c
    )

IF (MODULE_cPVR)
    LIST (APPEND  IPTV_C_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/pvr/ind_pvr.c
        ${CMAKE_CURRENT_LIST_DIR}/pvr/ind_pvr_file.c
        )
ENDIF (MODULE_cPVR)

LIST (APPEND INCLUDES 
    ${CMAKE_CURRENT_LIST_DIR}
    )




