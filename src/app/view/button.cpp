#include "app/view/button.h"

#include "core/inf/runnable.h"
#include "core/types/null.h"
#include "core/util/documents.h"
#include "core/util/log.h"

#include "graphics/base/frame.h"
#include "graphics/base/size.h"
#include "graphics/base/rect.h"
#include "graphics/base/v3.h"
#include "graphics/util/renderer_type.h"

#include "app/base/event.h"
#include "app/impl/renderer/renderer_with_state.h"
#include "app/inf/layout.h"
#include "app/util/layout_util.h"

namespace ark {

Button::Button(sp<Renderer> foreground, sp<Renderer> background, sp<Size> size, LayoutParam::Gravity gravity)
    : View(std::move(size)), _foreground(new RendererWithState(std::move(foreground), _state)), _background(new RendererWithState(std::move(background), _state)), _gravity(gravity)
{
}

Button::~Button()
{
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
    sp<Renderer> foreground = _foreground->build(args);
    sp<Renderer> background = _background->build(args);
    sp<Size> builtSize = _size->build(args);
    sp<Size> size = builtSize ? builtSize : static_cast<const sp<Size>&>(RendererType::size(background));
    sp<Button> button = sp<Button>::make(std::move(foreground), std::move(background), std::move(size), _gravity ? Strings::eval<LayoutParam::Gravity>(_gravity) : LayoutParam::GRAVITY_CENTER);
    return button;
}

}
