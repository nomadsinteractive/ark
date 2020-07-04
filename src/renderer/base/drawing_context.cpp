#include "renderer/base/drawing_context.h"

#include "graphics/base/camera.h"
#include "graphics/inf/render_command.h"

#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/snippet_delegate.h"
#include "renderer/inf/pipeline.h"

namespace ark {

namespace {

class RenderCommandBind : public RenderCommand {
public:
    RenderCommandBind(DrawingContext context)
        : _context(std::move(context)) {
    }

    virtual void draw(GraphicsContext& graphicsContext) override {
        const sp<Pipeline> pipeline = _context._shader_bindings->getPipeline(graphicsContext);
        pipeline->bind(graphicsContext, _context);
    }

private:
    DrawingContext _context;
};

class RenderCommandDraw : public RenderCommand {
public:
    RenderCommandDraw(DrawingContext context)
        : _context(std::move(context)) {
    }

    virtual void draw(GraphicsContext& graphicsContext) override {
        _context.upload(graphicsContext);

        const sp<Pipeline> pipeline = _context._shader_bindings->getPipeline(graphicsContext);
        pipeline->bind(graphicsContext, _context);
        pipeline->draw(graphicsContext, _context);
    }

private:
    DrawingContext _context;
};

class RenderCommandDrawWithEvents : public RenderCommand {
public:
    RenderCommandDrawWithEvents(DrawingContext context, sp<Snippet::DrawEvents> snippetDraw)
        : _context(std::move(context)), _snippet_draw(std::move(snippetDraw)) {
    }

