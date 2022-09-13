#ifndef ARK_GRAPHICS_IMPL_INPUT_INPUT_VEC3_H_
#define ARK_GRAPHICS_IMPL_INPUT_INPUT_VEC3_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class FlatableV3f {
public:
//  [[plugin::builder::by-value("v3f")]]
    class BUILDER : public Builder<Input> {
    public:
        BUILDER(BeanFactory& factory, const String& value);

        virtual sp<Input> build(const Scope& args) override;

    private:
        sp<Builder<Vec3>> _vec3;
    };
};

}

#endif
