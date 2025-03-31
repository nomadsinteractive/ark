#include "core/util/byte_array_type.h"

#include "core/inf/array.h"
#include "core/inf/runnable.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/v4.h"

namespace ark {

typedef std::function<void (const Vector<uintptr_t>&)> WatcherCallback;

namespace {

template<typename T> class ByteArrayToVariable final : public Variable<T> {
public:
    ByteArrayToVariable(sp<ByteArray> byteArray)
        : _byte_array(std::move(byteArray))
    {
        CHECK(_byte_array->size() >= sizeof(T), "Insufficient bytearray (size %d), can not cast to type\"%d\"(size %d)", _byte_array->size(), Type<T>::id(), sizeof(T));
        CHECK_WARN(_byte_array->size() == sizeof(T), "Bytearray (size %d) is longer than type\"%d\"(size %d)", _byte_array->size(), Type<T>::id(), sizeof(T));
    }

    bool update(uint64_t timestamp) override
    {
        if(_timestamp.update(timestamp))
            return true;

        const T v = *reinterpret_cast<const T*>(_byte_array->buf());
        const bool dirty = v != _value;
        if(dirty)
        {
            _value = v;
            _timestamp.markDirty();
        }
        return dirty;
    }

    T val() override
    {
        return _value;
    }

private:
    sp<ByteArray> _byte_array;
    Timestamp _timestamp;
    T _value;
};

template<typename T, T CHECK_VALUE> class ByteArrayWatcher final : public Runnable {
public:
    ByteArrayWatcher(sp<ByteArray> byteArray, const size_t stride, const size_t offset, WatcherCallback callback)
        : _byte_array(std::move(byteArray)), _stride(stride), _offset(offset), _callback(std::move(callback))
    {
        const size_t size = _byte_array->length();
        uint8_t* buf = _byte_array->buf();
        CHECK_WARN(size % _stride == 0, "Array size(%zd) doesn't match its stride(%zd)", size, _stride);
        ASSERT(_offset + sizeof(T) <= _stride);
        for(size_t i = _offset; i < size; i += _stride)
            _previous_value.push_back(*reinterpret_cast<T*>(buf + i));
    }

    void run() override
    {
        Vector<uintptr_t> dirty;
        uint8_t* buf = _byte_array->buf();
        for(size_t i = 0; i < _previous_value.size(); ++i) {
            uint8_t* ptr = buf + i * _stride;
            T& preValue = _previous_value[i];
            const T& value = *reinterpret_cast<T*>(ptr + _offset);
            if(value != CHECK_VALUE && value != preValue) {
                preValue = value;
                dirty.push_back(reinterpret_cast<uintptr_t>(ptr));
            }
        }
        if(dirty.size() > 0)
            _callback(dirty);
    }

private:
    sp<ByteArray> _byte_array;
    size_t _stride;
    size_t _offset;
    WatcherCallback _callback;

    Vector<T> _previous_value;
};

}

sp<Integer> ByteArrayType::toInteger(sp<ByteArray> self)
{
    return sp<Integer>::make<ByteArrayToVariable<int32_t>>(std::move(self));
}

sp<Numeric> ByteArrayType::toNumeric(sp<ByteArray> self)
{
    return sp<Numeric>::make<ByteArrayToVariable<float>>(std::move(self));
}

sp<Vec2> ByteArrayType::toVec2(sp<ByteArray> self)
{
    return sp<Vec2>::make<ByteArrayToVariable<V2>>(std::move(self));
}

sp<Vec3> ByteArrayType::toVec3(sp<ByteArray> self)
{
    return sp<Vec3>::make<ByteArrayToVariable<V3>>(std::move(self));
}

sp<Vec4> ByteArrayType::toVec4(sp<ByteArray> self)
{
    return sp<Vec4>::make<ByteArrayToVariable<V4>>(std::move(self));
}

sp<Runnable> ByteArrayType::makeWatcher(sp<ByteArray> self, size_t stride, size_t offset, WatcherCallback callback)
{
    return sp<Runnable>::make<ByteArrayWatcher<int32_t, 0>>(std::move(self), stride, offset, std::move(callback));
}

}
