#include "app/view/button.h"

#include "core/inf/runnable.h"
#include "core/types/null.h"
#include "core/util/documents.h"
#include "core/util/log.h"

#include "graphics/base/frame.h"
#include "graphics/base/size.h"
#include "graphics/base/rect.h"
#include "graphics/base/v3.h"

#include "app/base/event.h"
#include "app/impl/renderer/renderer_with_state.h"
#include "app/impl/layout/gravity_layout.h"
#include "app/inf/layout.h"

namespace ark {

Button::Button(const sp<Renderer>& foreground, const sp<Renderer>& background, const sp<Size>& block, LayoutParam::Gravity gravity)
    : View(block), _foreground(new RendererWithState(foreground, _state)), _background(new RendererWithState(background, _state)), _gravity(gravity)
{
}

Button::~Button()
{
}

void Button::render(RenderRequest& renderRequest, const V3& position)
{
    _background->render(renderRequest, position);
    if(_gravity != LayoutParam::GRAVITY_DEFAULT)
    {
        const sp<Renderer>& fg = _foreground->getRendererByCurrentStatus();
        if(fg)
        {
            const sp<Block> block = fg.as<Block>();
            const sp<Size>& size = block ? static_cast<const sp<Size>&>(block->size()) : sp<Size>::null();
            if(size)
            {
                Rect target = GravityLayout::place(_gravity, _layout_param->size()->widthAsFloat(), _layout_param->size()->heightAsFloat(), size->widthAsFloat(), size->heightAsFloat());
                fg->render(renderRequest, position + V3(target.left(), target.top(), 0));
            }
            else
                fg->render(renderRequest, position);
        }
    }
    else
        _foreground->render(renderRequest, position);
}

void Button::setForeground(View::State status, sp<Renderer> foreground, const sp<Boolean>& enabled)
{
    _foreground->setStateRenderer(status, std::move(foreground), enabled);
}

void Button::setBackground(View::State status, sp<Renderer> background, const sp<Boolean>& enabled)
{
    _background->setStateRenderer(status, std::move(background), enabled);
}

Button::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _factory(factory), _manifest(manifest),
      _foreground(factory.getBuilder<Renderer>(manifest, Constants::Attributes::FOREGROUND)),
      _background(factory.getBuilder<Renderer>(manifest, Constants::Attributes::BACKGROUND)),
      _size(factory.getBuilder<Size>(manifest, Constants::Attributes::SIZE)),
      _gravity(Documents::getAttribute(manifest, Constants::Attributes::GRAVITY))
{
}

sp<Button> Button::BUILDER::build(const Scope& args)
{
    const sp<Renderer> foreground = _foreground->build(args);
    const Frame background = _background->build(args);
    const sp<Size> builtSize = _size->build(args);
    const String style = Documents::getAttribute(_manifest, Constants::Attributes::STYLE);
    const sp<Size> size = builtSize ? builtSize : background.size();
    const sp<Button> button = sp<Button>::make(foreground, background.renderer(), size, _gravity ? Strings::parse<LayoutParam::Gravity>(_gravity) : LayoutParam::GRAVITY_CENTER);
    loadStatus(button, _manifest, _factory, args);
    if(style)
        _factory.decorate<Renderer>(sp<Builder<Renderer>::Prebuilt>::make(button), style)->build(args);
    return button;
}

void Button::BUILDER::loadStatus(const sp<Button>& button, const document& doc, BeanFactory& factory, const Scope& args)
{
    for(const document& node : doc->children("state"))
    {
        View::State status = Documents::ensureAttribute<View::State>(node, Constants::Attributes::NAME);
        sp<Renderer> foreground = factory.build<Renderer>(node, Constants::Attributes::FOREGROUND, args);
        sp<Renderer> background = factory.build<Renderer>(node, Constants::Attributes::BACKGROUND, args);
        const sp<Boolean> enabled = factory.build<Boolean>(node, "enabled", args);
        if(enabled)
            button->addState(status);

        if(foreground)
            button->setForeground(status, std::move(foreground), enabled);
        if(background)
            button->setBackground(status, std::move(background), enabled);
    }
}

Button::BUILDER_IMPL2::BUILDER_IMPL2(BeanFactory& parent, const document& doc)
    : _delegate(parent, doc)
{
}

sp<Renderer> Button::BUILDER_IMPL2::build(const Scope& args)
{
    return _delegate.build(args);
}

}
