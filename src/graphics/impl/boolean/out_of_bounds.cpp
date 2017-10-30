#include "graphics/impl/boolean/out_of_bounds.h"

#include "core/base/bean_factory.h"

#include "graphics/base/bounds.h"
#include "graphics/base/v2.h"

namespace ark {

OutOfBounds::OutOfBounds(const sp<Bounds>& bounds, const sp<VV2>& position)
    : _bounds(bounds), _position(position)
{
    NOT_NULL(bounds);
    NOT_NULL(position);
}

bool OutOfBounds::val()
{
    const V2 p = _position->val();
    return !_bounds->ptin(p.x(), p.y());
}

OutOfBounds::BUILDER::BUILDER(BeanFactory& parent, const document& doc)
    : _bounds(parent.ensureBuilder<Bounds>(doc, Constants::Attributes::BOUNDS)), _position(parent.ensureBuilder<VV2>(doc, Constants::Attributes::POSITION))
{
}

sp<Boolean> OutOfBounds::BUILDER::build(const sp<Scope>& args)
{
    return sp<OutOfBounds>::make(_bounds->build(args), _position->build(args));
}

}
