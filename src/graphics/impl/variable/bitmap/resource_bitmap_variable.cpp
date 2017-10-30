#include "graphics/impl/variable/bitmap/resource_bitmap_variable.h"

#include "core/inf/dictionary.h"

#include "graphics/base/bitmap.h"

namespace ark {

ResourceBitmapVariable::ResourceBitmapVariable(const sp<Dictionary<bitmap>>& bitmapResource, const String& name)
    : _bitmap_resource(bitmapResource), _name(name)
{
}

bitmap ResourceBitmapVariable::val()
{
    return _bitmap_resource->get(_name);
}

}
