#ifndef ARK_CORE_IMPL_NUMERIC_EXPECT_H_
#define ARK_CORE_IMPL_NUMERIC_EXPECT_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Expect : public Numeric {
public:
    Expect(const sp<Numeric>& delegate, const sp<Runnable>& onArrival, const sp<Numeric>& expectation);

    virtual float val() override;

//  [[plugin::builder("expect")]]
    class BUILDER : public Builder<Numeric> {
    public:
        BUILDER(BeanFactory& parent, const document& doc);

        virtual sp<Numeric> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _delegate;
        sp<Builder<Numeric>> _expectation;
        sp<Builder<Runnable>> _on_arrival;

    };

//  [[plugin::style("expect")]]
    class STYLE : public Builder<Numeric> {
    public:
        STYLE(BeanFactory& parent, const sp<Builder<Numeric>>& delegate, const String& value);

        virtual sp<Numeric> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _delegate;
        sp<Builder<Numeric>> _expectation;
        sp<Builder<Runnable>> _on_arrival;

    };
private:
    struct Stub {
        Stub(const sp<Numeric>& delegate, const sp<Runnable>& onArrival, const sp<Numeric>& expectataion);

        sp<Numeric> _delegate;
        sp<Runnable> _on_arrival;
        sp<Numeric> _expectation;

        float _delta;
        float _epsilon;
    };

private:
    sp<Stub> _stub;
};

}

#endif
