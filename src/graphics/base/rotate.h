#ifndef ARK_GRAPHICS_BASE_ROTATE_H_
#define ARK_GRAPHICS_BASE_ROTATE_H_

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/holder.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

//[[script::bindings::holder]]
class ARK_API Rotate : public Holder {
public:
//  [[script::bindings::auto]]
    Rotate(const sp<Numeric>& value, const sp<Vec3>& direction = nullptr);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Rotate);

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
    class BUILDER : public Builder<Rotate> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Rotate> build(const Scope& args) override;

    private:
        SafePtr<Builder<Numeric>> _rotation;
    };

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Rotate> {
    public:
        DICTIONARY(BeanFactory& factory, const String& str);

        virtual sp<Rotate> build(const Scope& args) override;

    private:
        SafePtr<Builder<Numeric>> _rotation;
    };


private:
    sp<NumericWrapper> _value;
    sp<Vec3> _direction;

};

}

#endif
