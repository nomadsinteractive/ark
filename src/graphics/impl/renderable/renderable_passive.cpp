#include "graphics/impl/renderable/renderable_passive.h"

#include "graphics/base/render_object.h"

namespace ark {

RenderablePassive::RenderablePassive(const sp<RenderObject>& renderObject)
    : _render_object(renderObject), _position_changed(true), _render_requested(false), _visible(true)
{
}

Renderable::Snapshot RenderablePassive::snapshot(const PipelineInput& pipelineInput, const RenderRequest& renderRequest)
{
    Renderable::Snapshot snapshot = _render_object ? _render_object->snapshot(pipelineInput, renderRequest) : Renderable::Snapshot(false);
    snapshot._dirty = snapshot._dirty || _position_changed || (_visible != _render_requested);
    snapshot._visible = _visible = _render_requested;
    snapshot._position += _position;
    _render_requested = false;
    _position_changed = false;
    return snapshot;
}

const sp<RenderObject>& RenderablePassive::renderObject() const
{
    return _render_object;
}

void RenderablePassive::requestUpdate(const V3& position)
{
    _position_changed = _position != position;
    if(_position_changed)
        _position = position;
    _render_requested = true;
}

}
