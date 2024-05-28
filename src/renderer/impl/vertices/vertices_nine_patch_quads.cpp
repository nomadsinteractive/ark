#include "renderer/impl/vertices/vertices_nine_patch_quads.h"

#include "renderer/base/vertex_writer.h"

namespace ark {

VerticesNinePatchQuads::VerticesNinePatchQuads()
    : VerticesNinePatch(36)
{
}

VerticesNinePatchQuads::VerticesNinePatchQuads(const Rect& bounds, const Rect& patches, uint32_t textureWidth, uint32_t textureHeight)
    : VerticesNinePatch(36, bounds, patches, textureWidth, textureHeight)
{
}

void VerticesNinePatchQuads::write(VertexWriter& buf, const V3& size)
{
    const Rect paintRect(0, 0, std::max(_paddings.left() + _paddings.right(), size.x()), std::max(_paddings.top() + _paddings.bottom(), size.y()));
    const float xData[4] = {paintRect.left(), paintRect.left() + _paddings.left(), paintRect.right() - _paddings.right(), paintRect.right()};
    const float yData[4] = {paintRect.bottom(), paintRect.bottom() - _paddings.top(), paintRect.top() + _paddings.bottom(), paintRect.top()};
    for(uint32_t i = 0; i < 3; i++) {
        for(uint32_t j = 0; j < 3; j++) {
            buf.next();
            buf.writePosition(V3(xData[j], yData[i], 0));
            buf.writeTexCoordinate(_x[j], _y[i]);
            buf.next();
            buf.writePosition(V3(xData[j + 1], yData[i], 0));
            buf.writeTexCoordinate(_x[j + 1], _y[i]);
            buf.next();
            buf.writePosition(V3(xData[j], yData[i + 1], 0));
            buf.writeTexCoordinate(_x[j], _y[i + 1]);
            buf.next();
            buf.writePosition(V3(xData[j + 1], yData[i + 1], 0));
            buf.writeTexCoordinate(_x[j + 1], _y[i + 1]);
        }
    }
}

}
