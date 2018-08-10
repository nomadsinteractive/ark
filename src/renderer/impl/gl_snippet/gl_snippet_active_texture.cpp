#include "renderer/impl/gl_snippet/gl_snippet_active_texture.h"

#include "renderer/base/gl_shader.h"
#include "renderer/base/gl_texture.h"

namespace ark {

GLSnippetActiveTexture::GLSnippetActiveTexture()
{
}

GLSnippetActiveTexture::GLSnippetActiveTexture(const sp<GLTexture>& texture, uint32_t name)
{
    _textures.emplace_back(name, texture);
}

void GLSnippetActiveTexture::preDraw(GraphicsContext& /*graphicsContext*/, const GLShader& shader, const GLDrawingContext& /*context*/)
{
    for(const auto i : _textures)
        i.second->active(shader.program(), i.first);
}

GLSnippetActiveTexture::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
{
    uint32_t defid = 0;
    for(const document& i : manifest->children())
    {
        uint32_t id = Documents::getAttribute<uint32_t>(i, Constants::Attributes::NAME, defid++);
        if(defid <= id)
            defid = id + 1;
        _textures.push_back(std::pair<uint32_t, sp<Builder<GLTexture>>>(id, factory.ensureBuilder<GLTexture>(i)));
    }
}

sp<GLSnippet> GLSnippetActiveTexture::BUILDER::build(const sp<Scope>& args)
{
    const sp<GLSnippetActiveTexture> snippet = sp<GLSnippetActiveTexture>::make();
    for(const auto iter : _textures)
        snippet->_textures.emplace_back(iter.first, iter.second->build(args));
    return snippet;
}

}
