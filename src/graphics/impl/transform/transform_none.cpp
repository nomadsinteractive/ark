#include "graphics/impl/transform/transform_none.h"

#include "core/types/global.h"
#include "core/types/null.h"

#include "graphics/base/mat.h"


namespace ark {

void TransformNone::snapshot(const Transform& /*transform*/, const V3& postTranslate, Transform::Snapshot& snapshot) const
{
    snapshot.makeData<Snapshot>()->postTranslate = postTranslate;
}

V3 TransformNone::transform(const Transform::Snapshot& snapshot, const V3& position) const
{
    return snapshot.getData<Snapshot>()->postTranslate + position;
}

M4 TransformNone::toMatrix(const Transform::Snapshot& /*snapshot*/) const
{
    return M4::identity();
}

template<> ARK_API sp<Transform::Delegate> Null::safePtr()
{
    return Global<TransformNone>().cast<Transform::Delegate>();
}

}
