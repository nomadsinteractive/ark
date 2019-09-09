#ifndef ARK_CORE_IMPL_INTEGER_INTEGER_BY_INTERVAL_H_
#define ARK_CORE_IMPL_INTEGER_INTEGER_BY_INTERVAL_H_

#include "core/forwarding.h"
#include "core/base/delegate.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class IntegerByInterval : public Integer, public Delegate<Integer> {
public:
    IntegerByInterval(const sp<Integer>& delegate, const sp<Numeric>& duration, const sp<Numeric>& interval);

    virtual int32_t val() override;

//  [[plugin::builder("by-interval")]]
    class BUILDER : public Builder<Integer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Integer> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Integer>> _delegate;
        sp<Builder<Duration>> _duration;
        sp<Builder<Numeric>> _interval;
    };

//  [[plugin::style("interval")]]
    class STYLE : public Builder<Integer> {
    public:
        STYLE(BeanFactory& factory, const sp<Builder<Integer>>& delegate, const String& value);

        virtual sp<Integer> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Integer>> _delegate;
        sp<Builder<Duration>> _duration;
        sp<Builder<Numeric>> _interval;
    };

private:
    sp<Numeric> _duration;
    sp<Numeric> _interval;

    int32_t _value;
    float _next_update_time;
};

}

#endif
