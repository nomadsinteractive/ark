#include "renderer/base/model.h"

namespace ark {

Model::Model(array<element_index_t> indices, array<V3> vertices, array<Model::UV> uvs, array<V3> normals, array<Model::Tangents> tangents)
    : _indices(std::move(indices)), _vertices(std::move(vertices)), _uvs(std::move(uvs)), _normals(std::move(normals)), _tangents(std::move(tangents))
{
}

const array<element_index_t>& Model::indices() const
{
    return _indices;
}

const array<V3>& Model::vertices() const
{
    return _vertices;
}

const array<Model::UV>& Model::uvs() const
{
    return _uvs;
}

const array<V3>& Model::normals() const
{
    return _normals;
}

const array<Model::Tangents>& Model::tangents() const
{
    return _tangents;
}

Model::UV::UV(uint16_t u, uint16_t v)
    : _u(u), _v(v)
{
}

Model::Tangents::Tangents(const V3& tangent, const V3& bitangent)
    : _tangent(tangent), _bitangent(bitangent)
{
}

}
