#ifndef ARK_RENDERER_OPENGL_ES30_GL_SNIPPET_FACTORY_GL_SNIPPET_FACTORY_GLES30_H_
#define ARK_RENDERER_OPENGL_ES30_GL_SNIPPET_FACTORY_GL_SNIPPET_FACTORY_GLES30_H_

#include "renderer/inf/gl_snippet_factory.h"

namespace ark {
namespace gles30 {

class GLSnippetFactoryGLES30 : public GLSnippetFactory {
public:
    virtual sp<GLSnippet> createCoreGLSnippet(GLResourceManager& glResourceManager, const Shader& shader, const sp<ShaderBindings>& shaderBindings) override;
};

}
}

#endif
