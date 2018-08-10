#include "renderer/impl/layer/alphabet_layer.h"

#include "core/util/documents.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/layer_context.h"
#include "graphics/inf/alphabet.h"

#include "renderer/base/atlas.h"
#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/gl_texture.h"
#include "renderer/base/gl_shader.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/impl/layer/image_layer.h"
#include "renderer/impl/layer/gl_model_layer.h"
#include "renderer/impl/gl_model_loader/gl_model_loader_quad.h"
#include "renderer/impl/gl_snippet/gl_snippet_ucolor.h"

namespace ark {

AlphabetLayer::AlphabetLayer(const sp<Alphabet>& alphabet, uint32_t textureWidth, uint32_t textureHeight, const sp<GLShader>& shader, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : Layer(resourceLoaderContext->memoryPool()), _stub(sp<Stub>::make(alphabet, resourceLoaderContext->glResourceManager(), textureWidth, textureHeight)),
      _resource_loader_context(resourceLoaderContext), _shader(shader), _layer(sp<GLModelLayer>::make(sp<GLModelLoaderQuad>::make(), _shader, _stub->atlas(), _resource_loader_context))
{
}

sp<RenderCommand> AlphabetLayer::render(const LayerContext::Snapshot& renderContext, float x, float y)
{
    if(_stub->checkUnpreparedCharacter(renderContext))
    {
        _stub->doPrepare(renderContext, true);
        _stub->prepareTexture(_resource_loader_context->glResourceManager());
    }
    return _layer->render(renderContext, x, y);
}

const sp<Alphabet>& AlphabetLayer::alphabet() const
{
    return _stub->alphabet();
}

const sp<Atlas>& AlphabetLayer::atlas() const
{
    return _stub->atlas();
}

AlphabetLayer::Stub::Stub(const sp<Alphabet>& alphabet, const sp<GLResourceManager>& glResourceManager, uint32_t textureWidth, uint32_t textureHeight)
    : _alphabet(alphabet), _font_glyph(bitmap::make(textureWidth, textureHeight, textureWidth, static_cast<uint8_t>(1))),
      _texture(glResourceManager->createGLTexture(textureWidth, textureHeight, sp<Variable<bitmap>::Impl>::make(_font_glyph))),
      _atlas(sp<Atlas>::make(_texture, true))
{
    reset();
}

const sp<Alphabet>& AlphabetLayer::Stub::alphabet() const
{
    return _alphabet;
}

bool AlphabetLayer::Stub::hasCharacterGlyph(uint32_t c) const
{
    return _atlas->at(c).width() != 0;
}

bool AlphabetLayer::Stub::prepare(uint32_t c, bool allowOverflow)
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
        {
            DWARN(allowOverflow, "TODO: Font image texture (%d, %d) overflow, you may need to create a larger image texture", _atlas->width(), _atlas->height());
            if(allowOverflow)
                return false;
        }
        _atlas->add(c, _flowx, _flowy, _flowx + metrics.bitmap_width, _flowy + metrics.bitmap_height + 1);
        _alphabet->draw(c, _font_glyph, _flowx, _flowy);
        _flowx += metrics.bitmap_width;
    }
    else
        DWARN(false, "Error loading character %d", c);
    return true;
}

bool AlphabetLayer::Stub::checkUnpreparedCharacter(const LayerContext::Snapshot& renderContext)
{
    bool updateNeeded = false;
    for(const RenderObject::Snapshot& i : renderContext._items)
    {
        bool contains = _characters.find(i._type) != _characters.end();
        _characters.insert(i._type);
        updateNeeded = updateNeeded || !contains;
    }
    return updateNeeded;
}

void AlphabetLayer::Stub::reset()
{
    _flowx = _flowy = 0;
    _max_glyph_height = 0;
    _atlas->clear();
    _characters.clear();
    memset(_font_glyph->at(0, 0), 0, _font_glyph->width() * _font_glyph->height());
}

void AlphabetLayer::Stub::doPrepare(const LayerContext::Snapshot& renderContext, bool allowReset)
{
    for(uint32_t c : _characters)
    {
        if(!hasCharacterGlyph(c))
        {
            if(!prepare(c, allowReset))
            {
                reset();
                checkUnpreparedCharacter(renderContext);
                return doPrepare(renderContext, false);
            }
        }
    }
}

void AlphabetLayer::Stub::prepareTexture(GLResourceManager& glResourceManager) const
{
    glResourceManager.prepare(_texture, GLResourceManager::PS_ONCE_FORCE);
}

const sp<Atlas>& AlphabetLayer::Stub::atlas() const
{
    return _atlas;
}

AlphabetLayer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext),
      _alphabet(factory.ensureBuilder<Alphabet>(manifest, Constants::Attributes::ALPHABET)),
      _shader(factory.getBuilder<GLShader>(manifest, Constants::Attributes::SHADER, false)),
      _color(factory.getBuilder<Vec4>(manifest, Constants::Attributes::TEXT_COLOR, false)),
      _texture_width(Documents::getAttribute<uint32_t>(manifest, "texture-width", 256)),
      _texture_height(Documents::getAttribute<uint32_t>(manifest, "texture-height", 256))
{
}

sp<Layer> AlphabetLayer::BUILDER::build(const sp<Scope>& args)
{
    const sp<Vec4> color = _color ? _color->build(args) : sp<Color>::make(Color::WHITE).cast<Vec4>();
    const sp<GLShader> shader = _shader ? _shader->build(args) : GLShader::fromStringTable("shaders/default.vert", "shaders/alpha.frag", sp<GLSnippetUColor>::make(color), _resource_loader_context);
    return sp<AlphabetLayer>::make(_alphabet->build(args), _texture_width, _texture_height, shader, _resource_loader_context);
}

}
