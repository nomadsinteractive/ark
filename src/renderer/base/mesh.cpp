#include "renderer/base/mesh.h"

#include "core/inf/array.h"
#include "core/util/log.h"

#include "renderer/base/vertex_stream.h"

namespace ark {

Mesh::UV::UV(uint16_t u, uint16_t v)
    : _u(u), _v(v)
{
}

Mesh::Tangent::Tangent(const V3& tangent, const V3& bitangent)
    : _tangent(tangent), _bitangent(bitangent)
{
}

Mesh::Mesh(array<element_index_t> indices, sp<Array<V3>> vertices, sp<Array<UV>> uvs, sp<Array<V3>> normals, sp<Array<Tangent>> tangents, sp<Array<BoneInfo>> boneInfos)
    : _indices(std::move(indices)), _vertices(std::move(vertices)), _uvs(std::move(uvs)), _normals(std::move(normals)), _tangents(std::move(tangents)), _bone_infos(std::move(boneInfos))
{
    DASSERT(_vertices->length() == _uvs->length() && (!_normals || _vertices->length() == _normals->length()) && (!_tangents || _vertices->length() == _tangents->length())
            && (!_bone_infos || _vertices->length() == _bone_infos->length()));
}

size_t Mesh::vertexLength() const
{
    return _vertices->length();
}

const array<element_index_t>& Mesh::indices() const
{
    return _indices;
}

const array<V3>& Mesh::vertices() const
{
    return _vertices;
}

void Mesh::write(VertexStream& buf, const V3& scale) const
{
    V3* vertice = _vertices->buf();
    UV* uv = _uvs->buf();
    V3* normal = _normals ? _normals->buf() : nullptr;
    Tangent* tangent = _tangents ? _tangents->buf() : nullptr;
    BoneInfo* boneInfo = _bone_infos ? _bone_infos->buf() : nullptr;
    size_t len = _vertices->length();

    for(size_t i = 0; i < len; ++i)
    {
        buf.next();
        buf.writePosition(*vertice * scale);
        ++vertice;
        buf.writeTexCoordinate(uv->_u, uv->_v);
        ++uv;
        if(normal)
            buf.writeNormal(*(normal++));
        if(tangent)
        {
            buf.writeTangent(tangent->_tangent);
            buf.writeBitangent(tangent->_bitangent);
            ++tangent;
        }
        if(boneInfo)
            buf.writeBoneInfo(*(boneInfo++));
    }
}

Mesh::BoneInfo::BoneInfo(std::array<float, 4> weights, std::array<int32_t, 4> ids)
    : _weights(std::move(weights)), _ids(std::move(ids))
{
}

void Mesh::BoneInfo::add(int32_t id, float weight)
{
    for(size_t i = 0; i < _weights.size(); ++i)
        if(_weights.at(i) == 0)
        {
            _weights[i] = weight;
            _ids[i] = id;
            return;
        }
    LOGW("Unable to add more weight to VertexID(%d), max weightarray length: %d", id, _weights.size());
}

}
