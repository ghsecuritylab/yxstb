list (APPEND NETWORK_SRC_LIST
    ${NETWORK_MODULE_DIR}/DiagnoseTools/NetworkDiagnose.cpp
)

IF (BUILD_Android)
    return ()    
ENDIF (BUILD_Android)

include_directories (
    ${NETWORK_MODULE_DIR}/DiagnoseTools/Basic
    ${NETWORK_MODULE_DIR}/DiagnoseTools/Advanced
)

include (${NETWORK_MODULE_DIR}/DiagnoseTools/Basic/CMakeLists.txt)
include (${NETWORK_MODULE_DIR}/DiagnoseTools/Advanced/CMakeLists.txt)
