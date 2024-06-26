project(ark-miniaudio)
cmake_minimum_required(VERSION 3.15)

include(${ARK_SRC_DIR}/tools/cmake/core.cmake)

ark_include_directories(${ARK_SRC_DIR})
ark_include_directories(${ARK_SRC_DIR}/src)
ark_include_directories(${CMAKE_CURRENT_SOURCE_DIR})
ark_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
ark_include_directories(${CMAKE_CURRENT_BINARY_DIR})

aux_source_directory(. LOCAL_SRC_LIST)
aux_source_directory(impl/audio_player LOCAL_SRC_LIST)

if(ANDROID)
    ark_link_libraries(OpenSLES)
    ark_export_dependency_libraries(OpenSLES)
endif()

if(IOS)
    add_library(miniaudio STATIC util/miniaudio.m)
    target_include_directories(miniaudio PRIVATE 3rdparty/miniaudio)
    ark_link_libraries(miniaudio)
    ark_export_dependency_libraries(miniaudio)
    ark_find_and_link_libraries(AVFoundation AudioToolbox)
else()
    ark_compile_definitions(-DARK_MINIAUDIO_UNIMPLEMENTED)
endif()

ark_setup_tools(ark_gen_plugin_class)
ark_gen_plugin_class(miniaudio core ark::plugin::miniaudio impl)

ark_add_plugin_library(${PROJECT_NAME} ${LOCAL_SRC_LIST} ${LOCAL_GENERATED_SRC_LIST})

find_path(MINIAUDIO_INCLUDE_DIRS "miniaudio.h")
target_include_directories(${PROJECT_NAME} PRIVATE ${MINIAUDIO_INCLUDE_DIRS})

ark_export_dependency_libraries(${PROJECT_NAME})
