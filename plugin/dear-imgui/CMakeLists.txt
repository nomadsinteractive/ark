project(ark-dear-imgui)
cmake_minimum_required(VERSION 3.15)

include(${ARK_SRC_DIR}/tools/cmake/core.cmake)

ark_include_directories(${ARK_INCLUDE_DIRS})
ark_include_directories(${CMAKE_CURRENT_SOURCE_DIR})
ark_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
ark_include_directories(${CMAKE_CURRENT_BINARY_DIR})

aux_source_directory(. LOCAL_SRC_LIST)

aux_source_directory(base LOCAL_SRC_LIST)
aux_source_directory(renderer LOCAL_SRC_LIST)
aux_source_directory(widget LOCAL_SRC_LIST)

ark_find_vcpkg_package(imgui LIBRARIES imgui::imgui)

ark_setup_tools(ark_gen_plugin_class)
ark_gen_plugin_class(dear_imgui core ark::plugin::dear_imgui base renderer)

ark_setup_tools(ark_gen_string_table)
ark_gen_string_table(shaders ark::plugin::imgui shaders)

ark_add_plugin_library(${PROJECT_NAME} ${LOCAL_SRC_LIST} ${LOCAL_GENERATED_SRC_LIST})
target_compile_definitions(${PROJECT_NAME} PRIVATE -DARK_FLAG_BUILDING_PLUGIN_DEAR_IMGUI)

ark_setup_tools(ark_gen_pybindings)
ark_gen_pybindings(dear-imgui ark::plugin::dear_imgui ark.dear_imgui base pybindings/pybindings_main.cpp ${PROJECT_NAME})

ark_export_dependency_libraries(${PROJECT_NAME})
