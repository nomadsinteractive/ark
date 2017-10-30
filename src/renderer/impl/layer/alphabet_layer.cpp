#include "renderer/impl/layer/alphabet_layer.h"

#include "core/util/documents.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/layer_context.h"
#include "graphics/inf/alphabet.h"
#include "graphics/inf/layer.h"

#include "renderer/base/atlas.h"
#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/gl_texture.h"
#include "renderer/base/gl_shader.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/impl/layer/image_layer.h"
#include "renderer/impl/gl_snippet/gl_snippet_ucolor.h"
#include "renderer/impl/gl_model/gl_model_quad.h"

#include "platform/platform.h"

namespace ark {

AlphabetLayer::AlphabetLayer(const sp<Alphabet>& alphabet, uint32_t textureWidth, uint32_t textureHeight, const sp<GLShader>& shader, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _stub(sp<Stub>::make(alphabet, resourceLoaderContext->glResourceManager(), textureWidth, textureHeight)), _resource_loader_context(resourceLoaderContext), _shader(shader),
      _image_layer(sp<ImageLayer>::make(_shader, _stub->atlas(), _resource_loader_context))
{
}

void AlphabetLayer::render(const LayerContext& renderContext, RenderCommandPipeline& pipeline, float x, float y)
{
    if(!_preparing_characters.empty())
    {

        for(const LayerContext::Item& i : renderContext.items())
            _preparing_characters.insert(i.renderObject->type());

        doPrepare(true);
    }
    _image_layer->render(renderContext, pipeline, x, y);
}

void AlphabetLayer::doPrepare(bool allowReset)
{
    bool updated = false;
    for(uint32_t c : _preparing_characters)
    {
        if(!_stub->hasCharacterGlyph(c))
        {
            if(!_stub->prepare(c, allowReset))
            {
                _stub->reset();
                doPrepare(false);
                return;
            }
            updated = true;
        }
    }
    _preparing_characters.clear();

    if(updated)
        _resource_loader_context->glResourceManager()->prepare(_stub->atlas()->texture(), GLResourceManager::PS_ONCE_FORCE);
}


const sp<Alphabet>& AlphabetLayer::alphabet() const
{
    return _stub->alphabet();
}

const sp<Atlas>& AlphabetLayer::atlas() const
{
    return _image_layer->atlas();
}

const sp<ImageLayer>& AlphabetLayer::imageLayer() const
{
    return _image_layer;
}

void AlphabetLayer::prepare(const std::wstring& text)
{
    for(wchar_t c : text)
        _preparing_characters.insert(static_cast<uint32_t>(c));
}

AlphabetLayer::Stub::Stub(const sp<Alphabet>& alphabet, const sp<GLResourceManager>& glResourceManager, uint32_t textureWidth, uint32_t textureHeight)
    : _alphabet(alphabet), _font_glyph(bitmap::make(textureWidth, textureHeight, textureWidth, static_cast<uint8_t>(1))),
      _atlas(sp<Atlas>::make(glResourceManager->createGLTexture(textureWidth, textureHeight, sp<Variable<bitmap>::Impl>::make(_font_glyph))))
{
    reset();
}

const sp<Alphabet>& AlphabetLayer::Stub::alphabet() const
{
    return _alphabet;
}

bool AlphabetLayer::Stub::hasCharacterGlyph(uint32_t c) const
{
    return _atlas->has(c);
}

bool AlphabetLayer::Stub::prepare(uint32_t c, bool allowOverflow)
{
    uint32_t width, height;
    if(_alphabet->load(c, width, height, true, false))
    {
        if(_flowx + width > _atlas->width())
        {
            _flowy += height;
            _flowx = 0;
            if(_flowy + height > _atlas->height())
            {
                DWARN(allowOverflow, "Font image buffer (%d, %d) overflow, you may need to create a larger image buffer", _atlas->width(), _atlas->height());
                if(allowOverflow)
                    return false;
            }
        }

        _atlas->add(c, _flowx, _flowy, _flowx + width, _flowy + height);
        _alphabet->draw(_font_glyph, _flowx, _flowy);
        _flowx += width;
    }
    else
        DWARN(false, "Error loading character %d", c);
    return true;
}

void AlphabetLayer::Stub::reset()
{
    _flowx = _flowy = 0;
    _atlas->clear();
    memset(_font_glyph->at(0, 0), 0, _font_glyph->width() * _font_glyph->height());
}

const sp<Atlas>& AlphabetLayer::Stub::atlas() const
{
    return _atlas;
}

AlphabetLayer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext),
      _alphabet(factory.ensureBuilder<Alphabet>(manifest, Constants::Attributes::ALPHABET)),
      _shader(factory.getBuilder<GLShader>(manifest, Constants::Attributes::SHADER, false)),
      _color(factory.getBuilder<VV4>(manifest, Constants::Attributes::TEXT_COLOR, false)),
      _texture_width(Documents::getAttribute<uint32_t>(manifest, "texture-width", 256)),
      _texture_height(Documents::getAttribute<uint32_t>(manifest, "texture-height", 256))
{
}

sp<Layer> AlphabetLayer::BUILDER::build(const sp<Scope>& args)
{
    const sp<VV4> color = _color ? _color->build(args) : sp<Color>::make(Color::WHITE).cast<VV4>();
    const sp<GLShader> shader = _shader ? _shader->build(args) : GLShader::fromStringTable("shaders/texture.vert", "shaders/alpha.frag", sp<GLSnippetUColor>::make(color), _resource_loader_context);
    return sp<AlphabetLayer>::make(_alphabet->build(args), _texture_width, _texture_height, shader, _resource_loader_context);
}

}
