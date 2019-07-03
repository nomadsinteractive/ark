#include "renderer/opengl/snippet/gl_snippet_clip_plane.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/impl/flatable/flatable_by_variable.h"
#include "core/util/strings.h"

#include "graphics/base/v4.h"

#include "renderer/base/drawing_context.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/render_context.h"

#include "platform/gl/gl.h"

namespace ark {

void GLSnippetClipPlane::preInitialize(PipelineBuildingContext& context)
{
    for(const auto& i : _planes)
    {
        const String uName = Strings::sprintf("u_Plane%d", i.first);
        context.addUniform(uName, Uniform::TYPE_F4, 1, sp<FlatableByVariable<V4>>::make(i.second), i.second.as<Notifier>(), -1);
    }
}

void GLSnippetClipPlane::preCompile(GraphicsContext& graphicsContext, PipelineBuildingContext& context, const PipelineLayout& /*pipelineLayout*/)
{
    _enabled = graphicsContext.renderContext()->version() >= Ark::OPENGL_30;

    DWARN(_enabled, "GL_CLIP_PLANE is not supported, GLVersion \"%d\"", graphicsContext.renderContext()->version());
    if(_enabled)
        for(const auto& i : _planes)
        {
            const String uName = Strings::sprintf("u_Plane%d", i.first);
            context._vertex.addPreMainSource(Strings::sprintf("gl_ClipDistance[%d] = dot(a_Position, %s);", i.first, uName.c_str()));
        }
}

void GLSnippetClipPlane::preDraw(GraphicsContext& /*graphicsContext*/, const DrawingContext& /*context*/)
{
    if(_enabled)
        for(const auto& i : _planes)
            glEnable(GL_CLIP_PLANE0 + i.first);
}

void GLSnippetClipPlane::postDraw(GraphicsContext& /*graphicsContext*/)
{
    if(_enabled)
        for(const auto& i : _planes)
            glDisable(GL_CLIP_PLANE0 + i.first);
}

GLSnippetClipPlane::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
{
    uint32_t defid = 0;
    for(const document& i : manifest->children())
    {
        uint32_t id = Documents::getAttribute<uint32_t>(i, Constants::Attributes::NAME, defid++);
        if(defid <= id)
            defid = id + 1;
        _planes.push_back(std::pair<uint32_t, sp<Builder<Vec4>>>(id, factory.ensureBuilder<Vec4>(i, Constants::Attributes::VALUE)));
    }
}

sp<Snippet> GLSnippetClipPlane::BUILDER::build(const sp<Scope>& args)
{
    const sp<GLSnippetClipPlane> snippet = sp<GLSnippetClipPlane>::make();
    for(const auto& i : _planes)
        snippet->_planes.push_back(std::pair<uint32_t, sp<Vec4>>(i.first, i.second->build(args)));
    return snippet;
}

}
