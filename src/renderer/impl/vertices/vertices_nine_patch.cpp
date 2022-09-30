#include "renderer/impl/vertices/vertices_nine_patch.h"

#include "renderer/base/vertex_writer.h"

namespace ark {

VerticesNinePatch::VerticesNinePatch(size_t length)
    : Vertices(length)
{
}

VerticesNinePatch::VerticesNinePatch(size_t length, const Rect& bounds, const Rect& patches, uint32_t textureWidth, uint32_t textureHeight)
    : Vertices(length), _paddings(patches)
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

}
