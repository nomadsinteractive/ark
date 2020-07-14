#include "graphics/impl/flatable/flatable_mat4f_array.h"

#include "core/inf/array.h"
#include "core/inf/variable.h"
#include "core/base/bean_factory.h"

#include "graphics/base/mat.h"

namespace ark {

FlatableMat4fArray::FlatableMat4fArray(array<sp<Mat4> > array)
    : _array(std::move(array))
{
}

void FlatableMat4fArray::flat(void* buf)
{
    M4* vbuf = reinterpret_cast<M4*>(buf);
    uint32_t idx = 0;
    for(const sp<Mat4>& i : *_array)
        vbuf[idx++] = i->val();
}

uint32_t FlatableMat4fArray::size()
{
    return _array->length() * sizeof(M4);
}

bool FlatableMat4fArray::update(uint64_t timestamp)
{
    bool dirty = false;
    for(const sp<Mat4>& i : *_array)
        dirty = i->update(timestamp) || dirty;
    return dirty;
}

FlatableMat4fArray::BUILDER::BUILDER(BeanFactory& factory, const String& value)
    : _array(factory.ensureBuilder<ark::Array<sp<Mat4>>>(value))
{
}

sp<Flatable> FlatableMat4fArray::BUILDER::build(const Scope& args)
{
    return sp<FlatableMat4fArray>::make(_array->build(args));
}

}
