#include "renderer/impl/model_loader/model_loader_text.h"

#include "core/ark.h"
#include "core/base/clock.h"
#include "core/base/future.h"
#include "core/types/global.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/components/size.h"
#include "graphics/inf/alphabet.h"

#include "renderer/base/atlas.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/drawing_buffer.h"
#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/model.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/shader.h"
#include "renderer/base/texture.h"
#include "renderer/impl/render_command_composer/rcc_draw_elements_incremental.h"
#include "renderer/impl/vertices/vertices_quad_lhs.h"
#include "renderer/impl/vertices/vertices_quad_rhs.h"


namespace ark {

ModelLoaderText::ModelLoaderText(sp<Alphabet> alphabet, sp<Atlas> atlas, const Font& font)
    : ModelLoader(enums::DRAW_MODE_TRIANGLES, atlas->texture(), MODEL_TRAIT_DISALLOW_CACHE), _alphabet(std::move(alphabet)), _atlas(std::move(atlas)), _glyph_attachment(_atlas->attachments().ensure<AtlasGlyphAttachment>(*_atlas)),
      _default_glyph_bundle(_glyph_attachment->ensureGlyphBundle(_alphabet, font))
{
}

sp<DrawingContextComposer> ModelLoaderText::makeRenderCommandComposer(const Shader& shader)
{
    _default_glyph_bundle->_is_lhs = shader.camera().isLHS();
    return Ark::instance().renderController()->makeDrawElementsIncremental(Global<Constants>()->MODEL_UNIT_QUAD_RHS);
}

sp<Model> ModelLoaderText::loadModel(int32_t type)
{
    const auto [font, unicode] = Font::partition(type);
    const sp<GlyphBundle>& glyphBundle = font ? _glyph_attachment->ensureGlyphBundle(_alphabet, font) : _default_glyph_bundle;
    const GlyphModel& glyph = glyphBundle->ensureGlyphModel(Ark::instance().appClock()->tick(), unicode, true);
    return glyph._model;
}

ModelLoaderText::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _alphabet(factory.ensureBuilder<Alphabet>(manifest, constants::ALPHABET)), _atlas(factory.ensureBuilder<Atlas>(manifest, constants::ATLAS)), _font(factory.ensureBuilder<Font>(manifest, constants::FONT))
{
}

sp<ModelLoader> ModelLoaderText::BUILDER::build(const Scope& args)
{
    return sp<ModelLoader>::make<ModelLoaderText>(_alphabet->build(args), _atlas->build(args), *_font->build(args));
}

ModelLoaderText::GlyphBundle::GlyphBundle(AtlasGlyphAttachment& atlasAttachment, sp<Alphabet> alphabet, const Font& font)
    : _atlas_attachment(atlasAttachment), _alphabet(std::move(alphabet)), _font(font), _unit_glyph_model(Global<Constants>()->MODEL_UNIT_QUAD_RHS), _is_lhs(false)
{
    ASSERT(_font);
}

bool ModelLoaderText::GlyphBundle::prepareOne(const uint64_t timestamp, int32_t c, int32_t ckey)
{
    if(Optional<Alphabet::Metrics> optMetrics = _alphabet->measure(c))
    {
        const auto& [width, height, bitmap_width, bitmap_height, bitmap_x, bitmap_y] = optMetrics.value();
        CHECK(width > 0 && height > 0, "Error loading character %d: width = %d, height = %d", c, width, height);
        const MaxRectsBinPack::Rect packedBounds = _atlas_attachment._bin_pack.Insert(bitmap_width + 2, bitmap_height + 2, MaxRectsBinPack::RectBestShortSideFit);
        if(packedBounds.height == 0)
            return false;

        const uint32_t cx = packedBounds.x + 1;
        const uint32_t cy = packedBounds.y + 1;
        _alphabet->draw(c, _atlas_attachment._glyph_bitmap, cx, cy);

        const float bottom = static_cast<float>(_is_lhs ? bitmap_y : height - bitmap_height - bitmap_y) / bitmap_height;
        const Rect bounds(0, bottom, 1.0f, bottom + 1.0f);
        const V2 charSize(static_cast<float>(bitmap_width), static_cast<float>(bitmap_height));
        Atlas::Item item = _atlas_attachment._atlas.makeItem(cx, cy, cx + bitmap_width, cy + bitmap_height, bounds, charSize, V2(0));
        const V3 xyz(static_cast<float>(bitmap_x), static_cast<float>(bitmap_y), 0);
        sp<Boundaries> content = sp<Boundaries>::make(V3(0), V3(charSize, 0));
        sp<Boundaries> occupies = sp<Boundaries>::make(-xyz, V3(static_cast<float>(width), static_cast<float>(height), 0) - xyz);
        _glyphs[ckey] = GlyphModel(sp<Model>::make(_unit_glyph_model->indices(), _is_lhs ? sp<Vertices>::make<VerticesQuadLHS>(item) : sp<Vertices>::make<VerticesQuadRHS>(item), std::move(content), std::move(occupies)), timestamp);
    }
    else
    {
        LOGW("Error loading character %d", c);
        return ckey == c && ckey != ' ' ? prepareOne(timestamp, ' ', ckey) : false;
    }
    return true;
}

void ModelLoaderText::GlyphBundle::reload(const uint64_t timestamp)
{
    Vector<int32_t> reloadVector;
    for(const auto& [k, v] : _glyphs)
    {
        v._model->dispose();
        if(timestamp - v._timestamp < 1000000)
            reloadVector.push_back(k);
    }

    _alphabet->setFont(_font);
    for(const int32_t i : reloadVector)
        ensureGlyphModel(timestamp, i, false);
}

const ModelLoaderText::GlyphModel& ModelLoaderText::GlyphBundle::ensureGlyphModel(const uint64_t timestamp, const int32_t c, const bool oneshot)
{
    const auto iter = _glyphs.find(c);
    if(iter == _glyphs.end())
    {
        if(oneshot)
            _alphabet->setFont(_font);

        while(!prepareOne(timestamp, c, c))
        {
            const uint32_t width = _atlas_attachment._glyph_bitmap->width() * 2;
            const uint32_t height = _atlas_attachment._glyph_bitmap->height() * 2;
            LOGD("Glyph bitmap overflow, reallocating it to (%dx%d), characters length: %d", width, height, _glyphs.size());
            _atlas_attachment.resize(width, height);
        }

        if(oneshot)
            _atlas_attachment.reloadTexture();
    }
    GlyphModel& gm = iter == _glyphs.end() ? _glyphs[c] : iter->second;
    gm._timestamp = timestamp;
    return gm;
}

ModelLoaderText::AtlasGlyphAttachment::AtlasGlyphAttachment(Atlas& atlas)
    : _atlas(atlas)
{
    initialize(_atlas.width(), _atlas.height());
}

const sp<ModelLoaderText::GlyphBundle>& ModelLoaderText::AtlasGlyphAttachment::ensureGlyphBundle(sp<Alphabet> alphabet, const Font& font)
{
    sp<GlyphBundle>& glyphBundle = _glyph_bundles[font];
    if(!glyphBundle)
        glyphBundle = sp<GlyphBundle>::make(*this, std::move(alphabet), font);
    return glyphBundle;
}

void ModelLoaderText::AtlasGlyphAttachment::initialize(uint32_t textureWidth, uint32_t textureHeight)
{
    _glyph_bitmap = bitmap::make(textureWidth, textureHeight, textureWidth, static_cast<uint8_t>(1), true);
    memset(_glyph_bitmap->at(0, 0), 0, _glyph_bitmap->rowBytes() * _glyph_bitmap->height());
    _bin_pack.Init(textureWidth, textureHeight, false);
}

bool ModelLoaderText::AtlasGlyphAttachment::resize(const uint32_t textureWidth, const uint32_t textureHeight)
{
    const uint64_t timestamp = Ark::instance().appClock()->tick();

    initialize(textureWidth, textureHeight);

    for(const auto& [k, v] : _glyph_bundles)
        v->reload(timestamp);

    reloadTexture();
    return true;
}

void ModelLoaderText::AtlasGlyphAttachment::reloadTexture()
{
    if(_texture_reload_future)
        _texture_reload_future->cancel();

    _texture_reload_future = sp<Future>::make();
    sp<Size> size = sp<Size>::make(static_cast<float>(_glyph_bitmap->width()), static_cast<float>(_glyph_bitmap->height()));
    const sp<Texture> texture = Ark::instance().renderController()->createTexture(std::move(size), _atlas.texture()->parameters(), sp<Texture::Uploader>::make<Texture::UploaderBitmap>(_glyph_bitmap), enums::UPLOAD_STRATEGY_RELOAD, _texture_reload_future);
    _atlas.texture()->reset(*texture);
}

ModelLoaderText::GlyphModel::GlyphModel()
    : _timestamp(0)
{
}

ModelLoaderText::GlyphModel::GlyphModel(sp<Model> model, const uint64_t timestamp)
    : _model(std::move(model)), _timestamp(timestamp)
{
}

}
