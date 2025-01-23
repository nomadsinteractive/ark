#pragma once

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
    ModelLoaderText(sp<Alphabet> alphabet, sp<Atlas> atlas, const Font& font);

    sp<RenderCommandComposer> makeRenderCommandComposer(const Shader& shader) override;
    sp<Model> loadModel(int32_t type) override;

//  [[plugin::builder("text")]]
    class BUILDER final : public Builder<ModelLoader> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<ModelLoader> build(const Scope& args) override;

    private:
        builder<Alphabet> _alphabet;
        builder<Atlas> _atlas;
        builder<Font> _font;
    };

private:

    struct AtlasGlyphAttachment;

    struct GlyphModel {
        GlyphModel();
        GlyphModel(sp<Model> model, uint64_t timestamp);

        sp<Model> _model;
        uint64_t _timestamp;
    };

    struct GlyphBundle {
        GlyphBundle(AtlasGlyphAttachment& atlasAttachment, sp<Alphabet> alphabet, const Font& font);

        const GlyphModel& ensureGlyphModel(uint64_t timestamp, int32_t c, bool reload);

        bool prepareOne(uint64_t timestamp, int32_t c, int32_t ckey);

        void reload(uint64_t timestamp);

        AtlasGlyphAttachment& _atlas_attachment;

        sp<Alphabet> _alphabet;
        Font _font;
        sp<Model> _unit_glyph_model;

        HashMap<int32_t, GlyphModel> _glyphs;

        bool _is_lhs;
    };

    struct AtlasGlyphAttachment {
        AtlasGlyphAttachment(Atlas& atlas);

        const sp<GlyphBundle>& ensureGlyphBundle(sp<Alphabet> alphabet, const Font& font);

        Atlas& _atlas;

        void initialize(uint32_t textureWidth, uint32_t textureHeight);
        bool resize(uint32_t textureWidth, uint32_t textureHeight);
        void reloadTexture();

        bitmap _glyph_bitmap;

        MaxRectsBinPack _bin_pack;
        Map<Font, sp<GlyphBundle>> _glyph_bundles;

        sp<Future> _texture_reload_future;
    };

private:
    sp<Alphabet> _alphabet;
    sp<Atlas> _atlas;
    sp<AtlasGlyphAttachment> _glyph_attachment;
    sp<GlyphBundle> _default_glyph_bundle;
};

}
