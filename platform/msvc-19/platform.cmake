
add_definitions(-DHAVE_STRUCT_TIMESPEC)
list(APPEND LOCAL_COMPILE_DEFINITIONS -DARK_USE_CONSTEXPR)

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    add_definitions(-D_X86_)
else()
    add_definitions(-D_AMD64_)
endif()

option(ARK_ENABLE_ADDRESS_SANITIZER "Use AddressSanitizer" OFF)

if(ARK_ENABLE_ADDRESS_SANITIZER)
    ark_compile_options(-fsanitize=address)
    ark_link_libraries(clang_rt.asan-i386)
endif()
