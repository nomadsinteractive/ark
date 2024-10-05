#include "renderer/base/node.h"

#include "mesh.h"
#include "core/inf/variable.h"


namespace ark {

Node::Node(String name, const M4& transform)
    : _name(std::move(name)), _matrix(transform) {
}

Node::Node(String name, const V3& translation, const V4& rotation, const V3& scale)
    : _name(std::move(name)), _translation(translation), _rotation(rotation), _scale(scale), _matrix(MatrixUtil::scale(MatrixUtil::rotate(MatrixUtil::translate({}, translation), rotation), scale)) {
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

void Node::addMesh(sp<Mesh> mesh)
{
    mesh->_parent = this;
    _meshes.push_back(std::move(mesh));
}

const M4& Node::matrix() const
{
    return _matrix;
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

}
