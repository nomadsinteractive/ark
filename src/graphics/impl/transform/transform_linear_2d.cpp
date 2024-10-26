#include "graphics/impl/transform/transform_linear_2d.h"

#include "core/util/updatable_util.h"

#include "graphics/base/quaternion.h"
#include "graphics/util/matrix_util.h"

namespace ark {

bool TransformLinear2D::update(const Transform::Stub& transform, uint64_t timestamp)
{
    return UpdatableUtil::update(timestamp, transform._rotation, transform._scale, transform._translation);
}

void TransformLinear2D::snapshot(const Transform::Stub& transform, Transform::Snapshot& snapshot) const
{
    Snapshot& data = snapshot.makeData<Snapshot>();
    const SafeVar<Numeric>& theta = transform._rotation.wrapped()->theta();
    data.matrix = MatrixUtil::translate(MatrixUtil::rotate(MatrixUtil::scale(M3::identity(), V2(transform._scale.val())), theta.val()), V2(transform._translation.val()));
}

V3 TransformLinear2D::transform(const Transform::Snapshot& snapshot, const V3& position) const
{
    const Snapshot& data = snapshot.getData<Snapshot>();
    const V3 pos = MatrixUtil::mul(data.matrix, V3(position.x(), position.y(), 1.0f));
    return {pos.x() / pos.z(), pos.y() / pos.z(), position.z()};
}

M4 TransformLinear2D::toMatrix(const Transform::Snapshot& snapshot) const
{
    M4 matrix = M4::identity();
    const Snapshot& data = snapshot.getData<Snapshot>();
    memcpy(matrix.value(), data.matrix.value(), sizeof(float) * 3);
    memcpy(matrix.value() + 4, data.matrix.value() + 3, sizeof(float) * 3);
    memcpy(matrix.value() + 8, data.matrix.value() + 6, sizeof(float) * 3);
    return matrix;
}

}
