#include "app/view/view_group.h"

#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/util/log.h"

#include "graphics/base/frame.h"
#include "graphics/base/layer.h"
#include "graphics/base/size.h"

#include "app/base/event.h"
#include "app/inf/layout.h"
#include "app/view/layout_hierarchy.h"

namespace ark {

ViewGroup::ViewGroup(const Frame& background, const sp<Layout>& layout, const sp<LayoutParam>& layoutParam)
    : View(!layoutParam && background ? sp<LayoutParam>::make(background.size()) : layoutParam), _background(background.renderer()), _layout_hierarchy(sp<LayoutHierarchy>::make(layout))
{
    DCHECK(!layout || _layout_param, "Null LayoutParam. This would happen if your ViewGroup has neither background or size defined.");
    if(_layout_param && background)
        if(background.size() != _layout_param->size())
            background.size()->adopt(static_cast<sp<Size>>(_layout_param->size()));
}

ViewGroup::~ViewGroup()
{
    LOGD("");
}

void ViewGroup::addRenderer(const sp<Renderer>& renderer)
{
    _layout_hierarchy->addRenderer(renderer);
}

void ViewGroup::render(RenderRequest& renderRequest, float x, float y)
{
    if(_background)
        _background->render(renderRequest, x, y);

    if(_layout_hierarchy->isLayoutNeeded())
        _layout_hierarchy->doLayout(_layout_param);

    _layout_hierarchy->render(renderRequest, x, y);
}

bool ViewGroup::onEvent(const Event& event, float x, float y)
{
    return _layout_hierarchy->onEvent(event, x, y);
}

ViewGroup::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _factory(factory), _manifest(manifest),
      _layout(factory.getBuilder<Layout>(manifest, Constants::Attributes::LAYOUT)),
      _background(factory.getBuilder<Renderer>(manifest, Constants::Attributes::BACKGROUND)),
      _layout_param(factory.ensureBuilder<LayoutParam>(manifest))
{
}

sp<Renderer> ViewGroup::BUILDER::build(const sp<Scope>& args)
{
    const sp<ViewGroup> viewGroup = sp<ViewGroup>::make(_background->build(args), _layout->build(args), _layout_param->build(args));
    for(const document& i : _manifest->children())
    {
        const String& name = i->name();
        if(name == Constants::Attributes::LAYER)
            viewGroup->addRenderer(_factory.ensure<Layer>(i, args));
        else if(name != Constants::Attributes::BACKGROUND)
            viewGroup->addRenderer(_factory.ensure<Renderer>(i, args));
    }
    return viewGroup;
}

}
