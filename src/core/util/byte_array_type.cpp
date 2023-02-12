#include "core/util/byte_array_type.h"

#include "core/inf/array.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/v4.h"

namespace ark {

namespace {

class ByteArraySliced : public ByteArray {
public:
    ByteArraySliced(sp<ByteArray> byteArray, size_t offset, size_t length)
        : _byte_array(std::move(byteArray)), _slice_offset(offset), _slice_length(length) {
    }

    virtual size_t length() override {
        return _slice_length;
    }

    virtual uint8_t* buf() override {
        return _byte_array->buf() + _slice_offset;
    }

private:
    sp<ByteArray> _byte_array;
    size_t _slice_offset;
    size_t _slice_length;
};

template<typename T> class ByteArrayToVariable : public Variable<T> {
public:
    ByteArrayToVariable(sp<ByteArray> byteArray)
        : _byte_array(std::move(byteArray)) {
        CHECK(_byte_array->size() >= sizeof(T), "Insufficient bytearray (size %d), can not cast to type\"%d\"(size %d)", _byte_array->size(), Type<T>::id(), sizeof(T));
        CHECK_WARN(_byte_array->size() == sizeof(T), "Bytearray (size %d) is longer than type\"%d\"(size %d)", _byte_array->size(), Type<T>::id(), sizeof(T));
    }

    virtual bool update(uint64_t /*timestamp*/) override {
        return true;
    }

    virtual T val() override {
        return *reinterpret_cast<const T*>(_byte_array->buf());
    }

private:
    sp<ByteArray> _byte_array;
};

}

size_t ByteArrayType::nativePtr(const sp<ByteArray>& self)
{
    return reinterpret_cast<size_t>(self->buf());
}

size_t ByteArrayType::len(const sp<ByteArray>& self)
{
    return self->length();
}

uint8_t ByteArrayType::subscribe(const sp<ByteArray>& self, ptrdiff_t index)
{
    CHECK(std::abs(index) <= self->length() && index != self->length(), "Index(%d) out of bounds, array size: %zd", index, self->length());
    return index >= 0 ? self->at(index) : self->at(self->length() + index);
}

sp<ByteArray> ByteArrayType::subscribe(sp<ByteArray> self, const Slice& slice)
{
    Slice adjusted = slice.adjustIndices(self->length());
    CHECK(adjusted.begin() < adjusted.end() && adjusted.begin() >= 0 && adjusted.end() <= static_cast<ptrdiff_t>(self->length()), "Illegal slice(%d, %d)", slice.begin(), slice.end());
    CHECK(adjusted.step() == 1, "Non-continuous slicing is not supported");
    return sp<ByteArraySliced>::make(std::move(self), adjusted.begin(), adjusted.end() - adjusted.begin());
}

Span ByteArrayType::toBytes(const sp<ByteArray>& self)
{
    return Span(reinterpret_cast<const char*>(self->buf()), self->size());
}

sp<Integer> ByteArrayType::toInteger(sp<ByteArray> self)
{
    return sp<ByteArrayToVariable<int32_t>>::make(std::move(self));
}

sp<Numeric> ByteArrayType::toNumeric(sp<ByteArray> self)
{
    return sp<ByteArrayToVariable<float>>::make(std::move(self));
}

sp<Vec2> ByteArrayType::toVec2(sp<ByteArray> self)
{
    return sp<ByteArrayToVariable<V2>>::make(std::move(self));
}

sp<Vec3> ByteArrayType::toVec3(sp<ByteArray> self)
{
    return sp<ByteArrayToVariable<V3>>::make(std::move(self));
}

sp<Vec4> ByteArrayType::toVec4(sp<ByteArray> self)
{
    return sp<ByteArrayToVariable<V4>>::make(std::move(self));
}

}
