
aux_source_directory(platform/android/opensles LOCAL_SRC_LIST)
list(APPEND LOCAL_LIBS OpenSLES)

add_definitions(-DPA_USE_SKELETON=1)

list(APPEND LOCAL_LIBS OpenSLES)
list(APPEND LOCAL_COMPILE_DEFINITIONS -DPA_LITTLE_ENDIAN)
