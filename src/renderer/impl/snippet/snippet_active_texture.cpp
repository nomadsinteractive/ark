#include "renderer/impl/snippet/snippet_active_texture.h"

#include "renderer/base/shader.h"
#include "renderer/base/texture.h"
#include "renderer/inf/pipeline.h"

namespace ark {

SnippetActiveTexture::SnippetActiveTexture()
{
}

SnippetActiveTexture::SnippetActiveTexture(const sp<Texture>& texture, uint32_t name)
{
    _textures.emplace_back(texture, name);
}

SnippetActiveTexture::SnippetActiveTexture(const sp<Resource>& texture, Texture::Type type, uint32_t name)
{
    _textures.emplace_back(texture, type, name);
}

void SnippetActiveTexture::preDraw(GraphicsContext& /*graphicsContext*/, const Shader& shader, const DrawingContext& /*context*/)
{
    for(const auto& i : _textures)
        shader.pipeline()->activeTexture(i.resource, i.type, i.name);
}

SnippetActiveTexture::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
{
    uint32_t defid = 0;
    for(const document& i : manifest->children())
    {
        uint32_t id = Documents::getAttribute<uint32_t>(i, Constants::Attributes::NAME, defid++);
        if(defid <= id)
            defid = id + 1;
        _textures.emplace_back(id, factory.ensureBuilder<Texture>(i));
    }
}

sp<Snippet> SnippetActiveTexture::BUILDER::build(const sp<Scope>& args)
{
    const sp<SnippetActiveTexture> snippet = sp<SnippetActiveTexture>::make();
    for(const auto& i : _textures)
        snippet->_textures.emplace_back(i.second->build(args), i.first);
    return snippet;
}

SnippetActiveTexture::Slot::Slot(const sp<Texture>& texture, uint32_t name)
    : Slot(texture, texture->type(), name)
{
}

SnippetActiveTexture::Slot::Slot(const sp<Resource>& texture, Texture::Type type, uint32_t name)
    : resource(texture), type(type), name(name)
{
}

}
