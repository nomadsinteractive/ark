#ifndef ARK_CORE_IMPL_NUMERIC_NUMERIC_SEQUENCE_H_
#define ARK_CORE_IMPL_NUMERIC_NUMERIC_SEQUENCE_H_

#include "core/base/bean_factory.h"
#include "core/collection/list.h"
#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class NumericSequence : public Numeric {
public:
    NumericSequence(const List<document>& sequence, bool looping, BeanFactory& beanFactory, const sp<Scope>& args);

    virtual float val() override;

//  [[plugin::builder("sequence")]]
    class BUILDER : public Builder<Numeric> {
    public:
        BUILDER(BeanFactory& parent, const document& manifest);

        virtual sp<Numeric> build(const sp<Scope>& args) override;

    private:
        BeanFactory _bean_factory;
        document _manifest;
        bool _looping;
    };

private:
    void moveToNext();

private:
    const List<document> _sequence;
    List<document>::const_iterator _iterator;

    sp<Numeric> _current;
    BeanFactory _bean_factory;
    sp<Scope> _args;

    bool _looping;
    float _next_value;
    float _previous_value;
};

}

#endif
