#include "graphics/impl/boolean/out_of_bounds.h"

#include "core/base/bean_factory.h"
#include "core/util/variable_util.h"

#include "graphics/base/bounds.h"
#include "graphics/base/v2.h"

namespace ark {

OutOfBounds::OutOfBounds(const sp<Bounds>& bounds, const sp<Vec>& position)
    : _bounds(bounds), _position(position)
{
    DASSERT(bounds);
    DASSERT(position);
}

bool OutOfBounds::val()
{
    const V pt = _position->val();
    return !_bounds->ptin(pt);
}

bool OutOfBounds::update(uint64_t timestamp)
{
    return VariableUtil::update(timestamp, _position, _bounds);
}

OutOfBounds::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _bounds(factory.ensureBuilder<Bounds>(manifest, Constants::Attributes::BOUNDS)), _position(factory.ensureBuilder<Vec>(manifest, Constants::Attributes::POSITION))
{
}

sp<Boolean> OutOfBounds::BUILDER::build(const Scope& args)
{
    return sp<OutOfBounds>::make(_bounds->build(args), _position->build(args));
}

}
