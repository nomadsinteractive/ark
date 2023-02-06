#include "graphics/base/render_layer_snapshot.h"

#include "graphics/base/layer_context.h"

#include "renderer/base/drawing_context.h"
#include "renderer/base/model.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/inf/render_command_composer.h"

namespace ark {

RenderLayerSnapshot::RenderLayerSnapshot(RenderRequest& renderRequest, const sp<RenderLayer::Stub>& stub)
    : _stub(stub), _index_count(0)
{
    DPROFILER_TRACE("Snapshot");

    bool needsReload = _stub->_layer_context->preSnapshot(renderRequest);

    for(auto iter = _stub->_layer_context_list.begin(); iter != _stub->_layer_context_list.end(); )
    {
        const sp<LayerContext>& layerContext = *iter;
        const SafeVar<Boolean>& disposed = layerContext->disposed();
        if((!disposed && layerContext.unique()) || disposed.val())
        {
            iter = _stub->_layer_context_list.erase(iter);
            needsReload = true;
        }
        else
        {
            needsReload = layerContext->preSnapshot(renderRequest) || needsReload;
            ++iter;
        }
    }

    _flag = needsReload ? SNAPSHOT_FLAG_RELOAD : SNAPSHOT_FLAG_DYNAMIC_UPDATE;

    _stub->_layer_context->snapshot(renderRequest, *this);
    for(LayerContext& i : _stub->_layer_context_list)
        i.snapshot(renderRequest, *this);

    _stub->_render_command_composer->postSnapshot(_stub->_render_controller, *this);

    _ubos = _stub->_shader->takeUBOSnapshot(renderRequest);
    _ssbos = _stub->_shader->takeSSBOSnapshot(renderRequest);

    if(_stub->_scissor && _stub->_scissor->update(renderRequest.timestamp()))
        _scissor = Rect(_stub->_scissor->val());

    DPROFILER_LOG("NeedsReload", needsReload);
}

sp<RenderCommand> RenderLayerSnapshot::render(const RenderRequest& renderRequest, const V3& /*position*/)
{
    if(_items.size() > 0 && _stub->_layer_context->visible().val())
        return _stub->_render_command_composer->compose(renderRequest, *this);

    DrawingContext drawingContext(_stub->_shader_bindings, nullptr, std::move(_ubos), std::move(_ssbos));
    return drawingContext.toBindCommand();
}

bool RenderLayerSnapshot::needsReload() const
{
    return _flag == RenderLayerSnapshot::SNAPSHOT_FLAG_RELOAD || _stub->_shader_bindings->vertices().size() == 0;
}

const sp<PipelineInput>& RenderLayerSnapshot::pipelineInput() const
{
    return _stub->_shader->input();
}

void RenderLayerSnapshot::addSnapshot(const LayerContext& lc, Renderable::Snapshot snapshot)
{
    if(!snapshot._model)
        snapshot._model = lc._model_loader->loadModel(snapshot._type);
    _index_count += snapshot._model->indexCount();
    _items.push_back(std::move(snapshot));
}

}
