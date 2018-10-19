#include "graphics/impl/boolean/out_of_bounds.h"

#include "core/base/bean_factory.h"

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

OutOfBounds::BUILDER::BUILDER(BeanFactory& parent, const document& doc)
    : _bounds(parent.ensureBuilder<Bounds>(doc, Constants::Attributes::BOUNDS)), _position(parent.ensureBuilder<Vec>(doc, Constants::Attributes::POSITION))
{
}

sp<Boolean> OutOfBounds::BUILDER::build(const sp<Scope>& args)
{
    return sp<OutOfBounds>::make(_bounds->build(args), _position->build(args));
}

}
