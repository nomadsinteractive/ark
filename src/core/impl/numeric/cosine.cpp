#include "core/impl/numeric/cosine.h"

#include "core/base/bean_factory.h"
#include "core/util/math.h"
#include "core/util/documents.h"
#include "core/util/bean_utils.h"

namespace ark {

Cosine::Cosine(const sp<Numeric>& x)
    : _x(x)
{
}

float Cosine::val()
{
    return Math::cos(_x->val());
}

sp<Numeric> Cosine::call(const sp<Numeric>& x)
{
    return sp<Cosine>::make(x);
}

Cosine::BUILDER::BUILDER(BeanFactory& parent, const document& doc)
    : _x(parent.ensureBuilder<Numeric>(doc, "x"))
{
}

sp<Numeric> Cosine::BUILDER::build(const Scope& args)
{
    return Cosine::call(_x->build(args));
}

}
