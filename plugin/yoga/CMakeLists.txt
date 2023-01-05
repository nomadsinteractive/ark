project(ark-yoga)
cmake_minimum_required(VERSION 3.7)

include(${ARK_SRC_DIR}/tools/cmake/core.cmake)

ark_include_directories(${ARK_SRC_DIR})
ark_include_directories(${ARK_SRC_DIR}/src)
ark_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
ark_include_directories(${CMAKE_CURRENT_BINARY_DIR})

aux_source_directory(. LOCAL_SRC_LIST)
aux_source_directory(impl/layout LOCAL_SRC_LIST)

#a quick workround for vcpkg Export-unofficial-yoga-config.patch issue
add_library(android STATIC android_dummy.cpp)
add_library(log STATIC android_dummy.cpp)

ark_find_vcpkg_package(unofficial-yoga LIBRARIES unofficial::yoga::yogacore)

ark_setup_tools(ark_gen_plugin_class)
ark_gen_plugin_class(yoga core ark::plugin::yoga yoga_plugin -i yoga impl/layout)

ark_add_plugin_library(${PROJECT_NAME} ${LOCAL_SRC_LIST} ${LOCAL_GENERATED_SRC_LIST})
ark_export_dependency_libraries(${PROJECT_NAME})
