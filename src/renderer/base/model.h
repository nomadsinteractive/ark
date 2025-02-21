#pragma once

#include "core/forwarding.h"
#include "core/collection/table.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/boundaries.h"
#include "graphics/base/v3.h"
#include "graphics/inf/renderable.h"

#include "renderer/forwarding.h"
#include "renderer/base/node.h"
#include "renderer/inf/vertices.h"

namespace ark {

class ARK_API Model {
public:
    Model() = default;
    Model(sp<Uploader> indices, sp<Vertices> vertices, sp<Boundaries> content, sp<Boundaries> occupy = nullptr, Table<String, sp<Animation>> animations = {});
    Model(Vector<sp<Material>> materials, Vector<sp<Mesh>> meshes, sp<Node> rootNode, sp<Boundaries> bounds = nullptr, sp<Boundaries> occupy = nullptr, Table<String, sp<Animation>> animations = {});
    DEFAULT_COPY_AND_ASSIGN(Model);

//  [[script::bindings::property]]
    const sp<Uploader>& indices() const;
//  [[script::bindings::property]]
    const sp<Vertices>& vertices() const;

    element_index_t writeIndices(element_index_t* buf, element_index_t baseIndex = 0) const;

//  [[script::bindings::property]]
    const Vector<sp<Material>>& materials() const;
//  [[script::bindings::property]]
    const Vector<sp<Mesh>>& meshes() const;
//  [[script::bindings::property]]
    const sp<Node>& rootNode() const;

//  [[script::bindings::auto]]
    sp<Node> findNode(const String& name) const;

//  [[script::bindings::property]]
    const sp<Boundaries>& content() const;
//  [[script::bindings::property]]
    const sp<Boundaries>& occupy() const;

//  [[script::bindings::property]]
    size_t indexCount() const;
//  [[script::bindings::property]]
    size_t vertexCount() const;

//  [[script::bindings::property]]
    const Table<String, sp<Animation>>& animations() const;
    void setAnimations(Table<String, sp<Animation>> animations);

//  [[script::bindings::auto]]
    const sp<Animation>& getAnimation(const String& name) const;

    void writeToStream(VertexWriter& buf, const V3& size) const;

    void writeRenderable(VertexWriter& buf, const Renderable::Snapshot& renderable) const;

    void dispose();
    bool isDiscarded() const;

    template<typename T> Vector<T> toFlatLayouts() const {
        Vector<T> nodeLayouts;
        loadFlatLayouts(_root_node, T(), nodeLayouts);
        return nodeLayouts;
    }

private:
    template<typename T> static void loadFlatLayouts(const sp<Node>& node, const T& parentLayout, Vector<T>& nodeLayouts) {
        T layout(node, parentLayout);

        if(!node->name().empty())
            nodeLayouts.emplace_back(layout);

        for(const sp<Node>& childNode : node->childNodes())
            loadFlatLayouts(childNode, layout, nodeLayouts);
    }

    Boundaries calcBoundingAABB() const;

private:
    sp<Uploader> _indices;
    sp<Vertices> _vertices;
    sp<Node> _root_node;
    Vector<sp<Material>> _materials;
    Vector<sp<Mesh>> _meshes;
    Vector<String> _node_names;
    sp<Boundaries> _content;
    sp<Boundaries> _occupy;

    Table<String, sp<Animation>> _animations;
};

}
