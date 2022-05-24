#ifndef ARK_RENDERER_IMPL_MODEL_LOADER_MODEL_LOADER_TEXT_H_
#define ARK_RENDERER_IMPL_MODEL_LOADER_MODEL_LOADER_TEXT_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/base/texture.h"
#include "renderer/inf/model_loader.h"

namespace ark {

class ModelLoaderText : public ModelLoader {
private:
    class Stub {
    public:
        Stub(const sp<RenderController>& renderController, const sp<Alphabet>& alphabet, uint32_t textureWidth, uint32_t textureHeight);
        Stub(const sp<RenderController>& renderController, const sp<Alphabet>& alphabet, sp<Texture> texture);

        void reset(uint32_t textureWidth, uint32_t textureHeight);

        bool checkUnpreparedCharacter(const RenderLayer::Snapshot& renderContext);
        bool prepare(const RenderLayer::Snapshot& snapshot, bool allowReset);

        void ensureCharacter(int32_t c);
        bool resize(uint32_t textureWidth, uint32_t textureHeight);

    private:
        void reloadTexture();
        bool prepareOne(int32_t c);
        void clear();

    private:
        sp<RenderController> _render_controller;
        sp<Alphabet> _alphabet;
        bitmap _font_glyph;
        sp<Atlas> _atlas;
        sp<ModelLoader> _delegate;
        sp<Size> _size;
        sp<Texture> _texture;
        sp<Future> _texture_reload_future;

        int32_t _flowx, _flowy;
        int32_t _max_glyph_height;

        friend class ModelLoaderText;
    };

public:
    ModelLoaderText(const sp<RenderController>& renderController, const sp<Alphabet>& alphabet, uint32_t textureWidth, uint32_t textureHeight);
    ModelLoaderText(const sp<RenderController>& renderController, const sp<Alphabet>& alphabet, sp<Texture> texture);

    virtual sp<RenderCommandComposer> makeRenderCommandComposer() override;

    virtual void initialize(ShaderBindings& shaderBindings) override;

    virtual sp<Model> loadModel(int32_t type) override;

//  [[plugin::resource-loader("text")]]
    class BUILDER : public Builder<ModelLoader> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<ModelLoader> build(const Scope& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;
        sp<Builder<Alphabet>> _alphabet;
        SafePtr<Builder<Texture>> _texture;
        uint32_t _texture_width, _texture_height;
    };

private:
    sp<Stub> _stub;

};

}

#endif
