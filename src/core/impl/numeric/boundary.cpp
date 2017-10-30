#include "core/impl/numeric/boundary.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/inf/message_loop.h"
#include "core/inf/runnable.h"

namespace ark {

Boundary::Boundary(const sp<Numeric>& delegate, const sp<Numeric>& boundary, const sp<Runnable>& oncross)
    : _delegate(delegate), _boundary(boundary), _oncross(oncross), _is_greater(delegate->val() > boundary->val())
{
}

float Boundary::val()
{
    float value = _delegate->val();
    float boundary = _boundary->val();
    if(_oncross)
    {
        bool isGreater = value > boundary;
        if(isGreater != _is_greater)
        {
            const sp<Runnable> onCross = std::move(_oncross);
            onCross->run();
            return boundary;
        }
    }
    else
        return boundary;
    return value;
}

Boundary::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _delegate(factory.ensureBuilder<Numeric>(manifest, Constants::Attributes::DELEGATE)),
      _boundary(factory.ensureBuilder<Numeric>(manifest, Constants::Attributes::BOUNDARY)),
      _oncross(factory.ensureBuilder<Runnable>(manifest, Constants::Attributes::ON_CROSS))
{
}

sp<Numeric> Boundary::BUILDER::build(const sp<Scope>& args)
{
    return sp<Numeric>::adopt(new Boundary(_delegate->build(args), _boundary->build(args), _oncross->build(args)));
}

}
