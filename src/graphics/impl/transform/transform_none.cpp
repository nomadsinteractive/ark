#include "graphics/impl/transform/transform_none.h"

#include "core/types/global.h"
#include "core/types/null.h"

#include "graphics/base/mat.h"

namespace ark {

bool TransformNone::update(const Transform::Stub& transform, uint64_t timestamp)
{
    return false;
}

void TransformNone::snapshot(const Transform::Stub& /*transform*/, Transform::Snapshot& snapshot) const
{
}

V3 TransformNone::transform(const Transform::Snapshot& snapshot, const V3& position) const
{
    return position;
}

M4 TransformNone::toMatrix(const Transform::Snapshot& /*snapshot*/) const
{
    return M4::identity();
}

}
