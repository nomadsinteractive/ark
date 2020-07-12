#include "renderer/impl/vertices/vertices_sphere.h"

#include "core/util/math.h"

#include "renderer/base/vertex_stream.h"

namespace ark {

VerticesSphere::VerticesSphere(uint32_t sampleCount)
    : Vertices((sampleCount * 2 + 1) * (sampleCount + 1))
{
}

VerticesSphere::VerticesSphere(sp<std::vector<ModelLoaderSphere::Vertex>> vertices, const Atlas::Item& item)
    : Vertices(vertices->size()), _vertices(std::move(vertices)), _item(item)
{
}

void VerticesSphere::write(VertexStream& buf, const V3& size)
{
    uint32_t vertexId = 0;
    for(const ModelLoaderSphere::Vertex& vertex : *_vertices)
    {
        buf.next();
        buf.writePosition(vertex._position * size, vertexId ++);
        buf.writeTexCoordinate(_item.ux() + static_cast<uint16_t>((_item.vx() - _item.ux()) * vertex._u), _item.uy() + static_cast<uint16_t>((_item.vy() - _item.uy()) * vertex._v));
        buf.writeNormal(vertex._position);
        buf.writeTangent(vertex._tangent);
    }
}

}
