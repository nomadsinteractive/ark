#ifndef ARK_CORE_IMPL_NUMERIC_TRACKER_H_
#define ARK_CORE_IMPL_NUMERIC_TRACKER_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Tracker : public Numeric {
public:
    Tracker(const sp<Numeric>& delegate, const sp<Runnable>& oncross, float pitch);

    virtual float val() override;

//  [[plugin::builder("tracker")]]
    class BUILDER : public Builder<Numeric> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Numeric> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _delegate;
        sp<Builder<Numeric>> _pitch;
        sp<Builder<Runnable>> _oncross;
    };

private:
    sp<Numeric> _delegate;
    sp<Runnable> _oncross;
    float _pitch;
    float _floor;

};

}

#endif
