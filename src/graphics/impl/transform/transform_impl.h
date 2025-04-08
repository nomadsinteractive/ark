#pragma once

#include "core/base/api.h"
#include "core/base/wrapper.h"
#include "core/forwarding.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"
#include "core/types/safe_var.h"

#include "graphics/forwarding.h"
#include "graphics/base/mat.h"
#include "graphics/inf/transform.h"
#include "graphics/util/transform_type.h"

namespace ark {

class ARK_API TransformImpl : public Wrapper<Transform>, public Transform, Implements<TransformImpl, Transform, Mat4> {
public:
    TransformImpl(TransformType::Type type = TransformType::TYPE_NONE, sp<Vec4> rotation = nullptr, sp<Vec3> scale = nullptr, sp<Vec3> translation = nullptr);
    explicit TransformImpl(sp<Transform> delegate);
    explicit TransformImpl(sp<Mat4> delegate);

    bool update(uint64_t timestamp) override;
    M4 val() override;

    Snapshot snapshot() override;
    V4 transform(const Snapshot& snapshot, const V4& xyzw) override;
    M4 toMatrix(const Snapshot& snapshot) override;

    const SafeVar<Vec4>& rotation() const;
    void setRotation(sp<Vec4> rotation);

    const SafeVar<Vec3>& scale() const;
    void setScale(sp<Vec3> scale);

    const SafeVar<Vec3>& translation() const;
    void setTranslation(sp<Vec3> translation);

    void reset(sp<Mat4> transform);

private:
    sp<Transform> makeDelegate() const;
    sp<Transform> makeTransformTRS() const;
    sp<Transform> makeTransformTS() const;

    void doUpdateDelegate();

    struct Stub {
        SafeVar<Vec3> _translation;
        SafeVar<Vec4> _rotation;
        SafeVar<Vec3> _scale;

        Timestamp _timestamp;

        bool update(uint64_t timestamp) const;
    };

    class TransformDelegateMat4;

private:
    TransformType::Type _type;
    sp<Stub> _stub;

    friend class TransformTS2D;
    friend class TransformTS3D;
    friend class TransformTRS2D;
    friend class TransformTRS3D;
};

}
