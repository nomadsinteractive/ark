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
    GLContext(Ark::RendererVersion version);

    Ark::RendererVersion version() const;
    void setVersion(Ark::RendererVersion version);

    const std::map<String, String>& annotations() const;
    std::map<String, String>& annotations();

    void setGLSnippetFactory(sp<SnippetFactory> snippetfactory);
    sp<Snippet> createCoreGLSnippet(const sp<ResourceManager>& glResourceManager, const sp<ShaderBindings>& shaderBindings) const;

    uint32_t getGLSLVersion() const;

private:
    Ark::RendererVersion _version;
    std::map<String, String> _annotations;

    sp<SnippetFactory> _gl_procedure_factory;
};

}

#endif
