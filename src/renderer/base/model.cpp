#include "renderer/base/model.h"

#include "core/inf/variable.h"

#include "renderer/base/mesh.h"
#include "renderer/base/vertex_writer.h"
#include "renderer/inf/animation.h"
#include "renderer/inf/vertices.h"


namespace ark {

Model::Model(sp<Input> indices, sp<Vertices> vertices, const Metrics& metrics)
    : _indices(std::move(indices)), _vertices(std::move(vertices)), _metrics(metrics)
{
}

Model::Model(std::vector<sp<Material>> materials, std::vector<sp<Mesh>> meshes, const Metrics& metrics)
    : _indices(sp<InputMeshIndices>::make(meshes)), _vertices(sp<MeshVertices>::make(meshes)), _materials(std::move(materials)), _meshes(std::move(meshes)), _metrics(metrics)
{
}

const sp<Input>& Model::indices() const
{
    return _indices;
}

const sp<Vertices>& Model::vertices() const
{
    return _vertices;
}

const std::vector<sp<Material>>& Model::materials() const
{
    return _materials;
}

const std::vector<sp<Mesh>>& Model::meshes() const
{
    return _meshes;
}

const Metrics& Model::metrics() const
{
    return _metrics;
}

const V3& Model::bounds() const
{
    return _metrics.bounds;
}

const V3& Model::size() const
{
    return _metrics.size;
}

const V3& Model::origin() const
{
    return _metrics.orgin;
}

size_t Model::indexCount() const
{
    return _indices->size() / sizeof(element_index_t);
}

size_t Model::vertexCount() const
{
    return _vertices->length();
}

const Table<String, sp<Animation>>& Model::animations() const
{
    return _animations;
}

void Model::setAnimations(Table<String, sp<Animation>> animates)
{
    _animations = std::move(animates);
}

const std::vector<String>& Model::nodeNames() const
{
    return _node_names;
}

void Model::setNodeNames(std::vector<String> nodes)
{
    _node_names = std::move(nodes);
}

const sp<Animation>& Model::getAnimation(const String& name) const
{
    CHECK(_animations.has(name), "Model has no animate(%s) defined", name.c_str());
    return _animations.at(name);
}

V3 Model::toScale(const V3& renderObjectSize) const
{
    return V3(renderObjectSize.x() == 0 ? _metrics.size.x() : renderObjectSize.x(), renderObjectSize.y() == 0 ? _metrics.size.y() : renderObjectSize.y(), renderObjectSize.z() == 0 ? _metrics.size.z() : renderObjectSize.z());
}

void Model::writeToStream(VertexWriter& buf, const V3& size) const
{
    _vertices->write(buf, toScale(size));
}

void Model::writeRenderable(VertexWriter& writer, const Renderable::Snapshot& renderable) const
{
    writer.setRenderObject(renderable);
    writeToStream(writer, renderable._size);
}

void Model::dispose()
{
    _indices = nullptr;
    _vertices = nullptr;
    _meshes.clear();
    _animations.clear();
}

bool Model::isDisposed() const
{
    return !static_cast<bool>(_indices);
}

Model::InputMeshIndices::InputMeshIndices(std::vector<sp<Mesh>> meshes)
    : Input(calcIndicesSize(meshes)), _meshes(std::move(meshes))
{
}

void Model::InputMeshIndices::upload(Writable& uploader)
{
    uint32_t offset = 0;
    for(const Mesh& i : _meshes)
    {
        const std::vector<element_index_t>& indices = i.indices();
        uint32_t size = static_cast<uint32_t>(indices.size() * sizeof(element_index_t));
        uploader.write(indices.data(), size, offset);
        offset += size;
    }
}

size_t Model::InputMeshIndices::calcIndicesSize(const std::vector<sp<Mesh>>& meshes) const
{
    size_t size = 0;
    for(const Mesh& i : meshes)
        size += i.indices().size() * sizeof(element_index_t);
    return size;
}

Model::MeshVertices::MeshVertices(std::vector<sp<Mesh>> meshes)
    : Vertices(calcVertexLength(meshes)), _meshes(std::move(meshes))
{
}

size_t Model::MeshVertices::calcVertexLength(const std::vector<sp<Mesh>>& meshes) const
{
    size_t vertexLength = 0;
    for(const Mesh& i : meshes)
        vertexLength += i.vertexLength();
    return vertexLength;
}

void Model::MeshVertices::write(VertexWriter& buf, const V3& /*size*/)
{
    for(const Mesh& m : _meshes)
        m.write(buf);
}

}
