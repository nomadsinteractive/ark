project(ark-portaudio)
cmake_minimum_required(VERSION 2.8)

include(${ARK_SRC_DIR}/tools/cmake/core.cmake)

ark_add_denpendency(3rdparty/portaudio OFF portaudio_static 3rdparty/portaudio/include 3rdparty/portaudio/src/common)

list(APPEND LOCAL_LIBS ${ARK_LIBRARIES})

list(APPEND LOCAL_INCLUDE_DIRS ${ARK_SRC_DIR})
list(APPEND LOCAL_INCLUDE_DIRS ${ARK_SRC_DIR}/src)
list(APPEND LOCAL_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})
list(APPEND LOCAL_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/..)
list(APPEND LOCAL_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR})

aux_source_directory(. LOCAL_SRC_LIST)
aux_source_directory(impl/audio_player LOCAL_SRC_LIST)
aux_source_directory(impl/readable LOCAL_SRC_LIST)

ark_setup_tools(ark_gen_plugin_class)
ark_gen_plugin_class(portaudio core ark::plugin::portaudio portaudio_plugin impl)

ark_add_shared_library(${PROJECT_NAME} ${LOCAL_SRC_LIST} ${LOCAL_GENERATED_SRC_LIST})
