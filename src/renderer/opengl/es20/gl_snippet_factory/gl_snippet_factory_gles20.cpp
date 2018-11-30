#include "renderer/opengl/es20/gl_snippet_factory/gl_snippet_factory_gles20.h"

#include "renderer/opengl/es20/gl_snippet/bind_attributes.h"

namespace ark {
namespace gles20 {

sp<GLSnippet> GLSnippetFactoryGLES20::createCoreGLSnippet(GLResourceManager& /*glResourceManager*/, const Shader& /*shader*/, const sp<ShaderBindings>& /*shaderBindings*/)
{
    return sp<BindAttributes>::make();
}

}
}
