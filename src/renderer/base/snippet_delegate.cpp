#include "renderer/base/snippet_delegate.h"

#include "renderer/base/buffer.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/gl_context.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/shader.h"
#include "renderer/impl/snippet/snippet_linked_chain.h"
#include "renderer/inf/snippet_factory.h"

namespace ark {

class CoreGLSnippet : public Snippet {
public:
    CoreGLSnippet(SnippetDelegate& wrapper, const sp<Snippet>& snippet);

    virtual void preInitialize(PipelineBuildingContext& context) override;
    virtual void preCompile(GraphicsContext& graphicsContext, PipelineBuildingContext& context, const sp<ShaderBindings>& shaderBindings) override;
    virtual void preDraw(GraphicsContext& graphicsContext, Shader& shader, const DrawingContext& context) override;
    virtual void postDraw(GraphicsContext& graphicsContext) override;

private:
    sp<Snippet> createCoreSnippet(GraphicsContext& graphicsContext, const sp<PipelineFactory>& pipelineFactory, const sp<ShaderBindings>& bindings) const;

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

void CoreGLSnippet::preCompile(GraphicsContext& graphicsContext, PipelineBuildingContext& context, const sp<ShaderBindings>& shaderBindings)
{
    const sp<Snippet> delegate = _wrapper._core;
    _wrapper._core = createCoreSnippet(graphicsContext, context._shader, shaderBindings);
    _wrapper.preCompile(graphicsContext, context, shaderBindings);
}

void CoreGLSnippet::preDraw(GraphicsContext& /*graphicsContext*/, Shader& shader, const DrawingContext& /*context*/)
{
    DFATAL("You're not supposed to be here");
}

void CoreGLSnippet::postDraw(GraphicsContext& /*graphicsContext*/)
{
    DFATAL("You're not supposed to be here");
}

sp<Snippet> CoreGLSnippet::createCoreSnippet(GraphicsContext& graphicsContext, const sp<PipelineFactory>& pipelineFactory, const sp<ShaderBindings>& bindings) const
{
    const sp<Snippet> coreSnippet = graphicsContext.glContext()->snippetFactory()->createCoreSnippet(graphicsContext.resourceManager(), pipelineFactory, bindings);
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

void SnippetDelegate::preCompile(GraphicsContext& graphicsContext, PipelineBuildingContext& context, const sp<ShaderBindings>& shaderBindings)
{
    _core->preCompile(graphicsContext, context, shaderBindings);
}

void SnippetDelegate::preDraw(GraphicsContext& graphicsContext, Shader& shader, const DrawingContext& context)
{
    _core->preDraw(graphicsContext, shader, context);
}

void SnippetDelegate::postDraw(GraphicsContext& graphicsContext)
{
    _core->postDraw(graphicsContext);
}

}
