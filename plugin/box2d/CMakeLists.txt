project(ark-box2d)
cmake_minimum_required(VERSION 2.8)

include(${ARK_SRC_DIR}/tools/cmake/core.cmake)

set(BOX2D_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/Box2D)

set(BOX2D_BUILD_EXAMPLES OFF CACHE BOOL "")
add_subdirectory(3rdparty/Box2D/Box2D)

list(APPEND LOCAL_LIBS ${ARK_LIBRARIES})
list(APPEND LOCAL_LIBS Box2D)

list(APPEND LOCAL_INCLUDE_DIRS ${ARK_SRC_DIR})
list(APPEND LOCAL_INCLUDE_DIRS ${ARK_SRC_DIR}/src)
list(APPEND LOCAL_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})
list(APPEND LOCAL_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/..)
list(APPEND LOCAL_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/Box2D/Box2D)
list(APPEND LOCAL_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR})

aux_source_directory(. LOCAL_SRC_LIST)
aux_source_directory(impl LOCAL_SRC_LIST)
aux_source_directory(impl/shapes LOCAL_SRC_LIST)

ark_setup_tools(ark_gen_plugin_class)
ark_gen_plugin_class(box2d core ark::plugin::box2d box2d_plugin impl)

ark_add_shared_library(${PROJECT_NAME} ${LOCAL_SRC_LIST} ${LOCAL_GENERATED_SRC_LIST})
target_compile_definitions(${PROJECT_NAME} PRIVATE -DARK_FLAG_BUILDING_PLUGIN_BOX2D)

ark_setup_tools(ark_gen_pybindings)
ark_gen_pybindings(box2d ark::plugin::box2d ark.box2d impl pybindings/pybindings_main.cpp ${PROJECT_NAME})
