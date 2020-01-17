project(ark-assimp)
cmake_minimum_required(VERSION 3.0)

include(${ARK_SRC_DIR}/tools/cmake/core.cmake)

set(ASSIMP_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/assimp)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_BUILD_SAMPLES OFF)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)

if(MSVC)
    add_compile_options(/utf-8)
endif()

if(ANDROID)
    set(ASSIMP_BUILD_BLEND_IMPORTER OFF)
    set(ASSIMP_BUILD_IFC_IMPORTER OFF)
    set(ASSIMP_BUILD_MMD_IMPORTER OFF)
    set(ASSIMP_BUILD_X3D_IMPORTER OFF)
endif()

# Assimp(v4.1.0) tries to include config.h in the source directory prior to the one generated in the binary directory,
# which might cause compiler errors. So here is a workaround below
include_directories(${CMAKE_CURRENT_BINARY_DIR}/3rdparty/assimp/include)
add_subdirectory(3rdparty/assimp)

get_filename_component(LOCAL_PLUGIN_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR} PATH)
ark_include_directories(${ARK_INCLUDE_DIRS})

list(APPEND LOCAL_INCLUDE_DIRS ${ARK_PLUGIN_DIR})
list(APPEND LOCAL_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})
list(APPEND LOCAL_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/assimp/include)
list(APPEND LOCAL_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR})
list(APPEND LOCAL_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/3rdparty/assimp/include)

ark_link_libraries(assimp)

aux_source_directory(. LOCAL_SRC_LIST)
aux_source_directory(impl/io LOCAL_SRC_LIST)
aux_source_directory(impl/model_loader LOCAL_SRC_LIST)
aux_source_directory(impl/vertices LOCAL_SRC_LIST)

ark_setup_tools(ark_gen_plugin_class)
ark_gen_plugin_class(assimp core ark::plugin::assimp assimp_plugin impl)

ark_add_plugin_library(${PROJECT_NAME} ${LOCAL_SRC_LIST} ${LOCAL_GENERATED_SRC_LIST})
ark_export_dependency_libraries(${PROJECT_NAME} assimp ${IRRXML_LIBRARY})
