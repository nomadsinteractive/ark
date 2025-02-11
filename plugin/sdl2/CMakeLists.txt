project(ark-sdl2)
cmake_minimum_required(VERSION 3.14)

include(${ARK_SRC_DIR}/tools/cmake/core.cmake)

ark_include_directories(${ARK_SRC_DIR})
ark_include_directories(${ARK_SRC_DIR}/src)
ark_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
ark_include_directories(${CMAKE_CURRENT_BINARY_DIR})

aux_source_directory(. LOCAL_SRC_LIST)
aux_source_directory(impl/application LOCAL_SRC_LIST)

ark_find_vcpkg_package(SDL2 LIBRARIES $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>)

ark_setup_tools(ark_gen_plugin_class)
ark_gen_plugin_class(sdl2 core ark::plugin::sdl2 -i sdl2 impl)

ark_add_plugin_library(${PROJECT_NAME} ${LOCAL_SRC_LIST} ${LOCAL_GENERATED_SRC_LIST})
ark_export_dependency_libraries(${PROJECT_NAME})
