#include "graphics/impl/renderer/renderer_wrapper.h"

namespace ark {

RendererWrapper::RendererWrapper(const sp<Renderer>& delegate)
    : Wrapper<Renderer>(delegate)
{
}

void RendererWrapper::render(RenderRequest& renderRequest, const V3& position)
{
    if(_wrapped)
        _wrapped->render(renderRequest, position);
}

}
