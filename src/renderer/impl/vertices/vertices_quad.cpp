#include "renderer/impl/vertices/vertices_quad.h"

#include "renderer/base/vertex_writer.h"

namespace ark {

VerticesQuad::VerticesQuad()
    : Vertices(4)
{
}

VerticesQuad::VerticesQuad(const Atlas::Item& quad)
    : Vertices(4), _bounds(quad.bounds()), _ux(quad.ux()), _uy(quad.uy()), _vx(quad.vx()), _vy(quad.vy())
{
}

void VerticesQuad::write(VertexWriter& buf, const V3& size)
{
    const float width = size.x();
    const float height = size.y();

    buf.next();
    buf.writePosition(V3(_bounds.left() * width, _bounds.top() * height, 0));
    buf.writeTexCoordinate(_ux, _uy);

    buf.next();
    buf.writePosition(V3(_bounds.left() * width, _bounds.bottom() * height, 0));
    buf.writeTexCoordinate(_ux, _vy);

    buf.next();
    buf.writePosition(V3(_bounds.right() * width, _bounds.top() * height, 0));
    buf.writeTexCoordinate(_vx, _uy);

    buf.next();
    buf.writePosition(V3(_bounds.right() * width, _bounds.bottom() * height, 0));
    buf.writeTexCoordinate(_vx, _vy);
}

}
