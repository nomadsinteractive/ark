#include "app/inf/layout.h"

#include "../traits/layout_param.h"

namespace ark {

Layout::Node::Node(sp<LayoutParam> layoutParam, sp<ViewHierarchy> viewHierarchy, void* tag)
    : _layout_param(std::move(layoutParam)), _view_hierarchy(std::move(viewHierarchy)), _tag(tag)
{
}

float Layout::Node::contentWidth() const
{
    return _size->x() + _paddings.y() + _paddings.w();
}

float Layout::Node::contentHeight() const
{
    return _size->y() + _paddings.x() + _paddings.z();
}

float Layout::Node::occupyWidth() const
{
    return contentWidth() + _margins.y() + _margins.w();
}

float Layout::Node::occupyHeight() const
{
    return contentHeight() + _margins.x() + _margins.z();
}

const V4& Layout::Node::paddings() const
{
    return _paddings;
}

void Layout::Node::setPaddings(const V4& paddings)
{
    _paddings = paddings;
}

const V4& Layout::Node::margins() const
{
    return _margins;
}

void Layout::Node::setMargins(const V4& margins)
{
    _margins = margins;
}

const WithTimestamp<V2>& Layout::Node::offsetPosition() const
{
    return _offset_position;
}

void Layout::Node::setOffsetPosition(const V2& offsetPosition)
{
    _offset_position.reset(offsetPosition);
}

const WithTimestamp<V2>& Layout::Node::size() const
{
    return _size;
}

void Layout::Node::setSize(const V2& size)
{
    _size.reset(size);
}

}
