
list(APPEND ARK_PYTHON_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/3rdparty/python-cmake-buildsystem/bin)

add_definitions(-DSOABI="arm")
if(IOS)
    add_definitions(-DXML_DEV_URANDOM)
endif()
