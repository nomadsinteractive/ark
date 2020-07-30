#include "renderer/impl/vertices/vertices_sphere.h"

#include "core/util/math.h"

#include "renderer/base/vertex_stream.h"

namespace ark {

VerticesSphere::VerticesSphere(size_t length)
    : Vertices(length)
{
}

VerticesSphere::VerticesSphere(sp<std::vector<ModelLoaderSphere::Vertex>> vertices, const Rect& uvBounds)
    : Vertices(vertices->size()), _vertices(std::move(vertices)), _uv_bounds(uvBounds)
{
}

void VerticesSphere::write(VertexStream& buf, const V3& size)
{
    for(const ModelLoaderSphere::Vertex& vertex : *_vertices)
    {
        buf.next();
        buf.writePosition(vertex._position * size);
        buf.writeTexCoordinate(static_cast<uint16_t>((_uv_bounds.left() + _uv_bounds.width() * vertex._u) * 0xffff),
                               static_cast<uint16_t>((_uv_bounds.top() + _uv_bounds.height() * vertex._v) * 0xffff));
        buf.writeNormal(vertex._position);
        buf.writeTangent(vertex._tangent);
    }
}

}
