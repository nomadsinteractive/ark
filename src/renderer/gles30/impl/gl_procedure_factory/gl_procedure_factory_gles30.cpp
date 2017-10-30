#include "renderer/gles30/impl/gl_procedure_factory/gl_procedure_factory_gles30.h"

#include "renderer/gles30/impl/gl_snippet/bind_vertex_array.h"
#include "renderer/gles30/gl_vertex_array.h"

#include "renderer/base/gl_resource_manager.h"

namespace ark {
namespace gles30 {

sp<GLSnippet> GLProcedureFactoryGLES30::createCoreGLSnippet(const sp<GLResourceManager>& glResourceManager, const GLShader& shader, const GLBuffer& arrayBuffer)
{
    return sp<BindVertexArray>::make(glResourceManager->createGLResource<GLVertexArray>(shader, arrayBuffer));
}

}
}
