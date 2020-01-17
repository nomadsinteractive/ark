#include "assimp/impl/vertices/vertices_assimp.h"

#include "core/inf/array.h"

#include "renderer/base/vertex_stream.h"

namespace ark {
namespace plugin {
namespace assimp {

VerticesAssimp::UV::UV(uint16_t u, uint16_t v)
    : _u(u), _v(v)
{
}

VerticesAssimp::Tangent::Tangent(const V3& tangent, const V3& bitangent)
    : _tangent(tangent), _bitangent(bitangent)
{
}

VerticesAssimp::VerticesAssimp(sp<Array<V3>> vertices, sp<Array<VerticesAssimp::UV>> uvs, sp<Array<V3>> normals, sp<Array<VerticesAssimp::Tangent>> tangents)
    : Vertices(vertices->length()), _vertices(std::move(vertices)), _uvs(std::move(uvs)), _normals(std::move(normals)), _tangents(std::move(tangents))
{
    DASSERT(_vertices->length() == _uvs->length() && (!_normals || _vertices->length() == _normals->length()) && (!_tangents || _vertices->length() == _tangents->length()));
}

void VerticesAssimp::write(VertexStream& buf, const V3& size)
{
    V3* vertice = _vertices->buf();
    UV* uv = _uvs->buf();
    V3* normal = _normals ? _normals->buf() : nullptr;
    Tangent* tangent = _tangents ? _tangents->buf() : nullptr;
    size_t len = _vertices->length();

    for(size_t i = 0; i < len; ++i)
    {
        buf.next();
        buf.writePosition(*vertice * size);
        ++vertice;
        buf.writeTexCoordinate(uv->_u, uv->_v);
        ++uv;
        if(normal)
        {
            buf.writeNormal(*normal);
            ++normal;
        }
        if(tangent)
        {
            buf.writeTangent(tangent->_tangent);
            buf.writeBitangent(tangent->_bitangent);
            ++tangent;
        }
    }
}

}
}
}
