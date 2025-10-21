#include "graphics/impl/transform/transform_trs_3d.h"

#include "graphics/base/mat.h"
#include "graphics/base/v4.h"
#include "graphics/util/matrix_util.h"

namespace ark {

TransformTRS3D::TransformTRS3D(const Transform& transform)
    : Transform(transform)
{
}

bool TransformTRS3D::update(const uint64_t timestamp)
{
    return _stub->update(timestamp);
}

Transform::Snapshot TransformTRS3D::snapshot()
{
    const V4 quat = _stub->_rotation.val();
    return {MatrixUtil::translate(MatrixUtil::scale(MatrixUtil::rotate({}, quat), _stub->_scale.val()), _stub->_translation.val())};
}

V4 TransformTRS3D::transform(const Snapshot& snapshot, const V4& xyzw)
{
    return MatrixUtil::mul(snapshot.data<M4>(), xyzw);
}

M4 TransformTRS3D::toMatrix(const Snapshot& snapshot)
{
    return snapshot.data<M4>();
}

}
