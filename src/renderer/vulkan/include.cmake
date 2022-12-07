aux_source_directory(src/renderer/vulkan/base LOCAL_SRC_LIST)
aux_source_directory(src/renderer/vulkan/pipeline_factory LOCAL_SRC_LIST)
aux_source_directory(src/renderer/vulkan/render_view LOCAL_SRC_LIST)
aux_source_directory(src/renderer/vulkan/renderer_factory LOCAL_SRC_LIST)
aux_source_directory(src/renderer/vulkan/snippet_factory LOCAL_SRC_LIST)
aux_source_directory(src/renderer/vulkan/util LOCAL_SRC_LIST)

#ark_add_denpendency(${ARK_3RDPARTY_DIR}/glslang glslang ${ARK_3RDPARTY_DIR}/glslang/glslang/Public)
#ark_link_libraries(SPIRV)
#ark_dependency_libraries(SPIRV OGLCompiler OSDependent HLSL)

ark_find_vcpkg_package(glslang LIBRARIES glslang::OSDependent glslang::glslang glslang::MachineIndependent glslang::GenericCodeGen glslang::OGLCompiler glslang::glslang-default-resource-limits glslang::SPVRemapper glslang::SPIRV glslang::HLSL)

ark_gen_plugin_class(vulkan builtin ark::vulkan vulkan_plugin src/renderer/vulkan)
