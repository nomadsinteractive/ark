#include "graphics/impl/transform/transform_linear_3d.h"

#include "graphics/base/mat.h"
#include "graphics/base/rotate.h"
#include "graphics/base/v4.h"
#include "graphics/util/matrix_util.h"

namespace ark {

void TransformLinear3D::snapshot(const Transform& transform, Transform::Snapshot& snapshot) const
{
    Snapshot* data = snapshot.makeData<Snapshot>();
    const Rotation rot = transform._rotate.val();
    data->matrix = MatrixUtil::rotate(MatrixUtil::scale(MatrixUtil::translate(M4::identity(), transform._pivot.val()), transform._scale.val()), rot.direction, rot.angle);
}

V3 TransformLinear3D::transform(const Transform::Snapshot& snapshot, const V3& position) const
{
    const Snapshot* data = snapshot.getData<Snapshot>();
    const V4 pos = MatrixUtil::mul(data->matrix, V4(position, 1.0f));
    return V3(pos.x() / pos.w(), pos.y() / pos.w(), pos.z() / pos.w());
}

M4 TransformLinear3D::toMatrix(const Transform::Snapshot& snapshot) const
{
    const Snapshot* data = snapshot.getData<Snapshot>();
    return data->matrix;
}

}
