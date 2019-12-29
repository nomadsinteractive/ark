#ifndef ARK_RENDERER_IMPL_VERTICES_VERTICES_QUAD_H_
#define ARK_RENDERER_IMPL_VERTICES_VERTICES_QUAD_H_

#include "graphics/base/rect.h"

#include "renderer/base/atlas.h"
#include "renderer/inf/vertices.h"

namespace ark {

class VerticesQuad : public Vertices {
public:
    VerticesQuad();
    VerticesQuad(const Atlas::Item& quad);

    virtual void write(VertexStream& buf, const V3& size) override;

private:
    Rect _bounds;
    uint16_t _ux, _uy;
    uint16_t _vx, _vy;
};

}


#endif
