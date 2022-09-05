#include "graphics/impl/input/input_color3b.h"

#include "core/inf/variable.h"
#include "core/inf/variable.h"
#include "core/base/bean_factory.h"

#include "graphics/base/v4.h"

namespace ark {

InputColor3b::InputColor3b(const sp<Vec4>& color)
    : _color(color)
{
}

void InputColor3b::flat(void* buf)
{
    const V4 color = _color->val();

    uint8_t* ptr = reinterpret_cast<uint8_t*>(buf);
    ptr[0] = static_cast<uint8_t>(color.x() * 255);
    ptr[1] = static_cast<uint8_t>(color.y() * 255);
    ptr[2] = static_cast<uint8_t>(color.z() * 255);
}

uint32_t InputColor3b::size()
{
    return 3;
}

bool InputColor3b::update(uint64_t timestamp)
{
    return _color->update(timestamp);
}

InputColor3b::BUILDER::BUILDER(BeanFactory& parent, const String& value)
    : _color(parent.ensureBuilder<Vec4>(value))
{
}

sp<Input> InputColor3b::BUILDER::build(const Scope& args)
{
    return sp<InputColor3b>::make(_color->build(args));
}

}
