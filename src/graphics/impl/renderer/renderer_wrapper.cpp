#include "graphics/impl/renderer/renderer_wrapper.h"

namespace ark {

RendererWrapper::RendererWrapper(const sp<Renderer>& delegate)
    : Delegate<Renderer>(delegate)
{
}

void RendererWrapper::render(RenderRequest& renderRequest, const V3& position)
{
    if(_delegate)
        _delegate->render(renderRequest, position);
}

}
