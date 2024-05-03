#pragma once

#include "core/forwarding.h"
#include "core/base/slice.h"
#include "core/base/wrapper.h"
#include "core/inf/array.h"
#include "core/types/implements.h"
#include "core/types/optional.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

template<typename T> class ArrayType {
private:

    class ArraySliced : public Array<T> {
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

    class ArrayWrapper : public Wrapper<Array<T>>, public Array<T>, Implements<ArrayWrapper, Array<T>> {
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

public:

    static sp<Array<T>> create(size_t length, T fill = 0) {
        return sp<typename Array<T>::Vector>::make(std::vector<T>(length, fill));
    }

    static sp<Array<T>> wrap(sp<Array<T>> self) {
        return sp<ArrayWrapper>::make(std::move(self));
    }

    static void reset(const sp<Array<T>>& self, sp<Array<T>> other) {
        sp<ArrayWrapper> wrapper = self.template as<ArrayWrapper>();
        ASSERT(wrapper);
        wrapper->reset(std::move(other));
    }

    static size_t nativePtr(const sp<Array<T>>& self) {
        return reinterpret_cast<size_t>(self->buf());
    }

    static size_t len(const sp<Array<T>>& self) {
        return self->length();
    }

    static Optional<T> getItem(const sp<Array<T>>& self, ptrdiff_t index) {
        return index >= 0 && static_cast<size_t>(index) < self->length() ? *getSubscriptionPtr(self, index) : Optional<T>();
    }

    static int32_t setItem(const sp<Array<T>>& self, ptrdiff_t index, T value) {
        *getSubscriptionPtr(self, index) = value;
        return 0;
    }

    static Optional<T> subscribe(const sp<Array<T>>& self, ptrdiff_t index) {
        return getItem(self, index);
    }

    static sp<Array<T>> subscribe(const sp<Array<T>>& self, const Slice& slice) {
        Slice adjusted = slice.adjustIndices(self->length());
        CHECK(adjusted.begin() < adjusted.end() && adjusted.begin() >= 0 && adjusted.end() <= static_cast<ptrdiff_t>(self->length()), "Illegal slice(%d, %d)", slice.begin(), slice.end());
        CHECK(adjusted.step() == 1, "Non-continuous slicing is not supported");
        return sp<ArraySliced>::make(std::move(self), adjusted.begin(), adjusted.end() - adjusted.begin());
    }

    static int32_t subscribeAssign(const sp<Array<T>>& self, ptrdiff_t index, T value) {
        return setItem(self, index, value);
    }

    static int32_t subscribeAssign(const sp<Array<T>>& self, const Slice& slice, const std::vector<T>& values) {
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

    static Span toBytes(const sp<Array<T>>& self) {
        return Span(reinterpret_cast<const char*>(self->buf()), self->size());
    }

    static sp<ByteArray> toByteArray(sp<Array<T>> self) {
        return sp<typename ByteArray::Casted<T>>::make(std::move(self));
    }

private:
    static T* getSubscriptionPtr(Array<T>& self, ptrdiff_t index) {
        CHECK(std::abs(index) <= static_cast<ptrdiff_t>(self.length()) && index != static_cast<ptrdiff_t>(self.length()), "Subscription index(%d) out of bounds, array size: %zd", index, self.length());
        return self.buf() + (index >= 0 ? index : self.length() + index);
    }
};

}
