#pragma once

#include "graphics/base/rect.h"

#include "renderer/base/atlas.h"
#include "renderer/inf/vertices.h"

namespace ark {

class VerticesQuad final : public Vertices {
public:
    VerticesQuad();
    VerticesQuad(const Atlas::Item& quad);

    void write(VertexWriter& buf, const V3& size) override;

private:
    Rect _bounds;
    uint16_t _ux, _uy;
    uint16_t _vx, _vy;
};

}
