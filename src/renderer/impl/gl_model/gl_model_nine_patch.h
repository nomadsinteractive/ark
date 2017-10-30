#ifndef ARK_RENDERER_IMPL_GL_MODEL_GL_MODEL_NINE_PATCH_H_
#define ARK_RENDERER_IMPL_GL_MODEL_GL_MODEL_NINE_PATCH_H_

#include "core/collection/by_index.h"

#include "graphics/forwarding.h"
#include "graphics/base/rect.h"

#include "renderer/forwarding.h"
#include "renderer/inf/gl_model.h"

namespace ark {

class GLModelNinePatch : public GLModel {
private:
    struct Item {
        Item(const Rect& bounds, const Rect& stretching, uint32_t textureWidth, uint32_t textureHeight);
        Item(const Item& other);
        Item();

        uint16_t _x[4];
        uint16_t _y[4];

        Rect paddings;
    };

public:
    GLModelNinePatch(const sp<GLShader>& shader, const document& manifest, const sp<Atlas>& atlas);

    virtual array<uint8_t> getArrayBuffer(GLResourceManager& resourceManager, const LayerContext& renderContext, float x, float y) override;
    virtual GLBuffer getIndexBuffer(GLResourceManager& glResourceManager, const LayerContext& renderContext) override;

    virtual uint32_t mode() const override;

private:
    void fillPaintingRect(float* buf, const Rect& paintRect, const Item& bounds, uint32_t floatStride, float x, float y) const;
    void fillMesh(float* mesh, Array<float>& xArray, Array<float>& yArray, uint32_t stride, uint32_t offset, float x, float y) const;
    void fillTexCoord(void* mesh, const Item& item, uint32_t shortStride, uint32_t offset) const;

private:
    ByIndex<Item> _nine_patch_items;
    uint32_t _tex_coordinate_offset;
    uint32_t _stride;

};

}

#endif
