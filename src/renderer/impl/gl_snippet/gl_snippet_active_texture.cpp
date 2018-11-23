#include "renderer/impl/gl_snippet/gl_snippet_active_texture.h"

#include "renderer/base/gl_pipeline.h"
#include "renderer/base/gl_texture.h"

namespace ark {

GLSnippetActiveTexture::GLSnippetActiveTexture()
{
}

GLSnippetActiveTexture::GLSnippetActiveTexture(const sp<GLTexture>& texture, uint32_t name)
{
    _textures.emplace_back(texture, name);
}

GLSnippetActiveTexture::GLSnippetActiveTexture(const sp<GLResource>& texture, uint32_t target, uint32_t name)
{
    _textures.emplace_back(texture, target, name);
}

void GLSnippetActiveTexture::preDraw(GraphicsContext& /*graphicsContext*/, const GLPipeline& shader, const GLDrawingContext& /*context*/)
{
    for(const auto& i : _textures)
        GLTexture::active(shader.program(), i.target, i.resource->id(), i.name);
}

GLSnippetActiveTexture::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
{
    uint32_t defid = 0;
    for(const document& i : manifest->children())
    {
        uint32_t id = Documents::getAttribute<uint32_t>(i, Constants::Attributes::NAME, defid++);
        if(defid <= id)
            defid = id + 1;
        _textures.emplace_back(id, factory.ensureBuilder<GLTexture>(i));
    }
}

sp<GLSnippet> GLSnippetActiveTexture::BUILDER::build(const sp<Scope>& args)
{
    const sp<GLSnippetActiveTexture> snippet = sp<GLSnippetActiveTexture>::make();
    for(const auto& i : _textures)
        snippet->_textures.emplace_back(i.second->build(args), i.first);
    return snippet;
}

GLSnippetActiveTexture::Texture::Texture(const sp<GLTexture>& texture, uint32_t name)
    : Texture(texture, texture->target(), name)
{
}

GLSnippetActiveTexture::Texture::Texture(const sp<GLResource>& texture, uint32_t target, uint32_t name)
    : resource(texture), target(target), name(name)
{
}

}
