project(ark-python)
cmake_minimum_required(VERSION 3.0)

set(SRC_DIR ${ARK_SRC_DIR}/plugin/python/3rdparty/Python)

list(APPEND DISABLED_MODULES "ARRAY" "AUDIOOP" "TESTCAPI" "CTYPES" "CTYPES_TEST" "PWD" "MULTIPROCESSING" "OSSAUDIODEV" "SCPROXY" "SQLITE3" "GRP" "TERMIOS" "MMAP" "OSSAUDIODEV")
if(IOS OR ANDROID)
    list(APPEND DISABLED_MODULES DECIMAL)
endif()

foreach(i IN LISTS DISABLED_MODULES)
    set(ENABLE_${i} OFF)
endforeach()

set(BUILD_TESTING OFF)
set(BUILD_EXTENSIONS_AS_BUILTIN ON)
#set(WITH_THREAD ON)
set(BUILD_WININST OFF)

set(BUILD_STATIC ON)

include(${ARK_SRC_DIR}/tools/cmake/core.cmake)

if(ARK_BUILD_STATIC_PLUGINS)
    set(BUILD_SHARED OFF)
    ark_compile_definitions(-DPy_NO_ENABLE_SHARED)
    list(APPEND ARK_PYTHON_LIBRARIES libpython-static)
else()
    set(BUILD_SHARED ON)
    list(APPEND ARK_PYTHON_LIBRARIES libpython-shared)
endif()

ark_ensure_dependency(3rdparty/Python)
add_subdirectory(3rdparty/python-cmake-buildsystem)

ark_link_libraries(${ARK_PYTHON_LIBRARIES})

get_filename_component(PLUGIN_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR} PATH)

list(APPEND LOCAL_INCLUDE_DIRS .)
list(APPEND LOCAL_INCLUDE_DIRS ${PLUGIN_ROOT_DIR})
list(APPEND LOCAL_INCLUDE_DIRS ${ARK_SRC_DIR})
list(APPEND LOCAL_INCLUDE_DIRS ${ARK_SRC_DIR}/src)
list(APPEND ARK_PYTHON_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR})
list(APPEND ARK_PYTHON_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/Python/Include)

aux_source_directory(. LOCAL_SRC_LIST)
aux_source_directory(extension LOCAL_SRC_LIST)
aux_source_directory(impl/adapter LOCAL_SRC_LIST)
aux_source_directory(impl/duck LOCAL_SRC_LIST)
aux_source_directory(impl/script LOCAL_SRC_LIST)

execute_process(
    COMMAND python plugin/python/tools/python/gen_py_bindings.py -d ${ARK_GENERATED_FILE_DIRECTORY} -c ${ARK_GENERATED_FILE_DIRECTORY}/py_ark_bindings.cmake -p ark::plugin::python -m ark -o ${ARK_GENERATED_FILE_DIRECTORY}/py_ark_bindings -l "${ARK_SRC_DIR}/tools/python" src/app src/core src/graphics src/renderer platform plugin/python/extension
    WORKING_DIRECTORY ${ARK_SRC_DIR}
)
include(${ARK_GENERATED_FILE_DIRECTORY}/py_ark_bindings.cmake)
if(NOT (ANDROID OR IOS))
    ark_add_generated_file(_freeze_importlib ${CMAKE_CURRENT_SOURCE_DIR}/extension/ark_py_importlib.h ${CMAKE_CURRENT_SOURCE_DIR}/extension extension/_ark_bootstrap.py _ark_bootstrap.py)
endif()

ark_setup_tools(ark_gen_plugin_class)
ark_gen_plugin_class(python core ark::plugin::python python_plugin impl)

ark_include_directories(${ARK_PYTHON_INCLUDE_DIR})
ark_add_plugin_library(${PROJECT_NAME} ${LOCAL_SRC_LIST} ${LOCAL_GENERATED_SRC_LIST})
target_compile_definitions(${PROJECT_NAME} PRIVATE -DARK_FLAG_BUILDING_PLUGIN_PYTHON)

ark_export_dependency_libraries(${PROJECT_NAME} ${ARK_PYTHON_LIBRARIES} ${ARK_PYTHON_PATCH_LIBS})

ark_unset_local_variables()

list(APPEND LOCAL_LIBS libpython-static)
list(APPEND LOCAL_INCLUDE_DIRS ${ARK_PYTHON_INCLUDE_DIR})

ark_add_executable(_freeze_importlib ${SRC_DIR}/Programs/_freeze_importlib.c)

set(ARK_PYTHON_INCLUDE_DIR ${ARK_PYTHON_INCLUDE_DIR} PARENT_SCOPE)
set(ARK_PYTHON_LIBRARIES ${ARK_PYTHON_LIBRARIES} PARENT_SCOPE)
set(ARK_PYTHON_PATCH_LIBS ${ARK_PYTHON_PATCH_LIBS} PARENT_SCOPE)
set(ARK_TOOLS_DIR ${ARK_TOOLS_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/tools/cmake PARENT_SCOPE)
