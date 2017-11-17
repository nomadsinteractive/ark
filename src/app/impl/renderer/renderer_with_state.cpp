#include "app/impl/renderer/renderer_with_state.h"

namespace ark {

RendererWithState::RendererWithState(const sp<Renderer>& def)
    : _state(View::STATE_DEFAULT), _default(def)
{
}

void RendererWithState::render(RenderRequest& renderRequest, float x, float y)
{
    const sp<Renderer>& current = getRendererByCurrentStatus();
    if(current)
        current->render(renderRequest, x, y);
}

View::State RendererWithState::status() const
{
    return _state;
}

void RendererWithState::setStatus(View::State status)
{
    _state = status;
}

void RendererWithState::setStateRenderer(View::State status, const sp<Renderer> &drawable)
{
    _selector[status] = drawable;
}

const sp<Renderer>& RendererWithState::getRendererByCurrentStatus()
{
    if(_state != View::STATE_DEFAULT)
    {
        auto iterator = _selector.find(_state);
        if(iterator != _selector.end())
            return iterator->second;
    }
    return _default;
}

}
