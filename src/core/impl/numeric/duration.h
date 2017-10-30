#ifndef ARK_CORE_IMPL_NUMERIC_DURATION_H_
#define ARK_CORE_IMPL_NUMERIC_DURATION_H_

#include "core/ark.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"

namespace ark {

class Duration {
public:
//  [[plugin::builder("duration")]]
    class BUILDER : public Builder<Numeric> {
    public:
        BUILDER(BeanFactory& parent, const document& doc);

        virtual sp<Numeric> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Clock>> _clock;
        sp<Builder<Numeric>> _util;
    };

};

}

#endif
