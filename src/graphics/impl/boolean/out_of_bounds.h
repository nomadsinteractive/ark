#ifndef ARK_GRAPHICS_IMPL_VARIABLE_BOOLEAN_OUT_OF_BOUNDS_H_
#define ARK_GRAPHICS_IMPL_VARIABLE_BOOLEAN_OUT_OF_BOUNDS_H_

#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class OutOfBounds : public Boolean {
public:
    OutOfBounds(const sp<Bounds>& bounds, const sp<Vec3>& position);

    virtual bool val() override;
    virtual bool update(uint64_t timestamp) override;

//  [[plugin::builder("out_of_bounds")]]
    class BUILDER : public Builder<Boolean> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Boolean> build(const Scope& args) override;

    private:
        sp<Builder<Bounds>> _bounds;
        sp<Builder<Vec3>> _position;
    };

private:
    sp<Bounds> _bounds;
    sp<Vec3> _position;

};

}

#endif
