#include "renderer/impl/render_command_composer/rcc_draw_elements_incremental.h"

#include "core/impl/uploader/uploader_array.h"

#include "graphics/base/render_layer.h"

#include "renderer/base/drawing_buffer.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/model.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/shader.h"
#include "renderer/base/vertex_writer.h"

namespace ark {

sp<PipelineBindings> RCCDrawElementsIncremental::makePipelineBindings(const Shader& shader, RenderController& renderController, Enum::DrawMode renderMode)
{
    _strips = renderController.gba().makeStrips(shader.layout()->getStreamLayout(0).stride());
    _indices = renderController.makeIndexBuffer();
    return shader.makeBindings(_strips->buffer(), renderMode, Enum::DRAW_PROCEDURE_DRAW_ELEMENTS);
}

DrawingContext RCCDrawElementsIncremental::compose(const RenderRequest& renderRequest, const RenderLayerSnapshot& snapshot)
{
    DrawingBuffer buf(snapshot._stub->_pipeline_bindings, snapshot._stub->_stride);
    bool hasNewCreatedSnapshot = false;

    for(const LayerContext::ElementState& i : snapshot._elements_deleted)
        if(i._index)
            _strips->free(i._index.value());

    for(const RenderLayerSnapshot::Element& i : snapshot._elements)
    {
        const Renderable::State& s = i._snapshot._state;
        if(const bool isStateNew = s.has(Renderable::RENDERABLE_STATE_NEW); isStateNew || s.has(Renderable::RENDERABLE_STATE_DIRTY))
        {
            Model& model = i._snapshot._model;
            const uint32_t vertexCount = static_cast<uint32_t>(model.vertexCount());
            if(isStateNew)
            {
                hasNewCreatedSnapshot = true;
                i._element_state._index = _strips->allocate(vertexCount);
            }
            VertexWriter writer = buf.makeVertexWriter(renderRequest, vertexCount, i._element_state._index.value());
            model.writeRenderable(writer, i._snapshot);
        }
    }

    sp<Uploader> indexUploader;
    if(hasNewCreatedSnapshot || !snapshot._elements_deleted.empty())
    {
        element_index_t offset = 0;
        std::vector<element_index_t> indices(snapshot._index_count);
        for(const RenderLayerSnapshot::Element& i : snapshot._elements)
        {
            Model& model = i._snapshot._model;
            if(i._snapshot._state.has(Renderable::RENDERABLE_STATE_VISIBLE))
                offset += model.writeIndices(indices.data() + offset, i._element_state._index.value());
            else
                offset += static_cast<element_index_t>(model.indices()->size() / sizeof(element_index_t));
        }
        indexUploader = sp<UploaderArray<element_index_t>>::make(std::move(indices));
    }

    const Buffer& vertices = snapshot._stub->_pipeline_bindings->vertices();
    return snapshot.toDrawingContext(renderRequest, buf.vertices().toSnapshot(vertices), _indices.snapshot(indexUploader), snapshot._index_count, DrawingParams::DrawElements{0});
}

}
