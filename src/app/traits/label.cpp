#include "app/traits/label.h"

namespace ark {

Label::Label(sp<Text> text, sp<LayoutParam> layoutParam)
    : _text(std::move(text)), _layout_param(std::move(layoutParam))
{
}

std::vector<std::pair<TypeId, Box>> Label::onWire(const Traits& components)
{
    return {};
}

}
