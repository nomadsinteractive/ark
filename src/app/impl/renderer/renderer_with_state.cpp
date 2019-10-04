#include "app/impl/renderer/renderer_with_state.h"

#include "core/inf/variable.h"
#include "core/util/math.h"

namespace ark {

RendererWithState::RendererWithState(const sp<Renderer>& def, const sp<View::State>& state)
    : _default(def), _state(state)
{
}

void RendererWithState::render(RenderRequest& renderRequest, const V3& position)
{
    const sp<Renderer>& current = getRendererByCurrentStatus();
    if(current)
        current->render(renderRequest, position);
}

void RendererWithState::setStateRenderer(View::State status, sp<Renderer> renderer, sp<Boolean> enabled)
{
    if(status == View::STATE_DEFAULT)
        _default = std::move(renderer);
    else
    {
        uint32_t idx = Math::log2(static_cast<uint32_t>(status));
        _selectors[idx] = StateRenderer(std::move(renderer), std::move(enabled));
    }
}

const sp<Renderer>& RendererWithState::getRendererByCurrentStatus()
{
    uint32_t s = static_cast<uint32_t>(*_state);
    if(s != View::STATE_DEFAULT)
    {
        for(uint32_t i = 0; i < View::STATE_COUNT - 1; ++i)
        {
            if(s & (1 << i) && _selectors[i]._renderer && (!_selectors[i]._enabled || _selectors[i]._enabled->val()))
                return _selectors[i]._renderer;
        }
    }
    return _default;
}

RendererWithState::StateRenderer::StateRenderer(sp<Renderer> renderer, sp<Boolean> boolean)
    : _renderer(std::move(renderer)), _enabled(std::move(boolean))
{
}

}
