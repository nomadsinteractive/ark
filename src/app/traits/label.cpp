#include "app/traits/label.h"

#include "app/view/view.h"
#include "core/base/constants.h"
#include "core/traits/expendable.h"

#include "graphics/base/text.h"
#include "graphics/base/boundaries.h"
#include "graphics/util/vec3_type.h"

namespace ark {

Label::Label(sp<Text> text, sp<LayoutParam> layoutParam)
    : _text(std::move(text)), _layout_param(std::move(layoutParam))
{
}

TypeId Label::onWire(WiringContext& context)
{
    sp<Vec3> position = context.getComponent<Vec3>();
    if(const sp<Boundaries> boundaries = context.getComponent<Boundaries>(); boundaries && position)
        position = Vec3Type::add(position, boundaries->aabbMin());
    if(position)
        _text->setPosition(std::move(position));
    if(sp<LayoutParam> layoutParam = context.getComponent<LayoutParam>())
        _text->setLayoutParam(std::move(layoutParam));
    else if(const sp<View>& view = context.getComponent<View>())
        _text->setLayoutParam(view->layoutParam());
    _text->show(context.getComponent<Expendable>());
    return TYPE_ID_NONE;
}

}