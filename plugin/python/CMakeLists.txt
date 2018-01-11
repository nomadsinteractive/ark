project(ark-python)
cmake_minimum_required(VERSION 3.0)

set(SRC_DIR ${ARK_SRC_DIR}/plugin/python/3rdparty/Python)

list(APPEND DISABLED_MODULES "ARRAY" "AUDIOOP" "TESTCAPI" "CTYPES" "CTYPES_TEST" "PWD" "MULTIPROCESSING" "OSSAUDIODEV" "SCPROXY" "SQLITE3" "GRP" "TERMIOS" "MMAP" "MSVCRT" "OSSAUDIODEV")
foreach(i IN LISTS DISABLED_MODULES)
    set(ENABLE_${i} OFF)
endforeach()

set(BUILD_TESTING OFF)
set(BUILD_EXTENSIONS_AS_BUILTIN ON)
set(WITH_THREAD OFF)
set(BUILD_WININST OFF)

set(BUILD_STATIC ON)

if(ARK_BUILD_SHARED_PLUGINS)
    set(BUILD_SHARED ON)
else()
    set(BUILD_SHARED OFF)
endif()

if(ARK_BUILD_SHARED_PLUGINS)
    list(APPEND ARK_PYTHON_LIBRARIES libpython-shared)
else()
    list(APPEND ARK_PYTHON_LIBRARIES libpython-static)
endif()

include(${ARK_SRC_DIR}/tools/cmake/core.cmake)

ark_ensure_dependency(3rdparty/Python)
add_subdirectory(3rdparty/python-cmake-buildsystem)

list(APPEND LOCAL_LIBS ${ARK_LIBRARIES})
list(APPEND LOCAL_LIBS ${ARK_PYTHON_LIBRARIES})

get_filename_component(PLUGIN_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR} PATH)

list(APPEND LOCAL_INCLUDE_DIRS .)
list(APPEND LOCAL_INCLUDE_DIRS ${PLUGIN_ROOT_DIR})
list(APPEND LOCAL_INCLUDE_DIRS ${ARK_SRC_DIR})
list(APPEND LOCAL_INCLUDE_DIRS ${ARK_SRC_DIR}/src)
list(APPEND LOCAL_INCLUDE_DIRS ${ARK_GLBINDINGS_INCLUDE_DIRS})
list(APPEND ARK_PYTHON_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR})
list(APPEND ARK_PYTHON_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/Python/Include)

aux_source_directory(. LOCAL_SRC_LIST)
aux_source_directory(extension LOCAL_SRC_LIST)
aux_source_directory(impl/adapter LOCAL_SRC_LIST)
aux_source_directory(impl/duck LOCAL_SRC_LIST)
aux_source_directory(impl/script LOCAL_SRC_LIST)

ark_add_python_generated_unit(py_ark_bindings ${ARK_SRC_DIR} "src/app src/core src/graphics src/renderer plugin/python/extension" plugin/python/tools/python/gen_py_bindings.py -p ark::plugin::python -m ark -o ${ARK_GENERATED_FILE_DIRECTORY}/py_ark_bindings -l "${ARK_SRC_DIR}/tools/python" src/app src/core src/graphics src/renderer platform plugin/python/extension)
if(MSVC)
    set_source_files_properties(${ARK_GENERATED_FILE_DIRECTORY}/py_ark_bindings.cpp PROPERTIES COMPILE_FLAGS /bigobj)
endif()
if(NOT ${SYSTEM} STREQUAL "android")
    ark_add_generated_file(_freeze_importlib ark_py_importlib.h ${CMAKE_CURRENT_SOURCE_DIR}/extension extension/_ark_bootstrap.py _ark_bootstrap.py)
endif()

function(gen_py_method_def METHOD_DEF_FILE_NAME NAMESPACES)
set(LOCAL_GENERATED_FILE_H ${ARK_GENERATED_FILE_DIRECTORY}/${METHOD_DEF_FILE_NAME}.h)
set(LOCAL_GENERATED_FILE_CPP ${ARK_GENERATED_FILE_DIRECTORY}/${METHOD_DEF_FILE_NAME}.cpp)
foreach(i IN LISTS ARGN)
file(GLOB LOCAL_FILES "${ARK_SRC_DIR}/${i}/*.h")
list(APPEND FILE_DEPENDS ${LOCAL_FILES})
endforeach()
add_custom_command(OUTPUT ${LOCAL_GENERATED_FILE_H} ${LOCAL_GENERATED_FILE_CPP}
    COMMAND python ${ARK_SRC_DIR}/plugin/python/tools/python/gen_py_method_def.py -o ${ARK_GENERATED_FILE_DIRECTORY} -n ${METHOD_DEF_FILE_NAME} -p ${NAMESPACES} ${ARGN}
    DEPENDS ${ARK_SRC_DIR}/plugin/python/tools/python/gen_py_method_def.py ${FILE_DEPENDS}
    WORKING_DIRECTORY ${ARK_SRC_DIR})
list(APPEND LOCAL_GENERATED_SRC_LIST ${LOCAL_GENERATED_FILE_H} ${LOCAL_GENERATED_FILE_CPP})
set(LOCAL_GENERATED_SRC_LIST ${LOCAL_GENERATED_SRC_LIST} PARENT_SCOPE)
endfunction(gen_py_method_def)

gen_py_method_def(py_ark_method_def ark::plugin::python plugin/python/extension)

ark_setup_tools(ark_gen_plugin_class)
ark_gen_plugin_class(python core ark::plugin::python python_plugin impl)

list(APPEND LOCAL_INCLUDE_DIRS ${ARK_PYTHON_INCLUDE_DIR})
ark_add_shared_library(${PROJECT_NAME} ${LOCAL_SRC_LIST} ${LOCAL_GENERATED_SRC_LIST})
target_compile_definitions(${PROJECT_NAME} PRIVATE -DARK_FLAG_BUILDING_PLUGIN_PYTHON)

ark_unset_local_variables()

list(APPEND LOCAL_LIBS libpython-static)
list(APPEND LOCAL_INCLUDE_DIRS ${ARK_PYTHON_INCLUDE_DIR})

ark_add_executable(_freeze_importlib ${SRC_DIR}/Programs/_freeze_importlib.c)

set(ARK_PYTHON_INCLUDE_DIR ${ARK_PYTHON_INCLUDE_DIR} PARENT_SCOPE)
set(ARK_PYTHON_LIBRARIES ${ARK_PYTHON_LIBRARIES} PARENT_SCOPE)
set(ARK_PYTHON_PATCH_LIBS ${ARK_PYTHON_PATCH_LIBS} PARENT_SCOPE)
set(ARK_TOOLS_DIR ${ARK_TOOLS_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/tools/cmake PARENT_SCOPE)
