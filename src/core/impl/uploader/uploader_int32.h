#pragma once

#include "core/inf/builder.h"

#include "core/types/shared_ptr.h"

namespace ark {

class UploaderInt32 {
public:
//  [[plugin::builder::by-value("int32")]]
    class BUILDER final : public Builder<Uploader> {
    public:
        BUILDER(BeanFactory& factory, const String& value);

        sp<Uploader> build(const Scope& args) override;

    private:
        sp<Builder<Integer>> _var;
    };
};

}
