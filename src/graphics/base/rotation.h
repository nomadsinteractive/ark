#pragma once

#include "core/base/api.h"
#include "core/base/timestamp.h"
#include "core/inf/builder.h"
#include "core/inf/holder.h"
#include "core/inf/variable.h"
#include "core/types/safe_ptr.h"
#include "core/types/safe_var.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v4.h"

namespace ark {

//[[script::bindings::holder]]
//[[script::bindings::extends(Vec4)]]
class ARK_API Rotation : public Holder, public Vec4 {
public:
    Rotation(const V4& quat);
//  [[script::bindings::auto]]
    Rotation(float theta, const V3& axis = Rotation::Z_AXIS);
//  [[script::bindings::auto]]
    Rotation(const sp<Numeric>& theta, const sp<Vec3>& axis = nullptr);
    Rotation(sp<Numeric> theta, sp<Vec3> axis, sp<Vec4> quaternion);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Rotation);

    virtual V4 val() override;
    virtual bool update(uint64_t timestamp) override;

    virtual void traverse(const Visitor& visitor) override;

//  [[script::bindings::property]]
    const sp<Numeric>& theta() const;
//  [[script::bindings::property]]
    void setTheta(const sp<Numeric>& theta);
//  [[script::bindings::property]]
    const sp<Vec3>& axis();

//  [[script::bindings::auto]]
    void setRotation(float theta, const V3& axis);
//  [[script::bindings::auto]]
    void setRotation(const sp<Numeric>& theta, const sp<Vec3>& axis);

//  [[script::bindings::auto]]
    void setEuler(float pitch, float yaw, float roll);
//  [[script::bindings::auto]]
    void setEuler(const sp<Numeric>& pitch, const sp<Numeric>& yaw, const sp<Numeric>& roll);

//[[plugin::builder]]
    class BUILDER : public Builder<Rotation> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Rotation> build(const Scope& args) override;

    private:
        SafePtr<Builder<Numeric>> _theta;
        SafePtr<Builder<Vec3>> _axis;
    };

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Rotation> {
    public:
        DICTIONARY(BeanFactory& factory, const String& str);

        virtual sp<Rotation> build(const Scope& args) override;

    private:
        SafePtr<Builder<Numeric>> _theta;
    };

    static const V3 Z_AXIS;

private:
    SafeVar<Numeric> _theta;
    SafeVar<Vec3> _axis;

    sp<Vec4> _quaternion;

    Timestamp _timestamp;
};

}
