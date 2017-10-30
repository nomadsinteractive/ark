#include "renderer/impl/gl_snippet/gl_snippet_clip_plane.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/util/strings.h"

#include "graphics/impl/flatable/flatable_v4f.h"

#include "renderer/base/gl_shader_source.h"
#include "renderer/base/gl_texture.h"
#include "renderer/base/gl_snippet_context.h"
#include "renderer/base/render_engine.h"

#include "platform/gl/gl.h"

namespace ark {

void GLSnippetClipPlane::preCompile(GLShaderSource& source, GLShaderPreprocessor::Context& context)
{
    for(const auto& i : _planes)
    {
        const String uName = Strings::sprintf("u_Plane%d", i.first);
        source.addUniform(uName, GLUniform::UNIFORM_F4, sp<FlatableV4f>::make(i.second), i.second.as<Changed>());
        context.addVertexSource(Strings::sprintf("gl_ClipDistance[%d] = dot(a_Position, %s);", i.first, uName.c_str()));
    }
}

void GLSnippetClipPlane::preDraw(GraphicsContext& /*graphicsContext*/, const GLShader& shader, const GLSnippetContext& /*context*/)
{
    for(const auto& i : _planes)
        glEnable(GL_CLIP_PLANE0 + i.first);
}

void GLSnippetClipPlane::postDraw(GraphicsContext& /*graphicsContext*/)
{
    for(const auto& i : _planes)
        glDisable(GL_CLIP_PLANE0 + i.first);
}

GLSnippetClipPlane::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _enabled(Ark::instance().global<RenderEngine>()->version() >= Ark::OPENGL_30)
{
    if(_enabled)
    {
        uint32_t defid = 0;
        for(const document& i : manifest->children())
        {
            uint32_t id = Documents::getAttribute<uint32_t>(i, Constants::Attributes::NAME, defid++);
            if(defid <= id)
                defid = id + 1;
            _planes.push_back(std::pair<uint32_t, sp<Builder<VV4>>>(id, factory.ensureBuilder<VV4>(i, Constants::Attributes::VALUE)));
        }
    }
}

sp<GLSnippet> GLSnippetClipPlane::BUILDER::build(const sp<Scope>& args)
{
    DWARN(_enabled, "GL_CLIP_PLANE not supported, GLVersion \"%d\"", Ark::instance().global<RenderEngine>()->version());
    if(!_enabled)
        return sp<GLSnippet>::make();

    const sp<GLSnippetClipPlane> snippet = sp<GLSnippetClipPlane>::make();
    for(const auto& i : _planes)
        snippet->_planes.push_back(std::pair<uint32_t, sp<VV4>>(i.first, i.second->build(args)));
    return snippet;
}

}
