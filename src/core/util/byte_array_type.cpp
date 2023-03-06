#include "core/util/byte_array_type.h"

#include "core/inf/array.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/v4.h"

namespace ark {

namespace {

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
