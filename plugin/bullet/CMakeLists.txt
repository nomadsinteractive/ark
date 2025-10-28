project(ark-bullet)
cmake_minimum_required(VERSION 3.15)

include(${ARK_SRC_DIR}/tools/cmake/core.cmake)

ark_find_vcpkg_package(Bullet LIBRARIES LinearMath Bullet3Common BulletDynamics BulletSoftBody)

ark_include_directories(${ARK_INCLUDE_DIRS})
ark_include_directories(${CMAKE_CURRENT_SOURCE_DIR})
ark_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
ark_include_directories(${CMAKE_CURRENT_BINARY_DIR})

aux_source_directory(. LOCAL_SRC_LIST)
aux_source_directory(base LOCAL_SRC_LIST)
aux_source_directory(importer LOCAL_SRC_LIST)

ark_setup_tools(ark_gen_plugin_class)
ark_gen_plugin_class(bullet core ark::plugin::bullet base rigid_body importer)

ark_link_libraries(BulletDynamics BulletCollision Bullet3Common LinearMath)

ark_add_plugin_library(${PROJECT_NAME} ${LOCAL_SRC_LIST} ${LOCAL_GENERATED_SRC_LIST})
target_compile_definitions(${PROJECT_NAME} PRIVATE -DARK_FLAG_BUILDING_PLUGIN_BULLET)
