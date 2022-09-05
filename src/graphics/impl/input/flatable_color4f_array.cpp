#include "graphics/impl/input/flatable_color4f_array.h"

#include "core/inf/array.h"
#include "core/base/bean_factory.h"

#include "graphics/base/color.h"

namespace ark {

InputColor4fv::InputColor4fv(const sp<ark::Array<Color>>& colorArray)
    : _color_array(colorArray)
{
}

void InputColor4fv::flat(void* buf)
{
    Color* colors = _color_array->buf();
    V4* vbuf = reinterpret_cast<V4*>(buf);
    for(uint32_t i = 0; i < _color_array->length(); i++)
        vbuf[i] = colors[i].val();
}

uint32_t InputColor4fv::size()
{
    return _color_array->length() * sizeof(V4);
}

bool InputColor4fv::update(uint64_t timestamp)
{
    bool dirty = false;
    size_t len = _color_array->length();
    Color* color = _color_array->buf();
    for(size_t i = 0; i < len; ++i)
        dirty = color[i].update(timestamp) || dirty;
    return dirty;
}

InputColor4fv::BUILDER::BUILDER(BeanFactory& parent, const String& value)
    : _color_array(parent.ensureBuilder<ark::Array<Color>>(value))
{
}

sp<Input> InputColor4fv::BUILDER::build(const Scope& args)
{
    return sp<InputColor4fv>::make(_color_array->build(args));
}

}
