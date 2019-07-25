project(ark-pbr)
cmake_minimum_required(VERSION 3.0)

include(${ARK_SRC_DIR}/tools/cmake/core.cmake)

ark_ensure_dependency(3rdparty/cmft)

list(APPEND LOCAL_INCLUDE_DIRS .)
ark_include_directories(${ARK_INCLUDE_DIRS})

list(APPEND LOCAL_INCLUDE_DIRS ${ARK_PLUGIN_DIR})
list(APPEND LOCAL_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR})

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    add_definitions(-D__i386__)
else()
    add_definitions(-D__amd64__)
endif()

ark_include_directories(3rdparty/cmft/include)
ark_include_directories(3rdparty/cmft/dependency)

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

ark_add_plugin_library(${PROJECT_NAME} ${LOCAL_SRC_LIST} ${LOCAL_GENERATED_SRC_LIST})
ark_export_dependency_libraries(${PROJECT_NAME})
