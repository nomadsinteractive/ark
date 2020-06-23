project(ark-bullet)
cmake_minimum_required(VERSION 3.0)

include(${ARK_SRC_DIR}/tools/cmake/core.cmake)

ark_ensure_dependency(3rdparty/bullet3)

add_subdirectory(3rdparty/bullet3)

ark_include_directories(${ARK_INCLUDE_DIRS})
ark_include_directories(${CMAKE_CURRENT_SOURCE_DIR})
ark_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
ark_include_directories(${CMAKE_CURRENT_BINARY_DIR})

ark_include_directories(3rdparty/bullet3/src)

aux_source_directory(. LOCAL_SRC_LIST)
aux_source_directory(base LOCAL_SRC_LIST)
aux_source_directory(importer LOCAL_SRC_LIST)
aux_source_directory(rigid_body LOCAL_SRC_LIST)

ark_setup_tools(ark_gen_plugin_class)
ark_gen_plugin_class(bullet core ark::plugin::bullet bullet_plugin base rigid_body importer)

ark_link_libraries(BulletDynamics BulletCollision Bullet3Common LinearMath)

ark_add_plugin_library(${PROJECT_NAME} ${LOCAL_SRC_LIST} ${LOCAL_GENERATED_SRC_LIST})
target_compile_definitions(${PROJECT_NAME} PRIVATE -DARK_FLAG_BUILDING_PLUGIN_BULLET)

ark_setup_tools(ark_gen_pybindings)
ark_gen_pybindings(bullet ark::plugin::bullet ark.bullet rigid_body pybindings/pybindings_main.cpp ${PROJECT_NAME})

#ark_export_dependency_libraries(${PROJECT_NAME} BulletDynamics BulletCollision Bullet3Common LinearMath)
