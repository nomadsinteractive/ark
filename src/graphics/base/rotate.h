#ifndef ARK_GRAPHICS_BASE_ROTATE_H_
#define ARK_GRAPHICS_BASE_ROTATE_H_

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API Rotate {
public:
//  [[script::bindings::auto]]
    Rotate(const sp<Numeric>& rotation, const sp<VV3>& direction = nullptr);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Rotate);

//  [[script::bindings::property]]
    sp<Numeric> rotation() const;
//  [[script::bindings::property]]
    void setRotation(float rotation);
//  [[script::bindings::property]]
    void setRotation(const sp<Numeric>& rotation);

//  [[script::bindings::property]]
    const sp<VV3>& direction() const;
//  [[script::bindings::property]]
    void setDirection(const sp<VV3>& direction);

    static const V3 Z_AXIS;

//  [[plugin::builder]]
    class BUILDER : public Builder<Rotate> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Rotate> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _rotation;
    };

private:
    sp<NumericWrapper> _rotation;
    sp<VV3> _direction;

};

}

#endif
