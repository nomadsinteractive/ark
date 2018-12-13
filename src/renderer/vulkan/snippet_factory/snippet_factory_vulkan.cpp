#include "renderer/vulkan/snippet_factory/snippet_factory_vulkan.h"

#include "renderer/opengl/es30/snippet/bind_vertex_array.h"
#include "renderer/opengl/es30/gl_resource/gl_vertex_array.h"

#include "renderer/base/resource_manager.h"

namespace ark {
namespace vulkan {

sp<Snippet> SnippetFactoryVulkan::createCoreSnippet(ResourceManager& glResourceManager, const Shader& shader, const sp<ShaderBindings>& shaderBindings)
{
    return sp<gles30::BindVertexArray>::make(glResourceManager.createGLResource<gles30::GLVertexArray>(shaderBindings, shader));
}

}
}
