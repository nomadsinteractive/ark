#ifndef ARK_RENDERER_OPENGL_ES20_SNIPPET_FACTORY_GL_SNIPPET_FACTORY_GLES20_H_
#define ARK_RENDERER_OPENGL_ES20_SNIPPET_FACTORY_GL_SNIPPET_FACTORY_GLES20_H_

#include "renderer/inf/snippet_factory.h"

namespace ark {
namespace gles20 {

class SnippetFactoryGLES20 : public SnippetFactory {
public:
    virtual sp<Snippet> createCoreSnippet(ResourceManager& glResourceManager, const Shader& shader, const sp<ShaderBindings>& shaderBindings) override;
};

}
}

#endif
