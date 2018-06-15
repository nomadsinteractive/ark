
add_definitions(-DHAVE_STRUCT_TIMESPEC)
list(APPEND LOCAL_COMPILE_DEFINITIONS -DARK_USE_CONSTEXPR)

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    add_definitions(-D_X86_)
else()
    add_definitions(-D_AMD64_)
endif()
