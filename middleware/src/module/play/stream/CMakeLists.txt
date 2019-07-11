

LIST (APPEND IPTV_C_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/http/http_ts_live.c
    ${CMAKE_CURRENT_LIST_DIR}/http/http_ts_play.c
    ${CMAKE_CURRENT_LIST_DIR}/http/http.c
    ${CMAKE_CURRENT_LIST_DIR}/http/http_live.c
    ${CMAKE_CURRENT_LIST_DIR}/http/http_ts.c
    ${CMAKE_CURRENT_LIST_DIR}/mid_stream.c
    ${CMAKE_CURRENT_LIST_DIR}/flash.c
    ${CMAKE_CURRENT_LIST_DIR}/mix/mix.c
    ${CMAKE_CURRENT_LIST_DIR}/mix/http_pcm.c
    ${CMAKE_CURRENT_LIST_DIR}/mix/mix_pcm.c
    ${CMAKE_CURRENT_LIST_DIR}/mix/http_mpa.c
    ${CMAKE_CURRENT_LIST_DIR}/mix/http_mp3.c
    ${CMAKE_CURRENT_LIST_DIR}/mix/mix_mp3.c
    ${CMAKE_CURRENT_LIST_DIR}/dvbs.c
    ${CMAKE_CURRENT_LIST_DIR}/strm_tools.c
    ${CMAKE_CURRENT_LIST_DIR}/mid_play.c
    ${CMAKE_CURRENT_LIST_DIR}/audio_stream.c
    ${CMAKE_CURRENT_LIST_DIR}/strm_play.c
    ${CMAKE_CURRENT_LIST_DIR}/strm_msgq.c
    ${CMAKE_CURRENT_LIST_DIR}/strm_buffer.c
    ${CMAKE_CURRENT_LIST_DIR}/rtsp/rtsp_app.c
    ${CMAKE_CURRENT_LIST_DIR}/rtsp/rtsp_stat.c
    ${CMAKE_CURRENT_LIST_DIR}/rtsp/rtsp_recv.c
    ${CMAKE_CURRENT_LIST_DIR}/rtsp/rtsp_clt.c
    ${CMAKE_CURRENT_LIST_DIR}/rtsp/rtsp_op_zte.c
    ${CMAKE_CURRENT_LIST_DIR}/rtsp/rtsp_op.c
    ${CMAKE_CURRENT_LIST_DIR}/rtsp/rtsp_op_huawei.c
    ${CMAKE_CURRENT_LIST_DIR}/rtsp/rtsp_op_live555.c
    )

LIST (APPEND IPTV_CXX_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/StreamAssertions.cpp
    )


LIST (APPEND DEFINITIONS
    "-DINCLUDE_MP3"
    )

IF (BUILD_SUPPORTHD)
    LIST (APPEND DEFINITIONS
        "-DSUPPORTE_HD=1"
        )
    LIST (APPEND IPTV_C_SOURCES 
        ${CMAKE_CURRENT_LIST_DIR}/http/http_apple.c
        ${CMAKE_CURRENT_LIST_DIR}/http/http_apple_audio.c
        ${CMAKE_CURRENT_LIST_DIR}/http/http_apple_buffer.c
        ${CMAKE_CURRENT_LIST_DIR}/http/http_apple_switch.c
        ${CMAKE_CURRENT_LIST_DIR}/mosaic.c
        ${CMAKE_CURRENT_LIST_DIR}/zebra.c
        ${CMAKE_CURRENT_LIST_DIR}/zebra_pcm.c
        )
ENDIF (BUILD_SUPPORTHD)


IF (MODULE_cPVR)
    LIST (APPEND IPTV_C_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/http/http_ts_record.c
        ${CMAKE_CURRENT_LIST_DIR}/pvr.c
        ${CMAKE_CURRENT_LIST_DIR}/mid_record.c
        ${CMAKE_CURRENT_LIST_DIR}/strm_record.c
        ${CMAKE_CURRENT_LIST_DIR}/strm_async.c
        )
ENDIF (MODULE_cPVR)

LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}
    )


