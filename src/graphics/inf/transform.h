#pragma once

#include "core/base/api.h"
#include "core/inf/variable.h"
#include "core/types/optional_var.h"

#include "graphics/forwarding.h"
#include "graphics/base/mat.h"

namespace ark {

class ARK_API Transform : public Mat4 {
public:
    Transform();
    Transform(sp<Vec4> rotation, sp<Vec3> scale, sp<Vec3> translation);
    ~Transform() override = default;

    class ARK_API Snapshot {
    public:
        Snapshot() = default;
        template<typename T> Snapshot(const T& data)
            : _magic(Type<T>::id()) {
            *reinterpret_cast<T*>(_data) = data;
        }
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Snapshot);

        template<typename T> const T& data() const {
            DCHECK(_magic == Type<T>::id(), "Transform magic mismatch, this Snapshot was taken by a different Transform class");
            const T* data = reinterpret_cast<const T*>(_data);
            return *data;
        }

        alignas(64) uint8_t _data[64];
        TypeId _magic;
    };

    struct Stub {
        OptionalVar<Vec3> _translation;
        OptionalVar<Vec4> _rotation;
        OptionalVar<Vec3> _scale;

        Timestamp _timestamp;

        bool update(uint64_t timestamp) const;
    };

    const OptionalVar<Vec4>& rotation() const;
    const OptionalVar<Vec3>& scale() const;
    const OptionalVar<Vec3>& translation() const;

    [[nodiscard]] virtual Snapshot snapshot() = 0;

    virtual V4 transform(const Snapshot& snapshot, const V4& xyzw) = 0;
    virtual M4 toMatrix(const Snapshot& snapshot) = 0;

    M4 val() override {
        return toMatrix(snapshot());
    }

protected:
    sp<Stub> _stub;
};

}
