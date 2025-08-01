#include "graphics/inf/layout.h"

#include "core/util/updatable_util.h"

#include "graphics/components/layout_param.h"

namespace ark {

Layout::Node::Node(sp<LayoutParam> layoutParam, void* tag)
    : _layout_param(std::move(layoutParam)), _tag(tag)
{
    if(_layout_param)
        setSize(V2(_layout_param->contentWidth(), _layout_param->contentHeight()));
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

bool Layout::Node::isAutoWidth() const
{
    return !_layout_param || _layout_param->width().isAuto();
}

const sp<Numeric>& Layout::Node::autoWidth() const
{
    return _auto_width;
}

void Layout::Node::setAutoWidth(sp<Numeric> autoWidth)
{
    _auto_width = std::move(autoWidth);
}

bool Layout::Node::isAutoHeight() const
{
    return !_layout_param || _layout_param->height().isAuto();
}

const sp<Numeric>& Layout::Node::autoHeight() const
{
    return _auto_height;
}

void Layout::Node::setAutoHeight(sp<Numeric> autoHeight)
{
    _auto_height = std::move(autoHeight);
}

bool Layout::Node::update(const uint32_t timestamp)
{
    const bool dirty = UpdatableUtil::update(timestamp,_offset_position, _size);
    if(_layout_param && UpdatableUtil::update(timestamp, _layout_param->margins(), _layout_param->paddings()))
    {
        _margins = _layout_param->margins().val();
        _paddings = _layout_param->paddings().val();
        return true;
    }
    return dirty;
}

}
