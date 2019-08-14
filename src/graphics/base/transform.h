#ifndef ARK_GRAPHICS_BASE_TRANSFORM_H_
#define ARK_GRAPHICS_BASE_TRANSFORM_H_

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/inf/dictionary.h"
#include "core/inf/builder.h"
#include "core/inf/holder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"
#include "core/types/safe_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

namespace ark {

//[[script::bindings::holder]]
class ARK_API Transform : public Holder {
public:
//  [[script::bindings::auto]]
    Transform(const sp<Rotate>& rotate = nullptr, const sp<Vec3>& scale = nullptr, const sp<Vec3>& pivot = nullptr);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Transform);

    virtual void traverse(const Visitor& visitor) override;

    class ARK_API Snapshot {
    public:
        Snapshot();
        Snapshot(const Snapshot& other) = default;

        Matrix toMatrix() const;

        bool operator ==(const Snapshot& other) const;
        bool operator !=(const Snapshot& other) const;

        void map(float x, float y, float tx, float ty, float& mx, float& my) const;
        V3 mapXYZ(const V3& p) const;

        float rotate_value;
        V3 rotate_direction;
        V3 scale;
        V3 pivot;
    };

    Snapshot snapshot() const;

//  [[script::bindings::property]]
    const sp<Rotate>& rotate();
//  [[script::bindings::property]]
    void setRotate(const sp<Rotate>& rotate);

//  [[script::bindings::property]]
    const sp<Vec3>& scale() const;
//  [[script::bindings::property]]
    void setScale(const sp<Vec3>& scale);

//  [[script::bindings::property]]
    const sp<Vec3>& pivot() const;
//  [[script::bindings::property]]
    void setPivot(const sp<Vec3>& pivot);

//  [[plugin::builder]]
    class BUILDER : public Builder<Transform> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Transform> build(const sp<Scope>& args) override;

    private:
        SafePtr<Builder<Rotate>> _rotate;
        SafePtr<Builder<Vec3>> _scale;
        SafePtr<Builder<Vec3>> _pivot;

    };

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Transform> {
    public:
        DICTIONARY(BeanFactory& factory, const String& value);

        virtual sp<Transform> build(const sp<Scope>& args) override;

    private:
        BUILDER _impl;

    };

private:
    SafePtr<Rotate> _rotate;
    sp<Vec3> _scale;
    SafePtr<Vec3> _pivot;

};

}

#endif
