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

Mesh::Mesh(array<element_index_t> indices, sp<Array<V3>> vertices, sp<Array<UV>> uvs, sp<Array<V3>> normals, sp<Array<Tangent>> tangents, sp<Array<BoneInfo>> boneInfos, sp<Material> material)
    : _indices(std::move(indices)), _vertices(std::move(vertices)), _uvs(std::move(uvs)), _normals(std::move(normals)), _tangents(std::move(tangents)), _bone_infos(std::move(boneInfos)),
      _material(std::move(material))
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

const sp<Material>& Mesh::material() const
{
    return _material;
}

const sp<Integer>& Mesh::nodeId() const
{
    return _node_id;
}

void Mesh::setNodeId(sp<Integer> nodeId)
{
    _node_id = std::move(nodeId);
}

void Mesh::write(VertexStream& buf) const
{
    V3* vertice = _vertices->buf();
    UV* uv = _uvs->buf();
    V3* normal = _normals ? _normals->buf() : nullptr;
    Tangent* tangent = _tangents ? _tangents->buf() : nullptr;
    BoneInfo* boneInfo = _bone_infos ? _bone_infos->buf() : nullptr;
    bool hasNodeId = static_cast<bool>(_node_id);
    int32_t nodeId = hasNodeId ? _node_id->val() : 0;
    size_t len = _vertices->length();

    for(size_t i = 0; i < len; ++i)
    {
        buf.next();
        buf.writePosition(*vertice);
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
        if(hasNodeId)
            buf.writeNodeId(nodeId);
    }
}

Mesh::BoneInfo::BoneInfo(std::array<float, 4> weights, std::array<uint32_t, 4> ids)
    : _weights(std::move(weights)), _ids(std::move(ids))
{
}

void Mesh::BoneInfo::add(uint32_t id, float weight)
{
    for(size_t i = 0; i < _weights.size(); ++i)
        if(_weights.at(i) == 0)
        {
            _weights[i] = weight;
            _ids[i] = id;
            return;
        }
    LOGW("Unable to add more weight to BoneId(%d), max weightarray length: %d", id, _weights.size());
}

}
