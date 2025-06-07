#pragma once

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class UploaderVec4i {
public:
//  [[plugin::builder::by-value("ivec4")]]
    class BUILDER final : public Builder<Uploader> {
    public:
        BUILDER(BeanFactory& factory, const String& value);

        sp<Uploader> build(const Scope& args) override;

    private:
        sp<Builder<Vec4i>> _vec4i;
    };
};

}
