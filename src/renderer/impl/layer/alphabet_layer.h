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
        Stub(const sp<Alphabet>& alphabet, const sp<GLResourceManager>& glResourceManager, uint32_t textureWidth, uint32_t textureHeight);

        const sp<Alphabet>& alphabet() const;
        const sp<Atlas>& atlas() const;

        bool prepare(uint32_t c, bool allowOverflow);
        bool checkUnpreparedCharacter(const LayerContext::Snapshot& renderContext);

        void reset();

        void doPrepare(const LayerContext::Snapshot& renderContext, bool allowReset);
        void prepareTexture(GLResourceManager& glResourceManager) const;

    private:
        bool hasCharacterGlyph(uint32_t c) const;

    private:
        sp<Alphabet> _alphabet;
        bitmap _font_glyph;
        sp<GLTexture> _texture;
        sp<Atlas> _atlas;
        uint32_t _flowx, _flowy;
        int32_t _max_glyph_height;

        std::unordered_set<uint32_t> _characters;
    };

public:
    AlphabetLayer(const sp<Alphabet>& alphabet, uint32_t textureWidth, uint32_t textureHeight, const sp<GLShader>& shader, const sp<ResourceLoaderContext>& resourceLoaderContext);

    virtual sp<RenderCommand> render(const LayerContext::Snapshot& layerContext, float x, float y) override;

    const sp<Alphabet>& alphabet() const;
    const sp<Atlas>& atlas() const;

//  [[plugin::resource-loader("alphabet-layer")]]
    class BUILDER : public Builder<Layer> {
    public:
        BUILDER(BeanFactory& parent, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        sp<Layer> build(const sp<Scope>& args);

    private:
        sp<ResourceLoaderContext> _resource_loader_context;
        sp<Builder<Alphabet>> _alphabet;
        sp<Builder<GLShader>> _shader;
        sp<Builder<VV4>> _color;

        uint32_t _texture_width, _texture_height;
    };

private:
    void doPrepare(bool allowReset);

private:
    sp<Stub> _stub;
    sp<ResourceLoaderContext> _resource_loader_context;
    sp<GLShader> _shader;
    sp<Layer> _layer;
    sp<Atlas> _atlas;
};

}

#endif
