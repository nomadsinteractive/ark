find_package(Vulkan)

if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "No Vulkan libraries found")
else()
    message("Vulkan include dirs: " ${Vulkan_INCLUDE_DIRS})
    message("Vulkan libraries: " ${Vulkan_LIBRARIES})
endif()

ark_include_directories(${Vulkan_INCLUDE_DIRS})
ark_link_libraries(${Vulkan_LIBRARIES})

aux_source_directory(src/renderer/vulkan/base LOCAL_SRC_LIST)
aux_source_directory(src/renderer/vulkan/gl_snippet_factory LOCAL_SRC_LIST)
aux_source_directory(src/renderer/vulkan/pipeline_factory LOCAL_SRC_LIST)
aux_source_directory(src/renderer/vulkan/render_view LOCAL_SRC_LIST)
aux_source_directory(src/renderer/vulkan/renderer_factory LOCAL_SRC_LIST)
aux_source_directory(src/renderer/vulkan/util LOCAL_SRC_LIST)

ark_gen_plugin_class(vulkan builtin ark::vulkan vulkan_plugin src/renderer/vulkan)
