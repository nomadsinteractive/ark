#ifndef ARK_GRAPHICS_BASE_QUATERNION_H_
#define ARK_GRAPHICS_BASE_QUATERNION_H_

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/holder.h"
#include "core/inf/variable.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

namespace ark {

struct ARK_API Rotation {
    Rotation();
    Rotation(float angle, const V3& direction);

    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Rotation);
    float angle;
    V3 direction;
};

//[[script::bindings::holder]]
class ARK_API Quaternion : public Holder, public Variable<Rotation> {
public:
//  [[script::bindings::auto]]
    Quaternion(const sp<Numeric>& value, const sp<Vec3>& direction = nullptr);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Quaternion);

    virtual Rotation val() override;
    virtual bool update(uint64_t timestamp) override;

    virtual void traverse(const Visitor& visitor) override;

//  [[script::bindings::property]]
    float rotation() const;
//  [[script::bindings::property]]
    void setRotation(float rotation);
//  [[script::bindings::property]]
    void setRotation(const sp<Numeric>& rotation);

//  [[script::bindings::property]]
    const sp<Vec3>& direction() const;
//  [[script::bindings::property]]
    void setDirection(const sp<Vec3>& direction);

    const sp<NumericWrapper>& value() const;

    static const V3 Z_AXIS;

//[[plugin::builder]]
    class BUILDER : public Builder<Quaternion> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Quaternion> build(const Scope& args) override;

    private:
        SafePtr<Builder<Numeric>> _angle;
        SafePtr<Builder<Vec3>> _direction;
    };

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Quaternion> {
    public:
        DICTIONARY(BeanFactory& factory, const String& str);

        virtual sp<Quaternion> build(const Scope& args) override;

    private:
        SafePtr<Builder<Numeric>> _rotation;
    };

private:
    sp<NumericWrapper> _value;
    sp<Vec3> _direction;
};

}

#endif
