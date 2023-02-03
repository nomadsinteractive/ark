#include "app/inf/layout.h"

namespace ark {

LayoutV3::Node::Node(sp<LayoutParam> layoutParam, sp<ViewHierarchy> viewHierarchy, void* tag)
    : _layout_param(std::move(layoutParam)), _view_hierarchy(std::move(viewHierarchy)), _tag(tag)
{
}

float LayoutV3::Node::contentWidth() const
{
    return _size->x() + _paddings.y() + _paddings.w();
}

float LayoutV3::Node::contentHeight() const
{
    return _size->y() + _paddings.x() + _paddings.z();
}

const V4& LayoutV3::Node::paddings() const
{
    return _paddings;
}

void LayoutV3::Node::setPaddings(const V4& paddings)
{
    _paddings = paddings;
}

const WithTimestamp<V2>& LayoutV3::Node::offsetPosition() const
{
    return _offset_position;
}

void LayoutV3::Node::setOffsetPosition(const V2& offsetPosition)
{
    _offset_position.reset(offsetPosition);
}

const WithTimestamp<V2>& LayoutV3::Node::size() const
{
    return _size;
}

void LayoutV3::Node::setSize(const V2& size)
{
    _size.reset(size);
}

}
