#include "renderer/opengl/es30/gl_snippet_factory/gl_snippet_factory_gles30.h"

#include "renderer/opengl/es30/gl_snippet/bind_vertex_array.h"
#include "renderer/opengl/es30/gl_resource/gl_vertex_array.h"

#include "renderer/base/resource_manager.h"

namespace ark {
namespace gles30 {

sp<GLSnippet> GLSnippetFactoryGLES30::createCoreGLSnippet(ResourceManager& glResourceManager, const Shader& shader, const sp<ShaderBindings>& shaderBindings)
{
    return sp<BindVertexArray>::make(glResourceManager.createGLResource<GLVertexArray>(shaderBindings, shader));
}

}
}
