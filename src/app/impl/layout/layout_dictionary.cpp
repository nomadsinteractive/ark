#include "app/impl/layout/layout_dictionary.h"

#include "core/util/strings.h"

#include "app/impl/layout/gravity_layout.h"
#include "app/impl/layout/frame_layout.h"
#include "app/impl/layout/horizontal_layout.h"
#include "app/impl/layout/vertical_layout.h"

namespace ark {

LayoutDictionary::DICTIONARY::DICTIONARY(BeanFactory& /*factory*/, const String& value)
{
    String gravity;
    if(Strings::splitFunction(value, _layout, gravity))
        _gravity = Strings::parse<View::Gravity>(gravity);
    else
    {
        _layout = value;
        _gravity = View::GRAVITY_DEFAULT;
    }
    DCHECK(_gravity != View::NONE, "Invaild gravity value: \"%s\"", value.c_str());
}

sp<Layout> LayoutDictionary::DICTIONARY::build(const Scope& /*args*/)
{
    if(_layout == "vertical")
        return sp<VerticalLayout>::make(_gravity);
    if(_layout == "horizontal")
        return sp<HorizontalLayout>::make(_gravity);
    if(_layout == "gravity")
        return sp<GravityLayout>::make(_gravity);
    if(_layout == "frame")
        return sp<FrameLayout>::make();
    DFATAL("Invaild layout value: \"%s\", possible values are [\"vertical\", \"horizontal\", \"gravity\", \"frame\"]", _layout.c_str());
    return nullptr;
}

}
