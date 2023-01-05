#include "app/view/view_group.h"

#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/util/holder_util.h"
#include "core/util/log.h"

#include "graphics/base/layer.h"
#include "graphics/base/render_request.h"
#include "graphics/base/size.h"
#include "graphics/util/renderer_type.h"

#include "app/base/event.h"
#include "app/inf/layout.h"

namespace ark {

ViewGroup::ViewGroup(sp<Renderer> background, sp<Layout> layout, sp<LayoutV3> layoutV3, sp<LayoutParam> layoutParam)
    : View(!layoutParam && background ? sp<LayoutParam>::make(RendererType::size(background)) : std::move(layoutParam)), _background(std::move(background)), _layout_hierarchy(sp<LayoutHierarchy>::make(std::move(layout), std::move(layoutV3)))
{
    CHECK(!_layout_hierarchy->_layout || _layout_param, "Null LayoutParam. This would happen if your ViewGroup has neither background or size defined.");
//    if(_layout_param && background)
//        if(background.size() != _layout_param->size())
//            background.size()->set(static_cast<sp<Size>>(_layout_param->size()));
}

ViewGroup::~ViewGroup()
{
    LOGD("");
}

void ViewGroup::addRenderer(const sp<Renderer>& renderer)
{
    _layout_hierarchy->addRenderer(renderer);
}

void ViewGroup::render(RenderRequest& renderRequest, const V3& position)
{
    if(_background)
        _background->render(renderRequest, position);

    _layout_hierarchy->updateLayout(_layout_param);
    _layout_hierarchy->render(renderRequest, position);
}

void ViewGroup::traverse(const Holder::Visitor& visitor)
{
    View::traverse(visitor);
    HolderUtil::visit(_background, visitor);
    _layout_hierarchy->traverse(visitor);
}

bool ViewGroup::onEvent(const Event& event, float x, float y, bool ptin)
{
    return _layout_hierarchy->onEvent(event, x, y) || dispatchEvent(event, ptin);
}

void ViewGroup::updateLayout() const
{
    _layout_hierarchy->updateLayout(_layout_param);
}

sp<LayoutV3::Node> ViewGroup::makeLayoutNode() const
{
    return _layout_hierarchy->makeLayoutNode(_layout_param);
}

ViewGroup::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _factory(factory), _manifest(manifest), _layout(factory.getBuilder<Layout>(manifest, Constants::Attributes::LAYOUT)), _layout_v3(factory.getBuilder<LayoutV3>(manifest, "layout-v3")),
      _background(factory.getBuilder<Renderer>(manifest, Constants::Attributes::BACKGROUND)), _layout_param(factory.ensureBuilder<LayoutParam>(manifest))
{
}

sp<Renderer> ViewGroup::BUILDER::build(const Scope& args)
{
    const sp<ViewGroup> viewGroup = sp<ViewGroup>::make(_background->build(args), _layout->build(args), _layout_v3->build(args), _layout_param->build(args));
    for(const document& i : _manifest->children())
    {
        const String& name = i->name();
        if(name == Constants::Attributes::LAYER)
            viewGroup->addRenderer(_factory.ensureDecorated<Renderer, Layer>(i, args));
        else if(name != Constants::Attributes::BACKGROUND)
            viewGroup->addRenderer(_factory.ensure<Renderer>(i, args));
    }
    return viewGroup;
}

}
