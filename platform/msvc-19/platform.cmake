
add_definitions(-DHAVE_STRUCT_TIMESPEC)
list(APPEND LOCAL_COMPILE_DEFINITIONS -DARK_USE_CONSTEXPR)

list(APPEND ARK_GLBINDINGS_INCLUDE_DIRS ${ARK_SRC_DIR}/3rdparty/glbinding/source/glbinding/include)
list(APPEND ARK_GLBINDINGS_INCLUDE_DIRS ${PROJECT_BINARY_DIR}/3rdparty/glbinding/source/glbinding/include)
