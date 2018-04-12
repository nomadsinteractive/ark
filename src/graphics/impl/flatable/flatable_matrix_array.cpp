#include "graphics/impl/flatable/flatable_matrix_array.h"

#include "core/inf/array.h"
#include "core/base/bean_factory.h"

namespace ark {

FlatableMatrixArray::FlatableMatrixArray(const sp<Array<Matrix>>& matrixArray)
    : _matrix_array(matrixArray)
{
}

void FlatableMatrixArray::flat(void* buf)
{
    memcpy(buf, _matrix_array->buf(), _matrix_array->length() * 16);
}

uint32_t FlatableMatrixArray::size()
{
    return _matrix_array->length() * 16;
}

uint32_t FlatableMatrixArray::length()
{
    return _matrix_array->length();
}

FlatableMatrixArray::BUILDER::BUILDER(BeanFactory& parent, const String& value)
    : _matrix_array(parent.ensureBuilder<Array<Matrix>>(value))
{
}

sp<Flatable> FlatableMatrixArray::BUILDER::build(const sp<Scope>& args)
{
    return sp<FlatableMatrixArray>::make(_matrix_array->build(args));
}

}
