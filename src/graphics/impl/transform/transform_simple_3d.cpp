#include "graphics/impl/transform/transform_simple_3d.h"

#include "graphics/base/mat.h"
#include "graphics/base/v3.h"
#include "graphics/util/matrix_util.h"

namespace ark {

void TransformSimple3D::snapshot(const Transform& transform, Transform::Snapshot& snapshot) const
{
    Snapshot* data = snapshot.makeData<Snapshot>();
    data->pivot = transform._pivot.val();
    data->scale = transform._scale.val();
}

V3 TransformSimple3D::transform(const Transform::Snapshot& snapshot, const V3& position) const
{
    const Snapshot* data = snapshot.getData<Snapshot>();
    return (position + data->pivot) * data->scale;
}

M4 TransformSimple3D::toMatrix(const Transform::Snapshot& snapshot) const
{
    const M4 m = M4::identity();
    const Snapshot* data = snapshot.getData<Snapshot>();
    return MatrixUtil::translate(MatrixUtil::scale(m, data->scale), data->pivot);
}

}
