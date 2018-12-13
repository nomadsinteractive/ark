#include "renderer/impl/gl_model/gl_model_text.h"

#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/inf/alphabet.h"

#include "renderer/base/atlas.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/model_buffer.h"
#include "renderer/base/texture.h"
#include "renderer/base/resource_manager.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/impl/gl_model/gl_model_quad.h"

namespace ark {

GLModelText::Stub::Stub(RenderController& renderController, const sp<Alphabet>& alphabet, uint32_t textureWidth, uint32_t textureHeight)
    : _alphabet(alphabet)
{
    reset(renderController, textureWidth, textureHeight);
}

void GLModelText::Stub::reset(RenderController& renderController, uint32_t textureWidth, uint32_t textureHeight)
{
    _font_glyph = bitmap::make(textureWidth, textureHeight, textureWidth, static_cast<uint8_t>(1));
    _texture = renderController.createTexture(textureWidth, textureHeight, sp<Variable<bitmap>::Const>::make(_font_glyph), ResourceManager::US_ON_SURFACE_READY);
    _atlas = sp<Atlas>::make(_texture, true);
    _delegate = sp<GLModelQuad>::make(_atlas);
    clear();
}

bool GLModelText::Stub::prepareOne(int32_t c)
{
    Alphabet::Metrics metrics;
    if(_alphabet->measure(c, metrics, false))
    {
        int32_t width = metrics.width;
        int32_t height = metrics.height;
        DCHECK(width > 0 && height > 0, "Error loading character %d: width = %d, height = %d", c, width, height);
        if(_max_glyph_height < metrics.bitmap_height)
            _max_glyph_height = metrics.bitmap_height;
        if(_flowx + width > _atlas->width())
        {
            _flowy += (_max_glyph_height + 1);
            _max_glyph_height = _flowx = 0;
        }
        if(_flowy + metrics.bitmap_height > _atlas->height())
            return false;

        _atlas->add(c, _flowx, _flowy, _flowx + metrics.bitmap_width, _flowy + metrics.bitmap_height + 1);
        _alphabet->draw(c, _font_glyph, _flowx, _flowy);
        _flowx += metrics.bitmap_width;
    }
    else
        DWARN(false, "Error loading character %d", c);
    return true;
}

bool GLModelText::Stub::checkUnpreparedCharacter(const Layer::Snapshot& renderContext)
{
    bool updateNeeded = false;
    for(const RenderObject::Snapshot& i : renderContext._items)
    {
        if(_characters.find(i._type) == _characters.end())
        {
            _characters.insert(i._type);
            updateNeeded = true;
        }
    }
    return updateNeeded;
}

void GLModelText::Stub::clear()
{
    _flowx = _flowy = 0;
    _max_glyph_height = 0;
    _atlas->clear();
    _characters.clear();
    memset(_font_glyph->at(0, 0), 0, _font_glyph->width() * _font_glyph->height());
}

bool GLModelText::Stub::prepare(const Layer::Snapshot& renderContext, bool allowReset)
{
    for(int32_t c : _characters)
    {
        if(!_atlas->has(c) && !prepareOne(c))
        {
            if(allowReset)
            {
                clear();
                checkUnpreparedCharacter(renderContext);
                return prepare(renderContext, false);
            }
            return false;
        }
    }
    return true;
}

uint32_t GLModelText::Stub::id()
{
    return _texture->id();
}

void GLModelText::Stub::upload(GraphicsContext& /*graphicsContext*/)
{
}

Resource::RecycleFunc GLModelText::Stub::recycle()
{
    return [](GraphicsContext&) {
    };
}

GLModelText::GLModelText(RenderController& renderController, const sp<Alphabet>& alphabet, uint32_t textureWidth, uint32_t textureHeight)
    : RenderModel(RENDER_MODE_TRIANGLES), _stub(sp<Stub>::make(renderController, alphabet, textureWidth, textureHeight))
{
}

void GLModelText::initialize(ShaderBindings& bindings)
{
    bindings.bindGLTexture(_stub, Texture::TYPE_2D, 0);
}

void GLModelText::start(ModelBuffer& buf, RenderController& renderController, const Layer::Snapshot& layerContext)
{
    if(_stub->checkUnpreparedCharacter(layerContext))
    {
        while(!_stub->prepare(layerContext, true))
        {
            uint32_t width = _stub->_font_glyph->width() * 2;
            uint32_t height = _stub->_font_glyph->height() * 2;
            LOGD("Glyph bitmap overflow, reallocating it to (%dx%d), characters length: %d", width, height, _stub->_characters.size());
            _stub->reset(renderController, width, height);
        }
        renderController.resourceManager()->upload(_stub->_texture, ResourceManager::US_ONCE_FORCE);
    }
    _stub->_delegate->start(buf, renderController, layerContext);
}

void GLModelText::load(ModelBuffer& buf, int32_t type, const V& scale)
{
    _stub->_delegate->load(buf, type, scale);
}

Metrics GLModelText::measure(int32_t type)
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

GLModelText::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _alphabet(factory.ensureBuilder<Alphabet>(manifest, Constants::Attributes::ALPHABET)),
      _texture_width(Documents::getAttribute<uint32_t>(manifest, "texture-width", 256)), _texture_height(Documents::getAttribute<uint32_t>(manifest, "texture-height", 256))
{
}

sp<RenderModel> GLModelText::BUILDER::build(const sp<Scope>& args)
{
    return sp<GLModelText>::make(_resource_loader_context->renderController(), _alphabet->build(args), _texture_width, _texture_height);
}

}
