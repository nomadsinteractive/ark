#include "graphics/impl/flatable/flatable_color3b.h"

#include "core/inf/variable.h"
#include "core/inf/variable.h"
#include "core/base/bean_factory.h"

#include "graphics/base/v4.h"

namespace ark {

FlatableColor3b::FlatableColor3b(const sp<Vec4>& color)
    : _color(color)
{
}

void FlatableColor3b::flat(void* buf)
{
    const V4 color = _color->val();

    uint8_t* ptr = reinterpret_cast<uint8_t*>(buf);
    ptr[0] = static_cast<uint8_t>(color.x() * 255);
    ptr[1] = static_cast<uint8_t>(color.y() * 255);
    ptr[2] = static_cast<uint8_t>(color.z() * 255);
}

uint32_t FlatableColor3b::size()
{
    return 3;
}

uint32_t FlatableColor3b::length()
{
    return 1;
}

FlatableColor3b::BUILDER::BUILDER(BeanFactory& parent, const String& value)
    : _color(parent.ensureBuilder<Vec4>(value))
{
}

sp<Flatable> FlatableColor3b::BUILDER::build(const sp<Scope>& args)
{
    return sp<FlatableColor3b>::make(_color->build(args));
}

}
