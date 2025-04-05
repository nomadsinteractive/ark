#include "renderer/base/pipeline_bindings.h"

#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/render_controller.h"
#include "renderer/impl/snippet/snippet_composite.h"
#include "renderer/inf/pipeline.h"

namespace ark {

namespace {

class PipelineComposite final : public Pipeline {
public:
    PipelineComposite(sp<Pipeline> pipelineDraw, sp<Pipeline> pipelineCompute)
        : _pipeline_draw(std::move(pipelineDraw)), _pipeline_compute(std::move(pipelineCompute)) {
    }

    uint64_t id() override
    {
        return _pipeline_draw->id() ? _pipeline_draw->id() : 0;
    }

    void upload(GraphicsContext& graphicsContext) override
    {
    }

    ResourceRecycleFunc recycle() override
    {
        ResourceRecycleFunc func1 = _pipeline_draw->recycle();
        ResourceRecycleFunc func2 = _pipeline_compute->recycle();
        return [func1 = std::move(func1), func2 = std::move(func2)] (GraphicsContext& graphicsContext) {
            func1(graphicsContext);
            func2(graphicsContext);
        };
    }

    void draw(GraphicsContext& graphicsContext, const DrawingContext& drawingContext) override
    {
        if(_pipeline_draw->id() == 0)
            _pipeline_draw->upload(graphicsContext);
        _pipeline_draw->draw(graphicsContext, drawingContext);
    }

    void compute(GraphicsContext& graphicsContext, const ComputeContext& computeContext) override
    {
        if(_pipeline_compute->id() == 0)
            _pipeline_compute->upload(graphicsContext);
        _pipeline_compute->compute(graphicsContext, computeContext);
    }

private:
    sp<Pipeline> _pipeline_draw;
    sp<Pipeline> _pipeline_compute;
};

}

struct PipelineBindings::Stub {
    Stub(const Enum::DrawMode drawMode, const Enum::DrawProcedure drawProcedure, Buffer vertices, sp<PipelineDescriptor> pipelineDescriptor, Vector<std::pair<uint32_t, Buffer>> streams)
        : _draw_mode(drawMode), _draw_procedure(drawProcedure), _vertices(std::move(vertices)), _pipeline_descriptor(std::move(pipelineDescriptor)), _attachments(sp<Traits>::make()), _streams(std::move(streams)),
          _samplers(_pipeline_descriptor->makeBindingSamplers()), _images(_pipeline_descriptor->makeBindingImages())
    {
    }

    Enum::DrawMode _draw_mode;
    Enum::DrawProcedure _draw_procedure;
    Buffer _vertices;

    sp<PipelineDescriptor> _pipeline_descriptor;
    sp<Traits> _attachments;

