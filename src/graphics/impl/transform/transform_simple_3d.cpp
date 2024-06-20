#include "graphics/impl/transform/transform_simple_3d.h"

#include "core/util/updatable_util.h"

#include "graphics/base/mat.h"
#include "graphics/base/v3.h"
#include "graphics/util/matrix_util.h"

namespace ark {

bool TransformSimple3D::update(const Transform::Stub& transform, uint64_t timestamp)
{
    return UpdatableUtil::update(timestamp, transform._scale, transform._translation);
}

void TransformSimple3D::snapshot(const Transform::Stub& transform, Transform::Snapshot& snapshot) const
{
    Snapshot& data = snapshot.makeData<Snapshot>();
    data.scale = transform._scale.val();
    data.translate = transform._translation.val();
}

V3 TransformSimple3D::transform(const Transform::Snapshot& snapshot, const V3& position) const
{
    const Snapshot& data = snapshot.getData<Snapshot>();
    return (position + data.translate) * data.scale;
}

M4 TransformSimple3D::toMatrix(const Transform::Snapshot& snapshot) const
{
    const Snapshot& data = snapshot.getData<Snapshot>();
    return MatrixUtil::translate(MatrixUtil::scale(M4::identity(), data.scale), data.translate);
}

}
