#include "renderer/impl/model_loader/model_loader_text.h"

#include "core/ark.h"
#include "core/base/clock.h"
#include "core/base/future.h"
#include "core/types/global.h"

#include "graphics/base/bitmap.h"
#include "graphics/components/size.h"
#include "graphics/inf/alphabet.h"

#include "renderer/base/atlas.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/drawing_buffer.h"
#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/model.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader.h"
#include "renderer/base/texture.h"
#include "renderer/impl/render_command_composer/rcc_draw_elements_incremental.h"
#include "renderer/impl/vertices/vertices_quad_rhs.h"


namespace ark {

ModelLoaderText::ModelLoaderText(sp<RenderController> renderController, sp<Alphabet> alphabet, sp<Atlas> atlas, const Font::TextSize& textSize)
    : ModelLoader(Enum::RENDER_MODE_TRIANGLES, atlas->texture()), _atlas(std::move(atlas)), _atlas_attachment(_atlas->attachments().ensure<AtlasAttachment>(*_atlas, std::move(renderController))),
      _glyph_bundle(_atlas_attachment->makeGlyphBundle(std::move(alphabet), textSize))
{
}

sp<RenderCommandComposer> ModelLoaderText::makeRenderCommandComposer(const Shader& shader)
{
    return Ark::instance().renderController()->makeDrawElementsIncremental(Global<Constants>()->MODEL_UNIT_QUAD_RHS);
}

sp<Model> ModelLoaderText::loadModel(int32_t type)
{
    const GlyphModel& glyph = _glyph_bundle->ensureGlyphModel(Ark::instance().appClock()->tick(), type, true);
    return glyph._model;
}

ModelLoaderText::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _alphabet(factory.ensureBuilder<Alphabet>(manifest, constants::ALPHABET)),
      _atlas(factory.ensureBuilder<Atlas>(manifest, constants::ATLAS)), _text_size(factory.getBuilder<String>(manifest, constants::TEXT_SIZE))
{
}

sp<ModelLoader> ModelLoaderText::BUILDER::build(const Scope& args)
{
    const sp<String> textSize = _text_size.build(args);
    return sp<ModelLoaderText>::make(_resource_loader_context->renderController(), _alphabet->build(args), _atlas->build(args), textSize ? Font::TextSize(*textSize) : Font::TextSize());
}

ModelLoaderText::GlyphBundle::GlyphBundle(AtlasAttachment& atlasAttachment, sp<Alphabet> alphabet, const Font::TextSize& textSize)
    : _atlas_attachment(atlasAttachment), _alphabet(std::move(alphabet)), _unit_glyph_model(Global<Constants>()->MODEL_UNIT_QUAD_RHS), _text_size(textSize)
{
}

bool ModelLoaderText::GlyphBundle::prepareOne(uint64_t timestamp, int32_t c, int32_t ckey)
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

        const float bottom = static_cast<float>(height - bitmap_height - bitmap_y) / bitmap_height;
        const Rect bounds(0, bottom, 1.0f, bottom + 1.0f);
        const V2 charSize(static_cast<float>(bitmap_width), static_cast<float>(bitmap_height));
        Atlas::Item item = _atlas_attachment._atlas.makeItem(cx, cy, cx + bitmap_width, cy + bitmap_height, bounds, charSize, V2(0));
        const V3 xyz(static_cast<float>(bitmap_x), static_cast<float>(bitmap_y), 0);
        sp<Boundaries> content = sp<Boundaries>::make(V3(0), V3(charSize, 0));
        sp<Boundaries> occupies = sp<Boundaries>::make(-xyz, V3(static_cast<float>(width), static_cast<float>(height), 0) - xyz);
        _glyphs[ckey] = GlyphModel(sp<Model>::make(_unit_glyph_model->indices(), sp<VerticesQuadRHS>::make(item), std::move(content), std::move(occupies)), timestamp);
    }
    else
    {
        LOGW("Error loading character %d", c);
        return ckey == c && ckey != ' ' ? prepareOne(timestamp, ' ', ckey) : false;
    }
    return true;
}

void ModelLoaderText::GlyphBundle::reload(uint64_t timestamp)
{
    std::vector<int32_t> reloadVector;
    for(const auto& [k, v] : _glyphs)
    {
        v._model->dispose();
        if(timestamp - v._timestamp < 1000000)
            reloadVector.push_back(k);
    }

    _alphabet->setTextSize(_text_size);
    for(const int32_t i : reloadVector)
        ensureGlyphModel(timestamp, i, false);
}

const ModelLoaderText::GlyphModel& ModelLoaderText::GlyphBundle::ensureGlyphModel(uint64_t timestamp, int32_t c, bool oneshot)
{
    const auto iter = _glyphs.find(c);
    if(iter == _glyphs.end())
    {
        if(oneshot)
            _alphabet->setTextSize(_text_size);

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

ModelLoaderText::AtlasAttachment::AtlasAttachment(Atlas& atlas, sp<RenderController> renderController)
    : _atlas(atlas), _render_controller(std::move(renderController))
{
    initialize(_atlas.width(), _atlas.height());
}

sp<ModelLoaderText::GlyphBundle> ModelLoaderText::AtlasAttachment::makeGlyphBundle(sp<Alphabet> alphabet, const Font::TextSize& textSize)
{
    sp<GlyphBundle> glyphBundle = sp<GlyphBundle>::make(*this, std::move(alphabet), textSize);
    _glyph_bundles.push_back(glyphBundle);
    return glyphBundle;
}

void ModelLoaderText::AtlasAttachment::initialize(uint32_t textureWidth, uint32_t textureHeight)
{
    _glyph_bitmap = bitmap::make(textureWidth, textureHeight, textureWidth, static_cast<uint8_t>(1), true);
    memset(_glyph_bitmap->at(0, 0), 0, _glyph_bitmap->rowBytes() * _glyph_bitmap->height());
    _bin_pack.Init(textureWidth, textureHeight, false);
}

bool ModelLoaderText::AtlasAttachment::resize(uint32_t textureWidth, uint32_t textureHeight)
{
    const uint64_t timestamp = Ark::instance().appClock()->tick();

    initialize(textureWidth, textureHeight);

    for(const sp<GlyphBundle>& i : _glyph_bundles)
        i->reload(timestamp);

    reloadTexture();
    return true;
}

void ModelLoaderText::AtlasAttachment::reloadTexture()
{
    if(_texture_reload_future)
        _texture_reload_future->cancel();

    _texture_reload_future = sp<Future>::make();
    sp<Size> size = sp<Size>::make(static_cast<float>(_glyph_bitmap->width()), static_cast<float>(_glyph_bitmap->height()));
    const sp<Texture> texture = _render_controller->createTexture(size, _atlas.texture()->parameters(), sp<Texture::Uploader>::make<Texture::UploaderBitmap>(_glyph_bitmap), RenderController::US_RELOAD, _texture_reload_future);
    _atlas.texture()->setDelegate(texture->delegate(), std::move(size));
}

ModelLoaderText::GlyphModel::GlyphModel()
    : _timestamp(0)
{
}

ModelLoaderText::GlyphModel::GlyphModel(sp<Model> model, uint64_t timestamp)
    : _model(std::move(model)), _timestamp(timestamp)
{
}

}
