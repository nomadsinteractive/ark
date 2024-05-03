
set(LOCAL_MSVC_RUNTIME_CONF_SUFFIX DLL)

option(ARK_FORCE_STATIC_VCRT "Force /MT for static VC runtimes" $<$<CONFIG:Release>:ON,OFF>)

if(ARK_FORCE_STATIC_VCRT)
  foreach(flag_var
      CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
      CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
      CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    if(${${flag_var}} MATCHES "/MD")
      string(REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endif()
  endforeach()

  set(LOCAL_MSVC_RUNTIME_CONF_SUFFIX "")
endif()

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>${LOCAL_MSVC_RUNTIME_CONF_SUFFIX}")

if(ARK_USE_OPEN_GL)
    ark_find_vcpkg_package(glbinding LIBRARIES glbinding::glbinding glbinding::glbinding-aux)
    ark_link_libraries(glbinding::glbinding)
endif()

if(ARK_USE_VULKAN)
    ark_find_vulkan()
endif()

aux_source_directory(platform/windows/impl LOCAL_SRC_LIST)
aux_source_directory(platform/windows/impl/runtime LOCAL_RUNTIME_LIBRARY_SRC_LIST)

set_source_files_properties(${ARK_GENERATED_FILE_DIRECTORY}/framework_plugin.cpp PROPERTIES COMPILE_FLAGS /bigobj)

if(ARK_USE_VULKAN)
    ark_compile_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
endif()

ark_compile_definitions(-DARK_PLATFORM_WINDOWS)
