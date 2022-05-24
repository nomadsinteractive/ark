#ifndef ARK_RENDERER_IMPL_VERTICES_VERTICES_NINE_PATCH_QUADS_H_
#define ARK_RENDERER_IMPL_VERTICES_VERTICES_NINE_PATCH_QUADS_H_

#include "graphics/base/rect.h"

#include "renderer/impl/vertices/vertices_nine_patch.h"

namespace ark {

class VerticesNinePatchQuads : public VerticesNinePatch {
public:
    VerticesNinePatchQuads();
    VerticesNinePatchQuads(const Rect& bounds, const Rect& patches, uint32_t textureWidth, uint32_t textureHeight);

    virtual void write(VertexStream& buf, const V3& size) override;

};

}


#endif
