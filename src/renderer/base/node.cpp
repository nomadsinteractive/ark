#include "renderer/base/node.h"

namespace ark {

Node::Node(String name, const M4& transform)
    : _name(std::move(name)), _transform(transform) {
}

const String& Node::name() const
{
    return _name;
}

const std::vector<sp<Node>>& Node::childNodes() const
{
    return _child_nodes;
}

std::vector<sp<Node>>& Node::childNodes()
{
    return _child_nodes;
}

const std::vector<sp<Mesh>>& Node::meshes() const
{
    return _meshes;
}

std::vector<sp<Mesh>>& Node::meshes()
{
    return _meshes;
}

const M4& Node::transform() const
{
    return _transform;
}

}
