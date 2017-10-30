#ifndef ARK_CORE_IMPL_NUMERIC_NUMERIC_EXPRESSION_H_
#define ARK_CORE_IMPL_NUMERIC_NUMERIC_EXPRESSION_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"


namespace ark {

class NumericExpression final {
public:
//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Numeric> {
    public:
        DICTIONARY(BeanFactory& parent, const String& value);

        virtual sp<Numeric> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _expression;
    };

//  [[plugin::builder("expression")]]
    class BUILDER : public Builder<Numeric> {
    public:
        BUILDER(BeanFactory& parent, const document& doc);

        virtual sp<Numeric> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _expression;
    };

//  [[plugin::builder("numeric")]]
    class NUMERIC_BUILDER : public Builder<Numeric> {
    public:
        NUMERIC_BUILDER(BeanFactory& parent, const document& doc);

        virtual sp<Numeric> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _expression;
    };

};

}

#endif
