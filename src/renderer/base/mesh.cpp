#include "renderer/base/mesh.h"

#include "core/inf/array.h"
#include "core/util/log.h"
#include "core/util/math.h"

#include "graphics/base/material.h"

#include "renderer/base/vertex_writer.h"

namespace ark {

Mesh::UV::UV(uint16_t u, uint16_t v)
    : _u(u), _v(v)
{
}

Mesh::Tangent::Tangent(const V3& tangent, const V3& bitangent)
    : _tangent(tangent), _bitangent(bitangent)
{
}

Mesh::Mesh(uint32_t id, String name, Vector<element_index_t> indices, Vector<V3> vertices, sp<Array<UV>> uvs, sp<Array<V3>> normals, sp<Array<Tangent>> tangents, sp<Array<BoneInfo>> boneInfos, sp<Material> material)
    : _id(id), _name(std::move(name)), _indices(std::move(indices)), _vertices(std::move(vertices)), _uvs(std::move(uvs)), _normals(std::move(normals)), _tangents(std::move(tangents)), _bone_infos(std::move(boneInfos)),
      _material(std::move(material))
{
    CHECK_WARN(_uvs, "Null UV coordinates may not be supported in the future");
    CHECK((!_uvs || _vertices.size() == _uvs->length()) && (!_normals || _vertices.size() == _normals->length()) && (!_tangents || _vertices.size() == _tangents->length()) &&
          (!_bone_infos || _vertices.size() == _bone_infos->length()), "Invalid mesh \"%s\", id: %d", _name.c_str(), id);
}

const Node& Mesh::parent() const
{
    ASSERT(_parent);
    return *_parent;
}

uint32_t Mesh::id() const
{
    return _id;
}

const String& Mesh::name() const
{
    return _name;
}

size_t Mesh::vertexCount() const
{
    return _vertices.size();
}

const Vector<element_index_t>& Mesh::indices() const
{
    return _indices;
}

const Vector<V3>& Mesh::vertices() const
{
    return _vertices;
}

const sp<Material>& Mesh::material() const
{
    return _material;
}

const sp<Array<V3>>& Mesh::normals() const
{
    return _normals;
}

const sp<Array<Mesh::Tangent>>& Mesh::tangents() const
{
    return _tangents;
}

void Mesh::write(VertexWriter& buf) const
{
    const V3* vertice = _vertices.data();
    const UV* uv = _uvs ? _uvs->buf() : nullptr;
    const V3* normal = _normals ? _normals->buf() : nullptr;
    const Tangent* tangent = _tangents ? _tangents->buf() : nullptr;
    const BoneInfo* boneInfo = _bone_infos ? _bone_infos->buf() : nullptr;
    const size_t len = _vertices.size();

    for(size_t i = 0; i < len; ++i)
    {
        buf.next();
        buf.writePosition(*vertice);
        ++vertice;
        if(uv)
        {
            buf.writeTexCoordinate(uv->_u, uv->_v);
            ++uv;
        }
        else
            buf.writeTexCoordinate(0, 0);
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

std::pair<V3, V3> Mesh::calculateBoundingAABB() const
{
    V3 aabbMin(std::numeric_limits<float>::max()), aabbMax(std::numeric_limits<float>::min());

    for(const V3& i : _vertices)
    {
        aabbMin = {std::min(i.x(), aabbMin.x()), std::min(i.y(), aabbMin.y()), std::min(i.z(), aabbMin.z())};
        aabbMax = {std::max(i.x(), aabbMax.x()), std::max(i.y(), aabbMax.y()), std::max(i.z(), aabbMax.z())};
    }
    return {aabbMin, aabbMax};
}

void Mesh::BoneInfo::add(const uint32_t id, const float weight)
{
    for(size_t i = 0; i < _weights.size(); ++i)
        if(_weights.at(i) == 0)
        {
            _weights[i] = weight;
            _ids[i] = id;
            return;
        }
    DCHECK_WARN(Math::almostEqual(weight, 0.0f), "Unable to add more weight to BoneId(%d), max weightarray length: %d", id, Mesh::INFO_ARRAY_LENGTH);
}

}

