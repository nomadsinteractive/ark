#include "app/traits/label.h"

#include "core/base/constants.h"
#include "graphics/base/text.h"

namespace ark {

Label::Label(sp<Text> text, sp<LayoutParam> layoutParam)
    : _text(std::move(text)), _layout_param(std::move(layoutParam))
{
}

TypeId Label::onWire(WiringContext& context)
{
    if(sp<Vec3> position = context.getComponent<Vec3>())
        _text->setPosition(std::move(position));
    sp<Boundaries> boundaries = context.getComponent<Boundaries>();
    sp<LayoutParam> layoutParam = context.getComponent<LayoutParam>();
    return TYPE_ID_NONE;
}

}
