#pragma once

#include "core/inf/builder.h"

#include "core/types/shared_ptr.h"

namespace ark {

class UploaderUint8 {
public:
//  [[plugin::builder::by-value("uint8")]]
    class BUILDER final : public Builder<Uploader> {
    public:
        BUILDER(BeanFactory& factory, const String& value);

        sp<Uploader> build(const Scope& args) override;

    private:
        sp<Builder<Integer>> _var;
    };
};

}
