#include "renderer/opengl/renderer_factory/renderer_factory_opengl.h"

#include "core/base/plugin_manager.h"
#include "core/types/global.h"
#include "core/util/log.h"

#include "renderer/base/gl_context.h"

#include "renderer/opengl/pipeline_factory/pipeline_factory_opengl.h"
#include "renderer/opengl/render_view/render_view_opengl.h"
#include "renderer/opengl/es20/gl_snippet_factory/gl_snippet_factory_gles20.h"
#include "renderer/opengl/es30/gl_snippet_factory/gl_snippet_factory_gles30.h"

#include "generated/opengl_plugin.h"

#include "platform/gl/gl.h"

namespace ark {
namespace opengl {

RendererFactoryOpenGL::RendererFactoryOpenGL(const sp<GLResourceManager>& glResources)
    : _resource_manager(glResources)
{
    const Global<PluginManager> pluginManager;
    pluginManager->addPlugin(sp<opengl::OpenglPlugin>::make());
}

void RendererFactoryOpenGL::initialize(GLContext& glContext)
{
    DTHREAD_CHECK(THREAD_ID_RENDERER);

    if(glContext.version() == Ark::AUTO)
    {
        int glMajorVersion = 0, glMinorVersion = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &glMajorVersion);
        glGetIntegerv(GL_MINOR_VERSION, &glMinorVersion);
        if(glMajorVersion != 0)
            setGLVersion(static_cast<Ark::RendererVersion>(glMajorVersion * 10 + glMinorVersion), glContext);
        else
            setGLVersion(Ark::OPENGL_20, glContext);
    }
}

void RendererFactoryOpenGL::setGLVersion(Ark::RendererVersion version, GLContext& glContext)
{
    DCHECK(version != Ark::AUTO, "Cannot set OpenGL version to \"auto\" manually.");
    LOGD("Choose GLVersion = %d", version);
    std::map<String, String>& annotations = glContext.annotations();
    if(version == Ark::OPENGL_20 || version == Ark::OPENGL_21)
    {
        annotations["vert.in"] = "attribute";
        annotations["vert.out"] = "varying";
        annotations["frag.in"] = "varying";
        annotations["frag.out"] = "varying";
        annotations["frag.color"] = "gl_FragColor";
        glContext.setGLSnippetFactory(sp<gles20::GLSnippetFactoryGLES20>::make());
    }
    else
    {
        annotations["vert.in"] = "in";
        annotations["vert.out"] = "out";
        annotations["frag.in"] = "in";
        annotations["frag.out"] = "out";
        annotations["frag.color"] = "v_FragColor";
        glContext.setGLSnippetFactory(sp<gles30::GLSnippetFactoryGLES30>::make());
    }
    glContext.setVersion(version);
}

sp<RenderView> RendererFactoryOpenGL::createRenderView(const sp<GLContext>& glContext, const Viewport& viewport)
{
    return sp<RenderView>::adopt(new RenderViewOpenGL(glContext, _resource_manager, viewport));
}

sp<PipelineFactory> RendererFactoryOpenGL::createPipelineFactory()
{
    return sp<PipelineFactoryOpenGL>::make();
}

}
}
