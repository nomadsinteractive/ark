#ifndef ARK_CORE_IMPL_NUMERIC_TRANSLATE_H_
#define ARK_CORE_IMPL_NUMERIC_TRANSLATE_H_

#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Translate : public Numeric {
public:
    Translate(const sp<Numeric>& delegate, float translation);

    virtual float val() override;

//  [[plugin::style("translation")]]
    class DECORATOR : public Builder<Numeric> {
    public:
        DECORATOR(BeanFactory& parent, const sp<Builder<Numeric>>& delegate, const String& value);

        virtual sp<Numeric> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _delegate;
        sp<Builder<Numeric>> _translation;
    };

private:
    sp<Numeric> _delegate;
    float _translation;
};

}

#endif
