#ifndef ARK_RENDERER_IMPL_LAYER_ALPHABET_LAYER_H_
#define ARK_RENDERER_IMPL_LAYER_ALPHABET_LAYER_H_

#include "core/inf/builder.h"

#include "graphics/forwarding.h"
#include "graphics/inf/layer.h"

#include "renderer/forwarding.h"
#include "renderer/inf/gl_resource.h"

namespace ark {

class AlphabetLayer : public Layer {
private:
    class Stub {
    public:
        Stub(const sp<Alphabet>& alphabet, const sp<ResourceLoaderContext>& resourceLoaderContext, const sp<GLShader>& shader, uint32_t textureWidth, uint32_t textureHeight);

        const sp<Alphabet>& alphabet() const;
        const sp<Atlas>& atlas() const;

        sp<RenderCommand> render(const Layer::Snapshot& renderContext, float x, float y) const;

        bool checkUnpreparedCharacter(const Layer::Snapshot& renderContext);

        void reset();

        bool prepare(const Layer::Snapshot& renderContext, bool allowReset);
        void prepareTexture() const;

    private:
        bool hasCharacterGlyph(int32_t c) const;
        bool prepareOne(int32_t c);

    private:
        sp<Alphabet> _alphabet;
        sp<GLResourceManager> _resource_manager;
        bitmap _font_glyph;
        sp<GLTexture> _texture;
        sp<Atlas> _atlas;
        uint32_t _flowx, _flowy;
        int32_t _max_glyph_height;

        std::unordered_set<int32_t> _characters;
        sp<Layer> _layer;

        friend class AlphabetLayer;
    };

public:
    AlphabetLayer(const sp<Alphabet>& alphabet, uint32_t textureWidth, uint32_t textureHeight, const sp<GLShader>& shader, const sp<ResourceLoaderContext>& resourceLoaderContext);

    virtual sp<RenderCommand> render(const Layer::Snapshot& layerContext, float x, float y) override;

    const sp<Alphabet>& alphabet() const;

//  [[plugin::resource-loader("alphabet-layer")]]
    class BUILDER : public Builder<Layer> {
    public:
        BUILDER(BeanFactory& parent, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        sp<Layer> build(const sp<Scope>& args);

    private:
        sp<ResourceLoaderContext> _resource_loader_context;
        sp<Builder<Alphabet>> _alphabet;
        sp<Builder<GLShader>> _shader;
        sp<Builder<Vec4>> _color;

        uint32_t _texture_width, _texture_height;
    };

private:
    void doPrepare(bool allowReset);

private:
    sp<Stub> _stub;
    sp<ResourceLoaderContext> _resource_loader_context;
    sp<GLShader> _shader;
    sp<Atlas> _atlas;
};

}

#endif
