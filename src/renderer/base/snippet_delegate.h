#ifndef ARK_RENDERER_BASE_SNIPPET_DELEGATE_H_
#define ARK_RENDERER_BASE_SNIPPET_DELEGATE_H_

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "renderer/inf/snippet.h"

namespace ark {

class CoreSnippet;

class ARK_API SnippetDelegate : public Snippet {
public:
    SnippetDelegate(const sp<Snippet>& snippet);

    virtual void preInitialize(PipelineBuildingContext& context) override;
    virtual void preCompile(GraphicsContext& graphicsContext, PipelineBuildingContext& context, const PipelineLayout& pipelineLayout) override;
    virtual sp<DrawEvents> makeDrawEvents(const RenderRequest& renderRequest) override;
    virtual sp<DrawEvents> makeDrawEvents() override;

private:
    sp<Snippet> _core;

    friend class CoreSnippet;
};

}

#endif
