#include "renderer/base/model.h"

#include "core/inf/variable.h"

#include "renderer/base/mesh.h"
#include "renderer/inf/animate_maker.h"
#include "renderer/inf/uploader.h"
#include "renderer/inf/vertices.h"


namespace ark {

Model::Model(sp<Uploader> indices, sp<Vertices> vertices, const Metrics& metrics)
    : _indices(std::move(indices)), _vertices(std::move(vertices)), _metrics(metrics)
{
}

Model::Model(sp<Array<Mesh>> meshes, const Metrics& metrics)
    : _indices(sp<MeshIndicesUploader>::make(meshes)), _vertices(sp<MeshVertices>::make(meshes, metrics.size)), _meshes(std::move(meshes)), _metrics(metrics)
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

const sp<Array<Mesh>>& Model::meshes() const
{
    return _meshes;
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

const Table<String, sp<AnimateMaker>>& Model::animates() const
{
    return _animates;
}

Table<String, sp<AnimateMaker>>& Model::animates()
{
    return _animates;
}

V3 Model::toScale(const V3& renderObjectSize) const
{
    return V3(renderObjectSize.x() == 0 ? _metrics.size.x() : renderObjectSize.x(), renderObjectSize.y() == 0 ? _metrics.size.y() : renderObjectSize.y(), renderObjectSize.z() == 0 ? _metrics.size.z() : renderObjectSize.z());
}

void Model::writeToStream(VertexStream& buf, const V3& size) const
{
    _vertices->write(buf, toScale(size));
}

Model::MeshIndicesUploader::MeshIndicesUploader(sp<ark::Array<Mesh>> meshes)
    : Uploader(calcIndicesSize(meshes)), _meshes(std::move(meshes))
{
}

void Model::MeshIndicesUploader::upload(Writable& uploader)
{
    uint32_t offset = 0;
    size_t length = _meshes->length();
    Mesh* buf = _meshes->buf();
    for(size_t i = 0; i < length; ++i)
    {
        const array<element_index_t>& indices = buf[i].indices();
        uint32_t size = static_cast<uint32_t>(indices->size());
        uploader.write(indices->buf(), size, offset);
        offset += size;
    }
}

size_t Model::MeshIndicesUploader::calcIndicesSize(ark::Array<Mesh>& meshes) const
{
    size_t size = 0;
    for(const Mesh& i : meshes)
        size += i.indices()->length() * sizeof(element_index_t);
    return size;
}

Model::MeshVertices::MeshVertices(sp<Array<Mesh>> meshes, const V3& size)
    : Vertices(calcVertexLength(meshes)), _meshes(std::move(meshes)), _size(size)
{
}

size_t Model::MeshVertices::calcVertexLength(Array<Mesh>& meshes) const
{
    size_t length = meshes.length();
    size_t vertexLength = 0;
    Mesh* m = meshes.buf();
    for(size_t i = 0; i < length; ++i)
        vertexLength += m[i].vertexLength();
    return vertexLength;
}

void Model::MeshVertices::write(VertexStream& buf, const V3& size)
{
    const V3 scale(size.x() != 0 ? size.x() / _size.x() : _size.x(), size.y() != 0 ? size.y() / _size.y() : _size.y(), size.z() != 0 ? size.z() /  _size.z() : _size.z());
    for(const Mesh& m : *_meshes)
        m.write(buf, scale);
}

}
