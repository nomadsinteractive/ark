project(ark-optick)
cmake_minimum_required(VERSION 3.0)

include(${ARK_SRC_DIR}/tools/cmake/core.cmake)

ark_include_directories(${ARK_SRC_DIR})
ark_include_directories(${ARK_SRC_DIR}/src)
ark_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
ark_include_directories(${CMAKE_CURRENT_BINARY_DIR})

aux_source_directory(. LOCAL_SRC_LIST)
aux_source_directory(impl/application_profiler LOCAL_SRC_LIST)

if(ARK_USE_VULKAN)
    set(OPTICK_USE_VULKAN ON)
endif()

ark_add_denpendency(3rdparty/optick OptickCore 3rdparty/optick/src)

ark_setup_tools(ark_gen_plugin_class)
ark_gen_plugin_class(optick core ark::plugin::optick optick_plugin -i optick impl/application_profiler)

ark_add_plugin_library(${PROJECT_NAME} ${LOCAL_SRC_LIST} ${LOCAL_GENERATED_SRC_LIST})
ark_export_dependency_libraries(${PROJECT_NAME})
