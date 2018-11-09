project(ark-vorbis)
cmake_minimum_required(VERSION 3.0)

include(${ARK_SRC_DIR}/tools/cmake/core.cmake)

ark_add_denpendency(3rdparty/libogg ogg 3rdparty/libogg/include)
set(OGG_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/libogg/include)
set(OGG_LIBRARIES ogg)
ark_add_denpendency(3rdparty/libvorbis vorbis 3rdparty/libvorbis/include)
ark_link_libraries(vorbisfile)

list(APPEND LOCAL_INCLUDE_DIRS ${ARK_INCLUDE_DIRS})
list(APPEND LOCAL_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})
list(APPEND LOCAL_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/..)
list(APPEND LOCAL_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR})

aux_source_directory(. LOCAL_SRC_LIST)
aux_source_directory(impl/readable LOCAL_SRC_LIST)

ark_setup_tools(ark_gen_plugin_class)
ark_gen_plugin_class(vorbis core ark::plugin::vorbis vorbis_plugin impl)

ark_add_plugin_library(${PROJECT_NAME} ${LOCAL_SRC_LIST} ${LOCAL_GENERATED_SRC_LIST})
