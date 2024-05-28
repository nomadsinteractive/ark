#pragma once

#include "graphics/base/rect.h"

#include "renderer/impl/vertices/vertices_nine_patch.h"

namespace ark {

class VerticesNinePatchQuads : public VerticesNinePatch {
public:
    VerticesNinePatchQuads();
    VerticesNinePatchQuads(const Rect& bounds, const Rect& patches, uint32_t textureWidth, uint32_t textureHeight);

    void write(VertexWriter& buf, const V3& size) override;

};

}
