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
    ark_include_directories($ENV{MOLTONVK_HOME}/MoltenVK/include)

    find_library(MoltenVK_LIBRARY MoltenVK PATHS $ENV{MOLTONVK_HOME}/MoltenVK/iOS/static NO_CMAKE_FIND_ROOT_PATH)
    if(MoltenVK_LIBRARY EQUAL MoltenVK_LIBRARY-NOTFOUND)
        message(FATAL_ERROR "MoltenVK library not found")
    endif()
    ark_link_libraries(${MoltenVK_LIBRARY})

    find_library(QuartzCore_LIBRARY QuartzCore)
    ark_link_libraries(${QuartzCore_LIBRARY})

    find_library(Metal_LIBRARY Metal)
    ark_link_libraries(${Metal_LIBRARY})

    find_library(IOSurface_LIBRARY IOSurface)
    ark_link_libraries(${IOSurface_LIBRARY})

    find_library(UIKit_LIBRARY UIKit)
    ark_link_libraries(${UIKit_LIBRARY})
endif()

ark_compile_definitions(-DARK_USE_CONSTEXPR)
if(IOS)
    ark_compile_definitions(-DARK_PLATFORM_IOS)
endif()

ark_compile_definitions(-DARK_PLATFORM_DARWIN)

aux_source_directory(platform/darwin/impl LOCAL_SRC_LIST)

#set(LOCAL_OBJC_SRC_LIST
#    platform/darwin/bridge.h
#    platform/darwin/bridge.m
#)
#add_library(darwin_bridge STATIC ${LOCAL_OBJC_SRC_LIST})
#ark_link_libraries(darwin_bridge)
