project(ark-bgfx)
cmake_minimum_required(VERSION 3.14)

include(${ARK_SRC_DIR}/tools/cmake/core.cmake)

ark_include_directories(${ARK_SRC_DIR})
ark_include_directories(${ARK_SRC_DIR}/src)
ark_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
ark_include_directories(${CMAKE_CURRENT_BINARY_DIR})

aux_source_directory(. LOCAL_SRC_LIST)
aux_source_directory(base LOCAL_SRC_LIST)
aux_source_directory(impl/pipeline_factory LOCAL_SRC_LIST)
aux_source_directory(impl/renderer_factory LOCAL_SRC_LIST)
aux_source_directory(impl/texture LOCAL_SRC_LIST)

ark_find_vcpkg_package(bgfx LIBRARIES bgfx::bx bgfx::bgfx bgfx::bimg bgfx::bimg_decode)

ark_setup_tools(ark_gen_plugin_class)
ark_gen_plugin_class(bgfx core ark::plugin::bgfx -i bgfx impl/layout)

ark_add_plugin_library(${PROJECT_NAME} ${LOCAL_SRC_LIST} ${LOCAL_GENERATED_SRC_LIST})
ark_export_dependency_libraries(${PROJECT_NAME})
