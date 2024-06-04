#include "app/traits/label.h"

#include "core/base/constants.h"

namespace ark {

Label::Label(sp<Text> text, sp<LayoutParam> layoutParam)
    : _text(std::move(text)), _layout_param(std::move(layoutParam))
{
}

TypeId Label::onWire(WiringContext& context)
{
    return TYPE_ID_NONE;
}

}
