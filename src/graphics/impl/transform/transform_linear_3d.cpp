#include "graphics/impl/transform/transform_linear_3d.h"

#include "graphics/base/mat.h"
#include "graphics/base/quaternion.h"
#include "graphics/base/v4.h"
#include "graphics/util/matrix_util.h"

namespace ark {

void TransformLinear3D::snapshot(const Transform& transform, const V3& postTranslate, Transform::Snapshot& snapshot) const
{
    Snapshot* data = snapshot.makeData<Snapshot>();
    const V4 quat = transform._rotation.val();
    data->matrix = MatrixUtil::translate(MatrixUtil::rotate(MatrixUtil::scale(MatrixUtil::translate(M4::identity(), postTranslate), transform._scale.val()), quat), transform._translation.val());
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
