#include "graphics/impl/transform/transform_ts_2d.h"

#include "core/util/updatable_util.h"

#include "graphics/base/mat.h"
#include "graphics/base/v2.h"
#include "graphics/util/matrix_util.h"

namespace ark {

namespace {

struct SnapshotTS2D {
    V2 _translation;
    V2 _scale;
};

}

TransformTS2D::TransformTS2D(const TransformImpl& transform)
    : _stub(transform._stub)
{
}

bool TransformTS2D::update(const uint64_t timestamp)
{
    return _stub->update(timestamp);
}

Transform::Snapshot TransformTS2D::snapshot()
{
    return {SnapshotTS2D{V2(_stub->_translation.val()), V2(_stub->_scale.val())}};
}

V4 TransformTS2D::transform(const Snapshot& snapshot, const V4& xyzw)
{
    const V2 xy(xyzw.x(), xyzw.y());
    const SnapshotTS2D& data = snapshot.data<SnapshotTS2D>();
    return {(xy + data._translation * xyzw.w()) * data._scale, xyzw.z(), xyzw.w()};
}

M4 TransformTS2D::toMatrix(const Snapshot& snapshot)
{
    const SnapshotTS2D& data = snapshot.data<SnapshotTS2D>();
    return MatrixUtil::translate(MatrixUtil::scale({}, V3(data._scale.x(), data._scale.y(), 1.0f)), V3(data._translation, 0));
}

}
