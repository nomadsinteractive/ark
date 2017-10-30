#include "core/impl/numeric/duration.h"

#include "core/base/bean_factory.h"
#include "core/base/clock.h"

namespace ark {

Duration::BUILDER::BUILDER(BeanFactory& parent, const document& manifest)
    : _clock(parent.ensureBuilder<Clock>(manifest, Constants::Attributes::CLOCK)), _util(parent.getBuilder<Numeric>(manifest, "util", false))
{
}

sp<Numeric> Duration::BUILDER::build(const sp<Scope>& args)
{
    const sp<Clock>& clock = _clock->build(args);
    return _util ? clock->durationUtil(_util->build(args)) : clock->duration();
}

}
