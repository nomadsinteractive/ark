#include "renderer/impl/vertices/vertices_nine_patch_triangle_strips.h"

#include "renderer/base/vertex_stream.h"

namespace ark {

VerticesNinePatchTriangleStrips::VerticesNinePatchTriangleStrips()
    : VerticesNinePatch(16)
{
}

VerticesNinePatchTriangleStrips::VerticesNinePatchTriangleStrips(const Rect& bounds, const Rect& patches, uint32_t textureWidth, uint32_t textureHeight)
    : VerticesNinePatch(16, bounds, patches, textureWidth, textureHeight)
{
}

void VerticesNinePatchTriangleStrips::write(VertexStream& buf, const V3& size)
{
    const Rect paintRect(0, 0, std::max(_paddings.left() + _paddings.right(), size.x()), std::max(_paddings.top() + _paddings.bottom(), size.y()));
    float xData[4] = {paintRect.left(), paintRect.left() + _paddings.left(), paintRect.right() - _paddings.right(), paintRect.right()};
    float yData[4] = {paintRect.bottom(), paintRect.bottom() - _paddings.top(), paintRect.top() + _paddings.bottom(), paintRect.top()};
    for(uint32_t i = 0; i < 4; i++) {
        for(uint32_t j = 0; j < 4; j++) {
            buf.next();
            buf.writePosition(V3(xData[j], yData[i], 0));
            buf.writeTexCoordinate(_x[j], _y[i]);
        }
    }
}

}
