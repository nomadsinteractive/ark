#ifndef ARK_CORE_IMPL_NUMERIC_ENVELOPE_H_
#define ARK_CORE_IMPL_NUMERIC_ENVELOPE_H_

#include <list>

#include "core/forwarding.h"
#include "core/base/bean_factory.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Envelope : public Numeric {
public:
    Envelope(BeanFactory& factory, const document& manifest, const sp<Scope>& args);

    virtual float val() override;

//  [[plugin::builder("envelope")]]
    class BUILDER : public Builder<Numeric> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Numeric> build(const sp<Scope>& args) override;

    private:
        BeanFactory _factory;
        document _manifest;

    };

private:
    struct Phrase {
        Phrase(BeanFactory& factory, const document& manifest);
        Phrase(const Phrase& other) = default;

        sp<Builder<Numeric>> _value;
        sp<Builder<Boolean>> _expired;
        sp<Builder<Expectation>> _expectation;
    };

    struct Stub {
        Stub(BeanFactory& factory, const document& manifest, const sp<Scope>& args);

        sp<Scope> _args;
        sp<Numeric> _value;
        std::list<Phrase> _phrases;
    };

    class ExpectedExpirable : public Boolean {
    public:
        ExpectedExpirable(const sp<Stub>& stub, const sp<Expectation>& expectation);

        virtual bool val() override;

    private:
        sp<Stub> _stub;
        sp<Expectation> _expectation;
    };

    sp<Boolean> buildState();

private:
    sp<Stub> _stub;
    sp<Boolean> _expired;
    std::list<Phrase>::const_iterator _phrases_iterator;

};

}

#endif
