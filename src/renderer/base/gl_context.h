#ifndef ARK_RENDERER_BASE_GL_CONTEXT_H_
#define ARK_RENDERER_BASE_GL_CONTEXT_H_

#include <map>

#include "core/ark.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

namespace ark {

class GLContext {
public:
    GLContext(Ark::GLVersion version);

    Ark::GLVersion version() const;
    void setVersion(Ark::GLVersion version);

    const std::map<String, String>& annotations() const;
    std::map<String, String>& annotations();

    uint32_t getGLSLVersion() const;

    sp<GLSnippet> createCoreGLSnippet(const sp<GLResourceManager>& glResourceManager, const sp<GLShader>& shader, const GLBuffer& arrayBuffer) const;

private:
    Ark::GLVersion _version;
    std::map<String, String> _annotations;

    sp<GLSnippetFactory> _gl_procedure_factory;

    friend class RenderEngine;
};

}

#endif
