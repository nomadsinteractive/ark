#include "graphics/impl/transform/transform_linear_3d.h"

#include "core/util/updatable_util.h"

#include "graphics/base/mat.h"
#include "graphics/base/quaternion.h"
#include "graphics/base/v4.h"
#include "graphics/util/matrix_util.h"

namespace ark {

bool TransformLinear3D::update(const Transform::Stub& transform, uint64_t timestamp)
{
    return UpdatableUtil::update(timestamp, transform._rotation, transform._scale, transform._translation);
}

void TransformLinear3D::snapshot(const Transform::Stub& transform, Transform::Snapshot& snapshot) const
{
    M4& matrix = snapshot.makeData<M4>();
    const V4 quat = transform._rotation.val();
    matrix = MatrixUtil::translate(MatrixUtil::rotate(MatrixUtil::scale(M4::identity(), transform._scale.val()), quat), transform._translation.val());
}

V3 TransformLinear3D::transform(const Transform::Snapshot& snapshot, const V3& position) const
{
    const M4& matrix = snapshot.getData<M4>();
    const V4 pos = MatrixUtil::mul(matrix, V4(position, 1.0f));
    return V3(pos.x() / pos.w(), pos.y() / pos.w(), pos.z() / pos.w());
}

M4 TransformLinear3D::toMatrix(const Transform::Snapshot& snapshot) const
{
    return snapshot.getData<M4>();
}

}
