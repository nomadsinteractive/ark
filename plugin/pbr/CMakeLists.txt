project(ark-pbr)
cmake_minimum_required(VERSION 3.0)

include(${ARK_SRC_DIR}/tools/cmake/core.cmake)

ark_ensure_dependency(3rdparty/cmft)
ark_ensure_dependency(3rdparty/sht)

list(APPEND LOCAL_LIBS ${ARK_LIBRARIES})

list(APPEND LOCAL_INCLUDE_DIRS .)
list(APPEND LOCAL_INCLUDE_DIRS ${ARK_SRC_DIR})
list(APPEND LOCAL_INCLUDE_DIRS ${ARK_SRC_DIR}/src)

list(APPEND LOCAL_INCLUDE_DIRS ${ARK_PLUGIN_DIR})

list(APPEND LOCAL_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR})

ark_include_directories(${ARK_OPENGL_INCLUDE_DIRS})
ark_include_directories(3rdparty/sht)
ark_include_directories(3rdparty/cmft/include)
ark_include_directories(3rdparty/cmft/dependency)

list(APPEND LOCAL_LIBS ${ARK_OPENGL_LIBRARIES})

aux_source_directory(. LOCAL_SRC_LIST)
aux_source_directory(impl/texture LOCAL_SRC_LIST)

list(APPEND LOCAL_SRC_LIST
    3rdparty/cmft/src/cmft/allocator.cpp
    3rdparty/cmft/src/cmft/clcontext.cpp
    3rdparty/cmft/src/cmft/cubemapfilter.cpp
    3rdparty/cmft/src/cmft/image.cpp
    3rdparty/cmft/src/cmft/common/print.cpp
    3rdparty/cmft/src/cmft/common/stb_image.cpp
)

ark_setup_tools(gen_plugin_class)
ark_gen_plugin_class(pbr builtin ark::plugin::pbr pbr_plugin impl)

ark_add_shared_library(${PROJECT_NAME} ${LOCAL_SRC_LIST} ${LOCAL_GENERATED_SRC_LIST})
