#include "graphics/impl/renderer/render_group.h"

#include "core/dom/dom_document.h"
#include "core/util/log.h"

#include "graphics/base/layer.h"
#include "graphics/base/render_layer.h"

namespace ark {

RendererGroup::~RendererGroup()
{
    LOGD("");
}

void RendererGroup::addRenderer(const sp<Renderer>& renderer)
{
    DASSERT(renderer);
    _items.push_back(renderer);
}

void RendererGroup::render(RenderRequest& renderRequest, const V3& position)
{
    for(const sp<Renderer>& i : _items)
        i->render(renderRequest, position);
}

RendererGroup::BUILDER::BUILDER(BeanFactory& beanFactory, const document& manifest)
    : _factory(beanFactory), _manifest(manifest)
{
}

sp<Renderer> RendererGroup::BUILDER::build(const Scope& args)
{
    const sp<RendererGroup> rendererGroup = sp<RendererGroup>::make();
    loadGroup(rendererGroup, args);
    return rendererGroup;
}

void RendererGroup::BUILDER::loadGroup(RendererGroup& rendererGroup, const Scope& args)
{
    for(const document& i : _manifest->children())
    {
        if(i->name() == Constants::Attributes::RENDER_LAYER)
            rendererGroup.addRenderer(_factory.ensureDecorated<Renderer, RenderLayer>(i, args));
        else if(i->name() == Constants::Attributes::LAYER)
            rendererGroup.addRenderer(_factory.ensureDecorated<Renderer, Layer>(i, args));
        else
            rendererGroup.addRenderer(_factory.ensure<Renderer>(i, args));
    }
}

}
