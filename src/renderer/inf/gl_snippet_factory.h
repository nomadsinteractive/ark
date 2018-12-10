#ifndef ARK_RENDERER_INF_GL_SNIPPET_FACTORY_H_
#define ARK_RENDERER_INF_GL_SNIPPET_FACTORY_H_

#include "core/ark.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API GLSnippetFactory {
public:
    virtual ~GLSnippetFactory() = default;

    virtual sp<GLSnippet> createCoreGLSnippet(ResourceManager& glResourceManager, const Shader& shader, const sp<ShaderBindings>& shaderBindings) = 0;
};

}

#endif
