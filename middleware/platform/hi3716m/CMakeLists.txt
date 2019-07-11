

LIST (APPEND IPTV_C_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/decode/codec.c
    ${CMAKE_CURRENT_LIST_DIR}/decode/YX_codec_porting.c
    ${CMAKE_CURRENT_LIST_DIR}/input/io_xkey.c
    # ${CMAKE_CURRENT_LIST_DIR}/mid_depend/mid_font_asc2412.c
    ${CMAKE_CURRENT_LIST_DIR}/mid_depend/mid_fpanel.c
    # ${CMAKE_CURRENT_LIST_DIR}/mid_depend/mid_fs.c
    ${CMAKE_CURRENT_LIST_DIR}/mid_depend/mid_sys.c
    )

include_directories(
		${PROJECT_SOURCE_DIR}/EC2108_C27/ipstb/src/include/config
	)

