#include "renderer/impl/render_model/render_model_text.h"

#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/size.h"
#include "graphics/inf/alphabet.h"

#include "renderer/base/atlas.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/drawing_buffer.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/shader.h"
#include "renderer/base/texture.h"
#include "renderer/impl/render_model/render_model_quad.h"

namespace ark {

RenderModelText::Stub::Stub(const sp<RenderController>& renderController, const sp<Alphabet>& alphabet, uint32_t textureWidth, uint32_t textureHeight)
    : _render_controller(renderController), _alphabet(alphabet), _size(sp<Size>::make())
{
    reset(textureWidth, textureHeight);
}

void RenderModelText::Stub::reset(uint32_t textureWidth, uint32_t textureHeight)
{
    _size->setWidth(static_cast<float>(textureWidth));
    _size->setHeight(static_cast<float>(textureHeight));
    _font_glyph = bitmap::make(textureWidth, textureHeight, textureWidth, static_cast<uint8_t>(1), true);
    _texture = _render_controller->createTexture2D(_size, sp<Texture::UploaderBitmap>::make(_font_glyph), RenderController::US_ON_SURFACE_READY);
    _atlas = sp<Atlas>::make(_texture, true);
    _delegate = sp<RenderModelQuad>::make(_render_controller, _atlas);
    clear();
}

bool RenderModelText::Stub::prepareOne(int32_t c)
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

bool RenderModelText::Stub::checkUnpreparedCharacter(const RenderLayer::Snapshot& renderContext)
{
    for(const RenderObject::Snapshot& i : renderContext._items)
    {
        if(!_atlas->has(i._type))
            return true;
    }
    return false;
}

void RenderModelText::Stub::clear()
{
    _flowx = _flowy = 0;
    _max_glyph_height = 0;
    _atlas->clear();
    memset(_font_glyph->at(0, 0), 0, _font_glyph->width() * _font_glyph->height());
}

bool RenderModelText::Stub::prepare(const RenderLayer::Snapshot& snapshot, bool allowReset)
{
    for(const RenderObject::Snapshot& i : snapshot._items)
    {
        if(!_atlas->has(i._type) && !prepareOne(i._type))
        {
            if(allowReset)
            {
                clear();
                return prepare(snapshot, false);
            }
            return false;
        }
    }
    return true;
}

sp<Texture::Delegate> RenderModelText::Stub::val()
{
    return _texture->delegate();
}

RenderModelText::RenderModelText(const sp<RenderController>& renderController, const sp<Alphabet>& alphabet, uint32_t textureWidth, uint32_t textureHeight)
    : _stub(sp<Stub>::make(renderController, alphabet, textureWidth, textureHeight))
{
}

sp<ShaderBindings> RenderModelText::makeShaderBindings(const Shader& shader)
{
    const sp<ShaderBindings> bindings = shader.makeBindings(RENDER_MODE_TRIANGLES);
    _shader_texture = sp<Texture>::make(_stub->_size, _stub, sp<Texture::Parameters>::make(Texture::TYPE_2D));
    bindings->pipelineBindings()->bindSampler(_shader_texture);
    return bindings;
}

void RenderModelText::postSnapshot(RenderController& renderController, RenderLayer::Snapshot& snapshot)
{
    if(_stub->checkUnpreparedCharacter(snapshot))
    {
        while(!_stub->prepare(snapshot, true))
        {
            uint32_t width = _stub->_font_glyph->width() * 2;
            uint32_t height = _stub->_font_glyph->height() * 2;
            LOGD("Glyph bitmap overflow, reallocating it to (%dx%d), characters length: %d", width, height, _stub->_atlas->items()->indices().size());
            _stub->reset(width, height);
        }
        _stub->_render_controller->upload(_shader_texture, nullptr, RenderController::US_RELOAD);
    }
    _stub->_delegate->postSnapshot(renderController, snapshot);
}

void RenderModelText::start(DrawingBuffer& buf, const RenderLayer::Snapshot& snapshot)
{
    _stub->_delegate->start(buf, snapshot);
}

void RenderModelText::load(DrawingBuffer& buf, const RenderObject::Snapshot& snapshot)
{
    _stub->_delegate->load(buf, snapshot);
}

Metrics RenderModelText::measure(int32_t type)
{
    Alphabet::Metrics metrics;
    bool r = _stub->_alphabet->measure(type, metrics, false);
    DCHECK(r, "Measuring failed, type: %d", type);
    return {
        {static_cast<float>(metrics.width), static_cast<float>(metrics.height)},
        {static_cast<float>(metrics.bitmap_width), static_cast<float>(metrics.bitmap_height)},
        {static_cast<float>(metrics.bitmap_x), static_cast<float>(metrics.bitmap_y)}
    };
}

RenderModelText::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _alphabet(factory.ensureBuilder<Alphabet>(manifest, Constants::Attributes::ALPHABET)),
      _texture_width(Documents::getAttribute<uint32_t>(manifest, "texture-width", 256)), _texture_height(Documents::getAttribute<uint32_t>(manifest, "texture-height", 256))
{
}

sp<RenderModel> RenderModelText::BUILDER::build(const Scope& args)
{
    return sp<RenderModelText>::make(_resource_loader_context->renderController(), _alphabet->build(args), _texture_width, _texture_height);
}

}
