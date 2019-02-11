
option(ARK_FORCE_STATIC_VCRT "Force /MT for static VC runtimes" OFF)
if(ARK_FORCE_STATIC_VCRT)
  foreach(flag_var
      CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
      CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
    if(${flag_var} MATCHES "/MD")
      string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endif()
  endforeach()
endif()

ark_ensure_dependency(3rdparty/dirent)
ark_include_directories(3rdparty/dirent/include)

if(ARK_USE_OPEN_GL)
    ark_add_denpendency(3rdparty/glbinding glbinding 3rdparty/glbinding/source/glbinding/include)

    list(APPEND ARK_OPENGL_INCLUDE_DIRS
        ${ARK_SRC_DIR}/3rdparty/glbinding/source/glbinding/include
        ${PROJECT_BINARY_DIR}/3rdparty/glbinding/source/glbinding/include
        )
    set(ARK_OPENGL_LIBRARIES glbinding)
endif()

if(ARK_USE_VULKAN)
    ark_find_vulkan()
endif()

aux_source_directory(platform/windows/impl LOCAL_SRC_LIST)
aux_source_directory(platform/windows/impl/runtime LOCAL_RUNTIME_LIBRARY_SRC_LIST)

set_source_files_properties(${ARK_GENERATED_FILE_DIRECTORY}/framework_plugin.cpp PROPERTIES COMPILE_FLAGS /bigobj)

set(CMAKE_SHARED_LINKER_FLAGS "/NODEFAULTLIB:python37_d.lib /NODEFAULTLIB:python37.lib /NODEFAULTLIB:python3.lib")

if(ARK_USE_VULKAN)
    ark_compile_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
endif()

ark_compile_definitions(-DARK_PLATFORM_WINDOWS)
ark_compile_definitions(-DNOMINMAX)
