#include "renderer/impl/vertices/vertices_point.h"

#include "renderer/base/vertex_writer.h"

namespace ark {

VerticesPoint::VerticesPoint()
    : Vertices(1)
{
}

VerticesPoint::VerticesPoint(const Atlas::Item& atlasItem)
    : Vertices(1), _atlas_item(atlasItem)
{
}

void VerticesPoint::write(VertexWriter& buf, const V3& /*size*/)
{
    buf.next();
    buf.writePosition(V3(0));
    buf.writeTexCoordinate(static_cast<uint16_t>((_atlas_item.ux() + _atlas_item.vx()) / 2), static_cast<uint16_t>((_atlas_item.uy() + _atlas_item.vy()) / 2));
}

}
