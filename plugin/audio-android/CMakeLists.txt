project(ark-audio-android)
cmake_minimum_required(VERSION 3.0)

include(${ARK_SRC_DIR}/tools/cmake/core.cmake)

ark_include_directories(${ARK_SRC_DIR})
ark_include_directories(${ARK_SRC_DIR}/src)
ark_include_directories(${CMAKE_CURRENT_SOURCE_DIR})
ark_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
ark_include_directories(${CMAKE_CURRENT_BINARY_DIR})

aux_source_directory(. LOCAL_SRC_LIST)
aux_source_directory(impl/audio_player LOCAL_SRC_LIST)

add_subdirectory(3rdparty/oboe)
ark_include_directories(3rdparty/oboe/include)

ark_link_libraries(oboe aaudio)

ark_setup_tools(ark_gen_plugin_class)
ark_gen_plugin_class(audio_android core ark::plugin::audio_android audio_android_plugin impl)

ark_add_plugin_library(${PROJECT_NAME} ${LOCAL_SRC_LIST} ${LOCAL_GENERATED_SRC_LIST})
