#pragma once

#include "core/base/string.h"
#include "core/types/weak_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/mat.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API Node {
public:
    Node(WeakPtr<Node> parentNode, String name, const M4& transform);
    Node(WeakPtr<Node> parentNode, String name, const V3& translation, const V4& rotation, const V3& scale);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Node);

//  [[script::bindings::property]]
    const String& name() const;

//  [[script::bindings::property]]
    sp<Node> parentNode() const;

//  [[script::bindings::property]]
    const Vector<sp<Node>>& childNodes() const;
    Vector<sp<Node>>& childNodes();

//  [[script::bindings::property]]
    const Vector<sp<Mesh>>& meshes() const;
    void addMesh(sp<Mesh> mesh);

//  [[script::bindings::property]]
    const M4& localMatrix() const;
//  [[script::bindings::property]]
    M4 globalMatrix() const;

//  [[script::bindings::property]]
    const V3& translation() const;
//  [[script::bindings::property]]
    const V4& rotation() const;
//  [[script::bindings::property]]
    const V3& scale() const;

//  [[script::bindings::auto]]
    sp<Node> findChildNode(const String& name) const;

//  [[script::bindings::property]]
    std::pair<V3, V3> localAABB() const;

    void calculateLocalAABB();

private:
    WeakPtr<Node> _parent_node;
    String _name;

    Vector<sp<Node>> _child_nodes;
    Vector<sp<Mesh>> _meshes;

    V3 _translation;
    V4 _rotation;
    V3 _scale;

    M4 _local_matrix;

    V3 _local_aabb_min;
    V3 _local_aabb_max;
};

}

