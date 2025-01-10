#include "renderer/impl/vertices/vertices_quad_rhs.h"

#include "renderer/base/vertex_writer.h"

namespace ark {

VerticesQuadRHS::VerticesQuadRHS(const Atlas::Item& quad)
    : Vertices(4), _bounds(quad._bounds), _ux(quad._ux), _uy(quad._uy), _vx(quad._vx), _vy(quad._vy)
{
}

VerticesQuadRHS::VerticesQuadRHS(const Rect& bounds, uint16_t ux, uint16_t uy, uint16_t vx, uint16_t vy)
    : Vertices(4), _bounds(bounds), _ux(ux), _uy(uy), _vx(vx), _vy(vy)
{
}

void VerticesQuadRHS::write(VertexWriter& buf, const V3& size)
{
    const float width = size.x();
    const float height = size.y();

    buf.next();
    buf.writePosition({_bounds.left() * width, _bounds.top() * height, 0});
    buf.writeTexCoordinate(_ux, _uy);

    buf.next();
    buf.writePosition({_bounds.left() * width, _bounds.bottom() * height, 0});
    buf.writeTexCoordinate(_ux, _vy);

    buf.next();
    buf.writePosition({_bounds.right() * width, _bounds.top() * height, 0});
    buf.writeTexCoordinate(_vx, _uy);

    buf.next();
    buf.writePosition({_bounds.right() * width, _bounds.bottom() * height, 0});
    buf.writeTexCoordinate(_vx, _vy);
}

}
