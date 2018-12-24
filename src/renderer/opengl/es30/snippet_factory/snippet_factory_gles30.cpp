#include "renderer/opengl/es30/snippet_factory/snippet_factory_gles30.h"

#include "renderer/opengl/es30/snippet/bind_vertex_array.h"
#include "renderer/opengl/es30/gl_resource/gl_vertex_array.h"

#include "renderer/base/resource_manager.h"

namespace ark {
namespace gles30 {

sp<Snippet> SnippetFactoryGLES30::createCoreSnippet(ResourceManager& glResourceManager, const sp<ShaderBindings>& shaderBindings)
{
    return sp<BindVertexArray>::make(glResourceManager.createGLResource<GLVertexArray>(shaderBindings));
}

}
}
