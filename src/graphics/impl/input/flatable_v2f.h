#ifndef ARK_GRAPHICS_IMPL_FLATABLE_FLATABLE_V2F_H_
#define ARK_GRAPHICS_IMPL_FLATABLE_FLATABLE_V2F_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class FlatableV2f {
public:
//  [[plugin::builder::by-value("v2f")]]
    class BUILDER : public Builder<Input> {
    public:
        BUILDER(BeanFactory& factory, const String& value);

        virtual sp<Input> build(const Scope& args) override;

    private:
        sp<Builder<Vec2>> _vec2;
    };
};

}

#endif
