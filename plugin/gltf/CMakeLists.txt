project(ark-gltf)
cmake_minimum_required(VERSION 3.15)

include(${ARK_SRC_DIR}/tools/cmake/core.cmake)

get_filename_component(LOCAL_PLUGIN_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR} PATH)
ark_include_directories(${ARK_INCLUDE_DIRS})

find_path(TINYGLTF_INCLUDE_DIRS "tiny_gltf.h")
ark_include_directories(${TINYGLTF_INCLUDE_DIRS})

list(APPEND LOCAL_INCLUDE_DIRS ${ARK_PLUGIN_DIR})
list(APPEND LOCAL_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})
list(APPEND LOCAL_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR})

aux_source_directory(. LOCAL_SRC_LIST)
aux_source_directory(impl/model_loader LOCAL_SRC_LIST)

ark_setup_tools(ark_gen_plugin_class)
ark_gen_plugin_class(gltf core ark::plugin::gltf gltf_plugin impl)

ark_add_plugin_library(${PROJECT_NAME} ${LOCAL_SRC_LIST} ${LOCAL_GENERATED_SRC_LIST})
