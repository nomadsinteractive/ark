
aux_source_directory(platform/android/internal LOCAL_PYTHON_PATCH_SRC_LIST)

add_definitions(-DSOABI="arm")

list(APPEND LOCAL_COMPILE_DEFINITIONS -DPY_FORMAT_LONG_LONG="long long")
list(APPEND LOCAL_INCLUDE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/3rdparty/python-cmake-buildsystem/bin")

add_library(python-ndk-patch ${LOCAL_PYTHON_PATCH_SRC_LIST})
target_include_directories(python-ndk-patch PRIVATE ${PROJECT_SOURCE_DIR}/platform/android)

list(APPEND ARK_PYTHON_PATCH_LIBS python-ndk-patch)
list(APPEND LOCAL_PYTHON_PATCH_COMPILE_OPTIONS -include ${PROJECT_SOURCE_DIR}/platform/android/internal/_local.h)

list(APPEND ARK_PYTHON_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/3rdparty/python-cmake-buildsystem/bin)