    Vector<std::pair<uint32_t, Buffer>> _streams;
    Vector<std::pair<sp<Texture>, PipelineLayout::DescriptorSet>> _samplers;
    Vector<std::pair<sp<Texture>, PipelineLayout::DescriptorSet>> _images;
};

PipelineBindings::PipelineBindings(const Enum::DrawMode drawMode, const Enum::DrawProcedure drawProcedure, Buffer vertices, sp<PipelineDescriptor> pipelineDescriptor, Vector<std::pair<uint32_t, Buffer>> streams)
    : _stub(sp<Stub>::make(drawMode, drawProcedure, std::move(vertices), std::move(pipelineDescriptor), std::move(streams)))
{
}

Enum::DrawMode PipelineBindings::drawMode() const
{
    return _stub->_draw_mode;
}

Enum::DrawProcedure PipelineBindings::drawProcedure() const
{
    return _stub->_draw_procedure;
}

const Buffer& PipelineBindings::vertices() const
{
    return _stub->_vertices;
}

const sp<PipelineDescriptor>& PipelineBindings::pipelineDescriptor() const
{
    return _stub->_pipeline_descriptor;
}

const sp<Snippet>& PipelineBindings::snippet() const
{
    return _stub->_pipeline_descriptor->snippet();
}

const sp<PipelineLayout>& PipelineBindings::pipelineLayout() const
{
    return _stub->_pipeline_descriptor->layout();
}

const sp<Traits>& PipelineBindings::attachments() const
{
    return _stub->_attachments;
}

const Vector<std::pair<uint32_t, Buffer>>& PipelineBindings::streams() const
{
    return _stub->_streams;
}

const Vector<std::pair<sp<Texture>, PipelineLayout::DescriptorSet>>& PipelineBindings::samplers() const
{
    return _stub->_samplers;
}

const Vector<std::pair<sp<Texture>, PipelineLayout::DescriptorSet>>& PipelineBindings::images() const
{
    return _stub->_images;
}

void PipelineBindings::bindSampler(sp<Texture> texture, const uint32_t name) const
{
    CHECK_WARN(_stub->_samplers.size() > name, "Illegal sampler binding position: %d, sampler count: %d", name, _stub->_samplers.size());
    if(_stub->_samplers.size() > name)
    {
        CHECK_WARN(!_stub->_samplers[name].first, "Overriding existing sampler binding: %d", name);
        _stub->_samplers[name].first = std::move(texture);
    }
}

const sp<Pipeline>& PipelineBindings::ensurePipeline(GraphicsContext& graphicsContext)
{
    if(!_pipeline)
        doEnsurePipeline(graphicsContext);

    if(_pipeline->id() == 0)
        _pipeline->upload(graphicsContext);

    return _pipeline;
}

void PipelineBindings::doEnsurePipeline(GraphicsContext& graphicsContext)
{
    _stub->_pipeline_descriptor->preCompile(graphicsContext);

    Map<Enum::ShaderStageBit, ShaderPreprocessor::Stage> stages = _stub->_pipeline_descriptor->getPreprocessedStages(graphicsContext.renderContext());
    ASSERT(!stages.empty());

    Map<Enum::ShaderStageBit, String> sources;
    for(auto& [k, v] : stages)
        sources.emplace(k, std::move(v._source));

    RenderEngine& renderEngine = Ark::instance().renderController()->renderEngine();
    if(const auto iter = sources.find(Enum::SHADER_STAGE_BIT_COMPUTE); iter != sources.end() && sources.size() > 1)
    {
        _compute_pipeline = renderEngine.createPipeline(graphicsContext, *this, Map<Enum::ShaderStageBit, String>{{iter->first, iter->second}});
        sources.erase(iter);

        _render_pipeline = renderEngine.createPipeline(graphicsContext, *this, std::move(sources));
        _pipeline = sp<Pipeline>::make<PipelineComposite>(_render_pipeline, _compute_pipeline);
    }
    else
    {
        const bool hasComputeStage = iter != sources.end();
        _pipeline = renderEngine.createPipeline(graphicsContext, *this, std::move(sources));
        (hasComputeStage ? _compute_pipeline : _render_pipeline) = _pipeline;
    }
}

const sp<Pipeline>& PipelineBindings::ensureRenderPipeline(GraphicsContext& graphicsContext)
{
    ensurePipeline(graphicsContext);
    const sp<Pipeline>& pipeline = _render_pipeline ? _render_pipeline : _pipeline;
    if(pipeline->id() == 0)
        pipeline->upload(graphicsContext);
    return pipeline;
}

const sp<Pipeline>& PipelineBindings::ensureComputePipeline(GraphicsContext& graphicsContext)
{
    ensurePipeline(graphicsContext);
    const sp<Pipeline>& pipeline = _compute_pipeline ? _compute_pipeline : _pipeline;
    if(pipeline->id() == 0)
        pipeline->upload(graphicsContext);
    return pipeline;
}

Map<uint32_t, Buffer::Factory> PipelineBindings::makeDividedBufferFactories() const
{
    Map<uint32_t, Buffer::Factory> builders;
    const sp<PipelineLayout>& shaderLayout = _stub->_pipeline_descriptor->layout();
    for(const auto& i : _stub->_streams)
    {
        const PipelineLayout::StreamLayout& stream = shaderLayout->getStreamLayout(i.first);
        builders.insert(std::make_pair(i.first, Buffer::Factory(stream.stride())));
    }
    return builders;
}

}
