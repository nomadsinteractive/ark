#include "graphics/impl/transform/transform_ts_3d.h"

#include "graphics/base/mat.h"
#include "graphics/base/v3.h"
#include "graphics/util/matrix_util.h"

namespace ark {

namespace {

struct SnapshotTS3D {
    V3 _translation;
    V3 _scale;
};

}

TransformTS3D::TransformTS3D(const TransformImpl& transform)
    : _stub(transform._stub)
{
}

bool TransformTS3D::update(const uint64_t timestamp)
{
    return _stub->update(timestamp);
}

Transform::Snapshot TransformTS3D::snapshot()
{
    return {SnapshotTS3D{_stub->_translation.val(), _stub->_scale.val()}};
}

V4 TransformTS3D::transform(const Snapshot& snapshot, const V4& xyzw)
{
    const V3 xyz(xyzw.x(), xyzw.y(), xyzw.z());
    const SnapshotTS3D& data = snapshot.data<SnapshotTS3D>();
    return {(xyz + data._translation * xyzw.w()) * data._scale, xyzw.w()};
}

M4 TransformTS3D::toMatrix(const Snapshot& snapshot)
{
    const SnapshotTS3D& data = snapshot.data<SnapshotTS3D>();
    return MatrixUtil::translate(MatrixUtil::scale(M4::identity(), data._scale), data._translation);
}

}
