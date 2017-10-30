#ifndef ARK_RENDERER_INF_GL_SNIPPET_FACTORY_H_
#define ARK_RENDERER_INF_GL_SNIPPET_FACTORY_H_

#include "core/ark.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API GLProcedureFactory {
public:
    virtual ~GLProcedureFactory() = default;

    virtual sp<GLSnippet> createCoreGLSnippet(const sp<GLResourceManager>& glResourceManager, const GLShader& shader, const GLBuffer& arrayBuffer) = 0;
};

}

#endif
