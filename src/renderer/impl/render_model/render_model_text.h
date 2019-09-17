#ifndef ARK_RENDERER_IMPL_RENDER_MODEL_RENDER_MODEL_TEXT_H_
#define ARK_RENDERER_IMPL_RENDER_MODEL_RENDER_MODEL_TEXT_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/base/texture.h"
#include "renderer/inf/render_model.h"

namespace ark {

class RenderModelText : public RenderModel {
private:
    class Stub : public Variable<sp<Texture::Delegate>> {
    public:
        Stub(const sp<RenderController>& renderController, const sp<Alphabet>& alphabet, uint32_t textureWidth, uint32_t textureHeight);

        void reset(uint32_t textureWidth, uint32_t textureHeight);

        bool checkUnpreparedCharacter(const RenderLayer::Snapshot& renderContext);
        bool prepare(const RenderLayer::Snapshot& snapshot, bool allowReset);

        virtual sp<Texture::Delegate> val() override;

    private:
        bool prepareOne(int32_t c);
        void clear();

    private:
        sp<RenderController> _render_controller;
        sp<Alphabet> _alphabet;
        bitmap _font_glyph;
        sp<Texture> _texture;
        sp<Atlas> _atlas;
        sp<RenderModel> _delegate;
        sp<Size> _size;

        uint32_t _flowx, _flowy;
        int32_t _max_glyph_height;

        friend class RenderModelText;
    };

public:
    RenderModelText(const sp<RenderController>& renderController, const sp<Alphabet>& alphabet, uint32_t textureWidth, uint32_t textureHeight);

    virtual sp<ShaderBindings> makeShaderBindings(const Shader& shader) override;

    virtual void postSnapshot(RenderController& renderController, RenderLayer::Snapshot& snapshot) override;

    virtual void start(DrawingBuffer& buf, const RenderLayer::Snapshot& snapshot) override;
    virtual void load(DrawingBuffer& buf, const RenderObject::Snapshot& snapshot) override;
    virtual Metrics measure(int32_t type) override;

//  [[plugin::resource-loader("text")]]
    class BUILDER : public Builder<RenderModel> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<RenderModel> build(const sp<Scope>& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;
        sp<Builder<Alphabet>> _alphabet;
        uint32_t _texture_width, _texture_height;
    };

private:
    sp<Stub> _stub;
    sp<Texture> _shader_texture;
};

}

#endif
