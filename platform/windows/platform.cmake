
aux_source_directory(platform/windows/impl LOCAL_SRC_LIST)
aux_source_directory(platform/windows/impl/runtime LOCAL_RUNTIME_LIBRARY_SRC_LIST)

list(APPEND ARK_COMPILE_DEFINITIONS -DARK_PLATFORM_WINDOWS)
