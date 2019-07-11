

LIST (APPEND IPTV_C_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/codec/audio_drv.c
    ${CMAKE_CURRENT_LIST_DIR}/codec/codec.c
    ${CMAKE_CURRENT_LIST_DIR}/codec/display_drv.c
    ${CMAKE_CURRENT_LIST_DIR}/codec/video_drv.c
    ${CMAKE_CURRENT_LIST_DIR}/codec/YX_codec_porting.c
    ${CMAKE_CURRENT_LIST_DIR}/input/io_xkey.c
    # ${CMAKE_CURRENT_LIST_DIR}/mid_depend/mid_font.c
    ${CMAKE_CURRENT_LIST_DIR}/mid_depend/mid_fpanel.c
    #${CMAKE_CURRENT_LIST_DIR}/mid_depend/utility.c
    ${CMAKE_CURRENT_LIST_DIR}/mid_depend/mid_sys.c
    ${CMAKE_CURRENT_LIST_DIR}/middle_drv/checkip.c
    ${CMAKE_CURRENT_LIST_DIR}/middle_drv/ConfigPartition.c
    ${CMAKE_CURRENT_LIST_DIR}/middle_drv/gpio_drv.c
    ${CMAKE_CURRENT_LIST_DIR}/middle_drv/hotplug.c
    ${CMAKE_CURRENT_LIST_DIR}/middle_drv/net_drv.c
    )

LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/include_priv
    )


