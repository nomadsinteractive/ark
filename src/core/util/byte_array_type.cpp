#include "core/util/byte_array_type.h"

#include "core/inf/array.h"
#include "core/inf/runnable.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/v4.h"

namespace ark {

namespace {

template<typename T> class ByteArrayToVariable final : public Variable<T> {
public:
    ByteArrayToVariable(sp<ByteArray> byteArray)
        : _byte_array(std::move(byteArray)), _ptr(reinterpret_cast<const T*>(_byte_array->buf()))
    {
        CHECK(_byte_array->size() >= sizeof(T), "Insufficient bytearray (size %d), can not cast to type\"%d\"(size %d)", _byte_array->size(), Type<T>::id(), sizeof(T));
        CHECK_WARN(_byte_array->size() == sizeof(T), "Bytearray (size %d) is longer than type\"%d\"(size %d)", _byte_array->size(), Type<T>::id(), sizeof(T));
    }

    bool update(const uint64_t timestamp) override
    {
        if(_timestamp.update(timestamp))
            return true;

        const T v = *_ptr;
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
        return *_ptr;
    }

private:
    sp<ByteArray> _byte_array;
    const T* _ptr;

    Timestamp _timestamp;
    T _value;
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

}
