#ifndef ARK_GRAPHICS_BASE_TRANSFORM_H_
#define ARK_GRAPHICS_BASE_TRANSFORM_H_

#include "core/base/api.h"
#include "core/base/timestamp.h"
#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/inf/holder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"
#include "core/types/safe_ptr.h"
#include "core/types/safe_var.h"

#include "graphics/forwarding.h"
#include "graphics/base/mat.h"
#include "graphics/base/rotation.h"
#include "graphics/base/v3.h"

namespace ark {

//[[script::bindings::holder]]
class ARK_API Transform : public Holder {
public:
    class Snapshot;

    class Delegate {
    public:
        virtual ~Delegate() = default;

        virtual void snapshot(const Transform& transform, const V3& postTranslate, Snapshot& snapshot) const = 0;

        virtual V3 transform(const Snapshot& snapshot, const V3& position) const = 0;
        virtual M4 toMatrix(const Snapshot& snapshot) const = 0;
    };

public:
//  [[script::bindings::enumeration]]
    enum Type {
        TYPE_LINEAR_2D,
        TYPE_LINEAR_3D,
        TYPE_DELEGATED
    };

//  [[script::bindings::auto]]
    Transform(Transform::Type type = Transform::TYPE_LINEAR_3D, sp<Rotation> rotation = nullptr, sp<Vec3> scale = nullptr, sp<Vec3> pivot = nullptr);
    Transform(sp<Delegate> delegate);

    virtual void traverse(const Visitor& visitor) override;

    class ARK_API Snapshot {
    public:
        Snapshot(const Transform& transform, const V3& postTranslate);
        Snapshot() = default;
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Snapshot);

        M4 toMatrix() const;

        V3 transform(const V3& p) const;

        template<typename T> T* makeData() {
            T* data = reinterpret_cast<T*>(_data);
            _magic = ark::Type<T>::id();
            return data;
        }

        template<typename T> const T* getData() const {
            DCHECK(_magic == ark::Type<T>::id(), "Transform magic mismatch, this Snapshot was taken by a different transform delegate");
            const T* data = reinterpret_cast<const T*>(_data);
            return data;
        }

    private:
        sp<Delegate> _delegate;
        TypeId _magic;
        alignas(64) uint8_t _data[72];
    };

    Snapshot snapshot(const V3& postTranslate) const;

    bool update(uint64_t timestamp);

//  [[script::bindings::property]]
    const sp<Rotation>& rotation();
//  [[script::bindings::property]]
    void setRotation(const sp<Rotation>& rotation);

//  [[script::bindings::property]]
    const sp<Vec3>& scale();
//  [[script::bindings::property]]
    void setScale(const sp<Vec3>& scale);

//  [[script::bindings::property]]
    const sp<Vec3>& pivot();
//  [[script::bindings::property]]
    void setPivot(const sp<Vec3>& pivot);

//  [[script::bindings::classmethod]]
    static sp<Mat4> toMatrix(sp<Transform> self);

//  [[plugin::builder]]
    class BUILDER : public Builder<Transform> {
    public:
        [[deprecated]]
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Transform> build(const Scope& args) override;

    private:
        Type _type;
        SafePtr<Builder<Rotation>> _rotation;
        SafePtr<Builder<Vec3>> _scale;
        SafePtr<Builder<Vec3>> _pivot;

    };

private:
    sp<Delegate> makeDelegate() const;
    sp<Delegate> makeTransformLinear() const;
    sp<Delegate> makeTransformSimple() const;

    class DelegateUpdater {
    public:
        DelegateUpdater(Transform& transform)
            : _transform(transform) {
        }

        void operator() () const {
            if(_transform._type != TYPE_DELEGATED)
                _transform.doUpdateDelegate();
        }

    private:
        Transform& _transform;
    };
    template<typename T> const sp<T>& tryUpdateDelegate(SafeVar<T>& safevar) {
        if(safevar)
            return safevar;

        const sp<T>& var = safevar.ensure();
        if(_type != TYPE_DELEGATED)
            doUpdateDelegate();
        return var;
    }

    void doUpdateDelegate();

private:
    Type _type;

    SafeVar<Rotation> _rotation;
    SafeVar<Vec3> _scale;
    SafeVar<Vec3> _pivot;

    sp<Delegate> _delegate;

    Timestamp _timestamp;

    friend class TransformSimple2D;
    friend class TransformSimple3D;
    friend class TransformLinear2D;
    friend class TransformLinear3D;
    friend class TransformTRS;
};

}

#endif
