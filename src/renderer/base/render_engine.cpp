#include "renderer/base/render_engine.h"

#include "core/base/string_table.h"
#include "core/types/global.h"
#include "core/util/log.h"

#include "renderer/base/gl_context.h"
#include "renderer/inf/render_view_factory.h"
#include "renderer/inf/gl_procedure_factory.h"

#include "renderer/gles20/impl/gl_procedure_factory/gl_procedure_factory_gles20.h"
#include "renderer/gles30/impl/gl_procedure_factory/gl_procedure_factory_gles30.h"

#include "platform/gl/gl.h"

namespace ark {

RenderEngine::RenderEngine(Ark::GLVersion version, const sp<RenderViewFactory>& renderViewFactory)
    : _gl_context(sp<GLContext>::make(version)), _render_view_factory(renderViewFactory)
{
    if(version != Ark::AUTO)
        chooseGLVersion(version);
}

void RenderEngine::initialize()
{
    if(_gl_context->version() == Ark::AUTO)
    {
        int glMajorVersion = 0, glMinorVersion = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &glMajorVersion);
        glGetIntegerv(GL_MINOR_VERSION, &glMinorVersion);
        if(glMajorVersion != 0)
            chooseGLVersion(static_cast<Ark::GLVersion>(glMajorVersion * 10 + glMinorVersion));
        else
            chooseGLVersion(Ark::OPENGL_20);
    }
}

sp<RenderView> RenderEngine::createRenderView(const Viewport& viewport) const
{
    return _render_view_factory->createRenderView(_gl_context, viewport);
}

sp<GLSnippet> RenderEngine::createCoreGLSnippet(const sp<GLResourceManager>& glResourceManager, const sp<GLShader>& shader, const GLBuffer& arrayBuffer) const
{
    return _gl_procedure_factory->createCoreGLSnippet(glResourceManager, shader, arrayBuffer);
}

void RenderEngine::chooseGLVersion(Ark::GLVersion version)
{
    DCHECK(version != Ark::AUTO, "Cannot set OpenGL version to \"auto\" manually.");
    LOGD("Choose GLVersion = %d", version);
    std::map<String, String>& annotations = _gl_context->annotations();
    if(version == Ark::OPENGL_20 || version == Ark::OPENGL_21)
    {
        annotations["vert.in"] = "attribute";
        annotations["vert.out"] = "varying";
        annotations["frag.in"] = "varying";
        annotations["frag.out"] = "varying";
        annotations["frag.color"] = "gl_FragColor";
        _gl_procedure_factory = sp<gles20::GLProcedureFactoryGLES20>::make();
    }
    else
    {
        annotations["vert.in"] = "in";
        annotations["vert.out"] = "out";
        annotations["frag.in"] = "in";
        annotations["frag.out"] = "out";
        annotations["frag.color"] = "v_FragColor";
        _gl_procedure_factory = sp<gles30::GLProcedureFactoryGLES30>::make();
    }
    _gl_context->setVersion(version);
}

}
