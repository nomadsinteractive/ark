#ifndef ARK_GRAPHICS_BASE_ROTATE_H_
#define ARK_GRAPHICS_BASE_ROTATE_H_

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API Rotate {
public:
//  [[script::bindings::auto]]
    Rotate(const sp<Numeric>& value, const sp<Vec3>& direction = nullptr);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Rotate);

//  [[script::bindings::property]]
    float radians() const;
//  [[script::bindings::property]]
    void setRadians(float radians);
//  [[script::bindings::property]]
    void setRadians(const sp<Numeric>& radians);

//  [[script::bindings::property]]
    const sp<Vec3>& direction() const;
//  [[script::bindings::property]]
    void setDirection(const sp<Vec3>& direction);

    const sp<NumericWrapper>& value() const;

    static const V3 Z_AXIS;

//[[plugin::builder]]
    class BUILDER : public Builder<Rotate> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Rotate> build(const sp<Scope>& args) override;

    private:
        SafePtr<Builder<Numeric>> _rotation;
    };

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Rotate> {
    public:
        DICTIONARY(BeanFactory& factory, const String& str);

        virtual sp<Rotate> build(const sp<Scope>& args) override;

    private:
        SafePtr<Builder<Numeric>> _rotation;
    };


private:
    sp<NumericWrapper> _value;
    sp<Vec3> _direction;

};

}

#endif