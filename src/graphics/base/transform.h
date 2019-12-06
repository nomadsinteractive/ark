#ifndef ARK_GRAPHICS_BASE_TRANSFORM_H_
#define ARK_GRAPHICS_BASE_TRANSFORM_H_

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/inf/holder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"
#include "core/types/safe_ptr.h"
#include "core/types/safe_var.h"

#include "graphics/forwarding.h"
#include "graphics/base/mat.h"
#include "graphics/base/rotate.h"
#include "graphics/base/v3.h"

namespace ark {

//[[script::bindings::holder]]
class ARK_API Transform : public Holder {
public:
//  [[script::bindings::enumeration]]
    enum Type {
        TYPE_LINEAR_2D,
        TYPE_LINEAR_3D
    };

//  [[script::bindings::auto]]
    Transform(Transform::Type type = Transform::TYPE_LINEAR_3D, const sp<Rotate>& rotate = nullptr, const sp<Vec3>& scale = nullptr, const sp<Vec3>& pivot = nullptr);

    virtual void traverse(const Visitor& visitor) override;

    class Snapshot;

    class Delegate {
    public:
        virtual ~Delegate() = default;

        virtual void snapshot(const Transform& transform, Snapshot& snapshot) const = 0;

        virtual V3 transform(const Snapshot& snapshot, const V3& position) const = 0;
        virtual M4 toMatrix(const Snapshot& snapshot) const = 0;
    };

    class ARK_API Snapshot {
    public:
        Snapshot(const Transform& transform);
        Snapshot(const Snapshot& other) = default;

        M4 toMatrix() const;

        V3 transform(const V3& p) const;

        template<typename T> T* makeData() {
            T* data = reinterpret_cast<T*>(_data);
            data->magic = ark::Type<T>::id();
            return data;
        }

        template<typename T> const T* getData() const {
            const T* data = reinterpret_cast<const T*>(_data);
            DCHECK(data->magic == ark::Type<T>::id(), "Transform magic mismatch, this Snapshot was taken by a different transform delegate");
            return data;
        }

    private:
        sp<Delegate> _delegate;
        uint8_t _data[72];
    };

    Snapshot snapshot() const;

//  [[script::bindings::property]]
    const sp<Rotate>& rotate();
//  [[script::bindings::property]]
    void setRotate(const sp<Rotate>& rotate);

//  [[script::bindings::property]]
    const sp<Vec3>& scale();
//  [[script::bindings::property]]
    void setScale(const sp<Vec3>& scale);

//  [[script::bindings::property]]
    const sp<Vec3>& pivot();
//  [[script::bindings::property]]
    void setPivot(const sp<Vec3>& pivot);

//  [[plugin::builder]]
    class BUILDER : public Builder<Transform> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Transform> build(const Scope& args) override;

    private:
        Type _type;
        SafePtr<Builder<Rotate>> _rotate;
        SafePtr<Builder<Vec3>> _scale;
        SafePtr<Builder<Vec3>> _pivot;

    };

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Transform> {
    public:
        DICTIONARY(BeanFactory& factory, const String& value);

        virtual sp<Transform> build(const Scope& args) override;

    private:
        BUILDER _impl;

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
            _transform._delegate = _transform.makeDelegate();
        }

    private:
        Transform& _transform;

    };

private:
    Type _type;

    SafeVar<Rotate, DelegateUpdater> _rotate;
    SafeVar<Vec3, DelegateUpdater> _scale;
    SafeVar<Vec3, DelegateUpdater> _pivot;

    sp<Delegate> _delegate;

    friend class TransformSimple2D;
    friend class TransformSimple3D;
    friend class TransformLinear2D;
    friend class TransformLinear3D;

};

}

#endif
