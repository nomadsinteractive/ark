project(ark-vulkan)
cmake_minimum_required(VERSION 3.14)

include(${ARK_SRC_DIR}/tools/cmake/core.cmake)

ark_include_directories(${ARK_SRC_DIR})
ark_include_directories(${ARK_SRC_DIR}/src)
ark_include_directories(${ARK_SRC_DIR}/platform/any)
ark_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
ark_include_directories(${CMAKE_CURRENT_BINARY_DIR})

aux_source_directory(. LOCAL_SRC_LIST)
aux_source_directory(base LOCAL_SRC_LIST)
aux_source_directory(impl/pipeline_factory LOCAL_SRC_LIST)
aux_source_directory(impl/snippet_factory LOCAL_SRC_LIST)
aux_source_directory(impl/render_view LOCAL_SRC_LIST)
aux_source_directory(impl/renderer_factory LOCAL_SRC_LIST)
aux_source_directory(util LOCAL_SRC_LIST)

ark_find_vulkan()
ark_find_vcpkg_package(glm LIBRARIES glm::glm)

ark_setup_tools(ark_gen_plugin_class)
ark_gen_plugin_class(vulkan core ark::plugin::vulkan -i vulkan impl)

ark_add_plugin_library(${PROJECT_NAME} ${LOCAL_SRC_LIST} ${LOCAL_GENERATED_SRC_LIST})
ark_export_dependency_libraries(${PROJECT_NAME})
