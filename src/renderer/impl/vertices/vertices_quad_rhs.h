#pragma once

#include "graphics/base/rect.h"

#include "renderer/base/atlas.h"
#include "renderer/inf/vertices.h"

namespace ark {

class VerticesQuadRHS final : public Vertices {
public:
    VerticesQuadRHS(const Atlas::Item& quad);
    VerticesQuadRHS(const Rect& bounds, uint16_t ux, uint16_t uy, uint16_t vx, uint16_t vy);

    void write(VertexWriter& buf, const V3& size) override;

private:
    Rect _bounds;
    uint16_t _ux, _uy;
    uint16_t _vx, _vy;
};

}
