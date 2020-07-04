#include "renderer/base/snippet_delegate.h"

#include "renderer/base/buffer.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/render_engine_context.h"
#include "renderer/base/shader.h"
#include "renderer/impl/snippet/snippet_linked_chain.h"
#include "renderer/inf/snippet_factory.h"

namespace ark {

class CoreSnippet : public Snippet {
public:
    CoreSnippet(SnippetDelegate& wrapper, const sp<Snippet>& snippet);

    virtual void preInitialize(PipelineBuildingContext& context) override;
    virtual void preCompile(GraphicsContext& graphicsContext, PipelineBuildingContext& context, const PipelineLayout& pipelineLayout) override;
    virtual sp<DrawEvents> makeDrawEvents(const RenderRequest& renderRequest) override;

private:
    sp<Snippet> createCoreSnippet(GraphicsContext& graphicsContext) const;

private:
    SnippetDelegate& _wrapper;
    sp<Snippet> _snippet;

};

CoreSnippet::CoreSnippet(SnippetDelegate& wrapper, const sp<Snippet>& snippet)
    : _wrapper(wrapper), _snippet(snippet)
{
}

void CoreSnippet::preInitialize(PipelineBuildingContext& context)
{
    if(_snippet)
        _snippet->preInitialize(context);
}

void CoreSnippet::preCompile(GraphicsContext& graphicsContext, PipelineBuildingContext& context, const PipelineLayout& pipelineLayout)
{
    const sp<Snippet> delegate = _wrapper._core;
    _wrapper._core = createCoreSnippet(graphicsContext);
    _wrapper.preCompile(graphicsContext, context, pipelineLayout);
}

sp<Snippet::DrawEvents> CoreSnippet::makeDrawEvents(const RenderRequest& /*renderRequest*/)
{
    return nullptr;
}

sp<Snippet> CoreSnippet::createCoreSnippet(GraphicsContext& graphicsContext) const
{
    const sp<Snippet> coreSnippet = graphicsContext.renderContext()->snippetFactory()->createCoreSnippet(graphicsContext.renderController());
    DASSERT(coreSnippet);
    return _snippet ? sp<Snippet>::make<SnippetLinkedChain>(coreSnippet, _snippet) : coreSnippet;
}

SnippetDelegate::SnippetDelegate(const sp<Snippet>& snippet)
    : _core(sp<CoreSnippet>::make(*this, snippet))
{
}

void SnippetDelegate::preInitialize(PipelineBuildingContext& context)
{
    _core->preInitialize(context);
}

void SnippetDelegate::preCompile(GraphicsContext& graphicsContext, PipelineBuildingContext& context, const PipelineLayout& pipelineLayout)
{
    _core->preCompile(graphicsContext, context, pipelineLayout);
}

sp<Snippet::DrawEvents> SnippetDelegate::makeDrawEvents(const RenderRequest& renderRequest)
{
    return _core->makeDrawEvents(renderRequest);
}

}
