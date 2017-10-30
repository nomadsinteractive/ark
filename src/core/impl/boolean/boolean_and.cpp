#include "core/impl/boolean/boolean_and.h"

#include "core/base/bean_factory.h"

namespace ark {

BooleanAnd::BooleanAnd(const sp<Boolean>& a1, const sp<Boolean>& a2)
    : _a1(a1), _a2(a2)
{
    NOT_NULL(_a1);
    NOT_NULL(_a2);
}

bool BooleanAnd::val()
{
    return _a1->val() && _a2->val();
}

BooleanAnd::BUILDER::BUILDER(BeanFactory& parent, const document& doc)
    : _a1(parent.ensureBuilder<Boolean>(doc, "a1")), _a2(parent.ensureBuilder<Boolean>(doc, "a2"))
{
}

sp<Boolean> BooleanAnd::BUILDER::build(const sp<Scope>& args)
{
    return sp<BooleanAnd>::make(_a1->build(args), _a2->build(args));
}

}
