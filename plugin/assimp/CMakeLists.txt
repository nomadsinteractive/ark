project(ark-assimp)
cmake_minimum_required(VERSION 3.15)

include(${ARK_SRC_DIR}/tools/cmake/core.cmake)

ark_find_vcpkg_package(assimp LIBRARIES assimp::assimp)

get_filename_component(LOCAL_PLUGIN_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR} PATH)
ark_include_directories(${ARK_INCLUDE_DIRS})

list(APPEND LOCAL_INCLUDE_DIRS ${ARK_PLUGIN_DIR})
list(APPEND LOCAL_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})
list(APPEND LOCAL_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR})

aux_source_directory(. LOCAL_SRC_LIST)
aux_source_directory(base LOCAL_SRC_LIST)
aux_source_directory(impl/animation LOCAL_SRC_LIST)
aux_source_directory(impl/io LOCAL_SRC_LIST)
aux_source_directory(impl/model_loader LOCAL_SRC_LIST)
aux_source_directory(util LOCAL_SRC_LIST)

ark_setup_tools(ark_gen_plugin_class)
ark_gen_plugin_class(assimp core ark::plugin::assimp assimp_plugin impl)

ark_add_plugin_library(${PROJECT_NAME} ${LOCAL_SRC_LIST} ${LOCAL_GENERATED_SRC_LIST})
ark_export_dependency_libraries(${PROJECT_NAME} assimp::assimp ${IRRXML_LIBRARY})
