#ifndef ARK_RENDERER_IMPL_VERTICES_VERTICES_NINE_PATCH_H_
#define ARK_RENDERER_IMPL_VERTICES_VERTICES_NINE_PATCH_H_

#include "graphics/base/rect.h"

#include "renderer/base/atlas.h"
#include "renderer/inf/vertices.h"

namespace ark {

class VerticesNinePatch : public Vertices {
public:
    VerticesNinePatch();
    VerticesNinePatch(const Rect& bounds, const Rect& patches, uint32_t textureWidth, uint32_t textureHeight);

    virtual void write(VertexStream& buf, const V3& size) override;

private:
    uint16_t _x[4];
    uint16_t _y[4];

    Rect _paddings;
};

}


#endif
