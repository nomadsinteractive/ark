#include "graphics/impl/transform/transform_trs.h"

#include "graphics/base/quaternion.h"
#include "graphics/util/matrix_util.h"

namespace ark {

void TransformTRS::snapshot(const Transform::Stub& transform, Transform::Snapshot& snapshot) const
{
    Snapshot& data = snapshot.makeData<Snapshot>();
    const SafeVar<Numeric>& theta = transform._rotation.wrapped()->theta();
    data.matrix = MatrixUtil::translate(MatrixUtil::rotate(MatrixUtil::scale(M3::identity(), V2(transform._scale.val())), theta.val()), V2(transform._translation.val()));
}

V3 TransformTRS::transform(const Transform::Snapshot& snapshot, const V3& position) const
{
    const Snapshot& data = snapshot.getData<Snapshot>();
    const V3 pos = MatrixUtil::mul(data.matrix, V3(position.x(), position.y(), 1.0f));
    return V3(pos.x() / pos.z(), pos.y() / pos.z(), position.z());
}

M4 TransformTRS::toMatrix(const Transform::Snapshot& snapshot) const
{
    M4 matrix;
    const Snapshot& data = snapshot.getData<Snapshot>();
    memcpy(matrix.value(), data.matrix.value(), sizeof(float) * 3);
    memcpy(matrix.value() + 4, data.matrix.value() + 3, sizeof(float) * 3);
    memcpy(matrix.value() + 8, data.matrix.value() + 6, sizeof(float) * 3);
    return matrix;
}

}
