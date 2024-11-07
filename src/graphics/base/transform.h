#pragma once

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"
#include "core/types/safe_var.h"

#include "graphics/forwarding.h"
#include "graphics/base/mat.h"
#include "graphics/base/v3.h"

namespace ark {

//[[script::bindings::extends(Mat4)]]
class ARK_API Transform : public Mat4 {
public:
    struct Stub {
        SafeVar<Vec4> _rotation;
        SafeVar<Vec3> _scale;
        SafeVar<Vec3> _translation;
    };

    class Snapshot;

    class Delegate {
    public:
        virtual ~Delegate() = default;

        virtual bool update(const Stub& transform, uint64_t timestamp) = 0;
        virtual void snapshot(const Stub& transform, Snapshot& snapshot) const = 0;

        virtual V3 transform(const Snapshot& snapshot, const V3& position) const = 0;
        virtual M4 toMatrix(const Snapshot& snapshot) const = 0;
    };

//  [[script::bindings::enumeration]]
    enum Type {
        TYPE_NONE,
        TYPE_LINEAR_2D,
        TYPE_LINEAR_3D,
        TYPE_DELEGATED
    };

//  [[script::bindings::auto]]
    Transform(sp<Vec4> rotation = nullptr, sp<Vec3> scale = nullptr, sp<Vec3> translation = nullptr, Transform::Type type = Transform::TYPE_NONE);
    Transform(sp<Delegate> delegate);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Transform);

    class ARK_API Snapshot {
    public:
        Snapshot(const Transform& transform);
        Snapshot() = default;
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Snapshot);

        M4 toMatrix() const;

        V3 transform(const V3& p) const;

        template<typename T> T& makeData() {
            T* data = reinterpret_cast<T*>(&_data);
            _magic = ark::Type<T>::id();
            return *data;
        }

        template<typename T> const T& getData() const {
            DCHECK(_magic == ark::Type<T>::id(), "Transform magic mismatch, this Snapshot was taken by a different transform delegate");
            const T* data = reinterpret_cast<const T*>(&_data);
            return *data;
        }

        alignas(64) M4 _data;
        sp<Delegate> _delegate;
        TypeId _magic;
    };

    Snapshot snapshot() const;

    bool update(uint64_t timestamp) override;
    M4 val() override;

//  [[script::bindings::property]]
    const SafeVar<Vec4>& rotation() const;
//  [[script::bindings::property]]
    void setRotation(sp<Vec4> rotation);

//  [[script::bindings::property]]
    const SafeVar<Vec3>& scale() const;
//  [[script::bindings::property]]
    void setScale(sp<Vec3> scale);

//  [[script::bindings::property]]
    const SafeVar<Vec3>& translation() const;
//  [[script::bindings::property]]
    void setTranslation(sp<Vec3> translation);

    void reset(sp<Mat4> transform);

private:
    sp<Delegate> makeDelegate() const;
    sp<Delegate> makeTransformLinear() const;
    sp<Delegate> makeTransformSimple() const;

    void doUpdateDelegate();

private:
    Type _type;
    sp<Stub> _stub;
    sp<Delegate> _delegate;

    SafeVar<Mat4> _matrix;

    friend class TransformSimple2D;
    friend class TransformSimple3D;
    friend class TransformLinear2D;
    friend class TransformLinear3D;

    class TransformToMat4;
};

}
