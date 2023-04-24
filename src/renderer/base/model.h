#pragma once

#include "core/forwarding.h"
#include "core/collection/table.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/metrics.h"
#include "graphics/base/v3.h"
#include "graphics/inf/renderable.h"

#include "renderer/forwarding.h"
#include "renderer/base/node.h"
#include "renderer/inf/vertices.h"

namespace ark {

class ARK_API Model {
public:
    Model() = default;
    Model(sp<Uploader> indices, sp<Vertices> vertices, sp<Metrics> bounds, sp<Metrics> occupies = nullptr);
    Model(std::vector<sp<Material>> materials, std::vector<sp<Mesh>> meshes, sp<Node> rootNode, sp<Metrics> bounds, sp<Metrics> occupies = nullptr);
    DEFAULT_COPY_AND_ASSIGN(Model);

//  [[script::bindings::property]]
    const sp<Uploader>& indices() const;
//  [[script::bindings::property]]
    const sp<Vertices>& vertices() const;

    element_index_t writeIndices(element_index_t* buf, element_index_t baseIndex = 0) const;

//  [[script::bindings::property]]
    const std::vector<sp<Material>>& materials() const;
//  [[script::bindings::property]]
    const std::vector<sp<Mesh>>& meshes() const;
//  [[script::bindings::property]]
    const sp<Node>& rootNode() const;

//  [[script::bindings::property]]
    const sp<Metrics>& bounds() const;
//  [[script::bindings::property]]
    const sp<Metrics>& occupies() const;

//  [[script::bindings::property]]
    size_t indexCount() const;
//  [[script::bindings::property]]
    size_t vertexCount() const;

//  [[script::bindings::property]]
    const Table<String, sp<Animation>>& animations() const;
    void setAnimations(Table<String, sp<Animation>> animations);

//  [[script::bindings::property]]
    const std::vector<String>& nodeNames() const;
    void setNodeNames(std::vector<String> nodes);

//  [[script::bindings::auto]]
    const sp<Animation>& getAnimation(const String& name) const;

    void writeToStream(VertexWriter& buf, const V3& size) const;

    void writeRenderable(VertexWriter& buf, const Renderable::Snapshot& renderable) const;

    void dispose();
    bool isDisposed() const;

    template<typename T> std::vector<T> toFlatLayouts() const {
        std::vector<T> nodeLayouts;
        loadFlatLayouts(_root_node, T(), nodeLayouts);
        return nodeLayouts;
    }

    template<typename T> static void loadFlatLayouts(const sp<Node>& node, const T& parentLayout, std::vector<T>& nodeLayouts) {
        T layout(node, parentLayout);

        if(!node->name().empty())
            nodeLayouts.emplace_back(layout);

        for(const sp<Node>& childNode : node->childNodes())
            loadFlatLayouts(childNode, layout, nodeLayouts);
    }


private:
    class InputMeshIndices : public Uploader {
    public:
        InputMeshIndices(std::vector<sp<Mesh>> meshes);

        virtual void upload(Writable& uploader) override;
        virtual bool update(uint64_t timestamp) override;

    private:
        size_t calcIndicesSize(const std::vector<sp<Mesh>>& meshes) const;

    private:
        std::vector<sp<Mesh>> _meshes;

    };

    class MeshVertices : public Vertices {
    public:
        MeshVertices(std::vector<sp<Mesh>> meshes);

        virtual void write(VertexWriter& buf, const V3& size) override;

    private:
        size_t calcVertexLength(const std::vector<sp<Mesh>>& meshes) const;

    private:
        std::vector<sp<Mesh>> _meshes;
        V3 _size;
    };

private:
    sp<Uploader> _indices;
    sp<Vertices> _vertices;
    sp<Metrics> _bounds;
    sp<Metrics> _occupies;
    sp<Node> _root_node;
    std::vector<sp<Material>> _materials;
    std::vector<sp<Mesh>> _meshes;
    Table<String, sp<Animation>> _animations;
    std::vector<String> _node_names;
};

}
