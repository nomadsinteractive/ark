#include "renderer/impl/gl_snippet/gl_snippet_active_texture.h"

#include "renderer/base/shader.h"
#include "renderer/base/texture.h"

#include "renderer/opengl/base/gl_texture.h"
#include "renderer/opengl/base/gl_pipeline.h"

namespace ark {

GLSnippetActiveTexture::GLSnippetActiveTexture()
{
}

GLSnippetActiveTexture::GLSnippetActiveTexture(const sp<Texture>& texture, uint32_t name)
{
    _textures.emplace_back(texture, name);
}

GLSnippetActiveTexture::GLSnippetActiveTexture(const sp<RenderResource>& texture, uint32_t target, uint32_t name)
{
    _textures.emplace_back(texture, target, name);
}

void GLSnippetActiveTexture::preDraw(GraphicsContext& /*graphicsContext*/, const Shader& shader, const DrawingContext& /*context*/)
{
    for(const auto& i : _textures)
        shader.pipeline()->activeTexture(i.resource, i.target, i.name);
}

GLSnippetActiveTexture::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
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

sp<GLSnippet> GLSnippetActiveTexture::BUILDER::build(const sp<Scope>& args)
{
    const sp<GLSnippetActiveTexture> snippet = sp<GLSnippetActiveTexture>::make();
    for(const auto& i : _textures)
        snippet->_textures.emplace_back(i.second->build(args), i.first);
    return snippet;
}

GLSnippetActiveTexture::Slot::Slot(const sp<Texture>& texture, uint32_t name)
    : Slot(texture, static_cast<sp<GLTexture>>(texture->resource())->target(), name)
{
}

GLSnippetActiveTexture::Slot::Slot(const sp<RenderResource>& texture, uint32_t target, uint32_t name)
    : resource(texture), target(target), name(name)
{
}

}
