#include "renderer/opengl/renderer_factory/renderer_factory_opengl.h"

#include "core/base/plugin_manager.h"
#include "core/types/global.h"
#include "core/util/log.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/camera.h"
#include "graphics/base/size.h"
#include "graphics/base/viewport.h"

#include "renderer/base/framebuffer.h"
#include "renderer/base/render_engine_context.h"
#include "renderer/base/render_controller.h"
#include "renderer/base/texture.h"

#include "renderer/opengl/base/gl_buffer.h"
#include "renderer/opengl/base/gl_cubemap.h"
#include "renderer/opengl/base/gl_framebuffer.h"
#include "renderer/opengl/base/gl_texture_2d.h"
#include "renderer/opengl/pipeline_factory/pipeline_factory_opengl.h"
#include "renderer/opengl/renderer/gl_framebuffer_renderer.h"
#include "renderer/opengl/render_view/render_view_opengl.h"
#include "renderer/opengl/es20/snippet_factory/snippet_factory_gles20.h"
#include "renderer/opengl/es30/snippet_factory/snippet_factory_gles30.h"
#include "renderer/opengl/util/gl_util.h"

#include "generated/opengl_plugin.h"

#include "platform/platform.h"
#include "platform/gl/gl.h"

namespace ark {
namespace opengl {

RendererFactoryOpenGL::RendererFactoryOpenGL(const sp<Recycler>& recycler)
    : _recycler(recycler)
{
    const Global<PluginManager> pluginManager;
    pluginManager->addPlugin(sp<opengl::OpenglPlugin>::make());
}

sp<RenderEngineContext> RendererFactoryOpenGL::initialize(Ark::RendererVersion version)
{
    const sp<RenderEngineContext> renderContext = sp<RenderEngineContext>::make(version, Ark::COORDINATE_SYSTEM_RHS, Viewport(-1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f));
    if(version != Ark::RENDERER_VERSION_AUTO)
        setVersion(version, renderContext);
    return renderContext;
}

void RendererFactoryOpenGL::onSurfaceCreated(RenderEngineContext& glContext)
{
    DTHREAD_CHECK(THREAD_ID_RENDERER);

    Platform::glInitialize();

    if(glContext.version() == Ark::RENDERER_VERSION_AUTO)
    {
        int glMajorVersion = 0, glMinorVersion = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &glMajorVersion);
        glGetIntegerv(GL_MINOR_VERSION, &glMinorVersion);
        if(glMajorVersion != 0)
            setVersion(static_cast<Ark::RendererVersion>(glMajorVersion * 10 + glMinorVersion), glContext);
        else
            setVersion(Ark::RENDERER_VERSION_OPENGL_20, glContext);
    }
}

void RendererFactoryOpenGL::setVersion(Ark::RendererVersion version, RenderEngineContext& glContext)
{
    LOGD("Choose GLVersion = %d", version);
    std::map<String, String>& annotations = glContext.definitions();
    if(version == Ark::RENDERER_VERSION_OPENGL_20 || version == Ark::RENDERER_VERSION_OPENGL_21)
    {
        annotations["vert.in"] = "attribute";
        annotations["vert.out"] = "varying";
        annotations["frag.in"] = "varying";
        annotations["frag.out"] = "varying";
        annotations["frag.color"] = "gl_FragColor";
        glContext.setSnippetFactory(sp<gles20::SnippetFactoryGLES20>::make());
    }
    else
    {
        annotations["vert.in"] = "in";
        annotations["vert.out"] = "out";
        annotations["frag.in"] = "in";
        annotations["frag.out"] = "out";
        annotations["frag.color"] = "f_FragColor";
        glContext.setSnippetFactory(sp<gles30::SnippetFactoryGLES30>::make());
    }
    glContext.setVersion(version);
}

sp<RenderView> RendererFactoryOpenGL::createRenderView(const sp<RenderEngineContext>& renderContext, const sp<RenderController>& renderController)
{
    return sp<RenderView>::adopt(new RenderViewOpenGL(renderContext, renderController));
}

sp<Buffer::Uploader> RendererFactoryOpenGL::createBuffer(Buffer::Type type, Buffer::Usage usage)
{
    return sp<GLBuffer>::make(type, usage, _recycler);
}

sp<Camera::Delegate> RendererFactoryOpenGL::createCamera(Ark::RendererCoordinateSystem cs)
{
    return cs == Ark::COORDINATE_SYSTEM_RHS ? sp<Camera::Delegate>::make<Camera::DelegateRH_NO>() : sp<Camera::Delegate>::make<Camera::DelegateLH_NO>();
}

sp<Framebuffer> RendererFactoryOpenGL::createFramebuffer(sp<Renderer> renderer, std::vector<sp<Texture>> colorAttachments, sp<Texture> depthStencilAttachments, int32_t clearMask)
{
    DCHECK(colorAttachments.size() > 0, "Framebuffer object should have at least one color attachment");
    int32_t width = colorAttachments.at(0)->width();
    int32_t height = colorAttachments.at(0)->height();
    uint32_t drawBufferCount = static_cast<uint32_t>(colorAttachments.size());
    sp<GLFramebuffer> fbo = sp<GLFramebuffer>::make(_recycler, std::move(colorAttachments), std::move(depthStencilAttachments));
    return sp<Framebuffer>::make(sp<GLFramebufferRenderer>::make(fbo, width, height, std::move(renderer), drawBufferCount, clearMask), std::move(fbo));
}

sp<PipelineFactory> RendererFactoryOpenGL::createPipelineFactory()
{
    return sp<PipelineFactoryOpenGL>::make();
}

sp<Texture::Delegate> RendererFactoryOpenGL::createTexture(sp<Size> size, sp<Texture::Parameters> parameters)
{
    if(parameters->_type == Texture::TYPE_2D)
        return sp<GLTexture2D>::make(_recycler, std::move(size), std::move(parameters));
    else if(parameters->_type == Texture::TYPE_CUBEMAP)
        return sp<GLCubemap>::make(_recycler, std::move(size), std::move(parameters));
    DFATAL("Unsupported texture type: %d", parameters->_type);
    return nullptr;
}

}
}
