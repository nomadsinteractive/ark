#ifndef ARK_RENDERER_INF_SNIPPET_FACTORY_H_
#define ARK_RENDERER_INF_SNIPPET_FACTORY_H_

#include "core/ark.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API SnippetFactory {
public:
    virtual ~SnippetFactory() = default;

    virtual sp<Snippet> createCoreSnippet(RenderController& resourceManager, const sp<PipelineFactory>& pipelineFactory, const sp<ShaderBindings>& shaderBindings) = 0;
};

}

#endif
