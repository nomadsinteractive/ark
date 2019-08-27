#ifndef ARK_CORE_IMPL_NUMERIC_DURATION_H_
#define ARK_CORE_IMPL_NUMERIC_DURATION_H_

#include "core/base/bean_factory.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Duration : public Numeric {
public:
    Duration(const sp<Numeric>& delegate);

    virtual float val() override;

//  [[plugin::function("duration")]]
    static sp<Numeric> duration();

//  [[plugin::builder]]
    class BUILDER : public Builder<Duration> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Duration> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Clock>> _clock;
        sp<Builder<Numeric>> _delegate;
        sp<Builder<Numeric>> _until;
    };

//  [[plugin::builder("duration")]]
    class NUMERIC_BUILDER : public Builder<Numeric> {
    public:
        NUMERIC_BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Numeric> build(const sp<Scope>& args) override;

    private:
        BUILDER _builder;
    };

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Duration> {
    public:
        DICTIONARY(BeanFactory& factory);

        virtual sp<Duration> build(const sp<Scope>& args) override;
    };


private:
    sp<Numeric> _delegate;
};

}

#endif
