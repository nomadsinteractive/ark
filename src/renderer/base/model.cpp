#include "renderer/base/model.h"

#include "core/inf/variable.h"

#include "renderer/inf/vertices.h"

namespace ark {

Model::Model(const array<element_index_t>& indices, const sp<Vertices>& vertices, const Metrics& metrics)
    : _indices(indices), _vertices(vertices), _metrics(metrics)
{
}

const array<element_index_t>& Model::indices() const
{
    return _indices;
}

const sp<Vertices>& Model::vertices() const
{
    return _vertices;
}

const Metrics& Model::metrics() const
{
    return _metrics;
}

V3 Model::toScale(const V3& renderObjectSize) const
{
    return V3(renderObjectSize.x() == 0 ? _metrics.size.x() : renderObjectSize.x(), renderObjectSize.y() == 0 ? _metrics.size.y() : renderObjectSize.y(), renderObjectSize.z() == 0 ? _metrics.size.z() : renderObjectSize.z());
}

void Model::writeToStream(VertexStream& buf, const V3& size) const
{
    _vertices->write(buf, toScale(size));
}

}
