#pragma once

#include "core/inf/builder.h"

#include "core/types/shared_ptr.h"

namespace ark {

class UploaderNumeric {
public:
//  [[plugin::builder::by-value("float")]]
    class BUILDER final : public Builder<Uploader> {
    public:
        BUILDER(BeanFactory& factory, const String& value);

        sp<Uploader> build(const Scope& args) override;

    private:
        sp<Builder<Numeric>> _numeric;
    };
};

}
