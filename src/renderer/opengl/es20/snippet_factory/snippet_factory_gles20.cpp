#include "renderer/opengl/es20/snippet_factory/snippet_factory_gles20.h"

#include "renderer/opengl/es20/snippet/bind_attributes.h"

namespace ark {
namespace gles20 {

sp<Snippet> SnippetFactoryGLES20::createCoreSnippet(ResourceManager& /*glResourceManager*/, const Shader& /*shader*/, const sp<ShaderBindings>& /*shaderBindings*/)
{
    return sp<BindAttributes>::make();
}

}
}
