#include "renderer/impl/gl_snippet/gl_snippet_textures.h"

#include "core/base/bean_factory.h"

#include "renderer/base/gl_cubemap.h"
#include "renderer/base/gl_shader.h"
#include "renderer/base/gl_texture.h"

namespace ark {

void GLSnippetTextures::addTexture(uint32_t id, const sp<GLTexture>& texture)
{
    _textures.push_back(std::pair<uint32_t, sp<GLTexture>>(id, texture));
}

void GLSnippetTextures::addCubemap(uint32_t id, const sp<GLCubemap>& cubemap)
{
    _cubemaps.push_back(std::pair<uint32_t, sp<GLCubemap>>(id, cubemap));
}

void GLSnippetTextures::preDraw(GraphicsContext& /*graphicsContext*/, const GLShader& shader, const GLDrawingContext& /*context*/)
{
    for(const auto i : _textures)
        i.second->active(shader.program(), i.first);
    for(const auto i : _cubemaps)
        i.second->active(shader.program(), i.first);
}

GLSnippetTextures::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
{
    uint32_t defid = 0;
    for(const document& i : manifest->children())
    {
        uint32_t id = Documents::getAttribute<uint32_t>(i, Constants::Attributes::NAME, defid++);
        if(defid <= id)
            defid = id + 1;
        if(i->name() == "texture")
            _textures.push_back(std::pair<uint32_t, sp<Builder<GLTexture>>>(id, factory.ensureBuilder<GLTexture>(i)));
        else if(i->name() == "cubemap")
            _cubemaps.push_back(std::pair<uint32_t, sp<Builder<GLCubemap>>>(id, factory.ensureBuilder<GLCubemap>(i)));
        else
            DFATAL("Unknow texture type: %s", i->name().c_str());
    }
}

sp<GLSnippet> GLSnippetTextures::BUILDER::build(const sp<Scope>& args)
{
    const sp<GLSnippetTextures> snippet = sp<GLSnippetTextures>::make();
    for(const auto iter : _textures)
        snippet->addTexture(iter.first, iter.second->build(args));
    for(const auto iter : _cubemaps)
        snippet->addCubemap(iter.first, iter.second->build(args));
    return snippet;
}

}
