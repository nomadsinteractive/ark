#include "renderer/base/snippet_delegate.h"

#include "renderer/base/buffer.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/render_context.h"
#include "renderer/base/shader.h"
#include "renderer/impl/snippet/snippet_linked_chain.h"
#include "renderer/inf/snippet_factory.h"

namespace ark {

class CoreGLSnippet : public Snippet {
public:
    CoreGLSnippet(SnippetDelegate& wrapper, const sp<Snippet>& snippet);

    virtual void preInitialize(PipelineBuildingContext& context) override;
    virtual void preBind(GraphicsContext& graphicsContext, const sp<Pipeline>& pipeline, ShaderBindings& bindings) override;
    virtual void preCompile(GraphicsContext& graphicsContext, PipelineBuildingContext& context, const PipelineLayout& pipelineLayout) override;
    virtual void preDraw(GraphicsContext& graphicsContext, const DrawingContext& context) override;
    virtual void postDraw(GraphicsContext& graphicsContext) override;

private:
    sp<Snippet> createCoreSnippet(GraphicsContext& graphicsContext) const;

private:
    SnippetDelegate& _wrapper;
    sp<Snippet> _snippet;
};

CoreGLSnippet::CoreGLSnippet(SnippetDelegate& wrapper, const sp<Snippet>& snippet)
    : _wrapper(wrapper), _snippet(snippet)
{
}

void CoreGLSnippet::preInitialize(PipelineBuildingContext& context)
{
    if(_snippet)
        _snippet->preInitialize(context);
}

void CoreGLSnippet::preCompile(GraphicsContext& graphicsContext, PipelineBuildingContext& context, const PipelineLayout& pipelineLayout)
{
    const sp<Snippet> delegate = _wrapper._core;
    _wrapper._core = createCoreSnippet(graphicsContext);
    _wrapper.preCompile(graphicsContext, context, pipelineLayout);
}

void CoreGLSnippet::preBind(GraphicsContext& /*graphicsContext*/, const sp<Pipeline>& /*pipeline*/, ShaderBindings& /*bindings*/)
{
    DFATAL("You're not supposed to be here");
}

void CoreGLSnippet::preDraw(GraphicsContext& /*graphicsContext*/, const DrawingContext& /*context*/)
{
    DFATAL("You're not supposed to be here");
}

void CoreGLSnippet::postDraw(GraphicsContext& /*graphicsContext*/)
{
    DFATAL("You're not supposed to be here");
}

sp<Snippet> CoreGLSnippet::createCoreSnippet(GraphicsContext& graphicsContext) const
{
    const sp<Snippet> coreSnippet = graphicsContext.renderContext()->snippetFactory()->createCoreSnippet(graphicsContext.renderController());
    DASSERT(coreSnippet);
    return _snippet ? sp<Snippet>::adopt(new SnippetLinkedChain(coreSnippet, _snippet)) : coreSnippet;
}

SnippetDelegate::SnippetDelegate(const sp<Snippet>& snippet)
    : _core(sp<CoreGLSnippet>::make(*this, snippet))
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

void SnippetDelegate::preBind(GraphicsContext& graphicsContext, const sp<Pipeline>& pipeline, ShaderBindings& bindings)
{
    _core->preBind(graphicsContext, pipeline, bindings);
}

void SnippetDelegate::preDraw(GraphicsContext& graphicsContext, const DrawingContext& context)
{
    _core->preDraw(graphicsContext, context);
}

void SnippetDelegate::postDraw(GraphicsContext& graphicsContext)
{
    _core->postDraw(graphicsContext);
}

}
