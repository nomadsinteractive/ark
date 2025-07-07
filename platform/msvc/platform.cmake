
set(LOCAL_MSVC_RUNTIME_CONF_SUFFIX DLL)

option(ARK_FORCE_STATIC_VCRT "Force /MT for static VC runtimes" ${ARK_USE_STATIC_TRIPLET})

if(ARK_FORCE_STATIC_VCRT)
  set(LOCAL_MSVC_RUNTIME_CONF_SUFFIX "")
endif()

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>${LOCAL_MSVC_RUNTIME_CONF_SUFFIX}")

add_definitions(-DHAVE_STRUCT_TIMESPEC)

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

ark_compile_options(/bigobj)