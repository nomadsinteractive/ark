#ifndef ARK_CORE_IMPL_BOOLEAN_BOOLEAN_EXPRESSION_H_
#define ARK_CORE_IMPL_BOOLEAN_BOOLEAN_EXPRESSION_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

namespace ark {

class BooleanExpression {
public:

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Boolean> {
    public:
        DICTIONARY(BeanFactory&, const String& value);

        virtual sp<Boolean> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Boolean>> _expression;
    };

};

}

#endif
