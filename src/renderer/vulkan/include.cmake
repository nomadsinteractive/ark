aux_source_directory(src/renderer/vulkan/base LOCAL_SRC_LIST)
aux_source_directory(src/renderer/vulkan/pipeline_factory LOCAL_SRC_LIST)
aux_source_directory(src/renderer/vulkan/render_view LOCAL_SRC_LIST)
aux_source_directory(src/renderer/vulkan/renderer_factory LOCAL_SRC_LIST)
aux_source_directory(src/renderer/vulkan/snippet_factory LOCAL_SRC_LIST)
aux_source_directory(src/renderer/vulkan/util LOCAL_SRC_LIST)

ark_find_vcpkg_package(glslang LIBRARIES glslang::glslang glslang::glslang-default-resource-limits glslang::SPIRV glslang::SPVRemapper)

ark_gen_plugin_class(vulkan builtin ark::vulkan src/renderer/vulkan)
