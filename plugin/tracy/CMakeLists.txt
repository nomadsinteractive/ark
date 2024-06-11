project(ark-tracy)
cmake_minimum_required(VERSION 3.15)

include(${ARK_SRC_DIR}/tools/cmake/core.cmake)

ark_include_directories(${ARK_SRC_DIR}/src)
ark_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
ark_include_directories(${CMAKE_CURRENT_BINARY_DIR})

aux_source_directory(. LOCAL_SRC_LIST)
aux_source_directory(impl/application_profiler LOCAL_SRC_LIST)

ark_setup_tools(ark_gen_plugin_class)
ark_gen_plugin_class(tracy core ark::plugin::tracy -i tracy impl/application_profiler)

ark_find_vcpkg_package(tracy LIBRARIES Tracy::TracyClient)

ark_compile_definition_option(TRACY_ON_DEMAND "Do not connect to server on startup" OFF)
ark_compile_definition_option(TRACY_NO_BROADCAST "Do not broadcast my existence" OFF)

ark_add_plugin_library(${PROJECT_NAME} ${LOCAL_SRC_LIST} ${LOCAL_GENERATED_SRC_LIST})
ark_export_dependency_libraries(${PROJECT_NAME})
