#include "renderer/base/model.h"

#include "renderer/inf/vertices.h"

namespace ark {

Model::Model(const array<element_index_t>& indices, const array<V3>& vertices, const array<UV>& uvs, const array<V3>& normals, const array<Tangents>& tangents, const V3& size)
    : _indices(indices), _vert(vertices), _uvs(uvs), _normals(normals), _tangents(tangents), _size(size)
{
}

Model::Model(const array<element_index_t>& indices, const sp<Vertices>& vertices, const V3& size)
    : _indices(indices), _vertices(vertices), _size(size)
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

const array<V3>& Model::vert() const
{
    return _vert;
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

const V3& Model::size() const
{
    return _size;
}

V3 Model::toScale(const V3& renderObjectSize) const
{
    return V3(renderObjectSize.x() == 0 ? _size.x() : renderObjectSize.x(), renderObjectSize.y() == 0 ? _size.y() : renderObjectSize.y(), renderObjectSize.z() == 0 ? _size.z() : renderObjectSize.z());
}

void Model::writeToStream(VertexStream& buf, const V3& size) const
{
    _vertices->write(buf, toScale(size));
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
