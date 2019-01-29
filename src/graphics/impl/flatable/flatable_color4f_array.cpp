#include "graphics/impl/flatable/flatable_color4f_array.h"

#include "core/inf/array.h"
#include "core/base/bean_factory.h"

#include "graphics/base/color.h"

namespace ark {

FlatableColor4fArray::FlatableColor4fArray(const sp<Array<Color>>& colorArray)
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

FlatableColor4fArray::BUILDER::BUILDER(BeanFactory& parent, const String& value)
    : _color_array(parent.ensureBuilder<Array<Color>>(value))
{
}

sp<Flatable> FlatableColor4fArray::BUILDER::build(const sp<Scope>& args)
{
    return sp<FlatableColor4fArray>::make(_color_array->build(args));
}

}
