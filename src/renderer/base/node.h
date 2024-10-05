#pragma once

#include <vector>

#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/mat.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API Node {
public:
    Node(String name, const M4& transform);
    Node(String name, const V3& translation, const V4& rotation, const V3& scale);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Node);

//  [[script::bindings::property]]
    const String& name() const;

//  [[script::bindings::property]]
    const std::vector<sp<Node>>& childNodes() const;
    std::vector<sp<Node>>& childNodes();

//  [[script::bindings::property]]
    const std::vector<sp<Mesh>>& meshes() const;
    void addMesh(sp<Mesh> mesh);

//  [[script::bindings::property]]
    const M4& matrix() const;

//  [[script::bindings::property]]
    const V3& translation() const;
//  [[script::bindings::property]]
    const V4& rotation() const;
//  [[script::bindings::property]]
    const V3& scale() const;

//  [[script::bindings::auto]]
    sp<Node> findChildNode(const String& name) const;

private:
    String _name;

    std::vector<sp<Node>> _child_nodes;
    std::vector<sp<Mesh>> _meshes;

    V3 _translation;
    V4 _rotation;
    V3 _scale;

    M4 _matrix;
};

}

