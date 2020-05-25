#include "renderer/base/model.h"

#include "core/inf/variable.h"

#include "renderer/inf/vertices.h"
#include "renderer/inf/uploader.h"

namespace ark {

Model::Model(sp<Uploader> indices, sp<Vertices> vertices, const Metrics& metrics)
    : _indices(std::move(indices)), _vertices(std::move(vertices)), _metrics(metrics)
{
}

const sp<Uploader>& Model::indices() const
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

size_t Model::indexLength() const
{
    return _indices->size() / sizeof(element_index_t);
}

size_t Model::vertexLength() const
{
    return _vertices->length();
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
