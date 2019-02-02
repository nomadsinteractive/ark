#include "graphics/impl/renderer/renderer_group.h"

#include "core/dom/dom_document.h"
#include "core/impl/builder/builder_by_instance.h"
#include "core/util/log.h"

#include "graphics/base/render_layer.h"
#include "graphics/base/layer.h"

namespace ark {

RendererGroup::~RendererGroup()
{
    LOGD("");
    _items.clear();
}

void RendererGroup::addRenderer(const sp<Renderer>& renderer)
{
    _items.push_back(renderer);
}

void RendererGroup::render(RenderRequest& renderRequest, float x, float y)
{
    for(const sp<Renderer>& i : _items)
        i->render(renderRequest, x, y);
}

void RendererGroup::loadGroup(const document& manifest, BeanFactory& factory, const sp<Scope>& args)
{
    for(const document& i : manifest->children())
    {
        if(i->name() == Constants::Attributes::LAYER)
            addRenderer(factory.ensureDecorated<Renderer, Layer>(i));
        else if(i->name() == Constants::Attributes::RENDER_LAYER)
            addRenderer(factory.ensureDecorated<Renderer, RenderLayer>(i));
        else
            addRenderer(factory.ensure<Renderer>(i, args));
    }
}

RendererGroup::BUILDER::BUILDER(BeanFactory& beanFactory, const document& manifest)
    : _bean_factory(beanFactory), _manifest(manifest)
{
}

sp<Renderer> RendererGroup::BUILDER::build(const sp<Scope>& args)
{
    const sp<RendererGroup> rendererGroup = sp<RendererGroup>::make();
    rendererGroup->loadGroup(_manifest, _bean_factory, args);
    return rendererGroup;
}

}
