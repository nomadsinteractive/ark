#include "app/view/button.h"

#include "core/inf/runnable.h"
#include "core/impl/builder/builder_by_instance.h"
#include "core/types/null.h"
#include "core/util/documents.h"
#include "core/util/log.h"

#include "graphics/base/frame.h"
#include "graphics/base/size.h"
#include "graphics/base/rect.h"

#include "app/base/event.h"
#include "app/impl/renderer/renderer_with_state.h"
#include "app/impl/layout/gravity_layout.h"
#include "app/inf/layout.h"

namespace ark {

Button::Button(const sp<Renderer>& foreground, const sp<Renderer>& background, const sp<Size>& block, Gravity gravity)
    : View(block), _foreground(new RendererWithState(foreground)), _background(new RendererWithState(background)), _gravity(gravity)
{
}

Button::~Button()
{
}

void Button::render(RenderRequest& renderRequest, float x, float y)
{
    _background->render(renderRequest, x, y);
    if(_gravity != NONE)
    {
        const sp<Renderer>& fg = _foreground->getRendererByCurrentStatus();
        if(fg)
        {
            const sp<Block> block = fg.as<Block>();
            const sp<Size>& size = block ? static_cast<const sp<Size>&>(block->size()) : sp<Size>::null();
            if(size)
            {
                Rect target = GravityLayout::place(_gravity, _layout_param->size()->width(), _layout_param->size()->height(), size->width(), size->height());
                fg->render(renderRequest, x + target.left(), y + target.top());
            }
            else
                fg->render(renderRequest, x, y);
        }
    }
    else
        _foreground->render(renderRequest, x, y);
}

void Button::setForeground(View::State status, const sp<Renderer>& foreground)
{
    _foreground->setStateRenderer(status, foreground);
}

void Button::setBackground(View::State status, const sp<Renderer>& background)
{
    _background->setStateRenderer(status, background);
}

bool Button::fireOnPush()
{
    _foreground->setStatus(View::STATE_PUSHING);
    _background->setStatus(View::STATE_PUSHING);
    return View::fireOnPush();
}

bool Button::fireOnRelease()
{
    _foreground->setStatus(View::STATE_DEFAULT);
    _background->setStatus(View::STATE_DEFAULT);
    return View::fireOnRelease();
}

Button::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _factory(factory), _manifest(manifest),
      _foreground(factory.getBuilder<Renderer>(manifest, Constants::Attributes::FOREGROUND)),
      _background(factory.getBuilder<Renderer>(manifest, Constants::Attributes::BACKGROUND)),
      _size(factory.getBuilder<Size>(manifest, Constants::Attributes::SIZE)),
      _gravity(Documents::getAttribute(manifest, Constants::Attributes::GRAVITY))
{
}

sp<Button> Button::BUILDER::build(const sp<Scope>& args)
{
    const sp<Renderer> foreground = _foreground->build(args);
    const Frame background = _background->build(args);
    const sp<Size> builtSize = _size->build(args);
    const String style = Documents::getAttribute(_manifest, Constants::Attributes::STYLE);
    const sp<Size> size = builtSize ? builtSize : background.size();
    const sp<Button> button = sp<Button>::make(foreground, background.renderer(), size, _gravity ? Strings::parse<Gravity>(_gravity) : CENTER);
    loadStatus(button, _manifest, _factory, args);
    if(style)
        _factory.decorate<Renderer>(sp<BuilderByInstance<Renderer>>::make(button), style)->build(args);
    return button;
}

void Button::BUILDER::loadStatus(const sp<Button>& button, const document& doc, BeanFactory& factory, const sp<Scope>& args)
{
    for(const document& node : doc->children("state"))
    {
        View::State status = Documents::ensureAttribute<View::State>(node, Constants::Attributes::NAME);
        const sp<Renderer> foreground = factory.build<Renderer>(node, Constants::Attributes::FOREGROUND, args);
        const sp<Renderer> background = factory.build<Renderer>(node, Constants::Attributes::BACKGROUND, args);
        if(foreground)
            button->setForeground(status, foreground);
        if(background)
            button->setBackground(status, background);
    }
}

Button::BUILDER_IMPL2::BUILDER_IMPL2(BeanFactory& parent, const document& doc)
    : _delegate(parent, doc)
{
}

sp<Renderer> Button::BUILDER_IMPL2::build(const sp<Scope>& args)
{
    return _delegate.build(args);
}

}
