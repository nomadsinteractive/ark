#include "graphics/base/layer_context.h"

#include "core/base/constants.h"
#include "core/types/global.h"
#include "core/util/updatable_util.h"

#include "graphics/components/render_object.h"
#include "graphics/base/layer_context_snapshot.h"

#include "renderer/base/pipeline_layout.h"
#include "renderer/base/shader.h"

namespace ark {

LayerContext::LayerContext(sp<Shader> shader, sp<ModelLoader> modelLoader, sp<Vec3> position, sp<Boolean> visible, sp<Boolean> discarded, sp<Varyings> varyings)
    : _shader(std::move(shader)), _model_loader(std::move(modelLoader)), _position(std::move(position)), _visible(std::move(visible), true), _discarded(std::move(discarded), false),
      _varyings(std::move(varyings))
{
}

bool LayerContext::update(const uint64_t timestamp)
{
    return _timestamp.update(timestamp);
}

const sp<Shader>& LayerContext::shader() const
{
    return _shader;
}

const SafeVar<Vec3>& LayerContext::position() const
{
    return _position;
}

void LayerContext::setPosition(sp<Vec3> position)
{
    _position.reset(std::move(position));
    _timestamp.markDirty();
}

SafeVar<Boolean>& LayerContext::visible()
{
    return _visible;
}

const SafeVar<Boolean>& LayerContext::visible() const
{
    return _visible;
}

const SafeVar<Boolean>& LayerContext::discarded() const
{
    return _discarded;
}

const sp<ModelLoader>& LayerContext::modelLoader() const
{
    return _model_loader;
}

void LayerContext::setModelLoader(sp<ModelLoader> modelLoader)
{
    _model_loader = std::move(modelLoader);
}

void LayerContext::pushFront(sp<Renderable> renderable)
{
    _created_push_front.push_back(std::move(renderable));
}

void LayerContext::pushBack(sp<Renderable> renderable)
{
    _created_push_back.push_back(std::move(renderable));
}

void LayerContext::clear()
{
    for(auto& [i, j] : _renderables)
        j = Renderable::RENDERABLE_STATE_DISCARDED;
    _created_push_front.clear();
    _created_push_back.clear();
}

void LayerContext::discard()
{
    _discarded.reset(Global<Constants>()->BOOLEAN_TRUE);
}

const sp<Varyings>& LayerContext::varyings() const
{
    return _varyings;
}

void LayerContext::setVaryings(sp<Varyings> varyings)
{
    _varyings = std::move(varyings);
}

void LayerContext::markDirty()
{
    _timestamp.markDirty();
}

bool LayerContext::processNewCreated()
{
    if(_created_push_front.empty() && _created_push_back.empty())
        return false;

    if(!_created_push_front.empty())
    {
        for(sp<Renderable>& i : _created_push_front)
        {
            addElementState(i.get());
            _renderables.emplace_front(std::move(i), Renderable::State(Renderable::RENDERABLE_STATE_NEW));
        }
    }
    for(sp<Renderable>& i : _created_push_back)
    {
        addElementState(i.get());
        _renderables.emplace_back(std::move(i), Renderable::State(Renderable::RENDERABLE_STATE_NEW));
    }
    _created_push_front.clear();
    _created_push_back.clear();
    return true;
}

LayerContextSnapshot LayerContext::snapshot(RenderLayer renderLayer, const RenderRequest& renderRequest, const PipelineLayout& pipelineLayout)
{
    const bool dirty = UpdatableUtil::update(renderRequest.timestamp(), _position, _visible, _discarded, _varyings);
    if(!_varyings)
        _varyings = sp<Varyings>::make(pipelineLayout);
    return {dirty, _position.val(), _visible.val(), _discarded.val(), _varyings->snapshot(pipelineLayout, renderRequest.allocator()), std::move(renderLayer)};
}

LayerContext::ElementState& LayerContext::addElementState(void* key)
{
    DASSERT(_element_states.find(key) == _element_states.end());
    return _element_states.insert(std::make_pair(key, ElementState{})).first->second;
}

}
