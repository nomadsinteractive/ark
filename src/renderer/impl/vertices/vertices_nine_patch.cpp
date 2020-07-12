#include "renderer/impl/vertices/vertices_nine_patch.h"

#include "renderer/base/vertex_stream.h"

namespace ark {

VerticesNinePatch::VerticesNinePatch()
    : Vertices(16)
{
}

VerticesNinePatch::VerticesNinePatch(const Rect& bounds, const Rect& patches, uint32_t textureWidth, uint32_t textureHeight)
    : Vertices(16), _paddings(patches)
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

void VerticesNinePatch::write(VertexStream& buf, const V3& size)
{
    const Rect paintRect(0, 0, std::max(_paddings.left() + _paddings.right(), size.x()), std::max(_paddings.top() + _paddings.bottom(), size.y()));
    float xData[4] = {paintRect.left(), paintRect.left() + _paddings.left(), paintRect.right() - _paddings.right(), paintRect.right()};
    float yData[4] = {paintRect.bottom(), paintRect.bottom() - _paddings.top(), paintRect.top() + _paddings.bottom(), paintRect.top()};
    for(uint32_t i = 0; i < 4; i++) {
        for(uint32_t j = 0; j < 4; j++) {
            buf.next();
            buf.writePosition(V3(xData[j], yData[i], 0), i * 4 + j);
            buf.writeTexCoordinate(_x[j], _y[i]);
        }
    }
}

}
