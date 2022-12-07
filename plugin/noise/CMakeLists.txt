project(ark-noise)
cmake_minimum_required(VERSION 3.0)

#git clone https://github.com/eXpl0it3r/libnoise.git
#git clone https://github.com/Reputeless/PerlinNoise.git
add_subdirectory(3rdparty/libnoise)

include(${ARK_SRC_DIR}/tools/cmake/core.cmake)

ark_include_directories(${ARK_INCLUDE_DIRS})

list(APPEND LOCAL_INCLUDE_DIRS .)
list(APPEND LOCAL_INCLUDE_DIRS ${ARK_INCLUDE_DIRS})

list(APPEND LOCAL_INCLUDE_DIRS ${ARK_PLUGIN_DIR})

ark_ensure_dependency(3rdparty/FastNoise)
list(APPEND LOCAL_INCLUDE_DIRS 3rdparty/FastNoise)
aux_source_directory(3rdparty/FastNoise LOCAL_SRC_LIST)

if(MSVC)
    ark_compile_options("/arch:AVX2")
endif()

#add_subdirectory(tools)

list(APPEND LOCAL_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR})

aux_source_directory(. LOCAL_SRC_LIST)
aux_source_directory(base LOCAL_SRC_LIST)
aux_source_directory(impl/resource LOCAL_SRC_LIST)

ark_setup_tools(gen_plugin_class)
ark_gen_plugin_class(noise core ark::plugin::noise noise_plugin impl)

ark_add_plugin_library(${PROJECT_NAME} ${LOCAL_SRC_LIST} ${LOCAL_GENERATED_SRC_LIST})
target_compile_definitions(${PROJECT_NAME} PRIVATE -DARK_FLAG_BUILDING_PLUGIN_NOISE)

ark_setup_tools(ark_gen_pybindings)
ark_gen_pybindings(noise ark::plugin::noise ark.noise base pybindings/pybindings_main.cpp ${PROJECT_NAME})

ark_export_dependency_libraries(${PROJECT_NAME})
