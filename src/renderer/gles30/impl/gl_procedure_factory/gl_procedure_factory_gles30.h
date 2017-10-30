#ifndef ARK_RENDERER_GLES30_IMPL_GL_SNIPPET_FACTORY_GL_SNIPPET_FACTORY_GLES30_H_
#define ARK_RENDERER_GLES30_IMPL_GL_SNIPPET_FACTORY_GL_SNIPPET_FACTORY_GLES30_H_

#include "renderer/inf/gl_procedure_factory.h"

namespace ark {
namespace gles30 {

class GLProcedureFactoryGLES30 : public GLProcedureFactory {
public:
    virtual sp<GLSnippet> createCoreGLSnippet(const sp<GLResourceManager>& glResourceManager, const GLShader& shader, const GLBuffer& arrayBuffer) override;
};

}
}

#endif
