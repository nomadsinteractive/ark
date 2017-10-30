#include "renderer/impl/gl_snippet/gl_snippet_textures.h"

#include "core/base/bean_factory.h"

#include "renderer/base/gl_shader.h"
#include "renderer/base/gl_texture.h"
#include "renderer/base/gl_snippet_context.h"

namespace ark {

void GLSnippetTextures::addTexture(uint32_t id, const sp<GLTexture>& texture)
{
    _textures.push_back(std::pair<uint32_t, sp<GLTexture>>(id, texture));
}

void GLSnippetTextures::preDraw(GraphicsContext& /*graphicsContext*/, const GLShader& shader, const GLSnippetContext& /*context*/)
{
    for(const auto& i : _textures)
        i.second->active(shader.program(), static_cast<uint32_t>(GL_TEXTURE0 + i.first));
}

GLSnippetTextures::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
{
    uint32_t defid = 0;
    for(const document& i : manifest->children("texture"))
    {
        uint32_t id = Documents::getAttribute<uint32_t>(i, Constants::Attributes::NAME, defid++);
        if(defid <= id)
            defid = id + 1;
        _textures.push_back(std::pair<uint32_t, sp<Builder<GLTexture>>>(id, factory.ensureBuilder<GLTexture>(i, Constants::Attributes::SRC)));
    }
}

sp<GLSnippet> GLSnippetTextures::BUILDER::build(const sp<Scope>& args)
{
    const sp<GLSnippetTextures> snippet = sp<GLSnippetTextures>::make();
    for(const auto& iter : _textures)
        snippet->addTexture(iter.first, iter.second->build(args));
    return snippet;
}

}
