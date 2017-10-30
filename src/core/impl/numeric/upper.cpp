#include "core/impl/numeric/upper.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/inf/message_loop.h"

namespace ark {

Upper::Upper(const sp<Numeric>& delegate, const sp<Numeric>& boundary, const sp<Runnable>& oncross)
    : _delegate(delegate), _boundary(boundary), _oncross(oncross)
{
}

float Upper::val()
{
    float value = _delegate->val();
    if(_oncross && value > _boundary->val())
    {
        const sp<Runnable> onCross = std::move(_oncross);
        if(onCross)
            Ark::instance().global<MessageLoop>()->post(onCross, 0);
    }
    return value;
}

Upper::BUILDER::BUILDER(BeanFactory& parent, const document& manifest)
    : _delegate(parent.ensureBuilder<Numeric>(manifest, Constants::Attributes::DELEGATE)), _boundary(parent.ensureBuilder<Numeric>(manifest, "boundary")),
      _oncross(parent.ensureBuilder<Runnable>(manifest, Constants::Attributes::ON_CROSS))
{
}

sp<Numeric> Upper::BUILDER::build(const sp<Scope>& args)
{
    return sp<Upper>::make(_delegate->build(args), _boundary->build(args), _oncross->build(args));
}

}
