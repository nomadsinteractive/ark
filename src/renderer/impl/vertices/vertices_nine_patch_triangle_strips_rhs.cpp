#include "renderer/impl/vertices/vertices_nine_patch_triangle_strips_rhs.h"

#include "renderer/base/vertex_writer.h"

namespace ark {

VerticesNinePatchTriangleStripsRHS::VerticesNinePatchTriangleStripsRHS()
    : VerticesNinePatch(16)
{
}

VerticesNinePatchTriangleStripsRHS::VerticesNinePatchTriangleStripsRHS(const Rect& bounds, const Rect& patches, uint32_t textureWidth, uint32_t textureHeight)
    : VerticesNinePatch(16, bounds, patches, textureWidth, textureHeight)
{
}

void VerticesNinePatchTriangleStripsRHS::write(VertexWriter& buf, const V3& size)
{
    const Rect content(0, 0, std::max(_paddings.left() + _paddings.right(), size.x()), std::max(_paddings.top() + _paddings.bottom(), size.y()));
    const Rect paintRect = content.translate(-size.x() / 2, -size.y() / 2);
    const float xData[4] = {paintRect.left(), paintRect.left() + _paddings.left(), paintRect.right() - _paddings.right(), paintRect.right()};
    const float yData[4] = {paintRect.bottom(), paintRect.bottom() - _paddings.top(), paintRect.top() + _paddings.bottom(), paintRect.top()};
    for(uint32_t i = 0; i < 4; i++) {
        for(uint32_t j = 0; j < 4; j++) {
            buf.next();
            buf.writePosition(V3(xData[j], yData[i], 0));
            buf.writeTexCoordinate(_x[j], _y[i]);
        }
    }
}

}
