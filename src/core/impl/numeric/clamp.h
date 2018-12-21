#ifndef ARK_CORE_IMPL_NUMERIC_CLAMP_H_
#define ARK_CORE_IMPL_NUMERIC_CLAMP_H_

#include "core/forwarding.h"
#include "core/base/observer.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Clamp : public Numeric {
public:
    Clamp(const sp<Numeric>& delegate, const sp<Numeric>& min, const sp<Numeric>& max, const sp<Runnable>& notifier);

    virtual float val() override;

//  [[plugin::builder("clamp")]]
    class BUILDER : public Builder<Numeric> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Numeric> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _delegate;
        sp<Builder<Numeric>> _min;
        sp<Builder<Numeric>> _max;
    };

//  [[plugin::style("clamp")]]
    class STYLE : public Builder<Numeric> {
    public:
        STYLE(BeanFactory& factory, const sp<Builder<Numeric>>& delegate, const String& style);

        virtual sp<Numeric> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _delegate;
        sp<Builder<Numeric>> _min;
        sp<Builder<Numeric>> _max;
        sp<Builder<Runnable>> _notifier;
    };


private:
    sp<Numeric> _delegate;
    sp<Numeric> _min;
    sp<Numeric> _max;
    Observer _notifier;
};

}

#endif
