#include "renderer/gles20/impl/gl_procedure_factory/gl_procedure_factory_gles20.h"

#include "renderer/gles20/impl/gl_snippet/bind_attributes.h"

namespace ark {
namespace gles20 {

sp<GLSnippet> GLProcedureFactoryGLES20::createCoreGLSnippet(const sp<GLResourceManager>& /*glResourceManager*/, const GLShader& /*shader*/, const GLBuffer& /*arrayBuffer*/)
{
    return sp<BindAttributes>::make();
}

}
}
