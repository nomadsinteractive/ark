#include "renderer/impl/vertices/vertices_nine_patch.h"

#include "renderer/base/vertex_stream.h"

namespace ark {

VerticesNinePatch::Item::Item(const Rect& bounds, const Rect& patches, uint32_t textureWidth, uint32_t textureHeight)
    : _paddings(patches)
{
    _x[0] = Atlas::unnormalize(static_cast<uint32_t>(bounds.left()), textureWidth);
    _x[1] = Atlas::unnormalize(static_cast<uint32_t>(bounds.left() + patches.left()), textureWidth);
    _x[2] = Atlas::unnormalize(static_cast<uint32_t>(bounds.left() + patches.right()), textureWidth);
    _x[3] = Atlas::unnormalize(static_cast<uint32_t>(bounds.right()), textureWidth);

    _y[0] = Atlas::unnormalize(static_cast<uint32_t>(bounds.top()), textureHeight);
    _y[1] = Atlas::unnormalize(static_cast<uint32_t>(bounds.top() + patches.top()), textureHeight);
    _y[2] = Atlas::unnormalize(static_cast<uint32_t>(bounds.top() + patches.bottom()), textureHeight);
    _y[3] = Atlas::unnormalize(static_cast<uint32_t>(bounds.bottom()), textureHeight);

    _paddings.setRight(bounds.width() - patches.right());
    _paddings.setBottom(bounds.height() - patches.bottom());
}

VerticesNinePatch::VerticesNinePatch(const Rect& bounds, const Rect& patches, uint32_t textureWidth, uint32_t textureHeight)
    : Vertices(16), _item(bounds, patches, textureWidth, textureHeight)
{
}

void VerticesNinePatch::write(VertexStream& buf, const V3& size)
{
    const Rect& paddings = _item._paddings;
    const Rect paintRect(0, 0, std::max(paddings.left() + paddings.right(), size.x()), std::max(paddings.top() + paddings.bottom(), size.y()));
    float xData[4] = {paintRect.left(), paintRect.left() + paddings.left(), paintRect.right() - paddings.right(), paintRect.right()};
    float yData[4] = {paintRect.bottom(), paintRect.bottom() - paddings.top(), paintRect.top() + paddings.bottom(), paintRect.top()};
    for(uint32_t i = 0; i < 4; i++) {
        for(uint32_t j = 0; j < 4; j++) {
            buf.next();
            buf.writePosition(xData[j], yData[i], 0);
            buf.writeTexCoordinate(_item._x[j], _item._y[i]);
        }
    }
}

}
