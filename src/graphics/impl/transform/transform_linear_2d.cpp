#include "graphics/impl/transform/transform_linear_2d.h"

#include "graphics/base/quaternion.h"
#include "graphics/util/matrix_util.h"

namespace ark {

void TransformLinear2D::snapshot(const Transform& transform, const V3& postTranslate, Transform::Snapshot& snapshot) const
{
    Snapshot* data = snapshot.makeData<Snapshot>();
    const sp<Numeric>& theta = transform._rotation.wrapped()->theta();
    data->matrix = MatrixUtil::translate(MatrixUtil::rotate(MatrixUtil::scale(MatrixUtil::translate(M3::identity(), V2(postTranslate)), V2(transform._scale.val())), theta->val()), V2(transform._pivot.val()));
}

V3 TransformLinear2D::transform(const Transform::Snapshot& snapshot, const V3& position) const
{
    const Snapshot* data = snapshot.getData<Snapshot>();
    const V3 pos = MatrixUtil::mul(data->matrix, V3(position.x(), position.y(), 1.0f));
    return V3(pos.x() / pos.z(), pos.y() / pos.z(), position.z());
}

M4 TransformLinear2D::toMatrix(const Transform::Snapshot& snapshot) const
{
    M4 matrix = M4::identity();
    const Snapshot* data = snapshot.getData<Snapshot>();
    memcpy(matrix.value(), data->matrix.value(), sizeof(float) * 3);
    memcpy(matrix.value() + 4, data->matrix.value() + 3, sizeof(float) * 3);
    memcpy(matrix.value() + 8, data->matrix.value() + 6, sizeof(float) * 3);
    return matrix;
}

}
