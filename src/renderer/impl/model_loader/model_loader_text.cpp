#include "renderer/impl/model_loader/model_loader_text.h"

#include "core/base/future.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/size.h"
#include "graphics/inf/alphabet.h"

#include "renderer/base/atlas.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/drawing_buffer.h"
#include "renderer/base/model.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/shader.h"
#include "renderer/base/texture.h"
#include "renderer/impl/model_loader/model_loader_quad.h"


namespace ark {

ModelLoaderText::Stub::Stub(const sp<RenderController>& renderController, const sp<Alphabet>& alphabet, uint32_t textureWidth, uint32_t textureHeight)
    : _render_controller(renderController), _alphabet(alphabet), _size(sp<Size>::make())
{
    resize(textureWidth, textureHeight);
}

ModelLoaderText::Stub::Stub(const sp<RenderController>& renderController, const sp<Alphabet>& alphabet, sp<Texture> texture)
    : _render_controller(renderController), _alphabet(alphabet), _size(sp<Size>::make()), _texture(std::move(texture))
{
    resize(static_cast<uint32_t>(_texture->width()), static_cast<uint32_t>(_texture->height()));
}

bool ModelLoaderText::Stub::prepareOne(int32_t c)
{
    Alphabet::Metrics metrics;
    if(_alphabet->measure(c, metrics, false))
    {
        int32_t width = metrics.width;
        int32_t height = metrics.height;
        DCHECK(width > 0 && height > 0, "Error loading character %d: width = %d, height = %d", c, width, height);
        if(_flowx + width > _atlas->width())
        {
            _flowy += (_max_glyph_height + 1);
            _max_glyph_height = _flowx = 0;
        }
        if(_flowy + metrics.bitmap_height > _atlas->height())
            return false;

        if(_max_glyph_height < metrics.bitmap_height)
            _max_glyph_height = metrics.bitmap_height;
        _atlas->add(c, _flowx, _flowy, _flowx + metrics.bitmap_width, _flowy + metrics.bitmap_height, Rect(0, 0, 1.0f, 1.0f), V2(metrics.bitmap_width, metrics.bitmap_height), V2(0));
        _alphabet->draw(c, _font_glyph, _flowx, _flowy);
        _flowx += (metrics.bitmap_width + 1);
    }
    else
        DWARN(false, "Error loading character %d", c);
    return true;
}

void ModelLoaderText::Stub::ensureCharacter(int32_t c)
{
    if(!_atlas->has(c))
    {
        while(!prepareOne(c))
        {
            uint32_t width = _font_glyph->width() * 2;
            uint32_t height = _font_glyph->height() * 2;
            LOGD("Glyph bitmap overflow, reallocating it to (%dx%d), characters length: %d", width, height, _atlas->items().size());
            resize(width, height);
        }
        reloadTexture();
    }
}

bool ModelLoaderText::Stub::resize(uint32_t textureWidth, uint32_t textureHeight)
{
    const sp<Atlas> oldAtlas = _atlas;
    _size->setWidth(static_cast<float>(textureWidth));
    _size->setHeight(static_cast<float>(textureHeight));
    _font_glyph = bitmap::make(textureWidth, textureHeight, textureWidth, static_cast<uint8_t>(1), true);
    memset(_font_glyph->at(0, 0), 0, _font_glyph->width() * _font_glyph->height());
    _flowx = _flowy = 0;
    _max_glyph_height = 0;

    if(oldAtlas)
        for(const auto& i : oldAtlas->items())
            if(!prepareOne(i.first))
                return false;

    if(_texture)
        reloadTexture();
    else
        _texture = _render_controller->createTexture2D(_size, sp<Texture::UploaderBitmap>::make(_font_glyph), RenderController::US_ONCE_AND_ON_SURFACE_READY);

    _atlas = sp<Atlas>::make(_texture, true);
    _delegate = sp<ModelLoaderQuad>::make(_atlas);
    return true;
}

void ModelLoaderText::Stub::reloadTexture()
{
    _texture->setDelegate(_render_controller->createTexture(_size, _texture->parameters(), sp<Texture::UploaderBitmap>::make(_font_glyph), RenderController::US_ONCE_AND_ON_SURFACE_READY)->delegate(), _size);
    if(_texture_reload_future)
        _texture_reload_future->cancel();
    _texture_reload_future = _render_controller->upload(_texture, nullptr, RenderController::US_RELOAD);
}

ModelLoaderText::ModelLoaderText(const sp<RenderController>& renderController, const sp<Alphabet>& alphabet, uint32_t textureWidth, uint32_t textureHeight)
    : ModelLoader(ModelLoader::RENDER_MODE_TRIANGLES), _stub(sp<Stub>::make(renderController, alphabet, textureWidth, textureHeight))
{
}

ModelLoaderText::ModelLoaderText(const sp<RenderController>& renderController, const sp<Alphabet>& alphabet, sp<Texture> texture)
    : ModelLoader(ModelLoader::RENDER_MODE_TRIANGLES), _stub(sp<Stub>::make(renderController, alphabet, std::move(texture)))
{
}

sp<RenderCommandComposer> ModelLoaderText::makeRenderCommandComposer()
{
    return _stub->_delegate->makeRenderCommandComposer();
}

void ModelLoaderText::initialize(ShaderBindings& shaderBindings)
{
    shaderBindings.pipelineBindings()->bindSampler(_stub->_texture);
}

sp<Model> ModelLoaderText::loadModel(int32_t type)
{
    Alphabet::Metrics metrics;
    _stub->ensureCharacter(type);
    bool r = _stub->_alphabet->measure(type, metrics, false);
    DCHECK(r, "Measuring failed, type: %d", type);
    V3 bounds(static_cast<float>(metrics.width), static_cast<float>(metrics.height), 0);
    V3 size(static_cast<float>(metrics.bitmap_width), static_cast<float>(metrics.bitmap_height), 0);
    V3 xyz = V3(static_cast<float>(metrics.bitmap_x), static_cast<float>(metrics.bitmap_y), 0);
    Model model = _stub->_delegate->loadModel(type);
    return sp<Model>::make(model.indices(), model.vertices(), Metrics{bounds, size, xyz});
}

ModelLoaderText::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _alphabet(factory.ensureBuilder<Alphabet>(manifest, Constants::Attributes::ALPHABET)),
      _texture(factory.getBuilder<Texture>(manifest, Constants::Attributes::TEXTURE)),
      _texture_width(Documents::getAttribute<uint32_t>(manifest, "texture-width", 256)), _texture_height(Documents::getAttribute<uint32_t>(manifest, "texture-height", 256))
{
}

sp<ModelLoader> ModelLoaderText::BUILDER::build(const Scope& args)
{
    if(_texture)
        return sp<ModelLoaderText>::make(_resource_loader_context->renderController(), _alphabet->build(args), _texture->build(args));
    return sp<ModelLoaderText>::make(_resource_loader_context->renderController(), _alphabet->build(args), _texture_width, _texture_height);
}

}
