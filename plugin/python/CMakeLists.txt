project(ark-python LANGUAGES C CXX ASM_MASM)
cmake_minimum_required(VERSION 3.15)

set(SRC_DIR ${ARK_SRC_DIR}/plugin/python/3rdparty/Python)

list(APPEND DISABLED_MODULES ARRAY AUDIOOP BZ2 TESTCAPI CTYPES_TEST PWD MULTIPROCESSING OSSAUDIODEV SCPROXY SQLITE3 GRP TERMIOS MMAP OSSAUDIODEV)
if(IOS OR ANDROID)
    list(APPEND DISABLED_MODULES CTYPES CURSES DECIMAL)
endif()

foreach(i IN LISTS DISABLED_MODULES)
    set(ENABLE_${i} OFF)
endforeach()

include(${ARK_SRC_DIR}/tools/cmake/core.cmake)

#if(ARK_BUILD_STATIC_PLUGINS)
#    set(BUILD_SHARED OFF)
#    ark_compile_definitions(-DPy_NO_ENABLE_SHARED)
#    list(APPEND ARK_PYTHON_LIBRARIES libpython-static)
#else()
#    set(BUILD_SHARED ON)
#    list(APPEND ARK_PYTHON_LIBRARIES libpython-shared)
#endif()

get_filename_component(PLUGIN_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR} PATH)

set(ARK_PYTHON_SRC_DIR 3rdparty/Python)

ark_find_vcpkg_package(ZLIB LIBRARIES ZLIB::ZLIB MODULE_MODE)
ark_find_vcpkg_package(Python3 LIBRARIES Python3::Python COMPONENTS Development MODULE_MODE EXPORT_VARS Python3_LIBRARIES Python3_INCLUDE_DIRS)
set(ARK_PYTHON_LIBRARIES Python3::Python)
list(APPEND ARK_PYTHON_INCLUDE_DIR ${Python3_INCLUDE_DIRS})
list(APPEND ARK_INCLUDE_DIRS ${ARK_PYTHON_INCLUDE_DIR})

list(APPEND LOCAL_INCLUDE_DIRS .)
list(APPEND LOCAL_INCLUDE_DIRS ${PLUGIN_ROOT_DIR})
list(APPEND LOCAL_INCLUDE_DIRS ${ARK_SRC_DIR})
list(APPEND LOCAL_INCLUDE_DIRS ${ARK_SRC_DIR}/src)
list(APPEND LOCAL_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR})

aux_source_directory(. LOCAL_SRC_LIST)
aux_source_directory(extension LOCAL_SRC_LIST)
aux_source_directory(impl/adapter LOCAL_SRC_LIST)
aux_source_directory(impl/duck LOCAL_SRC_LIST)
aux_source_directory(impl/script LOCAL_SRC_LIST)

execute_process(
    COMMAND ${Python_EXECUTABLE} plugin/python/tools/python/gen_py_bindings.py -d ${ARK_GENERATED_FILE_DIRECTORY} -c ${ARK_GENERATED_FILE_DIRECTORY}/py_ark_bindings.cmake -p ark::plugin::python -m ark -o ${ARK_GENERATED_FILE_DIRECTORY}/py_ark_bindings -l "${ARK_SRC_DIR}/tools/python" src/app src/core src/graphics src/renderer platform plugin/python/extension
    WORKING_DIRECTORY ${ARK_SRC_DIR}
)
include(${ARK_GENERATED_FILE_DIRECTORY}/py_ark_bindings.cmake)

ark_find_vcpkg_package(expat LIBRARIES expat::expat)
include(extensions.cmake)
if(WIN32)
    ark_link_libraries(ws2_32 iphlpapi)
endif()

if(NOT (ANDROID OR IOS))
    if(ARK_USE_STATIC_TRIPLET)
        ark_compile_definitions(-DPy_NO_ENABLE_SHARED)
    endif()
    ark_add_executable(_freeze_module ${SRC_DIR}/Programs/_freeze_module.c)
    set_property(TARGET _freeze_module PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    target_include_directories(_freeze_module PRIVATE ${ARK_PYTHON_SRC_DIR}/Include/internal)
    if(NOT ARK_USE_STATIC_TRIPLET)
        target_compile_definitions(_freeze_module PRIVATE Py_ENABLE_SHARED)
    endif()
    target_compile_definitions(_freeze_module PRIVATE Py_BUILD_CORE Py_BUILD_CORE_MODULE)
    target_link_libraries(_freeze_module PRIVATE ${ARK_PYTHON_LIBRARIES})
    ark_add_generated_file(_freeze_module ${CMAKE_CURRENT_SOURCE_DIR}/extension/ark_py_importlib.h ${CMAKE_CURRENT_SOURCE_DIR}/extension extension/_ark_bootstrap.py ark_importlib _ark_bootstrap.py)
endif()

ark_setup_tools(ark_gen_plugin_class)
ark_gen_plugin_class(python core ark::plugin::python python_plugin impl)

ark_include_directories(${ARK_PYTHON_INCLUDE_DIR})
ark_add_plugin_library(${PROJECT_NAME} ${LOCAL_SRC_LIST} ${LOCAL_GENERATED_SRC_LIST} extension_ctypes extension_decimal extension_socket extension_select extension_pyexpat extension_overlapped)
target_compile_definitions(${PROJECT_NAME} PRIVATE -DARK_FLAG_BUILDING_PLUGIN_PYTHON)

ark_export_dependency_libraries(${PROJECT_NAME} ${ARK_PYTHON_LIBRARIES} ${ARK_PYTHON_PATCH_LIBS})

set(ARK_PYTHON_INCLUDE_DIR ${ARK_PYTHON_INCLUDE_DIR} PARENT_SCOPE)
set(ARK_PYTHON_PATCH_LIBS ${ARK_PYTHON_PATCH_LIBS} PARENT_SCOPE)
set(ARK_TOOLS_DIR ${ARK_TOOLS_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/tools/cmake PARENT_SCOPE)
set(ARK_INCLUDE_DIRS ${ARK_INCLUDE_DIRS} PARENT_SCOPE)
