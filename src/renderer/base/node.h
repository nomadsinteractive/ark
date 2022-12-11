#ifndef ARK_RENDERER_BASE_NODE_H_
#define ARK_RENDERER_BASE_NODE_H_

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
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Node);

//  [[script::bindings::property]]
    const String& name() const;

//  [[script::bindings::property]]
    const std::vector<sp<Node>>& childNodes() const;
    std::vector<sp<Node>>& childNodes();

//  [[script::bindings::property]]
    const std::vector<sp<Mesh>>& meshes() const;
    std::vector<sp<Mesh>>& meshes();

    const M4& transform() const;

private:
    String _name;

    std::vector<sp<Node>> _child_nodes;
    std::vector<sp<Mesh>> _meshes;

    M4 _transform;
};

}

#endif
