#include "renderer/vulkan/gl_snippet_factory/gl_snippet_factory_vulkan.h"

#include "renderer/opengl/es30/gl_snippet/bind_vertex_array.h"
#include "renderer/opengl/es30/gl_resource/gl_vertex_array.h"

#include "renderer/base/gl_resource_manager.h"

namespace ark {
namespace vulkan {

sp<GLSnippet> GLSnippetFactoryVulkan::createCoreGLSnippet(GLResourceManager& glResourceManager, const GLShader& shader, const sp<GLShaderBindings>& shaderBindings)
{
    return sp<gles30::BindVertexArray>::make(glResourceManager.createGLResource<gles30::GLVertexArray>(shaderBindings, shader));
}

}
}
