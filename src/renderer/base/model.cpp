#include "renderer/base/model.h"

#include "core/inf/variable.h"
#include "core/impl/writable/writable_memory.h"
#include "core/util/uploader_type.h"

#include "renderer/base/mesh.h"
#include "renderer/base/vertex_writer.h"
#include "renderer/inf/vertices.h"

namespace ark {

namespace {

size_t calcIndicesSize(const Vector<sp<Mesh>>& meshes)
{
    size_t size = 0;
    for(const Mesh& i : meshes)
        size += i.indices().size() * sizeof(element_index_t);
    return size;
}

size_t calcVertexCount(const Vector<sp<Mesh>>& meshes)
{
    size_t vertexCount = 0;
    for(const Mesh& i : meshes)
        vertexCount += i.vertexCount();
    return vertexCount;
}

class InputMeshIndices final : public Uploader {
public:
    InputMeshIndices(Vector<sp<Mesh>> meshes)
        : Uploader(calcIndicesSize(meshes)), _meshes(std::move(meshes))
    {
    }

    void upload(Writable& uploader) override
    {
        uint32_t offset = 0;
        for(const Mesh& i : _meshes)
        {
            const Vector<element_index_t>& indices = i.indices();
            const uint32_t size = static_cast<uint32_t>(indices.size() * sizeof(element_index_t));
            uploader.write(indices.data(), size, offset);
            offset += size;
        }
    }

    bool update(uint32_t tick) override
    {
        return false;
    }

private:
    Vector<sp<Mesh>> _meshes;
};

class MeshVertices final : public Vertices {
public:
    MeshVertices(Vector<sp<Mesh>> meshes)
        : Vertices(calcVertexCount(meshes)), _meshes(std::move(meshes))
    {
    }

    void write(VertexWriter& buf, const V3& size) override
    {
        for(const Mesh& m : _meshes)
            m.write(buf);
    }

private:
    Vector<sp<Mesh>> _meshes;
};

struct NodeLayout {
    NodeLayout()
        : _node(nullptr), _transform(M4::identity()) {
    }
    NodeLayout(const Node& node, const NodeLayout& parentLayout)
        : _node(&node), _transform(parentLayout._transform* node.localMatrix()) {
    }

    void calculateTransformedBoundingAABB(const V3& a0, const V3& a1, V3& aabbMin, V3& aabbMax) const {
        calcTransformedPosition(a0, aabbMin, aabbMax);
        calcTransformedPosition(a1, aabbMin, aabbMax);
    }

    void calcTransformedPosition(const V3& p0, V3& aabbMin, V3& aabbMax) const {
        const V3 tp = _transform * p0;
        for(size_t i = 0; i < 3; ++i) {
            if(aabbMin[i] > tp[i])
                aabbMin[i] = tp[i];
            else if(aabbMax[i] < tp[i])
                aabbMax[i] = tp[i];
        }
    }

    const Node* _node;
    M4 _transform;
};

}

Model::Model(sp<Uploader> indices, sp<Vertices> vertices, sp<Boundaries> content, sp<Boundaries> occupy, Table<String, sp<Animation>> animations)
    : _indices(std::move(indices)), _vertices(std::move(vertices)), _content(std::move(content)), _occupy(occupy ? std::move(occupy) : sp<Boundaries>(_content)), _animations(std::move(animations))
{
}

Model::Model(Vector<sp<Material>> materials, Vector<sp<Mesh>> meshes, sp<Node> rootNode, sp<Boundaries> bounds, sp<Boundaries> occupy, Table<String, sp<Animation>> animations)
    : _indices(sp<Uploader>::make<InputMeshIndices>(meshes)), _vertices(sp<Vertices>::make<MeshVertices>(meshes)), _root_node(std::move(rootNode)), _materials(std::move(materials)), _meshes(std::move(meshes)),
      _content(bounds ? std::move(bounds) : sp<Boundaries>::make(calculateBoundingAABB())), _occupy(occupy ? std::move(occupy) : sp<Boundaries>(_content)), _animations(std::move(animations))
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

element_index_t Model::writeIndices(element_index_t* buf, const element_index_t baseIndex) const
{
    UploaderType::writeTo(_indices, buf);
    const element_index_t length = static_cast<element_index_t>(_indices->size() / sizeof(element_index_t));
    if(baseIndex != 0)
        for(size_t i = 0; i < length; ++i)
            buf[i] += baseIndex;
    return length;
}

const Vector<sp<Material>>& Model::materials() const
{
    return _materials;
}

const Vector<sp<Mesh>>& Model::meshes() const
{
    return _meshes;
}

const sp<Node>& Model::rootNode() const
{
    return _root_node;
}

sp<Node> Model::findNode(const String& name) const
{
    ASSERT(_root_node);
    return _root_node->findChildNode(name);
}

const sp<Boundaries>& Model::content() const
{
    return _content;
}

const sp<Boundaries>& Model::occupy() const
{
    return _occupy;
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

const sp<Animation>& Model::getAnimation(const String& name) const
{
    CHECK(_animations.has(name), "Model has no animate(%s) defined", name.c_str());
    return _animations.at(name);
}

void Model::writeToStream(VertexWriter& writer, const V3& size) const
{
    _vertices->write(writer, size == V3(0) && _content ? _content->size()->val() : size);
}

void Model::writeRenderable(VertexWriter& writer, const Renderable::Snapshot& renderable) const
{
    writer.setRenderable(renderable);
    writeToStream(writer, renderable._size);
}

void Model::discard()
{
    _indices = nullptr;
    _vertices = nullptr;
    _meshes.clear();
    _animations.clear();
}

bool Model::isDiscarded() const
{
    return !static_cast<bool>(_indices);
}

Boundaries Model::calculateBoundingAABB() const
{
    Vector<NodeLayout> nodeLayouts;
    loadFlatLayouts(_root_node, NodeLayout(), nodeLayouts);

    V3 aabbMin(std::numeric_limits<float>::max()), aabbMax(std::numeric_limits<float>::min());
    for(const NodeLayout& i : nodeLayouts)
    {
        const auto [p0, p1] = i._node->localAABB();
        i.calculateTransformedBoundingAABB(p0, p1, aabbMin, aabbMax);
    }

    return {aabbMin, aabbMax};
}

}
