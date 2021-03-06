#include "graphics/impl/input/flatable_color4f_array.h"

#include "core/inf/array.h"
#include "core/base/bean_factory.h"

#include "graphics/base/color.h"

namespace ark {

FlatableColor4fArray::FlatableColor4fArray(const sp<ark::Array<Color>>& colorArray)
    : _color_array(colorArray)
{
}

void FlatableColor4fArray::flat(void* buf)
{
    Color* colors = _color_array->buf();
    V4* vbuf = reinterpret_cast<V4*>(buf);
    for(uint32_t i = 0; i < _color_array->length(); i++)
        vbuf[i] = colors[i].val();
}

uint32_t FlatableColor4fArray::size()
{
    return _color_array->length() * sizeof(V4);
}

bool FlatableColor4fArray::update(uint64_t timestamp)
{
    bool dirty = false;
    size_t len = _color_array->length();
    Color* color = _color_array->buf();
    for(size_t i = 0; i < len; ++i)
        dirty = color[i].update(timestamp) || dirty;
    return dirty;
}

FlatableColor4fArray::BUILDER::BUILDER(BeanFactory& parent, const String& value)
    : _color_array(parent.ensureBuilder<ark::Array<Color>>(value))
{
}

sp<Input> FlatableColor4fArray::BUILDER::build(const Scope& args)
{
    return sp<FlatableColor4fArray>::make(_color_array->build(args));
}

}
