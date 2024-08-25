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

namespace {

sp<Snippet> createCoreSnippet(const GraphicsContext& graphicsContext, sp<Snippet> next)
{
    sp<Snippet> coreSnippet = graphicsContext.renderContext()->snippetFactory()->createCoreSnippet();
    DASSERT(coreSnippet);
    return next ? sp<Snippet>::make<SnippetLinkedChain>(std::move(coreSnippet), std::move(next)) : coreSnippet;
}

class CoreDrawEvents final : public Snippet::DrawEvents {
public:
    CoreDrawEvents(SnippetDelegate& wrapper, const sp<Snippet>& snippet)
        : _wrapper(wrapper), _snippet(snippet)
    {
    }

    void preDraw(GraphicsContext& graphicsContext, const DrawingContext& context) override
    {
        const sp<Snippet> delegate = _wrapper.wrapped();
        _wrapper.reset(createCoreSnippet(graphicsContext, _snippet));
        _delegate = _wrapper.wrapped()->makeDrawEvents();
        _delegate->preDraw(graphicsContext, context);
    }

    void postDraw(GraphicsContext& graphicsContext, const DrawingContext& context) override
    {
        _delegate->postDraw(graphicsContext, context);
    }

private:
    SnippetDelegate& _wrapper;
    sp<Snippet> _snippet;

    sp<Snippet::DrawEvents> _delegate;
};

class CoreSnippet final : public Snippet {
public:
    CoreSnippet(SnippetDelegate& wrapper, const sp<Snippet>& snippet)
        : _wrapper(wrapper), _snippet(snippet)
    {
    }

    void preInitialize(PipelineBuildingContext& context) override
    {
        if(_snippet)
            _snippet->preInitialize(context);
    }

    void preCompile(GraphicsContext& graphicsContext, PipelineBuildingContext& context, const PipelineLayout& pipelineLayout) override
    {
        const sp<Snippet> delegate = _wrapper.wrapped();
        _wrapper.reset(createCoreSnippet(graphicsContext, _snippet));
        _wrapper.preCompile(graphicsContext, context, pipelineLayout);
    }

    sp<DrawEvents> makeDrawEvents() override
    {
        return sp<CoreDrawEvents>::make(_wrapper, _snippet);
    }

private:
    SnippetDelegate& _wrapper;
    sp<Snippet> _snippet;
};

}

SnippetDelegate::SnippetDelegate(sp<Snippet> snippet)
    : Wrapper(sp<Snippet>::make<CoreSnippet>(*this, snippet))
{
}

void SnippetDelegate::preInitialize(PipelineBuildingContext& context)
{
    _wrapped->preInitialize(context);
}

void SnippetDelegate::preCompile(GraphicsContext& graphicsContext, PipelineBuildingContext& context, const PipelineLayout& pipelineLayout)
{
    _wrapped->preCompile(graphicsContext, context, pipelineLayout);
}

sp<Snippet::DrawEvents> SnippetDelegate::makeDrawEvents(const RenderRequest& renderRequest)
{
    return _wrapped->makeDrawEvents(renderRequest);
}

sp<Snippet::DrawEvents> SnippetDelegate::makeDrawEvents()
{
    return _wrapped->makeDrawEvents();
}

}
