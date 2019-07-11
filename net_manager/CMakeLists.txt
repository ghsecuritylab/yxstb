
LIST (APPEND NETMANAGER_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/json/arraylist.c
    ${CMAKE_CURRENT_LIST_DIR}/json/debug.c
    ${CMAKE_CURRENT_LIST_DIR}/json/json_object.c
    ${CMAKE_CURRENT_LIST_DIR}/json/json_public.c
    ${CMAKE_CURRENT_LIST_DIR}/json/json_tokener.c
    ${CMAKE_CURRENT_LIST_DIR}/json/json_util.c
    ${CMAKE_CURRENT_LIST_DIR}/json/linkhash.c
    ${CMAKE_CURRENT_LIST_DIR}/json/printbuf.c
    ${CMAKE_CURRENT_LIST_DIR}/common/aesCrypto.c
    ${CMAKE_CURRENT_LIST_DIR}/common/mid_select.c
    ${CMAKE_CURRENT_LIST_DIR}/common/mid_msgq.c
    ${CMAKE_CURRENT_LIST_DIR}/common/yx_crypto.c
    ${CMAKE_CURRENT_LIST_DIR}/common/mid_timer.c
    ${CMAKE_CURRENT_LIST_DIR}/dbg_utils/nm_dbg.c
    ${CMAKE_CURRENT_LIST_DIR}/tr069_interface.c
    ${CMAKE_CURRENT_LIST_DIR}/android/ICallbackListener.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/android/tr069_param_map.cpp
    ${CMAKE_CURRENT_LIST_DIR}/android/local_service.cpp
    ${CMAKE_CURRENT_LIST_DIR}/android/IHybroadService.cpp
    ${CMAKE_CURRENT_LIST_DIR}/android/BpHybroadService.cpp
    ${CMAKE_CURRENT_LIST_DIR}/android/CallbackListener.cpp
    ${CMAKE_CURRENT_LIST_DIR}/android/init.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/android/test_port.cpp
    ${CMAKE_CURRENT_LIST_DIR}/android/HybroadService.cpp
    ${CMAKE_CURRENT_LIST_DIR}/android/Android_port.cpp
    ${CMAKE_CURRENT_LIST_DIR}/android/hash_func.cpp
    ${CMAKE_CURRENT_LIST_DIR}/android/param_map.cpp
    ${CMAKE_CURRENT_LIST_DIR}/android/BpCallbackListener.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tr069PactetCapture/tr069_port_PacketCapture.c
    ${CMAKE_CURRENT_LIST_DIR}/HybroadLogcat/LogPostTerminal.cpp
    ${CMAKE_CURRENT_LIST_DIR}/HybroadLogcat/RingBuffer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/HybroadLogcat/MonitorLogPostFTP.cpp
    ${CMAKE_CURRENT_LIST_DIR}/HybroadLogcat/DataRecive.cpp
    ${CMAKE_CURRENT_LIST_DIR}/HybroadLogcat/LogInit.cpp
    ${CMAKE_CURRENT_LIST_DIR}/HybroadLogcat/tr069_log.cpp
    ${CMAKE_CURRENT_LIST_DIR}/HybroadLogcat/LogFilter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/HybroadLogcat/LogPool.cpp
    ${CMAKE_CURRENT_LIST_DIR}/HybroadLogcat/DataSink.cpp
    ${CMAKE_CURRENT_LIST_DIR}/HybroadLogcat/MonitorLogPostUDP.cpp
    ${CMAKE_CURRENT_LIST_DIR}/HybroadLogcat/DataSource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/HybroadLogcat/MonitorToolLog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/HybroadLogcat/PlatBuffer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/HybroadLogcat/LogPostUDP.cpp
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    )


LIST (APPEND INCLUDES
    ${CMAKE_CURRENT_LIST_DIR}/
    ${CMAKE_CURRENT_LIST_DIR}/../tr069
    ${CMAKE_CURRENT_LIST_DIR}/json
    ${CMAKE_CURRENT_LIST_DIR}/dbg_utils
    ${CMAKE_CURRENT_LIST_DIR}/android
    ${CMAKE_CURRENT_LIST_DIR}/common
    ${CMAKE_CURRENT_LIST_DIR}/HybroadLogcat
    ${CMAKE_CURRENT_LIST_DIR}/tr069PactetCapture
    ${iptv_BINARY_DIR}/third_party/include
    ${iptv_BINARY_DIR}/third_party/include/freetype2
    ${iptv_BINARY_DIR}/third_party/include/pixman
    ${iptv_BINARY_DIR}/third_party/include/curl
    ${iptv_BINARY_DIR}/third_party/include/cairo
    ${iptv_BINARY_DIR}/third_party/include/zlib
    ${iptv_BINARY_DIR}/third_party/include/jpeg
    ${iptv_BINARY_DIR}/third_party/include/png
    ${iptv_BINARY_DIR}
    )

foreach (INC ${INCLUDES})
    include_directories (${INC})
endforeach (INC)

foreach (DEF ${DEFINITIONS})
    add_definitions (${DEF})
endforeach (DEF)

add_definitions(-DANDROID_LOGCAT_OUTPUT)

foreach (LDS ${LIBS_DIRECTORIES})
    link_directories (${LDS})
endforeach (LDS)

add_executable(net_manager ${NETMANAGER_SOURCES})

add_dependencies(net_manager
    platform_sdk
    stbMonitor
    tr069
    )

target_link_libraries(net_manager 
    crypto
    stlport
    log
    utils
    binder
    z
    curl_withssl
    c
    stdc++
    m
    tr069
    stbMonitor
    cutils
    ${CRTBEGIN}
    ${CRTEND}
    )

foreach (LF ${ANDROID_NATIVE_LDFLAGS})
    target_link_libraries(net_manager ${LF})
endforeach (LF)

# /home/kazuki/tmp3/crtbegin_dynamic.o 
# /home/kazuki/tmp3/crtend_android.o
# -Wl,-rpath-link=out/target/product/Hi3719MV100/obj/lib
# out/target/product/Hi3719MV100/obj/lib/crtbegin_dynamic.o 
# -Wl,--whole-archive   
# -Wl,--no-whole-archive
# out/target/product/Hi3719MV100/obj/STATIC_LIBRARIES/libcompiler-rt-extras_intermediates/libcompiler-rt-extras.a
# out/target/product/Hi3719MV100/obj/lib/crtend_android.o




