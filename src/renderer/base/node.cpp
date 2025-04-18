#include "renderer/base/node.h"

#include "renderer/base/mesh.h"

namespace ark {

Node::Node(WeakPtr<Node> parentNode, String name, const M4& transform)
    : _parent_node(std::move(parentNode)), _name(std::move(name)), _local_matrix(transform) {
}

Node::Node(WeakPtr<Node> parentNode, String name, const V3& translation, const V4& rotation, const V3& scale)
    : _parent_node(std::move(parentNode)), _name(std::move(name)), _translation(translation), _rotation(rotation), _scale(scale), _local_matrix(MatrixUtil::scale(MatrixUtil::rotate(MatrixUtil::translate({}, translation), rotation), scale)) {
}

const String& Node::name() const
{
    return _name;
}

sp<Node> Node::parentNode() const
{
    return _parent_node.lock();
}

const Vector<sp<Node>>& Node::childNodes() const
{
    return _child_nodes;
}

Vector<sp<Node>>& Node::childNodes()
{
    return _child_nodes;
}

const Vector<sp<Mesh>>& Node::meshes() const
{
    return _meshes;
}

void Node::addMesh(sp<Mesh> mesh)
{
    mesh->_parent = this;
    _meshes.push_back(std::move(mesh));
}

const M4& Node::localMatrix() const
{
    return _local_matrix;
}

M4 Node::globalMatrix() const
{
    M4 m = _local_matrix;
    sp<Node> parent = _parent_node.lock();
    while(parent)
    {
        m = MatrixUtil::mul(parent->localMatrix(), m);
        parent = parent->parentNode();
    }
    return m;
}

const V3& Node::translation() const
{
    return _translation;
}

const V4& Node::rotation() const
{
    return _rotation;
}

const V3& Node::scale() const
{
    return _scale;
}

sp<Node> Node::findChildNode(const String& name) const
{
    for(const sp<Node>& i : _child_nodes)
    {
        if(i->name() == name)
            return i;
        if(sp<Node> childHit = i->findChildNode(name))
            return childHit;
    }
    return nullptr;
}

std::pair<V3, V3> Node::localAABB() const
{
    return {_local_aabb_min, _local_aabb_max};
}

void Node::calculateLocalAABB()
{
    _local_aabb_min = {std::numeric_limits<float>::max()};
    _local_aabb_max = {std::numeric_limits<float>::min()};

    for(const sp<Mesh>& i : _meshes)
    {
        const auto [mMin, mMax] = i->calculateBoundingAABB();
        _local_aabb_min = {std::min(mMin.x(), _local_aabb_min.x()), std::min(mMin.y(), _local_aabb_min.y()), std::min(mMin.z(), _local_aabb_min.z())};
        _local_aabb_max = {std::max(mMax.x(), _local_aabb_max.x()), std::max(mMax.y(), _local_aabb_max.y()), std::max(mMax.z(), _local_aabb_max.z())};
    }
}

}
