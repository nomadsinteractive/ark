project(ark-fmod)
cmake_minimum_required(VERSION 2.8)

include(${ARK_SRC_DIR}/tools/cmake/core.cmake)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules)

find_package(FMOD)

if(NOT FMOD_FOUND)
    message(FATAL_ERROR "FMOD not found")
endif()

message("FMOD_INCLUDE_DIR:" ${FMOD_INCLUDE_DIRS})
message("FMOD_LIBRARIES:" ${FMOD_LIBRARIES})

ark_include_directories(${ARK_SRC_DIR})
ark_include_directories(${ARK_SRC_DIR}/src)
ark_include_directories(${CMAKE_CURRENT_SOURCE_DIR})
ark_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
ark_include_directories(${CMAKE_CURRENT_BINARY_DIR})

ark_include_directories(${FMOD_INCLUDE_DIRS})
ark_link_libraries(${FMOD_LIBRARIES})

aux_source_directory(. LOCAL_SRC_LIST)
aux_source_directory(impl/audio_player LOCAL_SRC_LIST)

ark_setup_tools(ark_gen_plugin_class)
ark_gen_plugin_class(fmod core ark::plugin::fmod fmod_plugin impl)

ark_add_plugin_library(${PROJECT_NAME} ${LOCAL_SRC_LIST} ${LOCAL_GENERATED_SRC_LIST})
#ark_export_dependency_libraries(${PROJECT_NAME} portaudio_static)
