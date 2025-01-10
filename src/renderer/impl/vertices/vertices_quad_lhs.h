#pragma once

#include "graphics/base/rect.h"

#include "renderer/base/atlas.h"
#include "renderer/inf/vertices.h"

namespace ark {

class VerticesQuadLHS final : public Vertices {
public:
    VerticesQuadLHS(const Atlas::Item& quad);
    VerticesQuadLHS(const Rect& bounds, uint16_t ux, uint16_t uy, uint16_t vx, uint16_t vy);

    void write(VertexWriter& buf, const V3& size) override;

private:
    Rect _bounds;
    uint16_t _ux, _uy;
    uint16_t _vx, _vy;
};

}
