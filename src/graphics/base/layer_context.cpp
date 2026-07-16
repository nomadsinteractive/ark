#include "graphics/base/layer_context.h"

#include <ranges>

#include "core/base/constants.h"
#include "core/types/global.h"
#include "core/util/updatable_util.h"

#include "graphics/components/render_object.h"
#include "graphics/base/layer_context_snapshot.h"

#include "renderer/base/pipeline_layout.h"
#include "renderer/base/shader.h"

namespace ark {

LayerContext::LayerContext(sp<Shader> shader, sp<ModelLoader> modelLoader, sp<Vec3> position, sp<Boolean> visible, sp<Boolean> discarded, sp<Varyings> varyings, sp<Updatable> updatable)
    : _shader(std::move(shader)), _model_loader(std::move(modelLoader)), _position(std::move(position)), _visible(std::move(visible), true), _discarded(std::move(discarded), false),
      _varyings(std::move(varyings)), _updatable(std::move(updatable)), _tick(std::numeric_limits<uint32_t>::max()), _layer_dirty(false), _instances_dirty(false)
{
}

const sp<Shader>& LayerContext::shader() const
{
    return _shader;
}

const OptionalVar<Vec3>& LayerContext::position() const
{
    return _position;
}

void LayerContext::setPosition(sp<Vec3> position)
{
    _position.reset(std::move(position));
    _timestamp.markDirty();
}

OptionalVar<Boolean>& LayerContext::visible()
{
    return _visible;
}

const OptionalVar<Boolean>& LayerContext::visible() const
{
    return _visible;
}

const OptionalVar<Boolean>& LayerContext::discarded() const
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

void LayerContext::pushBack(sp<Renderable> renderable)
{
    _newly_created_renderables.push_back(std::move(renderable));
}

void LayerContext::clear()
{
    for(auto& j : std::views::values(_renderables))
        j = Renderable::RENDERABLE_STATE_DISCARDED;
    _newly_created_renderables.clear();
}

void LayerContext::discard()
{
    _discarded.reset(Global<Constants>()->BOOLEAN_TRUE);
}

void LayerContext::markDirty()
{
    _timestamp.markDirty();
}

bool LayerContext::processNewCreated()
{
    if(_newly_created_renderables.empty())
        return false;

    for(sp<Renderable>& i : _newly_created_renderables)
    {
        addElementState(i.get());
        _renderables.emplace_back(std::move(i), Renderable::State(Renderable::RENDERABLE_STATE_NEW));
    }
    _newly_created_renderables.clear();
    return true;
}

void LayerContext::updateFrameState(const RenderRequest& renderRequest)
{
    // A LayerContext may be shared by multiple RenderLayers. The state mutating operations below are
    // single-consumer(newly created draining, dirty timestamp updating, discard erasing), so they run
    // once per frame, and their outcome is cached for every RenderLayerSnapshot taken within this frame.
    if(_tick == renderRequest.tick())
        return;

    _tick = renderRequest.tick();
    _layer_dirty = UpdatableUtil::update(renderRequest.tick(), _updatable, _position, _visible, _discarded, _varyings, _timestamp);
    _instances_dirty = processNewCreated();
    _renderable_states.clear();
    _discarded_element_states.clear();

    const bool layerVisible = _visible.val();
    for(auto iter = _renderables.begin(); iter != _renderables.end(); )
    {
        Renderable& renderable = iter->first;
        Renderable::State& s = iter->second;
        const Renderable::State newState = renderable.updateState(renderRequest);
        const bool visibilityChanged = s.contains(Renderable::RENDERABLE_STATE_VISIBLE) != newState.contains(Renderable::RENDERABLE_STATE_VISIBLE);
        s = {newState, static_cast<Renderable::StateBits>(s.bits() & (Renderable::RENDERABLE_STATE_NEW | Renderable::RENDERABLE_STATE_DISCARDED))};
        if(Renderable::State state = s; state.contains(Renderable::RENDERABLE_STATE_DISCARDED))
        {
            _instances_dirty = true;
            if(const auto fiter = _element_states.find(&renderable); fiter != _element_states.end())
            {
                _discarded_element_states.push_back(std::move(fiter->second));
                _element_states.erase(fiter);
            }
            iter = _renderables.erase(iter);
        }
        else
        {
            if(visibilityChanged)
                state.set(Renderable::RENDERABLE_STATE_NEW, true);
            if(state.contains(Renderable::RENDERABLE_STATE_VISIBLE))
                state.set(Renderable::RENDERABLE_STATE_VISIBLE, layerVisible);
            if(s.contains(Renderable::RENDERABLE_STATE_NEW))
            {
                state.set(Renderable::RENDERABLE_STATE_DIRTY, true);
                s.set(Renderable::RENDERABLE_STATE_NEW, false);
            }

            const auto fiter = _element_states.find(&renderable);
            DASSERT(fiter != _element_states.end());
            _renderable_states.emplace_back(&renderable, &fiter->second, state);
            ++iter;
        }
    }
}

LayerContextSnapshot LayerContext::snapshot(const RenderRequest& renderRequest, const PipelineLayout& pipelineLayout)
{
    updateFrameState(renderRequest);
    const sp<Varyings>& varyings = _varyings ? _varyings : pipelineLayout.defaultVaryings();
    return {_position.val(), _layer_dirty, _visible.val(), _discarded.val(), varyings->snapshot(pipelineLayout, renderRequest.allocator())};
}

LayerContext::ElementState& LayerContext::addElementState(void* key)
{
    DASSERT(!_element_states.contains(key));
    return _element_states.insert(std::make_pair(key, ElementState{})).first->second;
}

}
