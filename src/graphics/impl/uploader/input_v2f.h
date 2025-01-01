#pragma once

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class InputV2f {
public:
//  [[plugin::builder::by-value("v2f")]]
    class BUILDER : public Builder<Uploader> {
    public:
        BUILDER(BeanFactory& factory, const String& value);

        virtual sp<Uploader> build(const Scope& args) override;

    private:
        sp<Builder<Vec2>> _vec2;
    };
};

}
