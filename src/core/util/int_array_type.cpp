#include "core/util/int_array_type.h"

#include "core/base/wrapper.h"
#include "core/inf/array.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

namespace {

template<typename T> class ArraySliced : public Array<T> {
public:
    ArraySliced(sp<Array<T>> array, size_t offset, size_t length)
        : _array(std::move(array)), _slice_offset(offset), _slice_length(length) {
    }

    virtual size_t length() override {
        return _slice_length;
    }

    virtual T* buf() override {
        return _array->buf() + _slice_offset;
    }

private:
    sp<Array<T>> _array;
    size_t _slice_offset;
    size_t _slice_length;

};

template<typename T> class ArrayWrapper : public Wrapper<Array<T>>, public Array<T>, Implements<ArrayWrapper<T>, Array<T>> {
public:
    ArrayWrapper(sp<Array<T>> delegate)
        : Wrapper<Array<T>>(std::move(delegate)) {
    }

    virtual size_t length() override {
        return this->_wrapped->length();
    }

    virtual T* buf() override {
        return this->_wrapped->buf();
    }

};


}

sp<IntArray> IntArrayType::create(size_t length, int32_t fill)
{
    return sp<IntArray::Vector>::make(std::vector<int32_t>(length, fill));
}

sp<IntArray> IntArrayType::wrap(sp<IntArray> self)
{
    return sp<ArrayWrapper<int32_t>>::make(std::move(self));
}

void IntArrayType::reset(const sp<IntArray>& self, sp<IntArray> other)
{
    sp<ArrayWrapper<int32_t>> wrapper = self.as<ArrayWrapper<int32_t>>();
    ASSERT(wrapper);
    wrapper->reset(std::move(other));
}

size_t IntArrayType::nativePtr(const sp<IntArray>& self)
{
    return reinterpret_cast<size_t>(self->buf());
}

size_t IntArrayType::len(const sp<IntArray>& self)
{
    return self->length();
}

int32_t IntArrayType::subscribe(const sp<IntArray>& self, ptrdiff_t index)
{
    return *getSubscriptionPtr(self, index);
}

sp<IntArray> IntArrayType::subscribe(sp<IntArray> self, const Slice& slice)
{
    Slice adjusted = slice.adjustIndices(self->length());
    CHECK(adjusted.begin() < adjusted.end() && adjusted.begin() >= 0 && adjusted.end() <= static_cast<ptrdiff_t>(self->length()), "Illegal slice(%d, %d)", slice.begin(), slice.end());
    CHECK(adjusted.step() == 1, "Non-continuous slicing is not supported");
    return sp<ArraySliced<int32_t>>::make(std::move(self), adjusted.begin(), adjusted.end() - adjusted.begin());
}

int32_t IntArrayType::setItem(sp<IntArray> self, ptrdiff_t index, int32_t value)
{
    *getSubscriptionPtr(self, index) = value;
    return 0;
}

int32_t IntArrayType::setItem(sp<IntArray> self, const Slice& slice, const std::vector<int32_t>& values)
{
    Slice adjusted = slice.adjustIndices(self->length());
    size_t stepCount = std::abs(adjusted.length() / adjusted.step());
    CHECK(adjusted.step() > 0 ? adjusted.begin() < adjusted.end() && adjusted.begin() >= 0 && adjusted.end() <= static_cast<ptrdiff_t>(self->length())
          : adjusted.begin() > adjusted.end() && adjusted.end() >= -1 && adjusted.begin() <= static_cast<ptrdiff_t>(self->length()), "Illegal slice(%d, %d, %d)", slice.begin(), slice.end(), slice.step());
    CHECK_WARN(stepCount == values.size() || slice.end() == std::numeric_limits<ptrdiff_t>::max(), "Assigning slice a different length array: slice length: %d, values length: %d", adjusted.length(), values.size());
    const size_t maxStepCount = std::min<size_t>(values.size(), stepCount);
    for(size_t i = 0; i < maxStepCount; ++i)
        self->at(i * adjusted.step() + adjusted.begin()) = values.at(i);
    return 0;
}

Span IntArrayType::toBytes(const sp<IntArray>& self)
{
    return Span(reinterpret_cast<const char*>(self->buf()), self->size());
}

int32_t* IntArrayType::getSubscriptionPtr(IntArray& self, ptrdiff_t index)
{
    CHECK(std::abs(index) <= self.length() && index != static_cast<ptrdiff_t>(self.length()), "Subscription index(%d) out of bounds, array size: %zd", index, self.length());
    return self.buf() + (index >= 0 ? index : self.length() + index);
}

}
