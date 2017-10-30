#include "core/impl/range/integer_array.h"

#include "core/inf/array.h"
#include "core/util/strings.h"

namespace ark {

IntegerArray::IntegerArray(const sp<Array<int32_t>>& array)
    : _array(array), _iter(0)
{
}

uint32_t IntegerArray::length()
{
    return _array->length();
}

int32_t* IntegerArray::array()
{
    return _array->array();
}

bool IntegerArray::hasNext()
{
    return _iter < _array->length();
}

int32_t IntegerArray::next()
{
    DCHECK(_iter < _array->length(), "Index out of bounds, _iter = %d, length = %d", _iter, _array->length());
    return _array->array()[_iter++];
}

IntegerArray::DICTIONARY_IMPL1::DICTIONARY_IMPL1(const String& value)
    : _array(Strings::toArray<int32_t>(value, '[', ']'))
{
}

sp<Array<int32_t>> IntegerArray::DICTIONARY_IMPL1::build(const sp<Scope>& /*args*/)
{
    return sp<Array<int32_t>>::adopt(new IntegerArray(_array));
}

sp<IntegerArray> IntegerArray::DICTIONARY_IMPL1::buildIntegerArray(const sp<Scope>& /*args*/) const
{
    return sp<IntegerArray>::make(_array);
}

IntegerArray::DICTIONARY_IMPL2::DICTIONARY_IMPL2(const String& value)
    : _impl(value)
{
}

sp<Range> IntegerArray::DICTIONARY_IMPL2::build(const sp<Scope>& args)
{
    return _impl.buildIntegerArray(args);
}

}
