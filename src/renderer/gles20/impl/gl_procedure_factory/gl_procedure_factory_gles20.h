#ifndef ARK_RENDERER_GLES20_IMPL_GL_SNIPPET_FACTORY_GL_SNIPPET_FACTORY_GLES20_H_
#define ARK_RENDERER_GLES20_IMPL_GL_SNIPPET_FACTORY_GL_SNIPPET_FACTORY_GLES20_H_

#include "renderer/inf/gl_procedure_factory.h"

namespace ark {
namespace gles20 {

class GLProcedureFactoryGLES20 : public GLProcedureFactory {
public:
    virtual sp<GLSnippet> createCoreGLSnippet(const sp<GLResourceManager>& glResourceManager, const GLShader& shader, const GLBuffer& arrayBuffer) override;
};

}
}

#endif
