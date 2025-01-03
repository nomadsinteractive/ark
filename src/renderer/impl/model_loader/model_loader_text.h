#pragma once

#include <vector>
#include <unordered_map>

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/font.h"
#include "graphics/util/max_rects_bin_pack.h"

#include "renderer/forwarding.h"
#include "renderer/base/texture.h"
#include "renderer/inf/model_loader.h"

namespace ark {

class ModelLoaderText final : public ModelLoader {
public:
    ModelLoaderText(sp<RenderController> renderController, sp<Alphabet> alphabet, sp<Atlas> atlas, const Font::TextSize& textSize);

    sp<RenderCommandComposer> makeRenderCommandComposer(const Shader& shader) override;
    sp<Model> loadModel(int32_t type) override;

//  [[plugin::resource-loader("text")]]
    class BUILDER : public Builder<ModelLoader> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        sp<ModelLoader> build(const Scope& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;
        sp<Builder<Alphabet>> _alphabet;
        sp<Builder<Atlas>> _atlas;
        SafeBuilder<String> _text_size;
    };

private:

    struct AtlasAttachment;

    struct GlyphModel {
        GlyphModel();
        GlyphModel(sp<Model> model, uint64_t timestamp);

        sp<Model> _model;
        uint64_t _timestamp;
    };

    struct GlyphBundle {
        GlyphBundle(AtlasAttachment& atlasAttachment, sp<Alphabet> alphabet, const Font::TextSize& textSize);

        const GlyphModel& ensureGlyphModel(uint64_t timestamp, int32_t c, bool reload);

        bool prepareOne(uint64_t timestamp, int32_t c, int32_t ckey);

        void reload(uint64_t timestamp);

        AtlasAttachment& _atlas_attachment;

        sp<Alphabet> _alphabet;
        sp<Model> _unit_glyph_model;
        Font::TextSize _text_size;
        std::unordered_map<int32_t, GlyphModel> _glyphs;
    };

    struct AtlasAttachment {
        AtlasAttachment(Atlas& atlas, sp<RenderController> renderController);

        sp<GlyphBundle> makeGlyphBundle(sp<Alphabet> alphabet, const Font::TextSize& textSize);

        Atlas& _atlas;
        sp<RenderController> _render_controller;

        void initialize(uint32_t textureWidth, uint32_t textureHeight);
        bool resize(uint32_t textureWidth, uint32_t textureHeight);
        void reloadTexture();

        bitmap _glyph_bitmap;

        MaxRectsBinPack _bin_pack;
        std::vector<sp<GlyphBundle>> _glyph_bundles;

        sp<Future> _texture_reload_future;
    };

private:
    sp<Atlas> _atlas;
    sp<AtlasAttachment> _atlas_attachment;
    sp<GlyphBundle> _glyph_bundle;
};

}
