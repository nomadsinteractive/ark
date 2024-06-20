#include "graphics/impl/transform/transform_simple_2d.h"

#include "core/util/updatable_util.h"

#include "graphics/base/mat.h"
#include "graphics/base/v2.h"
#include "graphics/util/matrix_util.h"

namespace ark {

bool TransformSimple2D::update(const Transform::Stub& transform, uint64_t timestamp)
{
    return UpdatableUtil::update(timestamp, transform._scale, transform._translation);
}

void TransformSimple2D::snapshot(const Transform::Stub& transform, Transform::Snapshot& snapshot) const
{
    Snapshot& data = snapshot.makeData<Snapshot>();
    data.scale = transform._scale.val();
    data.translate = transform._translation.val();
}

V3 TransformSimple2D::transform(const Transform::Snapshot& snapshot, const V3& position) const
{
    const Snapshot& data = snapshot.getData<Snapshot>();
    const V2 pos = (V2(position) + data.translate) * data.scale;
    return V3(pos.x(), pos.y(), position.z());
}

M4 TransformSimple2D::toMatrix(const Transform::Snapshot& snapshot) const
{
    const M4 m = M4::identity();
    const Snapshot& data = snapshot.getData<Snapshot>();
    return MatrixUtil::translate(MatrixUtil::scale(m, V3(data.scale.x(), data.scale.y(), 1.0f)), V3(data.translate, 0));
}

}
