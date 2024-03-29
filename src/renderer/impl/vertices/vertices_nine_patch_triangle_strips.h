#ifndef ARK_RENDERER_IMPL_VERTICES_VERTICES_NINE_PATCH_TRIANGLE_STRIP_H_
#define ARK_RENDERER_IMPL_VERTICES_VERTICES_NINE_PATCH_TRIANGLE_STRIP_H_

#include "graphics/base/rect.h"

#include "renderer/impl/vertices/vertices_nine_patch.h"

namespace ark {

class VerticesNinePatchTriangleStrips : public VerticesNinePatch {
public:
    VerticesNinePatchTriangleStrips();
    VerticesNinePatchTriangleStrips(const Rect& bounds, const Rect& patches, uint32_t textureWidth, uint32_t textureHeight);

    virtual void write(VertexWriter& buf, const V3& size) override;

};

}


#endif
