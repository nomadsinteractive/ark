#pragma once

#include "core/base/api.h"
#include "core/base/constants.h"
#include "core/base/wrapper.h"
#include "core/inf/variable.h"
#include "core/types/safe_builder.h"
#include "core/types/safe_var.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v4.h"

namespace ark {

//[[script::bindings::extends(Vec4)]]
class ARK_API Rotation final : public Vec4, public Wrapper<Vec4> {
public:
    Rotation(const V4& quat);
    Rotation(sp<Vec4> quaternion);
//  [[script::bindings::auto]]
    Rotation(float theta, const V3& axis = constants::AXIS_Z);
//  [[script::bindings::auto]]
    Rotation(sp<Numeric> theta, sp<Vec3> axis = nullptr);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Rotation);

    V4 val() override;
    bool update(uint64_t timestamp) override;

//  [[script::bindings::auto]]
    void reset(sp<Vec4> quaternion);

//  [[script::bindings::property]]
    const SafeVar<Numeric>& theta() const;
//  [[script::bindings::property]]
    void setTheta(sp<Numeric> theta);
//  [[script::bindings::property]]
    const SafeVar<Vec3>& axis() const;
//  [[script::bindings::auto]]
    void setRotation(float theta, const V3& axis);
//  [[script::bindings::auto]]
    void setRotation(sp<Numeric> theta, sp<Vec3> axis);

//  [[script::bindings::auto]]
    void setEuler(float pitch, float yaw, float roll);
//  [[script::bindings::auto]]
    void setEuler(sp<Numeric> pitch, sp<Numeric> yaw, sp<Numeric> roll);

//  [[plugin::builder]]
    class BUILDER : public Builder<Rotation> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Rotation> build(const Scope& args) override;

    private:
        SafeBuilder<Numeric> _theta;
        SafeBuilder<Vec3> _axis;
    };

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Rotation> {
    public:
        DICTIONARY(BeanFactory& factory, const String& str);

        sp<Rotation> build(const Scope& args) override;

    private:
        SafeBuilder<Numeric> _theta;
    };

private:
    SafeVar<Numeric> _theta;
    SafeVar<Vec3> _axis;
};

}
