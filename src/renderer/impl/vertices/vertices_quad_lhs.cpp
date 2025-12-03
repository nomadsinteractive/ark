#include "renderer/impl/vertices/vertices_quad_lhs.h"

#include "renderer/base/vertex_writer.h"

namespace ark {

VerticesQuadLHS::VerticesQuadLHS(const Atlas::Item& quad)
    : Vertices(4), _bounds(quad._bounds), _ux(quad._uv._ux), _uy(quad._uv._uy), _vx(quad._uv._vx), _vy(quad._uv._vy)
{
}

VerticesQuadLHS::VerticesQuadLHS(const Rect& bounds, const uint16_t ux, const uint16_t uy, const uint16_t vx, const uint16_t vy)
    : Vertices(4), _bounds(bounds), _ux(ux), _uy(uy), _vx(vx), _vy(vy)
{
}

void VerticesQuadLHS::write(VertexWriter& buf, const V3& size)
{
    const float width = size.x();
    const float height = size.y();

    buf.next();
    buf.writePosition({_bounds.left() * width, _bounds.top() * height, 0});
    buf.writeTexCoordinate(_ux, _vy);

    buf.next();
    buf.writePosition({_bounds.right() * width, _bounds.top() * height, 0});
    buf.writeTexCoordinate(_vx, _vy);

    buf.next();
    buf.writePosition({_bounds.left() * width, _bounds.bottom() * height, 0});
    buf.writeTexCoordinate(_ux, _uy);

    buf.next();
    buf.writePosition({_bounds.right() * width, _bounds.bottom() * height, 0});
    buf.writeTexCoordinate(_vx, _uy);
}

}
