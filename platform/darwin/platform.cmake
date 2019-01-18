if(ARK_USE_OPEN_GL)
    find_package(OpenGL REQUIRED)
    list(APPEND LOCAL_LIBS ${OPENGL_gl_LIBRARY})
    list(APPEND LOCAL_INCLUDE_DIRS ${OPENGL_INCLUDE_DIR})

    ark_add_denpendency(3rdparty/glbinding glbinding 3rdparty/glbinding/source/glbinding/include)

    list(APPEND ARK_OPENGL_INCLUDE_DIRS
        ${ARK_SRC_DIR}/3rdparty/glbinding/source/glbinding/include
        ${PROJECT_BINARY_DIR}/3rdparty/glbinding/source/glbinding/include
        )
    set(ARK_OPENGL_LIBRARIES glbinding)
endif()

if(ARK_USE_VULKAN)
    ark_compile_definitions(-DVK_USE_PLATFORM_MACOS_MVK)
endif()

ark_include_directories($ENV{VULKAN_SDK}/../MoltenVK/include)

ark_compile_definitions(-DARK_USE_CONSTEXPR)
ark_compile_definitions(-DARK_PLATFORM_DARWIN)

aux_source_directory(platform/darwin/impl LOCAL_SRC_LIST)

set(LOCAL_OBJC_SRC_LIST
    platform/darwin/bridge.h
    platform/darwin/bridge.m
)
add_library(darwin_bridge STATIC ${LOCAL_OBJC_SRC_LIST})
ark_link_libraries(darwin_bridge)
