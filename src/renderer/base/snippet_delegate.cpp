#include "renderer/base/snippet_delegate.h"

#include "renderer/base/buffer.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/render_engine_context.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/shader.h"
#include "renderer/impl/snippet/snippet_linked_chain.h"
#include "renderer/inf/snippet_factory.h"

namespace ark {

class CoreSnippet : public Snippet {
public:
    CoreSnippet(SnippetDelegate& wrapper, const sp<Snippet>& snippet);

    virtual void preInitialize(PipelineBuildingContext& context) override;
    virtual void preCompile(GraphicsContext& graphicsContext, PipelineBuildingContext& context, const PipelineLayout& pipelineLayout) override;
    virtual sp<DrawEvents> makeDrawEvents() override;

private:
    SnippetDelegate& _wrapper;
    sp<Snippet> _snippet;
};

class CoreDrawEvents : public Snippet::DrawEvents {
public:
    CoreDrawEvents(SnippetDelegate& wrapper, const sp<Snippet>& snippet);

    virtual void preDraw(GraphicsContext& graphicsContext, const DrawingContext& context) override;
    virtual void postDraw(GraphicsContext& graphicsContext) override;

private:
    SnippetDelegate& _wrapper;
    sp<Snippet> _snippet;

    sp<Snippet::DrawEvents> _delegate;
};

static sp<Snippet> createCoreSnippet(GraphicsContext& graphicsContext, sp<Snippet> next)
{
    const sp<Snippet> coreSnippet = graphicsContext.renderContext()->snippetFactory()->createCoreSnippet();
    DASSERT(coreSnippet);
    return next ? sp<Snippet>::make<SnippetLinkedChain>(std::move(coreSnippet), std::move(next)) : coreSnippet;
}


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
    _wrapper._core = createCoreSnippet(graphicsContext, _snippet);
    _wrapper.preCompile(graphicsContext, context, pipelineLayout);
}

sp<Snippet::DrawEvents> CoreSnippet::makeDrawEvents()
{
    return sp<CoreDrawEvents>::make(_wrapper, _snippet);
}

CoreDrawEvents::CoreDrawEvents(SnippetDelegate& wrapper, const sp<Snippet>& snippet)
    : _wrapper(wrapper), _snippet(snippet)
{
}

void CoreDrawEvents::preDraw(GraphicsContext& graphicsContext, const DrawingContext& context)
{
    const sp<Snippet> delegate = _wrapper._core;
    _wrapper._core = createCoreSnippet(graphicsContext, _snippet);
    _delegate = _wrapper._core->makeDrawEvents();
    _delegate->preDraw(graphicsContext, context);
}

void CoreDrawEvents::postDraw(GraphicsContext& graphicsContext)
{
    _delegate->postDraw(graphicsContext);
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

sp<Snippet::DrawEvents> SnippetDelegate::makeDrawEvents()
{
    return _core->makeDrawEvents();
}

}
