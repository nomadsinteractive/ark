find_package(OpenGL REQUIRED)
list(APPEND LOCAL_LIBS ${OPENGL_gl_LIBRARY})
list(APPEND LOCAL_INCLUDE_DIRS ${OPENGL_INCLUDE_DIR})

ark_add_denpendency(3rdparty/glbinding ON glbinding 3rdparty/glbinding/source/glbinding/include)

list(APPEND ARK_OPENGL_INCLUDE_DIRS
    ${ARK_SRC_DIR}/3rdparty/glbinding/source/glbinding/include
    ${PROJECT_BINARY_DIR}/3rdparty/glbinding/source/glbinding/include
    )
set(ARK_OPENGL_LIBRARIES glbinding)

list(APPEND LOCAL_COMPILE_DEFINITIONS -DARK_USE_CONSTEXPR)

aux_source_directory(platform/darwin/impl LOCAL_SRC_LIST)
