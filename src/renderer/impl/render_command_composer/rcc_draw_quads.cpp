#include "renderer/impl/render_command_composer/rcc_draw_quads.h"

#include "core/impl/uploader/uploader_array.h"

#include "graphics/base/render_layer.h"
#include "graphics/base/render_request.h"

#include "renderer/base/drawing_buffer.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/shader.h"
#include "renderer/base/shared_indices.h"
#include "renderer/base/vertex_writer.h"
#include "renderer/inf/model_loader.h"
#include "renderer/inf/vertices.h"

namespace ark {

RCCDrawQuads::RCCDrawQuads(Model model)
    : _model(std::move(model))
{
}

sp<ShaderBindings> RCCDrawQuads::makeShaderBindings(Shader& shader, RenderController& renderController, Enum::RenderMode renderMode)
{
    _strips = renderController.gba().makeStrips(shader.input()->getStream(0).stride(), _model.vertexCount());
    _indices = renderController.makeIndexBuffer();
    return shader.makeBindings(_strips->buffer(), renderMode, Enum::DRAW_PROCEDURE_DRAW_ELEMENTS);
}

void RCCDrawQuads::postSnapshot(RenderController& renderController, RenderLayerSnapshot& snapshot)
{
}

sp<RenderCommand> RCCDrawQuads::compose(const RenderRequest& renderRequest, RenderLayerSnapshot& snapshot)
{
    DrawingBuffer buf(snapshot._stub->_shader_bindings, snapshot._stub->_stride);
    bool hasNewCreatedSnapshot = false;

    for(const LayerContext::ElementState& i : snapshot._item_deleted)
        if(i._index)
            _strips->free(i._index.value());

    for(RenderLayerSnapshot::Droplet& i : snapshot._droplets)
    {
        const Renderable::State& s = i._snapshot._state;
        bool isStateNew = s.hasState(Renderable::RENDERABLE_STATE_NEW);
        if(isStateNew || s.hasState(Renderable::RENDERABLE_STATE_DIRTY))
        {
            Model& model = i._snapshot._model;
            uint32_t vertexCount = static_cast<uint32_t>(model.vertexCount());
            if(isStateNew)
            {
                hasNewCreatedSnapshot = true;
                i._element_state._index = _strips->allocate(vertexCount);
            }
            model.writeRenderable(buf.makeVertexWriter(renderRequest, vertexCount, i._element_state._index.value()), i._snapshot);
        }
    }

    sp<Uploader> indexUploader;
    if(hasNewCreatedSnapshot || snapshot._item_deleted.size() != 0)
    {
        element_index_t offset = 0;
        std::vector<element_index_t> indices(snapshot._index_count);
        for(RenderLayerSnapshot::Droplet& i : snapshot._droplets)
        {
            Model& model = i._snapshot._model;
            if(i._snapshot._state.hasState(Renderable::RENDERABLE_STATE_VISIBLE))
                offset += model.writeIndices(indices.data() + offset, i._element_state._index.value());
            else
                offset += static_cast<element_index_t>(model.indices()->size() / sizeof(element_index_t));
        }
        indexUploader = sp<UploaderArray<element_index_t>>::make(std::move(indices));
    }

    const Buffer& vertices = snapshot._stub->_shader_bindings->vertices();
    return snapshot.toRenderCommand(renderRequest, buf.vertices().toSnapshot(vertices), _indices.snapshot(indexUploader), snapshot._index_count, DrawingParams::DrawElements{0});
}

}
