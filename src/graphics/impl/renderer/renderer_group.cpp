#include "graphics/impl/renderer/renderer_group.h"

#include "core/util/log.h"

#include "graphics/base/layer.h"
#include "graphics/base/render_layer.h"

namespace ark {

RendererGroup::~RendererGroup()
{
    LOGD("");
}

void RendererGroup::render(RenderRequest& renderRequest, const V3& position)
{
    for(const sp<Renderer>& i : _renderers.update(renderRequest.timestamp()))
        i->render(renderRequest, position);
}

void RendererGroup::addRenderer(sp<Renderer> renderer, const Traits& traits)
{
    ASSERT(renderer);
    add(renderer, renderer.tryCast<Expendable>(), renderer.tryCast<Visibility>());
}

void RendererGroup::add(sp<Renderer> renderer, sp<Boolean> discarded, sp<Boolean> visible)
{
    _renderers.emplace_back(std::move(renderer), std::move(discarded), std::move(visible));
}

}
