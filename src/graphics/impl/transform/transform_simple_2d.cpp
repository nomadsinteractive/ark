#include "graphics/impl/transform/transform_simple_2d.h"

#include "graphics/base/mat.h"
#include "graphics/base/v2.h"
#include "graphics/util/matrix_util.h"

namespace ark {

void TransformSimple2D::snapshot(const Transform& transform, Transform::Snapshot& snapshot) const
{
    Snapshot* data = snapshot.makeData<Snapshot>();
    data->pivot = transform._pivot.val();
    data->scale = transform._scale.val();
}

V3 TransformSimple2D::transform(const Transform::Snapshot& snapshot, const V3& position) const
{
    const Snapshot* data = snapshot.getData<Snapshot>();
    const V2 pos = (V2(position) + data->pivot) * data->scale;
    return V3(pos.x(), pos.y(), position.z());
}

M4 TransformSimple2D::toMatrix(const Transform::Snapshot& snapshot) const
{
    const M4 m = M4::identity();
    const Snapshot* data = snapshot.getData<Snapshot>();
    return MatrixUtil::translate(MatrixUtil::scale(m, V3(data->scale.x(), data->scale.y(), 1.0f)), data->pivot);
}

}
