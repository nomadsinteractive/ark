#ifndef ARK_GRAPHICS_IMPL_FLATABLE_FLATABLE_V4F_H_
#define ARK_GRAPHICS_IMPL_FLATABLE_FLATABLE_V4F_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class FlatableV4f {
public:
//  [[plugin::builder::by-value("v4f")]]
    class BUILDER : public Builder<Flatable> {
    public:
        BUILDER(BeanFactory& factory, const String& value);

        virtual sp<Flatable> build(const Scope& args) override;

    private:
        sp<Builder<Vec4>> _vec4;
    };
};

}

#endif
