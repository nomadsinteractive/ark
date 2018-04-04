
ark_add_denpendency(3rdparty/glbinding ON glbinding 3rdparty/glbinding/source/glbinding/include)
ark_ensure_dependency(3rdparty/dirent)
ark_include_directories(3rdparty/dirent/include)

LIST(APPEND ARK_OPENGL_INCLUDE_DIRS
    ${ARK_SRC_DIR}/3rdparty/glbinding/source/glbinding/include
    ${PROJECT_BINARY_DIR}/3rdparty/glbinding/source/glbinding/include
    )

set(ARK_OPENGL_LIBRARIES glbinding)

aux_source_directory(platform/windows/impl LOCAL_SRC_LIST)
aux_source_directory(platform/windows/impl/runtime LOCAL_RUNTIME_LIBRARY_SRC_LIST)

set_source_files_properties(${ARK_GENERATED_FILE_DIRECTORY}/framework_plugin.cpp PROPERTIES COMPILE_FLAGS /bigobj)

set(CMAKE_SHARED_LINKER_FLAGS "/NODEFAULTLIB:python36_d.lib /NODEFAULTLIB:python36.lib /NODEFAULTLIB:python3.lib")
