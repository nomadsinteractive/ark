#include "graphics/impl/transform/transform_trs_2d.h"

#include "core/util/updatable_util.h"

#include "graphics/util/matrix_util.h"

namespace ark {

TransformTRS2D::TransformTRS2D(const Transform& transform)
    : Transform(transform)
{
}

bool TransformTRS2D::update(const uint64_t timestamp)
{
    return _stub->update(timestamp);
}

Transform::Snapshot TransformTRS2D::snapshot()
{
    const V4 quaternion = _stub->_rotation.val();
    CHECK(quaternion.x() == 0 && quaternion.y() == 0, "2D rotation should be always in the XY plane");
    const float s = 2 * quaternion.z() * quaternion.w();
    const float c = 2 * quaternion.w() * quaternion.w() - 1.0f;
    return {MatrixUtil::translate(MatrixUtil::rotate(MatrixUtil::scale(M3(), V2(_stub->_scale.val())), s, c), V2(_stub->_translation.val()))};
}

V4 TransformTRS2D::transform(const Snapshot& snapshot, const V4& xyzw)
{
    const M3& matrix = snapshot.data<M3>();
    const V2 transformed = MatrixUtil::mul(matrix, V3(xyzw.x(), xyzw.y(), 1.0f)).toNonHomogeneous();
    return {transformed * xyzw.w(), xyzw.z(), xyzw.w()};
}

M4 TransformTRS2D::toMatrix(const Snapshot& snapshot)
{
    M4 matrix;
    const M3& mat3 = snapshot.data<M3>();
    memcpy(matrix.value(), mat3.value(), sizeof(float) * 3);
    memcpy(matrix.value() + 4, mat3.value() + 3, sizeof(float) * 3);
    memcpy(matrix.value() + 8, mat3.value() + 6, sizeof(float) * 3);
    return matrix;
}

}
