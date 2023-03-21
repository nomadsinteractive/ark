#include "renderer/base/model.h"

#include "core/inf/variable.h"
#include "core/impl/writable/writable_memory.h"

#include "renderer/base/mesh.h"
#include "renderer/base/vertex_writer.h"
#include "renderer/inf/animation.h"
#include "renderer/inf/vertices.h"


namespace ark {

Model::Model(sp<Uploader> indices, sp<Vertices> vertices, sp<Metrics> bounds, sp<Metrics> occupies)
    : _indices(std::move(indices)), _vertices(std::move(vertices)), _bounds(std::move(bounds)), _occupies(occupies ? std::move(occupies) : sp<Metrics>(_bounds))
{
}

Model::Model(std::vector<sp<Material>> materials, std::vector<sp<Mesh>> meshes, sp<Node> rootNode, sp<Metrics> bounds, sp<Metrics> occupies)
    : _indices(sp<InputMeshIndices>::make(meshes)), _vertices(sp<MeshVertices>::make(meshes)), _bounds(std::move(bounds)), _occupies(occupies ? std::move(occupies) : sp<Metrics>(_bounds)),
      _root_node(std::move(rootNode)), _materials(std::move(materials)),_meshes(std::move(meshes))
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

element_index_t Model::writeIndices(element_index_t* buf, element_index_t baseIndex) const
{
    _indices->upload(WritableMemory(buf));
    element_index_t length = static_cast<element_index_t>(_indices->size() / sizeof(element_index_t));
    if(baseIndex != 0)
        for(size_t i = 0; i < length; ++i)
            buf[i] += baseIndex;
    return length;
}

const std::vector<sp<Material>>& Model::materials() const
{
    return _materials;
}

const std::vector<sp<Mesh>>& Model::meshes() const
{
    return _meshes;
}

const sp<Node>& Model::rootNode() const
{
    return _root_node;
}

const sp<Metrics>& Model::bounds() const
{
    return _bounds;
}

const sp<Metrics>& Model::occupies() const
{
    return _occupies;
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

void Model::writeToStream(VertexWriter& buf, const V3& size) const
{
    _vertices->write(buf, size == V3(0) && _bounds ? _bounds->size() : size);
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
    : Uploader(calcIndicesSize(meshes)), _meshes(std::move(meshes))
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

bool Model::InputMeshIndices::update(uint64_t /*timestamp*/)
{
    return false;
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
