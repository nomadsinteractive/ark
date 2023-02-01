#include "graphics/impl/renderable/renderable_passive.h"

#include "graphics/base/render_object.h"

namespace ark {

RenderablePassive::RenderablePassive(sp<RenderObject> renderObject)
    : _render_object(std::move(renderObject)), _position_changed(true), _render_requested(false), _visible(true)
{
}

Renderable::StateBits RenderablePassive::updateState(const RenderRequest& renderRequest)
{
    return Renderable::RENDERABLE_STATE_NONE;

    Renderable::StateBits state = _render_object ? _render_object->updateState(renderRequest) : Renderable::RENDERABLE_STATE_NONE;
    bool dirty = (state & Renderable::RENDERABLE_STATE_DIRTY) || _position_changed || (_visible != _render_requested);
    _visible = (state & Renderable::RENDERABLE_STATE_VISIBLE) && _render_requested;
    return Renderable::toState(state & Renderable::RENDERABLE_STATE_DISPOSED, dirty, _visible);
}

Renderable::Snapshot RenderablePassive::snapshot(const PipelineInput& pipelineInput, const RenderRequest& renderRequest, const V3& postTranslate, StateBits state)
{
    Renderable::Snapshot snapshot = _render_object ? _render_object->snapshot(pipelineInput, renderRequest, postTranslate + _position, state) : Renderable::Snapshot(state);
    _render_requested = false;
    _position_changed = false;
    return snapshot;
}

void RenderablePassive::requestUpdate(const V3& position)
{
    _position_changed = _position != position;
    if(_position_changed)
        _position = position;
    _render_requested = true;
}

}
