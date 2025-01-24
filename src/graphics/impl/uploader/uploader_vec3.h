#pragma once

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class UploaderVec3 {
public:
//  [[plugin::builder::by-value("vec3")]]
    class BUILDER final : public Builder<Uploader> {
    public:
        BUILDER(BeanFactory& factory, const String& value);

        sp<Uploader> build(const Scope& args) override;

    private:
        sp<Builder<Vec3>> _vec3;
    };
};

}