    virtual void draw(GraphicsContext& graphicsContext) override {
        _context.upload(graphicsContext);

        const sp<Pipeline> pipeline = _context._shader_bindings->getPipeline(graphicsContext);
        _snippet_draw->preDraw(graphicsContext, _context);
        pipeline->bind(graphicsContext, _context);
        pipeline->draw(graphicsContext, _context);
        _snippet_draw->postDraw(graphicsContext);
    }

private:
    DrawingContext _context;
    sp<Snippet::DrawEvents> _snippet_draw;
};

}


DrawingContext::DrawingContext(const sp<ShaderBindings>& shaderBindings, const sp<ByType>& attachments, std::vector<RenderLayer::UBOSnapshot> ubo)
    : _shader_bindings(shaderBindings), _attachments(attachments), _ubos(std::move(ubo)), _scissor(0, 0, -1.0f, -1.0f)
{
}

DrawingContext::DrawingContext(const sp<ShaderBindings>& shaderBindings, const sp<ByType>& attachments, std::vector<RenderLayer::UBOSnapshot> ubo, Buffer::Snapshot vertexBuffer, Buffer::Snapshot indexBuffer, Parameters parameters)
    : _shader_bindings(shaderBindings), _attachments(attachments), _ubos(std::move(ubo)), _vertex_buffer(std::move(vertexBuffer)), _index_buffer(std::move(indexBuffer)), _scissor(0, 0, -1.0f, -1.0f), _parameters(std::move(parameters))
{
}

sp<RenderCommand> DrawingContext::toRenderCommand(const RenderRequest& renderRequest)
{
    DCHECK(_shader_bindings, "DrawingContext cannot be converted to RenderCommand more than once");
    sp<Snippet::DrawEvents> drawEvents = _shader_bindings->snippet()->makeDrawEvents(renderRequest);
    return drawEvents ? sp<RenderCommand>::make<RenderCommandDrawWithEvents>(std::move(*this), std::move(drawEvents))
                      : sp<RenderCommand>::make<RenderCommandDraw>(std::move(*this));
}

sp<RenderCommand> DrawingContext::toBindCommand()
{
    DCHECK(_shader_bindings, "DrawingContext cannot be converted to RenderCommand more than once");
    return sp<RenderCommandBind>::make(std::move(*this));
}

void DrawingContext::upload(GraphicsContext& graphicsContext)
{
    _vertex_buffer.upload(graphicsContext);
    DCHECK(_vertex_buffer.id(), "Invaild VertexBuffer");
    if(_index_buffer)
    {
        _index_buffer.upload(graphicsContext);
        DCHECK(_index_buffer.id(), "Invaild IndexBuffer");
    }
}

DrawingContext::ParamDrawElements::ParamDrawElements(uint32_t start, uint32_t count)
    : _count(count), _start(start)
{
}

DrawingContext::Parameters::Parameters()
    : _draw_elements(0, 0)
{
}

DrawingContext::Parameters::Parameters(DrawingContext::Parameters&& other)
{
    if(other._draw_elements.isActive())
        _draw_elements = std::move(other._draw_elements);
    else if(other._draw_elements_instanced.isActive())
        new(&_draw_elements_instanced) auto(std::move(other._draw_elements_instanced));
    else if(other._draw_multi_elements_indirect.isActive())
        new(&_draw_multi_elements_indirect) auto(std::move(other._draw_multi_elements_indirect));
    else
        DFATAL("Shouldn't be here");
}

DrawingContext::Parameters::Parameters(const DrawingContext::Parameters& other)
{
    if(other._draw_elements.isActive())
        _draw_elements = other._draw_elements;
    else if(other._draw_elements_instanced.isActive())
        new(&_draw_elements_instanced) auto(other._draw_elements_instanced);
    else if(other._draw_multi_elements_indirect.isActive())
        new(&_draw_multi_elements_indirect) auto(other._draw_multi_elements_indirect);
    else
        DFATAL("Shouldn't be here");
}

DrawingContext::Parameters::Parameters(const DrawingContext::ParamDrawElements& drawElements)
    : _draw_elements(drawElements)
{
}

DrawingContext::Parameters::Parameters(DrawingContext::ParamDrawElementsInstanced drawElementsInstanced)
    : _draw_elements_instanced(std::move(drawElementsInstanced))
{
}

DrawingContext::Parameters::Parameters(ParamDrawMultiElementsIndirect drawMultiElementsIndirect)
    : _draw_multi_elements_indirect(std::move(drawMultiElementsIndirect))
{
}

DrawingContext::Parameters::~Parameters()
{
    if(_draw_elements.isActive())
        _draw_elements.~ParamDrawElements();
    else if(_draw_elements_instanced.isActive())
        _draw_elements_instanced.~ParamDrawElementsInstanced();
    else if(_draw_multi_elements_indirect.isActive())
        _draw_multi_elements_indirect.~ParamDrawMultiElementsIndirect();
    else
        DFATAL("Shouldn't be here");
}

DrawingContext::Parameters& DrawingContext::Parameters::operator =(const DrawingContext::Parameters& other)
{
    if(other._draw_elements.isActive())
        _draw_elements = other._draw_elements;
    else if(other._draw_elements_instanced.isActive())
        assign(_draw_elements_instanced, other._draw_elements_instanced);
    else if(other._draw_multi_elements_indirect.isActive())
        assign(_draw_multi_elements_indirect, other._draw_multi_elements_indirect);
    else
        DFATAL("Shouldn't be here");
    return *this;
}

DrawingContext::Parameters& DrawingContext::Parameters::operator =(DrawingContext::Parameters&& other)
{
    if(other._draw_elements.isActive())
        _draw_elements = other._draw_elements;
    else if(other._draw_elements_instanced.isActive())
        assign(_draw_elements_instanced, std::move(other._draw_elements_instanced));
    else if(other._draw_multi_elements_indirect.isActive())
        assign(_draw_multi_elements_indirect, std::move(other._draw_multi_elements_indirect));
    else
        DFATAL("Shouldn't be here");
    return *this;
}

DrawingContext::ParamDrawElementsInstanced::ParamDrawElementsInstanced(uint32_t start, uint32_t count, int32_t instanceCount, std::vector<std::pair<uint32_t, Buffer::Snapshot>> snapshots)
    : _count(count), _start(start), _instance_count(instanceCount), _instanced_array_snapshots(std::move(snapshots))
{
}

DrawingContext::ParamDrawMultiElementsIndirect::ParamDrawMultiElementsIndirect(std::vector<std::pair<uint32_t, Buffer::Snapshot>> snapshots, Buffer::Snapshot indirectCmds, uint32_t drawCount)
    : _instanced_array_snapshots(std::move(snapshots)), _indirect_cmds(std::move(indirectCmds)), _draw_count(drawCount)
{
}

}
